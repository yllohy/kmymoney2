/***************************************************************************
                          kmymoney2.h
                             -------------------
    copyright            : (C) 2000-2001 by Michael Edwardes
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
#ifndef KMYMONEY2_H
#define KMYMONEY2_H
 
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qapplication.h>

// include files for KDE
#if QT_VERSION > 300
#include <kapplication.h>
#else
#include <kapp.h>
#endif

#include <kmainwindow.h>
#include <kaccel.h>
#include <kaction.h>
#include <kprocess.h>

class KProgress;
class KStartupLogo;

#include "views/kmymoneyview.h"

/*! \mainpage KMyMoney2 Main Page for API documentation.
 *
 * \section intro Introduction
 *
 * This is the API documentation for KMyMoney2.  It should be used as a reference
 * for KMyMoney2 developers and users who wish to see how KMyMoney2 works.  This
 * documentation will be kept up-to-date as development progresses and should be
 * read for new features that have been developed in KMyMoney2.
 *
 * The latest version of this document is available from the project's web-site
 * at http://kmymoney2.sourceforge.net/ and is generated daily by doxygen reading
 * the header files found in the CVS main branch.
 */

/**
  * The base class for KMyMoney2 application windows. It sets up the main
  * window and reads the config file as well as providing a menubar, toolbar
  * and statusbar.  All functionality is passed down to KMyMoneyView.
  *
  * @see KMyMoneyView
  *
  * @author Michael Edwardes 2000-2001
  * $Id: kmymoney2.h,v 1.28 2003/02/26 19:41:46 ipwizard Exp $
  *
  * @short Main application class.
**/
class KMyMoney2App : public KMainWindow
{
  Q_OBJECT

protected slots:
  void slotKeySettings();

  /**
    * Called when the user asks for file information.
  **/
  void slotFileFileInfo();

  /**
    * Called when the user asks for the personal information.
  **/
  void slotFileViewPersonal();

  /**
    * Called when the user wishes to add a bank.  The file must
    * be open for this to work. Calls KMyMoneyView::slotBankNew.
    *
    * @see MyMoneyFile
  **/
  void slotBankAdd();

  /**
    * Called when the user wishes to add an account.  A bank
    * must exist for this to work.  Calls KMyMoneyView::slotAccountNew.
    *
    * @see MyMoneyBank
  **/
  void slotAccountAdd();

  /**
    * Called when the user wishes to reconcile an account. An
    * account must be 'open' for this to work.
    * Calls KMyMoneyView::slotAccountReconcile
    *
    * @see MyMoneyAccount
  **/
  void slotAccountReconcile();

  /**
    * Called when the user wishes to import tab delimeted transactions
    * into the current account.  An account must be open for this to
    * work.  Calls KMyMoneyView::slotAccountImportAscii.
    *
    * @see MyMoneyAccount
    **/
  void slotQifImport();

  /**
    * Called when the user wishes to export some transaction to a
    * QIF formatted file. An account must be open for this to work.
    * Uses MyMoneyQifWriter() for the actual output.
    **/
  void slotQifExport();

  /**
    * Called when the user wishes to add a recurring transaction/bill/deposit.
    * A bank and account must be specified for this to work.
    *
    * @see MyMoneyBank
    * @see MyMoneyAccount
  **/
  void slotBillsAdd();

  // Not implemeted, not documented!
  void slotReportBasic();
  void slotPluginLoad();
  void slotPluginUnload();
  void slotPluginList();

  /**
    * A slot that is connected to the
    * @ref KMyMoneyView::fileOperations
    * signal.  It enables or disables some KActions depending
    * upon the argument enable.
    *
    * @param enable If true enable all the KActions otherwise disable them
    *
    * @see KMyMoneyView
  **/
  //void enableFileOperations(bool enable=true);

  /**
    * A slot that is connected to the
    * @ref KMyMoneyView::bankOperations
    * signal.  It enables or disables some KActions depending
    * upon the argument enable.
    *
    * @param enable If true enable all the KActions otherwise disable them
    *
    * @see KMyMoneyView
  **/
  //void enableBankOperations(bool enable=true);

  /**
    * A slot that is connected to the
    * @ref KMyMoneyView::accountOperations
    * signal.  It enables or disables some KActions depending
    * upon the argument enable.
    *
    * @param enable If true enable all the KActions otherwise disable them
    *
    * @see KMyMoneyView
  **/
  //void enableAccountOperations(bool enable=true);

  /**
    * A slot that is connected to the
    * @ref KMyMoneyView::transactionOperations
    * signal.  It enables or disables some KActions depending
    * upon the argument enable.
    *
    * @param enable If true enable all the KActions otherwise disable them
    *
    * @see KMyMoneyView
  **/
  //void enableTransactionOperations(bool enable=true);

  /**
    * Open up the application wide settings dialog.
    *
    * @see KSettingsDlg
  **/
  void slotSettings();

  /**
    * Called when the user wishes to search for specific
    * transactions.  Brings up a dialog to get the options
    * and then opens a results window.
    *
    * @see KFindTransactionDlg
    * @see KTFindResultsDlg
  **/
  void slotAccountFind();

  /**
    * Simulates moving up from the transaction view to the bank/account
    * view.  It just hides the transaction view and shows the bank view.
    * Checks where the user is before moving 'up'.
    *
    * @see KBanksView
    * @see KTransactionView
  **/
  void slotViewUp();
  /** No descriptions */
  void slotFileBackup();

  void slotHomeView();
  void slotAccountsView();
  void slotScheduledView();
  void slotCategoryView();
  void slotPayeeView();

  void slotEnableKMyMoneyOperations(bool enable=true);

  void slotShowTipOfTheDay(void);
  void slotQifProfileEditor(void);

public:
  /**
    * This method is used to select the setting for the startup with
    * dialog. As a side effect, it closes the splash screen that is
    * shown during program start.
    *
    * @return true if dialog request, false if start with last file requested
    */
  bool startWithDialog(void);

  /**
    * construtor of KMyMoney2App, calls all init functions to create the application.
    */
  KMyMoney2App(QWidget* parent=0, const char* name=0);

  /**
    * Destructor
    **/
  ~KMyMoney2App();

  /** Init wizard dialog */
  bool initWizard();

  /** initial file loader */
  void readFile(void);

protected:
  /** save general Options like all bar positions and status as well as the geometry and the recent file list to the configuration
   * file
   */
  void saveOptions();

  /** read general Options again and initialize all variables like the recent file list
   */
  void readOptions();

  /** initializes the KActions of the application */
  void initActions();

  /** sets up the statusbar for the main window by initialzing a statuslabel.
   */
  void initStatusBar();

  /** queryClose is called by KTMainWindow on each closeEvent of a window. Against the
   * default implementation (only returns true), this calles saveModified() on the document object to ask if the document shall
   * be saved if Modified; on cancel the closeEvent is rejected.
   * @see KTMainWindow#queryClose
   * @see KTMainWindow#closeEvent
   */
  virtual bool queryClose();

  /** queryExit is called by KTMainWindow when the last window of the application is going to be closed during the closeEvent().
   * Against the default implementation that just returns true, this calls saveOptions() to save the settings of the last window's
   * properties.
   * @see KTMainWindow#queryExit
   * @see KTMainWindow#closeEvent
   */
  virtual bool queryExit();

public slots:
  /** */
  void slotAccountOpen();

  /** Open a new window */
  void slotFileNewWindow();

  /** clears the document in the actual view to reuse it as the new document */
  void slotFileNew();

  /** open a file and load it into the document*/
  void slotFileOpen();

  /** opens a file from the recent files menu */
  void slotFileOpenRecent(const KURL& url);

  /** save a document */
  void slotFileSave();

  /** save a document by a new filename*/
  void slotFileSaveAs();

  /** asks for saving if the file is modified, then closes the actual file and window */
  void slotFileCloseWindow();

  /** asks for saving if the file is modified, then closes the actual file */
  void slotFileClose();

  /** print the actual file
    * TODO
  **/
  void slotFilePrint();

  /** closes all open windows by calling close() on each memberList item until the list is empty, then quits the application.
   * If queryClose() returns false because the user canceled the saveModified() dialog, the closing breaks.
   */
  void slotFileQuit();

  /** put the marked text/object into the clipboard and remove
   *	it from the document
   * TODO
  **/
  void slotEditCut();

  /** put the marked text/object into the clipboard
    * TODO
  **/
  void slotEditCopy();

  /** paste the clipboard into the document
    * TODO
  **/
  void slotEditPaste();

  /** toggles the toolbar
   */
  void slotViewToolBar();

  /** toggles the statusbar
   */
  void slotViewStatusBar();

  /**
    * changes the statusbar contents for the standard label permanently,
    * used to indicate current actions. Returns the previous value for
    * 'stacked' usage.
   * @param text the text that is displayed in the statusbar
   */
  const QString slotStatusMsg(const QString &text);

  /**
    * This method changes the progress bar in the status line according
    * to the parameters @p current and @p total. The following special
    * cases exist:
    *
    * - current = -1 and total = -1  will reset the progress bar
    * - current = ?? and total != 0  will setup the 100% mark to @p total
    * - current = xx and total == 0  will set the percentage
    *
    * @param current the current value with respect to the initialised
    *                 100% mark
    * @param total the total value (100%)
  */
  void slotStatusProgressBar(const int current, const int total = 0);

  /** No descriptions */
  void slotProcessExited();

private:
  /** the configuration object of the application */


  KConfig *config;

  // KAction pointers to enable/disable actions
  KAction *fileNewWindow;
  KAction* fileNew;
  KAction* fileOpen;
  KRecentFilesAction* fileOpenRecent;
  KAction* fileSave;
  KAction* fileSaveAs;
  KAction* fileBackup;
  KAction* fileClose;
  KAction* fileCloseWindow;
  KAction* filePrint;
  KAction* fileQuit;
  KAction* editCut;
  KAction* editCopy;
  KAction* editPaste;
  KToggleAction* viewToolBar;
  KToggleAction* viewStatusBar;
  KToggleAction* viewTransactionForm;

  KAction *fileViewInfo;
  KAction *filePersonalData;

	KAction *settings;
	KAction *settingsKey;

  KAction *bankAdd;

  KAction *accountOpen;
  KAction *accountAdd;
  KAction *accountReconcile;
  KAction *accountFind;
  KAction *actionQifImport;
  KAction *actionQifExport;

  KAction *billsAdd;

  KAction *reportBasic;

  KAction *pluginLoad;
  KAction *pluginUnload;
  KAction *pluginList;

  KAction *viewUp;

  enum backupStateE {
    BACKUP_IDLE = 0,
    BACKUP_MOUNTING,
    BACKUP_COPYING,
    BACKUP_UNMOUNTING
  };
  /**
    * The following variable represents the state while crafting a backup.
    * It can have the following values
    *
    * - IDLE: the default value if not performing a backup
    * - MOUNTING: when a mount command has been issued
    * - COPYING:  when a copy command has been issued
    * - UNMOUNTING: when an unmount command has been issued
    */
  backupStateE   m_backupState;

  /**
    * This variable keeps the result of the backup operation.
    */
  int     m_backupResult;

  /**
    * This variable is set, when the user selected to mount/unmount
    * the backup volume.
    */
  bool    m_backupMount;
  
      
/*
  bool mountbackup;
  bool copybackup;
  bool unmountbackup;
*/

  KProcess proc;
  // A pointer to the view holding the tabs.
  KMyMoneyView *myMoneyView;
  // The URL of the file currently being edited when open.
  KURL  fileName;

  bool m_startDialog;
  QString mountpoint;

  KProgress* progressBar;

  QString m_statusMsg;

  KStartupLogo* m_startLogo;

  int m_progressUpdate;

private:
  //void disableAllAccountActions(bool enable=true);

};

extern  KMyMoney2App *kmymoney2;


#endif // KMYMONEY2_H
