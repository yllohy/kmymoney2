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

// ----------------------------------------------------------------------------
// QT Includes

#include <qapplication.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kapplication.h>
#include <kmainwindow.h>
#include <kaccel.h>
#include <kaction.h>
#include <kprocess.h>
#include <kurl.h>
#include <dcopobject.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoney/mymoneyobserver.h"
#include "mymoney/mymoneyscheduled.h"
#include "mymoney/mymoneyinstitution.h"

class QSignalMapper;
class KProgress;
class KMyMoneyView;
class MyMoneyQifReader;
class MyMoneyStatementReader;
class MyMoneyStatement;
class IMyMoneyStorage;
class KFindTransactionDlg;

namespace KMyMoneyPlugin { class ImporterPlugin; }

/*! \mainpage KMyMoney Main Page for API documentation.
 *
 * \section intro Introduction
 *
 * This is the API documentation for KMyMoney.  It should be used as a reference
 * for KMyMoney developers and users who wish to see how KMyMoney works.  This
 * documentation will be kept up-to-date as development progresses and should be
 * read for new features that have been developed in KMyMoney.
 *
 * The latest version of this document is available from the project's web-site
 * at http://kmymoney2.sourceforge.net/ and is generated daily by doxygen reading
 * the header files found in the CVS main branch.
 */

/**
  * The base class for KMyMoney application windows. It sets up the main
  * window and reads the config file as well as providing a menubar, toolbar
  * and statusbar.  All functionality is passed down to KMyMoneyView.
  *
  * @see KMyMoneyView
  *
  * @author Michael Edwardes 2000-2001
  *
  * @short Main application class.
  */
class KMyMoney2App : public KMainWindow, MyMoneyObserver, public DCOPObject
{
  Q_OBJECT
  K_DCOP

protected slots:
  /**
    * This slot re-enables all message for which the "Don't show again"
    * option had been selected.
    */
  void slotEnableMessages(void);

  void slotKeySettings();
  void slotEditToolbars();
  void slotNewToolBarConfig();
  /**
    * Called when the user asks for file information.
    */
  void slotFileFileInfo();

  void slotPerformanceTest(void);

  /**
    * Called when the user asks for the personal information.
    */
  void slotFileViewPersonal();

  /**
    * Called when the user wishes to import tab delimeted transactions
    * into the current account.  An account must be open for this to
    * work.  Calls KMyMoneyView::slotAccountImportAscii.
    *
    * @see MyMoneyAccount
    */
  void slotQifImport();

  /**
    * Called when a QIF import is finished.
    */
  void slotQifImportFinished(void);

  /**
    * Opens a file selector dialog for the user to choose an existing OFX
    * file from the file system to be imported.  This slot is expected to
    * be called from the UI.
    */
  void slotGncImport(void);

  void slotPluginImport(const QString&);

  void slotPluginImport(const QString& format, const QString& url);

  void slotAccountUpdateOFX(void);

  /**
    * Opens a file selector dialog for the user to choose an existing KMM
    * statement file from the file system to be imported.  This is for testing
    * only.  KMM statement files are not designed to be exposed to the user.
    */
  void slotStatementImport(void);

  void slotStatementImportFinished(void);

  void slotLoadAccountTemplates(void);
  void slotSaveAccountTemplates(void);

  void loadAccountTemplates(const QStringList& filelist);

  /**
    * Called when the user wishes to export some transaction to a
    * QIF formatted file. An account must be open for this to work.
    * Uses MyMoneyQifWriter() for the actual output.
    */
  void slotQifExport();

  /**
    * Open up the application wide settings dialog.
    *
    * @see KSettingsDlg
    */
  void slotSettings();

  /** No descriptions */
  void slotFileBackup();

  /**
    * This slot modifies the actions according to the specific view passed
    * as argument @p view.
    */
  void slotSetViewSpecificActions(int view);

  void slotShowTipOfTheDay(void);

  void slotQifProfileEditor(void);

  void slotShowPreviousView(void);

  void slotShowNextView(void);

  void slotSecurityEditor(void);

  /**
    * Brings up a dialog to let the user search for specific transaction(s).  It then
    * opens a results window to display those transactions.
    */
  void slotFindTransaction();

  /**
    * Destroys a possibly open the search dialog
    */
  void slotCloseSearchDialog(void);

  /**
    * Brings up the input dialog and saves the information.
    */
  void slotInstitutionNew(void);

  /**
    * Preloads the input dialog with the data of the current
    * selected institution and brings up the input dialog
    * and saves the information entered.
    */
  void slotInstitutionEdit(void);

  /**
    * Deletes the current selected institution.
    */
  void slotInstitutionDelete(void);

  /**
    * Creates a new institution entry in the MyMoneyFile engine
    *
    * @param institution MyMoneyInstitution object containing the data of
    *                    the institution to be created.
    */
  void slotInstitutionNew(MyMoneyInstitution institution);

  /**
    * Brings up the new account wizard and saves the information.
    */
  void slotAccountNew(void);

  /**
    * Brings up the new category editor and saves the information.
    */
  void slotCategoryNew(void);

  /**
    * Calls the print logic for the current view
    */
  void slotPrintView(void);

  /**
    * Create a new investment
    */
  void slotInvestmentNew(void);

  /**
    * This slot opens the investment editor to edit the currently
    * selected investment if possible
    */
  void slotInvestmentEdit(void);

  /**
    * Deletes the current selected investment.
    */
  void slotInvestmentDelete(void);

  /**
    * Performs online update for currently selected investment
    */
  void slotOnlinePriceUpdate(void);

  /**
    * Performs manual update for currently selected investment
    */
  void slotManualPriceUpdate(void);

public:
  /**
    * This method checks if there is at least one asset or liability account
    * in the current storage object. If not, it starts the new account wizard.
    */
  void createInitialAccount(void);

  /**
    * This method returns the last URL used or an empty URL
    * depending on the option setting if the last file should
    * be opened during startup or the open file dialog should
    * be displayed.
    *
    * @return URL of last opened file or empty if the program
    *         should start with the open file dialog
    */
  const KURL lastOpenedURL(void);

  /**
    * construtor of KMyMoney2App, calls all init functions to create the application.
    */
  KMyMoney2App(QWidget* parent=0, const char* name=0);

  /**
    * Destructor
    */
  ~KMyMoney2App();

  /** Init wizard dialog */
  bool initWizard();

  static void progressCallback(int current, int total, const QString&);

  void writeLastUsedDir(const QString& directory);
  QString readLastUsedDir() const;
  void writeLastUsedFile(const QString& fileName);
  QString readLastUsedFile() const;

  /**
    * Returns whether there is an importer available that can handle this file
    */
  bool isImportableFile( const KURL& url );

  /**
    * This function will be called by the engine when the engine data changed
    * and the application object needs to update it's state.
    */
  virtual void update(const QCString& id);

  /**
    * This method is used to update the caption of the application window.
    * It set's the caption to "filename [modified] - KMyMoney".
    *
    * @param skipActions if true, the actions will not be updated. This
    *                    is usually onyl required by some early calls when
    *                    these widgets are not yet created (the default is false).
    */
  void updateCaption(bool skipActions = false);

  /**
    * This method returns a list of all 'other' dcop registered kmymoney processes.
    * It's a subset of the return of DCOPclient()->registeredApplications().
    *
    * @retval QCStringList of process ids
    */
  const QCStringList instanceList(void) const;

  void selectAccount(const MyMoneyAccount& account = MyMoneyAccount());

  void selectInstitution(const MyMoneyInstitution& institution = MyMoneyInstitution());

  void selectInvestment(const MyMoneyAccount& account = MyMoneyAccount());

  void selectSchedule(const MyMoneySchedule& schedule = MyMoneySchedule());

  /**
    * Dump a list of the names of all defined KActions to stdout.
    */
  void dumpActions(void) const;

  /**
    * Popup the context menu with the respective @p containerName.
    * Valid container names are defined in kmymoney2ui.rc
    */
  void showContextMenu(const QString& containerName);

k_dcop:
  const QString filename() const;

  void webConnect(const QString&, const QCString& asn_id);

protected:
  /** save general Options like all bar positions and status as well as the geometry and the recent file list to the configuration
   * file
   */
  void saveOptions();

  /**
    * Creates the interfaces necessary for the plugins to work. Therefore,
    * this method must be called prior to loadPlugins().
    */
  void createInterfaces(void);

  /**
    * load all available plugins. Make sure you have called createInterfaces()
    * before you call this one.
    */
  void loadPlugins(void);

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

  void slotCheckSchedules(void);

  virtual void resizeEvent(QResizeEvent*);

  void createSchedule(MyMoneySchedule newSchedule, MyMoneyAccount& newAccount);

  void createAccount(MyMoneyAccount& newAccount, MyMoneyAccount& parentAccount, MyMoneyAccount& brokerageAccount, MyMoneyMoney openingBal, MyMoneySchedule& schedule);

  /**
    * This method updates all KAction items to the current state.
    */
  void updateActions(void);

public slots:
  void slotFileInfoDialog(void);

  /** */
  void slotFileNew();

  /** Open a new window */
  void slotFileNewWindow();

  /** open a file and load it into the document*/
  void slotFileOpen();

  /** opens a file from the recent files menu */

  void slotFileOpenRecent(const KURL& url);

  /**
    * saves the current document. If it has no name yet, the user
    * will be queried for it.
    *
    * @retval false save operation failed
    * @retval true save operation was successful
    */
  const bool slotFileSave();

  /**
    * ask the user for the filename and save the current document
    *
    * @retval false save operation failed
    * @retval true save operation was successful
    */
  const bool slotFileSaveAs();

  /** asks for saving if the file is modified, then closes the actual file and window */
  void slotFileCloseWindow();

  /** asks for saving if the file is modified, then closes the actual file */
  void slotFileClose();

  /**
    * closes all open windows by calling close() on each memberList item
    * until the list is empty, then quits the application.
    * If queryClose() returns false because the user canceled the
    * saveModified() dialog, the closing breaks.
    */
  void slotFileQuit();

  void slotFileConsitencyCheck(void);

  /**
    * fires up the price table editor
    */
  void slotPriceDialog(void);

  /**
    * fires up the currency table editor
    */
  void slotCurrencyDialog(void);

  /**
    * toggles the toolbar
    */
  void slotViewToolBar();

  /**
    * toggles the statusbar
    */
  void slotViewStatusBar();

  /**
    * changes the statusbar contents for the standard label permanently,
    * used to indicate current actions. Returns the previous value for
    * 'stacked' usage.
    *
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

  /**
    * Called to update stock and currency prices from the user menu
    */
  void slotEquityPriceUpdate();

  /**
    * Imports a KMM statement into the engine, triggering the appropriate
    * UI to handle account matching, payee creation, and someday
    * payee and transaction matching.
    */
  bool slotStatementImport(const MyMoneyStatement& s);

  /**
    * Essentially similiar to the above slot, except this will load the file
    * from disk first, given the URL.
    */
  bool slotStatementImport(const QString& url);

  /**
    * Essentially similiar to the above slot, except this imports the whole
    * list of statements
    */
  bool slotStatementImport(const QValueList<MyMoneyStatement>& list);

  /**
    * This slot starts the reconciliation of the currently selected account
    */
  void slotAccountReconcile(void);

  /**
    * This slot deletes the currently selected account if possible
    */
  void slotAccountDelete(void);

  /**
    * This slot opens the account editor to edit the currently
    * selected account if possible
    */
  void slotAccountEdit(void);

  /**
    * This slot opens the selected account in the ledger view
    */
  void slotAccountOpen(void);

  /**
    * This slot opens the account options menu at the current cursor
    * position.
    */
  void slotShowAccountContextMenu(void);

  /**
    * This slot opens the institution options menu at the current cursor
    * position.
    */
  void slotShowInstitutionContextMenu(void);

  /**
    * This slot opens the investment options menu at the current cursor
    * position.
    */
  void slotShowInvestmentContextMenu(void);

  /**
    * This slot collects information for a new schedule transaction (bill)
    * and saves it in the engine
    */
  void slotScheduleNewBill(void);

  /**
    * This slot collects information for a new schedule transaction (deposit)
    * and saves it in the engine
    */
  void slotScheduleNewDeposit(void);

  /**
    * This slot collects information for a new schedule transaction (transfer)
    * and saves it in the engine
    */
  void slotScheduleNewTransfer(void);

  /**
    * This slot allows to edit information the currently selected schedule
    */
  void slotScheduleEdit(void);

  /**
    * This slot allows to delete the currently selected schedule
    */
  void slotScheduleDelete(void);

  /**
    * This slot allows to enter the next scheduled transaction of
    * the currently selected schedule
    */
  void slotScheduleEnter(void);

  /**
    * This slot fires up the KCalc application
    */
  void slotToolsStartKCalc(void);

private:
  bool verifyImportedData(const MyMoneyAccount& account);
  bool slotCommitTransaction(const MyMoneySchedule& schedule, const QDate&);

  /**
    * Load the status bar with the 'ready' message. This is hold in a single
    * place, so that is consistent with isReady().
    */
  void ready(void);

  /**
    * Check if the status bar contains the 'ready' message. The return
    * value is used e.g. to detect if a quit operation is allowed or not.
    *
    * @retval true application is idle
    * @retval false application is active working on a longer operation
    */
  bool isReady(void);

  void KMyMoney2App::scheduleNew(const QCString& scheduleType);

signals:
  /**
    * This signal is emitted when a new file is loaded. In the case file
    * is closed, this signal is also emitted with an empty url.
    */
  void fileLoaded(const KURL& url);

  /**
    * This signal is emitted when a new account has been selected by
    * the GUI. If no account is selected or the selection is removed,
    * account is identical to MyMoneyAccount(). This signal is used
    * by plugins to get information about changes.
    */
  void accountSelected(const MyMoneyAccount& account);

  /**
    * This signal is emitted when a new institution has been selected by
    * the GUI. If no institution is selected or the selection is removed,
    * institution is identical to MyMoneyInstitution(). This signal is used
    * by plugins to get information about changes.
    */
  void institutionSelected(const MyMoneyInstitution& institution);

public:
  /**
    * This method retrieves a pointer to a KAction object from actionCollection().
    * If the action with the name @p actionName is not found, a pointer to
    * a static non-configured KAction object is returned and a warning is
    * printed to stderr.
    *
    * @param actionName name of the action to be retrieved
    * @return pointer to KAction object (or derivative)
    */
  KAction* action(const QString& actionName) const;

  /**
    * This method is implemented for convience. It returns a dynamic_cast-ed
    * pointer to an action found in actionCollection().
    * If the action with the name @p actionName is not found or the object
    * is not of type KToggleAction, a pointer to a static non-configured
    * KToggleAction object is returned and a warning is printed to stderr.
    */
  KToggleAction* toggleAction(const QString& actionName) const;


private:
  /** the configuration object of the application */

  KConfig *config;

  QSignalMapper *m_pluginSignalMapper;
  QMap<QString,KMyMoneyPlugin::ImporterPlugin*> m_importerPlugins;

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

  KProcess proc;

  /// A pointer to the view holding the tabs.
  KMyMoneyView *myMoneyView;

  /// The URL of the file currently being edited when open.
  KURL  fileName;

  bool m_startDialog;
  QString mountpoint;

  KProgress* progressBar;

  QString m_statusMsg;

  int m_progressUpdate;
  int m_nextUpdate;

  IMyMoneyStorage*  m_engineBackup;
  MyMoneyQifReader* m_reader;
  MyMoneyStatementReader* m_smtReader;
  KFindTransactionDlg* m_searchDlg;

  bool m_bCheckSchedules;

  KToolBarPopupAction*  m_previousViewButton;
  KToolBarPopupAction*  m_nextViewButton;

  QObject*              m_pluginInterface;

  MyMoneyAccount        m_selectedAccount;
  MyMoneyAccount        m_selectedInvestment;
  MyMoneyInstitution    m_selectedInstitution;
  MyMoneySchedule       m_selectedSchedule;
};

extern  KMyMoney2App *kmymoney2;


#endif // KMYMONEY2_H
