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

// include files for QT
#include <qapp.h>
#include <qdir.h>
#include <qprinter.h>
#include <qpainter.h>
#include <qlayout.h>

// include files for KDE
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
#else
#include <kstddirs.h>
#endif
#include <ktip.h>

#include <kkeydialog.h>

#include <stdio.h>
#include <iostream>
// application specific includes
#include "kmymoney2.h"
#include "dialogs/kstartdlg.h"
#include "dialogs/ksettingsdlg.h"
#include "kstartuplogo.h"
#include "dialogs/kbackupdlg.h"

#if QT_VERSION > 300
#include <kstatusbar.h>
#endif

#define ID_STATUS_MSG 1

KMyMoney2App::KMyMoney2App(QWidget *parent , const char* name)
 : KMainWindow(0, name)
{
  KStartupLogo *start_logo = new KStartupLogo;
  config=kapp->config();
  config->setGroup("General Options");

  // splash screen setting
  bool bViewSplash = config->readBoolEntry("Show Splash", true);
  if(bViewSplash)
    start_logo->show();

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

  ///////////////////////////////////////////////////////////////////
  // disable actions at startup
/* Future
  editCut->setEnabled(false);
  editCopy->setEnabled(false);
  editPaste->setEnabled(false);
*/

  // setCentralWidget(myMoneyView);
  setCentralWidget(frame);
/*
  connect(myMoneyView, SIGNAL(fileOperations(bool)), this, SLOT(enableFileOperations(bool)));
  connect(myMoneyView, SIGNAL(bankOperations(bool)), this, SLOT(enableBankOperations(bool)));
  connect(myMoneyView, SIGNAL(accountOperations(bool)), this, SLOT(enableAccountOperations(bool)));
  connect(myMoneyView, SIGNAL(transactionOperations(bool)), this, SLOT(enableTransactionOperations(bool)));
*/
  connect(myMoneyView, SIGNAL(signalEnableKMyMoneyOperations(bool)), this, SLOT(slotEnableKMyMoneyOperations(bool)));

  connect(myMoneyView, SIGNAL(signalHomeView()), this, SLOT(slotHomeView()));
  connect(myMoneyView, SIGNAL(signalAccountsView()), this, SLOT(slotAccountsView()));
  connect(myMoneyView, SIGNAL(signalScheduledView()), this, SLOT(slotScheduledView()));
  connect(myMoneyView, SIGNAL(signalCategoryView()), this, SLOT(slotCategoryView()));
  connect(myMoneyView, SIGNAL(signalPayeeView()), this, SLOT(slotPayeeView()));

  //enableFileOperations(false);
  //enableBankOperations(false);
  //enableAccountOperations(false);
  //enableTransactionOperations(false);
  slotEnableKMyMoneyOperations(false);
  connect(&proc,SIGNAL(processExited(KProcess *)),this,SLOT(slotProcessExited()));
  mountbackup = false;
  copybackup = false;
  unmountbackup = false;

  KTipDialog::showTip(myMoneyView, "", false);

  if (!m_startDialog)
    myMoneyView->readFile(fileName);
}

KMyMoney2App::~KMyMoney2App()
{
}

