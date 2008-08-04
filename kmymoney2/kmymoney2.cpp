/***************************************************************************
                          kmymoney2.cpp
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes
                           (C) 2007 by Thomas Baumgart
    email                : mte@users.sourceforge.net
                           ipwizard@users.sourceforge.net
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

// ----------------------------------------------------------------------------
// Std C++ / STL Includes

#include <typeinfo>
#include <cstdio>
#include <iostream>

// ----------------------------------------------------------------------------
// QT Includes

#include <qdir.h>
#include <qprinter.h>
#include <qlayout.h>
#include <qsignalmapper.h>
#include <qclipboard.h>        // temp for problem 1105503
#include <qdatetime.h>         // only for performance tests
#include <qtimer.h>
#include <qsqlpropertymap.h>
#include <qvbox.h>
#include <qeventloop.h>

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
#include <krun.h>
#include <kconfigdialog.h>
#include <kinputdialog.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoney2.h"
#include "kmymoneyglobalsettings.h"
#include "kmymoney2_stub.h"

#include "dialogs/kstartdlg.h"
#include "dialogs/settings/ksettingsgeneral.h"
#include "dialogs/settings/ksettingsregister.h"
#include "dialogs/settings/ksettingsgpg.h"
#include "dialogs/settings/ksettingscolors.h"
#include "dialogs/settings/ksettingsfonts.h"
#include "dialogs/settings/ksettingsschedules.h"
#include "dialogs/settings/ksettingsonlinequotes.h"
#include "dialogs/settings/ksettingshome.h"
#include "dialogs/settings/ksettingsforecast.h"
#include "dialogs/kbackupdlg.h"
#include "dialogs/kexportdlg.h"
#include "dialogs/kimportdlg.h"
#include "dialogs/mymoneyqifprofileeditor.h"
#include "dialogs/kenterscheduledlg.h"
#include "dialogs/kconfirmmanualenterdlg.h"
#include "dialogs/kmymoneypricedlg.h"
#include "dialogs/kcurrencyeditdlg.h"
#include "dialogs/kequitypriceupdatedlg.h"
#include "dialogs/ksecuritylisteditor.h"
#include "dialogs/kmymoneyfileinfodlg.h"
#include "dialogs/kfindtransactiondlg.h"
#include "dialogs/knewbankdlg.h"
#include "dialogs/knewinvestmentwizard.h"
#include "dialogs/knewaccountdlg.h"
#include "dialogs/knewfiledlg.h"
#include "dialogs/kselectdatabasedlg.h"
#include "dialogs/kcurrencycalculator.h"
#include "dialogs/keditscheduledlg.h"
#include "dialogs/knewloanwizard.h"
#include "dialogs/keditloanwizard.h"
#include "dialogs/kpayeereassigndlg.h"
#include "dialogs/kcategoryreassigndlg.h"
#include "dialogs/kmergetransactionsdlg.h"
#include "dialogs/kendingbalancedlg.h"
#include "dialogs/kbalancechartdlg.h"
#include "dialogs/kplugindlg.h"
#include "dialogs/kloadtemplatedlg.h"
#include "dialogs/kgpgkeyselectiondlg.h"
#include "dialogs/transactionmatcher.h"
#include "wizards/newuserwizard/knewuserwizard.h"
#include "wizards/newaccountwizard/knewaccountwizard.h"

#include "views/kmymoneyview.h"

#include "mymoney/mymoneyutils.h"
#include "mymoney/mymoneystatement.h"
#include "mymoney/storage/mymoneystoragedump.h"
#include "mymoney/mymoneyforecast.h"

#include "converter/mymoneyqifwriter.h"
#include "converter/mymoneyqifreader.h"
#include "converter/mymoneystatementreader.h"
#include "converter/mymoneytemplate.h"

#include "plugins/interfaces/kmmviewinterface.h"
#include "plugins/interfaces/kmmstatementinterface.h"
#include "plugins/interfaces/kmmimportinterface.h"

#include <libkgpgfile/kgpgfile.h>

#include <kmymoney/transactioneditor.h>
#include <kmymoney/kmymoneylistviewitem.h>

#include "kmymoneyutils.h"
#include "kdecompat.h"

#define RECOVER_KEY_ID        "59B0F826D2B08440"
#define ID_STATUS_MSG 1

class KMyMoneyPrivate
{
public:
  KMyMoneyPrivate() :
    m_moveToAccountSelector(0), m_pluginDlg(0)
  {}

  kMyMoneyAccountSelector* m_moveToAccountSelector;
  KPluginDlg*              m_pluginDlg;
};

KMyMoney2App::KMyMoney2App(QWidget * /*parent*/ , const char* name) :
  KMainWindow(0, name),
  DCOPObject("kmymoney2app"),
  d(new KMyMoneyPrivate),
  myMoneyView(0),
  m_searchDlg(0),
  m_autoSaveTimer(0),
  m_inAutoSaving(false),
  m_transactionEditor(0),
  m_endingBalanceDlg(0)
{
  ::timetrace("start kmymoney2app constructor");
  // preset the pointer because we need it during the course of this constructor
  kmymoney2 = this;
  config = kapp->config();

  MyMoneyTransactionFilter::setFiscalYearStart(KMyMoneyGlobalSettings::firstFiscalMonth(), KMyMoneyGlobalSettings::firstFiscalDay());

  updateCaption(true);

  QFrame* frame = new QFrame(this);
  frame->setFrameStyle(QFrame::NoFrame);
  // values for margin (11) and spacing(6) taken from KDialog implementation
  QBoxLayout* layout = new QBoxLayout(frame, QBoxLayout::TopToBottom, 2, 6);

  ::timetrace("init statusbar");
  initStatusBar();
  ::timetrace("init actions");
  initActions();

  initDynamicMenus();

  ::timetrace("create view");
  myMoneyView = new KMyMoneyView(frame, "KMyMoneyView");
  layout->addWidget(myMoneyView, 10);
  connect(myMoneyView, SIGNAL(aboutToShowPage(QWidget*)), this, SLOT(slotResetSelections()));

  ///////////////////////////////////////////////////////////////////
  // call inits to invoke all other construction parts
  ::timetrace("init options");
  readOptions();

#if 0
  m_pluginSignalMapper = new QSignalMapper( this );
  connect( m_pluginSignalMapper, SIGNAL( mapped( const QString& ) ), this, SLOT( slotPluginImport( const QString& ) ) );
#endif

  // now initialize the plugin structure
  ::timetrace("load plugins");
  d->m_pluginDlg = new KPluginDlg(this);
  d->m_pluginDlg->buttonCancel->hide();
  d->m_pluginDlg->buttonOk->hide();

  new KListViewItem(d->m_pluginDlg->m_listView, i18n("No plugins loaded"));
  createInterfaces();
  loadPlugins();

  setCentralWidget(frame);

  ::timetrace("done");

  connect(&proc,SIGNAL(processExited(KProcess *)),this,SLOT(slotProcessExited()));

  // force to show the home page if the file is closed
  connect(action("view_show_transaction_detail"), SIGNAL(toggled(bool)), myMoneyView, SLOT(slotShowTransactionDetail(bool)));

  m_backupState = BACKUP_IDLE;

  m_qifReader = 0;
  m_smtReader = 0;

  m_autoSaveEnabled = KMyMoneyGlobalSettings::autoSaveFile();
  m_autoSavePeriod = KMyMoneyGlobalSettings::autoSavePeriod();

  m_autoSaveTimer = new QTimer(this);
  connect(m_autoSaveTimer, SIGNAL(timeout()), this, SLOT(slotAutoSave()));

  // make sure, we get a note when the engine changes state
  connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotDataChanged()));

  // kickstart date change timer
  slotDateChanged();
}

KMyMoney2App::~KMyMoney2App()
{
  delete m_searchDlg;
  delete m_qifReader;
  delete m_transactionEditor;
  delete m_endingBalanceDlg;
  delete d->m_moveToAccountSelector;
  delete d;
  delete myMoneyView;
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

void KMyMoney2App::initDynamicMenus(void)
{
  QWidget* w = factory()->container("transaction_move_menu", this);
  QPopupMenu *menu = dynamic_cast<QPopupMenu*>(w);
  if(menu) {
    d->m_moveToAccountSelector = new kMyMoneyAccountSelector(menu, 0, 0, false);
    menu->insertItem(d->m_moveToAccountSelector);
    connect(d->m_moveToAccountSelector, SIGNAL(itemSelected(const QCString&)), this, SLOT(slotMoveToAccount(const QCString&)));
    connect(this, SIGNAL(accountSelected(const MyMoneyAccount&)), this, SLOT(slotUpdateMoveToAccountMenu()));
    connect(this, SIGNAL(transactionsSelected(const KMyMoneyRegister::SelectedTransactions&)), this, SLOT(slotUpdateMoveToAccountMenu()));
    connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotUpdateMoveToAccountMenu()));
  }
}

void KMyMoney2App::initActions(void)
{
  KAction* p;

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
  new KToggleAction(i18n("Hide reconciled transactions"), "hide_reconciled", KShortcut("Ctrl+R"), this, SLOT(slotHideReconciledTransactions()), actionCollection(), "view_hide_reconciled_transactions");
  new KToggleAction(i18n("Hide unused categories"), "hide_categories", KShortcut("Ctrl+U"), this, SLOT(slotHideUnusedCategories()), actionCollection(), "view_hide_unused_categories");
  new KToggleAction(i18n("Show all accounts"), "", KShortcut("Ctrl+Shift+A"), this, SLOT(slotShowAllAccounts()), actionCollection(), "view_show_all_accounts");

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
  // note : action "category_new" is included in this menu but defined below
  new KAction(i18n("Open ledger"), "account", 0, this, SLOT(slotAccountOpen()), actionCollection(), "account_open");
  new KAction(i18n("Reconcile..."), "reconcile", KShortcut("Ctrl+Shift+R"), this, SLOT(slotAccountReconcileStart()), actionCollection(), "account_reconcile");
  new KAction(i18n("Finish reconciliation", "Finish"), "player_end", 0, this, SLOT(slotAccountReconcileFinish()), actionCollection(), "account_reconcile_finish");
  new KAction(i18n("Postpone reconciliation", "Postpone"), "player_pause", 0, this, SLOT(slotAccountReconcilePostpone()), actionCollection(), "account_reconcile_postpone");
  new KAction(i18n("Edit account..."), "edit", 0, this, SLOT(slotAccountEdit()), actionCollection(), "account_edit");
  new KAction(i18n("Delete account..."), "delete", 0, this, SLOT(slotAccountDelete()), actionCollection(), "account_delete");
  new KAction(i18n("Close account"), "", 0, this, SLOT(slotAccountClose()), actionCollection(), "account_close");
  new KAction(i18n("Reopen account"), "", 0, this, SLOT(slotAccountReopen()), actionCollection(), "account_reopen");
  new KAction(i18n("Transaction report"), "view_info", 0, this, SLOT(slotAccountTransactionReport()), actionCollection(), "account_transaction_report");
#ifdef HAVE_KDCHART
  new KAction(i18n("Show balance chart..."), "kchart_chrt", 0, this, SLOT(slotAccountChart()), actionCollection(), "account_chart");
#endif
  new KAction(i18n("Map to online account"), "news_subscribe", 0, this, SLOT(slotAccountMapOnline()), actionCollection(), "account_online_map");
  new KAction(i18n("Update account..."), "reload", 0, this, SLOT(slotAccountUpdateOnline()), actionCollection(), "account_online_update");

  // *******************
  // The categories menu
  // *******************
  new KAction(i18n("New category..."), "account_add", 0, this, SLOT(slotCategoryNew()), actionCollection(), "category_new");
  new KAction(i18n("Edit category..."), "edit", 0, this, SLOT(slotAccountEdit()), actionCollection(), "category_edit");
  new KAction(i18n("Delete category..."), "delete", 0, this, SLOT(slotAccountDelete()), actionCollection(), "category_delete");


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
  new KAction(i18n("Plugins"), "", 0, this, SLOT(slotToolsPluginDlg()), actionCollection(), "tools_plugin_list");

  // *****************
  // The settings menu
  // *****************
  KStdAction::preferences(this, SLOT( slotSettings() ), actionCollection());
  new KAction(i18n("Enable all messages"), "", 0, this, SLOT(slotEnableMessages()), actionCollection(), "settings_enable_messages");
  new KAction(i18n("KDE language settings..."), "", 0, this, SLOT(slotKDELanguageSettings()), actionCollection(), "settings_language");

  // *************
  // The help menu
  // *************
  new KAction(i18n("&Show tip of the day"), "idea", 0, this, SLOT(slotShowTipOfTheDay()), actionCollection(), "help_show_tip");

  // ***************************
  // Actions w/o main menu entry
  // ***************************
  new KAction(i18n("New transaction button", "New"), "filenew", QKeySequence(Qt::CTRL | Qt::Key_Insert), this, SLOT(slotTransactionsNew()), actionCollection(), "transaction_new");

  // we use Return as the same shortcut for Edit and Enter. Therefore, we don't allow
  // to change them (The standard KDE dialog complains anyway if you want to assign
  // the same shortcut to two actions)
  p = new KAction(i18n("Edit transaction button", "Edit"), "edit", 0, this, SLOT(slotTransactionsEdit()), actionCollection(), "transaction_edit");
  p->setShortcutConfigurable(false);
  p = new KAction(i18n("Enter transaction", "Enter"), "button_ok", 0, this, SLOT(slotTransactionsEnter()), actionCollection(), "transaction_enter");
  p->setShortcutConfigurable(false);

  new KAction(i18n("Edit split button", "Edit splits"), "split_transaction", 0, this, SLOT(slotTransactionsEditSplits()), actionCollection(), "transaction_editsplits");
  new KAction(i18n("Cancel transaction edit", "Cancel"), "button_cancel", 0, this, SLOT(slotTransactionsCancel()), actionCollection(), "transaction_cancel");
  new KAction(i18n("Delete transaction", "Delete"), "delete", 0, this, SLOT(slotTransactionsDelete()), actionCollection(), "transaction_delete");
  new KAction(i18n("Duplicate transaction", "Duplicate"), "editcopy", 0, this, SLOT(slotTransactionDuplicate()), actionCollection(), "transaction_duplicate");

  new KAction(i18n("Button text for match transaction", "Match"), "stop", 0, this, SLOT(slotTransactionMatch()), actionCollection(), "transaction_match");
  new KAction(i18n("Accept 'imported' and 'matched' transaction", "Accept"), "apply", 0, this, SLOT(slotTransactionsAccept()), actionCollection(), "transaction_accept");

  new KAction(i18n("Toggle reconciliation flag", "Toggle"), 0, KShortcut("Ctrl+Space"), this, SLOT(slotToggleReconciliationFlag()), actionCollection(), "transaction_mark_toggle");
  new KAction(i18n("Mark transaction cleared", "Cleared"), 0, KShortcut("Ctrl+Alt+Space"), this, SLOT(slotMarkTransactionCleared()), actionCollection(), "transaction_mark_cleared");
  new KAction(i18n("Mark transaction reconciled", "Reconciled"), "", KShortcut("Ctrl+Shift+Space"), this, SLOT(slotMarkTransactionReconciled()), actionCollection(), "transaction_mark_reconciled");
  new KAction(i18n("Mark transaction not reconciled", "Not reconciled"), "", 0, this, SLOT(slotMarkTransactionNotReconciled()), actionCollection(), "transaction_mark_notreconciled");
  new KAction(i18n("Select all transactions", "Select all"), 0, KShortcut("Ctrl+A"), this, SIGNAL(selectAllTransactions()), actionCollection(), "transaction_select_all");

  new KAction(i18n("Goto account"), "goto", 0, this, SLOT(slotTransactionGotoAccount()), actionCollection(), "transaction_goto_account");
  new KAction(i18n("Goto payee"), "goto", 0, this, SLOT(slotTransactionGotoPayee()), actionCollection(), "transaction_goto_payee");
  new KAction(i18n("Create schedule..."), "bookmark_add", 0, this, SLOT(slotTransactionCreateSchedule()), actionCollection(), "transaction_create_schedule");
  new KAction(i18n("Assign next number"), "", KShortcut("Ctrl+Shift+N"), this, SLOT(slotTransactionAssignNumber()), actionCollection(), "transaction_assign_number");
  new KAction(i18n("Combine transactions", "Combine"), "", 0, this, SLOT(slotTransactionCombine()), actionCollection(), "transaction_combine");

  new KAction(i18n("New investment"), "filenew", 0, this, SLOT(slotInvestmentNew()), actionCollection(), "investment_new");
  new KAction(i18n("Edit investment..."), "edit", 0, this, SLOT(slotInvestmentEdit()), actionCollection(), "investment_edit");
  new KAction(i18n("Delete investment..."), "delete", 0, this, SLOT(slotInvestmentDelete()), actionCollection(), "investment_delete");
  new KAction(i18n("Online price update..."), "", 0, this, SLOT(slotOnlinePriceUpdate()), actionCollection(), "investment_online_price_update");
  new KAction(i18n("Manual price update..."), "", 0, this, SLOT(slotManualPriceUpdate()), actionCollection(), "investment_manual_price_update");

  new KAction(i18n("New schedule..."), "filenew", 0, this, SLOT(slotScheduleNew()), actionCollection(), "schedule_new");
  new KAction(i18n("Edit schedule..."), "edit", 0, this, SLOT(slotScheduleEdit()), actionCollection(), "schedule_edit");
  new KAction(i18n("Delete schedule..."), "delete", 0, this, SLOT(slotScheduleDelete()), actionCollection(), "schedule_delete");
  new KAction(i18n("Duplicate schedule"), "editcopy", 0, this, SLOT(slotScheduleDuplicate()), actionCollection(), "schedule_duplicate");
  new KAction(i18n("Enter schedule..."), "", 0, this, SLOT(slotScheduleEnter()), actionCollection(), "schedule_enter");
  new KAction(i18n("Skip schedule..."), "player_fwd", 0, this, SLOT(slotScheduleSkip()), actionCollection(), "schedule_skip");

  new KAction(i18n("New payee"), "filenew", 0, this, SLOT(slotPayeeNew()), actionCollection(), "payee_new");
  new KAction(i18n("Rename payee"), "edit", 0, this, SIGNAL(payeeRename()), actionCollection(), "payee_rename");
  new KAction(i18n("Delete payee"), "delete", 0, this, SLOT(slotPayeeDelete()), actionCollection(), "payee_delete");

  new KAction(i18n("New budget"), "filenew", 0, this, SLOT(slotBudgetNew()), actionCollection(), "budget_new");
  new KAction(i18n("Rename budget"), "edit", 0, this, SIGNAL(budgetRename()), actionCollection(), "budget_rename");
  new KAction(i18n("Delete budget"), "delete", 0, this, SLOT(slotBudgetDelete()), actionCollection(), "budget_delete");
  new KAction(i18n("Copy budget"), "editcopy", 0, this, SLOT(slotBudgetCopy()), actionCollection(), "budget_copy");
  new KAction(i18n("Change budget year"), "", 0, this, SLOT(slotBudgetChangeYear()), actionCollection(), "budget_change_year");
  new KAction(i18n("Budget based on forecast", "Forecast"), "forcast", 0, this, SLOT(slotBudgetForecast()), actionCollection(), "budget_forecast");

  // ************************
  // Currency actions
  // ************************
  new KAction(i18n("New currency"), "filenew", 0, this, SLOT(slotCurrencyNew()), actionCollection(), "currency_new");
  new KAction(i18n("Rename currency"), "edit", 0, this, SIGNAL(currencyRename()), actionCollection(), "currency_rename");
  new KAction(i18n("Delete currency"), "delete", 0, this, SLOT(slotCurrencyDelete()), actionCollection(), "currency_delete");
  new KAction(i18n("Select as base currency"), "kmymoney2", 0, this, SLOT(slotCurrencySetBase()), actionCollection(), "currency_setbase");

#ifdef KMM_DEBUG
  new KAction("Test new feature", "", KShortcut("Ctrl+G"), this, SLOT(slotNewFeature()), actionCollection(), "new_user_wizard");
  new KToggleAction("Debug Traces", "", 0, this, SLOT(slotToggleTraces()), actionCollection(), "debug_traces");
  new KToggleAction("Debug Timers", "", 0, this, SLOT(slotToggleTimers()), actionCollection(), "debug_timers");
#endif
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
  toggleAction("view_show_transaction_detail")->setChecked(KMyMoneyGlobalSettings::showRegisterDetailed());
  toggleAction("view_hide_reconciled_transactions")->setChecked(KMyMoneyGlobalSettings::hideReconciledTransactions());
  toggleAction("view_hide_unused_categories")->setChecked(KMyMoneyGlobalSettings::hideUnusedCategory());
  toggleAction("view_show_all_accounts")->setChecked(false);

  // use the absolute path to your kmymoney2ui.rc file for testing purpose in createGUI();
  // createGUI(QString::null, false);
  setupGUI();
}

void KMyMoney2App::dumpActions(void) const
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


void KMyMoney2App::initStatusBar(void)
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

void KMyMoney2App::saveOptions(void)
{
  config->setGroup("General Options");
  config->writeEntry("Geometry", size());
  // config->writeEntry("Show Statusbar", toggleAction("options_show_statusbar")->isChecked());
  // toolBar("mainToolBar")->saveSettings(config, "mainToolBar");

  dynamic_cast<KRecentFilesAction*>(action("file_open_recent"))->saveEntries(config,"Recent Files");
}


