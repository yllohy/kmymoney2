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

#if !KDE_IS_VERSION(3,2,0)
#include <kwin.h>
#endif

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoney2.h"
#include "kmymoney2_stub.h"
#include "kstartuplogo.h"

#include "dialogs/kstartdlg.h"
#include "dialogs/ksettingsdlg.h"
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

#include "views/kmymoneyview.h"

#include "mymoney/mymoneyutils.h"
#include "mymoney/mymoneystatement.h"

#include "converter/mymoneyqifwriter.h"
#include "converter/mymoneyqifreader.h"
#include "converter/mymoneystatementreader.h"
#include "converter/mymoneytemplate.h"
#include "converter/mymoneyofxstatement.h"

#include "plugins/kmymoneyplugin.h"
#include "plugins/interfaces/kmmviewinterface.h"
#include "plugins/interfaces/kmmstatementinterface.h"

#include "kmymoneyutils.h"
#include "kdecompat.h"

#define ID_STATUS_MSG 1

KMyMoney2App::KMyMoney2App(QWidget * /*parent*/ , const char* name)
 : KMainWindow(0, name),
 DCOPObject("kmymoney2app"),
 myMoneyView(0)
{
  updateCaption(true);

  // splash screen
  m_startLogo = new KStartupLogo;

  // initial setup of settings
  KMyMoneyUtils::updateSettings();

  QFrame* frame = new QFrame(this);
  frame->setFrameStyle(QFrame::NoFrame);
  // values for margin (11) and spacing(6) taken from KDialog implementation
  QBoxLayout* layout = new QBoxLayout(frame, QBoxLayout::TopToBottom, 11, 6);

  myMoneyView = new KMyMoneyView(frame, "/KMyMoneyView");
  layout->addWidget(myMoneyView, 10);

  config = kapp->config();

  ///////////////////////////////////////////////////////////////////
  // call inits to invoke all other construction parts
  initStatusBar();
  initActions();
  readOptions();

  // now initialize the plugin structure
  createInterfaces();
  loadPlugins();

  setCentralWidget(frame);

  connect(myMoneyView, SIGNAL(viewActivated(int)), this, SLOT(slotSetViewSpecificActions(int)));

  connect(&proc,SIGNAL(processExited(KProcess *)),this,SLOT(slotProcessExited()));

  m_backupState = BACKUP_IDLE;

  m_reader = 0;
  m_smtReader = 0;
  m_engineBackup = 0;

  // make sure, we get a note when the engine changes state
  MyMoneyFile::instance()->attach(MyMoneyFile::NotifyClassAnyChange, this);
}

KMyMoney2App::~KMyMoney2App()
{
  MyMoneyFile::instance()->detach(MyMoneyFile::NotifyClassAnyChange, this);

  if(m_startLogo)
    delete m_startLogo;
  if(m_reader != 0)
    delete m_reader;
  if(m_engineBackup != 0)
    delete m_engineBackup;
}

const KURL KMyMoney2App::lastOpenedURL(void)
{
  KURL url = m_startDialog ? KURL() : fileName;

  if(!url.isValid())
  {
    url = readLastUsedFile();
  }

  if(m_startLogo)
    delete m_startLogo;

  slotStatusMsg(i18n("Ready."));
  return url;
}