void KMyMoney2App::initActions()
{
  fileNew = KStdAction::openNew(this, SLOT(slotFileNew()), actionCollection());
  fileNewWindow = new KAction(i18n("New &Window"), "filenew", 0, this, SLOT(slotFileNewWindow()), actionCollection(), "file_new_window");
  fileOpen = KStdAction::open(this, SLOT(slotFileOpen()), actionCollection());
  fileOpenRecent = KStdAction::openRecent(this, SLOT(slotFileOpenRecent(const KURL&)), actionCollection());
  fileSave = KStdAction::save(this, SLOT(slotFileSave()), actionCollection());
  fileSaveAs = KStdAction::saveAs(this, SLOT(slotFileSaveAs()), actionCollection());
/* Future
  filePrint = KStdAction::print(this, SLOT(slotFilePrint()), actionCollection());
*/
  fileClose = KStdAction::close(this, SLOT(slotFileClose()), actionCollection());
  fileCloseWindow = new KAction(i18n("&Close Window"), "close_window", 0, this, SLOT(slotFileCloseWindow()), actionCollection(), "file_close_window");

  fileQuit = KStdAction::quit(this, SLOT(slotFileQuit()), actionCollection());

/*  Future
  editCut = KStdAction::cut(this, SLOT(slotEditCut()), actionCollection());
  editCopy = KStdAction::copy(this, SLOT(slotEditCopy()), actionCollection());
  editPaste = KStdAction::paste(this, SLOT(slotEditPaste()), actionCollection());
*/
  viewToolBar = KStdAction::showToolbar(this, SLOT(slotViewToolBar()), actionCollection());
  viewStatusBar = KStdAction::showStatusbar(this, SLOT(slotViewStatusBar()), actionCollection());

  // Setup transaction detail switch
  viewTransactionForm = new KToggleAction(i18n("Show Transaction Detail"), KShortcut("Ctrl+T"), actionCollection(), "show_transaction_detail");
  config->setGroup("List Options");
  if(config->readBoolEntry("ShowRegisterDetailed", true) == true)
    viewTransactionForm->setChecked(true);
  else
    viewTransactionForm->setChecked(false);

  connect(viewTransactionForm, SIGNAL(toggled(bool)), myMoneyView, SLOT(slotShowTransactionDetail(bool)));

  // Additions to the file menu
  fileViewInfo = new KAction(i18n("Dump Memory..."), "view_info", 0, this, SLOT(slotFileFileInfo()), actionCollection(), "file_view_info");
  filePersonalData = new KAction(i18n("Personal Data..."), "personal_data", 0, this, SLOT(slotFileViewPersonal()), actionCollection(), "file_personal_data");
  fileBackup = new KAction(i18n("Backup..."), "backup",0,this,SLOT(slotFileBackup()),actionCollection(),"file_backup");

  // The Settings Menu
	settingsKey = KStdAction::keyBindings(this, SLOT(slotKeySettings()), actionCollection());
	settings 	= KStdAction::preferences(this, SLOT( slotSettings() ), actionCollection());

  // The Bank Menu
  bankAdd = new KAction(i18n("Add new institution..."), "bank", 0, this, SLOT(slotBankAdd()), actionCollection(), "bank_add");

  // The Account Menu
  accountOpen = new KAction(i18n("Open account register..."), "account_open", 0, this, SLOT(slotAccountOpen()), actionCollection(), "account_open");
  accountAdd = new KAction(i18n("Add new account..."), "account"/*QIconSet(QPixmap(KGlobal::dirs()->findResource("appdata", "toolbar/kmymoney_newacc.xpm")))*/, 0, this, SLOT(slotAccountAdd()), actionCollection(), "account_add");
  accountReconcile = new KAction(i18n("Reconcile account..."), "reconcile", 0, this, SLOT(slotAccountReconcile()), actionCollection(), "account_reconcile");

  accountFind = new KAction(i18n("Find transaction..."), "transaction_find", 0, this, SLOT(slotAccountFind()), actionCollection(), "account_find");
  accountImport = new KAction(i18n("Import transactions..."), "transaction_import", 0, this, SLOT(slotAccountImport()), actionCollection(), "account_import");
  accountExport = new KAction(i18n("Export transactions..."), "transaction_export", 0, this, SLOT(slotAccountExport()), actionCollection(), "account_export");

  // The Bill Menu
/* Future
  billsAdd = new KAction(i18n("Add Bill/Deposit..."), 0, 0, this, SLOT(slotBillsAdd()), actionCollection(), "bills_add");
  billsAdd->setStatusText(i18n("Add a new Bill or Deposit"));

  // The Report Menu
  reportBasic = new KAction(i18n("Basic report..."), 0, 0, this, SLOT(slotReportBasic()), actionCollection(), "report_basic");
  reportBasic->setStatusText(i18n("Basic report on your transactions"));

  // The Plugin Menu
  pluginLoad = new KAction(i18n("Load plugin..."), 0, 0, this, SLOT(slotPluginLoad()), actionCollection(), "plugin_load");
  pluginLoad->setStatusText(i18n("Load a plugin enabling a new feature"));
  pluginUnload = new KAction(i18n("Unload plugin..."), 0, 0, this, SLOT(slotPluginUnload()), actionCollection(), "plugin_unload");
  pluginUnload->setStatusText(i18n("Unlaod a plugin disabling that feature"));
  pluginList = new KAction(i18n("List plugins..."), 0, 0, this, SLOT(slotPluginList()), actionCollection(), "plugin_list");
  pluginList->setStatusText(i18n("View all plugins and/or add new ones"));
*/
  // The help menu
  KAction* showTip = new KAction(i18n("&Show tip of the day"), "idea", 0, this, SLOT(slotShowTipOfTheDay()), actionCollection(), "show_tip");

  // For the toolbar only
//  viewUp = new KAction(i18n("Move view up..."), QIconSet(QPixmap(KGlobal::dirs()->findResource("appdata", "toolbar/kmymoney_up.xpm"))), 0, this, SLOT(slotViewUp()), actionCollection(), "view_up");
  viewUp = KStdAction::back(this, SLOT(slotViewUp()), actionCollection());

#if QT_VERSION < 300
  fileNew->setStatusText(i18n("Creates a new document"));
  fileNewWindow->setStatusText(i18n("Creates a new window"));
  fileOpen->setStatusText(i18n("Opens an existing document"));
  fileOpenRecent->setStatusText(i18n("Opens a recently used file"));
  fileSave->setStatusText(i18n("Saves the actual document"));
  fileSaveAs->setStatusText(i18n("Saves the actual document as..."));
  fileClose->setStatusText(i18n("Closes the actual document"));
  fileCloseWindow->setStatusText(i18n("Closes the actual window"));
/* Future
  filePrint ->setStatusText(i18n("Prints out the actual document"));
*/
  fileQuit->setStatusText(i18n("Quits the application"));
/* Future
  editCut->setStatusText(i18n("Cuts the selected section and puts it to the clipboard"));
  editCopy->setStatusText(i18n("Copies the selected section to the clipboard"));
  editPaste->setStatusText(i18n("Pastes the clipboard contents to actual position"));
*/
  viewToolBar->setStatusText(i18n("Enables/disables the toolbar"));
  viewStatusBar->setStatusText(i18n("Enables/disables the statusbar"));

  fileViewInfo->setStatusText(i18n("View information about the file"));
  filePersonalData->setStatusText(i18n("Lets you view/edit your personal data"));
  fileBackup->setStatusText(i18n("Lets you backup your file to a removeable drive"));
  bankAdd->setStatusText(i18n("Lets you create a new institution"));
  accountOpen->setStatusText(i18n("View the account register"));
  accountAdd->setStatusText(i18n("Lets you create a new account"));
  accountReconcile->setStatusText(i18n("Balance your account"));
  accountFind->setStatusText(i18n("Find transactions"));
  accountImport->setStatusText(i18n("Import transactions"));
  accountExport->setStatusText(i18n("Export transactions"));
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
  slotStatusMsg(i18n("Creating new document..."));

  if (myMoneyView->fileOpen()) {
#if QT_VERSION > 300

    int answer = KMessageBox::warningContinueCancel(this, i18n("KMyMoney file already open.  Close it ?"), "Close File"/*, "Close", "dont_ask_again"*/);
#else
    int answer = KMessageBox::warningContinueCancel(this, i18n("KMyMoney file already open.  Close it ?"), "Close File", "Close", "dont_ask_again");
#endif
    if (answer==KMessageBox::Cancel) {
      slotStatusMsg(i18n("Ready"));
      return;
    }
    slotFileClose();
  }
  fileName = KURL();
  myMoneyView->newFile();
  slotStatusMsg(i18n("Ready."));
}

// General open
void KMyMoney2App::slotFileOpen()
{
  slotStatusMsg(i18n("Open a document."));

  if (myMoneyView->fileOpen()) {
#if QT_VERSION > 300

    int answer = KMessageBox::warningContinueCancel(this, i18n("KMyMoney file already open.  Close it ?"), "Close File"/*, "Close", "dont_ask_again"*/);
#else
    int answer = KMessageBox::warningContinueCancel(this, i18n("KMyMoney file already open.  Close it ?"), "Close File", "Close", "dont_ask_again");
#endif
		if (answer==KMessageBox::Cancel) {
      slotStatusMsg(i18n("Ready"));
      return;
    }
    slotFileClose();
  }
  fileName = KURL();
	initWizard();
  slotStatusMsg(i18n("Ready."));
}

void KMyMoney2App::slotFileOpenRecent(const KURL& url)
{
  slotStatusMsg(i18n("Opening file..."));

  if (myMoneyView->fileOpen()) {
#if QT_VERSION > 300
    int answer = KMessageBox::warningContinueCancel(this, i18n("KMyMoney file already open.  Close it ?"), "Close File"/*, "Close", "dont_ask_again"*/);
#else
    int answer = KMessageBox::warningContinueCancel(this, i18n("KMyMoney file already open.  Close it ?"), "Close File", "Close", "dont_ask_again");
#endif
		if (answer==KMessageBox::Cancel) {
      slotStatusMsg(i18n("Ready"));
      return;
    }
    slotFileClose();
  }

  myMoneyView->readFile( url );
  fileName = url;
	fileOpenRecent->addURL( url );

  slotStatusMsg(i18n("Ready."));
}

void KMyMoney2App::slotFileSave()
{
  slotStatusMsg(i18n("Saving file..."));

  if (fileName.isEmpty()) {
    slotFileSaveAs();
    slotStatusMsg(i18n("Ready"));
    return;
  }

  myMoneyView->saveFile(fileName);
	
  slotStatusMsg(i18n("Ready."));
}

void KMyMoney2App::slotFileSaveAs()
{
  slotStatusMsg(i18n("Saving file with a new filename..."));

  QString newName=KFileDialog::getSaveFileName(QDir::currentDirPath(),
                                               i18n("*.kmy|KMyMoney files\n"
                                               "*.*|All files"), this, i18n("Save as..."));

	//
	//If there is no file extension, then append a .kmy at the end of the file name.
	//If there is a file extension, make sure it is .kmy, delete any others.
	//
  if(!newName.isEmpty())
  {
		//find last . delminator
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

  slotStatusMsg(i18n("Ready."));
}

void KMyMoney2App::slotFileCloseWindow()
{
  slotStatusMsg(i18n("Closing window..."));

  if (myMoneyView->dirty()) {
    int answer = KMessageBox::warningYesNoCancel(this, i18n("The file has been changed, save it ?"));
		if (answer == KMessageBox::Cancel)
			return;
		else if (answer == KMessageBox::Yes)
      slotFileSave();
  }

  close();

  slotStatusMsg(i18n("Ready."));
}

void KMyMoney2App::slotFileClose()
{
  slotStatusMsg(i18n("Closing file..."));


  if (myMoneyView->dirty()) {
    int answer = KMessageBox::warningYesNoCancel(this, i18n("The file has been changed, save it ?"));
		if (answer == KMessageBox::Cancel)
			return;
		else if (answer == KMessageBox::Yes)
      slotFileSave();
  }

  myMoneyView->closeFile();
  fileName = KURL();

  slotStatusMsg(i18n("Ready."));
}

void KMyMoney2App::slotFilePrint()
{
  slotStatusMsg(i18n("Printing..."));
  KMessageBox::information(this, i18n("Sorry, unavailable at this time."));
/*
  QPrinter printer;
  if (printer.setup(this))
  {
    view->print(&printer);
  }
*/
  slotStatusMsg(i18n("Ready."));
}

void KMyMoney2App::slotFileQuit()
{
  slotStatusMsg(i18n("Exiting..."));


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

//  slotStatusMsg(i18n("Ready."));
}


void KMyMoney2App::slotEditCut()
{
  slotStatusMsg(i18n("Cutting selection..."));

  slotStatusMsg(i18n("Ready."));
}

void KMyMoney2App::slotEditCopy()
{
  slotStatusMsg(i18n("Copying selection to clipboard..."));

  slotStatusMsg(i18n("Ready."));
}

void KMyMoney2App::slotEditPaste()
{

  slotStatusMsg(i18n("Inserting clipboard contents..."));

  slotStatusMsg(i18n("Ready."));
}

void KMyMoney2App::slotViewToolBar()
{
  slotStatusMsg(i18n("Toggling toolbar..."));
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

  slotStatusMsg(i18n("Ready."));
}

void KMyMoney2App::slotViewStatusBar()
{
  slotStatusMsg(i18n("Toggle the statusbar..."));
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

  slotStatusMsg(i18n("Ready."));
}


void KMyMoney2App::slotStatusMsg(const QString &text)
{
  ///////////////////////////////////////////////////////////////////
  // change status message permanently
  statusBar()->clear();
  statusBar()->changeItem(text, ID_STATUS_MSG);
}

void KMyMoney2App::slotFileViewPersonal()
{
  if ( !myMoneyView->fileOpen() ) {
    KMessageBox::information(this, i18n("No MyMoneyFile open"));
    return;
  }

  slotStatusMsg(i18n("Viewing personal data..."));

  myMoneyView->viewPersonal();

  slotStatusMsg(i18n("Ready"));
}

void KMyMoney2App::slotFileFileInfo()
{
  if ( !myMoneyView->fileOpen() ) {
    KMessageBox::information(this, i18n("No MyMoneyFile open"));
    return;
  }

  int answer = KMessageBox::warningYesNoCancel(this, i18n("This function no longer exists and is used by the developers.\n\nPerform a dump of the data in memory?."));
	if (answer == KMessageBox::Cancel || answer == KMessageBox::No)
		return;

  myMoneyView->memoryDump();
}

void KMyMoney2App::slotBankAdd()
{
  myMoneyView->slotBankNew();
}

void KMyMoney2App::slotAccountAdd()
{
  myMoneyView->slotAccountNew();
}

void KMyMoney2App::slotAccountReconcile()
{
  myMoneyView->slotAccountReconcile();
}

void KMyMoney2App::slotAccountImport()
{
  myMoneyView->slotAccountImportAscii();
}

void KMyMoney2App::slotAccountExport()
{
  myMoneyView->slotAccountExportAscii();
}

void KMyMoney2App::slotBillsAdd()
{
  KMessageBox::information(this, "Placement holder for future addition...\nPlease wait, I'm on it.\nMichael (mte@users.sourceforge.net.");
}

void KMyMoney2App::slotReportBasic()
{
  KMessageBox::information(this, "Placement holder for future addition...\nPlease wait, I'm on it.\nMichael (mte@users.sourceforge.net.");
}

void KMyMoney2App::slotPluginLoad()
{
  KMessageBox::information(this, "Placement holder for future addition...\nPlease wait, I'm on it.\nMichael (mte@users.sourceforge.net.");
}

void KMyMoney2App::slotPluginUnload()
{
  KMessageBox::information(this, "Placement holder for future addition...\nPlease wait, I'm on it.\nMichael (mte@users.sourceforge.net.");
}

void KMyMoney2App::slotPluginList()
{
  KMessageBox::information(this, "Placement holder for future addition...\nPlease wait, I'm on it.\nMichael (mte@users.sourceforge.net.");
}

/*
void KMyMoney2App::enableFileOperations(bool enable)
{
  enableBankOperations(false);
  enableAccountOperations(false);
  enableTransactionOperations(false);

  fileClose->setEnabled(enable);
  fileSave->setEnabled(enable);
  fileSaveAs->setEnabled(enable);
/* Future
  filePrint->setEnabled(enable);
* /
  fileViewInfo->setEnabled(enable);
  filePersonalData->setEnabled(enable);
/* Future
  billsAdd->setEnabled(enable);
  reportBasic->setEnabled(enable);
* /
  categoriesEdit->setEnabled(enable);
  categoriesPayees->setEnabled(enable);
  settings->setEnabled(enable);
  fileBackup->setEnabled(enable);

//  fileNew->setEnabled(!enable);
//  fileOpen->setEnabled(!enable);
//  fileOpenRecent->setEnabled(!enable);
}

void KMyMoney2App::enableBankOperations(bool enable)
{
  if (enable) {
    enableFileOperations(true);
    enableAccountOperations(false);
    enableTransactionOperations(false);
  }

  // Make sure there is a bank selected before enabling
  // accountAdd
  if (myMoneyView->currentAccountName() == i18n("Unknown Account"))
    accountAdd->setEnabled(false);
  else
    accountAdd->setEnabled(enable);

  bankAdd->setEnabled(enable);
  //setCaption(myMoneyView->currentBankName());
}

void KMyMoney2App::enableAccountOperations(bool enable)
{
  if (enable) {
    enableFileOperations(true);
    enableBankOperations(false);
    enableTransactionOperations(false);
  }

//  accountAdd->setEnabled(false);
  accountOpen->setEnabled(enable);
  accountReconcile->setEnabled(enable);
  accountImport->setEnabled(enable);
  accountExport->setEnabled(enable);
  accountFind->setEnabled(enable);

  QString caption = myMoneyView->currentAccountName();
  setCaption(caption);
}
*/
void KMyMoney2App::slotViewUp()
{
  myMoneyView->viewUp();
}
/*
void KMyMoney2App::enableTransactionOperations(bool enable)
{
  if (enable) {
    enableFileOperations(true);
    enableBankOperations(false);
    enableAccountOperations(true);
  }

  bankAdd->setEnabled(false);
  accountAdd->setEnabled(false);
  accountOpen->setEnabled(false);

  viewUp->setEnabled(enable);

  QString caption = myMoneyView->currentAccountName();
  setCaption(caption);
}
*/
void KMyMoney2App::slotSettings()
{
  KSettingsDlg dlg( this, "Settings");
  connect(&dlg, SIGNAL(signalApply()), myMoneyView, SLOT(settingsLists()));
  if( dlg.exec() )
  {
    myMoneyView->settingsLists();
  }
}

void KMyMoney2App::slotAccountFind()
{
  myMoneyView->accountFind();
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
//  connect(backupDlg->btnOK,SIGNAL(clicked()),backupDlg,SLOT(accept()));
//  connect(backupDlg->btnCancel,SIGNAL(clicked()),backupDlg,SLOT(reject()));
//  backupDlg->txtMountPoint->setText(mountpoint);
  int returncode = backupDlg->exec();

  if(returncode)
  {
    // make sure the file doesn't exist already
    QString today;
    today.sprintf("-%d-%d-%d.kmy",QDate::currentDate().day(), QDate::currentDate().month(), QDate::currentDate().year());
    QFile f(mountpoint + fileName.url().mid(fileName.url().findRev("/")) + today);
    if (f.exists()) {
      int answer = KMessageBox::warningContinueCancel(this, i18n("Backup file for today exists on that device.  Replace ?"), i18n("Backup"), i18n("&Replace"));
      if (answer==KMessageBox::Cancel)

        return;
    }

    mountpoint = backupDlg->txtMountPoint->text();
    if (backupDlg->mountCheckBox->isChecked()) {
      mountbackup = true;
      copybackup = false;
      unmountbackup = false;

      proc.clearArguments();
      proc << "mount";
      proc << mountpoint;
      proc.start();
    } else {
      mountbackup = false;
      copybackup = false;
      unmountbackup = false;

      proc.clearArguments();
      QString backupfile = mountpoint + fileName.url().mid(fileName.url().findRev("/")) + today;
      proc << "cp" << "-f" << fileName.url() << backupfile;
      proc.start();
    }
  }

  delete backupDlg;
}

/** No descriptions */
void KMyMoney2App::slotProcessExited(){

	if(mountbackup)
   {
		if(proc.normalExit())
		{
			if(proc.exitStatus() == 0)
			{
				QString backupfile;
				proc.clearArguments();
				backupfile = mountpoint + fileName.url().mid(fileName.url().findRev("/"));
				proc << "cp";
  			proc << "-f" << fileName.url() << backupfile;
				proc.start();
				mountbackup = false;
       			copybackup = true;
				unmountbackup = false;
			}
			else
			{
    			QMessageBox::information(this, i18n("Backup"), i18n("Error Mounting Device"));
				mountbackup = false;
       			copybackup = false;
				unmountbackup = false;
			}
		}
	}
	else if(copybackup)
	{
			if(proc.exitStatus() == 0)
			{
				proc.clearArguments();
				proc << "umount";
    			proc << mountpoint;
				proc.start();
				mountbackup = false;
       			copybackup = false;
				unmountbackup = true;
			}
			else
			{
    			QMessageBox::information(this, i18n("Backup"), i18n("Error Copying File to Device"));
				mountbackup = false;
       			copybackup = false;
				unmountbackup = false;
			}
	}
	else if(unmountbackup)
	{
			if(proc.exitStatus() == 0)
			{
    			QMessageBox::information(this, i18n("Backup"), i18n("File Successfully Backed up"));
				mountbackup = false;
       			copybackup = false;
				unmountbackup = false;
			}
			else

			{
    			QMessageBox::information(this, i18n("Backup"), i18n("Error unmounting device"));
				mountbackup = false;
       			copybackup = false;
				unmountbackup = false;
			}
  } else {
    if(proc.exitStatus() == 0) {
      KMessageBox::information(this, i18n("File Successfully Backed Up"), i18n("Backup"));
    }
    else {
      KMessageBox::information(this, i18n("Error copying file"), i18n("Backup"));
    }
  }
}

void KMyMoney2App::slotFileNewWindow()
{
  KMyMoney2App *newWin = new KMyMoney2App;
  newWin->show();
}

void KMyMoney2App::slotAccountOpen()
{
  myMoneyView->slotAccountDoubleClick();
}

void KMyMoney2App::slotKeySettings()
{
  QString path = KGlobal::dirs()->findResource("appdata", "kmymoney2ui.rc");
  KKeyDialog::configureKeys(actionCollection(), path);
}

void KMyMoney2App::slotHomeView()
{
  //disableAllAccountActions();
}

void KMyMoney2App::slotAccountsView()
{
}
/*
void KMyMoney2App::disableAllAccountActions(bool enable)
{
  enableBankOperations(false);
  enableAccountOperations(false);
  enableTransactionOperations(false);
}
*/
void KMyMoney2App::slotScheduledView()
{
  //disableAllAccountActions();
}

void KMyMoney2App::slotCategoryView()
{
  //disableAllAccountActions();
}

void KMyMoney2App::slotPayeeView()
{

  //disableAllAccountActions();
}

void KMyMoney2App::slotEnableKMyMoneyOperations(bool enable)
{
}

void KMyMoney2App::slotShowTipOfTheDay(void)
{
  KTipDialog::showTip(myMoneyView, "", true);
}