void KMyMoney2App::readOptions(void)
{
  config->setGroup("General Options");

  toggleAction("view_hide_reconciled_transactions")->setChecked(KMyMoneyGlobalSettings::hideReconciledTransactions());
  toggleAction("view_hide_unused_categories")->setChecked(KMyMoneyGlobalSettings::hideUnusedCategory());

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

bool KMyMoney2App::queryClose(void)
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
  if (myMoneyView->isSyncDatabase())
    slotFileClose(); // close off the database
  return true;
}

bool KMyMoney2App::queryExit(void)
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
  MyMoneyFile::instance()->preloadCache();
  measurement[0] = measurement[1] = 0;
  timer.start();
  for(int i = 0; i < 1000; ++i) {
    QValueList<MyMoneyAccount> list;
    MyMoneyFile::instance()->accountList(list);
    measurement[i != 0] = timer.elapsed();
  }
  std::cerr << "accountList()" << std::endl;
  std::cerr << "First time: " << measurement[0] << " msec" << std::endl;
  std::cerr << "Total time: " << (measurement[0] + measurement[1]) << " msec" << std::endl;
  std::cerr << "Average   : " << (measurement[0] + measurement[1]) / 1000 << " msec" << std::endl;

  // Balance of asset account(s)
  MyMoneyFile::instance()->preloadCache();
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
  MyMoneyFile::instance()->preloadCache();
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
  MyMoneyFile::instance()->preloadCache();
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
  MyMoneyFile::instance()->preloadCache();
  measurement[0] = measurement[1] = 0;
  acc = MyMoneyFile::instance()->expense();
  timer.start();
  for(int i = 0; i < 1000; ++i) {
    MyMoneyMoney result = MyMoneyFile::instance()->totalBalance(acc.id());
    measurement[i != 0] = timer.elapsed();
  }
  std::cerr << "totalBalance(Expense)" << std::endl;
  std::cerr << "First time: " << measurement[0] << " msec" << std::endl;
  std::cerr << "Total time: " << (measurement[0] + measurement[1]) << " msec" << std::endl;
  std::cerr << "Average   : " << (measurement[0] + measurement[1]) / 1000 << " msec" << std::endl;

  // transaction list
  MyMoneyFile::instance()->preloadCache();
  measurement[0] = measurement[1] = 0;
  if(MyMoneyFile::instance()->asset().accountCount()) {
    MyMoneyTransactionFilter filter(MyMoneyFile::instance()->asset().accountList()[0]);
    filter.setDateFilter(QDate(), QDate::currentDate());
    QValueList<MyMoneyTransaction> list;

    timer.start();
    for(int i = 0; i < 100; ++i) {
      list = MyMoneyFile::instance()->transactionList(filter);
      measurement[i != 0] = timer.elapsed();
    }
    std::cerr << "transactionList()" << std::endl;
    std::cerr << "First time: " << measurement[0] << " msec" << std::endl;
    std::cerr << "Total time: " << (measurement[0] + measurement[1]) << " msec" << std::endl;
    std::cerr << "Average   : " << (measurement[0] + measurement[1]) / 100 << " msec" << std::endl;
  }

  // transaction list
  MyMoneyFile::instance()->preloadCache();
  measurement[0] = measurement[1] = 0;
  if(MyMoneyFile::instance()->asset().accountCount()) {
    MyMoneyTransactionFilter filter(MyMoneyFile::instance()->asset().accountList()[0]);
    filter.setDateFilter(QDate(), QDate::currentDate());
    QValueList<MyMoneyTransaction> list;

    timer.start();
    for(int i = 0; i < 100; ++i) {
      MyMoneyFile::instance()->transactionList(list, filter);
      measurement[i != 0] = timer.elapsed();
    }
    std::cerr << "transactionList(list)" << std::endl;
    std::cerr << "First time: " << measurement[0] << " msec" << std::endl;
    std::cerr << "Total time: " << (measurement[0] + measurement[1]) << " msec" << std::endl;
    std::cerr << "Average   : " << (measurement[0] + measurement[1]) / 100 << " msec" << std::endl;
  }
  MyMoneyFile::instance()->preloadCache();
}

void KMyMoney2App::slotFileNew(void)
{
  KMSTATUS(i18n("Creating new document..."));

  slotFileClose();

  if(!myMoneyView->fileOpen()) {
    // next line required until we move all file handling out of KMyMoneyView
    myMoneyView->newFile();

    m_fileName = KURL();
    updateCaption();

    // before we create the wizard, we need to preload the currencies
    MyMoneyFileTransaction ft;
    myMoneyView->loadDefaultCurrencies();
    myMoneyView->loadAncientCurrencies();
    ft.commit();

    NewUserWizard::Wizard *wizard = new NewUserWizard::Wizard();

    if(wizard->exec() == QDialog::Accepted) {
      MyMoneyFile* file = MyMoneyFile::instance();
      ft.restart();
      try {
        // store the user info
        file->setUser(wizard->user());

        // setup base currency
        file->setBaseCurrency(wizard->baseCurrency());

        // create a possible institution
        MyMoneyInstitution inst = wizard->institution();
        if(inst.name().length()) {
          file->addInstitution(inst);
        }

        // create a possible checking account
        MyMoneyAccount acc = wizard->account();
        if(acc.name().length()) {
          acc.setInstitutionId(inst.id());
          MyMoneyAccount asset = file->asset();
          file->addAccount(acc, asset);

          // create possible opening balance transaction
          if(!wizard->openingBalance().isZero()) {
            file->createOpeningBalanceTransaction(acc, wizard->openingBalance());
          }
        }

        // import the account templates
        QValueList<MyMoneyTemplate> templates = wizard->templates();
        QValueList<MyMoneyTemplate>::iterator it_t;
        for(it_t = templates.begin(); it_t != templates.end(); ++it_t) {
          (*it_t).importTemplate(&progressCallback);
        }

        m_fileName = KURL(wizard->url());
        ft.commit();
        KMyMoneyGlobalSettings::setFirstTimeRun(false);
      } catch(MyMoneyException* e) {
        delete e;
        // next line required until we move all file handling out of KMyMoneyView
        myMoneyView->closeFile();
      }

    } else {
      // next line required until we move all file handling out of KMyMoneyView
      myMoneyView->closeFile();
    }
    delete wizard;
    updateCaption();

    emit fileLoaded(m_fileName);
  }
}

KURL KMyMoney2App::selectFile(const QString& title, const QString& _path, const QString& mask, KFile::Mode mode)
{
  KURL url;
  QString path(_path);

  if(path.isEmpty())
    path = KGlobalSettings::documentPath();

  KFileDialog* dialog = new KFileDialog(path, mask, this, title, true);
  dialog->setMode(mode);

  if(dialog->exec() == QDialog::Accepted) {
    url = dialog->selectedURL();
  }
  delete dialog;

  return url;
}

// General open
void KMyMoney2App::slotFileOpen(void)
{
  KMSTATUS(i18n("Open a file."));

  KFileDialog* dialog = new KFileDialog(KGlobalSettings::documentPath(),
                            i18n("%1|KMyMoney files\n%2|All files (*.*)").arg("*.kmy *.xml").arg("*"),
                            this, i18n("Open File..."), true);
  dialog->setMode(KFile::File | KFile::ExistingOnly);

  if(dialog->exec() == QDialog::Accepted) {
    slotFileOpenRecent(dialog->selectedURL());
  }
  delete dialog;
}

void KMyMoney2App::slotOpenDatabase(void)
{
  KMSTATUS(i18n("Open a file."));
  KSelectDatabaseDlg dialog;
  dialog.setMode(IO_ReadWrite);

  if(dialog.exec() == QDialog::Accepted) {
    slotFileOpenRecent(dialog.selectedURL());
  }
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
  KMSTATUS(i18n("Loading file..."));
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
        if(myMoneyView->readFile(url)) {
          if((myMoneyView->isNativeFile())) {
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
      KMessageBox::sorry(this, QString("<p>")+i18n("<b>%1</b> is either an invalid filename or the file does not exist. You can open another file or create a new one.").arg(url.prettyURL(0, KURL::StripFileProtocol)), i18n("File not found"));
    }
  } else {
    KMessageBox::sorry(this, QString("<p>")+i18n("File <b>%1</b> is already opened in another instance of KMyMoney").arg(url.prettyURL(0, KURL::StripFileProtocol)), i18n("Duplicate open"));
  }
}

const bool KMyMoney2App::slotFileSave(void)
{
  // if there's nothing changed, there's no need to save anything
  if(!myMoneyView->dirty())
    return true;

  bool rc = false;

  KMSTATUS(i18n("Saving file..."));

  if (m_fileName.isEmpty())
    return slotFileSaveAs();

  /*if (myMoneyView->isDatabase()) {
    rc = myMoneyView->saveDatabase(m_fileName);
    // the 'save' function is no longer relevant for a database*/
  setEnabled(false);
  rc = myMoneyView->saveFile(m_fileName, MyMoneyFile::instance()->value("kmm-encryption-key"));
  setEnabled(true);

  m_autoSaveTimer->stop();

  updateCaption();
  return rc;
}

void KMyMoney2App::slotFileSaveAsFilterChanged(const QString& filter)
{
  if(filter != "*.kmy") {
    m_saveEncrypted->setCurrentItem(0);
    m_saveEncrypted->setEnabled(false);
  } else {
    m_saveEncrypted->setEnabled(true);
  }
}

void KMyMoney2App::slotManageGpgKeys(void)
{
  KGpgKeySelectionDlg dlg(this);
  dlg.setKeys(m_additionalGpgKeys);
  if(dlg.exec() == QDialog::Accepted) {
    m_additionalGpgKeys = dlg.keys();
    m_additionalKeyLabel->setText(i18n("Additional encryption key(s) to be used: %1").arg(m_additionalGpgKeys.count()));
  }
}

void KMyMoney2App::slotKeySelected(int idx)
{
  int cnt = 0;
  if(idx != 0) {
    cnt = m_additionalGpgKeys.count();
  }
  m_additionalKeyLabel->setEnabled(idx != 0);
  m_additionalKeyButton->setEnabled(idx != 0);
  m_additionalKeyLabel->setText(i18n("Additional encryption key(s) to be used: %1").arg(cnt));
}

const bool KMyMoney2App::slotFileSaveAs(void)
{
  bool rc = false;
  // in event of it being a database, ensure that all data is read into storage for saveas
  if (myMoneyView->isSyncDatabase())
        dynamic_cast<IMyMoneySerialize*> (MyMoneyFile::instance()->storage())->fillStorage();
  KMSTATUS(i18n("Saving file with a new filename..."));
  QString prevDir= ""; // don't prompt file name if not a native file
  if (myMoneyView->isNativeFile())
    prevDir = readLastUsedDir();

  // fill the additional key list with the default
  m_additionalGpgKeys = KMyMoneyGlobalSettings::gpgRecipientList();

  QVBox* vbox = new QVBox();
  if(KGPGFile::GPGAvailable()) {
    QHBox* keyBox = new QHBox(vbox);
    new QLabel(i18n("Encryption key to be used"), keyBox);
    m_saveEncrypted = new KComboBox(keyBox);

    QHBox* labelBox = new QHBox(vbox);
    m_additionalKeyLabel = new QLabel(i18n("Additional encryption key(s) to be used: %1").arg(m_additionalGpgKeys.count()), labelBox);
    m_additionalKeyButton = new KPushButton(i18n("Manage additional keys"), labelBox);
    connect(m_additionalKeyButton, SIGNAL(clicked()), this, SLOT(slotManageGpgKeys()));
    connect(m_saveEncrypted, SIGNAL(activated(int)), this, SLOT(slotKeySelected(int)));

    // fill the secret key list and combo box
    QStringList keyList;
    KGPGFile::secretKeyList(keyList);
    m_saveEncrypted->insertItem(i18n("No encryption"));

    for(QStringList::iterator it = keyList.begin(); it != keyList.end(); ++it) {
      QStringList fields = QStringList::split(":", *it);
      if(fields[0] != RECOVER_KEY_ID) {
        // replace parenthesis in name field with brackets
        QString name = fields[1];
        name.replace('(', "[");
        name.replace(')', "]");
        name = QString("%1 (0x%2)").arg(name).arg(fields[0]);
        m_saveEncrypted->insertItem(name);
        if(name.contains(KMyMoneyGlobalSettings::gpgRecipient())) {
          m_saveEncrypted->setCurrentItem(name);
        }
      }
    }
  }

  // the following code is copied from KFileDialog::getSaveFileName,
  // adjust to our local needs (filetypes etc.) and
  // enhanced to show the m_saveEncrypted combo box
  bool specialDir = prevDir.at(0) == ':';
  KFileDialog dlg( specialDir ? prevDir : QString::null,
                   QString("%1|%2\n").arg("*.kmy").arg(i18n("KMyMoney (Filefilter)", "KMyMoney files")) +
                   QString("%1|%2\n").arg("*.xml").arg(i18n("XML (Filefilter)", "XML files")) +
                   QString("%1|%2\n").arg("*.anon.xml").arg(i18n("Anonymous (Filefilter)", "Anonymous files")) +
                   QString("%1|%2\n").arg("*").arg(i18n("All files")),
                   this, "filedialog", true, vbox);
  connect(&dlg, SIGNAL(filterChanged(const QString&)), this, SLOT(slotFileSaveAsFilterChanged(const QString&)));

  if ( !specialDir )
    dlg.setSelection( prevDir ); // may also be a filename

  dlg.setOperationMode( KFileDialog::Saving );
  dlg.setCaption(i18n("Save As"));

  if(dlg.exec() == QDialog::Accepted) {

    KURL newURL = dlg.selectedURL();
    if (!newURL.isEmpty()) {
      QString newName = newURL.prettyURL(0, KURL::StripFileProtocol);

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

      if(okToWriteFile(newName)) {
        KRecentFilesAction *p = dynamic_cast<KRecentFilesAction*>(action("file_open_recent"));
        if(p)
          p->addURL( newName );

        setEnabled(false);
        // If this is the anonymous file export, just save it, don't actually take the
        // name, or remember it! Don't even try to encrypt it
        if (newName.right(9).lower() == ".anon.xml")
        {
          rc = myMoneyView->saveFile(newName);
        }
        else
        {

          m_fileName = newName;
          QString encryptionKeys;
          if(m_saveEncrypted->currentItem() != 0) {
            QRegExp keyExp(".* \\((.*)\\)");
            if(keyExp.search(m_saveEncrypted->currentText()) != -1) {
              encryptionKeys = keyExp.cap(1);
            }
            if(!m_additionalGpgKeys.isEmpty()) {
              if(!encryptionKeys.isEmpty())
                encryptionKeys += ",";
              encryptionKeys += m_additionalGpgKeys.join(",");
            }
          }
          rc = myMoneyView->saveFile(newName, encryptionKeys);
          //write the directory used for this file as the default one for next time.
          writeLastUsedDir(newName);
          writeLastUsedFile(newName);
        }
        m_autoSaveTimer->stop();
        setEnabled(true);
      }
    }
  }

  updateCaption();
  return rc;
}

const bool KMyMoney2App::slotSaveAsDatabase(void)
{

  bool rc = false;
  // in event of it being a database, ensure that all data is read into storage for saveas
  if (myMoneyView->isSyncDatabase())
    dynamic_cast<IMyMoneySerialize*> (MyMoneyFile::instance()->storage())->fillStorage();
  KMSTATUS(i18n("Saving file to database..."));
  KSelectDatabaseDlg dialog;
  dialog.setMode(IO_WriteOnly);
  KURL oldUrl = m_fileName.isEmpty() ? lastOpenedURL() : m_fileName;
  KURL url = oldUrl;

  while (oldUrl == url && dialog.exec() == QDialog::Accepted) {
    url = dialog.selectedURL();
    // If the protocol is SQL for the old and new, and the hostname and database names match
    // Let the user know that the current database cannot be saved on top of itself.
    if (url.protocol() == "sql" && oldUrl.protocol() == "sql"
    && oldUrl.host() == url.host()
    && oldUrl.queryItem("driver") == url.queryItem("driver")
    && oldUrl.path().right(oldUrl.path().length() - 1) == url.path().right(url.path().length() - 1)) {
      KMessageBox::sorry(this, i18n("Cannot save to current database."));
    } else {
      rc = myMoneyView->saveAsDatabase(url);
    }
  }
  if (rc) writeLastUsedFile(url.prettyURL(0, KURL::StripFileProtocol));
  m_autoSaveTimer->stop();
  updateCaption();
  return rc;
}

void KMyMoney2App::slotFileCloseWindow(void)
{
  KMSTATUS(i18n("Closing window..."));

  if (myMoneyView->dirty()) {
    int answer = KMessageBox::warningYesNoCancel(this, i18n("The file has been changed, save it ?"));
    if (answer == KMessageBox::Cancel)
      return;
    else if (answer == KMessageBox::Yes)
      slotFileSave();
  }
  close();
}

void KMyMoney2App::slotFileClose(void)
{
  bool okToSelect = true;

  // check if transaction editor is open and ask user what he wants to do
  slotTransactionsCancelOrEnter(okToSelect);

  if(!okToSelect)
    return;

  // no update status here, as we might delete the status too early.
  if (myMoneyView->dirty()) {
    int answer = KMessageBox::warningYesNoCancel(this, i18n("The file has been changed, save it ?"));
    if (answer == KMessageBox::Cancel)
      return;
    else if (answer == KMessageBox::Yes)
      slotFileSave();
  }

  slotSelectAccount();
  slotSelectInstitution();
  slotSelectInvestment();
  slotSelectSchedule();
  slotSelectCurrency();
  slotSelectBudget(QValueList<MyMoneyBudget>());
  slotSelectPayees(QValueList<MyMoneyPayee>());
  slotSelectTransactions(KMyMoneyRegister::SelectedTransactions());

  m_reconciliationAccount = MyMoneyAccount();
  myMoneyView->finishReconciliation(m_reconciliationAccount);

  myMoneyView->closeFile();
  m_fileName = KURL();
  updateCaption();

  emit fileLoaded(m_fileName);
}

void KMyMoney2App::slotFileQuit(void)
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

void KMyMoney2App::slotHideReconciledTransactions(void)
{
  KMyMoneyGlobalSettings::setHideReconciledTransactions(toggleAction("view_hide_reconciled_transactions")->isChecked());
  myMoneyView->slotRefreshViews();
}

void KMyMoney2App::slotHideUnusedCategories(void)
{
  KMyMoneyGlobalSettings::setHideUnusedCategory(toggleAction("view_hide_unused_categories")->isChecked());
  myMoneyView->slotRefreshViews();
}

void KMyMoney2App::slotShowAllAccounts(void)
{
  myMoneyView->slotRefreshViews();
}

void KMyMoney2App::slotToggleTraces(void)
{
  MyMoneyTracer::onOff(toggleAction("debug_traces")->isChecked() ? 1 : 0);
}

void KMyMoney2App::slotToggleTimers(void)
{
  extern bool timersOn; // main.cpp
  timersOn = toggleAction("debug_timers")->isChecked();
}

const QString KMyMoney2App::slotStatusMsg(const QString &text)
{
  ///////////////////////////////////////////////////////////////////
  // change status message permanently
  QString msg = m_statusMsg;

  m_statusMsg = text;
  if(m_statusMsg.isEmpty())
    m_statusMsg = i18n("Ready.");
  statusBar()->clear();
  statusBar()->changeItem(text, ID_STATUS_MSG);
  return msg;
}

void KMyMoney2App::ready(void)
{
  slotStatusMsg(QString());
}

bool KMyMoney2App::isReady(void)
{
  return m_statusMsg == i18n("Ready.");
}

void KMyMoney2App::slotStatusProgressBar(int current, int total)
{
  if(total == -1 && current == -1) {      // reset
    progressBar->reset();
    progressBar->hide();
    m_nextUpdate = 0;

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
      QApplication::eventLoop()->processEvents(QEventLoop::ExcludeUserInput, 10);
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

void KMyMoney2App::slotFileViewPersonal(void)
{
  if ( !myMoneyView->fileOpen() ) {
    KMessageBox::information(this, i18n("No KMyMoneyFile open"));
    return;
  }

  KMSTATUS(i18n("Viewing personal data..."));

  MyMoneyFile* file = MyMoneyFile::instance();
  MyMoneyPayee user = file->user();

  KNewFileDlg newFileDlg(user.name(), user.address(),
    user.city(), user.state(), user.postcode(), user.telephone(),
    user.email(), this, "NewFileDlg", i18n("Edit Personal Data"));

  if (newFileDlg.exec() == QDialog::Accepted)
  {
    user.setName(newFileDlg.userNameText);
    user.setAddress(newFileDlg.userStreetText);
    user.setCity(newFileDlg.userTownText);
    user.setState(newFileDlg.userCountyText);
    user.setPostcode(newFileDlg.userPostcodeText);
    user.setTelephone(newFileDlg.userTelephoneText);
    user.setEmail(newFileDlg.userEmailText);
    MyMoneyFileTransaction ft;
    try {
      file->setUser(user);
      ft.commit();
    } catch(MyMoneyException *e) {
      KMessageBox::information(this, i18n("Unable to store user information: %1").arg(e->what()));
      delete e;
    }
  }
}

void KMyMoney2App::slotFileFileInfo(void)
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
  KMSTATUS(i18n("Importing account templates."));

  int rc;
  KLoadTemplateDlg* dlg = new KLoadTemplateDlg();
  if((rc = dlg->exec()) == QDialog::Accepted) {
    MyMoneyFileTransaction ft;
    try {
    // import the account templates
      QValueList<MyMoneyTemplate> templates = dlg->templates();
      QValueList<MyMoneyTemplate>::iterator it_t;
      for(it_t = templates.begin(); it_t != templates.end(); ++it_t) {
        (*it_t).importTemplate(&progressCallback);
      }
      ft.commit();
    } catch(MyMoneyException* e) {
      KMessageBox::detailedSorry(0, i18n("Error"), i18n("Unable to import template(s): %1, thrown in %2:%3").arg(e->what()).arg(e->file()).arg(e->line()));
      delete e;
    }
  }
  delete dlg;
}

void KMyMoney2App::slotSaveAccountTemplates(void)
{
  KMSTATUS(i18n("Exporting account templates."));

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

    if(okToWriteFile(newName)) {
      MyMoneyTemplate templ;
      templ.exportTemplate(&progressCallback);
      templ.saveTemplate(newName);
    }
  }
}