void KMyMoney2App::initActions()
{
  fileNew = KStdAction::openNew(this, SLOT(slotFileNew()), actionCollection());
  fileOpen = KStdAction::open(this, SLOT(slotFileOpen()), actionCollection());
  fileOpenRecent = KStdAction::openRecent(this, SLOT(slotFileOpenRecent(const KURL&)), actionCollection());
  fileSave = KStdAction::save(this, SLOT(slotFileSave()), actionCollection());
  fileSaveAs = KStdAction::saveAs(this, SLOT(slotFileSaveAs()), actionCollection());
  fileClose = KStdAction::close(this, SLOT(slotFileClose()), actionCollection());
  fileQuit = KStdAction::quit(this, SLOT(slotFileQuit()), actionCollection());
  filePrint = KStdAction::print(myMoneyView, SLOT(slotPrintView()), actionCollection());

  viewToolBar = KStdAction::showToolbar(this, SLOT(slotViewToolBar()), actionCollection());
  viewStatusBar = KStdAction::showStatusbar(this, SLOT(slotViewStatusBar()), actionCollection());

  actionFindTransaction = new KAction(i18n("Find transaction..."),
                                      "find",
                                      KShortcut("Ctrl+F"),
                                      myMoneyView, SLOT(slotFindTransaction()),
                                      actionCollection(), "account_find");

  // Setup transaction detail switch
  viewTransactionForm = new KToggleAction(i18n("Show Transaction Detail"), KShortcut("Ctrl+T"), actionCollection(), "show_transaction_detail");
  config->setGroup("List Options");
  if(config->readBoolEntry("ShowRegisterDetailed", true) == true)
    viewTransactionForm->setChecked(true);
  else
    viewTransactionForm->setChecked(false);
  connect(viewTransactionForm, SIGNAL(toggled(bool)), myMoneyView, SLOT(slotShowTransactionDetail(bool)));

  // Additions to the file menu
  fileViewInfo = new KAction(i18n("Dump Memory..."), "", 0, this, SLOT(slotFileFileInfo()), actionCollection(), "file_view_info");
  filePersonalData = new KAction(i18n("Personal Data..."), "info", 0, this, SLOT(slotFileViewPersonal()), actionCollection(), "file_personal_data");
  fileBackup = new KAction(i18n("Backup..."), "backup",0,this,SLOT(slotFileBackup()),actionCollection(),"file_backup");
  actionQifImport = new KAction(i18n("QIF ..."), "", 0, this, SLOT(slotQifImport()), actionCollection(), "file_import_qif");
  actionOfxImport = new KAction(i18n("OFX ..."), "", 0, this, SLOT(slotOfxImport()), actionCollection(), "file_import_ofx");
  actionGncImport = new KAction(i18n("Gnucash ..."), "", 0, this, SLOT(slotGncImport()), actionCollection(), "file_import_gnc");
  actionStatementImport = new KAction(i18n("Statement file ..."), "", 0, this, SLOT(slotStatementImport()), actionCollection(), "file_import_statement");

  actionLoadTemplate = new KAction(i18n("Account Template ..."), "", 0, this, SLOT(slotLoadAccountTemplates()), actionCollection(), "file_import_template");
  actionQifExport = new KAction(i18n("QIF ..."), "", 0, this, SLOT(slotQifExport()), actionCollection(), "file_export_qif");
  new KAction(i18n("Consistency Check"), "", 0, this, SLOT(slotFileConsitencyCheck()), actionCollection(), "file_consistency_check");

  new KAction(i18n("Securities ..."), "", 0, this, SLOT(slotSecurityEditor()), actionCollection(), "tool_security_editor");
  new KAction(i18n("Currencies ..."), "", 0, this, SLOT(slotCurrencyDialog()), actionCollection(), "tool_currency_editor");

  new KAction(i18n("Prices ..."), "", 0, this, SLOT(slotPriceDialog()), actionCollection(), "tool_price_editor");
  new KAction(i18n("Update Stock and Currency Prices..."), "", 0, this, SLOT(slotEquityPriceUpdate()), actionCollection(), "equity_price_update");

  // The Settings Menu
  settingsKey = KStdAction::keyBindings(this, SLOT(slotKeySettings()), actionCollection());
  settings = KStdAction::preferences(this, SLOT( slotSettings() ), actionCollection());
  new KAction(i18n("Enable all messages"), "", 0, this, SLOT(slotEnableMessages()), actionCollection(), "enable_messages");

  // The Bank Menu
  bankAdd = new KAction(i18n("Add new institution..."), "bank", 0, myMoneyView, SLOT(slotBankNew()), actionCollection(), "bank_add");

  // The Account Menu
  accountAdd = new KAction(i18n("Add new account..."), "account"/*QIconSet(QPixmap(KGlobal::dirs()->findResource("appdata", "toolbar/kmymoney_newacc.xpm")))*/, 0, myMoneyView, SLOT(slotAccountNew()), actionCollection(), "account_add");

  // The tool menu
  new KAction(i18n("QIF Profile Editor..."), "edit", 0, this, SLOT(slotQifProfileEditor()), actionCollection(), "qif_editor");

  // The help menu
  new KAction(i18n("&Show tip of the day"), "idea", 0, this, SLOT(slotShowTipOfTheDay()), actionCollection(), "show_tip");

  m_previousViewButton = new KToolBarPopupAction(i18n("View back"), "back", 0, this, SLOT(slotShowPreviousView()), actionCollection(), "go_back");
  m_nextViewButton = new KToolBarPopupAction(i18n("View forward"), "forward", 0, this, SLOT(slotShowNextView()), actionCollection(), "go_forward");

  m_previousViewButton->setEnabled(false);
  m_nextViewButton->setEnabled(false);

  // use the absolute path to your kmymoney2ui.rc file for testing purpose in createGUI();
  createGUI();
}

void KMyMoney2App::initStatusBar()
{
  ///////////////////////////////////////////////////////////////////
  // STATUSBAR

  // TODO: add your own items you need for displaying current application status.
  statusBar()->insertItem(i18n("Ready."), ID_STATUS_MSG);

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
  config->writeEntry("Show Toolbar", viewToolBar->isChecked());
  config->writeEntry("Show Statusbar",viewStatusBar->isChecked());
  config->writeEntry("ToolBarPos", (int) toolBar("mainToolBar")->barPos());
  fileOpenRecent->saveEntries(config,"Recent Files");
}


void KMyMoney2App::readOptions()
{
  config->setGroup("General Options");

  // bar status settings
  bool bViewToolbar = config->readBoolEntry("Show Toolbar", true);
  viewToolBar->setChecked(bViewToolbar);
  slotViewToolBar();

  bool bViewStatusbar = config->readBoolEntry("Show Statusbar", true);
  viewStatusBar->setChecked(bViewStatusbar);
  slotViewStatusBar();

  // bar position settings
  KToolBar::BarPosition toolBarPos;
  toolBarPos=(KToolBar::BarPosition) config->readNumEntry("ToolBarPos", KToolBar::Top);
  toolBar("mainToolBar")->setBarPos(toolBarPos);

  // initialize the recent file list
  fileOpenRecent->loadEntries(config,"Recent Files");

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
void KMyMoney2App::slotFileNew()
{
  QString prevMsg = slotStatusMsg(i18n("Creating new document..."));

  slotFileClose();
  if(myMoneyView->fileOpen())
    return;

  fileName = KURL();
  myMoneyView->newFile();
  KMessageBox::information(this, QString("<p>") +
                i18n("The next dialog allows you to add predefined account/category templates to the new file. Different languages are available to select from. You can skip loading any template  now by selecting <b>Cancel</b> from the next dialog. If you wish to add more templates later, you can restart this operation by selecting <b>File/Import/Account Templates</b>."),
                i18n("Load predefined accounts/categories"));
  slotLoadAccountTemplates();

  slotStatusMsg(prevMsg);
  updateCaption();

  emit fileLoaded(fileName);
}

// General open
void KMyMoney2App::slotFileOpen()
{
  QString prevMsg = slotStatusMsg(i18n("Open a file."));

  KFileDialog* dialog = new KFileDialog(KGlobalSettings::documentPath(),
                            i18n("%1|KMyMoney files\n%2|All files (*.*)").arg("*.kmy *.xml").arg("*.*"),
                            this, i18n("Open File..."), true);
  dialog->setMode(KFile::File | KFile::ExistingOnly);

  if(dialog->exec() == QDialog::Accepted) {
    slotFileOpenRecent(dialog->selectedURL());
  }
  delete dialog;

  slotStatusMsg(prevMsg);
}

void KMyMoney2App::slotFileOpenRecent(const KURL& url)
{
  QString prevMsg = slotStatusMsg(i18n("Loading file..."));
  KURL lastFile = fileName;

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

  if(m_startLogo)
    delete m_startLogo;

  if(!duplicate) {

#if KDE_IS_VERSION(3,2,0)
    if(url.isValid() && KIO::NetAccess::exists(url, true, this)) {
#else
    if(url.isValid() && KIO::NetAccess::exists(url)) {
#endif
      slotFileClose();
      if(!myMoneyView->fileOpen()) {
        if(myMoneyView->readFile(url)) {
          fileName = url;
          fileOpenRecent->addURL( url );
          writeLastUsedFile(url.url());

          // Check the schedules
          slotCheckSchedules();
        }

        updateCaption();
        emit fileLoaded(fileName);
      }
    } else {
      slotFileClose();
      KMessageBox::sorry(this, QString("<p>")+i18n("<b>%1</b> is either an invalid filename or the file does not exist. You can open another file or create a new one.").arg(url.url()), i18n("File not found"));
    }
  } else {
    KMessageBox::sorry(this, QString("<p>")+i18n("File <b>%1</b> is already opened in another instance of KMyMoney").arg(url.url()), i18n("Duplicate open"));
    return;
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

  if (fileName.isEmpty()) {
    rc = slotFileSaveAs();
    slotStatusMsg(prevMsg);
    return rc;
  }

  rc = myMoneyView->saveFile(fileName);

  slotStatusMsg(prevMsg);
  updateCaption();
  return rc;
}

const bool KMyMoney2App::slotFileSaveAs()
{
  bool rc = false;

  QString prevMsg = slotStatusMsg(i18n("Saving file with a new filename..."));

  QString newName=KFileDialog::getSaveFileName(readLastUsedDir(),//KGlobalSettings::documentPath(),
                                               i18n("*.kmy|KMyMoney files\n""*.xml|XML Files\n""*.ANON.xml|Anonymous Files\n"

                                               "*.*|All files"), this, i18n("Save as..."));

  //
  // If there is no file extension, then append a .kmy at the end of the file name.
  // If there is a file extension, make sure it is .kmy, delete any others.
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
    // name, or remember it!
    if (newName.right(9).lower() == ".anon.xml")
    {
      rc = myMoneyView->saveFile(newName);
    }
    else
    {

      QFileInfo saveAsInfo(newName);

      fileName = newName;
      rc = myMoneyView->saveFile(newName);

      //write the directory used for this file as the default one for next time.
      writeLastUsedDir(newName);
      writeLastUsedFile(newName);
    }
  }

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

  myMoneyView->closeFile();
  fileName = KURL();
  updateCaption();

  emit fileLoaded(fileName);
}

void KMyMoney2App::slotFileQuit()
{
  QString prevMsg = slotStatusMsg(i18n("Exiting..."));

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

  // in case we don't really quit
  slotStatusMsg(prevMsg);
}

void KMyMoney2App::slotViewToolBar()
{
  QString prevMsg = slotStatusMsg(i18n("Toggling toolbar..."));
  ///////////////////////////////////////////////////////////////////
  // turn Toolbar on or off
  if(!viewToolBar->isChecked())
  {
    toolBar("mainToolBar")->hide();
  }
  else
  {
    toolBar("mainToolBar")->show();
  }

  slotStatusMsg(prevMsg);
}

void KMyMoney2App::slotViewStatusBar()
{
  QString prevMsg = slotStatusMsg(i18n("Toggle the statusbar..."));
  ///////////////////////////////////////////////////////////////////
  //turn Statusbar on or off
  if(!viewStatusBar->isChecked())
  {
    statusBar()->hide();
  }
  else
  {
    statusBar()->show();
  }

  slotStatusMsg(prevMsg);
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

    KMessageBox::information(this, i18n("No MyMoneyFile open"));
    return;
  }

  myMoneyView->viewPersonal();

  slotStatusMsg(prevMsg);
}

void KMyMoney2App::slotFileFileInfo()
{
  if ( !myMoneyView->fileOpen() ) {

    KMessageBox::information(this, i18n("No MyMoneyFile open"));

    return;
  }

  int answer = KMessageBox::warningYesNoCancel(this, i18n("This function is used by the developers to\n\nperform a dump of the engine's data in memory."));
  if (answer == KMessageBox::Cancel || answer == KMessageBox::No)
    return;


  myMoneyView->memoryDump();
}

void KMyMoney2App::slotLoadAccountTemplates(void)
{
  QString prevMsg = slotStatusMsg(i18n("Importing account templates."));

  // create a dialog that drops the user in the base directory for templates
  KFileDialog* dialog = new KFileDialog(KGlobal::dirs()->findResourceDir("appdata", "templates/README")+"templates",
                                        i18n("*.kmt|Account templates"),
                                        this, "defaultaccounts",
                                        true);
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

void KMyMoney2App::loadAccountTemplates(const QStringList& filelist)
{
  QStringList::ConstIterator it;
  for(it = filelist.begin(); it != filelist.end(); ++it) {
    MyMoneyTemplate templ;
    if(templ.loadTemplate(*it)) {
      templ.import(&progressCallback);
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
  slotStatusMsg(i18n("Ready."));

  // re-enable all standard widgets
  setEnabled(true);
}

void KMyMoney2App::slotOfxImport(void)
{
#if defined(HAVE_LIBOFX) || defined(HAVE_NEW_OFX)
  QString prevMsg = slotStatusMsg(i18n("Importing a statement from OFX"));

  KFileDialog* dialog = new KFileDialog(KGlobalSettings::documentPath(),
                            i18n("%1|OFX files\n%2|All files (*.*)").arg("*.ofx").arg("*.*"),
                            this, i18n("Import OFX Statement..."), true);
  dialog->setMode(KFile::File | KFile::ExistingOnly);

  if(dialog->exec() == QDialog::Accepted)
  {
    slotOfxStatementImport(dialog->selectedURL().path());
  }
  slotStatusMsg(prevMsg);
#else
  KMessageBox::information( this, QString("<p>")+i18n("<b>OFX</b> import is unavailable.  This version of <b>KMyMoney</b> was built without <b>OFX</b> support."), i18n("Function not available"));
#endif
}

void KMyMoney2App::slotGncImport(void)
{
  QString prevMsg = slotStatusMsg(i18n("Importing a Gnucash file."));

  KFileDialog* dialog = new KFileDialog(KGlobalSettings::documentPath(),
                            i18n("%1|Gnucash files\n%2|All files (*.*)").arg("*.xac").arg("*.*"),
                            this, i18n("Import Gnucash file..."), true);
  dialog->setMode(KFile::File | KFile::ExistingOnly);

  if(dialog->exec() == QDialog::Accepted) {
    slotFileClose();
    if(myMoneyView->fileOpen())
      return;

    // call the importer
    myMoneyView->readFile(dialog->selectedURL());
    // imported files don't have a name
    fileName = KURL();

    updateCaption();
    emit fileLoaded(fileName);
  }
  delete dialog;

  slotStatusMsg(prevMsg);
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

bool KMyMoney2App::slotOfxStatementImport(const MyMoneyOfxStatement& ofx)
{
  bool hasstatements = (ofx.begin() != ofx.end());
  bool ok = true;
  bool abort = false;

  if ( ofx.errors().count() )
  {
    if ( KMessageBox::warningContinueCancelList(this,i18n("The following errors were returned from your bank"),ofx.errors(),i18n("OFX Errors")) == KMessageBox::Cancel )
      abort = true;
  }

  if ( ofx.warnings().count() )
  {
    if ( KMessageBox::warningContinueCancelList(this,i18n("The following warnings were returned from your bank"),ofx.warnings(),i18n("OFX Warnings"),KStdGuiItem::cont(),"ofxwarnings") == KMessageBox::Cancel )
      abort = true;
  }

  QValueList<MyMoneyStatement>::const_iterator it_s = ofx.begin();
  while ( it_s != ofx.end() && !abort )
  {
    ok = ok && slotStatementImport(*it_s);
    ++it_s;
  }
  return hasstatements && ok && !abort;
}

bool KMyMoney2App::slotOfxStatementImport(const QString& url)
{
  bool result = false;
  MyMoneyOfxStatement s( url );

  if ( s.isValid() )
    result = slotOfxStatementImport(s);
  else
    QMessageBox::critical( this, i18n("Invalid OFX"), i18n("Error importing %1: This file is not a valid OFX file.").arg(url), QMessageBox::Ok, 0 );

  return result;
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
  slotStatusMsg(i18n("Ready."));

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
  KSettingsDlg dlg( this, "Settings");
  connect(&dlg, SIGNAL(signalApply()), myMoneyView, SLOT(slotRefreshViews()));
  // terminate any edit session
  myMoneyView->slotCancelEdit();
  if( dlg.exec() )
  {
    myMoneyView->slotRefreshViews();


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

      fileName = url.url();
      slotFileOpenRecent(url);
    } else { // Wizard / Template
      fileName = start.getURL();
    }

    //save off directory as the last one used.
    if(fileName.isLocalFile() && fileName.hasPath())
    {
      writeLastUsedDir(fileName.path(0));
      writeLastUsedFile(fileName.path(0));
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





  if(!fileName.isLocalFile()) {
    KMessageBox::sorry(this,
                       i18n("The current implementation of the backup functionality only supports local files as source files! Your current source file is '%1'.")
                            .arg(fileName.url()),

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
    mountpoint = backupDlg->txtMountPoint->text();

    if (m_backupMount) {
      progressCallback(0, 300, i18n("Mounting %1").arg(mountpoint));
      proc << "mount";
      proc << mountpoint;
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
        QString backupfile = mountpoint + "/" + fileName.fileName(false);
        KMyMoneyUtils::appendCorrectFileExt(backupfile, today);

        // check if file already exists and ask what to do
        m_backupResult = 0;
        QFile f(backupfile);
        if (f.exists()) {
          int answer = KMessageBox::warningContinueCancel(this, i18n("Backup file for today exists on that device.  Replace ?"), i18n("Backup"), i18n("&Replace"));
          if (answer == KMessageBox::Cancel) {
            m_backupResult = 1;

            if (m_backupMount) {
              progressCallback(250, 0, i18n("Unmounting %1").arg(mountpoint));
              proc.clearArguments();
              proc << "umount";
              proc << mountpoint;
              m_backupState = BACKUP_UNMOUNTING;
              proc.start();
            } else {
              m_backupState = BACKUP_IDLE;
              progressCallback(-1, -1, i18n("Ready."));
            }
          }
        }

        if(m_backupResult == 0) {
          progressCallback(50, 0, i18n("Writing %1").arg(backupfile));
          proc << "cp" << "-f" << fileName.path(0) << backupfile;
          m_backupState = BACKUP_COPYING;
          proc.start();
        }

      } else {
        KMessageBox::information(this, i18n("Error mounting device"), i18n("Backup"));
        m_backupResult = 1;
        if (m_backupMount) {
          progressCallback(250, 0, i18n("Unmounting %1").arg(mountpoint));
          proc.clearArguments();
          proc << "umount";
          proc << mountpoint;
          m_backupState = BACKUP_UNMOUNTING;
          proc.start();

        } else {
          m_backupState = BACKUP_IDLE;
          progressCallback(-1, -1, i18n("Ready."));
        }
      }
      break;

    case BACKUP_COPYING:
      if(proc.normalExit() && proc.exitStatus() == 0) {

        if (m_backupMount) {
          progressCallback(250, 0, i18n("Unmounting %1").arg(mountpoint));
          proc.clearArguments();
          proc << "umount";
          proc << mountpoint;
          m_backupState = BACKUP_UNMOUNTING;
          proc.start();
        } else {
          progressCallback(300, 0, i18n("Done"));
          KMessageBox::information(this, i18n("File successfully backed up"), i18n("Backup"));
          m_backupState = BACKUP_IDLE;
          progressCallback(-1, -1, i18n("Ready."));
        }
      } else {
        qDebug("cp exit status is %d", proc.exitStatus());
        m_backupResult = 1;
        KMessageBox::information(this, i18n("Error copying file to device"), i18n("Backup"));

        if (m_backupMount) {
          progressCallback(250, 0, i18n("Unmounting %1").arg(mountpoint));
          proc.clearArguments();
          proc << "umount";
          proc << mountpoint;
          m_backupState = BACKUP_UNMOUNTING;
          proc.start();


        } else {
          m_backupState = BACKUP_IDLE;
          progressCallback(-1, -1, i18n("Ready."));
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
      progressCallback(-1, -1, i18n("Ready."));
      break;

    default:
      qWarning("Unknown state for backup operation!");
      progressCallback(-1, -1, i18n("Ready."));
      break;
  }
}

void KMyMoney2App::slotFileNewWindow()
{
  KMyMoney2App *newWin = new KMyMoney2App;

  newWin->show();
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

void KMyMoney2App::slotSetViewSpecificActions(int view)
{
  filePrint->setEnabled(false);
  switch(view) {
    case KMyMoneyView::ReportsView:
      filePrint->setEnabled(true);
      break;
    default:
      break;
  }
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

bool KMyMoney2App::verifyImportedData(const MyMoneyAccount& account)
{
  bool rc;
  KImportVerifyDlg *dialog = new KImportVerifyDlg(account, this);
  dialog->setProgressCallback(progressCallback);
  rc = (dialog->exec() == QDialog::Accepted);
  delete dialog;
  return rc;
}

void KMyMoney2App::updateCaption(const bool skipActions)
{
  QString caption;

  caption = fileName.filename(false);

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
  }

  QString sizeMsg;
  sizeMsg.sprintf(" (%d x %d)", width(), height());
  caption += sizeMsg;

  caption = kapp->makeStdCaption(caption, false, modified);
  if(caption.length() > 0)
    caption += " - ";
  caption += "KMyMoney";
  setPlainCaption(caption);

  if(!skipActions) {
    fileSave->setEnabled(modified);
    fileSaveAs->setEnabled(myMoneyView->fileOpen());
    filePersonalData->setEnabled(myMoneyView->fileOpen());
    fileBackup->setEnabled(myMoneyView->fileOpen());
    fileViewInfo->setEnabled(myMoneyView->fileOpen());
    actionFindTransaction->setEnabled(myMoneyView->fileOpen());
    actionQifExport->setEnabled(myMoneyView->fileOpen());
    actionQifImport->setEnabled(myMoneyView->fileOpen());
    actionOfxImport->setEnabled(myMoneyView->fileOpen());
    actionGncImport->setEnabled(myMoneyView->fileOpen());
    actionLoadTemplate->setEnabled(myMoneyView->fileOpen());
    bankAdd->setEnabled(myMoneyView->fileOpen());
    accountAdd->setEnabled(myMoneyView->fileOpen());
    myMoneyView->enableViews();
  }
}

void KMyMoney2App::update(const QCString& /* id */)
{
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
  QString prevMsg = slotStatusMsg(i18n("Running consitency check..."));

  QStringList msg = MyMoneyFile::instance()->consistencyCheck();

  KMessageBox::warningContinueCancelList(0, "Result", msg, i18n("Consitency check result"));

  slotStatusMsg(prevMsg);
  updateCaption();
}

void KMyMoney2App::slotCheckSchedules(void)
{
  KConfig *kconfig = KGlobal::config();
  kconfig->setGroup("Schedule Options");
  if(kconfig->readBoolEntry("CheckSchedules", false) == true) {

    QString prevMsg = slotStatusMsg(i18n("Checking for overdue schedules..."));
    MyMoneyFile *file = MyMoneyFile::instance();
    QDate checkDate = QDate::currentDate().addDays(kconfig->readNumEntry("CheckSchedulePreview", 0));

    QValueList<MyMoneySchedule> scheduleList =  file->scheduleList();
    QValueList<MyMoneySchedule>::Iterator it;

    for (it=scheduleList.begin(); it!=scheduleList.end(); ++it)
    {
      // Get the copy in the file because it might be modified by commitTransaction
      MyMoneySchedule schedule = file->schedule((*it).id());

      if (schedule.autoEnter())
      {
        while ((schedule.nextPayment(schedule.lastPayment()) < checkDate) && !schedule.isFinished())
        {
          if (schedule.isFixed())
          {
            //qDebug("Auto Entering schedule: %s", schedule.name().latin1());
            //qDebug("\tAuto enter date: %s", schedule.nextPayment(schedule.lastPayment()).toString().latin1());
            slotCommitTransaction(schedule, schedule.nextPayment(schedule.lastPayment()));
          }
          else
          {
            // 0.8 will feature a list of schedules for a better ui
            KEnterScheduleDialog *dlg = new KEnterScheduleDialog(this, schedule, schedule.nextPayment(schedule.lastPayment()));
            if (!dlg->exec())
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
    slotStatusMsg(prevMsg);
    updateCaption();
  }
}

void KMyMoney2App::slotCommitTransaction(const MyMoneySchedule& sched, const QDate& date)
{
  MyMoneySchedule schedule = sched;
  QDate schedDate;

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

QString KMyMoney2App::readLastUsedDir()
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

QString KMyMoney2App::readLastUsedFile()
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
    myMoneyView->slotAccountNew();
  }
}

const QString KMyMoney2App::filename() const
{
  return fileName.url();
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

void KMyMoney2App::slotAccountNew(void)
{
  Q_CHECK_PTR(myMoneyView);
  myMoneyView->slotAccountNew();
}

void KMyMoney2App::ofxWebConnect(const QString& url, const QCString& asn_id)
{
  // Bring this window to the forefront.  This method was suggested by
  // Lubos Lunak <l.lunak@suse.cz> of the KDE core development team.

#if KDE_IS_VERSION(3,2,0)
  KStartupInfo::setNewStartupId(this,asn_id);
#else
  QCString keepCompilerHappy1(asn_id);
  KWin::setActiveWindow( winId() );
#endif

#if defined(HAVE_LIBOFX) || defined(HAVE_NEW_OFX)
  // Make sure we have an open file
  if ( ! myMoneyView->fileOpen() &&
    KMessageBox::warningContinueCancel(kmymoney2, i18n("You must first select a KMyMoney file before you can import a statement.")) == KMessageBox::Continue )
      kmymoney2->slotFileOpen();

  // only continue if the user really did open a file.
  if ( myMoneyView->fileOpen() )
  {
    QString prevMsg = slotStatusMsg(i18n("Importing a statement from OFX"));

    if ( MyMoneyOfxStatement::isOfxFile( url ) )
      slotOfxStatementImport(url);
    else if ( MyMoneyStatement::isStatementFile( url ) )
      slotStatementImport(url);
  }

#else
  KMessageBox::information( this, QString("<p>")+i18n("<b>OFX</b> import is unavailable.  This version of <b>KMyMoney</b> was built without <b>OFX</b> support."), i18n("Function not available"));
  QString keepCompilerHappy2(url);
#endif

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
                << service->name() << "' service" << endl;
      kdDebug() << KLibLoader::self()->lastErrorMessage() << endl;
    }
  }
}
