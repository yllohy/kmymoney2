/***************************************************************************
                          kmymoney2.cpp
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes
    email                : mte@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "kdecompat.h"

#include <stdio.h>
#include <iostream>

// ----------------------------------------------------------------------------
// QT Includes

#include <qdir.h>
#include <qprinter.h>
#include <qlayout.h>
#include <qsignalmapper.h>
#include <qclipboard.h>        // temp for problem 1105503
#include <qmessagebox.h>       // ditto
#include <qdatetime.h>         // only for performance tests
#include <qtimer.h>
#include <qsqlpropertymap.h>
#include <qcheckbox.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kdebug.h>
#include <kapplication.h>
#include <kshortcut.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kmenubar.h>
#include <klocale.h>
#include <kconfig.h>
#include <kstdaction.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kstatusbar.h>
#include <ktip.h>
#include <kkeydialog.h>
#include <kprogress.h>
#include <kio/netaccess.h>
#include <dcopclient.h>
#include <kstartupinfo.h>
#include <kparts/componentfactory.h>
#include <kedittoolbar.h>

#if !KDE_IS_VERSION(3,2,0)
#include <kwin.h>
#endif

#include <krun.h>
#include <kconfigdialog.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoney2.h"
#include "kmymoneysettings.h"
#include "kmymoney2_stub.h"

#include "dialogs/kstartdlg.h"
// #include "dialogs/ksettingsdlg.h"
#include "dialogs/settings/ksettingsgeneral.h"
#include "dialogs/settings/ksettingsregister.h"
#include "dialogs/settings/ksettingsgpg.h"
#include "dialogs/settings/ksettingscolors.h"
#include "dialogs/settings/ksettingsfonts.h"
#include "dialogs/settings/ksettingsschedules.h"
#include "dialogs/settings/ksettingsonlinequotes.h"
#include "dialogs/settings/ksettingshome.h"
#include "dialogs/kbackupdlg.h"
#include "dialogs/kexportdlg.h"
#include "dialogs/kimportdlg.h"
#include "dialogs/mymoneyqifprofileeditor.h"
#include "dialogs/kimportverifydlg.h"
#include "dialogs/kenterscheduledialog.h"
#include "dialogs/kmymoneypricedlg.h"
#include "dialogs/kcurrencyeditdlg.h"
#include "dialogs/kequitypriceupdatedlg.h"
#include "dialogs/ksecuritylisteditor.h"
#include "dialogs/kmymoneyfileinfodlg.h"
#include "dialogs/kfindtransactiondlg.h"
#include "dialogs/knewbankdlg.h"
#include "dialogs/knewaccountwizard.h"
#include "dialogs/knewinvestmentwizard.h"
#include "dialogs/knewaccountdlg.h"
#include "dialogs/knewfiledlg.h"
#include "dialogs/kselectdatabasedlg.h"
#include "dialogs/kcurrencycalculator.h"
#include "dialogs/ieditscheduledialog.h"
#include "dialogs/knewloanwizard.h"
#include "dialogs/keditloanwizard.h"
#include "dialogs/ktransactionreassigndlg.h"

#ifdef USE_OFX_DIRECTCONNECT
#include "dialogs/kofxdirectconnectdlg.h"
#endif

#include "views/kmymoneyview.h"

#include "mymoney/mymoneyutils.h"
#include "mymoney/mymoneystatement.h"
#include "mymoney/storage/mymoneystoragedump.h"

#include "converter/mymoneyqifwriter.h"
#include "converter/mymoneyqifreader.h"
#include "converter/mymoneystatementreader.h"
#include "converter/mymoneytemplate.h"

#include "plugins/kmymoneyplugin.h"
#include "plugins/interfaces/kmmviewinterface.h"
#include "plugins/interfaces/kmmstatementinterface.h"

#include <libkgpgfile/kgpgfile.h>

#include "kmymoneyutils.h"
#include "kdecompat.h"

#define ID_STATUS_MSG 1

KMyMoney2App::KMyMoney2App(QWidget * /*parent*/ , const char* name)
 : KMainWindow(0, name),
 DCOPObject("kmymoney2app"),
 myMoneyView(0),
 m_searchDlg(0),
 m_currentFileEncrypted(false),
 m_autoSaveTimer(0)
{
  ::timetrace("start kmymoney2app constructor");
  // preset the pointer because we need it during the course of this constructor
  kmymoney2 = this;
  config = kapp->config();

  updateCaption(true);

  // initial setup of settings
  KMyMoneyUtils::updateSettings();

  QFrame* frame = new QFrame(this);
  frame->setFrameStyle(QFrame::NoFrame);
  // values for margin (11) and spacing(6) taken from KDialog implementation
  QBoxLayout* layout = new QBoxLayout(frame, QBoxLayout::TopToBottom, 11, 6);

  ::timetrace("init statusbar");
  initStatusBar();
  ::timetrace("init actions");
  initActions();

  ::timetrace("create view");
  myMoneyView = new KMyMoneyView(frame, "KMyMoneyView");
  layout->addWidget(myMoneyView, 10);

  ///////////////////////////////////////////////////////////////////
  // call inits to invoke all other construction parts
  ::timetrace("init options");
  readOptions();

  m_pluginSignalMapper = new QSignalMapper( this );
  connect( m_pluginSignalMapper, SIGNAL( mapped( const QString& ) ), this, SLOT( slotPluginImport( const QString& ) ) );

  // now initialize the plugin structure
  ::timetrace("load plugins");
  createInterfaces();
  loadPlugins();

  setCentralWidget(frame);

  ::timetrace("done");

  connect(&proc,SIGNAL(processExited(KProcess *)),this,SLOT(slotProcessExited()));

  // force to show the home page if the file is closed
  connect(action("file_close"), SIGNAL(activated()), myMoneyView, SLOT(slotShowHomePage()));
  connect(action("view_show_transaction_detail"), SIGNAL(toggled(bool)), myMoneyView, SLOT(slotShowTransactionDetail(bool)));

  m_backupState = BACKUP_IDLE;

  m_reader = 0;
  m_smtReader = 0;
  m_engineBackup = 0;

  //this initializes Auto Saving related stuff
  config->setGroup("General Options");
  m_autoSaveEnabled = config->readBoolEntry("AutoSaveFile", false);
  m_autoSavePeriod = config->readNumEntry("AutoSavePeriod", 0);

  m_autoSaveTimer = new QTimer(this);
  connect(m_autoSaveTimer, SIGNAL(timeout()), this, SLOT(slotAutoSave()));

  // make sure, we get a note when the engine changes state
  MyMoneyFile::instance()->attach(MyMoneyFile::NotifyClassAnyChange, this);
}

KMyMoney2App::~KMyMoney2App()
{
  MyMoneyFile::instance()->detach(MyMoneyFile::NotifyClassAnyChange, this);

  if(m_searchDlg)
    delete m_searchDlg;

  if(m_reader != 0)
    delete m_reader;

  if(m_engineBackup != 0)
    delete m_engineBackup;
}

const KURL KMyMoney2App::lastOpenedURL(void)
{
  KURL url = m_startDialog ? KURL() : m_fileName;

  if(!url.isValid())
  {
    url = readLastUsedFile();
  }

  ready();

  return url;
}

void KMyMoney2App::initActions()
{
  // *************
  // The File menu
  // *************
  KStdAction::openNew(this, SLOT(slotFileNew()), actionCollection());
  KStdAction::open(this, SLOT(slotFileOpen()), actionCollection());
  KStdAction::openRecent(this, SLOT(slotFileOpenRecent(const KURL&)), actionCollection());
  KStdAction::save(this, SLOT(slotFileSave()), actionCollection());
  KStdAction::saveAs(this, SLOT(slotFileSaveAs()), actionCollection());
  KStdAction::close(this, SLOT(slotFileClose()), actionCollection());
  KStdAction::quit(this, SLOT(slotFileQuit()), actionCollection());
  KStdAction::print(this, SLOT(slotPrintView()), actionCollection());

  new KAction(i18n("Open database ..."), "",0,this,SLOT(slotOpenDatabase()),actionCollection(),"open_database");
  new KAction(i18n("Save as database ..."), "",0,this,SLOT(slotSaveAsDatabase()),actionCollection(),"saveas_database");
  new KAction(i18n("Backup..."), "backup",0,this,SLOT(slotFileBackup()),actionCollection(),"file_backup");
  new KAction(i18n("QIF ..."), "", 0, this, SLOT(slotQifImport()), actionCollection(), "file_import_qif");
  new KAction(i18n("Gnucash ..."), "", 0, this, SLOT(slotGncImport()), actionCollection(), "file_import_gnc");
  new KAction(i18n("Statement file ..."), "", 0, this, SLOT(slotStatementImport()), actionCollection(), "file_import_statement");

  new KAction(i18n("Account Template ..."), "", 0, this, SLOT(slotLoadAccountTemplates()), actionCollection(), "file_import_template");
  new KAction(i18n("Account Template ..."), "", 0, this, SLOT(slotSaveAccountTemplates()), actionCollection(), "file_export_template");
  new KAction(i18n("QIF ..."), "", 0, this, SLOT(slotQifExport()), actionCollection(), "file_export_qif");
  new KAction(i18n("Personal Data..."), "personal_data", 0, this, SLOT(slotFileViewPersonal()), actionCollection(), "view_personal_data");

#if KMM_DEBUG
  new KAction(i18n("Dump Memory"), "", 0, this, SLOT(slotFileFileInfo()),  actionCollection(), "file_dump");
  new KAction(i18n("File-Information..."), "info", 0, this, SLOT(slotFileInfoDialog()), actionCollection(), "view_file_info");
#endif

  // *************
  // The Edit menu
  // *************
  new KAction(i18n("Find transaction..."), "transaction_find", KShortcut("Ctrl+F"), this, SLOT(slotFindTransaction()), actionCollection(), "edit_find_transaction");

  // *************
  // The View menu
  // *************
  new KToggleAction(i18n("Show Transaction Detail"), KShortcut("Ctrl+T"), actionCollection(), "view_show_transaction_detail");
  new KToggleAction(i18n("Hide reconciled transactions"), "", KShortcut("Ctrl+R"), this, SLOT(slotHideReconciledTransactions()), actionCollection(), "view_hide_reconciled_transactions");
  new KToggleAction(i18n("Hide unused categories"), "", KShortcut("Ctrl+U"), this, SLOT(slotHideUnusedCategories()), actionCollection(), "view_hide_unused_categories");

  // *********************
  // The institutions menu
  // *********************
  new KAction(i18n("New institution..."), "institution_add", 0, this, SLOT(slotInstitutionNew()), actionCollection(), "institution_new");
  new KAction(i18n("Edit institution..."), "edit", 0, this, SLOT(slotInstitutionEdit()), actionCollection(), "institution_edit");
  new KAction(i18n("Delete institution..."), "delete", 0, this, SLOT(slotInstitutionDelete()), actionCollection(), "institution_delete");

  // *****************
  // The accounts menu
  // *****************
  new KAction(i18n("New account..."), "account_add", 0, this, SLOT(slotAccountNew()), actionCollection(), "account_new");
  new KAction(i18n("New category..."), "category_add", 0, this, SLOT(slotCategoryNew()), actionCollection(), "category_new");
  new KAction(i18n("Open ledger"), "account", 0, this, SLOT(slotAccountOpen()), actionCollection(), "account_open");
  new KAction(i18n("Reconcile..."), "reconcile", KShortcut("Ctrl+Shift+R"), this, SLOT(slotAccountReconcile()), actionCollection(), "account_reconcile");
  new KAction(i18n("Delete account..."), "delete", 0, this, SLOT(slotAccountDelete()), actionCollection(), "account_delete");
  new KAction(i18n("Edit account..."), "edit", 0, this, SLOT(slotAccountEdit()), actionCollection(), "account_edit");

#ifdef USE_OFX_DIRECTCONNECT
  new KAction(i18n("Online update using OFX..."), "account", 0, this, SLOT(slotAccountUpdateOFX()), actionCollection(), "account_update_ofx");
#endif

  // **************
  // The tools menu
  // **************
  new KAction(i18n("QIF Profile Editor..."), "edit", 0, this, SLOT(slotQifProfileEditor()), actionCollection(), "tools_qif_editor");
  new KAction(i18n("Securities ..."), "", 0, this, SLOT(slotSecurityEditor()), actionCollection(), "tools_security_editor");
  new KAction(i18n("Currencies ..."), "", 0, this, SLOT(slotCurrencyDialog()), actionCollection(), "tools_currency_editor");
  new KAction(i18n("Prices ..."), "", 0, this, SLOT(slotPriceDialog()), actionCollection(), "tools_price_editor");
  new KAction(i18n("Update Stock and Currency Prices..."), "", 0, this, SLOT(slotEquityPriceUpdate()), actionCollection(), "tools_update_prices");
  new KAction(i18n("Consistency Check"), "", 0, this, SLOT(slotFileConsitencyCheck()), actionCollection(), "tools_consistency_check");
  new KAction(i18n("Performance-Test"), "fork", 0, this, SLOT(slotPerformanceTest()), actionCollection(), "tools_performancetest");
  new KAction(i18n("KCalc..."), "kcalc", 0, this, SLOT(slotToolsStartKCalc()), actionCollection(), "tools_kcalc");

  // *****************
  // The settings menu
  // *****************
  KStdAction::showToolbar(this, SLOT(slotViewToolBar()), actionCollection());
  KStdAction::showStatusbar(this, SLOT(slotViewStatusBar()), actionCollection());
  KStdAction::keyBindings(this, SLOT(slotKeySettings()), actionCollection());
  KStdAction::configureToolbars( this, SLOT( slotEditToolbars() ), actionCollection());
  KStdAction::preferences(this, SLOT( slotSettings() ), actionCollection());
  new KAction(i18n("Enable all messages"), "", 0, this, SLOT(slotEnableMessages()), actionCollection(), "settings_enable_messages");

  // *************
  // The help menu
  // *************
  new KAction(i18n("&Show tip of the day"), "idea", 0, this, SLOT(slotShowTipOfTheDay()), actionCollection(), "help_show_tip");

  // ***************************
  // Actions w/o main menu entry
  // ***************************
  new KAction(i18n("New transaction"), QKeySequence(Qt::CTRL | Qt::Key_Insert), actionCollection(), "transaction_new");
  new KAction(i18n("New investment"), "file_new", 0, this, SLOT(slotInvestmentNew()), actionCollection(), "investment_new");
  new KAction(i18n("Edit investment..."), "edit", 0, this, SLOT(slotInvestmentEdit()), actionCollection(), "investment_edit");
  new KAction(i18n("Delete investment..."), "delete", 0, this, SLOT(slotInvestmentDelete()), actionCollection(), "investment_delete");
  new KAction(i18n("Online price update..."), "", 0, this, SLOT(slotOnlinePriceUpdate()), actionCollection(), "investment_online_price_update");
  new KAction(i18n("Manual price update..."), "", 0, this, SLOT(slotManualPriceUpdate()), actionCollection(), "investment_manual_price_update");

  new KAction(i18n("New bill..."), "", 0, this, SLOT(slotScheduleNewBill()), actionCollection(), "schedule_new_bill");
  new KAction(i18n("New deposit..."), "", 0, this, SLOT(slotScheduleNewDeposit()), actionCollection(), "schedule_new_deposit");
  new KAction(i18n("New transfer..."), "", 0, this, SLOT(slotScheduleNewTransfer()), actionCollection(), "schedule_new_transfer");
  new KAction(i18n("Edit schedule..."), "edit", 0, this, SLOT(slotScheduleEdit()), actionCollection(), "schedule_edit");
  new KAction(i18n("Delete schedule..."), "delete", 0, this, SLOT(slotScheduleDelete()), actionCollection(), "schedule_delete");
  new KAction(i18n("Enter schedule..."), "", 0, this, SLOT(slotScheduleEnter()), actionCollection(), "schedule_enter");

  new KAction(i18n("New payee"), "filenew", 0, this, SLOT(slotPayeeNew()), actionCollection(), "payee_new");
  new KAction(i18n("Rename payee"), "edit", 0, this, SIGNAL(payeeRename()), actionCollection(), "payee_rename");
  new KAction(i18n("Delete payee"), "delete", 0, this, SLOT(slotPayeeDelete()), actionCollection(), "payee_delete");

  // ************************
  // Currently unused actions
  // ************************
#if 0
  new KToolBarPopupAction(i18n("View back"), "back", 0, this, SLOT(slotShowPreviousView()), actionCollection(), "go_back");
  new KToolBarPopupAction(i18n("View forward"), "forward", 0, this, SLOT(slotShowNextView()), actionCollection(), "go_forward");

  action("go_back")->setEnabled(false);
  action("go_forward")->setEnabled(false);
#endif

  // Setup transaction detail switch
  toggleAction("view_show_transaction_detail")->setChecked(KMyMoneySettings::showRegisterDetailed());
  toggleAction("view_hide_reconciled_transactions")->setChecked(KMyMoneySettings::hideReconciledTransactions());
  toggleAction("view_hide_unused_categories")->setChecked(KMyMoneySettings::hideUnusedCategory());

  // use the absolute path to your kmymoney2ui.rc file for testing purpose in createGUI();
  createGUI(QString::null, false);
}