void KMyMoney2App::slotQifImport(void)
{
  if(m_qifReader == 0) {
    // FIXME: the menu entry for qif import should be disabled here

    KImportDlg* dlg = new KImportDlg(0);

    if(dlg->exec()) {
      KMSTATUS(i18n("Importing file..."));
      m_qifReader = new MyMoneyQifReader;
      connect(m_qifReader, SIGNAL(importFinished()), this, SLOT(slotQifImportFinished()));

      m_qifReader->setURL(dlg->filename());

      m_qifReader->setProfile(dlg->profile());
      m_qifReader->setAutoCreatePayee(dlg->autoCreatePayee());
      m_qifReader->setProgressCallback(&progressCallback);

      // disable all standard widgets during the import
      setEnabled(false);

      m_qifReader->startImport();
    }
    delete dlg;

    slotUpdateActions();
  }
}

void KMyMoney2App::slotQifImportFinished(void)
{
  if(m_qifReader != 0) {
    m_qifReader->finishImport();
#if 0
    // fixme: re-enable the QIF import menu options
    if(m_qifReader->finishImport()) {
      if(verifyImportedData(m_qifReader->account())) {
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
#endif

    // update the views as they might still contain invalid data
    // from the import session. The same applies for the window caption
    myMoneyView->slotRefreshViews();
    updateCaption();

    delete m_qifReader;
    m_qifReader = 0;
  }
  slotStatusProgressBar(-1, -1);
  ready();

  // re-enable all standard widgets
  setEnabled(true);
  slotUpdateActions();
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

  KMSTATUS(i18n("Importing a Gnucash file."));

  KFileDialog* dialog = new KFileDialog(KGlobalSettings::documentPath(),
                            i18n("%1|Gnucash files\n%2|All files (*.*)").arg("*").arg("*"),
                            this, i18n("Import Gnucash file..."), true);
  dialog->setMode(KFile::File | KFile::ExistingOnly);

  if(dialog->exec() == QDialog::Accepted) {
//    slotFileClose();
//    if(myMoneyView->fileOpen())
//      return;

    // call the importer
    myMoneyView->readFile(dialog->selectedURL());
    // imported files don't have a name
    m_fileName = KURL();

    updateCaption();
    emit fileLoaded(m_fileName);
  }
  delete dialog;

}

void KMyMoney2App::slotAccountChart(void)
{
#ifdef HAVE_KDCHART
  if(!m_selectedAccount.id().isEmpty()) {
    KBalanceChartDlg dlg(m_selectedAccount, this);
    dlg.exec();
  }
#endif
}


//
// KMyMoney2App::slotStatementImport() is for testing only.  The MyMoneyStatement
// is not intended to be exposed to users in XML form.
//

void KMyMoney2App::slotStatementImport(void)
{
  bool result = false;
  KMSTATUS(i18n("Importing an XML Statement."));

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

    }*/
  }
  delete dialog;

  if ( !result )
  {
    // re-enable all standard widgets
    setEnabled(true);
  }
}

bool KMyMoney2App::slotStatementImport(const QString& url)
{
  bool result = false;
  MyMoneyStatement s;
  if ( MyMoneyStatement::readXMLFile( s, url ) )
    result = slotStatementImport(s);
  else
    KMessageBox::error(this, i18n("Error importing %1: This file is not a valid KMM statement file.").arg(url), i18n("Invalid Statement"));

  return result;
}

bool KMyMoney2App::slotStatementImport(const MyMoneyStatement& s)
{
  bool result = false;

  // keep a copy of the statement
  MyMoneyStatement::writeXMLFile(s, "/home/thb/kmm-statement.txt");

  // we use an object on the heap here, so that we can check the presence
  // of it during slotUpdateActions() by looking at the pointer.
  m_smtReader = new MyMoneyStatementReader;
  m_smtReader->setAutoCreatePayee(true);
  m_smtReader->setProgressCallback(&progressCallback);

  // disable all standard widgets during the import
  setEnabled(false);

  result = m_smtReader->import(s);

  // get rid of the statement reader and tell everyone else
  // about the destruction by setting the pointer to zero
  delete m_smtReader;
  m_smtReader = 0;

  slotStatusProgressBar(-1, -1);
  ready();

  // re-enable all standard widgets
  setEnabled(true);

  return result;
}

void KMyMoney2App::slotQifExport(void)
{
  KMSTATUS(i18n("Exporting file..."));

  KExportDlg* dlg = new KExportDlg(0);

  if(dlg->exec()) {
    if(okToWriteFile(dlg->filename())) {
      MyMoneyQifWriter writer;
      connect(&writer, SIGNAL(signalProgress(int, int)), this, SLOT(slotStatusProgressBar(int, int)));

      writer.write(dlg->filename(), dlg->profile(), dlg->accountId(),
            dlg->accountSelected(), dlg->categorySelected(),
            dlg->startDate(), dlg->endDate());
    }
  }
  delete dlg;
}

bool KMyMoney2App::okToWriteFile(const KURL& url)
{
  // check if the file exists and warn the user
  bool reallySaveFile = true;

  if(KIO::NetAccess::exists(url, true, this)) {
    if(KMessageBox::warningYesNo(this, QString("<qt>")+i18n("The file <b>%1</b> already exists. Do you really want to override it?").arg(url.prettyURL(0, KURL::StripFileProtocol))+QString("</qt>"), i18n("File already exists")) != KMessageBox::Yes)
      reallySaveFile = false;
  }
  return reallySaveFile;
}

void KMyMoney2App::slotSettings(void)
{
  // if we already have an instance of the settings dialog, then use it
  if(KConfigDialog::showDialog("KMyMoney-Settings"))
    return;

  // otherwise, we have to create it
  KConfigDialog* dlg = new KConfigDialog(this, "KMyMoney-Settings", KMyMoneyGlobalSettings::self(),
    KDialogBase::IconList, KDialogBase::Default | KDialogBase::Ok | KDialogBase::Cancel | KDialogBase::Help, KDialogBase::Ok, true);

  // create the pages ...
  KSettingsGeneral* generalPage = new KSettingsGeneral();
  KSettingsRegister* registerPage = new KSettingsRegister();
  KSettingsHome* homePage = new KSettingsHome();
  KSettingsSchedules* schedulesPage = new KSettingsSchedules();
  KSettingsGpg* encryptionPage = new KSettingsGpg();
  KSettingsColors* colorsPage = new KSettingsColors();
  KSettingsFonts* fontsPage = new KSettingsFonts();
  KSettingsOnlineQuotes* onlineQuotesPage = new KSettingsOnlineQuotes();
  KSettingsForecast* forecastPage = new KSettingsForecast();

  // ... and add them to the dialog
  dlg->addPage(generalPage, i18n("General"), "misc");
  dlg->addPage(registerPage, i18n("Register"), "ledger");
  dlg->addPage(homePage, i18n("Home"), "home");
  dlg->addPage(schedulesPage, i18n("Schedules"), "schedule");
  dlg->addPage(encryptionPage, i18n("Encryption"), "kgpg");
  dlg->addPage(colorsPage, i18n("Colors"), "colorscm");
  dlg->addPage(fontsPage, i18n("Fonts"), "font");
  dlg->addPage(onlineQuotesPage, i18n("Online Quotes"), "network_local");
  dlg->addPage(forecastPage, i18n("Forecast"), "forcast");

  connect(dlg, SIGNAL(settingsChanged()), this, SLOT(slotUpdateConfiguration()));

  dlg->show();
}

void KMyMoney2App::slotUpdateConfiguration(void)
{
  MyMoneyTransactionFilter::setFiscalYearStart(KMyMoneyGlobalSettings::firstFiscalMonth(), KMyMoneyGlobalSettings::firstFiscalDay());

  myMoneyView->slotRefreshViews();

  // re-read autosave configuration
  m_autoSaveEnabled = KMyMoneyGlobalSettings::autoSaveFile();
  m_autoSavePeriod = KMyMoneyGlobalSettings::autoSavePeriod();

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
bool KMyMoney2App::initWizard(void)
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
void KMyMoney2App::slotFileBackup(void)
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
void KMyMoney2App::slotProcessExited(void)
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

void KMyMoney2App::slotFileNewWindow(void)
{
  KMyMoney2App *newWin = new KMyMoney2App;

  newWin->show();
}

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

void KMyMoney2App::slotToolsStartKCalc(void)
{
  KRun::runCommand("kcalc");
}

void KMyMoney2App::slotToolsPluginDlg(void)
{
  if(d->m_pluginDlg) {
    d->m_pluginDlg->exec();
  }
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
  m_searchDlg->setActiveWindow();
}

void KMyMoney2App::slotCloseSearchDialog(void)
{
  if(m_searchDlg)
    m_searchDlg->deleteLater();
  m_searchDlg = 0;
}

void KMyMoney2App::createInstitution(MyMoneyInstitution& institution)
{
  MyMoneyFile* file = MyMoneyFile::instance();

  MyMoneyFileTransaction ft;

  try {
    file->addInstitution(institution);
    ft.commit();

  } catch (MyMoneyException *e) {
    KMessageBox::information(this, i18n("Cannot add institution: %1").arg(e->what()));
    delete e;
  }
}

void KMyMoney2App::slotInstitutionNew(void)
{
  MyMoneyInstitution institution;
  slotInstitutionNew(institution);
}

void KMyMoney2App::slotInstitutionNew(MyMoneyInstitution& institution)
{
  institution.clearId();
  KNewBankDlg dlg(institution);
  if (dlg.exec()) {
    institution = dlg.institution();
    createInstitution(institution);
  }
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
      MyMoneyFileTransaction ft;
      try {
        file->modifyInstitution(dlg.institution());
        ft.commit();
        slotSelectInstitution(dlg.institution());

      } catch(MyMoneyException *e) {
        KMessageBox::information(this, i18n("Unable to store institution: %1").arg(e->what()));
        delete e;
      }
    }

  } catch(MyMoneyException *e) {
    if(!obj.id().isEmpty())
      KMessageBox::information(this, i18n("Unable to edit institution: %1").arg(e->what()));
    delete e;
  }
}

void KMyMoney2App::slotInstitutionDelete(void)
{
  MyMoneyFile *file = MyMoneyFile::instance();
  try {

    MyMoneyInstitution institution = file->institution(m_selectedInstitution.id());
    if ((KMessageBox::questionYesNo(this, QString("<p>")+i18n("Do you really want to delete institution <b>%1</b> ?").arg(institution.name()))) == KMessageBox::No)
      return;
    MyMoneyFileTransaction ft;

    try {
      file->removeInstitution(institution);
      ft.commit();
    } catch (MyMoneyException *e) {
      KMessageBox::information(this, i18n("Unable to delete institution: %1").arg(e->what()));
      delete e;
    }
  } catch (MyMoneyException *e) {
    KMessageBox::information(this, i18n("Unable to delete institution: %1").arg(e->what()));
    delete e;
  }
}

void KMyMoney2App::createAccount(MyMoneyAccount& newAccount, MyMoneyAccount& parentAccount, MyMoneyAccount& brokerageAccount, MyMoneyMoney openingBal)
{
  MyMoneyFile* file = MyMoneyFile::instance();

  MyMoneyFileTransaction ft;
  try
  {
    int pos;
    // check for ':' in the name and use it as separator for a hierarchy
    while((pos = newAccount.name().find(MyMoneyFile::AccountSeperator)) != -1) {
      QString part = newAccount.name().left(pos);
      QString remainder = newAccount.name().mid(pos+1);
      const MyMoneyAccount& existingAccount = file->subAccountByName(parentAccount, part);
      if(existingAccount.id().isEmpty()) {
        newAccount.setName(part);

        file->addAccount(newAccount, parentAccount);
        parentAccount = newAccount;
      } else {
        parentAccount = existingAccount;
      }
      newAccount.setParentAccountId(QCString());  // make sure, there's no parent
      newAccount.clearId();                       // and no id set for adding
      newAccount.removeAccountIds();              // and no sub-account ids
      newAccount.setName(remainder);
    }

    const MyMoneySecurity& sec = file->security(newAccount.currencyId());
    // Check the opening balance
    if (openingBal.isPositive() && newAccount.accountGroup() == MyMoneyAccount::Liability)
    {
      QString message = i18n("This account is a liability and if the "
          "opening balance represents money owed, then it should be negative.  "
          "Negate the amount?\n\n"
          "Please click Yes to change the opening balance to %1,\n"
          "Please click No to leave the amount as %2,\n"
          "Please click Cancel to abort the account creation.")
          .arg((-openingBal).formatMoney(newAccount, sec))
          .arg(openingBal.formatMoney(newAccount, sec));

      int ans = KMessageBox::questionYesNoCancel(this, message);
      if (ans == KMessageBox::Yes) {
        openingBal = -openingBal;

      } else if (ans == KMessageBox::Cancel)
        return;
    }

    file->addAccount(newAccount, parentAccount);

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

      a.setShares(a.value());
      b.setValue(-a.value());
      b.setShares(b.value());
      a.setMemo(i18n("Loan payout"));
      b.setMemo(i18n("Loan payout"));
      t.setPostDate(QDate::fromString(acc.value("kmm-loan-payment-date"), Qt::ISODate));
      newAccount.deletePair("kmm-loan-payment-acc");
      newAccount.deletePair("kmm-loan-payment-date");
      MyMoneyFile::instance()->modifyAccount(newAccount);

      t.addSplit(a);
      t.addSplit(b);
      file->addTransaction(t);
      file->createOpeningBalanceTransaction(newAccount, openingBal);

    // in case of an investment account we check if we should create
    // a brokerage account
    } else if(newAccount.accountType() == MyMoneyAccount::Investment
            && !brokerageAccount.name().isEmpty()) {
      file->addAccount(brokerageAccount, parentAccount);

      // set a link from the investment account to the brokerage account
      newAccount.setValue("kmm-brokerage-account", brokerageAccount.id());
      file->modifyAccount(newAccount);
      file->createOpeningBalanceTransaction(brokerageAccount, openingBal);

    } else
      file->createOpeningBalanceTransaction(newAccount, openingBal);

    ft.commit();
  }
  catch (MyMoneyException *e)
  {
    KMessageBox::information(this, i18n("Unable to add account: %1").arg(e->what()));
    delete e;
  }
}

void KMyMoney2App::slotCategoryNew(const QString& name, QCString& id)
{
  MyMoneyAccount account;
  account.setName(name);

  slotCategoryNew(account, MyMoneyFile::instance()->expense());

  id = account.id();
}

void KMyMoney2App::slotCategoryNew(MyMoneyAccount& account, const MyMoneyAccount& parent)
{
  if(KMessageBox::questionYesNo(this,
    QString("<qt>%1</qt>").arg(i18n("The category <b>%1</b> currently does not exist. Do you want to create it?<p><i>The parent account will default to <b>%2</b> but can be changed in the following dialog</i>.").arg(account.name()).arg(parent.name())), i18n("Create category"),
    KStdGuiItem::yes(), KStdGuiItem::no(), "CreateNewCategories") == KMessageBox::Yes) {
    createCategory(account, parent);
  }
}

void KMyMoney2App::slotCategoryNew(void)
{
  MyMoneyAccount parent;
  MyMoneyAccount account;

  // Preselect the parent account by looking at the current selected account/category
  if(!m_selectedAccount.id().isEmpty() && m_selectedAccount.isIncomeExpense()) {
    MyMoneyFile* file = MyMoneyFile::instance();
    try {
      parent = file->account(m_selectedAccount.id());
    } catch(MyMoneyException *e) {
      delete e;
    }
  }

  createCategory(account, parent);
}

void KMyMoney2App::createCategory(MyMoneyAccount& account, const MyMoneyAccount& parent)
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

    createAccount(account, parentAccount, brokerageAccount, MyMoneyMoney(0,1));
  }
}

void KMyMoney2App::slotAccountNew(void)
{
  MyMoneyAccount acc;
  acc.setOpeningDate(QDate::currentDate());

  slotAccountNew(acc);
}

void KMyMoney2App::slotAccountNew(MyMoneyAccount& account)
{
  NewAccountWizard::Wizard* wizard = new NewAccountWizard::Wizard();
  connect(wizard, SIGNAL(createInstitution(MyMoneyInstitution&)), this, SLOT(slotInstitutionNew(MyMoneyInstitution&)));
  connect(wizard, SIGNAL(createAccount(MyMoneyAccount&)), this, SLOT(slotAccountNew(MyMoneyAccount&)));
  connect(wizard, SIGNAL(createPayee(const QString&, QCString&)), this, SLOT(slotPayeeNew(const QString&, QCString&)));
  connect(wizard, SIGNAL(createCategory(MyMoneyAccount&, const MyMoneyAccount&)), this, SLOT(slotCategoryNew(MyMoneyAccount&, const MyMoneyAccount&)));

  wizard->setAccount(account);

  if(wizard->exec() == QDialog::Accepted) {
    MyMoneyAccount acc = wizard->account();
    if(!(acc == MyMoneyAccount())) {
      MyMoneyFileTransaction ft;
      MyMoneyFile* file = MyMoneyFile::instance();
      try {
        // create the account
        MyMoneyAccount parent = wizard->parentAccount();
        file->addAccount(acc, parent);

        // tell the wizard about the accound id which it
        // needs to create a possible schedule and transactions
        wizard->setAccount(acc);

        // create the opening balance transaction if any
        file->createOpeningBalanceTransaction(acc, wizard->openingBalance());

        // create the payout transaction for loans if any
        MyMoneyTransaction payoutTransaction = wizard->payoutTransaction();
        if(payoutTransaction.splits().count() > 0) {
          file->addTransaction(payoutTransaction);
        }

        // create a brokerage account if selected
        MyMoneyAccount brokerageAccount = wizard->brokerageAccount();
        if(!(brokerageAccount == MyMoneyAccount())) {
          file->addAccount(brokerageAccount, parent);
        }

        // create a possible schedule
        MyMoneySchedule sch = wizard->schedule();
        if(!(sch == MyMoneySchedule())) {
          MyMoneyFile::instance()->addSchedule(sch);
          if(acc.isLoan()) {
            MyMoneyAccountLoan accLoan = MyMoneyFile::instance()->account(acc.id());
            accLoan.setSchedule(sch.id());
            acc = accLoan;
            MyMoneyFile::instance()->modifyAccount(acc);
          }
        }
        ft.commit();
        account = acc;
      } catch (MyMoneyException *e) {
        KMessageBox::error(this, i18n("Unable to create account: %1").arg(e->what()));
      }
    }
  }
  delete wizard;
}

void KMyMoney2App::slotInvestmentNew(MyMoneyAccount& account, const MyMoneyAccount& parent)
{
  if(KMessageBox::questionYesNo(this,
    QString("<qt>")+i18n("The security <b>%1</b> currently does not exist as sub-account of <b>%2</b>. "
          "Do you want to create it?").arg(account.name()).arg(parent.name())+QString("</qt>"), i18n("Create category"),
    KStdGuiItem::yes(), KStdGuiItem::no(), "CreateNewInvestments") == KMessageBox::Yes) {
    KNewInvestmentWizard dlg;
    if(dlg.exec() == QDialog::Accepted) {
      dlg.createObjects(parent.id());
      account = dlg.account();
    }
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
    MyMoneyFile* file = MyMoneyFile::instance();
    MyMoneyFileTransaction ft;
    try {
      file->removeAccount(m_selectedInvestment);
      ft.commit();
    } catch(MyMoneyException *e) {
      KMessageBox::information(this, i18n("Unable to delete investment: %1").arg(e->what()));
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
      signed64 fract = MyMoneyMoney::precToDenom(KMyMoneyGlobalSettings::pricePrecision());

      KCurrencyCalculator calc(security, currency, MyMoneyMoney(1,1), price.rate(currency.id()), price.date(), fract);
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
  MyMoneyFile* file = MyMoneyFile::instance();
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
#if 0
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
#endif

      MyMoneyFileTransaction ft;
      try {
        file->addSchedule(newSchedule);

        // in case of a loan account, we keep a reference to this
        // schedule in the account
        if(newAccount.accountType() == MyMoneyAccount::Loan
        || newAccount.accountType() == MyMoneyAccount::AssetLoan) {
          newAccount.setValue("schedule", newSchedule.id());
          file->modifyAccount(newAccount);
        }
        ft.commit();
      } catch (MyMoneyException *e) {
        KMessageBox::information(this, i18n("Unable to add schedule: "), e->what());
        delete e;
      }
    }
    catch (MyMoneyException *e)
    {
      KMessageBox::information(this, i18n("Unable to add schedule: "), e->what());
      delete e;
    }
  }
}

bool KMyMoney2App::exchangeAccountInTransaction(MyMoneyTransaction& _t, const QCString& fromId, const QCString& toId)
{
  bool rc = false;
  MyMoneyTransaction t(_t);
  QValueList<MyMoneySplit>::iterator it_s;
  for(it_s = t.splits().begin(); it_s != t.splits().end(); ++it_s) {
    if((*it_s).accountId() == fromId) {
      (*it_s).setAccountId(toId);
      _t.modifySplit(*it_s);
      rc = true;
    }
  }
  return rc;
}

void KMyMoney2App::slotAccountDelete(void)
{
  if (m_selectedAccount.id().isEmpty())
    return;  // need an account ID

  MyMoneyFile* file = MyMoneyFile::instance();
  // can't delete standard accounts or account which still have transactions assigned
  if (file->isStandardAccount(m_selectedAccount.id()))
    return;

  // check if the account is referenced by a transaction or schedule
  MyMoneyFileBitArray skip(IMyMoneyStorage::MaxRefCheckBits);
  skip.fill(false);
  skip.setBit(IMyMoneyStorage::RefCheckAccount);
  skip.setBit(IMyMoneyStorage::RefCheckInstitution);
  skip.setBit(IMyMoneyStorage::RefCheckPayee);
  skip.setBit(IMyMoneyStorage::RefCheckSecurity);
  skip.setBit(IMyMoneyStorage::RefCheckCurrency);
  skip.setBit(IMyMoneyStorage::RefCheckPrice);
  bool hasReference = file->isReferenced(m_selectedAccount, skip);

  // make sure we only allow transactions in a 'category' (income/expense account)
  switch(m_selectedAccount.accountType()) {
    case MyMoneyAccount::Income:
    case MyMoneyAccount::Expense:
      break;

    default:
      // if the account is still referenced
      if(hasReference) {
        return;
      }
      break;
  }

  // if we get here and still have transactions referencing the account, we
  // need to check with the user to possibly re-assign them to a different account
  bool needAskUser = true;
  bool exit = false;

  MyMoneyFileTransaction ft;

  if(hasReference) {
    // show transaction reassignment dialog

    needAskUser = false;
    KCategoryReassignDlg* dlg = new KCategoryReassignDlg(this);
    QCString categoryId = dlg->show(m_selectedAccount);
    delete dlg; // and kill the dialog
    if (categoryId.isEmpty())
      return; // the user aborted the dialog, so let's abort as well

    MyMoneyAccount newCategory = file->account(categoryId);
    try {
      {
        KMSTATUS(i18n("Adjusting transactions ..."));
        /*
          m_selectedAccount.id() is the old id, categoryId the new one
          Now search all transactions and schedules that reference m_selectedAccount.id()
          and replace that with categoryId.
        */
        // get the list of all transactions that reference the old account
        MyMoneyTransactionFilter filter(m_selectedAccount.id());
        filter.setReportAllSplits(false);
        QValueList<MyMoneyTransaction> tlist;
        QValueList<MyMoneyTransaction>::iterator it_t;
        file->transactionList(tlist, filter);

        slotStatusProgressBar(0, tlist.count());
        int cnt = 0;
        for(it_t = tlist.begin(); it_t != tlist.end(); ++it_t) {
          slotStatusProgressBar(++cnt, 0);
          MyMoneyTransaction t = (*it_t);
          if(exchangeAccountInTransaction(t, m_selectedAccount.id(), categoryId))
            file->modifyTransaction(t);
        }
        slotStatusProgressBar(tlist.count(), 0);
      }
      // now fix all schedules
      {
        KMSTATUS(i18n("Adjusting schedules ..."));
        QValueList<MyMoneySchedule> slist = file->scheduleList(m_selectedAccount.id());
        QValueList<MyMoneySchedule>::iterator it_s;

        int cnt = 0;
        slotStatusProgressBar(0, slist.count());
        for(it_s = slist.begin(); it_s != slist.end(); ++it_s) {
          slotStatusProgressBar(++cnt, 0);
          MyMoneyTransaction t = (*it_s).transaction();
          if(exchangeAccountInTransaction(t, m_selectedAccount.id(), categoryId)) {
            (*it_s).setTransaction(t);
            file->modifySchedule(*it_s);
          }
        }
        slotStatusProgressBar(slist.count(), 0);
      }
      // now fix all budgets
      {
        KMSTATUS(i18n("Adjusting budgets ..."));
        QValueList<MyMoneyBudget> blist = file->budgetList();
        QValueList<MyMoneyBudget>::const_iterator it_b;
        for(it_b = blist.begin(); it_b != blist.end(); ++it_b) {
          if((*it_b).hasReferenceTo(m_selectedAccount.id())) {
            MyMoneyBudget b = (*it_b);
            MyMoneyBudget::AccountGroup fromBudget = b.account(m_selectedAccount.id());
            MyMoneyBudget::AccountGroup toBudget = b.account(categoryId);
            toBudget += fromBudget;
            b.setAccount(toBudget, toBudget.id());
            b.removeReference(m_selectedAccount.id());
            file->modifyBudget(b);

          }
        }
        slotStatusProgressBar(blist.count(), 0);
      }
    } catch(MyMoneyException  *e) {
      KMessageBox::error( this, i18n("Unable to exchange category <b>%1</b> with category <b>%2</b>. Reason: %3").arg(m_selectedAccount.name()).arg(newCategory.name()).arg(e->what()));
      delete e;
      exit = true;
    }
    slotStatusProgressBar(-1, -1);
  }

  if(exit)
    return;

  // at this point, we must not have a reference to the account
  // to be deleted anymore
  switch(m_selectedAccount.accountGroup()) {
    // special handling for categories to allow deleting of empty subcategories
    case MyMoneyAccount::Income:
    case MyMoneyAccount::Expense:
    { // open a compound statement here to be able to declare variables
      // which would otherwise not work within a case label.

      // case A - only a single, unused category without subcats selected
      if (m_selectedAccount.accountList().isEmpty()) {
        if (!needAskUser || (KMessageBox::questionYesNo(this, QString("<qt>")+i18n("Do you really want to delete category <b>%1</b>?").arg(m_selectedAccount.name())+QString("</qt>")) == KMessageBox::Yes)) {
          try {
            file->removeAccount(m_selectedAccount);
            ft.commit();
            m_selectedAccount.clearId();
            slotUpdateActions();
          } catch(MyMoneyException* e) {
            KMessageBox::error( this, QString("<qt>")+i18n("Unable to delete category <b>%1</b>. Cause: %2").arg(m_selectedAccount.name()).arg(e->what())+QString("</qt>"));
            delete e;
          }
        }
        return;
      }
      // case B - we have some subcategories, maybe the user does not want to
      //          delete them all, but just the category itself?
      MyMoneyAccount parentAccount = file->account(m_selectedAccount.parentAccountId());

      QCStringList accountsToReparent;
      int result = KMessageBox::questionYesNoCancel(this, QString("<qt>")+
          i18n("Do you want to delete category <b>%1</b> with all its sub-categories or only "
               "the category itself? If you only delete the category itself, all its sub-categories "
               "will be made sub-categories of <b>%2</b>.").arg(m_selectedAccount.name()).arg(parentAccount.name())+QString("</qt>"),
          QString::null,
          KGuiItem(i18n("Delete all")),
          KGuiItem(i18n("Just the category")));
      if (result == KMessageBox::Cancel)
        return; // cancel pressed? ok, no delete then...
      // "No" means "Just the category" and that means we need to reparent all subaccounts
      bool need_confirmation = false;
      // case C - User only wants to delete the category itself
      if (result == KMessageBox::No)
        accountsToReparent = m_selectedAccount.accountList();
      else {
        // case D - User wants to delete all subcategories, now check all subcats of
        //          m_selectedAccount and remember all that cannot be deleted and
        //          must be "reparented"
        for (QCStringList::const_iterator it = m_selectedAccount.accountList().begin();
          it != m_selectedAccount.accountList().end(); ++it)
        {
          // reparent account if a transaction is assigned
          if (file->transactionCount(*it)!=0)
            accountsToReparent.push_back(*it);
          else if (!file->account(*it).accountList().isEmpty()) {
            // or if we have at least one sub-account that is used for transactions
            if (!file->hasOnlyUnusedAccounts(file->account(*it).accountList())) {
              accountsToReparent.push_back(*it);
              //kdDebug() << "subaccount not empty" << endl;
            }
          }
        }
        if (!accountsToReparent.isEmpty())
          need_confirmation = true;
      }
      if (!accountsToReparent.isEmpty() && need_confirmation) {
        if (KMessageBox::questionYesNo(this, QString("<p>")+i18n("Some sub-categories of category <b>%1</b> cannot "
          "be deleted, because they are still used. They will be made sub-categories of <b>%2</b>. Proceed?").arg(m_selectedAccount.name()).arg(parentAccount.name())) != KMessageBox::Yes) {
          return; // user gets wet feet...
        }
      }
      // all good, now first reparent selected sub-categories
      try {
        MyMoneyAccount parent = file->account(m_selectedAccount.parentAccountId());
        for (QCStringList::const_iterator it = accountsToReparent.begin(); it != accountsToReparent.end(); ++it) {
          MyMoneyAccount child = file->account(*it);
          file->reparentAccount(child, parent);
        }
        // reload the account because the sub-account list might have changed
        m_selectedAccount = file->account(m_selectedAccount.id());
        // now recursively delete remaining sub-categories
        file->removeAccountList(m_selectedAccount.accountList());
        // don't forget to update m_selectedAccount, because we still have a copy of
        // the old account list, which is no longer valid
        m_selectedAccount = file->account(m_selectedAccount.id());
      } catch(MyMoneyException* e) {
        KMessageBox::error( this, QString("<qt>")+i18n("Unable to delete a sub-category of category <b>%1</b>. Reason: %2").arg(m_selectedAccount.name()).arg(e->what())+QString("</qt>"));
        delete e;
        return;
      }
    }
    break; // the category/account is deleted after the switch

    default:
      if (!m_selectedAccount.accountList().isEmpty())
        return; // can't delete accounts which still have subaccounts

      if (KMessageBox::questionYesNo(this, QString("<p>")+i18n("Do you really want to "
          "delete account <b>%1</b>?").arg(m_selectedAccount.name())) != KMessageBox::Yes) {
        return; // ok, you don't want to? why did you click then, hmm?
      }
  } // switch;

  try {
    file->removeAccount(m_selectedAccount);
    m_selectedAccount.clearId();
    slotUpdateActions();
    ft.commit();
  } catch(MyMoneyException* e) {
    KMessageBox::error( this, i18n("Unable to delete account '%1'. Cause: %2").arg(m_selectedAccount.name()).arg(e->what()));
    delete e;
  }
}

void KMyMoney2App::slotAccountEdit(void)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  if(!m_selectedAccount.id().isEmpty()) {
    if(!file->isStandardAccount(m_selectedAccount.id())) {
      if(m_selectedAccount.accountType() != MyMoneyAccount::Loan
      && m_selectedAccount.accountType() != MyMoneyAccount::AssetLoan) {
        QString caption;
        bool category = false;
        switch(MyMoneyAccount::accountGroup(m_selectedAccount.accountType())) {
          default:
            caption = i18n("Edit account '%1'").arg(m_selectedAccount.name());
            break;

          case MyMoneyAccount::Expense:
          case MyMoneyAccount::Income:
            caption = i18n("Edit category '%1'").arg(m_selectedAccount.name());
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

        // check for online modules
        QMap<QString, KMyMoneyPlugin::OnlinePlugin *>::const_iterator it_plugin = m_onlinePlugins.end();
        const MyMoneyKeyValueContainer& kvp = m_selectedAccount.onlineBankingSettings();
        if(!kvp["provider"].isEmpty()) {
          // if we have an online provider for this account, we need to check
          // that we have the corresponding plugin. If that exists, we ask it
          // to provide an additional tab for the account editor.
          it_plugin = m_onlinePlugins.find(kvp["provider"]);
          if(it_plugin != m_onlinePlugins.end()) {
            QString name;
            QWidget *w = 0;
            w = (*it_plugin)->accountConfigTab(m_selectedAccount, name);
            dlg.addTab(w, name);
          }
        }

        if (dlg.exec() == QDialog::Accepted) {
          try {
            MyMoneyFileTransaction ft;

            MyMoneyAccount account = dlg.account();
            MyMoneyAccount parent = dlg.parentAccount();
            if(it_plugin != m_onlinePlugins.end()) {
              account.setOnlineBankingSettings((*it_plugin)->onlineBankingSettings(account.onlineBankingSettings()));
            }
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
              file->createOpeningBalanceTransaction(m_selectedAccount, bal);
            }

            ft.commit();

            slotSelectAccount(account);

          } catch(MyMoneyException* e) {
            KMessageBox::error( this, i18n("Unable to modify account '%1'. Cause: %2").arg(m_selectedAccount.name()).arg(e->what()));
            delete e;
          }
        }
      } else {
        KEditLoanWizard* wizard = new KEditLoanWizard(m_selectedAccount);
        connect(wizard, SIGNAL(newCategory(MyMoneyAccount&)), this, SLOT(slotCategoryNew(MyMoneyAccount&)));
        connect(wizard, SIGNAL(createPayee(const QString&, QCString&)), this, SLOT(slotPayeeNew(const QString&, QCString&)));
        if(wizard->exec() == QDialog::Accepted) {
          MyMoneySchedule sch = file->schedule(m_selectedAccount.value("schedule").latin1());
          if(!(m_selectedAccount == wizard->account())
          || !(sch == wizard->schedule())) {
            MyMoneyFileTransaction ft;
            try {
              file->modifyAccount(wizard->account());
              sch = wizard->schedule();
              try {
                file->schedule(sch.id());
                file->modifySchedule(sch);
                ft.commit();
              } catch (MyMoneyException *e) {
                try {
                  file->addSchedule(sch);
                  ft.commit();
                } catch (MyMoneyException *f) {
                  qDebug("Cannot add schedule: '%s'", f->what().data());
                  delete f;
                }
                delete e;
              }
            } catch(MyMoneyException *e) {
              qDebug("Unable to modify account %s: '%s'", m_selectedAccount.name().data(),
                  e->what().data());
              delete e;
            }
          }
        }
        delete wizard;
      }
    }
  }
}

void KMyMoney2App::slotAccountReconcileStart(void)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  MyMoneyAccount account;

  // we cannot reconcile standard accounts
  if(!file->isStandardAccount(m_selectedAccount.id())) {
    // check if we can reconcile this account
    // it make's sense for asset and liability accounts
    try {
      account = file->account(m_selectedAccount.id());
      // get rid of previous run.
      if(m_endingBalanceDlg)
        delete m_endingBalanceDlg;
      m_endingBalanceDlg = new KEndingBalanceDlg(account, this);
      if(account.isAssetLiability()) {
        connect(m_endingBalanceDlg, SIGNAL(createPayee(const QString&, QCString&)), this, SLOT(slotPayeeNew(const QString&, QCString&)));
        connect(m_endingBalanceDlg, SIGNAL(createCategory(MyMoneyAccount&, const MyMoneyAccount&)), this, SLOT(slotCategoryNew(MyMoneyAccount&, const MyMoneyAccount&)));

        if(m_endingBalanceDlg->exec() == QDialog::Accepted) {
          if(myMoneyView->startReconciliation(account, m_endingBalanceDlg->statementDate(), m_endingBalanceDlg->endingBalance())) {

            // check if the user requests us to create interest
            // or charge transactions.
            MyMoneyTransaction ti = m_endingBalanceDlg->interestTransaction();
            MyMoneyTransaction tc = m_endingBalanceDlg->chargeTransaction();
            MyMoneyFileTransaction ft;
            try {
              if(ti != MyMoneyTransaction()) {
                MyMoneyFile::instance()->addTransaction(ti);
              }
              if(tc != MyMoneyTransaction()) {
                MyMoneyFile::instance()->addTransaction(tc);
              }
              ft.commit();
            } catch(MyMoneyException *e) {
              qWarning("interest transaction not stored: '%s'", e->what().data());
              delete e;
            }

            m_reconciliationAccount = account;
            slotUpdateActions();
          }
        }
      }
    } catch(MyMoneyException *e) {
      delete e;
    }
  }
}

void KMyMoney2App::slotAccountReconcileFinish(void)
{
  MyMoneyFile* file = MyMoneyFile::instance();

  if(!m_reconciliationAccount.id().isEmpty()) {
    // retrieve list of all transactions that are not reconciled or cleared
    QValueList<QPair<MyMoneyTransaction, MyMoneySplit> > transactionList;
    MyMoneyTransactionFilter filter(m_reconciliationAccount.id());
    filter.addState(MyMoneyTransactionFilter::cleared);
    filter.addState(MyMoneyTransactionFilter::notReconciled);
    filter.setDateFilter(QDate(), m_endingBalanceDlg->statementDate());
    filter.setConsiderCategory(false);
    filter.setReportAllSplits(true);
    file->transactionList(transactionList, filter);

    MyMoneyMoney balance = MyMoneyFile::instance()->balance(m_reconciliationAccount.id(), m_endingBalanceDlg->statementDate());
    MyMoneyMoney actBalance, clearedBalance;
    actBalance = clearedBalance = balance;

    // walk the list of transactions to figure out the balance(s)
    QValueList<QPair<MyMoneyTransaction, MyMoneySplit> >::const_iterator it;
    for(it = transactionList.begin(); it != transactionList.end(); ++it) {
      if((*it).second.reconcileFlag() == MyMoneySplit::NotReconciled) {
        clearedBalance -= (*it).second.shares();
      }
    }

    if(m_endingBalanceDlg->endingBalance() != clearedBalance) {
      QString message = i18n("You are about to finish the reconciliation of this account with a difference between your bank statement and the transactions marked as cleared.\n"
                             "Are you sure you want to finish the reconciliation ?");
      if (KMessageBox::questionYesNo(this, message, i18n("Confirm end of reconciliation"), KStdGuiItem::yes(), KStdGuiItem::no()) == KMessageBox::No)
        return;
    }

    MyMoneyFileTransaction ft;

    // refresh object
    m_reconciliationAccount = file->account(m_reconciliationAccount.id());

    // Turn off reconciliation mode
    myMoneyView->finishReconciliation(m_reconciliationAccount);

    m_reconciliationAccount.setValue("lastStatementBalance", m_endingBalanceDlg->endingBalance().toString());
    m_reconciliationAccount.setValue("lastStatementDate", m_endingBalanceDlg->statementDate().toString(Qt::ISODate));

    m_reconciliationAccount.deletePair("lastReconciledBalance");
    m_reconciliationAccount.deletePair("statementBalance");
    m_reconciliationAccount.deletePair("statementDate");

    try {
      // update the account data
      file->modifyAccount(m_reconciliationAccount);

      /*
      // collect the list of cleared splits for this account
      filter.clear();
      filter.addAccount(m_reconciliationAccount.id());
      filter.addState(MyMoneyTransactionFilter::cleared);
      filter.setConsiderCategory(false);
      filter.setReportAllSplits(true);
      file->transactionList(transactionList, filter);
      */

      // walk the list of transactions/splits and mark the cleared ones as reconciled
      QValueList<QPair<MyMoneyTransaction, MyMoneySplit> >::iterator it;

      for(it = transactionList.begin(); it != transactionList.end(); ++it) {
        MyMoneySplit sp = (*it).second;
        // skip the ones that are not marked cleared
        if(sp.reconcileFlag() != MyMoneySplit::Cleared)
          continue;

        // always retrieve a fresh copy of the transaction because we
        // might have changed it already with another split
        MyMoneyTransaction t = file->transaction((*it).first.id());
        sp.setReconcileFlag(MyMoneySplit::Reconciled);
        sp.setReconcileDate(m_endingBalanceDlg->statementDate());
        t.modifySplit(sp);

        // update the engine ...
        file->modifyTransaction(t);

        // ... and the list
        (*it) = qMakePair(t, sp);
      }
      ft.commit();

      emit accountReconciled(m_reconciliationAccount,
                             m_endingBalanceDlg->statementDate(),
                             m_endingBalanceDlg->previousBalance(),
                             m_endingBalanceDlg->endingBalance(),
                             transactionList);

    } catch(MyMoneyException *e) {
      qDebug("Unexpected exception when setting cleared to reconcile");
      delete e;
    }
  }
  // Turn off reconciliation mode
  m_reconciliationAccount = MyMoneyAccount();
  slotUpdateActions();
}

void KMyMoney2App::slotAccountReconcilePostpone(void)
{
  MyMoneyFileTransaction ft;
  MyMoneyFile* file = MyMoneyFile::instance();

  if(!m_reconciliationAccount.id().isEmpty()) {
    // update the account data
    file->modifyAccount(m_reconciliationAccount);

    // Turn off reconciliation mode
    myMoneyView->finishReconciliation(m_reconciliationAccount);

    m_reconciliationAccount.setValue("lastReconciledBalance", m_endingBalanceDlg->previousBalance().toString());
    m_reconciliationAccount.setValue("statementBalance", m_endingBalanceDlg->endingBalance().toString());
    m_reconciliationAccount.setValue("statementDate", m_endingBalanceDlg->statementDate().toString(Qt::ISODate));

    try {
      file->modifyAccount(m_reconciliationAccount);
      ft.commit();
      m_reconciliationAccount = MyMoneyAccount();
      slotUpdateActions();
    } catch(MyMoneyException *e) {
      qDebug("Unexpected exception when setting last reconcile info into account");
      delete e;
      ft.rollback();
      m_reconciliationAccount = file->account(m_reconciliationAccount.id());
    }
  }
}

void KMyMoney2App::slotAccountOpen(const MyMoneyObject& obj)
{
  if(typeid(obj) != typeid(MyMoneyAccount))
    return;

  MyMoneyFile* file = MyMoneyFile::instance();
  QCString id = m_selectedAccount.id();

  // if the caller passed a non-empty object, we need to select that
  if(!obj.id().isEmpty()) {
    id = obj.id();
  }

  // we cannot reconcile standard accounts
  if(!file->isStandardAccount(id)) {
    // check if we can open this account
    // currently it make's sense for asset and liability accounts
    try {
      MyMoneyAccount account = file->account(id);
      myMoneyView->slotLedgerSelected(account.id());
    } catch(MyMoneyException *e) {
      delete e;
    }
  }
}

bool KMyMoney2App::canCloseAccount(const MyMoneyAccount& acc) const
{
  // balance must be zero
  if(!acc.balance().isZero())
    return false;

  // all children must be already closed
  QCStringList::const_iterator it_a;
  for(it_a = acc.accountList().begin(); it_a != acc.accountList().end(); ++it_a) {
    MyMoneyAccount a = MyMoneyFile::instance()->account(*it_a);
    if(!a.isClosed()) {
      return false;
    }
  }

  // there must be no unfinished schedule referencing the account
  QValueList<MyMoneySchedule> list = MyMoneyFile::instance()->scheduleList();
  QValueList<MyMoneySchedule>::const_iterator it_l;
  for(it_l = list.begin(); it_l != list.end(); ++it_l) {
    if((*it_l).isFinished())
      continue;
    if((*it_l).hasReferenceTo(acc.id()))
      return false;
  }
  return true;
}

void KMyMoney2App::slotAccountClose(void)
{
  MyMoneyAccount a;
  if(!m_selectedInvestment.id().isEmpty())
    a = m_selectedInvestment;
  else if(!m_selectedAccount.id().isEmpty())
    a = m_selectedAccount;
  if(a.id().isEmpty())
    return;  // need an account ID

  MyMoneyFileTransaction ft;
  try {
    a.setClosed(true);
    MyMoneyFile::instance()->modifyAccount(a);
    ft.commit();
    if(KMyMoneyGlobalSettings::hideClosedAccounts()) {
      KMessageBox::information(this, QString("<qt>")+i18n("You have closed this account. It remains in the system because you have transactions which still refer to it, but is not shown in the views. You can make it visible again by going to the View menu and selecting <b>Show all accounts</b> or by unselecting the <b>Don't show closed accounts</b> setting.")+QString("</qt>"), i18n("Information"), "CloseAccountInfo");
    }
  } catch(MyMoneyException* e) {
    delete e;
  }
}

void KMyMoney2App::slotAccountReopen(void)
{
  MyMoneyAccount a;
  if(!m_selectedInvestment.id().isEmpty())
    a = m_selectedInvestment;
  else if(!m_selectedAccount.id().isEmpty())
    a = m_selectedAccount;
  if(a.id().isEmpty())
    return;  // need an account ID

  MyMoneyFile* file = MyMoneyFile::instance();
  MyMoneyFileTransaction ft;
  try {
    while(a.isClosed()) {
      a.setClosed(false);
      file->modifyAccount(a);
      a = file->account(a.parentAccountId());
    }
    ft.commit();
  } catch(MyMoneyException* e) {
    delete e;
  }
}

void KMyMoney2App::slotReparentAccount(const MyMoneyAccount& _src, const MyMoneyInstitution& _dst)
{
  MyMoneyAccount src(_src);
  src.setInstitutionId(_dst.id());
  MyMoneyFileTransaction ft;
  try {
    MyMoneyFile::instance()->modifyAccount(src);
    ft.commit();
  } catch(MyMoneyException* e) {
    KMessageBox::sorry(this, QString("<p>")+i18n("<b>%1</b> cannot be moved to institution <b>%2</b>. Reason: %3").arg(src.name()).arg(_dst.name()).arg(e->what()));
    delete e;
  }
}

void KMyMoney2App::slotReparentAccount(const MyMoneyAccount& _src, const MyMoneyAccount& _dst)
{
  MyMoneyAccount src(_src);
  MyMoneyAccount dst(_dst);
  MyMoneyFileTransaction ft;
  try {
    MyMoneyFile::instance()->reparentAccount(src, dst);
    ft.commit();
  } catch(MyMoneyException* e) {
    KMessageBox::sorry(this, QString("<p>")+i18n("<b>%1</b> cannot be moved to <b>%2</b>. Reason: %3").arg(src.name()).arg(dst.name()).arg(e->what()));
    delete e;
  }
}

void KMyMoney2App::slotAccountTransactionReport(void)
{
  // Generate a transaction report that contains transactions for only the
  // currently selected account.
  if(!m_selectedAccount.id().isEmpty()) {
    MyMoneyReport report(
        MyMoneyReport::eAccount,
        MyMoneyReport::eQCnumber|MyMoneyReport::eQCpayee|MyMoneyReport::eQCcategory,
        MyMoneyTransactionFilter::yearToDate,
        false,
        i18n("%1 YTD Account Transactions").arg(m_selectedAccount.name()),
        i18n("Generated Report")
      );
    report.setGroup(i18n("Transactions"));
    report.addAccount(m_selectedAccount.id());

    myMoneyView->slotShowReport(report);
  }
}

void KMyMoney2App::slotScheduleNew(void)
{
  slotScheduleNew(MyMoneyTransaction());
}

void KMyMoney2App::slotScheduleNew(const MyMoneyTransaction& _t, MyMoneySchedule::occurenceE occurence)
{
  MyMoneySchedule schedule;
  schedule.setOccurence(occurence);

  // if the schedule is based on an existing transaction,
  // we take the post date and project it to the next
  // schedule in a month.
  if(_t != MyMoneyTransaction()) {
    MyMoneyTransaction t(_t);
    if(occurence != MyMoneySchedule::OCCUR_ONCE)
      t.setPostDate(schedule.nextPayment(t.postDate()));
    schedule.setTransaction(t);
  }

  KEditScheduleDlg dlg(schedule, this);
  TransactionEditor* transactionEditor = dlg.startEdit();
  if(transactionEditor) {
    if(dlg.exec() == QDialog::Accepted) {
      MyMoneyFileTransaction ft;
      try {
        schedule = dlg.schedule();
        MyMoneyFile::instance()->addSchedule(schedule);
        ft.commit();

      } catch (MyMoneyException *e) {
        KMessageBox::error(this, i18n("Unable to add schedule: %1").arg(e->what()), i18n("Add schedule"));
        delete e;
      }
    }
  }
  delete transactionEditor;
}

void KMyMoney2App::slotScheduleEdit(void)
{
  if (!m_selectedSchedule.id().isEmpty()) {
    try {
      MyMoneySchedule schedule = MyMoneyFile::instance()->schedule(m_selectedSchedule.id());

      KEditScheduleDlg* sched_dlg = 0;
      KEditLoanWizard* loan_wiz = 0;


      switch (schedule.type()) {
        case MyMoneySchedule::TYPE_BILL:
        case MyMoneySchedule::TYPE_DEPOSIT:
        case MyMoneySchedule::TYPE_TRANSFER:
          sched_dlg = new KEditScheduleDlg(schedule, this);
          m_transactionEditor = sched_dlg->startEdit();
          if(m_transactionEditor) {
            if(sched_dlg->exec() == QDialog::Accepted) {
              MyMoneyFileTransaction ft;
              try {
                MyMoneySchedule sched = sched_dlg->schedule();
                // Check whether the new Schedule Date
                // is at or before the lastPaymentDate
                // If it is, ask the user whether to clear the
                // lastPaymentDate
                const QDate& next = sched.nextDueDate();
                const QDate& last = sched.lastPayment();
                if ( next.isValid() && last.isValid() && next <= last ) {
                  // Entered a date effectively no later
                  // than previous payment.  Date would be
                  // updated automatically so we probably
                  // want to clear it.  Let's ask the user.
                  if(KMessageBox::questionYesNo(this, QString("<qt>")+i18n("You have entered a Schedule date of <b>%1</b>.  Because the schedule was last paid on <b>%2</b>, KMyMoney will automatically adjust the schedule date to the next date unless the last payment date is reset.  Do you want to reset the last payment date?").arg(KGlobal::locale()->formatDate(next, true)).arg(KGlobal::locale()->formatDate(last, true))+QString("</qt>"),i18n("Reset Last Payment Date" ), KStdGuiItem::yes(), KStdGuiItem::no()) == KMessageBox::Yes) {
                    sched.setLastPayment( QDate() );
                  }
                }
                MyMoneyFile::instance()->modifySchedule(sched);
                ft.commit();
              } catch (MyMoneyException *e) {
                KMessageBox::detailedSorry(this, i18n("Unable to modify schedule '%1'").arg(m_selectedSchedule.name()), e->what());
                delete e;
              }
            }
          }
          deleteTransactionEditor();
          delete sched_dlg;
          break;

        case MyMoneySchedule::TYPE_LOANPAYMENT:
          loan_wiz = new KEditLoanWizard(schedule.account(2));
          connect(loan_wiz, SIGNAL(newCategory(MyMoneyAccount&)), this, SLOT(slotCategoryNew(MyMoneyAccount&)));
          connect(loan_wiz, SIGNAL(createPayee(const QString&, QCString&)), this, SLOT(slotPayeeNew(const QString&, QCString&)));
          if (loan_wiz->exec() == QDialog::Accepted) {
            MyMoneyFileTransaction ft;
            try {
              MyMoneyFile::instance()->modifySchedule(loan_wiz->schedule());
              MyMoneyFile::instance()->modifyAccount(loan_wiz->account());
              ft.commit();
            } catch (MyMoneyException *e) {
              KMessageBox::detailedSorry(this, i18n("Unable to modify schedule '%1'").arg(m_selectedSchedule.name()), e->what());
              delete e;
            }
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
    MyMoneyFileTransaction ft;
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
      ft.commit();

    } catch (MyMoneyException *e) {
      KMessageBox::detailedSorry(this, i18n("Unable to remove schedule '%1'").arg(m_selectedSchedule.name()), e->what());
      delete e;
    }
  }
}

void KMyMoney2App::slotScheduleDuplicate(void)
{
  // since we may jump here via code, we have to make sure to react only
  // if the action is enabled
  if(kmymoney2->action("schedule_duplicate")->isEnabled()) {
    MyMoneySchedule sch = m_selectedSchedule;
    sch.clearId();
    sch.setName(i18n("Copy of schedulename", "Copy of %1").arg(sch.name()));

    MyMoneyFileTransaction ft;
    try {
      MyMoneyFile::instance()->addSchedule(sch);
      ft.commit();

      // select the new schedule in the view
      if(!m_selectedSchedule.id().isEmpty())
        myMoneyView->slotScheduleSelected(sch.id());

    } catch(MyMoneyException* e) {
      KMessageBox::detailedSorry(0, i18n("Error"), i18n("Unable to duplicate transaction(s): %1, thrown in %2:%3").arg(e->what()).arg(e->file()).arg(e->line()));
      delete e;
    }
  }
}

void KMyMoney2App::slotScheduleSkip(void)
{
  if (!m_selectedSchedule.id().isEmpty()) {
    try {
      MyMoneySchedule schedule = MyMoneyFile::instance()->schedule(m_selectedSchedule.id());
      if(!schedule.isFinished()) {
        if(!schedule.isFinished()) {
          if(schedule.occurence() != MyMoneySchedule::OCCUR_ONCE) {
            QDate next = schedule.nextDueDate();
            if(!schedule.isFinished() && (KMessageBox::questionYesNo(this, QString("<qt>")+i18n("Do you really want to skip the transaction of schedule <b>%1</b> on <b>%2</b>?").arg(schedule.name(), KGlobal::locale()->formatDate(next, true))+QString("</qt>"))) == KMessageBox::Yes) {
              MyMoneyFileTransaction ft;
              schedule.setLastPayment(next);
              schedule.setNextDueDate(schedule.nextPayment(next));
              MyMoneyFile::instance()->modifySchedule(schedule);
              ft.commit();
            }
          }
        }
      }
    } catch (MyMoneyException *e) {
      KMessageBox::detailedSorry(this, QString("<qt>")+i18n("Unable to skip schedule <b>%1</b>.").arg(m_selectedSchedule.name())+QString("</qt>"), e->what());
      delete e;
    }
  }
}

void KMyMoney2App::slotScheduleEnter(void)
{
  if (!m_selectedSchedule.id().isEmpty()) {
    try {
      MyMoneySchedule schedule = MyMoneyFile::instance()->schedule(m_selectedSchedule.id());
      enterSchedule(schedule);
    } catch (MyMoneyException *e) {
      KMessageBox::detailedSorry(this, i18n("Unknown schedule '%1'").arg(m_selectedSchedule.name()), e->what());
      delete e;
    }
  }
}

bool KMyMoney2App::enterSchedule(MyMoneySchedule& schedule, bool autoEnter)
{
  bool rc = false;
  if (!schedule.id().isEmpty()) {
    try {
      schedule = MyMoneyFile::instance()->schedule(schedule.id());
      QDate origDueDate = schedule.nextDueDate();

      KEnterScheduleDlg dlg(this, schedule);
      m_transactionEditor = dlg.startEdit();
      if(m_transactionEditor) {
        MyMoneyTransaction torig, taccepted;
        m_transactionEditor->createTransaction(torig, dlg.transaction(), schedule.transaction().splits()[0], true);
        // force actions to be available no matter what (will be updated according to the state during
        // slotTransactionsEnter or slotTransactionsCancel)
        kmymoney2->action("transaction_cancel")->setEnabled(true);
        kmymoney2->action("transaction_enter")->setEnabled(true);

        KConfirmManualEnterDlg::Action action = KConfirmManualEnterDlg::ModifyOnce;
        if(!autoEnter || !schedule.isFixed()) {
          for(;;) {
            if(dlg.exec() == QDialog::Accepted) {
              m_transactionEditor->createTransaction(taccepted, torig, torig.splits()[0], true);
              // make sure to suppress comparison of some data: postDate
              torig.setPostDate(taccepted.postDate());
              if(torig != taccepted) {
                KConfirmManualEnterDlg cdlg(schedule, this);
                cdlg.loadTransactions(torig, taccepted);
                if(cdlg.exec() == QDialog::Accepted) {
                  action = cdlg.action();
                  break;
                }
                // the user has choosen 'cancel' during confirmation,
                // we go back to the editor
                continue;
              }
            } else {
              if(autoEnter) {
                if(KMessageBox::warningYesNo(this, i18n("Are you sure you wish to stop this schedule from being entered into the register?\n\nKMyMoney will prompt you again next time it starts unless you manually enter it later.")) == KMessageBox::No) {
                  // the user has choosen 'No' for the above question,
                  // we go back to the editor
                  continue;
                }
              }
              slotTransactionsCancel();
            }
            break;
          }
        }

        // if we still have the editor around here, the user did not cancel
        if(m_transactionEditor) {
          MyMoneyFileTransaction ft;
          try {
            MyMoneyTransaction t;
            // add the new transaction
            switch(action) {
              case KConfirmManualEnterDlg::UseOriginal:
                // setup widgets with original transaction data
                m_transactionEditor->setTransaction(dlg.transaction(), dlg.transaction().splits()[0]);
                // and create a transaction based on that data
                taccepted = MyMoneyTransaction();
                m_transactionEditor->createTransaction(taccepted, dlg.transaction(), dlg.transaction().splits()[0], true);
                break;

              case KConfirmManualEnterDlg::ModifyAlways:
                schedule.setTransaction(taccepted);
                break;

              case KConfirmManualEnterDlg::ModifyOnce:
                break;
            }

            QCString newId;
            if(m_transactionEditor->enterTransactions(newId, false)) {
              if(!newId.isEmpty()) {
                MyMoneyTransaction t = MyMoneyFile::instance()->transaction(newId);
                schedule.setLastPayment(t.postDate());
              }
              schedule.setNextDueDate(schedule.nextPayment(origDueDate));
              MyMoneyFile::instance()->modifySchedule(schedule);
              rc = true;
              ft.commit();
            }
            deleteTransactionEditor();

          } catch (MyMoneyException *e) {
            KMessageBox::detailedSorry(this, i18n("Unable to enter transaction for schedule '%1'").arg(m_selectedSchedule.name()), e->what());
            delete e;
          }
        }
      }
    } catch (MyMoneyException *e) {
      KMessageBox::detailedSorry(this, i18n("Unable to enter transaction for schedule '%1'").arg(m_selectedSchedule.name()), e->what());
      delete e;
    }
  }
  return rc;
}

void KMyMoney2App::slotPayeeNew(const QString& newnameBase, QCString& id)
{
  bool doit = true;

  if(newnameBase != i18n("New Payee")) {
    // Ask the user if that is what he intended to do?
    QString msg = QString("<qt>") + i18n("Do you want to add <b>%1</b> as payer/receiver ?").arg(newnameBase) + QString("</qt>");

    if(KMessageBox::questionYesNo(this, msg, i18n("New payee/receiver"), KStdGuiItem::yes(), KStdGuiItem::no(), "NewPayee") == KMessageBox::No)
      doit = false;
  }

  if(doit) {
    MyMoneyFileTransaction ft;
    try {
      QString newname(newnameBase);
      // adjust name until a unique name has been created
      int count = 0;
      for(;;) {
        try {
          MyMoneyFile::instance()->payeeByName(newname);
          newname = QString("%1 [%2]").arg(newnameBase).arg(++count);
        } catch(MyMoneyException* e) {
          delete e;
          break;
        }
      }

      MyMoneyPayee p;
      p.setName(newname);
      MyMoneyFile::instance()->addPayee(p);
      id = p.id();
      ft.commit();
    } catch (MyMoneyException *e) {
      KMessageBox::detailedSorry(this, i18n("Unable to add payee"),
        QString("%1 thrown in %2:%3").arg(e->what()).arg(e->file()).arg(e->line()));
      delete e;
    }
  }
}

void KMyMoney2App::slotPayeeNew(void)
{
  QCString id;
  slotPayeeNew(i18n("New Payee"), id);

  // the callbacks should have made sure, that the payees view has been
  // updated already. So we search for the id in the list of items
  // and select it.
  emit payeeCreated(id);
}

bool KMyMoney2App::payeeInList(const QValueList<MyMoneyPayee>& list, const QCString& id) const
{
  bool rc = false;
  QValueList<MyMoneyPayee>::const_iterator it_p = list.begin();
  while(it_p != list.end()) {
    if((*it_p).id() == id) {
      rc = true;
      break;
    }
    ++it_p;
  }
  return rc;
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
    if(m_selectedPayees.find(*it_p) != m_selectedPayees.end()) {
      it_p = remainingPayees.erase(it_p);
    } else {
      ++it_p;
    }
  }

  // get confirmation from user
  QString prompt;
  if (m_selectedPayees.size() == 1)
    prompt = QString("<p>")+i18n("Do you really want to remove the payee <b>%1</b>?").arg(m_selectedPayees.front().name());
  else
    prompt = i18n("Do you really want to remove all selected payees?");

  if (KMessageBox::questionYesNo(this, prompt, i18n("Remove Payee"))==KMessageBox::No)
    return;

  MyMoneyFileTransaction ft;
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
        if(payeeInList(m_selectedPayees, (*s_it).payeeId())) {
          used_schedules.push_back(*it); // remember this schedule
          break;
        }
      }
    }
//     kdDebug() << "[KPayeesView::slotDeletePayee]  " << used_schedules.count() << " schedules use one of the selected payees" << endl;

    MyMoneyPayee newPayee;
    bool addToMatchList = false;
    // if at least one payee is still referenced, we need to reassign its transactions first
    if (!translist.isEmpty() || !used_schedules.isEmpty()) {
      // show error message if no payees remain
      if (remainingPayees.isEmpty()) {
        KMessageBox::sorry(this, i18n("At least one transaction/schedule is still referenced by a payee. "
          "Currently you have all payees selected. However, at least one payee must remain so "
          "that the transactions/schedules can be reassigned."));
        return;
      }

      // show transaction reassignment dialog
      KPayeeReassignDlg * dlg = new KPayeeReassignDlg(this);
      QCString payee_id = dlg->show(remainingPayees);
      addToMatchList = dlg->addToMatchList();
      delete dlg; // and kill the dialog
      if (payee_id.isEmpty())
        return; // the user aborted the dialog, so let's abort as well

      newPayee = file->payee(payee_id);

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
            if(payeeInList(m_selectedPayees, (*s_it).payeeId())) {
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
            if(payeeInList(m_selectedPayees, (*s_it).payeeId())) {
              (*s_it).setPayeeId(payee_id);
              trans.modifySplit(*s_it); // does not modify the list object 'splits'!
            }
          } // for - Splits
          // store transaction in current schedule
          (*it).setTransaction(trans);
          file->modifySchedule(*it);  // modify the schedule in the MyMoney engine
        } // for - Schedules
      } catch(MyMoneyException *e) {
        KMessageBox::detailedSorry(0, i18n("Unable to reassign payee of transaction/split"),
          (e->what() + " " + i18n("thrown in") + " " + e->file()+ ":%1").arg(e->line()));
        delete e;
      }
    } // if !translist.isEmpty()

    bool ignorecase;
    QStringList payeeNames;
    MyMoneyPayee::payeeMatchType matchType = newPayee.matchData(ignorecase, payeeNames);
    QStringList deletedPayeeNames;

    // now loop over all selected payees and remove them
    for (QValueList<MyMoneyPayee>::iterator it = m_selectedPayees.begin();
      it != m_selectedPayees.end(); ++it) {
      if(addToMatchList) {
        deletedPayeeNames << (*it).name();
      }
      file->removePayee(*it);
    }

    // if we initially have no matching turned on, we just ignore the case (default)
    if(matchType == MyMoneyPayee::matchDisabled)
      ignorecase = true;

    // update the destination payee if this was requested by the user
    if(addToMatchList && deletedPayeeNames.count() > 0) {
      // add new names to the list
      // TODO: it would be cool to somehow shrink the list to make better use
      //       of regular expressions at this point. For now, we leave this task
      //       to the user himeself.
      QStringList::const_iterator it_n;
      for(it_n = deletedPayeeNames.begin(); it_n != deletedPayeeNames.end(); ++it_n) {
        if(matchType == MyMoneyPayee::matchKey) {
          // make sure we really need it and it is not caught by an existing regexp
          QStringList::const_iterator it_k;
          for(it_k = payeeNames.begin(); it_k != payeeNames.end(); ++it_k) {
            QRegExp exp(*it_k, ignorecase);
            if(exp.search(*it_n) != -1)
              break;
          }
          if(it_k == payeeNames.end())
            payeeNames << QRegExp::escape(*it_n);
        } else if(payeeNames.contains(*it_n) == 0)
          payeeNames << QRegExp::escape(*it_n);
      }

      // and update the payee in the engine context
      // make sure to turn on matching for this payee in the right mode
      newPayee.setMatchData(MyMoneyPayee::matchKey, ignorecase, payeeNames);
      file->modifyPayee(newPayee);
    }
    ft.commit();

    // If we just deleted the payees, they sure don't exist anymore
    slotSelectPayees(QValueList<MyMoneyPayee>());

  } catch(MyMoneyException *e) {
    KMessageBox::detailedSorry(0, i18n("Unable to remove payee(s)"),
      (e->what() + " " + i18n("thrown in") + " " + e->file()+ ":%1").arg(e->line()));
    delete e;
  }
}

void KMyMoney2App::slotCurrencyNew(void)
{
  QString sid = KInputDialog::getText(i18n("New currency"), i18n("Enter ISO 4217 code for the new currency"), QString::null, 0, 0, 0, 0, ">AAA");
  if(!sid.isEmpty()) {
    QCString id(sid);
    MyMoneySecurity currency(id, i18n("New currency"));
    MyMoneyFileTransaction ft;
    try {
      MyMoneyFile::instance()->addCurrency(currency);
      ft.commit();
    } catch(MyMoneyException* e) {
      KMessageBox::sorry(this, i18n("Cannot create new currency. %1").arg(e->what()), i18n("New currency"));
      delete e;
    }
    emit currencyCreated(id);
  }
}

void KMyMoney2App::slotCurrencyRename(QListViewItem* item, int, const QString& txt)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  KMyMoneyListViewItem* p = static_cast<KMyMoneyListViewItem *>(item);

  try {
    if(txt != m_selectedCurrency.name()) {
      MyMoneySecurity currency = file->currency(p->id());
      currency.setName(txt);
      MyMoneyFileTransaction ft;
      try {
        file->modifyCurrency(currency);
        m_selectedCurrency = currency;
        ft.commit();
      } catch(MyMoneyException* e) {
        KMessageBox::sorry(this, i18n("Cannot rename currency. %1").arg(e->what()), i18n("Rename currency"));
        delete e;
      }
    }
  } catch(MyMoneyException *e) {
    KMessageBox::sorry(this, i18n("Cannot rename currency. %1").arg(e->what()), i18n("Rename currency"));
    delete e;
  }
}

void KMyMoney2App::slotCurrencyDelete(void)
{
  if(!m_selectedCurrency.id().isEmpty()) {
    MyMoneyFileTransaction ft;
    try {
      MyMoneyFile::instance()->removeCurrency(m_selectedCurrency);
      ft.commit();
    } catch(MyMoneyException* e) {
      KMessageBox::sorry(this, i18n("Cannot delete currency %1. %2").arg(m_selectedCurrency.name()).arg(e->what()), i18n("Delete currency"));
      delete e;
    }
  }
}

void KMyMoney2App::slotCurrencySetBase(void)
{
  if(!m_selectedCurrency.id().isEmpty()) {
    if(m_selectedCurrency.id() != MyMoneyFile::instance()->baseCurrency().id()) {
      MyMoneyFileTransaction ft;
      try {
        MyMoneyFile::instance()->setBaseCurrency(m_selectedCurrency);
        ft.commit();
      } catch(MyMoneyException *e) {
        KMessageBox::sorry(this, i18n("Cannot set %1 as base currency: %2").arg(m_selectedCurrency.name()).arg(e->what()), i18n("Set base currency"));
        delete e;
      }
    }
  }
}

void KMyMoney2App::slotBudgetNew(void)
{
  QDate date = QDate::currentDate(Qt::LocalTime);
  date.setYMD(date.year(), 1, 1);
  QString newname = i18n("Budget %1").arg(QString::number(date.year()));

  MyMoneyBudget budget;

  // make sure we have a unique name
  try {
    int i=1;
    // Exception thrown when the name is not found
    while (1) {
      MyMoneyFile::instance()->budgetByName(newname);
      newname = i18n("Budget %1 (%2)").arg(QString::number(date.year()), QString::number(i++));
    }
  } catch(MyMoneyException *e) {
    // all ok, the name is unique
    delete e;
  }

  MyMoneyFileTransaction ft;
  try {
    budget.setName(newname);
    budget.setBudgetStart(date);

    MyMoneyFile::instance()->addBudget(budget);
    ft.commit();
  } catch(MyMoneyException *e) {
    KMessageBox::detailedSorry(0, i18n("Error"), i18n("Unable to add budget: %1, thrown in %2:%3").arg(e->what()).arg(e->file()).arg(e->line()));
    delete e;
  }
}

void KMyMoney2App::slotBudgetDelete(void)
{
  if(m_selectedBudgets.isEmpty())
    return; // shouldn't happen

  MyMoneyFile * file = MyMoneyFile::instance();

  // get confirmation from user
  QString prompt;
  if (m_selectedBudgets.size() == 1)
    prompt = QString("<p>")+i18n("Do you really want to remove the budget <b>%1</b>?").arg(m_selectedBudgets.front().name());
  else
    prompt = i18n("Do you really want to remove all selected budgets?");

  if (KMessageBox::questionYesNo(this, prompt, i18n("Remove Budget"))==KMessageBox::No)
    return;

  MyMoneyFileTransaction ft;
  try {
    // now loop over all selected budgets and remove them
    for (QValueList<MyMoneyBudget>::iterator it = m_selectedBudgets.begin();
      it != m_selectedBudgets.end(); ++it) {
      file->removeBudget(*it);
    }
    ft.commit();

  } catch(MyMoneyException *e) {
    KMessageBox::detailedSorry(0, i18n("Error"), i18n("Unable to remove budget: %1, thrown in %2:%3").      arg(e->what()).arg(e->file()).arg(e->line()));
    delete e;
  }
}

void KMyMoney2App::slotBudgetCopy(void)
{
  if(m_selectedBudgets.size() == 1) {
    MyMoneyFileTransaction ft;
    try {
      MyMoneyBudget budget = m_selectedBudgets[0];
      budget.clearId();
      budget.setName(i18n("Copy of %1").arg(budget.name()));

      MyMoneyFile::instance()->addBudget(budget);
      ft.commit();
    } catch(MyMoneyException *e) {
      KMessageBox::detailedSorry(0, i18n("Error"), i18n("Unable to add budget: %1, thrown in %2:%3").arg(e->what()).arg(e->file()).arg(e->line()));
      delete e;
    }
  }
}

void KMyMoney2App::slotBudgetChangeYear(void)
{
  if(m_selectedBudgets.size() == 1) {
    QStringList years;
    int current = 0;
    bool haveCurrent = false;
    MyMoneyBudget budget = *(m_selectedBudgets.begin());
    for(int i = (QDate::currentDate().year()-3); i < (QDate::currentDate().year()+5); ++i) {
      years << QString("%1").arg(i);
      if(i == budget.budgetStart().year()) {
        haveCurrent = true;
      }
      if(!haveCurrent)
        ++current;
    }
    if(!haveCurrent)
      current = 0;
    bool ok = false;

    QString yearString = KInputDialog::getItem(i18n("Select year"), i18n("Budget year"), years, current, false, &ok, this);

    if(ok) {
      int year = yearString.toInt(0, 0);
      QDate newYear = QDate(year, 1, 1);
      if(newYear != budget.budgetStart()) {
        MyMoneyFileTransaction ft;
        try {
          budget.setBudgetStart(newYear);
          MyMoneyFile::instance()->modifyBudget(budget);
          ft.commit();
        } catch(MyMoneyException *e) {
          KMessageBox::detailedSorry(0, i18n("Error"), i18n("Unable to modify budget: %1, thrown in %2:%3").      arg(e->what()).arg(e->file()).arg(e->line()));
          delete e;
        }
      }
    }
  }
}

void KMyMoney2App::slotBudgetForecast(void)
{
  if(m_selectedBudgets.size() == 1) {
    MyMoneyFileTransaction ft;
    try {
      MyMoneyBudget budget = m_selectedBudgets[0];
      bool calcBudget = budget.getaccounts().count() == 0;
      if(!calcBudget) {
        if(KMessageBox::warningContinueCancel(0, i18n("The current budget already contains data. Continuing will replace all current values of this budget."), i18n("Warning")) == KMessageBox::Continue)
          calcBudget = true;
      }

      if(calcBudget) {
        QDate historyStart;
        QDate historyEnd;
        QDate budgetStart;
        QDate budgetEnd;

        budgetStart = budget.budgetStart();
        budgetEnd = budgetStart.addYears(1).addDays(-1);
        historyStart = budgetStart.addYears(-1);
        historyEnd = budgetEnd.addYears(-1);

        MyMoneyForecast forecast;
        forecast.createBudget(budget, historyStart, historyEnd, budgetStart, budgetEnd, true);

        MyMoneyFile::instance()->modifyBudget(budget);
        ft.commit();
      }
    } catch(MyMoneyException *e) {
      KMessageBox::detailedSorry(0, i18n("Error"), i18n("Unable to modify budget: %1, thrown in %2:%3").arg(e->what()).arg(e->file()).arg(e->line()));
      delete e;
    }
  }
}

void KMyMoney2App::slotKDELanguageSettings(void)
{
  KMessageBox::information(this, i18n("Please be aware that changes made in the following dialog affect all KDE applications not only KMyMoney."), i18n("Warning"), "LanguageSettingsWarning");

  QStringList args;
  args << "language";
  QString error;
  int pid;

  KApplication::kdeinitExec("kcmshell", args, &error, &pid);
}

void KMyMoney2App::slotNewFeature(void)
{
  // slotStatementImport();
#if 0
  if(m_selectedBudgets.size() == 1) {
    MyMoneyFileTransaction ft;
    try {
      MyMoneyBudget budget = m_selectedBudgets[0];
      bool calcBudget = budget.getaccounts().count() == 0;
      if(!calcBudget) {
        if(KMessageBox::warningContinueCancel(0, i18n("The current budget already contains data. Continuing will replace all current values of this budget."), i18n("Warning")) == KMessageBox::Continue)
          calcBudget = true;
      }

      if(calcBudget) {
        QDate historyStart;
        QDate historyEnd;
        QDate budgetStart;
        QDate budgetEnd;

        budgetStart = budget.budgetStart();
        budgetEnd = budgetStart.addYears(1).addDays(-1);
        historyStart = budgetStart.addYears(-1);
        historyEnd = budgetEnd.addYears(-1);

        MyMoneyForecast forecast;
        budget = forecast.createBudget (historyStart, historyEnd, budgetStart, budgetEnd, true);

        budget.setName(m_selectedBudgets[0].name());
        MyMoneyFile::instance()->removeBudget(m_selectedBudgets[0]);
        MyMoneyFile::instance()->addBudget(budget);
        ft.commit();
      }
    } catch(MyMoneyException *e) {
      KMessageBox::detailedSorry(0, i18n("Error"), i18n("Unable to modify budget: %1, thrown in %2:%3").arg(e->what()).arg(e->file()).arg(e->line()));
      delete e;
    }
  }
#endif
}

void KMyMoney2App::slotTransactionsDelete(void)
{
  // since we may jump here via code, we have to make sure to react only
  // if the action is enabled
  if(!kmymoney2->action("transaction_delete")->isEnabled())
    return;
  if(m_selectedTransactions.count() == 0)
    return;
  if(m_selectedTransactions.warnLevel() == 1) {
    if(KMessageBox::warningContinueCancel(0,
        i18n(
            "At least one split of the selected transactions has been reconciled. "
            "Do you wish to delete the transactions anyway?"
            ),
            i18n("Transaction already reconciled"), KStdGuiItem::cont(),
                "DeleteReconciledTransaction") == KMessageBox::Cancel)
                 return;
  }
  QString msg;
  if(m_selectedTransactions.count() == 1) {
    msg = i18n("Do you really want to delete the selected transaction?");
  } else {
    msg = i18n("Do you really want to delete all %1 selected transactions?").arg(m_selectedTransactions.count());
  }
  if(KMessageBox::questionYesNo(this, msg, i18n("Delete transaction")) == KMessageBox::Yes) {
    KMSTATUS(i18n("Deleting transactions"));
    doDeleteTransactions();
  }
}

void KMyMoney2App::slotTransactionDuplicate(void)
{
  // since we may jump here via code, we have to make sure to react only
  // if the action is enabled
  if(kmymoney2->action("transaction_duplicate")->isEnabled()) {
    KMyMoneyRegister::SelectedTransactions list = m_selectedTransactions;
    KMyMoneyRegister::SelectedTransactions::iterator it_t;

    int i = 0;
    int cnt = m_selectedTransactions.count();
    KMSTATUS(i18n("Duplicating transactions"));
    slotStatusProgressBar(0, cnt);
    MyMoneyFileTransaction ft;
    MyMoneyTransaction lt;
    try {
      for(it_t = list.begin(); it_t != list.end(); ++it_t) {
        MyMoneyTransaction t = (*it_t).transaction();
        QValueList<MyMoneySplit>::iterator it_s;
        // wipe out any reconciliation information
        for(it_s = t.splits().begin(); it_s != t.splits().end(); ++it_s) {
          (*it_s).setReconcileFlag(MyMoneySplit::NotReconciled);
          (*it_s).setReconcileDate(QDate());
          (*it_s).setBankID(QString());
        }
        // clear invalid data
        t.setEntryDate(QDate());
        t.clearId();
        // and set the post date to today
        t.setPostDate(QDate::currentDate());

        MyMoneyFile::instance()->addTransaction(t);
        lt = t;
        slotStatusProgressBar(i++, 0);
      }
      ft.commit();

      // select the new transaction in the ledger
      if(!m_selectedAccount.id().isEmpty())
        myMoneyView->slotLedgerSelected(m_selectedAccount.id(), lt.id());

    } catch(MyMoneyException* e) {
      KMessageBox::detailedSorry(0, i18n("Error"), i18n("Unable to duplicate transaction(s): %1, thrown in %2:%3").arg(e->what()).arg(e->file()).arg(e->line()));
      delete e;
    }
    // switch off the progress bar
    slotStatusProgressBar(-1, -1);
  }
}

void KMyMoney2App::doDeleteTransactions(void)
{
  KMyMoneyRegister::SelectedTransactions list = m_selectedTransactions;
  KMyMoneyRegister::SelectedTransactions::const_iterator it_t;
  int cnt = list.count();
  int i = 0;
  slotStatusProgressBar(0, cnt);
  MyMoneyFileTransaction ft;
  try {
    for(it_t = list.begin(); it_t != list.end(); ++it_t) {
      MyMoneyFile::instance()->removeTransaction((*it_t).transaction());
      slotStatusProgressBar(i++, 0);
    }
    ft.commit();
  } catch(MyMoneyException* e) {
      KMessageBox::detailedSorry(0, i18n("Error"), i18n("Unable to delete transaction(s): %1, thrown in %2:%3").arg(e->what()).arg(e->file()).arg(e->line()));
    delete e;
  }
  slotStatusProgressBar(-1, -1);
}

void KMyMoney2App::slotTransactionsNew(void)
{
  // since we jump here via code, we have to make sure to react only
  // if the action is enabled
  if(kmymoney2->action("transaction_new")->isEnabled()) {
    if(myMoneyView->createNewTransaction()) {
      m_transactionEditor = myMoneyView->startEdit(m_selectedTransactions);
      if(m_transactionEditor) {
        connect(m_transactionEditor, SIGNAL(statusProgress(int, int)), this, SLOT(slotStatusProgressBar(int, int)));
        connect(m_transactionEditor, SIGNAL(statusMsg(const QString&)), this, SLOT(slotStatusMsg(const QString&)));
        connect(m_transactionEditor, SIGNAL(scheduleTransaction(const MyMoneyTransaction&, MyMoneySchedule::occurenceE)), this, SLOT(slotScheduleNew(const MyMoneyTransaction&, MyMoneySchedule::occurenceE)));
      }
      slotUpdateActions();
    }
  }
}

void KMyMoney2App::slotTransactionsEdit(void)
{
  // qDebug("KMyMoney2App::slotTransactionsEdit()");
  // since we jump here via code, we have to make sure to react only
  // if the action is enabled
  if(kmymoney2->action("transaction_edit")->isEnabled()) {
    m_transactionEditor = myMoneyView->startEdit(m_selectedTransactions);
    slotUpdateActions();
  }
}

void KMyMoney2App::deleteTransactionEditor(void)
{
  // make sure, we don't use the transaction editor pointer
  // anymore from now on
  TransactionEditor* p = m_transactionEditor;
  m_transactionEditor = 0;
  delete p;
}

void KMyMoney2App::slotTransactionsEditSplits(void)
{
  // since we jump here via code, we have to make sure to react only
  // if the action is enabled
  if(kmymoney2->action("transaction_editsplits")->isEnabled()) {
    m_transactionEditor = myMoneyView->startEdit(m_selectedTransactions);
    slotUpdateActions();

    if(m_transactionEditor) {
      if(m_transactionEditor->slotEditSplits() == QDialog::Accepted) {
        MyMoneyFileTransaction ft;
        try {
          QCString id;
          m_transactionEditor->enterTransactions(id);
          ft.commit();
        } catch(MyMoneyException* e) {
          KMessageBox::detailedSorry(0, i18n("Error"), i18n("Unable to modify transaction: %1, thrown in %2:%3").arg(e->what()).arg(e->file()).arg(e->line()));
          delete e;
        }
      }
    }
    deleteTransactionEditor();
    slotUpdateActions();
  }
}

void KMyMoney2App::slotTransactionsCancel(void)
{
  // since we jump here via code, we have to make sure to react only
  // if the action is enabled
  if(kmymoney2->action("transaction_cancel")->isEnabled()) {
    // make sure, we block the enter function
    action("transaction_enter")->setEnabled(false);
    // qDebug("KMyMoney2App::slotTransactionsCancel");
    deleteTransactionEditor();
    slotUpdateActions();
  }
}

void KMyMoney2App::slotTransactionsEnter(void)
{
  // since we jump here via code, we have to make sure to react only
  // if the action is enabled
  if(kmymoney2->action("transaction_enter")->isEnabled()) {
    // qDebug("KMyMoney2App::slotTransactionsEnter");
    if(m_transactionEditor) {
      QCString accountId = m_selectedAccount.id();
      QCString newId;
      if(m_transactionEditor->enterTransactions(newId))
        deleteTransactionEditor();
      if(!newId.isEmpty()) {
        myMoneyView->slotLedgerSelected(accountId, newId);
      }
    }
    slotUpdateActions();
  }
}

void KMyMoney2App::slotTransactionsCancelOrEnter(bool& okToSelect)
{
  static bool oneTime = false;
  if(!oneTime) {
    oneTime = true;
    QString dontShowAgain = "CancelOrEditTransaction";
    // qDebug("KMyMoney2App::slotCancelOrEndEdit");
    if(m_transactionEditor) {
      if(KMyMoneyGlobalSettings::focusChangeIsEnter() && kmymoney2->action("transaction_enter")->isEnabled()) {
        slotTransactionsEnter();
      } else {
        // okToSelect is preset to true if a cancel of the dialog is useful and false if it is not
        int rc;
        if(okToSelect == true) {
          rc = KMessageBox::warningYesNoCancel(0, QString("<p>")+i18n("Do you really want to cancel editing this transaction without saving it?<p>- <b>Yes</b> cancels editing the transaction<br>- <b>No</b> saves the transaction prior to cancelling and<br>- <b>Cancel</b> returns to the transaction editor.<p>You can also select an option to save the transaction automatically when e.g. selecting another transaction."), i18n("Cancel transaction edit"), KStdGuiItem::yes(), KStdGuiItem::no(), dontShowAgain);

        } else {
          rc = KMessageBox::warningYesNo(0, QString("<p>")+i18n("Do you really want to cancel editing this transaction without saving it?<p>- <b>Yes</b> cancels editing the transaction<br>- <b>No</b> saves the transaction prior to cancelling.<p>You can also select an option to save the transaction automatically when e.g. selecting another transaction."), i18n("Cancel transaction edit"), KStdGuiItem::yes(), KStdGuiItem::no(), dontShowAgain);
        }

        switch(rc) {
          case KMessageBox::Yes:
            slotTransactionsCancel();
            break;
          case KMessageBox::No:
            slotTransactionsEnter();
            // make sure that we'll see this message the next time no matter
            // if the user has chosen the 'Don't show again' checkbox
            KMessageBox::enableMessage(dontShowAgain);
            break;
          case KMessageBox::Cancel:
            // make sure that we'll see this message the next time no matter
            // if the user has chosen the 'Don't show again' checkbox
            KMessageBox::enableMessage(dontShowAgain);
            okToSelect = false;
            break;
        }
      }
    }
    oneTime = false;
  }
}

void KMyMoney2App::slotToggleReconciliationFlag(void)
{
  markTransaction(MyMoneySplit::Unknown);
}

void KMyMoney2App::slotMarkTransactionCleared(void)
{
  markTransaction(MyMoneySplit::Cleared);
}

void KMyMoney2App::slotMarkTransactionReconciled(void)
{
  markTransaction(MyMoneySplit::Reconciled);
}

void KMyMoney2App::slotMarkTransactionNotReconciled(void)
{
  markTransaction(MyMoneySplit::NotReconciled);
}

void KMyMoney2App::markTransaction(MyMoneySplit::reconcileFlagE flag)
{
  KMyMoneyRegister::SelectedTransactions list = m_selectedTransactions;
  KMyMoneyRegister::SelectedTransactions::const_iterator it_t;
  int cnt = list.count();
  int i = 0;
  slotStatusProgressBar(0, cnt);
  MyMoneyFileTransaction ft;
  try {
    for(it_t = list.begin(); it_t != list.end(); ++it_t) {
      // turn on signals before we modify the last entry in the list
      cnt--;
      MyMoneyFile::instance()->blockSignals(cnt != 0);

      // get a fresh copy
      MyMoneyTransaction t = MyMoneyFile::instance()->transaction((*it_t).transaction().id());
      MyMoneySplit sp = t.splitById((*it_t).split().id());
      if(sp.reconcileFlag() != flag) {
        if(flag == MyMoneySplit::Unknown) {
          if(m_reconciliationAccount.id().isEmpty()) {
            // in normal mode we cycle through all states
            switch(sp.reconcileFlag()) {
              case MyMoneySplit::NotReconciled:
                sp.setReconcileFlag(MyMoneySplit::Cleared);
                break;
              case MyMoneySplit::Cleared:
                sp.setReconcileFlag(MyMoneySplit::Reconciled);
                break;
              case MyMoneySplit::Reconciled:
                sp.setReconcileFlag(MyMoneySplit::NotReconciled);
                break;
              default:
                break;
            }
          } else {
            // in reconciliation mode we skip the reconciled state
            switch(sp.reconcileFlag()) {
              case MyMoneySplit::NotReconciled:
                sp.setReconcileFlag(MyMoneySplit::Cleared);
                break;
              case MyMoneySplit::Cleared:
                sp.setReconcileFlag(MyMoneySplit::NotReconciled);
                break;
              default:
                break;
            }
          }
        } else {
          sp.setReconcileFlag(flag);
        }

        t.modifySplit(sp);
        MyMoneyFile::instance()->modifyTransaction(t);
      }
      slotStatusProgressBar(i++, 0);
    }
    slotStatusProgressBar(-1, -1);
    ft.commit();
  } catch(MyMoneyException* e) {
    KMessageBox::detailedSorry(0, i18n("Error"), i18n("Unable to modify transaction: %1, thrown in %2:%3").arg(e->what()).arg(e->file()).arg(e->line()));
    delete e;
  }
}

void KMyMoney2App::slotTransactionsAccept(void)
{
  KMyMoneyRegister::SelectedTransactions list = m_selectedTransactions;
  KMyMoneyRegister::SelectedTransactions::const_iterator it_t;
  int cnt = list.count();
  int i = 0;
  slotStatusProgressBar(0, cnt);
  MyMoneyFileTransaction ft;
  try {
    for(it_t = list.begin(); it_t != list.end(); ++it_t) {
      // reload transaction in case it got changed during the course of this loop
      MyMoneyTransaction t = MyMoneyFile::instance()->transaction((*it_t).transaction().id());
      if(t.isImported()) {
        t.deletePair("Imported");
        if(!m_selectedAccount.id().isEmpty()) {
          QValueList<MyMoneySplit> list = t.splits();
          QValueList<MyMoneySplit>::const_iterator it_s;
          for(it_s = list.begin(); it_s != list.end(); ++it_s) {
            if((*it_s).accountId() == m_selectedAccount.id()) {
              if((*it_s).reconcileFlag() == MyMoneySplit::NotReconciled) {
                MyMoneySplit s = (*it_s);
                s.setReconcileFlag(MyMoneySplit::Cleared);
                t.modifySplit(s);
              }
            }
          }
        }
        MyMoneyFile::instance()->modifyTransaction(t);
      }
      else if((*it_t).split().isMatched()) {
        // reload split in case it got changed during the course of this loop
        MyMoneySplit s = t.splitById((*it_t).split().id());
        TransactionMatcher matcher(m_selectedAccount);
        matcher.accept(t, s);
      }
      slotStatusProgressBar(i++, 0);
    }
    slotStatusProgressBar(-1, -1);
    ft.commit();
  } catch(MyMoneyException* e) {
    KMessageBox::detailedSorry(0, i18n("Error"), i18n("Unable to accept transaction: %1, thrown in %2:%3").arg(e->what()).arg(e->file()).arg(e->line()));
    delete e;
  }
}

void KMyMoney2App::slotTransactionGotoAccount(void)
{
  if(!m_accountGoto.isEmpty()) {
    try {
      QCString transactionId;
      if(m_selectedTransactions.count() == 1) {
        transactionId = m_selectedTransactions[0].transaction().id();
      }
      // make sure to pass a copy, as myMoneyView->slotLedgerSelected() overrides
      // m_accountGoto while calling slotUpdateActions()
      QCString accountId = m_accountGoto;
      myMoneyView->slotLedgerSelected(accountId, transactionId);
    } catch(MyMoneyException* e) {
      delete e;
    }
  }
}

void KMyMoney2App::slotTransactionGotoPayee(void)
{
  if(!m_payeeGoto.isEmpty()) {
    try {
      QCString transactionId;
      if(m_selectedTransactions.count() == 1) {
        transactionId = m_selectedTransactions[0].transaction().id();
      }
      // make sure to pass copies, as myMoneyView->slotPayeeSelected() overrides
      // m_payeeGoto and m_selectedAccount while calling slotUpdateActions()
      QCString payeeId = m_payeeGoto;
      QCString accountId = m_selectedAccount.id();
      myMoneyView->slotPayeeSelected(payeeId, accountId, transactionId);
    } catch(MyMoneyException* e) {
      delete e;
    }
  }
}

void KMyMoney2App::slotTransactionCreateSchedule(void)
{
  if(m_selectedTransactions.count() == 1)
    slotScheduleNew(m_selectedTransactions[0].transaction());
}

void KMyMoney2App::slotTransactionAssignNumber(void)
{
  if(m_transactionEditor)
    m_transactionEditor->assignNextNumber();
}

void KMyMoney2App::slotTransactionCombine(void)
{
  qDebug("slotTransactionCombine() not implemented yet");
}

void KMyMoney2App::slotMoveToAccount(const QCString& id)
{
  // close the menu, if it is still open
  QWidget* w = factory()->container("transaction_move_menu", this);
  if(w) {
    if(w->isVisible()) {
      w->close();
    }
  }

  if(m_selectedTransactions.count() > 0) {
    MyMoneyFileTransaction ft;
    try {
      KMyMoneyRegister::SelectedTransactions::const_iterator it_t;
      for(it_t = m_selectedTransactions.begin(); it_t != m_selectedTransactions.end(); ++it_t) {
        QValueList<MyMoneySplit>::const_iterator it_s;
        bool changed = false;
        MyMoneyTransaction t = (*it_t).transaction();
        for(it_s = (*it_t).transaction().splits().begin(); it_s != (*it_t).transaction().splits().end(); ++it_s) {
          if((*it_s).accountId() == m_selectedAccount.id()) {
            MyMoneySplit s = (*it_s);
            s.setAccountId(id);
            t.modifySplit(s);
            changed = true;
          }
        }
        if(changed) {
          MyMoneyFile::instance()->modifyTransaction(t);
        }
      }
      ft.commit();
    } catch(MyMoneyException *e) {
      delete e;
    }
  }
}

void KMyMoney2App::slotUpdateMoveToAccountMenu(void)
{
  if(!m_selectedAccount.id().isEmpty()) {
    AccountSet accountSet;
    if(m_selectedAccount.isAssetLiability()) {
      accountSet.addAccountType(MyMoneyAccount::Checkings);
      accountSet.addAccountType(MyMoneyAccount::Savings);
      accountSet.addAccountType(MyMoneyAccount::Cash);
      accountSet.addAccountType(MyMoneyAccount::AssetLoan);
      accountSet.addAccountType(MyMoneyAccount::CertificateDep);
      accountSet.addAccountType(MyMoneyAccount::MoneyMarket);
      accountSet.addAccountType(MyMoneyAccount::Asset);
      accountSet.addAccountType(MyMoneyAccount::Currency);
      accountSet.addAccountType(MyMoneyAccount::CreditCard);
      accountSet.addAccountType(MyMoneyAccount::Loan);
      accountSet.addAccountType(MyMoneyAccount::Liability);
    } else if(m_selectedAccount.isIncomeExpense()) {
      accountSet.addAccountType(MyMoneyAccount::Income);
      accountSet.addAccountType(MyMoneyAccount::Expense);
    }

    accountSet.load(d->m_moveToAccountSelector);
    // remove those accounts that we currently reference
    KMyMoneyRegister::SelectedTransactions::const_iterator it_t;
    for(it_t = m_selectedTransactions.begin(); it_t != m_selectedTransactions.end(); ++it_t) {
      QValueList<MyMoneySplit>::const_iterator it_s;
      for(it_s = (*it_t).transaction().splits().begin(); it_s != (*it_t).transaction().splits().end(); ++it_s) {
        d->m_moveToAccountSelector->removeItem((*it_s).accountId());
      }
    }
    // remove those accounts from the list that are denominated
    // in a different currency
    QCStringList list = d->m_moveToAccountSelector->accountList();
    QValueList<QCString>::const_iterator it_a;
    for(it_a = list.begin(); it_a != list.end(); ++it_a) {
      MyMoneyAccount acc = MyMoneyFile::instance()->account(*it_a);
      if(acc.currencyId() != m_selectedAccount.currencyId())
        d->m_moveToAccountSelector->removeItem((*it_a));
    }
    // Now update the width of the list
    d->m_moveToAccountSelector->setOptimizedWidth();
  }
}

void KMyMoney2App::slotTransactionMatch(void)
{
  if(action("transaction_match")->text() == i18n("Button text for match transaction", "Match"))
    transactionMatch();
  else
    transactionUnmatch();
}

void KMyMoney2App::transactionUnmatch(void)
{
  KMyMoneyRegister::SelectedTransactions::const_iterator it;
  MyMoneyFileTransaction ft;
  try {
    for(it = m_selectedTransactions.begin(); it != m_selectedTransactions.end(); ++it) {
      if((*it).split().isMatched()) {
        TransactionMatcher matcher(m_selectedAccount);
        matcher.unmatch((*it).transaction(), (*it).split());
      }
    }
    ft.commit();

  } catch(MyMoneyException *e) {
    KMessageBox::detailedSorry(0, i18n("Unable to unmatch the selected transactions"), e->what() );
    delete e;
  }
}

void KMyMoney2App::transactionMatch(void)
{
  if(m_selectedTransactions.count() != 2)
    return;

  MyMoneyTransaction startMatchTransaction;
  MyMoneyTransaction endMatchTransaction;
  MyMoneySplit startSplit;
  MyMoneySplit endSplit;

  KMyMoneyRegister::SelectedTransactions::const_iterator it;
  KMyMoneyRegister::SelectedTransactions toBeDeleted;
  for(it = m_selectedTransactions.begin(); it != m_selectedTransactions.end(); ++it) {
    if((*it).transaction().isImported()) {
      endMatchTransaction = (*it).transaction();
      endSplit = (*it).split();
      toBeDeleted << *it;
    } else if(!(*it).split().isMatched()) {
      startMatchTransaction = (*it).transaction();
      startSplit = (*it).split();
    }
  }

#if 0
  KMergeTransactionsDlg dlg(m_selectedAccount);
  dlg.addTransaction(startMatchTransaction);
  dlg.addTransaction(endMatchTransaction);
  if (dlg.exec() == QDialog::Accepted)
#endif
  {
    MyMoneyFileTransaction ft;
    try
    {
      if(startMatchTransaction.id().isEmpty())
        throw new MYMONEYEXCEPTION(i18n("No manually entered transaction selected for matching"));
      if(endMatchTransaction.id().isEmpty())
        throw new MYMONEYEXCEPTION(i18n("No imported transaction selected for matching"));

      TransactionMatcher matcher(m_selectedAccount);
      matcher.match(startMatchTransaction, startSplit, endMatchTransaction, endSplit);
      ft.commit();
    }
    catch(MyMoneyException *e)
    {
      KMessageBox::detailedSorry(0, i18n("Unable to match the selected transactions"), e->what() );
      delete e;
    }
  }
}


void KMyMoney2App::showContextMenu(const QString& containerName)
{
  QWidget* w = factory()->container(containerName, this);
  QPopupMenu *menu = dynamic_cast<QPopupMenu*>(w);
  if(menu)
    menu->exec(QCursor::pos());
}

void KMyMoney2App::slotShowTransactionContextMenu(void)
{
  showContextMenu("transaction_context_menu");
}

void KMyMoney2App::slotShowInvestmentContextMenu(void)
{
  showContextMenu("investment_context_menu");
}

void KMyMoney2App::slotShowScheduleContextMenu(void)
{
  showContextMenu("schedule_context_menu");
}

void KMyMoney2App::slotShowAccountContextMenu(const MyMoneyObject& obj)
{
//  qDebug("KMyMoney2App::slotShowAccountContextMenu");
  if(typeid(obj) != typeid(MyMoneyAccount))
    return;

  const MyMoneyAccount& acc = dynamic_cast<const MyMoneyAccount&>(obj);

  // if the selected account is actually a stock account, we
  // call the right slot instead
  if(acc.isInvest()) {
    showContextMenu("investment_context_menu");
  } else if(acc.isIncomeExpense()){
    showContextMenu("category_context_menu");
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

void KMyMoney2App::slotShowBudgetContextMenu(void)
{
  showContextMenu("budget_context_menu");
}

void KMyMoney2App::slotShowCurrencyContextMenu(void)
{
  showContextMenu("currency_context_menu");
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
  caption += QString(" (%1 x %2)").arg(width()).arg(height());
#endif

  caption = kapp->makeStdCaption(caption, false, modified);
  if(caption.length() > 0)
    caption += " - ";
  caption += "KMyMoney";
  setPlainCaption(caption);

  if(!skipActions) {
    myMoneyView->enableViews();
    slotUpdateActions();
  }
}

void KMyMoney2App::slotUpdateActions(void)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  bool fileOpen = myMoneyView->fileOpen();
  bool modified = file->dirty();
  QWidget* w;

  action("open_database")->setEnabled(true);
  action("saveas_database")->setEnabled(fileOpen);
  action("file_save")->setEnabled(modified && !myMoneyView->isSyncDatabase());
  action("file_save_as")->setEnabled(fileOpen);
  action("file_close")->setEnabled(fileOpen);
  action("view_personal_data")->setEnabled(fileOpen);
  action("file_backup")->setEnabled(fileOpen && !myMoneyView->isDatabase());
  action("file_print")->setEnabled(fileOpen && myMoneyView->canPrint());
#if KMM_DEBUG
  action("view_file_info")->setEnabled(fileOpen);
  action("file_dump")->setEnabled(fileOpen);
#endif

  action("edit_find_transaction")->setEnabled(fileOpen);

  bool importRunning = (m_qifReader != 0) || (m_smtReader != 0);
  action("file_export_qif")->setEnabled(fileOpen && !importRunning);
  action("file_import_qif")->setEnabled(fileOpen && !importRunning);
  action("file_import_gnc")->setEnabled(!importRunning);
  action("file_import_template")->setEnabled(fileOpen && !importRunning);
  action("file_export_template")->setEnabled(fileOpen && !importRunning);


  action("tools_security_editor")->setEnabled(fileOpen);
  action("tools_currency_editor")->setEnabled(fileOpen);
  action("tools_price_editor")->setEnabled(fileOpen);
  action("tools_update_prices")->setEnabled(fileOpen);
  action("tools_consistency_check")->setEnabled(fileOpen);

  action("account_new")->setEnabled(fileOpen);
  action("account_reconcile")->setEnabled(false);
  action("account_reconcile_finish")->setEnabled(false);
  action("account_reconcile_postpone")->setEnabled(false);
  action("account_edit")->setEnabled(false);
  action("account_delete")->setEnabled(false);
  action("account_open")->setEnabled(false);
  action("account_close")->setEnabled(false);
  action("account_reopen")->setEnabled(false);
  action("account_transaction_report")->setEnabled(false);
  action("account_online_map")->setEnabled(false);
  action("account_online_update")->setEnabled(false);
#ifdef HAVE_KDCHART
  action("account_chart")->setEnabled(false);
#endif

  action("category_new")->setEnabled(fileOpen);
  action("category_edit")->setEnabled(false);
  action("category_delete")->setEnabled(false);

  action("institution_new")->setEnabled(fileOpen);
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
  action("schedule_skip")->setEnabled(false);

  action("payee_delete")->setEnabled(false);
  action("payee_rename")->setEnabled(false);

  action("budget_delete")->setEnabled(false);
  action("budget_rename")->setEnabled(false);
  action("budget_change_year")->setEnabled(false);
  action("budget_new")->setEnabled(true);
  action("budget_copy")->setEnabled(false);
  action("budget_forecast")->setEnabled(false);

  QString tooltip = i18n("Create a new transaction");
  action("transaction_new")->setEnabled(fileOpen && myMoneyView->canCreateTransactions(KMyMoneyRegister::SelectedTransactions(), tooltip));
  action("transaction_new")->setToolTip(tooltip);

  action("transaction_edit")->setEnabled(false);
  action("transaction_editsplits")->setEnabled(false);
  action("transaction_enter")->setEnabled(false);
  action("transaction_cancel")->setEnabled(false);
  action("transaction_delete")->setEnabled(false);
  action("transaction_match")->setEnabled(false);
  action("transaction_match")->setText("Match");
  action("transaction_match")->setIcon("connect_creating");

  action("transaction_accept")->setEnabled(false);
  action("transaction_duplicate")->setEnabled(false);
  action("transaction_mark_toggle")->setEnabled(false);
  action("transaction_mark_cleared")->setEnabled(false);
  action("transaction_mark_reconciled")->setEnabled(false);
  action("transaction_mark_notreconciled")->setEnabled(false);
  action("transaction_goto_account")->setEnabled(false);
  action("transaction_goto_payee")->setEnabled(false);
  action("transaction_assign_number")->setEnabled(false);
  action("transaction_create_schedule")->setEnabled(false);
  action("transaction_combine")->setEnabled(false);
  action("transaction_select_all")->setEnabled(false);

  action("schedule_new")->setEnabled(fileOpen);
  action("schedule_edit")->setEnabled(false);
  action("schedule_delete")->setEnabled(false);
  action("schedule_duplicate")->setEnabled(false);
  action("schedule_enter")->setEnabled(false);
  action("schedule_skip")->setEnabled(false);

  action("currency_new")->setEnabled(fileOpen);
  action("currency_rename")->setEnabled(false);
  action("currency_delete")->setEnabled(false);
  action("currency_setbase")->setEnabled(false);

  action("tools_plugin_list")->setEnabled(d->m_pluginDlg != 0);

  w = factory()->container("transaction_move_menu", this);
  if(w)
    w->setEnabled(false);

  // FIXME for now it's always on, but we should only allow it, if we
  //       can select at least a single transaction
  action("transaction_select_all")->setEnabled(true);
  if(!m_selectedTransactions.isEmpty()) {
    action("transaction_delete")->setEnabled(m_selectedTransactions.count() != 0);

    if(!m_transactionEditor) {
      tooltip = i18n("Duplicate the current selected transactions");
      action("transaction_duplicate")->setEnabled(myMoneyView->canDuplicateTransactions(m_selectedTransactions, tooltip) && !m_selectedTransactions[0].transaction().id().isEmpty());
      action("transaction_duplicate")->setToolTip(tooltip);
      if(myMoneyView->canEditTransactions(m_selectedTransactions, tooltip)) {
        action("transaction_edit")->setEnabled(true);
        // editing splits is allowed only if we have one transaction selected
        if(m_selectedTransactions.count() == 1) {
          action("transaction_editsplits")->setEnabled(true);
        }
        if(m_selectedAccount.isAssetLiability()) {
          action("transaction_create_schedule")->setEnabled(m_selectedTransactions.count() == 1);
        }
      }
      action("transaction_edit")->setToolTip(tooltip);

      w = factory()->container("transaction_move_menu", this);
      if(w)
        w->setEnabled(true);

      // Allow marking the transaction if at least one is selected
      action("transaction_mark_cleared")->setEnabled(true);
      action("transaction_mark_reconciled")->setEnabled(true);
      action("transaction_mark_notreconciled")->setEnabled(true);
      action("transaction_mark_toggle")->setEnabled(true);

      if(!m_accountGoto.isEmpty())
        action("transaction_goto_account")->setEnabled(true);
      if(!m_payeeGoto.isEmpty())
        action("transaction_goto_payee")->setEnabled(true);

      // Matching is enabled as soon as one regular and one imported transaction is selected
      int matchedCount = 0;
      int importedCount = 0;
      KMyMoneyRegister::SelectedTransactions::const_iterator it;
      for(it = m_selectedTransactions.begin(); it != m_selectedTransactions.end(); ++it) {
        if((*it).transaction().isImported())
          ++importedCount;
        if((*it).split().isMatched())
          ++matchedCount;
      }

      if(m_selectedTransactions.count() == 2 /* && action("transaction_edit")->isEnabled() */) {
        if(importedCount == 1 && matchedCount == 0) {
          action("transaction_match")->setEnabled(true);
        }
      }
      if(importedCount != 0 || matchedCount != 0)
        action("transaction_accept")->setEnabled(true);
      if(matchedCount != 0) {
        action("transaction_match")->setEnabled(true);
        action("transaction_match")->setText("Unmatch");
        action("transaction_match")->setIcon("stop");
      }

      if(m_selectedTransactions.count() > 1) {
        action("transaction_combine")->setEnabled(true);
      }
    } else {
      action("transaction_assign_number")->setEnabled(m_transactionEditor->canAssignNumber());
      action("transaction_new")->setEnabled(false);
      action("transaction_delete")->setEnabled(false);
      action("transaction_enter")->setEnabled(m_transactionEditor->isComplete());
      action("transaction_cancel")->setEnabled(true);
    }
  }

  MyMoneyFileBitArray skip(IMyMoneyStorage::MaxRefCheckBits);
  if(!m_selectedAccount.id().isEmpty()) {
    if(!file->isStandardAccount(m_selectedAccount.id())) {
      switch(m_selectedAccount.accountGroup()) {
        case MyMoneyAccount::Asset:
        case MyMoneyAccount::Liability:
        case MyMoneyAccount::Equity:
          action("account_transaction_report")->setEnabled(true);
          action("account_edit")->setEnabled(true);
          action("account_delete")->setEnabled(!file->isReferenced(m_selectedAccount));
          action("account_open")->setEnabled(true);
          if(m_selectedAccount.accountGroup() != MyMoneyAccount::Equity) {
            if(m_reconciliationAccount.id().isEmpty()) {
              action("account_reconcile")->setEnabled(true);
            } else {
              if(!m_transactionEditor) {
                action("account_reconcile_finish")->setEnabled(m_selectedAccount.id() == m_reconciliationAccount.id());
                action("account_reconcile_postpone")->setEnabled(m_selectedAccount.id() == m_reconciliationAccount.id());
              }
            }
          }

          if(m_selectedAccount.accountType() == MyMoneyAccount::Investment)
            action("investment_new")->setEnabled(true);

          if(m_selectedAccount.isClosed())
            action("account_reopen")->setEnabled(true);
          else if(canCloseAccount(m_selectedAccount))
            action("account_close")->setEnabled(true);

          if ( !m_selectedAccount.onlineBankingSettings().value("provider").isEmpty() )
            action("account_online_update")->setEnabled(true);
          else
            action("account_online_map")->setEnabled(m_onlinePlugins.count() > 0);

#ifdef HAVE_KDCHART
          action("account_chart")->setEnabled(true);
#endif
          break;

        case MyMoneyAccount::Income :
        case MyMoneyAccount::Expense :
          action("category_edit")->setEnabled(true);
          // enable delete action, if category/account itself is not referenced
          // by any object except accounts, because we want to allow
          // deleting of sub-categories. Also, we allow transactions, schedules and budgets
          // to be present because we can re-assign them during the delete process
          skip.fill(false);
          skip.setBit(IMyMoneyStorage::RefCheckTransaction);
          skip.setBit(IMyMoneyStorage::RefCheckAccount);
          skip.setBit(IMyMoneyStorage::RefCheckSchedule);
          skip.setBit(IMyMoneyStorage::RefCheckBudget);
          action("category_delete")->setEnabled(!file->isReferenced(m_selectedAccount, skip));
          action("account_open")->setEnabled(true);
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
    if(m_selectedInvestment.isClosed())
      action("account_reopen")->setEnabled(true);
    else if(canCloseAccount(m_selectedInvestment))
      action("account_close")->setEnabled(true);
  }

  if(!m_selectedSchedule.id().isEmpty()) {
    action("schedule_edit")->setEnabled(true);
    action("schedule_duplicate")->setEnabled(true);
    action("schedule_delete")->setEnabled(!file->isReferenced(m_selectedSchedule));
    if(!m_selectedSchedule.isFinished()) {
      action("schedule_enter")->setEnabled(true);
      // a schedule with a single occurence cannot be skipped
      if(m_selectedSchedule.occurence() != MyMoneySchedule::OCCUR_ONCE) {
        action("schedule_skip")->setEnabled(true);
      }
    }
  }

  if(m_selectedPayees.count() >= 1) {
    action("payee_rename")->setEnabled(m_selectedPayees.count() == 1);
    action("payee_delete")->setEnabled(true);
  }

  if(m_selectedBudgets.count() >= 1) {
    action("budget_delete")->setEnabled(true);
    if(m_selectedBudgets.count() == 1) {
      action("budget_change_year")->setEnabled(true);
      action("budget_copy")->setEnabled(true);
      action("budget_rename")->setEnabled(true);
      action("budget_forecast")->setEnabled(true);
    }
  }

  if(!m_selectedCurrency.id().isEmpty()) {
    action("currency_rename")->setEnabled(true);
    // no need to check each transaction. accounts are enough in this case
    skip.fill(false);
    skip.setBit(IMyMoneyStorage::RefCheckTransaction);
    action("currency_delete")->setEnabled(!file->isReferenced(m_selectedCurrency, skip));
    if(m_selectedCurrency.id() != file->baseCurrency().id())
      action("currency_setbase")->setEnabled(true);
  }
}

void KMyMoney2App::slotResetSelections(void)
{
  slotSelectAccount();
  slotSelectInstitution();
  slotSelectInvestment();
  slotSelectSchedule();
  slotSelectCurrency();
  slotSelectPayees(QValueList<MyMoneyPayee>());
  slotSelectBudget(QValueList<MyMoneyBudget>());
  slotSelectTransactions(KMyMoneyRegister::SelectedTransactions());
  slotUpdateActions();
}

void KMyMoney2App::slotSelectCurrency(const MyMoneySecurity& currency)
{
  m_selectedCurrency = currency;
  slotUpdateActions();
  emit currencySelected(m_selectedCurrency);
}

void KMyMoney2App::slotSelectBudget(const QValueList<MyMoneyBudget>& list)
{
  m_selectedBudgets = list;
  slotUpdateActions();
  emit budgetSelected(m_selectedBudgets);
}

void KMyMoney2App::slotSelectPayees(const QValueList<MyMoneyPayee>& list)
{
  m_selectedPayees = list;
  slotUpdateActions();
  emit payeesSelected(m_selectedPayees);
}

void KMyMoney2App::slotSelectTransactions(const KMyMoneyRegister::SelectedTransactions& list)
{
  m_selectedTransactions = list;

  m_accountGoto = QCString();
  m_payeeGoto = QCString();
  if(list.count() == 1) {
    const MyMoneySplit& sp = m_selectedTransactions[0].split();
    if(!sp.payeeId().isEmpty()) {
      try {
        MyMoneyPayee payee = MyMoneyFile::instance()->payee(sp.payeeId());
        if(!payee.name().isEmpty()) {
          m_payeeGoto = payee.id();
          QString name = payee.name();
          name.replace(QRegExp("&(?!&)"), "&&");
          action("transaction_goto_payee")->setText(i18n("Goto '%1'").arg(name));
        }
      } catch(MyMoneyException *e) {
        delete e;
      }
    }
    try {
      QValueList<MyMoneySplit>::const_iterator it_s;
      const MyMoneyTransaction& t = m_selectedTransactions[0].transaction();
      // search the first non-income/non-expense accunt and use it for the 'goto account'
      const MyMoneySplit& sp = m_selectedTransactions[0].split();
      for(it_s = t.splits().begin(); it_s != t.splits().end(); ++it_s) {
        if((*it_s).id() != sp.id()) {
          MyMoneyAccount acc = MyMoneyFile::instance()->account((*it_s).accountId());
          if(!acc.isIncomeExpense()) {
            // for stock accounts we show the portfolio account
            if(acc.isInvest()) {
              acc = MyMoneyFile::instance()->account(acc.parentAccountId());
            }
            m_accountGoto = acc.id();
            QString name = acc.name();
            name.replace(QRegExp("&(?!&)"), "&&");
            action("transaction_goto_account")->setText(i18n("Goto '%1'").arg(name));
            break;
          }
        }
      }
    } catch(MyMoneyException* e) {
      delete e;
    }
  }

  // make sure, we show some neutral menu entry if we don't have an object
  if(m_payeeGoto.isEmpty())
    action("transaction_goto_payee")->setText(i18n("Goto payee"));
  if(m_accountGoto.isEmpty())
    action("transaction_goto_account")->setText(i18n("Goto account"));

  slotUpdateActions();
  emit transactionsSelected(m_selectedTransactions);
}

void KMyMoney2App::slotSelectInstitution(const MyMoneyObject& institution)
{
  if(typeid(institution) != typeid(MyMoneyInstitution))
    return;

  m_selectedInstitution = dynamic_cast<const MyMoneyInstitution&>(institution);
  // qDebug("slotSelectInstitution('%s')", m_selectedInstitution.name().data());
  slotUpdateActions();
  emit institutionSelected(m_selectedInstitution);
}

void KMyMoney2App::slotSelectAccount(const MyMoneyObject& obj)
{
  if(typeid(obj) != typeid(MyMoneyAccount))
    return;

  m_selectedAccount = MyMoneyAccount();
  const MyMoneyAccount& acc = dynamic_cast<const MyMoneyAccount&>(obj);
  if(!acc.isInvest())
    m_selectedAccount = acc;

  // qDebug("slotSelectAccount('%s')", m_selectedAccount.name().data());
  slotUpdateActions();
  emit accountSelected(m_selectedAccount);
}

void KMyMoney2App::slotSelectInvestment(const MyMoneyObject& obj)
{
  if(typeid(obj) != typeid(MyMoneyAccount))
    return;

  // qDebug("slotSelectInvestment('%s')", account.name().data());
  m_selectedInvestment = MyMoneyAccount();
  const MyMoneyAccount& acc = dynamic_cast<const MyMoneyAccount&>(obj);
  if(acc.isInvest())
    m_selectedInvestment = acc;

  slotUpdateActions();
  emit investmentSelected(m_selectedInvestment);
}

void KMyMoney2App::slotSelectSchedule(const MyMoneySchedule& schedule)
{
  // qDebug("slotSelectSchedule('%s')", schedule.name().data());
  m_selectedSchedule = schedule;
  slotUpdateActions();
  emit scheduleSelected(m_selectedSchedule);
}

void KMyMoney2App::slotDataChanged(void)
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
  connect(&dlg, SIGNAL(selectObject(const MyMoneySecurity&)), this, SLOT(slotSelectCurrency(const MyMoneySecurity&)));
  connect(&dlg, SIGNAL(openContextMenu(const MyMoneySecurity&)), this, SLOT(slotShowCurrencyContextMenu()));
  connect(this, SIGNAL(currencyRename()), &dlg, SLOT(slotStartRename()));
  connect(&dlg, SIGNAL(renameCurrency(QListViewItem*, int, const QString&)), this, SLOT(slotCurrencyRename(QListViewItem*,int,const QString&)));
  connect(this, SIGNAL(currencyCreated(const QCString&)), &dlg, SLOT(slotSelectCurrency(const QCString&)));
  connect(&dlg, SIGNAL(selectBaseCurrency(const MyMoneySecurity&)), this, SLOT(slotCurrencySetBase()));

  dlg.exec();

  slotSelectCurrency(MyMoneySecurity());
}

void KMyMoney2App::slotPriceDialog(void)
{
  KMyMoneyPriceDlg dlg(this, "Price Editor");
  dlg.exec();
}

void KMyMoney2App::slotFileConsitencyCheck(void)
{
  KMSTATUS(i18n("Running consistency check..."));

  QStringList msg;
  MyMoneyFileTransaction ft;
  try {
    msg = MyMoneyFile::instance()->consistencyCheck();
    ft.commit();
  } catch(MyMoneyException *e) {
    msg = i18n("Consistency check failed: %1").arg(e->what());
    delete e;
  }

  KMessageBox::warningContinueCancelList(0, "Result", msg, i18n("Consistency check result"));

  updateCaption();
}

void KMyMoney2App::slotCheckSchedules(void)
{
  if(KMyMoneyGlobalSettings::checkSchedule() == true) {

    KMSTATUS(i18n("Checking for overdue schedules..."));
    MyMoneyFile *file = MyMoneyFile::instance();
    QDate checkDate = QDate::currentDate().addDays(KMyMoneyGlobalSettings::checkSchedulePreview());

    QValueList<MyMoneySchedule> scheduleList =  file->scheduleList();
    QValueList<MyMoneySchedule>::Iterator it;

    for (it=scheduleList.begin(); it!=scheduleList.end(); ++it) {
      // Get the copy in the file because it might be modified by commitTransaction
      MyMoneySchedule schedule = file->schedule((*it).id());

      if(schedule.autoEnter()) {
        try {
          while(!schedule.isFinished() && (schedule.nextDueDate() <= checkDate)) {
            if(!enterSchedule(schedule, true)) {
              // the user has selected to stop processing this schedule
              break;
            }
            schedule = file->schedule((*it).id()); // get a copy of the modified schedule
          }
        } catch(MyMoneyException* e) {
          delete e;
        }
      }
    }
    updateCaption();
  }
}

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

QString KMyMoney2App::readLastUsedDir(void) const
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

QString KMyMoney2App::readLastUsedFile(void) const
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

const QString KMyMoney2App::filename(void) const
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

void KMyMoney2App::slotEquityPriceUpdate(void)
{
  KEquityPriceUpdateDlg dlg(this);
  dlg.exec();
}

void KMyMoney2App::webConnect(const QString& url, const QCString& asn_id)
{
  //
  // Web connect attempts to go through the known importers and see if the file
  // can be importing using that method.  If so, it will import it using that
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
    KMSTATUS(i18n("Importing a statement via Web Connect"));

    QMap<QString,KMyMoneyPlugin::ImporterPlugin*>::const_iterator it_plugin = m_importerPlugins.begin();
    while ( it_plugin != m_importerPlugins.end() )
    {
      if ( (*it_plugin)->isMyFormat(url) )
      {
        QValueList<MyMoneyStatement> statements;
        if (!(*it_plugin)->import(url) )
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
  new KMyMoneyPlugin::KMMImportInterface(this, m_pluginInterface, "import interface");
}

void KMyMoney2App::loadPlugins(void)
{
  bool firstPlugin = true;
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

      if(firstPlugin)
        d->m_pluginDlg->m_listView->clear();
      firstPlugin = false;
      if (plugin) {
        guiFactory()->addClient(plugin);
        kdDebug() << "Loaded '"
                  << plugin->name() << "' plugin" << endl;
        KListViewItem* item = new KListViewItem(d->m_pluginDlg->m_listView, service->name(), i18n("Loaded"));

        // check for online plugin
        KMyMoneyPlugin::OnlinePlugin* op = dynamic_cast<KMyMoneyPlugin::OnlinePlugin *>(plugin);
        if(op) {
          m_onlinePlugins[plugin->name()] = op;
          QStringList protocolList;
          op->protocols(protocolList);
          new KListViewItem(item, i18n("Plugin interface description", "Online access"), "", protocolList.join(", "));
          item->setOpen(true);
          kdDebug() << "It's an online banking plugin and supports '" << protocolList << "'" << endl;
        }

        // check for importer plugin
        KMyMoneyPlugin::ImporterPlugin* ip = dynamic_cast<KMyMoneyPlugin::ImporterPlugin *>(plugin);
        if(ip) {
          m_importerPlugins[plugin->name()] = ip;
          new KListViewItem(item, i18n("Plugin interface description", "File import"), "");
        }

      } else {
        new KListViewItem(d->m_pluginDlg->m_listView, service->name(), QString(), i18n("not loaded: %1").arg(KLibLoader::self()->lastErrorMessage()));

        kdDebug() << "Failed to load '"
                  << service->name() << "' service, error=" << errCode << endl;
        kdDebug() << KLibLoader::self()->lastErrorMessage() << endl;
      }
    }
  }
}

void KMyMoney2App::slotAutoSave(void)
{
  if(!m_inAutoSaving) {
    m_inAutoSaving = true;
    KMSTATUS(i18n("Auto saving ..."));

    //calls slotFileSave if needed, and restart the timer
    //it the file is not saved, reinitializes the countdown.
    if (myMoneyView->dirty() && m_autoSaveEnabled) {
      if (!slotFileSave() && m_autoSavePeriod > 0) {
        m_autoSaveTimer->start(m_autoSavePeriod * 60 * 1000, true);
      }
    }

    m_inAutoSaving = false;
  }
}

void KMyMoney2App::slotDateChanged(void)
{
  QDateTime dt = QDateTime::currentDateTime();
  QDateTime nextDay( QDate(dt.date().addDays(1)), QTime(0, 0, 0) );

  QTimer::singleShot(dt.secsTo(nextDay)*1000, this, SLOT(slotDateChanged()));
  myMoneyView->slotRefreshViews();
}

const MyMoneyAccount& KMyMoney2App::account(const QString& key, const QString& value) const
{
  QValueList<MyMoneyAccount> list;
  QValueList<MyMoneyAccount>::const_iterator it_a;
  MyMoneyFile::instance()->accountList(list);
  for(it_a = list.begin(); it_a != list.end(); ++it_a) {
    if((*it_a).onlineBankingSettings().value(QCString(key)) == value) {
      return MyMoneyFile::instance()->account((*it_a).id());
    }
  }

  // return reference to empty element
  return MyMoneyFile::instance()->account(QCString());
}

void KMyMoney2App::setAccountOnlineParameters(const MyMoneyAccount& _acc, const MyMoneyKeyValueContainer& kvps)
{
  MyMoneyFileTransaction ft;
  try {
    MyMoneyAccount acc = MyMoneyFile::instance()->account(_acc.id());
    acc.setOnlineBankingSettings(kvps);
    MyMoneyFile::instance()->modifyAccount(acc);
    ft.commit();

  } catch(MyMoneyException *e) {
    KMessageBox::detailedSorry(0, i18n("Unable to setup online parameters for account ''%1'").arg(_acc.name()), e->what() );
    delete e;
  }
}

void KMyMoney2App::slotAccountMapOnline(void)
{
  // no account selected
  if(m_selectedAccount.id().isEmpty())
    return;

  // already an account mapped
  if(!m_selectedAccount.onlineBankingSettings().value("provider").isEmpty())
    return;

  // if we have more than one provider display a dialog to select the current providers
  KPluginDlg dlg(this);
  dlg.setCaption(i18n("Select online banking plugin"));
  dlg.closeButton->hide();
  QString provider;
  QMap<QString, KMyMoneyPlugin::OnlinePlugin*>::const_iterator it_p;
  switch(m_onlinePlugins.count()) {
    case 0:
      break;
    case 1:
      provider = m_onlinePlugins.begin().key();
      break;
    default:
      for(it_p = m_onlinePlugins.begin(); it_p != m_onlinePlugins.end(); ++it_p) {
        QStringList protocolList;
        (*it_p)->protocols(protocolList);
        new KListViewItem(dlg.m_listView, it_p.key(), "Loaded", protocolList.join(", "));
      }
      if(dlg.exec() == QDialog::Accepted) {
        if(dlg.m_listView->selectedItem()) {
          provider = dlg.m_listView->selectedItem()->text(0);
        }
      }
      break;
  }

  if(provider.isEmpty())
    return;

  // find the provider
  it_p = m_onlinePlugins.find(provider);
  if(it_p != m_onlinePlugins.end()) {
    // plugin found, call it
    MyMoneyKeyValueContainer settings;
    if((*it_p)->mapAccount(m_selectedAccount, settings)) {
      settings["provider"] = provider;
      MyMoneyAccount acc(m_selectedAccount);
      acc.setOnlineBankingSettings(settings);
      MyMoneyFileTransaction ft;
      try {
        MyMoneyFile::instance()->modifyAccount(acc);
        ft.commit();
      } catch(MyMoneyException* e) {
        KMessageBox::error(this, i18n("Unable to map account to online account: %1").arg(e->what()));
        delete e;
      }
    }
  }
}

void KMyMoney2App::slotAccountUpdateOnline(void)
{
  // no account selected
  if(m_selectedAccount.id().isEmpty())
    return;

  // no online account mapped
  if(m_selectedAccount.onlineBankingSettings().value("provider").isEmpty())
    return;

  // find the provider
  QMap<QString, KMyMoneyPlugin::OnlinePlugin*>::const_iterator it_p;
  it_p = m_onlinePlugins.find(m_selectedAccount.onlineBankingSettings().value("provider"));
  if(it_p != m_onlinePlugins.end()) {
    // plugin found, call it
    (*it_p)->updateAccount(m_selectedAccount);
  }
}

KMStatus::KMStatus (const QString &text)
{
  m_prevText = kmymoney2->slotStatusMsg(text);
}

KMStatus::~KMStatus()
{
  kmymoney2->slotStatusMsg(m_prevText);
}

#include "kmymoney2.moc"
// vim:cin:si:ai:et:ts=2:sw=2:

