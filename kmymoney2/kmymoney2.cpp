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

#include <stdio.h>
#include <iostream>

// ----------------------------------------------------------------------------
// QT Includes

#include <qapp.h>
#include <qdir.h>
#include <qprinter.h>
#include <qpainter.h>
#include <qlayout.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kshortcut.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kmenubar.h>
#include <klocale.h>
#include <kconfig.h>
#include <kstdaction.h>
#include <kglobal.h>
#if QT_VERSION > 300
#include <kstandarddirs.h>
#include <kstatusbar.h>
#else
#include <kstddirs.h>
#endif
#include <ktip.h>
#include <kkeydialog.h>
#include <kprogress.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoney2.h"
#include "kstartuplogo.h"

#include "dialogs/kstartdlg.h"
#include "dialogs/ksettingsdlg.h"
#include "dialogs/kbackupdlg.h"
#include "dialogs/kexportdlg.h"
#include "dialogs/kimportdlg.h"
#include "dialogs/mymoneyqifprofileeditor.h"
#include "dialogs/kimportverifydlg.h"

#include "views/kmymoneyview.h"

#include "mymoney/mymoneyutils.h"

#include "converter/mymoneyqifwriter.h"
#include "converter/mymoneyqifreader.h"

#include "kmymoneyutils.h"

#define ID_STATUS_MSG 1

KMyMoney2App::KMyMoney2App(QWidget * /*parent*/ , const char* name)
 : KMainWindow(0, name)
{
  m_startLogo = new KStartupLogo;
  config=kapp->config();
  config->setGroup("General Options");

  // splash screen setting
  bool bViewSplash = config->readBoolEntry("Show Splash", true);
  if(bViewSplash)
    m_startLogo->show();

  QFrame* frame = new QFrame(this);
  frame->setFrameStyle(QFrame::NoFrame);
  // values for margin (11) and spacing(6) taken from KDialog implementation
  QBoxLayout* layout = new QBoxLayout(frame, QBoxLayout::TopToBottom, 11, 6);

  myMoneyView = new KMyMoneyView(frame);
  layout->addWidget(myMoneyView, 10);

  ///////////////////////////////////////////////////////////////////
  // call inits to invoke all other construction parts
  initStatusBar();
  initActions();
  readOptions();

  // setCentralWidget(myMoneyView);
  setCentralWidget(frame);

  connect(myMoneyView, SIGNAL(signalHomeView()), this, SLOT(slotHomeView()));
  connect(myMoneyView, SIGNAL(signalAccountsView()), this, SLOT(slotAccountsView()));
  connect(myMoneyView, SIGNAL(signalScheduledView()), this, SLOT(slotScheduledView()));
  connect(myMoneyView, SIGNAL(signalCategoryView()), this, SLOT(slotCategoryView()));
  connect(myMoneyView, SIGNAL(signalPayeeView()), this, SLOT(slotPayeeView()));

  connect(&proc,SIGNAL(processExited(KProcess *)),this,SLOT(slotProcessExited()));

  m_backupState = BACKUP_IDLE;

  m_reader = 0;
  m_engineBackup = 0;

  // make sure, we get a note when the engine changes state
  MyMoneyFile::instance()->attach(MyMoneyFile::NotifyClassAnyChange, this);
}

KMyMoney2App::~KMyMoney2App()
{
  MyMoneyFile::instance()->detach(MyMoneyFile::NotifyClassAnyChange, this);
  
  delete m_startLogo;
  if(m_reader != 0)
    delete m_reader;
  if(m_engineBackup != 0)
    delete m_engineBackup;
}

bool KMyMoney2App::startWithDialog(void)
{
  m_startLogo->close();
  slotStatusMsg(i18n("Ready."));
  return m_startDialog;
}

void KMyMoney2App::readFile(void)
{
  QString prevMsg = slotStatusMsg(i18n("Loading file..."));

  myMoneyView->readFile(fileName);

  slotStatusMsg(prevMsg);
  updateCaption();
}

void KMyMoney2App::initActions()
{
  fileOpen = KStdAction::open(this, SLOT(slotFileOpen()), actionCollection());
  fileOpenRecent = KStdAction::openRecent(this, SLOT(slotFileOpenRecent(const KURL&)), actionCollection());
  fileSave = KStdAction::save(this, SLOT(slotFileSave()), actionCollection());
  fileSaveAs = KStdAction::saveAs(this, SLOT(slotFileSaveAs()), actionCollection());
  fileClose = KStdAction::close(this, SLOT(slotFileClose()), actionCollection());
  fileQuit = KStdAction::quit(this, SLOT(slotFileQuit()), actionCollection());
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
  actionQifExport = new KAction(i18n("QIF ..."), "", 0, this, SLOT(slotQifExport()), actionCollection(), "file_export_qif");

  // The Settings Menu
  settingsKey = KStdAction::keyBindings(this, SLOT(slotKeySettings()), actionCollection());
  settings = KStdAction::preferences(this, SLOT( slotSettings() ), actionCollection());

  // The Bank Menu
  bankAdd = new KAction(i18n("Add new institution..."), "bank", 0, myMoneyView, SLOT(slotBankNew()), actionCollection(), "bank_add");

  // The Account Menu
  accountAdd = new KAction(i18n("Add new account..."), "account"/*QIconSet(QPixmap(KGlobal::dirs()->findResource("appdata", "toolbar/kmymoney_newacc.xpm")))*/, 0, myMoneyView, SLOT(slotAccountNew()), actionCollection(), "account_add");

  // The tool menu
  new KAction(i18n("QIF Profile Editor..."), "edit", 0, this, SLOT(slotQifProfileEditor()), actionCollection(), "qif_editor");
  
  // The help menu
  new KAction(i18n("&Show tip of the day"), "idea", 0, this, SLOT(slotShowTipOfTheDay()), actionCollection(), "show_tip");

#if QT_VERSION < 300
  fileNewWindow->setStatusText(i18n("Creates a new window"));
  fileOpen->setStatusText(i18n("Opens an existing document"));
  fileOpenRecent->setStatusText(i18n("Opens a recently used file"));


  fileSave->setStatusText(i18n("Saves the actual document"));
  fileSaveAs->setStatusText(i18n("Saves the actual document as..."));
  fileClose->setStatusText(i18n("Closes the actual document"));
  fileCloseWindow->setStatusText(i18n("Closes the actual window"));
  fileQuit->setStatusText(i18n("Quits the application"));
  viewToolBar->setStatusText(i18n("Enables/disables the toolbar"));
  viewStatusBar->setStatusText(i18n("Enables/disables the statusbar"));
  fileViewInfo->setStatusText(i18n("View information about the file"));
  filePersonalData->setStatusText(i18n("Lets you view/edit your personal data"));
  fileBackup->setStatusText(i18n("Lets you backup your file to a removeable drive"));
  bankAdd->setStatusText(i18n("Lets you create a new institution"));
  accountOpen->setStatusText(i18n("View the account register"));
  accountAdd->setStatusText(i18n("Lets you create a new account"));
  actionQifImport->setStatusText(i18n("Import transactions using QIF format"));
  actionQifExport->setStatusText(i18n("Export transactions using QIF format"));
#endif


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
  config->writeEntry("LastFile", fileName.url());
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
  if (!m_startDialog)
    fileName = config->readEntry("LastFile");
}

bool KMyMoney2App::queryClose()
{
  if (myMoneyView->dirty()) {
    int ans = KMessageBox::warningYesNoCancel(this, i18n("KMyMoney file needs saving.  Save ?"));
    if (ans==KMessageBox::Cancel)
      return false;
    else if (ans==KMessageBox::Yes)
      slotFileSave();
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

  if (myMoneyView->fileOpen()) {
#if QT_VERSION > 300

    int answer = KMessageBox::warningContinueCancel(this, i18n("KMyMoney file already open.  Close it ?"), "Close File"/*, "Close", "dont_ask_again"*/);
#else
    int answer = KMessageBox::warningContinueCancel(this, i18n("KMyMoney file already open.  Close it ?"), "Close File", "Close", "dont_ask_again");
#endif
    if (answer==KMessageBox::Cancel) {
      slotStatusMsg(prevMsg);
      return;
    }
    slotFileClose();
  }
  fileName = KURL();
  myMoneyView->newFile();
  slotStatusMsg(prevMsg);
}

// General open
void KMyMoney2App::slotFileOpen()


{
  QString prevMsg = slotStatusMsg(i18n("Open a document."));

  if (myMoneyView->fileOpen()) {
#if QT_VERSION > 300

    int answer = KMessageBox::warningContinueCancel(this, i18n("KMyMoney file already open.  Close it ?"), "Close File"/*, "Close", "dont_ask_again"*/);
#else
    int answer = KMessageBox::warningContinueCancel(this, i18n("KMyMoney file already open.  Close it ?"), "Close File", "Close", "dont_ask_again");
#endif
    if (answer==KMessageBox::Cancel) {
      slotStatusMsg(prevMsg);
      return;
    }
    slotFileClose();
  }
  fileName = KURL();
  initWizard();
  slotStatusMsg(prevMsg);
  updateCaption();
}

void KMyMoney2App::slotFileOpenRecent(const KURL& url)
{
  QString prevMsg = slotStatusMsg(i18n("Loading file..."));

  if (myMoneyView->fileOpen()) {
#if QT_VERSION > 300
    int answer = KMessageBox::warningContinueCancel(this, i18n("KMyMoney file already open.  Close it ?"), "Close File"/*, "Close", "dont_ask_again"*/);
#else
    int answer = KMessageBox::warningContinueCancel(this, i18n("KMyMoney file already open.  Close it ?"), "Close File", "Close", "dont_ask_again");
#endif

    if (answer==KMessageBox::Cancel) {
      slotStatusMsg(prevMsg);
      return;
    }
    slotFileClose();
  }

  myMoneyView->readFile( url );
  fileName = url;
  fileOpenRecent->addURL( url );

  slotStatusMsg(prevMsg);
}

void KMyMoney2App::slotFileSave()
{
  QString prevMsg = slotStatusMsg(i18n("Saving file..."));

  if (fileName.isEmpty()) {
    slotFileSaveAs();
    slotStatusMsg(prevMsg);
    return;
  }

  myMoneyView->saveFile(fileName);

  slotStatusMsg(prevMsg);
  updateCaption();
}

void KMyMoney2App::slotFileSaveAs()
{
  QString prevMsg = slotStatusMsg(i18n("Saving file with a new filename..."));

  QString newName=KFileDialog::getSaveFileName(KGlobalSettings::documentPath(),
                                               i18n("*.kmy|KMyMoney files\n"

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
      if(strExt.find("kmy", 0, FALSE) == -1)

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

    QFileInfo saveAsInfo(newName);
//    addRecentFile(newName);

    fileName = newName;
    myMoneyView->saveFile(newName);
  }

  slotStatusMsg(prevMsg);
  updateCaption();
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
      m_reader->setProgressCallback(&progressCallback);

      m_reader->startImport();
    }
    delete dlg;
  }
}

void KMyMoney2App::slotQifImportFinished(void)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  
  if(m_reader != 0) {
    // fixme: re-enable the QIF import menu options
    if(m_reader->finishImport()) {
      if(verifyImportedData()) {
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
    

    myMoneyView->suspendUpdate(false);
    // update the views as they might still contain invalid data
    // from the import session
    myMoneyView->slotRefreshViews();
    
    // slotStatusMsg(prevMsg);
    delete m_reader;
    m_reader = 0;
    slotStatusProgressBar(-1, -1);
    slotStatusMsg(i18n("Ready."));

  }
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
      return true;

    } else {
      // cancel clicked so post an exit call
      return false;
    }
}
/** No descriptions */
void KMyMoney2App::slotFileBackup()
{
  if(!fileName.isLocalFile()) {
    KMessageBox::sorry(this,
                       i18n("The current immplementation of the backup functionality "
                            "only supports local files as source files! "
                            "Your current source file is '%1'.)").arg(fileName.url()),
                       i18n("Local files only"));
    return;
  }

  if (myMoneyView->dirty()) {
    int answer = KMessageBox::warningYesNoCancel(this, i18n("The file has been changed, save it ?"));
    if (answer == KMessageBox::Cancel)
      return;
    else if (answer == KMessageBox::Yes)
      slotFileSave();



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
  QString path = KGlobal::dirs()->findResource("appdata", "kmymoney2ui.rc");
  KKeyDialog::configureKeys(actionCollection(), path);
}

void KMyMoney2App::slotHomeView()
{
}

void KMyMoney2App::slotAccountsView()
{
}

void KMyMoney2App::slotScheduledView()
{
}

void KMyMoney2App::slotCategoryView()
{
}

void KMyMoney2App::slotPayeeView()
{
}

void KMyMoney2App::slotShowTipOfTheDay(void)
{
  KTipDialog::showTip(myMoneyView, "", true);
}

void KMyMoney2App::slotQifProfileEditor(void)
{
  MyMoneyQifProfileEditor* editor = new MyMoneyQifProfileEditor(true, this, "QIF Profile Editor");

  editor->exec();

  delete editor;

}

bool KMyMoney2App::verifyImportedData()
{
  bool rc;
  QDialog *dialog = new KImportVerifyDlg(m_reader->account(), this);
  rc = (dialog->exec() == QDialog::Accepted);
  delete dialog;
  return rc;
}

void KMyMoney2App::updateCaption(void)
{
  QString caption;

  caption = kapp->makeStdCaption(fileName.filename(false), false, MyMoneyFile::instance()->dirty());
  caption += " - KMyMoney";
  setPlainCaption(caption);

  fileSave->setEnabled(MyMoneyFile::instance()->dirty());
}

void KMyMoney2App::update(const QCString& /* id */)
{
  updateCaption();
}