void KMyMoney2App::dumpActions() const
{
  KActionPtrList list = actionCollection()->actions();
  KActionPtrList::const_iterator it;
  for(it = list.begin(); it != list.end(); ++it) {
    std::cout << (*it)->name() << ": " << (*it)->text() << std::endl;
  }
}

KAction* KMyMoney2App::action(const QString& actionName) const
{
  static KShortcut shortcut("");
  static KAction dummyAction(QString("Dummy"), QString(), shortcut, static_cast<const QObject*>(this), 0, static_cast<KActionCollection*>(0), "");

  KAction* p = actionCollection()->action(actionName.latin1());
  if(p)
    return p;

  qWarning("Action with name '%s' not found!", actionName.latin1());
  return &dummyAction;
}

KToggleAction* KMyMoney2App::toggleAction(const QString& actionName) const
{
  static KShortcut shortcut("");
  static KToggleAction dummyAction(QString("Dummy"), QString(), shortcut, static_cast<const QObject*>(this), 0, static_cast<KActionCollection*>(0), "");

  KAction* q = actionCollection()->action(actionName.latin1());

  if(q) {
    KToggleAction* p = dynamic_cast<KToggleAction*>(q);
    if(!p) {
      qWarning("Action '%s' is not of type KToggleAction", actionName.latin1());
      p = &dummyAction;
    }
    return p;
  }

  qWarning("Action with name '%s' not found!", actionName.latin1());
  return &dummyAction;
}


void KMyMoney2App::initStatusBar()
{
  ///////////////////////////////////////////////////////////////////
  // STATUSBAR

  statusBar()->insertItem("", ID_STATUS_MSG);
  ready();

  // Initialization of progress bar taken from KDevelop ;-)
  progressBar = new KProgress(statusBar());
  progressBar->setFrameStyle(QFrame::NoFrame | QFrame::Plain);
  progressBar->setMargin(0);
  progressBar->setLineWidth(0);
  progressBar->setBackgroundMode(QWidget::PaletteBackground);
  statusBar()->addWidget(progressBar);
  progressBar->setFixedHeight(progressBar->sizeHint().height() - 8);

  // hide the progress bar for now
  slotStatusProgressBar(-1, -1);
}

void KMyMoney2App::saveOptions()
{
  config->setGroup("General Options");
  config->writeEntry("Geometry", size());
  config->writeEntry("Show Toolbar", toggleAction("options_show_toolbar")->isChecked());
  config->writeEntry("Show Statusbar", toggleAction("options_show_statusbar")->isChecked());
  config->writeEntry("ToolBarPos", (int) toolBar("mainToolBar")->barPos());

  dynamic_cast<KRecentFilesAction*>(action("file_open_recent"))->saveEntries(config,"Recent Files");
}


void KMyMoney2App::readOptions()
{
  config->setGroup("General Options");

  // bar status settings
  toggleAction("options_show_toolbar")->setChecked(config->readBoolEntry("Show Toolbar", true));
  slotViewToolBar();

  toggleAction("options_show_statusbar")->setChecked(config->readBoolEntry("Show Statusbar", true));
  slotViewStatusBar();

  toggleAction("view_hide_reconciled_transactions")->setChecked(KMyMoneySettings::hideReconciledTransactions());
  toggleAction("view_hide_unused_categories")->setChecked(KMyMoneySettings::hideUnusedCategory());

  // bar position settings
  KToolBar::BarPosition toolBarPos;
  toolBarPos=(KToolBar::BarPosition) config->readNumEntry("ToolBarPos", KToolBar::Top);
  toolBar("mainToolBar")->setBarPos(toolBarPos);

  // initialize the recent file list
  KRecentFilesAction *p = dynamic_cast<KRecentFilesAction*>(action("file_open_recent"));
  if(p)
    p->loadEntries(config,"Recent Files");

  QSize size=config->readSizeEntry("Geometry");
  if(!size.isEmpty())
  {
    resize(size);
  }

  // Startdialog is written in the settings dialog
  m_startDialog = config->readBoolEntry("StartDialog", true);

  config->setGroup("Schedule Options");
  m_bCheckSchedules = config->readBoolEntry("CheckSchedules", true);
}

void KMyMoney2App::resizeEvent(QResizeEvent* ev)
{
  KMainWindow::resizeEvent(ev);
  updateCaption(true);
}

bool KMyMoney2App::queryClose()
{
  if(!isReady())
    return false;

  if (myMoneyView->dirty()) {
    int ans = KMessageBox::warningYesNoCancel(this, i18n("KMyMoney file needs saving.  Save ?"));
    if (ans==KMessageBox::Cancel)
      return false;
    else if (ans==KMessageBox::Yes)
      return slotFileSave();
  }
  return true;
}

bool KMyMoney2App::queryExit()
{
  saveOptions();

  return true;
}

/////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATION
/////////////////////////////////////////////////////////////////////
void KMyMoney2App::slotFileInfoDialog(void)
{
  KMyMoneyFileInfoDlg dlg(0);
  dlg.exec();
}

void KMyMoney2App::slotPerformanceTest(void)
{
  // dump performance report to stderr

  int measurement[2];
  QTime timer;
  MyMoneyAccount acc;

  qDebug("--- Starting performance tests ---");

  // AccountList
  MyMoneyFile::instance()->clearCache();
  measurement[0] = measurement[1] = 0;
  for(int i = 0; i < 1000; ++i) {
    QValueList<MyMoneyAccount> list;
    timer.start();
    list = MyMoneyFile::instance()->accountList();
    measurement[i != 0] += timer.elapsed();
  }
  std::cerr << "accountList()" << std::endl;
  std::cerr << "First time: " << measurement[0] << " msec" << std::endl;
  std::cerr << "Average   : " << (measurement[0] + measurement[1]) / 1000 << " msec" << std::endl;

  // Balance of asset account(s)
  MyMoneyFile::instance()->clearCache();
  measurement[0] = measurement[1] = 0;
  acc = MyMoneyFile::instance()->asset();
  for(int i = 0; i < 1000; ++i) {
    timer.start();
    MyMoneyMoney result = MyMoneyFile::instance()->balance(acc.id());
    measurement[i != 0] += timer.elapsed();
  }
  std::cerr << "balance(Asset)" << std::endl;
  std::cerr << "First time: " << measurement[0] << " msec" << std::endl;
  std::cerr << "Average   : " << (measurement[0] + measurement[1]) / 1000 << " msec" << std::endl;

  // total balance of asset account
  MyMoneyFile::instance()->clearCache();
  measurement[0] = measurement[1] = 0;
  acc = MyMoneyFile::instance()->asset();
  for(int i = 0; i < 1000; ++i) {
    timer.start();
    MyMoneyMoney result = MyMoneyFile::instance()->totalBalance(acc.id());
    measurement[i != 0] += timer.elapsed();
  }
  std::cerr << "totalBalance(Asset)" << std::endl;
  std::cerr << "First time: " << measurement[0] << " msec" << std::endl;
  std::cerr << "Average   : " << (measurement[0] + measurement[1]) / 1000 << " msec" << std::endl;

  // Balance of expense account(s)
  MyMoneyFile::instance()->clearCache();
  measurement[0] = measurement[1] = 0;
  acc = MyMoneyFile::instance()->expense();
  for(int i = 0; i < 1000; ++i) {
    timer.start();
    MyMoneyMoney result = MyMoneyFile::instance()->balance(acc.id());
    measurement[i != 0] += timer.elapsed();
  }
  std::cerr << "balance(Expense)" << std::endl;
  std::cerr << "First time: " << measurement[0] << " msec" << std::endl;
  std::cerr << "Average   : " << (measurement[0] + measurement[1]) / 1000 << " msec" << std::endl;

  // total balance of expense account
  MyMoneyFile::instance()->clearCache();
  measurement[0] = measurement[1] = 0;
  acc = MyMoneyFile::instance()->expense();
  for(int i = 0; i < 1000; ++i) {
    timer.start();
    MyMoneyMoney result = MyMoneyFile::instance()->totalBalance(acc.id());
    measurement[i != 0] += timer.elapsed();
  }
  std::cerr << "totalBalance(Expense)" << std::endl;
  std::cerr << "First time: " << measurement[0] << " msec" << std::endl;
  std::cerr << "Average   : " << (measurement[0] + measurement[1]) / 1000 << " msec" << std::endl;

  // total value of expense account
  MyMoneyFile::instance()->clearCache();
  measurement[0] = measurement[1] = 0;
  acc = MyMoneyFile::instance()->expense();
  for(int i = 0; i < 1000; ++i) {
    timer.start();
    MyMoneyMoney result = MyMoneyFile::instance()->totalValue(acc.id());
    measurement[i != 0] += timer.elapsed();
  }
  std::cerr << "totalValue(Expense)" << std::endl;
  std::cerr << "First time: " << measurement[0] << " msec" << std::endl;
  std::cerr << "Average   : " << (measurement[0] + measurement[1]) / 1000 << " msec" << std::endl;

  // total value valid of expense account
  MyMoneyFile::instance()->clearCache();
  measurement[0] = measurement[1] = 0;
  acc = MyMoneyFile::instance()->expense();
  for(int i = 0; i < 1000; ++i) {
    timer.start();
    MyMoneyMoney result = MyMoneyFile::instance()->totalValueValid(acc.id());
    measurement[i != 0] += timer.elapsed();
  }
  std::cerr << "totalValueValid(Expense)" << std::endl;
  std::cerr << "First time: " << measurement[0] << " msec" << std::endl;
  std::cerr << "Average   : " << (measurement[0] + measurement[1]) / 1000 << " msec" << std::endl;

  // transaction list
  MyMoneyFile::instance()->clearCache();
  measurement[0] = measurement[1] = 0;
  if(MyMoneyFile::instance()->asset().accountCount()) {
    MyMoneyTransactionFilter filter(MyMoneyFile::instance()->asset().accountList()[0]);
    filter.setDateFilter(QDate(), QDate::currentDate());
    QValueList<MyMoneyTransaction> list;

    for(int i = 0; i < 100; ++i) {
      timer.start();
      list = MyMoneyFile::instance()->transactionList(filter);
      measurement[i != 0] += timer.elapsed();
    }
    std::cerr << "transactionList()" << std::endl;
    std::cerr << "First time: " << measurement[0] << " msec" << std::endl;
    std::cerr << "Average   : " << (measurement[0] + measurement[1]) / 100 << " msec" << std::endl;
  }

  // transaction list
  MyMoneyFile::instance()->clearCache();
  measurement[0] = measurement[1] = 0;
  if(MyMoneyFile::instance()->asset().accountCount()) {
    MyMoneyTransactionFilter filter(MyMoneyFile::instance()->asset().accountList()[0]);
    filter.setDateFilter(QDate(), QDate::currentDate());
    QValueList<MyMoneyTransaction> list;

    for(int i = 0; i < 100; ++i) {
      timer.start();
      MyMoneyFile::instance()->transactionList(list, filter);
      measurement[i != 0] += timer.elapsed();
    }
    std::cerr << "transactionList(list)" << std::endl;
    std::cerr << "First time: " << measurement[0] << " msec" << std::endl;
    std::cerr << "Average   : " << (measurement[0] + measurement[1]) / 100 << " msec" << std::endl;
  }

}

void KMyMoney2App::slotFileNew()
{
  QString prevMsg = slotStatusMsg(i18n("Creating new document..."));

  slotFileClose();
  if(myMoneyView->fileOpen())
    return;

  m_fileName = KURL();
  m_currentFileEncrypted = false;
  if(myMoneyView->newFile()) {
    KMessageBox::information(this, QString("<p>") +
                  i18n("The next dialog allows you to add predefined account/category templates to the new file. Different languages are available to select from. You can skip loading any template  now by selecting <b>Cancel</b> from the next dialog. If you wish to add more templates later, you can restart this operation by selecting <b>File/Import/Account Templates</b>."),
                  i18n("Load predefined accounts/categories"));
    slotLoadAccountTemplates();
  }

  slotStatusMsg(prevMsg);
  updateCaption();

  emit fileLoaded(m_fileName);
}

// General open
void KMyMoney2App::slotFileOpen()
{
  QString prevMsg = slotStatusMsg(i18n("Open a file."));

  KFileDialog* dialog = new KFileDialog(KGlobalSettings::documentPath(),
                            i18n("%1|KMyMoney files\n%2|All files (*.*)").arg("*.kmy *.xml").arg("*"),
                            this, i18n("Open File..."), true);
  dialog->setMode(KFile::File | KFile::ExistingOnly);

  if(dialog->exec() == QDialog::Accepted) {
    slotFileOpenRecent(dialog->selectedURL());
  }
  delete dialog;

  slotStatusMsg(prevMsg);
}

void KMyMoney2App::slotOpenDatabase() {
  QString prevMsg = slotStatusMsg(i18n("Open a file."));
  KSelectDatabaseDlg dialog;

  if(dialog.exec() == QDialog::Accepted) {
    slotFileOpenRecent(dialog.selectedURL());
  }

  slotStatusMsg(prevMsg);

}

bool KMyMoney2App::isImportableFile( const KURL& url )
{
  bool result = false;

  // Iterate through the plugins and see if there's a loaded plugin who can handle it
  QMap<QString,KMyMoneyPlugin::ImporterPlugin*>::const_iterator it_plugin = m_importerPlugins.begin();
  while ( it_plugin != m_importerPlugins.end() )
  {
    if ( (*it_plugin)->isMyFormat(url.path()) )
    {
      result = true;
      break;
    }
    ++it_plugin;
  }

  // If we did not find a match, try importing it as a KMM statement file,
  // which is really just for testing.  the statement file is not exposed
  // to users.
  if ( it_plugin == m_importerPlugins.end() )
    if ( MyMoneyStatement::isStatementFile( url.path() ) )
      result = true;

  // Place code here to test for QIF and other locally-supported formats
  // (i.e. not a plugin). If you add them here, be sure to add it to
  // the webConnect function.

  return result;
}

void KMyMoney2App::slotFileOpenRecent(const KURL& url)
{
  QString prevMsg = slotStatusMsg(i18n("Loading file..."));
  KURL lastFile = m_fileName;

  // check if there are other instances which might have this file open
  QCStringList list = instanceList();
  QCStringList::ConstIterator it;
  bool duplicate = false;

  for(it = list.begin(); duplicate == false && it != list.end(); ++it) {
    KMyMoney2App_stub* remoteApp = new KMyMoney2App_stub(kapp->dcopClient(), (*it), "kmymoney2app");
    QString remoteFile = remoteApp->filename();
    if(!remoteApp->ok()) {
      qDebug("DCOP error while calling app->filename()");
    } else {
      if(remoteFile == url.url()) {
        duplicate = true;
      }
    }
    delete remoteApp;
  }

  if(!duplicate) {

#if KDE_IS_VERSION(3,2,0)
    if((url.protocol() == "sql") || (url.isValid() && KIO::NetAccess::exists(url, true, this))) {
#else
    if((url.protocol() == "sql") || (url.isValid() && KIO::NetAccess::exists(url))) {
#endif
      slotFileClose();
      if(!myMoneyView->fileOpen()) {
        if(myMoneyView->readFile(url, m_currentFileEncrypted)) {
          if((myMoneyView->isNativeFile() || (url.protocol() == "sql"))) {
            m_fileName = url;
            KRecentFilesAction *p = dynamic_cast<KRecentFilesAction*>(action("file_open_recent"));
            if(p)
              p->addURL( url );
            writeLastUsedFile(url.url());
          } else {
            m_fileName = KURL(); // imported files have no filename
          }
          ::timetrace("Start checking schedules");
          // Check the schedules
          slotCheckSchedules();
          ::timetrace("Done checking schedules");
        }

        updateCaption();
        ::timetrace("Announcing new filename");
        emit fileLoaded(m_fileName);
        ::timetrace("Announcing new filename done");
      }
    } else {
      slotFileClose();
      KMessageBox::sorry(this, QString("<p>")+i18n("<b>%1</b> is either an invalid filename or the file does not exist. You can open another file or create a new one.").arg(url.url()), i18n("File not found"));
    }
  } else {
    KMessageBox::sorry(this, QString("<p>")+i18n("File <b>%1</b> is already opened in another instance of KMyMoney").arg(url.url()), i18n("Duplicate open"));
  }
  slotStatusMsg(prevMsg);
}

const bool KMyMoney2App::slotFileSave()
{
  // if there's nothing changed, there's no need to save anything
  if(!myMoneyView->dirty())
    return true;

  bool rc = false;

  QString prevMsg = slotStatusMsg(i18n("Saving file..."));

  if (m_fileName.isEmpty()) {
    rc = slotFileSaveAs();
    slotStatusMsg(prevMsg);
    return rc;
  }
  if (m_fileName.protocol() == "sql") {
    rc = myMoneyView->saveDatabase(m_fileName);
  } else {
    rc = myMoneyView->saveFile(m_fileName, m_currentFileEncrypted);
  }
  m_autoSaveTimer->stop();

  slotStatusMsg(prevMsg);
  updateCaption();
  return rc;
}

const bool KMyMoney2App::slotFileSaveAs()
{
  bool rc = false;

  QString prevMsg = slotStatusMsg(i18n("Saving file with a new filename..."));
  QString prevDir= ""; // don't prompt file name if not a native file
  if (myMoneyView->isNativeFile()) prevDir = readLastUsedDir();

#if 0
  QString newName=KFileDialog::getSaveFileName(prevDir,//KGlobalSettings::documentPath(),
                                               i18n("*.kmy|KMyMoney files\n""*.xml|XML Files\n""*.ANON.xml|Anonymous Files\n"

                                               "*.*|All files"), this, i18n("Save as..."));
#endif

  QCheckBox* saveEncrypted = new QCheckBox(i18n("Save file encrypted (if supported by filetype)"), 0);
  saveEncrypted->setEnabled(KGPGFile::GPGAvailable() && KGPGFile::keyAvailable(KMyMoneySettings::gpgRecipient()));
  if(saveEncrypted->isEnabled())
    saveEncrypted->setChecked(KMyMoneySettings::writeDataEncrypted());

  // the following code is copied from KFileDialog::getSaveFileName,
  // adjust to our local needs (filetypes etc.) and
  // enhanced to show the saveEncrypted checkbox
  bool specialDir = prevDir.at(0) == ':';
  KFileDialog dlg( specialDir ? prevDir : QString::null,
                   i18n("*.kmy|KMyMoney files\n"
                        "*.xml|XML Files\n"
                        "*.ANON.xml|Anonymous Files\n"
                        "*.*|All files"),
                   this, "filedialog", true, saveEncrypted);
  if ( !specialDir )
    dlg.setSelection( prevDir ); // may also be a filename

  dlg.setOperationMode( KFileDialog::Saving );
  dlg.setCaption(i18n("Save As"));

  if(dlg.exec() == QDialog::Accepted) {

    QString newName = dlg.selectedFile();
    if (!newName.isEmpty()) {
      KRecentFilesAction *p = dynamic_cast<KRecentFilesAction*>(action("file_open_recent"));
      if(p)
        p->addURL( newName );
      writeLastUsedFile(newName);

  // end of copy

      // find last . delimiter
      int nLoc = newName.findRev('.');
      if(nLoc != -1)
      {
        QString strExt, strTemp;
        strTemp = newName.left(nLoc + 1);
        strExt = newName.right(newName.length() - (nLoc + 1));
        if((strExt.find("kmy", 0, FALSE) == -1) && (strExt.find("xml", 0, FALSE) == -1))
        {

          strTemp.append("kmy");
          //append to make complete file name
          newName = strTemp;
        }
      }
      else
      {
        newName.append(".kmy");
      }

      // If this is the anonymous file export, just save it, don't actually take the
      // name, or remember it! Don't even try to encrypt it
      if (newName.right(9).lower() == ".anon.xml")
      {
        rc = myMoneyView->saveFile(newName, false);
      }
      else
      {
        QFileInfo saveAsInfo(newName);

        m_fileName = newName;
        rc = myMoneyView->saveFile(newName, saveEncrypted->isChecked());

        //write the directory used for this file as the default one for next time.
        writeLastUsedDir(newName);
        writeLastUsedFile(newName);
      }
      m_autoSaveTimer->stop();
    }
  }
  slotStatusMsg(prevMsg);
  updateCaption();
  return rc;
}

const bool KMyMoney2App::slotSaveAsDatabase() {

  bool rc = false;
  QString prevMsg = slotStatusMsg(i18n("Saving file to database..."));
  KSelectDatabaseDlg dialog;
  KURL url;

  if(dialog.exec() == QDialog::Accepted) {
    url = dialog.selectedURL();
    rc = myMoneyView->saveAsDatabase(url);
  }
  if (rc) writeLastUsedFile(url.prettyURL());
  m_autoSaveTimer->stop();
  slotStatusMsg(prevMsg);
  updateCaption();
  return rc;
}

void KMyMoney2App::slotFileCloseWindow()
{
  QString prevMsg = slotStatusMsg(i18n("Closing window..."));

  if (myMoneyView->dirty()) {
    int answer = KMessageBox::warningYesNoCancel(this, i18n("The file has been changed, save it ?"));
    if (answer == KMessageBox::Cancel)
      return;
    else if (answer == KMessageBox::Yes)
      slotFileSave();
  }

  close();

  slotStatusMsg(prevMsg);
}

void KMyMoney2App::slotFileClose()
{
  // no update status here, as we might delete the status too early.
  if (myMoneyView->dirty()) {
    int answer = KMessageBox::warningYesNoCancel(this, i18n("The file has been changed, save it ?"));
    if (answer == KMessageBox::Cancel)
      return;
    else if (answer == KMessageBox::Yes)
      slotFileSave();
  }

  slotSelectAccount(MyMoneyAccount());
  slotSelectInstitution(MyMoneyInstitution());
  slotSelectInvestment(MyMoneyAccount());

  myMoneyView->closeFile();
  m_fileName = KURL();
  updateCaption();

  emit fileLoaded(m_fileName);
}

void KMyMoney2App::slotFileQuit()
{
  // don't modify the status message here as this will prevent quit from working!!
  // See the beginning of queryClose() and isReady() why. Thomas Baumgart 2005-10-17

  KMainWindow* w = 0;

  if(memberList) {

    for(w=memberList->first(); w!=0; w=memberList->next()) {
      // only close the window if the closeEvent is accepted. If the user presses Cancel on the saveModified() dialog,

      // the window and the application stay open.
      if(!w->close())
        break;
    }
    // We will only quit if all windows were processed and not cancelled
    if(w == 0)
      kapp->quit();

  } else
      kapp->quit();
}

void KMyMoney2App::slotViewToolBar()
{
  QString prevMsg = slotStatusMsg(i18n("Toggling toolbar..."));
  ///////////////////////////////////////////////////////////////////
  // turn Toolbar on or off
  toolBar("mainToolBar")->setShown(toggleAction("options_show_toolbar")->isChecked());
  slotStatusMsg(prevMsg);
}

void KMyMoney2App::slotViewStatusBar()
{
  QString prevMsg = slotStatusMsg(i18n("Toggle the statusbar..."));
  ///////////////////////////////////////////////////////////////////
  //turn Statusbar on or off
  statusBar()->setShown(toggleAction("options_show_statusbar")->isChecked());
  slotStatusMsg(prevMsg);
}

void KMyMoney2App::slotHideReconciledTransactions(void)
{
  KMyMoneySettings::setHideReconciledTransactions(toggleAction("view_hide_reconciled_transactions")->isChecked());
  myMoneyView->slotRefreshViews();
}

void KMyMoney2App::slotHideUnusedCategories(void)
{
  KMyMoneySettings::setHideUnusedCategory(toggleAction("view_hide_unused_categories")->isChecked());
  myMoneyView->slotRefreshViews();
}

const QString KMyMoney2App::slotStatusMsg(const QString &text)
{
  ///////////////////////////////////////////////////////////////////
  // change status message permanently
  QString msg = m_statusMsg;

  m_statusMsg = text;
  statusBar()->clear();
  statusBar()->changeItem(text, ID_STATUS_MSG);

  return msg;
}

void KMyMoney2App::ready(void)
{
  slotStatusMsg(i18n("Ready."));
}

bool KMyMoney2App::isReady(void)
{
  return m_statusMsg == i18n("Ready.");
}

void KMyMoney2App::slotStatusProgressBar(const int current, const int total)
{
  if(total == -1 && current == -1) {      // reset
    progressBar->reset();
    progressBar->hide();

  } else if(total != 0) {                 // init
    progressBar->setTotalSteps(total);
    progressBar->show();

    // make sure, we don't waste too much time for updateing the screen.
    // if we have more than 1000 steps, we update the progress bar
    // every 100 steps. If we have less, we allow to update
    // every 10 steps.
    m_progressUpdate = 1;
    if(total > 100)
      m_progressUpdate = 10;
    if(total > 1000)
      m_progressUpdate = 100;
    m_nextUpdate = 0;

  } else {                                // update
    if(current > m_nextUpdate) {
      progressBar->setProgress(current);
      qApp->processEvents();
      m_nextUpdate += m_progressUpdate;
    }
  }
}

void KMyMoney2App::progressCallback(int current, int total, const QString& msg)
{
  if(!msg.isEmpty())
    kmymoney2->slotStatusMsg(msg);

  kmymoney2->slotStatusProgressBar(current, total);
}

void KMyMoney2App::slotFileViewPersonal()
{
  QString prevMsg = slotStatusMsg(i18n("Viewing personal data..."));


  if ( !myMoneyView->fileOpen() ) {

    KMessageBox::information(this, i18n("No KMyMoneyFile open"));
    return;
  }

  MyMoneyFile* file = MyMoneyFile::instance();
  MyMoneyPayee user = file->user();

  KNewFileDlg newFileDlg(user.name(), user.address(),
    user.city(), user.state(), user.postcode(), user.telephone(),
    user.email(), this, "NewFileDlg", i18n("Edit Personal Data"));

  if (newFileDlg.exec())
  {
    user.setName(newFileDlg.userNameText);
    user.setAddress(newFileDlg.userStreetText);
    user.setCity(newFileDlg.userTownText);
    user.setState(newFileDlg.userCountyText);
    user.setPostcode(newFileDlg.userPostcodeText);
    user.setTelephone(newFileDlg.userTelephoneText);
    user.setEmail(newFileDlg.userEmailText);
    file->setUser(user);
  }

  slotStatusMsg(prevMsg);
}

void KMyMoney2App::slotFileFileInfo()
{
  if ( !myMoneyView->fileOpen() ) {
    KMessageBox::information(this, i18n("No KMyMoneyFile open"));
    return;
  }

  QFile g( "kmymoney2.dump" );
  g.open( IO_WriteOnly );
  QDataStream st(&g);
  MyMoneyStorageDump dumper;
  dumper.writeStream(st, dynamic_cast<IMyMoneySerialize*> (MyMoneyFile::instance()->storage()));
  g.close();
}

void KMyMoney2App::slotLoadAccountTemplates(void)
{
  QString prevMsg = slotStatusMsg(i18n("Importing account templates."));

  // create a dialog that drops the user in the base directory for templates
  QLabel* label = new QLabel(i18n("Change into one of the directories and select the desired file."), 0);
  KFileDialog* dialog = new KFileDialog(KGlobal::dirs()->findResourceDir("appdata", "templates/README")+"templates",
                                        i18n("*.kmt|Account templates"),
                                        this, "defaultaccounts",
                                        true,
                                        label);
  dialog->setMode(KFile::Files | KFile::ExistingOnly);
  dialog->setCaption(i18n("Select account template(s)"));

  if(dialog->exec() == QDialog::Accepted) {
    MyMoneyFile::instance()->suspendNotify(true);
    loadAccountTemplates(dialog->selectedFiles());
    MyMoneyFile::instance()->suspendNotify(false);
    myMoneyView->slotRefreshViews();
  }
  delete dialog;

  slotStatusMsg(prevMsg);
}

void KMyMoney2App::slotSaveAccountTemplates(void)
{
  QString prevMsg = slotStatusMsg(i18n("Exporting account templates."));

  QString newName = KFileDialog::getSaveFileName(KGlobalSettings::documentPath(),
                                               i18n("*.kmt|KMyMoney template files\n"
                                               "*.*|All files"), this, i18n("Save as..."));
  //
  // If there is no file extension, then append a .kmt at the end of the file name.
  // If there is a file extension, make sure it is .kmt, delete any others.
  //
  if(!newName.isEmpty())
  {
    // find last . delimiter
    int nLoc = newName.findRev('.');
    if(nLoc != -1)
    {
      QString strExt, strTemp;
      strTemp = newName.left(nLoc + 1);
      strExt = newName.right(newName.length() - (nLoc + 1));
      if((strExt.find("kmt", 0, FALSE) == -1))
      {
        strTemp.append("kmt");
        //append to make complete file name
        newName = strTemp;
      }
    }
    else
    {
      newName.append(".kmt");
    }

    MyMoneyTemplate templ;
    templ.exportTemplate(&progressCallback);
    templ.saveTemplate(newName);
  }
  slotStatusMsg(prevMsg);
}

void KMyMoney2App::loadAccountTemplates(const QStringList& filelist)
{
  QStringList::ConstIterator it;
  for(it = filelist.begin(); it != filelist.end(); ++it) {
    MyMoneyTemplate templ;
    if(templ.loadTemplate(*it)) {
      templ.importTemplate(&progressCallback);
    }
  }
}

void KMyMoney2App::slotQifImport()
{
  if(m_reader == 0) {
    // FIXME: the menu entry for qif import should be disabled here

    KImportDlg* dlg = new KImportDlg(0);

    if(dlg->exec()) {
      slotStatusMsg(i18n("Importing file..."));
      m_reader = new MyMoneyQifReader;
      connect(m_reader, SIGNAL(importFinished()), this, SLOT(slotQifImportFinished()));

      myMoneyView->suspendUpdate(true);

      // construct a copy of the current engine
      if(m_engineBackup)
        delete m_engineBackup;
      m_engineBackup = MyMoneyFile::instance()->storage()->duplicate();

      m_reader->setFilename(dlg->filename());

      m_reader->setProfile(dlg->profile());
      m_reader->setAutoCreatePayee(dlg->autoCreatePayee());
      m_reader->setProgressCallback(&progressCallback);

      // disable all standard widgets during the import
      setEnabled(false);

      m_reader->startImport();
    }
    delete dlg;
  }
}

void KMyMoney2App::slotQifImportFinished(void)
{
  MyMoneyFile* file = MyMoneyFile::instance();

  myMoneyView->suspendUpdate(false);
  if(m_reader != 0) {
    // fixme: re-enable the QIF import menu options
    if(m_reader->finishImport()) {
      if(verifyImportedData(m_reader->account())) {
        // keep the new data set, destroy the backup copy
        delete m_engineBackup;
        m_engineBackup = 0;
      }
    }

    if(m_engineBackup != 0) {
      // user cancelled, destroy the updated set and keep the backup copy
      IMyMoneyStorage* data = file->storage();


      if(data != 0) {
        file->detachStorage(data);
        delete data;
      }
      file->attachStorage(m_engineBackup);
      m_engineBackup = 0;

    }

    // update the views as they might still contain invalid data
    // from the import session. The same applies for the window caption
    myMoneyView->slotRefreshViews();
    updateCaption();

    // slotStatusMsg(prevMsg);
    delete m_reader;
    m_reader = 0;
  }
  slotStatusProgressBar(-1, -1);
  ready();

  // re-enable all standard widgets
  setEnabled(true);
}

void KMyMoney2App::slotGncImport(void)
{
  if (myMoneyView->fileOpen()) {
    switch (KMessageBox::questionYesNoCancel (0,
          i18n("You cannot import GnuCash data into an existing file. Do you wish to save this file?"), PACKAGE)) {
    case KMessageBox::Yes:
      slotFileSave();
      break;
    case KMessageBox::No:
      myMoneyView->closeFile();
      m_fileName = KURL();
      break;
    default:
      return;
    }
  }

  QString prevMsg = slotStatusMsg(i18n("Importing a Gnucash file."));

  KFileDialog* dialog = new KFileDialog(KGlobalSettings::documentPath(),
                            i18n("%1|Gnucash files\n%2|All files (*.*)").arg("*").arg("*"),
                            this, i18n("Import Gnucash file..."), true);
  dialog->setMode(KFile::File | KFile::ExistingOnly);

  if(dialog->exec() == QDialog::Accepted) {
//    slotFileClose();
//    if(myMoneyView->fileOpen())
//      return;

    // call the importer
    myMoneyView->readFile(dialog->selectedURL(), m_currentFileEncrypted);
    // imported files don't have a name
    m_fileName = KURL();

    updateCaption();
    emit fileLoaded(m_fileName);
  }
  delete dialog;

  slotStatusMsg(prevMsg);
}

void KMyMoney2App::slotPluginImport(const QString& format)
{
  kdDebug() << __PRETTY_FUNCTION__ << ": Activated '" << format << "'plugin." << endl;

  if ( m_importerPlugins.contains(format) )
  {
    KMyMoneyPlugin::ImporterPlugin* plugin = m_importerPlugins[format];

    QString prevMsg = slotStatusMsg(i18n("Importing a statement using %1 plugin").arg(format));

    KFileDialog* dialog = new KFileDialog
    (
      KGlobalSettings::documentPath(),
      i18n("%1|%2 files (%3)\n*.*|All files (*.*)")
        .arg(plugin->formatFilenameFilter().lower())
        .arg(format)
        .arg(plugin->formatFilenameFilter().lower()),
      this,
      i18n("Import %1 Statement...").arg(format),
      true
    );

    dialog->setMode(KFile::File | KFile::ExistingOnly);

    if(dialog->exec() == QDialog::Accepted)
    {
      if ( plugin->isMyFormat(dialog->selectedURL().path()) )
      {
        QValueList<MyMoneyStatement> statements;
        if ( plugin->import(dialog->selectedURL().path(),statements) )
        {
#if 0
          /*
           * This section is under construction
           */

          bool ok = true;
          unsigned nerrors = plugin->errors().count();
          unsigned nwarns = plugin->warnings().count();

          if ( nerrors || nwarns )
          {
            int answer = KMessageBox::warningYesNoCancel(this, i18n("There were %1 errors and %2 warnings found when processing this file.  Would you like to view them?").arg(nerrors).arg(nwarns));
            if (answer == KMessageBox::Cancel)
              ok = false;

            if (answer == KMessageBox::Yes)
            {
              // is there a better way to do this?!?!

              QString errstring = "<p><b>Errors<b>" + plugin->errors().join("</p><p>") + "</p><b>Warnings</b>" + plugin->warnings().join("</p><p>") + "</p>";

              QDialog dlg;
              QVBoxLayout layout( &dlg, 11, 6, "Warnings Layout");
              KTextBrowser te(&dlg,"Warnings");
              layout.addWidget(&te);
              te.setReadOnly(true);
              te.setTextFormat(Qt::RichText);
              te.setText(errstring);
              dlg.setCaption(i18n("Imported Data Warnings"));
              unsigned width = QApplication::desktop()->width();
              unsigned height = QApplication::desktop()->height();
              te.setMinimumSize(width/2,height/2);
              layout.setResizeMode(QLayout::Minimum);
              dlg.exec();
            }
          }

          if ( ok )
#endif
          slotStatementImport(statements);
        }
        else
        {
          KMessageBox::error( this, i18n("Unable to import %1 using %2 plugin.  The plugin returned the following error: %3").arg(dialog->selectedURL().prettyURL(),format,plugin->lastError()), i18n("Importing error"));
        }
      }
      else
      {
          KMessageBox::error( this, i18n("Unable to import %1 using %2 plugin.  This file is not the correct format.").arg(dialog->selectedURL().prettyURL(),format), i18n("Incorrect format"));
      }
    }
    slotStatusMsg(prevMsg);
  }
  else
  {
    KMessageBox::error( this, i18n("Unable to import <b>%1</b> file.  There is no such plugin loaded.").arg(format), i18n("Function not available"));
  }
}

void KMyMoney2App::slotAccountUpdateOFX(void)
{
  // TODO: This would be a good place to support other protocols and tie into
  // a plugin.  The "protocol" setting could be used to choose the correct
  // plugin.  We'd pass the online banking MMKVPC to the plugin, and expect
  // it to provide a statement which we could then pass to the importer
  // plugin.  Or better yet it would just provide a MMStatement.

#ifdef USE_OFX_DIRECTCONNECT
  bool accountSuccess=false;
  try
  {
    MyMoneyFile* file = MyMoneyFile::instance();
    MyMoneyAccount account;

    if(!m_selectedAccount.id().isEmpty())
    {
      KOfxDirectConnectDlg dlg(m_selectedAccount);

      connect(&dlg, SIGNAL(statementReady(const QString&, const QString&)), this,
        SLOT(slotPluginImport(const QString&, const QString&)));

      dlg.init();
      dlg.exec();
    }
  }
  catch (MyMoneyException *e)
  {
    KMessageBox::information(this,i18n("Error connecting to bank: %1").arg(e->what()));
    delete e;
  }
#endif
}

void KMyMoney2App::slotPluginImport(const QString& format, const QString& url)
{
  kdDebug() << __PRETTY_FUNCTION__ << ": Activated '" << format << "'plugin." << endl;

  if ( m_importerPlugins.contains(format) )
  {
    KMyMoneyPlugin::ImporterPlugin* plugin = m_importerPlugins[format];

    QString prevMsg = slotStatusMsg(i18n("Importing a statement using %1 plugin").arg(format));

    if ( plugin->isMyFormat(url) )
    {
      QValueList<MyMoneyStatement> statements;
      if ( plugin->import(url,statements) )
      {
        slotStatementImport(statements);
      }
      else
      {
        KMessageBox::error( this, i18n("Unable to import %1 using %2 plugin.  The plugin returned the following error: %3").arg(url,format,plugin->lastError()), i18n("Importing error"));
      }
    }
    else
    {
        KMessageBox::error( this, i18n("Unable to import %1 using %2 plugin.  This file is not the correct format.").arg(url,format), i18n("Incorrect format"));
    }
    slotStatusMsg(prevMsg);
  }
  else
  {
    KMessageBox::error( this, i18n("Unable to import <b>%1</b> file.  There is no such plugin loaded.").arg(format), i18n("Function not available"));
  }
}

//
// KMyMoney2App::slotStatementImport() is for testing only.  The MyMoneyStatement
// is not intended to be exposed to users in XML form.
//

void KMyMoney2App::slotStatementImport()
{
  bool result = false;
  QString prevMsg = slotStatusMsg(i18n("Importing an XML Statement."));

  KFileDialog* dialog = new KFileDialog(KGlobalSettings::documentPath(),
                            i18n("%1|XML files\n%2|All files (*.*)").arg("*.xml").arg("*.*"),
                            this, i18n("Import XML Statement..."), true);
  dialog->setMode(KFile::File | KFile::ExistingOnly);

  if(dialog->exec() == QDialog::Accepted)
  {
    result = slotStatementImport(dialog->selectedURL().path());

/*    QFile f( dialog->selectedURL().path() );
    f.open( IO_ReadOnly );
    QString error = "Unable to parse file";
    QDomDocument* doc = new QDomDocument;
    if(doc->setContent(&f, FALSE))
    {
      if ( doc->doctype().name() == "KMYMONEY-STATEMENT" )
      {
        QDomElement rootElement = doc->documentElement();
        if(!rootElement.isNull())
        {
          QDomNode child = rootElement.firstChild();
          if(!child.isNull() && child.isElement())
          {
            MyMoneyStatement s;
            if ( s.read(child.toElement()) )
              result = slotStatementImport(s);
            else
              error = "File does not contain any statements";
          }
        }
      }
      else
        error = "File is not a KMyMoney Statement";
    }
    delete doc;

    if ( !result )
    {
      QMessageBox::critical( this, i18n("Critical Error"), i18n("Unable to read file %1: %2").arg( dialog->selectedURL().path()).arg(error), QMessageBox::Ok, 0 );

      slotStatusMsg(prevMsg);
    }*/
  }
  delete dialog;

  if ( !result )
  {
    // re-enable all standard widgets
    setEnabled(true);
    slotStatusMsg(prevMsg);
  }
}

bool KMyMoney2App::slotStatementImport(const QString& url)
{
  bool result = false;
  MyMoneyStatement s;
  if ( MyMoneyStatement::readXMLFile( s, url ) )
    result = slotStatementImport(s);
  else
    QMessageBox::critical( this, i18n("Invalid Statement"), i18n("Error importing %1: This file is not a valid KMM statement file.").arg(url), QMessageBox::Ok, 0 );

  return result;
}

bool KMyMoney2App::slotStatementImport(const MyMoneyStatement& s)
{
  bool result = false;

  m_smtReader = new MyMoneyStatementReader;
  connect(m_smtReader, SIGNAL(importFinished()), this, SLOT(slotStatementImportFinished()));

  myMoneyView->suspendUpdate(true);

  // construct a copy of the current engine
  if(m_engineBackup)
    delete m_engineBackup;
  m_engineBackup = MyMoneyFile::instance()->storage()->duplicate();

  m_smtReader->setAutoCreatePayee(true);
  m_smtReader->setProgressCallback(&progressCallback);

  // disable all standard widgets during the import
  setEnabled(false);

  result = m_smtReader->startImport(s);

  return result;
}

bool KMyMoney2App::slotStatementImport(const QValueList<MyMoneyStatement>& statements)
{
  bool hasstatements = (statements.count() > 0);
  bool ok = true;
  bool abort = false;

  // FIXME Deal with warnings/errors coming back from plugins
  /*if ( ofx.errors().count() )
  {
    if ( KMessageBox::warningContinueCancelList(this,i18n("The following errors were returned from your bank"),ofx.errors(),i18n("OFX Errors")) == KMessageBox::Cancel )
      abort = true;
  }

  if ( ofx.warnings().count() )
  {
    if ( KMessageBox::warningContinueCancelList(this,i18n("The following warnings were returned from your bank"),ofx.warnings(),i18n("OFX Warnings"),KStdGuiItem::cont(),"ofxwarnings") == KMessageBox::Cancel )
      abort = true;
  }*/

  QValueList<MyMoneyStatement>::const_iterator it_s = statements.begin();
  while ( it_s != statements.end() && !abort )
  {
    ok = ok && slotStatementImport(*it_s);
    ++it_s;
  }

  if ( hasstatements && !ok )
  {
    KMessageBox::error( this, i18n("Importing process terminated unexpectedly."), i18n("Failed to import all statements."));
  }

  return ( !hasstatements || ok );
}

void KMyMoney2App::slotStatementImportFinished(void)
{
  MyMoneyFile* file = MyMoneyFile::instance();

  myMoneyView->suspendUpdate(false);
  if(m_smtReader != 0) {
    if(m_smtReader->finishImport()) {
      if(verifyImportedData(m_smtReader->account())) {
        // keep the new data set, destroy the backup copy
        delete m_engineBackup;
        m_engineBackup = 0;
      }
    }

    if(m_engineBackup != 0) {
      // user cancelled, destroy the updated set and keep the backup copy
      IMyMoneyStorage* data = file->storage();


      if(data != 0) {
        file->detachStorage(data);
        delete data;
      }
      file->attachStorage(m_engineBackup);
      m_engineBackup = 0;
    }

    // update the views as they might still contain invalid data
    // from the import session. The same applies for the window caption
    myMoneyView->slotRefreshViews();
    updateCaption();

    // slotStatusMsg(prevMsg);
    delete m_smtReader;
    m_smtReader = 0;
  }
  slotStatusProgressBar(-1, -1);
  ready();

  // re-enable all standard widgets
  setEnabled(true);
}

void KMyMoney2App::slotQifExport()
{
  QString prevMsg = slotStatusMsg(i18n("Exporting file..."));

  KExportDlg* dlg = new KExportDlg(0);

  if(dlg->exec()) {

    MyMoneyQifWriter writer;
    connect(&writer, SIGNAL(signalProgress(const int, const int)), this, SLOT(slotStatusProgressBar(const int, const int)));

    writer.write(dlg->filename(), dlg->profile(), dlg->accountId(),
           dlg->accountSelected(), dlg->categorySelected(),
           dlg->startDate(), dlg->endDate());
  }
  delete dlg;

  slotStatusMsg(prevMsg);
}

void KMyMoney2App::slotSettings()
{
  // if we already have an instance of the settings dialog, then use it
  if(KConfigDialog::showDialog("KMyMoney-Settings"))
    return;

  // otherwise, we have to create it
  KConfigDialog* dlg = new KConfigDialog(this, "KMyMoney-Settings", KMyMoneySettings::self(),
    KDialogBase::IconList, KDialogBase::Default | KDialogBase::Ok | KDialogBase::Cancel | KDialogBase::Help);

  // create the pages ...
  KSettingsGeneral* generalPage = new KSettingsGeneral();
  KSettingsRegister* registerPage = new KSettingsRegister();
  KSettingsHome* homePage = new KSettingsHome();
  KSettingsSchedules* schedulesPage = new KSettingsSchedules();
  KSettingsGpg* encryptionPage = new KSettingsGpg();
  KSettingsColors* colorsPage = new KSettingsColors();
  KSettingsFonts* fontsPage = new KSettingsFonts();
  KSettingsOnlineQuotes* onlineQuotesPage = new KSettingsOnlineQuotes();

  // ... and add them to the dialog
  dlg->addPage(generalPage, i18n("General"), "misc");
  dlg->addPage(registerPage, i18n("Register"), "ledger");
  dlg->addPage(homePage, i18n("Home"), "home");
  dlg->addPage(schedulesPage, i18n("Schedules"), "schedule");
  dlg->addPage(encryptionPage, i18n("Encryption"), "kgpg");
  dlg->addPage(colorsPage, i18n("Colors"), "colorscm");
  dlg->addPage(fontsPage, i18n("Fonts"), "font");
  dlg->addPage(onlineQuotesPage, i18n("Online Quotes"), "network_local");

  connect(dlg, SIGNAL(settingsChanged()), this, SLOT(slotUpdateConfiguration()));

  dlg->show();
}

void KMyMoney2App::slotUpdateConfiguration(void)
{
  myMoneyView->slotRefreshViews();

  // re-read autosave configuration
  m_autoSaveEnabled = KMyMoneySettings::autoSaveFile();
  m_autoSavePeriod = KMyMoneySettings::autoSavePeriod();

  // stop timer if turned off but running
  if(m_autoSaveTimer->isActive() && !m_autoSaveEnabled) {
    m_autoSaveTimer->stop();
  }
  // start timer if turned on and needed but not running
  if(!m_autoSaveTimer->isActive() && m_autoSaveEnabled && myMoneyView->dirty()) {
    m_autoSaveTimer->start(m_autoSavePeriod * 60 * 1000, true);
  }
}

/** Init wizard dialog */
bool KMyMoney2App::initWizard()
{
  KStartDlg start;
  if (start.exec()) {
    slotFileClose();
    if (start.isNewFile()) {
      slotFileNew();
    } else if (start.isOpenFile()) {
      KURL url;
      url = start.getURL();

      m_fileName = url.url();
      slotFileOpenRecent(url);
    } else { // Wizard / Template
      m_fileName = start.getURL();
    }

    //save off directory as the last one used.
    if(m_fileName.isLocalFile() && m_fileName.hasPath())
    {
      writeLastUsedDir(m_fileName.path(0));
      writeLastUsedFile(m_fileName.path(0));
    }

    return true;

  } else {
    // cancel clicked so post an exit call
    return false;
  }
}

/** No descriptions */
void KMyMoney2App::slotFileBackup()
{
  // Save the file first so isLocalFile() works
  if (myMoneyView && myMoneyView->dirty())

  {
    if (KMessageBox::questionYesNo(this, i18n("The file must be saved first "
        "before it can be backed up.  Do you want to continue?")) == KMessageBox::No)
    {
      return;

    }

    slotFileSave();
  }



  if ( m_fileName.isEmpty() )
      return;

  if(!m_fileName.isLocalFile()) {
    KMessageBox::sorry(this,
                       i18n("The current implementation of the backup functionality only supports local files as source files! Your current source file is '%1'.")
                            .arg(m_fileName.url()),

                       i18n("Local files only"));
    return;
  }

  KBackupDlg *backupDlg = new KBackupDlg(this,0/*,true*/);
  int returncode = backupDlg->exec();
  if(returncode)
  {

    m_backupMount = backupDlg->mountCheckBox->isChecked();
    proc.clearArguments();
    m_backupState = BACKUP_MOUNTING;
    m_mountpoint = backupDlg->txtMountPoint->text();

    if (m_backupMount) {
      progressCallback(0, 300, i18n("Mounting %1").arg(m_mountpoint));
      proc << "mount";
      proc << m_mountpoint;
      proc.start();

    } else {
      // If we don't have to mount a device, we just issue
      // a dummy command to start the copy operation
      progressCallback(0, 300, "");
      proc << "echo";
      proc.start();
    }

  }

  delete backupDlg;
}


/** No descriptions */
void KMyMoney2App::slotProcessExited()
{
  switch(m_backupState) {
    case BACKUP_MOUNTING:

      if(proc.normalExit() && proc.exitStatus() == 0) {
        proc.clearArguments();
        QString today;
        today.sprintf("-%04d-%02d-%02d.kmy",
          QDate::currentDate().year(),
          QDate::currentDate().month(),
          QDate::currentDate().day());
        QString backupfile = m_mountpoint + "/" + m_fileName.fileName(false);
        KMyMoneyUtils::appendCorrectFileExt(backupfile, today);

        // check if file already exists and ask what to do
        m_backupResult = 0;
        QFile f(backupfile);
        if (f.exists()) {
          int answer = KMessageBox::warningContinueCancel(this, i18n("Backup file for today exists on that device.  Replace ?"), i18n("Backup"), i18n("&Replace"));
          if (answer == KMessageBox::Cancel) {
            m_backupResult = 1;

            if (m_backupMount) {
              progressCallback(250, 0, i18n("Unmounting %1").arg(m_mountpoint));
              proc.clearArguments();
              proc << "umount";
              proc << m_mountpoint;
              m_backupState = BACKUP_UNMOUNTING;
              proc.start();
            } else {
              m_backupState = BACKUP_IDLE;
              progressCallback(-1, -1, QString());
              ready();
            }
          }
        }

        if(m_backupResult == 0) {
          progressCallback(50, 0, i18n("Writing %1").arg(backupfile));
          proc << "cp" << "-f" << m_fileName.path(0) << backupfile;
          m_backupState = BACKUP_COPYING;
          proc.start();
        }

      } else {
        KMessageBox::information(this, i18n("Error mounting device"), i18n("Backup"));
        m_backupResult = 1;
        if (m_backupMount) {
          progressCallback(250, 0, i18n("Unmounting %1").arg(m_mountpoint));
          proc.clearArguments();
          proc << "umount";
          proc << m_mountpoint;
          m_backupState = BACKUP_UNMOUNTING;
          proc.start();

        } else {
          m_backupState = BACKUP_IDLE;
          progressCallback(-1, -1, QString());
          ready();
        }
      }
      break;

    case BACKUP_COPYING:
      if(proc.normalExit() && proc.exitStatus() == 0) {

        if (m_backupMount) {
          progressCallback(250, 0, i18n("Unmounting %1").arg(m_mountpoint));
          proc.clearArguments();
          proc << "umount";
          proc << m_mountpoint;
          m_backupState = BACKUP_UNMOUNTING;
          proc.start();
        } else {
          progressCallback(300, 0, i18n("Done"));
          KMessageBox::information(this, i18n("File successfully backed up"), i18n("Backup"));
          m_backupState = BACKUP_IDLE;
          progressCallback(-1, -1, QString());
          ready();
        }
      } else {
        qDebug("cp exit status is %d", proc.exitStatus());
        m_backupResult = 1;
        KMessageBox::information(this, i18n("Error copying file to device"), i18n("Backup"));

        if (m_backupMount) {
          progressCallback(250, 0, i18n("Unmounting %1").arg(m_mountpoint));
          proc.clearArguments();
          proc << "umount";
          proc << m_mountpoint;
          m_backupState = BACKUP_UNMOUNTING;
          proc.start();


        } else {
          m_backupState = BACKUP_IDLE;
          progressCallback(-1, -1, QString());
          ready();
        }
      }
      break;


    case BACKUP_UNMOUNTING:
      if(proc.normalExit() && proc.exitStatus() == 0) {

        progressCallback(300, 0, i18n("Done"));
        if(m_backupResult == 0)
          KMessageBox::information(this, i18n("File successfully backed up"), i18n("Backup"));
      } else {
        KMessageBox::information(this, i18n("Error unmounting device"), i18n("Backup"));
      }
      m_backupState = BACKUP_IDLE;
      progressCallback(-1, -1, QString());
      ready();
      break;

    default:
      qWarning("Unknown state for backup operation!");
      progressCallback(-1, -1, QString());
      ready();
      break;
  }
}

void KMyMoney2App::slotFileNewWindow()
{
  KMyMoney2App *newWin = new KMyMoney2App;

  newWin->show();
}

void KMyMoney2App::slotEditToolbars()
{
    saveMainWindowSettings( KGlobal::config(), "main_window_settings" );
    KEditToolbar dlg( factory(),this );
    connect( &dlg, SIGNAL( newToolbarConfig() ), SLOT( slotNewToolBarConfig() ) );
    dlg.exec();
}

void KMyMoney2App::slotNewToolBarConfig() {
  applyMainWindowSettings( KGlobal::config(), "main_window_settings" );
}

void KMyMoney2App::slotKeySettings()
{

#if KDE_IS_VERSION(3,2,0)
  KKeyDialog::configure( actionCollection() );
#else
  QString path = KGlobal::dirs()->findResource("appdata", "kmymoney2ui.rc");
  KKeyDialog::configureKeys(actionCollection(), path);
#endif
}

#if 0
void KMyMoney2App::slotSetViewSpecificActions(int view)
{
  action("file_print")->setEnabled(false);
  switch(view) {
    case KMyMoneyView::ReportsView:
      action("file_print")->setEnabled(true);
      break;
    default:
      break;
  }
}
#endif

void KMyMoney2App::slotShowTipOfTheDay(void)
{
  KTipDialog::showTip(myMoneyView, "", true);
}

void KMyMoney2App::slotShowPreviousView(void)
{

}

void KMyMoney2App::slotShowNextView(void)
{

}

void KMyMoney2App::slotQifProfileEditor(void)
{
  MyMoneyQifProfileEditor* editor = new MyMoneyQifProfileEditor(true, this, "QIF Profile Editor");


  editor->exec();

  delete editor;

}

bool KMyMoney2App::verifyImportedData(const MyMoneyAccount& account)
{
  bool rc;
  KImportVerifyDlg *dialog = new KImportVerifyDlg(account, this);
  dialog->setProgressCallback(progressCallback);
  rc = (dialog->exec() == QDialog::Accepted);
  delete dialog;
  return rc;
}

void KMyMoney2App::slotToolsStartKCalc(void)
{
  KRun::runCommand("kcalc");
}

void KMyMoney2App::slotFindTransaction(void)
{
  if(m_searchDlg == 0) {
    m_searchDlg = new KFindTransactionDlg();
    connect(m_searchDlg, SIGNAL(destroyed()), this, SLOT(slotCloseSearchDialog()));
    connect(m_searchDlg, SIGNAL(transactionSelected(const QCString&, const QCString&)),
            myMoneyView, SLOT(slotLedgerSelected(const QCString&, const QCString&)));
  }
  m_searchDlg->show();
  m_searchDlg->raise();
}

void KMyMoney2App::slotCloseSearchDialog(void)
{
  if(m_searchDlg)
    m_searchDlg->deleteLater();
  m_searchDlg = 0;
}

void KMyMoney2App::slotInstitutionNew(MyMoneyInstitution institution)
{
  try {
    MyMoneyFile* file = MyMoneyFile::instance();
    file->addInstitution(institution);

  } catch (MyMoneyException *e) {
    KMessageBox::information(this, i18n("Cannot add institution: %1").arg(e->what()));
    delete e;
  }
}

void KMyMoney2App::slotInstitutionNew(void)
{
  MyMoneyInstitution institution;

  KNewBankDlg dlg(institution);
  if (dlg.exec())
    slotInstitutionNew(dlg.institution());
}

void KMyMoney2App::slotInstitutionEdit(const MyMoneyObject& obj)
{
  if(typeid(obj) != typeid(MyMoneyInstitution))
    return;

  try {
    MyMoneyFile* file = MyMoneyFile::instance();

    //grab a pointer to the view, regardless of it being a account or institution view.
    MyMoneyInstitution institution = file->institution(m_selectedInstitution.id());

    // bankSuccess is not checked anymore because m_file->institution will throw anyway
    KNewBankDlg dlg(institution);
    if (dlg.exec()) {
      file->modifyInstitution(dlg.institution());
      slotSelectInstitution(dlg.institution());
    }

  } catch(MyMoneyException *e) {
    if(!obj.id().isEmpty())
      KMessageBox::information(this, i18n("Unable to edit institution: %1").arg(e->what()));
    delete e;
  }
}

void KMyMoney2App::slotInstitutionDelete(void)
{
  try {
    MyMoneyFile *file = MyMoneyFile::instance();

    MyMoneyInstitution institution = file->institution(m_selectedInstitution.id());
    if ((KMessageBox::questionYesNo(this, QString("<p>")+i18n("Do you really want to delete institution <b>%1</b> ?").arg(institution.name()))) == KMessageBox::No)
      return;
    file->removeInstitution(institution);

  } catch (MyMoneyException *e) {
    KMessageBox::information(this, i18n("Unable to delete institution: %1").arg(e->what()));
    delete e;
  }
}

void KMyMoney2App::createAccount(MyMoneyAccount& newAccount, MyMoneyAccount& parentAccount, MyMoneyAccount& brokerageAccount, MyMoneyMoney openingBal, MyMoneySchedule& schedule)
{

  try
  {
    int pos;
    // check for ':' in the name and use it as separator for a hierarchy
    while((pos = newAccount.name().find(':')) != -1) {
      QString part = newAccount.name().left(pos);
      QString remainder = newAccount.name().mid(pos+1);
      newAccount.setName(part);

      MyMoneyFile::instance()->addAccount(newAccount, parentAccount);
      parentAccount = newAccount;
      newAccount.setParentAccountId(QCString());  // make sure, there's no parent
      newAccount.clearId();                       // and no id set for adding
      newAccount.setName(remainder);
    }

    // Check the opening balance
    if (openingBal.isPositive() && newAccount.accountGroup() == MyMoneyAccount::Liability)
    {
      openingBal = -openingBal;
      QString message = i18n("This account is a liability and if the "
          "opening balance represents money owed, then it should be negative.  "
          "Negate the amount?\n\n"
          "Please click Yes to change the opening balance to %1,\n"
          "Please click No to leave the amount as %2,\n"
          "Please click Cancel to abort the account creation.")
          .arg(openingBal.formatMoney())
          .arg((-openingBal).formatMoney());

      int ans = KMessageBox::questionYesNoCancel(this, message);
      if (ans == KMessageBox::Yes)
      {
        openingBal = -openingBal;
      }
      else if (ans == KMessageBox::Cancel)
        return;
    }

    MyMoneyFile::instance()->addAccount(newAccount, parentAccount);

    // We MUST add the schedule AFTER adding the account because
    // otherwise an unknown account exception will be thrown.
    createSchedule(schedule, newAccount);

    // in case of a loan account, we add the initial payment
    if((newAccount.accountType() == MyMoneyAccount::Loan
    || newAccount.accountType() == MyMoneyAccount::AssetLoan)
    && !newAccount.value("kmm-loan-payment-acc").isEmpty()
    && !newAccount.value("kmm-loan-payment-date").isEmpty()) {
      MyMoneyAccountLoan acc(newAccount);
      MyMoneyTransaction t;
      MyMoneySplit a, b;
      a.setAccountId(acc.id());
      b.setAccountId(acc.value("kmm-loan-payment-acc").latin1());
      a.setValue(acc.loanAmount());
      if(acc.accountType() == MyMoneyAccount::Loan)
        a.setValue(-a.value());
      b.setValue(-a.value());
      a.setMemo(i18n("Loan payout"));
      b.setMemo(i18n("Loan payout"));
      t.setPostDate(QDate::fromString(acc.value("kmm-loan-payment-date"), Qt::ISODate));
      try {
        newAccount.deletePair("kmm-loan-payment-acc");
        newAccount.deletePair("kmm-loan-payment-date");
        MyMoneyFile::instance()->modifyAccount(newAccount);

        t.addSplit(a);
        t.addSplit(b);
        MyMoneyFile::instance()->addTransaction(t);
      } catch(MyMoneyException *e) {
        qDebug("Cannot add loan payout transaction: %s", e->what().latin1());
        delete e;
      }
      MyMoneyFile::instance()->createOpeningBalanceTransaction(newAccount, openingBal);

    // in case of an investment account we check if we should create
    // a brokerage account
    } else if(newAccount.accountType() == MyMoneyAccount::Investment
            && !brokerageAccount.name().isEmpty()) {
      try {
        MyMoneyFile::instance()->addAccount(brokerageAccount, parentAccount);

        // set a link from the investment account to the brokerage account
        newAccount.setValue("kmm-brokerage-account", brokerageAccount.id());
        MyMoneyFile::instance()->modifyAccount(newAccount);
        MyMoneyFile::instance()->createOpeningBalanceTransaction(brokerageAccount, openingBal);
      } catch(MyMoneyException *e) {
        qDebug("Cannot add brokerage account: %s", e->what().latin1());
        delete e;
      }
    } else
      MyMoneyFile::instance()->createOpeningBalanceTransaction(newAccount, openingBal);

  }
  catch (MyMoneyException *e)
  {
    QString message("Unable to add account: ");
    message += e->what();
    KMessageBox::information(this, message);
    delete e;
  }
}

void KMyMoney2App::slotCategoryNew(MyMoneyAccount& account, const MyMoneyAccount& parent)
{
  if(!parent.id().isEmpty()) {
    try {
      // make sure parent account exists
      MyMoneyFile::instance()->account(parent.id());
      account.setParentAccountId(parent.id());
      account.setAccountType( parent.accountType() );
    } catch(MyMoneyException *e) {
      delete e;
    }
  }

  KNewAccountDlg dialog(account, false, true, 0, 0, i18n("Create a new Category"));

  if(dialog.exec() == QDialog::Accepted) {
    MyMoneyAccount parentAccount, brokerageAccount;
    account = dialog.account();
    parentAccount = dialog.parentAccount();
    MyMoneySchedule schedule;

    createAccount(account, parentAccount, brokerageAccount, MyMoneyMoney(0,1), schedule);
  }
}

void KMyMoney2App::slotCategoryNew(void)
{
  MyMoneyAccount parent;
  MyMoneyAccount account;

  // Preselect the parent account by looking at the current selected account/category
  if(!m_selectedAccount.id().isEmpty()
  && (m_selectedAccount.accountGroup() == MyMoneyAccount::Expense
    || m_selectedAccount.accountGroup() == MyMoneyAccount::Income)) {
    try {
      MyMoneyFile* file = MyMoneyFile::instance();
      parent = file->account(m_selectedAccount.id());
    } catch(MyMoneyException *e) {
      delete e;
    }
  }

  slotCategoryNew(account, parent);
}

void KMyMoney2App::slotAccountNew(void)
{
  KNewAccountWizard newAccountWizard;

  connect(&newAccountWizard, SIGNAL(newInstitutionClicked()), this, SLOT(slotInstitutionNew()));
  connect(&newAccountWizard, SIGNAL(newCategory(MyMoneyAccount&)), this, SLOT(slotCategoryNew(MyMoneyAccount&)));

  newAccountWizard.setAccountName(QString());
  // Preselect the current selected institution (or none)
  newAccountWizard.setInstitution(m_selectedInstitution);

  if(newAccountWizard.exec() == QDialog::Accepted) {
    // The wizard doesn't check the parent.
    // An exception will be thrown on the next line instead.
    MyMoneyAccount newAccount, parentAccount, brokerageAccount;
    newAccount = newAccountWizard.account();
    parentAccount = newAccountWizard.parentAccount();
    brokerageAccount = newAccountWizard.brokerageAccount();
    MyMoneySchedule schedule = newAccountWizard.schedule();
    createAccount(newAccount, parentAccount, brokerageAccount, newAccountWizard.openingBalance(), schedule);
  }
}

void KMyMoney2App::slotInvestmentNew(void)
{
  KNewInvestmentWizard dlg;
  if(dlg.exec() == QDialog::Accepted) {
    dlg.createObjects(m_selectedAccount.id());
  }
}

void KMyMoney2App::slotInvestmentEdit(void)
{
  KNewInvestmentWizard dlg(m_selectedInvestment);
  if(dlg.exec() == QDialog::Accepted) {
    dlg.createObjects(m_selectedAccount.id());
  }
}

void KMyMoney2App::slotInvestmentDelete(void)
{
  if(KMessageBox::questionYesNo(this, QString("<p>")+i18n("Do you really want to delete the investment <b>%1</b>?").arg(m_selectedInvestment.name()), i18n("Delete investment"), KStdGuiItem::yes(), KStdGuiItem::no(), "DeleteInvestment") == KMessageBox::Yes) {
    try {
      MyMoneyFile::instance()->removeAccount(m_selectedInvestment);
    } catch(MyMoneyException *e) {
      qDebug("Cannot delete investment: %s", e->what().latin1());
      delete e;
    }
  }
}

void KMyMoney2App::slotOnlinePriceUpdate(void)
{
  if(!m_selectedInvestment.id().isEmpty()) {
    KEquityPriceUpdateDlg dlg(0, m_selectedInvestment.currencyId());
    dlg.exec();
  }
}

void KMyMoney2App::slotManualPriceUpdate(void)
{
  if(!m_selectedInvestment.id().isEmpty()) {
    try {
      MyMoneySecurity security = MyMoneyFile::instance()->security(m_selectedInvestment.currencyId());
      MyMoneySecurity currency = MyMoneyFile::instance()->security(security.tradingCurrency());
      MyMoneyPrice price = MyMoneyFile::instance()->price(security.id(), currency.id());
      signed64 fract = MyMoneyMoney::precToDenom(KMyMoneySettings::pricePrecision());

      KCurrencyCalculator calc(security, currency, MyMoneyMoney(1,1), price.rate(), price.date(), fract);
      calc.setupPriceEditor();

      // The dialog takes care of adding the price if necessary
      calc.exec();
    } catch(MyMoneyException* e) {
      qDebug("Error in price update: %s", e->what().data());
      delete e;
    }
  }
}

void KMyMoney2App::createSchedule(MyMoneySchedule newSchedule, MyMoneyAccount& newAccount)
{
  // Add the schedule only if one exists
  //
  // Remember to modify the first split to reference the newly created account
  if (!newSchedule.name().isEmpty())
  {
    try
    {
      // We assume at least 2 splits in the transaction
      MyMoneyTransaction t = newSchedule.transaction();
      if(t.splitCount() < 2) {
        throw new MYMONEYEXCEPTION("Transaction for schedule has less than 2 splits!");
      }
      // now search the split that does not have an account reference
      // and set it up to be the one of the account we just added
      // to the account pool. Note: the schedule code used to leave
      // this always the first split, but the loan code leaves it as
      // the second one. So I thought, searching is a good alternative ....
      QValueList<MyMoneySplit>::ConstIterator it_s;
      for(it_s = t.splits().begin(); it_s != t.splits().end(); ++it_s) {
        if((*it_s).accountId().isEmpty()) {
          MyMoneySplit s = (*it_s);
          s.setAccountId(newAccount.id());
          t.modifySplit(s);
          break;
        }
      }
      newSchedule.setTransaction(t);

      MyMoneyFile::instance()->addSchedule(newSchedule);

      // in case of a loan account, we keep a reference to this
      // schedule in the account
      if(newAccount.accountType() == MyMoneyAccount::Loan
      || newAccount.accountType() == MyMoneyAccount::AssetLoan) {
        newAccount.setValue("schedule", newSchedule.id());
        MyMoneyFile::instance()->modifyAccount(newAccount);
      }
    }
    catch (MyMoneyException *e)
    {
      KMessageBox::information(this, i18n("Unable to add schedule: "), e->what());
      delete e;
    }
  }
}

void KMyMoney2App::slotAccountDelete(void)
{
  bool canDelete = false;
  MyMoneyFile* file = MyMoneyFile::instance();
  if(!m_selectedAccount.id().isEmpty()) {
    if(!file->isStandardAccount(m_selectedAccount.id())) {
      switch(m_selectedAccount.accountGroup()) {
        case MyMoneyAccount::Asset:
          if(m_selectedAccount.accountType() == MyMoneyAccount::Investment) {
            canDelete = (file->transactionCount(m_selectedAccount.id())==0) && (m_selectedAccount.accountList().count() == 0);
          } else {
            canDelete = file->transactionCount(m_selectedAccount.id())==0;
          }
          break;

        default:
          canDelete = file->transactionCount(m_selectedAccount.id())==0;
          break;
      }
    }
  }
  if(canDelete) {
    if (KMessageBox::questionYesNo(this, QString("<p>")+i18n("Do you really want to delete account <b>%1</b>?").arg(m_selectedAccount.name())) == KMessageBox::Yes) {
      try {
        file->removeAccount(m_selectedAccount);
      } catch(MyMoneyException* e) {
        KMessageBox::error( this, i18n("Unable to delete account '%1'. Cause: %2").arg(m_selectedAccount.name()).arg(e->what()));
        delete e;
      }
    }
  }
}

void KMyMoney2App::slotAccountEdit(void)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  if(!m_selectedAccount.id().isEmpty()) {
    if(!file->isStandardAccount(m_selectedAccount.id())) {
      QString caption;
      bool category = false;
      switch(MyMoneyAccount::accountGroup(m_selectedAccount.accountType())) {
        default:
          caption = i18n("Edit an account");
          break;

        case MyMoneyAccount::Expense:
        case MyMoneyAccount::Income:
          caption = i18n("Edit a category");
          category = true;
          break;
      }
      QCString tid = file->openingBalanceTransaction(m_selectedAccount);
      MyMoneyTransaction t;
      MyMoneySplit s0, s1;
      KNewAccountDlg dlg(m_selectedAccount, true, category, 0, 0, caption);

      if(category || m_selectedAccount.accountType() == MyMoneyAccount::Investment) {
        dlg.setOpeningBalanceShown(false);
      } else {
        if(!tid.isEmpty()) {
          try {
            t = file->transaction(tid);
            s0 = t.splitByAccount(m_selectedAccount.id());
            s1 = t.splitByAccount(m_selectedAccount.id(), false);
            dlg.setOpeningBalance(s0.shares());
            if(m_selectedAccount.accountGroup() == MyMoneyAccount::Liability) {
              dlg.setOpeningBalance(-s0.shares());
            }
          } catch(MyMoneyException *e) {
            kdDebug(2) << "Error retrieving opening balance transaction " << tid << ": " << e->what() << "\n";
            tid = QCString();
            delete e;
          }
        }
      }

      if (dlg.exec()) {
        try {
          MyMoneyAccount account = dlg.account();
          MyMoneyAccount parent = dlg.parentAccount();
          MyMoneyMoney bal = dlg.openingBalance();
          if(m_selectedAccount.accountGroup() == MyMoneyAccount::Liability) {
            bal = -bal;
          }

          // we need to reparent first, as modify will check for same type
          if(account.parentAccountId() != parent.id()) {
            file->reparentAccount(account, parent);
          }
          file->modifyAccount(account);
          if(!tid.isEmpty() && dlg.openingBalance().isZero()) {
            file->removeTransaction(t);

          } else if(!tid.isEmpty() && !dlg.openingBalance().isZero()) {
            s0.setShares(bal);
            s0.setValue(bal);
            t.modifySplit(s0);
            s1.setShares(-bal);
            s1.setValue(-bal);
            t.modifySplit(s1);
            file->modifyTransaction(t);

          } else if(tid.isEmpty() && !dlg.openingBalance().isZero()){
            MyMoneyFile::instance()->createOpeningBalanceTransaction(m_selectedAccount, bal);
          }

          slotSelectAccount(account);

        } catch(MyMoneyException* e) {
          KMessageBox::error( this, i18n("Unable to modify account '%1'. Cause: %2").arg(m_selectedAccount.name()).arg(e->what()));
          delete e;
        }
      }
    }
  }
}

void KMyMoney2App::slotAccountReconcile(void)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  MyMoneyAccount account;

  // we cannot reconcile standard accounts
  if(!file->isStandardAccount(m_selectedAccount.id())) {
    // check if we can reconcile this account
    // it make's sense for asset and liability accounts
    try {
      account = file->account(m_selectedAccount.id());
      switch(file->accountGroup(account.accountType())) {
        case MyMoneyAccount::Asset:
        case MyMoneyAccount::Liability:
          myMoneyView->slotAccountReconcile(account);
          break;

        default:
          break;
      }
    } catch(MyMoneyException *e) {
      delete e;
    }
  }
}

void KMyMoney2App::slotAccountOpen(const MyMoneyObject& obj)
{
  if(typeid(obj) != typeid(MyMoneyAccount))
    return;

  MyMoneyFile* file = MyMoneyFile::instance();
  MyMoneyAccount account;

  // we cannot reconcile standard accounts
  if(!file->isStandardAccount(m_selectedAccount.id())) {
    // check if we can open this account
    // currently it make's sense for asset and liability accounts
    try {
      account = file->account(m_selectedAccount.id());
      myMoneyView->slotLedgerSelected(account.id());
    } catch(MyMoneyException *e) {
      delete e;
    }
  }
}

void KMyMoney2App::slotReparentAccount(const MyMoneyAccount& _src, const MyMoneyInstitution& _dst)
{
  MyMoneyAccount src(_src);
  src.setInstitutionId(_dst.id());
  try {
    MyMoneyFile::instance()->modifyAccount(src);
  } catch(MyMoneyException* e) {
    KMessageBox::sorry(this, QString("<p>")+i18n("<b>%1</b> cannot be moved to institution <b>%2</b>. Reason: %3").arg(src.name()).arg(_dst.name()).arg(e->what()));
    delete e;
  }
}

void KMyMoney2App::slotReparentAccount(const MyMoneyAccount& _src, const MyMoneyAccount& _dst)
{
  MyMoneyAccount src(_src);
  MyMoneyAccount dst(_dst);
  try {
    MyMoneyFile::instance()->reparentAccount(src, dst);
  } catch(MyMoneyException* e) {
    KMessageBox::sorry(this, QString("<p>")+i18n("<b>%1</b> cannot be moved to <b>%2</b>. Reason: %3").arg(src.name()).arg(dst.name()).arg(e->what()));
    delete e;
  }
}

void KMyMoney2App::scheduleNew(const QCString& scheduleType)
{
  MyMoneySchedule schedule;

  KEditScheduleDialog dlg(scheduleType, schedule, this);
  connect(&dlg, SIGNAL(newCategory(MyMoneyAccount&)), this, SLOT(slotCategoryNew(MyMoneyAccount&)));

  if (dlg.exec() == QDialog::Accepted) {
    schedule = dlg.schedule();
    try {
      MyMoneyFile::instance()->addSchedule(schedule);

    } catch (MyMoneyException *e) {
      KMessageBox::error(this, i18n("Unable to add schedule: "), e->what());
      delete e;
    }
  }
}

void KMyMoney2App::slotScheduleNewBill(void)
{
  scheduleNew(MyMoneySplit::ActionCheck);
}

void KMyMoney2App::slotScheduleNewDeposit(void)
{
  scheduleNew(MyMoneySplit::ActionDeposit);
}

void KMyMoney2App::slotScheduleNewTransfer(void)
{
  scheduleNew(MyMoneySplit::ActionTransfer);
}

void KMyMoney2App::slotScheduleEdit(void)
{
  if (!m_selectedSchedule.id().isEmpty()) {
    try {
      MyMoneySchedule schedule = MyMoneyFile::instance()->schedule(m_selectedSchedule.id());

      const char *action = 0;
      switch (schedule.type()) {
        case MyMoneySchedule::TYPE_BILL:
          action = MyMoneySplit::ActionWithdrawal;
          break;

        case MyMoneySchedule::TYPE_DEPOSIT:
          action = MyMoneySplit::ActionDeposit;
          break;

        case MyMoneySchedule::TYPE_TRANSFER:
          action = MyMoneySplit::ActionTransfer;
          break;

        case MyMoneySchedule::TYPE_LOANPAYMENT:
        case MyMoneySchedule::TYPE_ANY:
          break;
      }

      KEditScheduleDialog* sched_dlg = 0;
      KEditLoanWizard* loan_wiz = 0;

      switch (schedule.type()) {
        case MyMoneySchedule::TYPE_BILL:
        case MyMoneySchedule::TYPE_DEPOSIT:
        case MyMoneySchedule::TYPE_TRANSFER:
          sched_dlg = new KEditScheduleDialog(action, schedule, this);
          connect(sched_dlg, SIGNAL(newCategory(MyMoneyAccount&)), this, SLOT(slotCategoryNew(MyMoneyAccount&)));

          if (sched_dlg->exec() == QDialog::Accepted) {
            MyMoneySchedule sched = sched_dlg->schedule();
            MyMoneyFile::instance()->modifySchedule(sched);
          }
          delete sched_dlg;
          break;

        case MyMoneySchedule::TYPE_LOANPAYMENT:
          loan_wiz = new KEditLoanWizard(schedule.account(2));
          if (loan_wiz->exec() == QDialog::Accepted) {
            MyMoneyFile::instance()->modifySchedule(loan_wiz->schedule());
            MyMoneyFile::instance()->modifyAccount(loan_wiz->account());
          }
          delete loan_wiz;
          break;

        case MyMoneySchedule::TYPE_ANY:
          break;
      }

    } catch (MyMoneyException *e) {
      KMessageBox::detailedSorry(this, i18n("Unable to modify schedule '%1'").arg(m_selectedSchedule.name()), e->what());
      delete e;
    }
  }
}

void KMyMoney2App::slotScheduleDelete(void)
{
  if (!m_selectedSchedule.id().isEmpty()) {
    try {
      MyMoneySchedule sched = MyMoneyFile::instance()->schedule(m_selectedSchedule.id());
      QString msg = QString("<p>")+i18n("Are you sure you want to delete the schedule <b>%1</b>?").arg(m_selectedSchedule.name());
      if(sched.type() == MyMoneySchedule::TYPE_LOANPAYMENT) {
        msg += QString(" ");
        msg += i18n("In case of loan payments it is currently not possible to recreate the schedule.");
      }
      if (KMessageBox::questionYesNo(this, msg) == KMessageBox::No)
        return;

      MyMoneyFile::instance()->removeSchedule(sched);

    } catch (MyMoneyException *e) {
      KMessageBox::detailedSorry(this, i18n("Unable to remove schedule '%1'").arg(m_selectedSchedule.name()), e->what());
      delete e;
    }
  }
}

void KMyMoney2App::slotScheduleEnter(void)
{
  if (!m_selectedSchedule.id().isEmpty()) {
    try {
      MyMoneySchedule schedule = MyMoneyFile::instance()->schedule(m_selectedSchedule.id());

      KEnterScheduleDialog dlg(this, schedule);
      connect(&dlg, SIGNAL(newCategory(MyMoneyAccount&)), this, SLOT(slotCategoryNew(MyMoneyAccount&)));
      dlg.exec();

    } catch (MyMoneyException *e) {
      KMessageBox::detailedSorry(this, i18n("Unable to enter transaction for schedule '%1'").arg(m_selectedSchedule.name()), e->what());
      delete e;
    }
  }
}

void KMyMoney2App::slotPayeeNew(void)
{
  try {
    QString newname = i18n("New Payee");
    // adjust name until a unique name has been created
    int count = 0;
    for(;;) {
      try {
        MyMoneyFile::instance()->payeeByName(newname);
        newname = i18n("New Payee [%1]").arg(++count);
      } catch(MyMoneyException* e) {
        delete e;
        break;
      }
    }

    MyMoneyPayee p;
    p.setName(newname);
    MyMoneyFile::instance()->addPayee(p);
    // the callbacks should have made sure, that the payees view has been
    // updated already. So we search for the id in the list of items
    // and select it.
    emit payeeCreated(p.id());
  } catch (MyMoneyException *e) {
    KMessageBox::detailedSorry(0, i18n("Unable to add payee"),
      (e->what() + " " + i18n("thrown in") + " " + e->file()+ ":%1").arg(e->line()));
    delete e;
  }
}

void KMyMoney2App::slotPayeeDelete(void)
{
  if(m_selectedPayees.isEmpty())
    return; // shouldn't happen

  MyMoneyFile * file = MyMoneyFile::instance();

  // first create list with all non-selected payees
  QValueList<MyMoneyPayee> remainingPayees = file->payeeList();
  QValueList<MyMoneyPayee>::iterator it_p;
  for(it_p = remainingPayees.begin(); it_p != remainingPayees.end(); ) {
    if (std::find(m_selectedPayees.begin(), m_selectedPayees.end(), (*it_p)) != m_selectedPayees.end()) {
      it_p = remainingPayees.erase(it_p);
    } else {
      ++it_p;
    }
  }

  // get confirmation from user
  QString prompt;
  if (m_selectedPayees.size() == 1)
    prompt = QString("<p>")+i18n("Do you really want to remove the payee <b>%1</b>").arg(m_selectedPayees.front().name());
  else
    prompt = i18n("Do you really want to remove all selected payees?");

  if (KMessageBox::questionYesNo(this, prompt, i18n("Remove Payee"))==KMessageBox::No)
    return;

  try {
    // create a transaction filter that contains all payees selected for removal
    MyMoneyTransactionFilter f = MyMoneyTransactionFilter();
    for (QValueList<MyMoneyPayee>::const_iterator it = m_selectedPayees.begin();
         it != m_selectedPayees.end(); ++it) {
      f.addPayee( (*it).id() );
    }
    // request a list of all transactions that still use the payees in question
    QValueList<MyMoneyTransaction> translist = file->transactionList(f);
//     kdDebug() << "[KPayeesView::slotDeletePayee]  " << translist.count() << " transaction still assigned to payees" << endl;

    // now get a list of all schedules that make use of one of the payees
    QValueList<MyMoneySchedule> all_schedules = file->scheduleList();
    QValueList<MyMoneySchedule> used_schedules;
    for (QValueList<MyMoneySchedule>::ConstIterator it = all_schedules.begin();
         it != all_schedules.end(); ++it)
    {
      // loop over all splits in the transaction of the schedule
      for (QValueList<MyMoneySplit>::ConstIterator s_it = (*it).transaction().splits().begin();
           s_it != (*it).transaction().splits().end(); ++s_it)
      {
        // is the payee in the split to be deleted?
        if (std::find(m_selectedPayees.begin(), m_selectedPayees.end(), (*s_it).payeeId()) != m_selectedPayees.end())
          used_schedules.push_back(*it); // remember this schedule
      }
    }
//     kdDebug() << "[KPayeesView::slotDeletePayee]  " << used_schedules.count() << " schedules use one of the selected payees" << endl;

    // finally remove the payees, but don't signal each change
    file->suspendNotify(true);

    // if at least one payee is still referenced, we need to reassign its transactions first
    if (!translist.isEmpty() || !used_schedules.isEmpty()) {
      file->suspendNotify(false);
      // show error message if no payees remain
      if (remainingPayees.isEmpty()) {
        KMessageBox::sorry(this, i18n("At least one transaction/schedule is still referenced by a payee. "
          "Currently you have all payees selected. However, at least one payee must remain so "
          "that the transactions/schedules can be reassigned."));
        return;
      }
      // sort the payee list by payee's name
      QMap<QString, MyMoneyPayee>sortMap;
      QValueList<MyMoneyPayee>::const_iterator it_p;
      for(it_p = remainingPayees.begin(); it_p != remainingPayees.end(); ++it_p)
        sortMap[(*it_p).name()] = *it_p;
      QMap<QString, MyMoneyPayee>::const_iterator it_pm;
      remainingPayees.clear();
      for(it_pm = sortMap.begin(); it_pm != sortMap.end(); ++it_pm)
        remainingPayees << *it_pm;
      sortMap.clear();

      // show transaction reassignment dialog
      KTransactionReassignDlg * dlg = new KTransactionReassignDlg(this);
      int index = dlg->show(remainingPayees);
      delete dlg; // and kill the dialog
      if (index == -1)
        return; // the user aborted the dialog, so let's abort as well

      // remember the id of the selected target payee
      QCString payee_id = remainingPayees[index].id();

      // finally remove the payees, but don't signal each change
      file->suspendNotify(true);

      // TODO : check if we have a report that explicitely uses one of our payees
      //        and issue an appropriate warning
      try {
        QValueList<MyMoneySplit>::iterator s_it;
        // now loop over all transactions and reassign payee
        for (QValueList<MyMoneyTransaction>::iterator it = translist.begin(); it != translist.end(); ++it) {
          // create a copy of the splits list in the transaction
          QValueList<MyMoneySplit> splits = (*it).splits();
          // loop over all splits
          for (s_it = splits.begin(); s_it != splits.end(); ++s_it) {
            // if the split is assigned to one of the selected payees, we need to modify it
            if ( std::find(m_selectedPayees.begin(), m_selectedPayees.end(), (*s_it).payeeId()) != m_selectedPayees.end()) {
              (*s_it).setPayeeId(payee_id); // first modify payee in current split
              // then modify the split in our local copy of the transaction list
              (*it).modifySplit(*s_it); // this does not modify the list object 'splits'!
            }
          } // for - Splits
          file->modifyTransaction(*it);  // modify the transaction in the MyMoney object
        } // for - Transactions

        // now loop over all schedules and reassign payees
        for (QValueList<MyMoneySchedule>::iterator it = used_schedules.begin();
             it != used_schedules.end(); ++it)
        {
          // create copy of transaction in current schedule
          MyMoneyTransaction trans = (*it).transaction();
          // create copy of lists of splits
          QValueList<MyMoneySplit> splits = trans.splits();
          for (s_it = splits.begin(); s_it != splits.end(); ++s_it) {
            if ( std::find(m_selectedPayees.begin(), m_selectedPayees.end(), (*s_it).payeeId()) != m_selectedPayees.end()) {
              (*s_it).setPayeeId(payee_id);
              trans.modifySplit(*s_it); // does not modify the list object 'splits'!
            }
          } // for - Splits
          // store transaction in current schedule
          (*it).setTransaction(trans);
          file->modifySchedule(*it);  // modify the schedule in the MyMoney object
        } // for - Schedules
      } catch(MyMoneyException *e) {
        KMessageBox::detailedSorry(0, i18n("Unable to reassign payee of transaction/split"),
          (e->what() + " " + i18n("thrown in") + " " + e->file()+ ":%1").arg(e->line()));
        delete e;
      }
    } // if !translist.isEmpty()

    // now loop over all selected payees and remove them
    for (QValueList<MyMoneyPayee>::iterator it = m_selectedPayees.begin();
      it != m_selectedPayees.end(); ++it) {
      file->removePayee(*it);
    }
    file->suspendNotify(false);

  } catch(MyMoneyException *e) {
    KMessageBox::detailedSorry(0, i18n("Unable to remove payee(s)"),
      (e->what() + " " + i18n("thrown in") + " " + e->file()+ ":%1").arg(e->line()));
    delete e;
  }
}

void KMyMoney2App::showContextMenu(const QString& containerName)
{
  QWidget* w = factory()->container(containerName, this);
  QPopupMenu *menu = dynamic_cast<QPopupMenu*>(w);
  if(menu)
    menu->exec(QCursor::pos());
}

void KMyMoney2App::slotShowInvestmentContextMenu(void)
{
  showContextMenu("investment_context_menu");
}

void KMyMoney2App::slotShowAccountContextMenu(const MyMoneyObject& obj)
{
  qDebug("KMyMoney2App::slotShowAccountContextMenu");
  if(typeid(obj) != typeid(MyMoneyAccount))
    return;

  const MyMoneyAccount& acc = dynamic_cast<const MyMoneyAccount&>(obj);

  // if the selected account is actually a stock account, we
  // call the right slot instead
  if(acc.accountType() == MyMoneyAccount::Stock) {
    showContextMenu("investment_context_menu");
  } else {
    showContextMenu("account_context_menu");
  }
}

void KMyMoney2App::slotShowInstitutionContextMenu(const MyMoneyObject& obj)
{
  if(typeid(obj) != typeid(MyMoneyInstitution))
    return;

  showContextMenu("institution_context_menu");
}

void KMyMoney2App::slotShowPayeeContextMenu(void)
{
  showContextMenu("payee_context_menu");
}

void KMyMoney2App::slotPrintView(void)
{
  myMoneyView->slotPrintView();
}

void KMyMoney2App::updateCaption(bool skipActions)
{
  QString caption;

  caption = m_fileName.filename(false);

  if(caption.isEmpty() && myMoneyView && myMoneyView->fileOpen())
    caption = i18n("Untitled");

  // MyMoneyFile::instance()->dirty() throws an exception, if
  // there's no storage object available. In this case, we
  // assume that the storage object is not changed. Actually,
  // this can only happen if we are newly created early on.
  bool modified;
  try {
    modified = MyMoneyFile::instance()->dirty();
  } catch(MyMoneyException *e) {
    delete e;
    modified = false;
    skipActions = true;
  }

#if KMM_DEBUG
  QString sizeMsg;
  sizeMsg.sprintf(" (%d x %d)", width(), height());
  caption += sizeMsg;
#endif

  caption = kapp->makeStdCaption(caption, false, modified);
  if(caption.length() > 0)
    caption += " - ";
  caption += "KMyMoney";
  setPlainCaption(caption);

  if(!skipActions) {
    myMoneyView->enableViews();
    updateActions();
  }
}

void KMyMoney2App::updateActions(void)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  bool fileOpen = myMoneyView->fileOpen();
  bool modified = file->dirty();
  action("open_database")->setEnabled(true);
  action("saveas_database")->setEnabled(fileOpen);
  action("file_save")->setEnabled(modified);
  action("file_save_as")->setEnabled(fileOpen);
  action("file_close")->setEnabled(fileOpen);
  action("view_personal_data")->setEnabled(fileOpen);
  action("file_backup")->setEnabled(fileOpen);
#if KMM_DEBUG
  action("view_file_info")->setEnabled(fileOpen);
  action("file_dump")->setEnabled(fileOpen);
#endif

  action("edit_find_transaction")->setEnabled(fileOpen);
  action("file_export_qif")->setEnabled(fileOpen);
  action("file_import_qif")->setEnabled(fileOpen);
  action("file_import_gnc")->setEnabled(true);
  action("file_import_template")->setEnabled(fileOpen);
  action("file_export_template")->setEnabled(fileOpen);

  action("institution_new")->setEnabled(fileOpen);
  action("account_new")->setEnabled(fileOpen);
  action("category_new")->setEnabled(fileOpen);

  action("tools_security_editor")->setEnabled(fileOpen);
  action("tools_currency_editor")->setEnabled(fileOpen);
  action("tools_price_editor")->setEnabled(fileOpen);
  action("tools_update_prices")->setEnabled(fileOpen);
  action("tools_consistency_check")->setEnabled(fileOpen);

  action("account_reconcile")->setEnabled(false);
  action("account_edit")->setEnabled(false);
  action("account_delete")->setEnabled(false);
  action("account_open")->setEnabled(false);

#ifdef USE_OFX_DIRECTCONNECT
  action("account_update_ofx")->setEnabled(false);
#endif

  action("institution_edit")->setEnabled(false);
  action("institution_delete")->setEnabled(false);
  action("investment_new")->setEnabled(false);
  action("investment_edit")->setEnabled(false);
  action("investment_delete")->setEnabled(false);
  action("investment_online_price_update")->setEnabled(false);
  action("investment_manual_price_update")->setEnabled(false);

  action("schedule_edit")->setEnabled(false);
  action("schedule_delete")->setEnabled(false);
  action("schedule_enter")->setEnabled(false);

  action("payee_delete")->setEnabled(false);
  action("payee_rename")->setEnabled(false);

  if(!m_selectedAccount.id().isEmpty()) {
    if(!file->isStandardAccount(m_selectedAccount.id())) {
      action("account_edit")->setEnabled(true);
      action("account_delete")->setEnabled(!file->isReferenced(m_selectedAccount));
      switch(m_selectedAccount.accountGroup()) {
        case MyMoneyAccount::Asset:
        case MyMoneyAccount::Liability:
          action("account_open")->setEnabled(true);
          action("account_reconcile")->setEnabled(true);

          if(m_selectedAccount.accountType() == MyMoneyAccount::Investment)
            action("investment_new")->setEnabled(true);

#ifdef USE_OFX_DIRECTCONNECT
          if ( !m_selectedAccount.onlineBankingSettings().value("protocol").isEmpty() )
            action("account_update_ofx")->setEnabled(true);
#endif
          break;

        default:
          break;
      }
    }
  }

  if(!m_selectedInstitution.id().isEmpty()) {
    action("institution_edit")->setEnabled(true);
    action("institution_delete")->setEnabled(!file->isReferenced(m_selectedInstitution));
  }

  if(!m_selectedInvestment.id().isEmpty()) {
    action("investment_edit")->setEnabled(true);
    action("investment_delete")->setEnabled(!file->isReferenced(m_selectedInvestment));
    action("investment_manual_price_update")->setEnabled(true);
    try {
      MyMoneySecurity security = MyMoneyFile::instance()->security(m_selectedInvestment.currencyId());
      if(!security.value("kmm-online-source").isEmpty())
        action("investment_online_price_update")->setEnabled(true);

    } catch(MyMoneyException *e) {
      qDebug("Error retrieving security for investment %s: %s", m_selectedInvestment.name().data(), e->what().data());
      delete e;
    }
  }

  if(!m_selectedSchedule.id().isEmpty()) {
    action("schedule_edit")->setEnabled(true);
    action("schedule_delete")->setEnabled(!file->isReferenced(m_selectedSchedule));
    if(!m_selectedSchedule.isFinished())
      action("schedule_enter")->setEnabled(true);
  }

  if(m_selectedPayees.count() >= 1) {
    action("payee_rename")->setEnabled(m_selectedPayees.count() == 1);
    action("payee_delete")->setEnabled(true);
  }
}

void KMyMoney2App::slotSelectPayees(const QValueList<MyMoneyPayee>& list)
{
  m_selectedPayees = list;
  updateActions();
  emit payeesSelected(m_selectedPayees);
}

void KMyMoney2App::slotSelectInstitution(const MyMoneyObject& institution)
{
  if(typeid(institution) != typeid(MyMoneyInstitution))
    return;

  m_selectedInstitution = dynamic_cast<const MyMoneyInstitution&>(institution);
  // qDebug("slotSelectInstitution('%s')", m_selectedInstitution.name().data());
  updateActions();
  emit institutionSelected(m_selectedInstitution);
}

void KMyMoney2App::slotSelectAccount(const MyMoneyObject& obj)
{
  if(typeid(obj) != typeid(MyMoneyAccount))
    return;

  m_selectedAccount = MyMoneyAccount();
  const MyMoneyAccount& acc = dynamic_cast<const MyMoneyAccount&>(obj);
  if(acc.accountType() != MyMoneyAccount::Stock)
    m_selectedAccount = acc;

  // qDebug("slotSelectAccount('%s')", m_selectedAccount.name().data());
  updateActions();
  emit accountSelected(m_selectedAccount);
}

void KMyMoney2App::slotSelectInvestment(const MyMoneyObject& obj)
{
  if(typeid(obj) != typeid(MyMoneyAccount))
    return;

  // qDebug("slotSelectInvestment('%s')", account.name().data());
  m_selectedInvestment = MyMoneyAccount();
  const MyMoneyAccount& acc = dynamic_cast<const MyMoneyAccount&>(obj);
  if(acc.accountType() == MyMoneyAccount::Stock)
    m_selectedInvestment = acc;

  updateActions();
  emit investmentSelected(m_selectedInvestment);
}

void KMyMoney2App::slotSelectSchedule(const MyMoneySchedule& schedule)
{
  // qDebug("slotSelectSchedule('%s')", schedule.name().data());
  m_selectedSchedule = schedule;
  updateActions();
  emit scheduleSelected(m_selectedSchedule);
}

void KMyMoney2App::update(const QCString& /* id */)
{
  // As this method is called everytime the MyMoneyFile instance
  // notifies a modification, it's the perfect place to start the timer if needed
  if (m_autoSaveEnabled && !m_autoSaveTimer->isActive()) {
    m_autoSaveTimer->start(m_autoSavePeriod * 60 * 1000, true);  //miliseconds
  }
  updateCaption();
}

void KMyMoney2App::slotCurrencyDialog(void)
{
  KCurrencyEditDlg dlg(this, "Currency Editor");
  dlg.exec();

  myMoneyView->slotRefreshViews();
}

void KMyMoney2App::slotPriceDialog(void)
{
  KMyMoneyPriceDlg dlg(this, "Price Editor");
  dlg.exec();

  myMoneyView->slotRefreshViews();
}

void KMyMoney2App::slotFileConsitencyCheck(void)
{
  QString prevMsg = slotStatusMsg(i18n("Running consistency check..."));

  QStringList msg = MyMoneyFile::instance()->consistencyCheck();

  KMessageBox::warningContinueCancelList(0, "Result", msg, i18n("Consistency check result"));

  slotStatusMsg(prevMsg);
  updateCaption();
}

void KMyMoney2App::slotCheckSchedules(void)
{
  if(KMyMoneySettings::checkSchedule() == true) {

    MyMoneyFile::instance()->suspendNotify(true);
    QString prevMsg = slotStatusMsg(i18n("Checking for overdue schedules..."));
    MyMoneyFile *file = MyMoneyFile::instance();
    QDate checkDate = QDate::currentDate().addDays(KMyMoneySettings::checkSchedulePreview());

    QValueList<MyMoneySchedule> scheduleList =  file->scheduleList();
    QValueList<MyMoneySchedule>::Iterator it;

    for (it=scheduleList.begin(); it!=scheduleList.end(); ++it)
    {
      // Get the copy in the file because it might be modified by commitTransaction
      MyMoneySchedule schedule = file->schedule((*it).id());

      if (schedule.autoEnter())
      {
        while ((schedule.nextPayment(schedule.lastPayment()) <= checkDate) && !schedule.isFinished())
        {
          if (schedule.isFixed())
          {
            KEnterScheduleDialog dlg(0, schedule, schedule.nextPayment(schedule.lastPayment()));
            dlg.commitTransaction();
          }
          else
          {
            // 0.8 will feature a list of schedules for a better ui
            KEnterScheduleDialog dlg(0, schedule, schedule.nextPayment(schedule.lastPayment()));
            connect(&dlg, SIGNAL(newCategory(MyMoneyAccount&)), this, SLOT(slotCategoryNew(MyMoneyAccount&)));
            if (!dlg.exec())
            {
              int r = KMessageBox::warningYesNo(this, i18n("Are you sure you wish to stop this schedule from being entered into the register?\n\nKMyMoney will prompt you again next time it starts unless you manually enter it later."));
              if (r == KMessageBox::Yes)
              {
                break;
              }
            }
          }

          schedule = file->schedule((*it).id()); // get a copy of the modified schedule
        }
      }
    }
    MyMoneyFile::instance()->suspendNotify(false);
    slotStatusMsg(prevMsg);
    updateCaption();
  }
}

#if 0
bool KMyMoney2App::slotCommitTransaction(const MyMoneySchedule& sched, const QDate& date)
{
  MyMoneySchedule schedule = sched;
  QDate schedDate;
  // gather data for debug - problem 1105503
  QString d = "To:- kmymoney2-developer@lists.sourceforge.net\n";
  d += QString ("Subject:- autoEnter schedule post date problem (bug 1105503)\n");
  d += QString ("kmymoney2.cpp - slotCommitTransaction entered with:\n");
  d += QString ("Date %1\n").arg (QDate::currentDate().toString());
  d += QString ("Req date %1\n").arg (date.toString());
  d += QString ("Start date %1\n").arg (schedule.startDate().toString());
  d += QString ("End date %1\n").arg (schedule.endDate().toString());
  d += QString ("Last payment %1\n").arg (schedule.lastPayment().toString());
  d += QString ("Next payment %1\n").arg (schedule.nextPayment(schedule.lastPayment()).toString());
  d += QString ("Freq %1\n").arg ((int)schedule.occurence());
  d += QString ("W/e option %1").arg ((int)schedule.weekendOption());
  //

  if (date.isValid())
    schedDate = date;
  else
    schedDate = schedule.nextPayment(schedule.lastPayment());

  try
  {
    MyMoneyTransaction transaction = schedule.transaction();

    if (schedDate > schedule.nextPayment(schedule.lastPayment()))
      schedule.recordPayment(schedDate);
    else
      schedule.setLastPayment(schedDate);

    if (schedule.weekendOption() != MyMoneySchedule::MoveNothing)
    {
      int dayOfWeek = schedDate.dayOfWeek();
      if (dayOfWeek >= 6)
      {
        if (schedule.weekendOption() == MyMoneySchedule::MoveFriday)
        {
          if (dayOfWeek == 7)
            schedDate = schedDate.addDays(-2);
          else
            schedDate = schedDate.addDays(-1);
        }
        else
        {
          if (dayOfWeek == 6)
            schedDate = schedDate.addDays(2);
          else
            schedDate = schedDate.addDays(1);
        }
      }
    }
    if (!schedDate.isValid()) {
      QClipboard *cb = QApplication::clipboard();
      cb->setText(d, QClipboard::Clipboard);
      QMessageBox::information( 0, PACKAGE,
        i18n("Unable to autoEnter schedule %1. Please check manually!\n"
  "Debug data has been copied to clipboard; please paste into an\n"
  "email and send to kmymoney2-developer@lists.sourceforge.net").arg (schedule.name()));
      return (false);
    }

    transaction.setEntryDate(QDate::currentDate());
    transaction.setPostDate(schedDate);

    MyMoneyFile::instance()->addTransaction(transaction);

    try
    {
      MyMoneyFile::instance()->modifySchedule(schedule);
    }
    catch (MyMoneyException *e)
    {
      KMessageBox::error(this, i18n("Unable to modify schedule: ") + e->what());
      delete e;

    }
  }
  catch (MyMoneyException *e)
  {
    KMessageBox::error(this, i18n("Unable to add transaction: ") + e->what());

    delete e;
  }
  return (true);
}
#endif

void KMyMoney2App::writeLastUsedDir(const QString& directory)
{
  //get global config object for our app.
  KConfig *kconfig = KGlobal::config();
  if(kconfig)
  {
    kconfig->setGroup("General Options");

    //write path entry, no error handling since its void.
    kconfig->writePathEntry("LastUsedDirectory", directory);
  }
}

void KMyMoney2App::writeLastUsedFile(const QString& fileName)
{
  //get global config object for our app.
  KConfig *kconfig = KGlobal::config();
  if(kconfig)
  {
    kconfig->setGroup("General Options");

    // write path entry, no error handling since its void.
    // use a standard string, as fileName could contain a protocol
    // e.g. file:/home/thb/....
    kconfig->writeEntry("LastUsedFile", fileName);
  }
}

QString KMyMoney2App::readLastUsedDir() const
{
  QString str;

  //get global config object for our app.
  KConfig *kconfig = KGlobal::config();
  if(kconfig)
  {
    kconfig->setGroup("General Options");

    //read path entry.  Second parameter is the default if the setting is not found, which will be the default document path.
    str = kconfig->readPathEntry("LastUsedDirectory", KGlobalSettings::documentPath());
    // if the path stored is empty, we use the default nevertheless
    if(str.isEmpty())
      str = KGlobalSettings::documentPath();
  }

  return str;
}

QString KMyMoney2App::readLastUsedFile() const
{
  QString str;

  // get global config object for our app.
  KConfig *kconfig = KGlobal::config();
  if(kconfig)
  {
    kconfig->setGroup("General Options");

    // read filename entry.
    str = kconfig->readEntry("LastUsedFile", "");
  }

  return str;
}

#if 0
void KMyMoney2App::createInitialAccount(void)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  if(file->asset().accountCount() == 0
  && file->liability().accountCount() == 0
  && myMoneyView != 0) {
    KMessageBox::information(
      this,
      // I had some trouble with xgettext to get this in a single message when
      // I split the message over multiple lines. Make sure, you don't break
      // the i18n stuff when you modify this message. See kmymoney2.pot for
      // this entry. (ipwizard)
      i18n(
        "The currently opened KMyMoney document does not contain a single asset account. In order to maintain your finances you need at least one asset account (e.g. your checkings account). KMyMoney will start the \"New Account Wizard\" now which allows you to create your first asset account."
          ),
      i18n("No asset account"));
    slotAccountNew();
  }
}
#endif

const QString KMyMoney2App::filename() const
{
  return m_fileName.url();
}

const QCStringList KMyMoney2App::instanceList(void) const
{
  QCStringList list;
  QCStringList apps = kapp->dcopClient()->registeredApplications();
  QCStringList::ConstIterator it;

  for(it = apps.begin(); it != apps.end(); ++it) {
    // skip over myself
    if((*it) == kapp->dcopClient()->appId())
      continue;
    if((*it).find("kmymoney-") == 0) {
      list += (*it);
    }
  }
  return list;
}

void KMyMoney2App::slotEquityPriceUpdate()
{
  KEquityPriceUpdateDlg dlg(this);
  dlg.exec();
}

void KMyMoney2App::webConnect(const QString& url, const QCString& asn_id)
{
  //
  // Web connect attempts to go through the known importers and see if the file
  // can be importing using that method.  If so, it will import it unsing that
  // plugin
  //

  // Bring this window to the forefront.  This method was suggested by
  // Lubos Lunak <l.lunak@suse.cz> of the KDE core development team.
  KStartupInfo::setNewStartupId(this,asn_id);

  // Make sure we have an open file
  if ( ! myMoneyView->fileOpen() &&
    KMessageBox::warningContinueCancel(kmymoney2, i18n("You must first select a KMyMoney file before you can import a statement.")) == KMessageBox::Continue )
      kmymoney2->slotFileOpen();

  // only continue if the user really did open a file.
  if ( myMoneyView->fileOpen() )
  {
    QString prevMsg = slotStatusMsg(i18n("Importing a statement via Web Connect"));

    QMap<QString,KMyMoneyPlugin::ImporterPlugin*>::const_iterator it_plugin = m_importerPlugins.begin();
    while ( it_plugin != m_importerPlugins.end() )
    {
      if ( (*it_plugin)->isMyFormat(url) )
      {
        QValueList<MyMoneyStatement> statements;
        if ( (*it_plugin)->import(url,statements) )
        {
          slotStatementImport(statements);
        }
        else
        {
          KMessageBox::error( this, i18n("Unable to import %1 using %2 plugin.  The plugin returned the following error: %3").arg(url,(*it_plugin)->formatName(),(*it_plugin)->lastError()), i18n("Importing error"));
        }

        break;
      }
      ++it_plugin;
    }

    // If we did not find a match, try importing it as a KMM statement file,
    // which is really just for testing.  the statement file is not exposed
    // to users.
    if ( it_plugin == m_importerPlugins.end() )
      if ( MyMoneyStatement::isStatementFile( url ) )
        slotStatementImport(url);
  }
}

void KMyMoney2App::slotEnableMessages(void)
{
  KMessageBox::enableAllMessages();
  KMessageBox::information(this, i18n("All messages have been enabled."), i18n("All messages"));
}

void KMyMoney2App::slotSecurityEditor(void)
{
  KSecurityListEditor dlg(this, "KSecurityListEditor");
  dlg.exec();
}

void KMyMoney2App::createInterfaces(void)
{
  // Sets up the plugin interface, and load the plugins
  m_pluginInterface = new QObject( this, "_pluginInterface" );

  new KMyMoneyPlugin::KMMViewInterface(this, myMoneyView, m_pluginInterface, "view interface");
  new KMyMoneyPlugin::KMMStatementInterface(this, m_pluginInterface, "statement interface");
}

void KMyMoney2App::loadPlugins(void)
{
  {
    KTrader::OfferList offers = KTrader::self()->query("KMyMoneyPlugin");

    KTrader::OfferList::ConstIterator iter;
    for(iter = offers.begin(); iter != offers.end(); ++iter) {
      KService::Ptr service = *iter;
      int errCode = 0;

      KMyMoneyPlugin::Plugin* plugin =
        KParts::ComponentFactory::createInstanceFromService<KMyMoneyPlugin::Plugin>
        ( service, m_pluginInterface, service->name(), QStringList(), &errCode);
      // here we ought to check the error code.

      if (plugin) {
        guiFactory()->addClient(plugin);
        kdDebug() << "Loaded '"
                  << plugin->name() << "' plugin" << endl;
      } else {
        kdDebug() << "Failed to load '"
                  << service->name() << "' service, error=" << errCode << endl;
        kdDebug() << KLibLoader::self()->lastErrorMessage() << endl;
      }
    }
  }
  {
    KTrader::OfferList offers = KTrader::self()->query("KMyMoneyImporterPlugin");

    KTrader::OfferList::ConstIterator iter;
    for(iter = offers.begin(); iter != offers.end(); ++iter) {
      KService::Ptr service = *iter;
      int errCode = 0;

      KMyMoneyPlugin::ImporterPlugin* plugin =
        KParts::ComponentFactory::createInstanceFromService<KMyMoneyPlugin::ImporterPlugin>
        ( service, NULL, service->name(), QStringList(), &errCode);
      // here we ought to check the error code.

      if (plugin) {
        kdDebug() << "Loaded '"
                  << plugin->name() << "' importer plugin" << endl;

        // Create the custom action for this plugin
        QString format = plugin->formatName();
        KAction* action = new KAction(i18n("%1 (Plugin)...").arg(format), "", 0, m_pluginSignalMapper, SLOT(map()), actionCollection(), QString("file_import_plugin_%1").arg(format));

        // Add it to the signal mapper, so we'll know which plugin triggered the signal
        m_pluginSignalMapper->setMapping( action, format );

        // Add it to the plugin map, so we can find it later
        // FIXME: Check for duplicate and error out if there is already a plugin to handle this format.
        m_importerPlugins[format] = plugin;

        // Add it into the UI at the 'file_import_plugins' insertion point
        QPtrList<KAction> import_actions;
        import_actions.append( action );
        plugActionList( "file_import_plugins", import_actions );
      } else {
        kdDebug() << "Failed to load '"
                  << service->name() << "' service, error=" << errCode << endl;
        kdDebug() << KLibLoader::self()->lastErrorMessage() << endl;
      }
    }
  }
}

void KMyMoney2App::slotAutoSave()
{
  QString prevMsg = slotStatusMsg(i18n("Auto saving ..."));

  //calls slotFileSave if needed, and restart the timer
  //it the file is not saved, reinitializes the countdown.
  if (myMoneyView->dirty() && m_autoSaveEnabled) {
    if (!slotFileSave() && m_autoSavePeriod > 0) {
      m_autoSaveTimer->start(m_autoSavePeriod * 60 * 1000, true);
    }
  }

  slotStatusMsg(prevMsg);
}

#include "kmymoney2.moc"
// vim:cin:si:ai:et:ts=2:sw=2:
