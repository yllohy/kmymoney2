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
class QTimer;

// ----------------------------------------------------------------------------
// KDE Includes

#include <kapplication.h>
#include <kmainwindow.h>
#include <kaccel.h>
#include <kaction.h>
#include <kprocess.h>
#include <kurl.h>
#include <dcopobject.h>
class KComboBox;

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoney/mymoneyobserver.h"
#include "mymoney/mymoneyscheduled.h"
#include <kmymoney/mymoneyinstitution.h>
#include <kmymoney/mymoneypayee.h>
#include <kmymoney/mymoneybudget.h>
#include <kmymoney/kmymoneyplugin.h>
#include <kmymoney/register.h>

class QSignalMapper;
class KProgress;
class KMyMoneyView;
class MyMoneyQifReader;
class MyMoneyStatementReader;
class MyMoneyStatement;
class IMyMoneyStorage;
class KFindTransactionDlg;
class TransactionEditor;
class KEndingBalanceDlg;

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
  * and statusbar.
  *
  * @see KMyMoneyView
  *
  * @author Michael Edwardes 2000-2001
  * @author Thomas Baumgart 2006
  *
  * @short Main application class.
  */
class KMyMoney2App : public KMainWindow, MyMoneyObserver, public DCOPObject
{
  Q_OBJECT
  K_DCOP

protected slots:
  void slotFileSaveAsFilterChanged(const QString& filter);

  /**
    * This slot is intended to be used as part of auto saving. This is used when the
    * QTimer emits the timeout signal and simply checks that the file is dirty (has
    * received modifications to it's contents), and call the apropriate method to
    * save the file. Furthermore, re-starts the timer (possibly not needed).
    * @author mvillarino 2005
    * @see KMyMoney2App::update(), MyMoneyFile::instance()->attach()
    */
  void slotAutoSave();

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
  // void slotSetViewSpecificActions(int view);

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
  void slotInstitutionEdit(const MyMoneyObject& obj = MyMoneyInstitution());

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
    * Brings up the new category editor and saves the information.
    * The dialog will be preset with the name and parent account.
    *
    * @param account reference of category to be created. The @p name member
    *                should be filled by the caller. The object will be filled
    *                with additional information during the creation process
    *                esp. the @p id member.
    * @param parent reference to parent account (defaults to none)
    *
    * @note Typically, this slot can be connected to the
    *       kMyMoneyCategory::newCategory(MyMoneyAccount&) signal.
    */
  void slotCategoryNew(MyMoneyAccount& account, const MyMoneyAccount& parent = MyMoneyAccount());

  /**
    * Brings up the new category editor and saves the information.
    * The dialog will be preset with the name. The parent defaults to
    * MyMoneyFile::expense()
    *
    * @param name Name of the account to be created. Could include a full hierarchy
    * @param id reference to storage which will receive the id after successful creation
    *
    * @note Typically, this slot can be connected to the
    *       StdTransactionEditor::createCategory(const QString&, QCString&) or
    *       KMyMoneyCombo::createItem(const QString&, QCString&) signal.
    */
  void slotCategoryNew(const QString& name, QCString& id);

  /**
    * Calls the print logic for the current view
    */
  void slotPrintView(void);

  /**
    * Create a new investment
    */
  void slotInvestmentNew(void);

  /**
    * Create a new investment in a given @p parent investment account
    */
  void slotInvestmentNew(MyMoneyAccount& account, const MyMoneyAccount& parent);

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

  /**
    * Call this slot, if any configuration parameter has changed
    */
  void slotUpdateConfiguration(void);

  /**
    */
  void slotPayeeNew(const QString& newnameBase, QCString& id);
  void slotPayeeNew(void);

  /**
    */
  void slotPayeeDelete(void);

  /**
    */
  void slotBudgetNew(void);

  /**
    */
  void slotBudgetDelete(void);

  /**
    */
  void slotNewUserWizard(void);

  /**
    */
  void slotTransactionsNew(void);

  /**
    */
  void slotTransactionsEdit(void);

  /**
    */
  void slotTransactionsEditSplits(void);

  /**
    */
  void slotTransactionsDelete(void);

  /**
    */
  void slotTransactionsEnter(void);

  /**
    */
  void slotTransactionsCancel(void);

  /**
    */
  void slotTransactionsCancelOrEnter(void);

  /**
    */
  void slotTransactionDuplicate(void);

  /**
    */
  void slotMarkTransactionCleared(void);

  /**
    */
  void slotMarkTransactionReconciled(void);

  /**
    */
  void slotMarkTransactionNotReconciled(void);

  /**
    */
  void slotTransactionGotoAccount(void);

  /**
    */
  void slotTransactionGotoPayee(void);

  /**
    */
  void slotTransactionCreateSchedule(void);

  /**
    */
  void slotTransactionAssignNumber(void);

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

  /**
    * Dump a list of the names of all defined KActions to stdout.
    */
  void dumpActions(void) const;

  /**
    * Popup the context menu with the respective @p containerName.
    * Valid container names are defined in kmymoney2ui.rc
    */
  void showContextMenu(const QString& containerName);

  /**
    * This method updates all KAction items to the current state.
    */
  void updateActions(void);

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

  void createCategory(MyMoneyAccount& account, const MyMoneyAccount& parent);

  /**
    * This method checks, if an account can be closed or not.
    *
    * @param acc reference to MyMoneyAccount object in question
    * @retval true account can be closed
    * @retval false account cannot be closed
    */
  bool canCloseAccount(const MyMoneyAccount& acc) const;

  /**
    * Check if a list contains a payee with a given id
    *
    * @param list const reference to value list
    * @param id const reference to id
    *
    * @retval true object has been found
    * @retval false object is not in list
    */
  bool payeeInList(const QValueList<MyMoneyPayee>& list, const QCString& id) const;

  /**
    * Mark the selected transactions as provided by @a flag.
    */
  void markTransaction(MyMoneySplit::reconcileFlagE flag);

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

  /** open a SQL database */
  void slotOpenDatabase();

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

  /**
   * ask the user to select a database and save the current document
   *
   * @retval false save operation failed
   * @retval true save operation was successful
   */
  const bool slotSaveAsDatabase();

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
    * Toggles the hide reconciled transactions setting
    */
  void slotHideReconciledTransactions(void);

  /**
    * Toggles the hide unused categories setting
    */
  void slotHideUnusedCategories(void);

  /**
    * Toggles the show all accounts setting
    */
  void slotShowAllAccounts(void);

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
  void slotAccountReconcileStart(void);

  /**
    * This slot finishes a previously started reconciliation
    */
  void slotAccountReconcileFinish(void);

  /**
    * This slot postpones a previously started reconciliations
    */
  void slotAccountReconcilePostpone(void);

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
  void slotAccountOpen(const MyMoneyObject& = MyMoneyAccount());

  /**
    * This slot closes the currently selected account if possible
    */
  void slotAccountClose(void);

  /**
    * This slot re-openes the currently selected account if possible
    */
  void slotAccountReopen(void);

  /**
    * This slot reparents account @p src to be a child of account @p dest
    *
    * @param src account to be reparented
    * @param dest new parent
    */
  void slotReparentAccount(const MyMoneyAccount& src, const MyMoneyAccount& dest);

  /**
    * This slot reparents account @p src to be a held at institution @p dest
    *
    * @param src account to be reparented
    * @param dest new parent institution
    */
  void slotReparentAccount(const MyMoneyAccount& src, const MyMoneyInstitution& dest);

  /**
    * This slot creates a transaction report for the selected account
    * and opens it in the reports view.
    */
  void slotAccountTransactionReport(void);

  /**
    * This slot opens the account options menu at the current cursor
    * position.
    */
  void slotShowAccountContextMenu(const MyMoneyObject&);

  /**
    * This slot opens the institution options menu at the current cursor
    * position.
    */
  void slotShowInstitutionContextMenu(const MyMoneyObject&);

  /**
    * This slot opens the investment options menu at the current cursor
    * position.
    */
  void slotShowInvestmentContextMenu(void);

  /**
    * This slot opens the payee options menu at the current cursor
    * position.
    */
  void slotShowPayeeContextMenu(void);

  /**
    * This slot opens the budget options menu at the current cursor
    * position.
    */
  void slotShowBudgetContextMenu(void);

  /**
    * This slot opens the transaction options menu at the current cursor
    * position.
    */
  void slotShowTransactionContextMenu(void);

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
   * This slot allows to skip the next scheduled transaction of
   * the currently selected schedule
   */
  void slotScheduleSkip(void);

  /**
    * This slot fires up the KCalc application
    */
  void slotToolsStartKCalc(void);

  void slotSelectAccount(const MyMoneyObject& account = MyMoneyAccount());

  void slotSelectInstitution(const MyMoneyObject& institution = MyMoneyInstitution());

  void slotSelectInvestment(const MyMoneyObject& account = MyMoneyAccount());

  void slotSelectSchedule(const MyMoneySchedule& schedule = MyMoneySchedule());

  void slotSelectPayees(const QValueList<MyMoneyPayee>& list);

  void slotSelectBudget(const QValueList<MyMoneyBudget>& list);

  void slotSelectTransactions(const QValueList<KMyMoneyRegister::SelectedTransaction>& list);

  void slotStartMatch(void);

  void slotCancelMatch(void);

  void slotEndMatch(void);

private:
  bool verifyImportedData(const MyMoneyAccount& account);

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

  void scheduleNew(const QCString& scheduleType);

  /**
    * Delete a possibly existing transaction editor but make sure to remove
    * any reference to it so that we avoid using a half-dead object
    */
  void deleteTransactionEditor(void);

  /**
    * delete all selected transactions w/o further questions
    */
  void doDeleteTransactions(void);

signals:
  /**
    * This signal is emitted when a new file is loaded. In the case file
    * is closed, this signal is also emitted with an empty url.
    */
  void fileLoaded(const KURL& url);

  /**
    * This signal is emitted when a payee/list of payees has been selected by
    * the GUI. If no payee is selected or the selection is removed,
    * @p payees is identical to an empty QValueList. This signal is used
    * by plugins to get information about changes.
    */
  void payeesSelected(const QValueList<MyMoneyPayee>& payees);

  /**
    * This signal is emitted when a transaction/list of transactions has been selected by
    * the GUI. If no transaction is selected or the selection is removed,
    * @p transactions is identical to an empty QValueList. This signal is used
    * by plugins to get information about changes.
    */
  void transactionsSelected(const QValueList<KMyMoneyRegister::SelectedTransaction>& transactions);

  /**
    * This signal is emitted when a list of payees has been selected by
    * the GUI. If no payee is selected or the selection is removed,
    * payees is identical to an empty QValueList. This signal is used
    * by plugins to get information about changes.
    */
  void budgetSelected(const QValueList<MyMoneyBudget>& budget);
  void budgetRename(void);

  /**
    * This signal is emitted when a new account has been selected by
    * the GUI. If no account is selected or the selection is removed,
    * account is identical to MyMoneyAccount(). This signal is used
    * by plugins to get information about changes.
    */
  void accountSelected(const MyMoneyAccount& account);
  void investmentSelected(const MyMoneyAccount& account);

  /**
    * This signal is emitted when a new institution has been selected by
    * the GUI. If no institution is selected or the selection is removed,
    * institution is identical to MyMoneyInstitution(). This signal is used
    * by plugins to get information about changes.
    */
  void institutionSelected(const MyMoneyInstitution& institution);

  /**
    * This signal is emitted when a new schedule has been selected by
    * the GUI. If no schedule is selected or the selection is removed,
    * schedule is identical to MyMoneySchedule(). This signal is used
    * by plugins to get information about changes.
    */
  void scheduleSelected(const MyMoneySchedule& schedule);

  void payeeRename(void);
  void payeeCreated(const QCString& id);

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

  QMap<QString, KMyMoneyPlugin::OnlinePlugin*> m_onlinePlugins;

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
  KURL  m_fileName;

  bool m_startDialog;
  QString m_mountpoint;

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
  MyMoneyAccount        m_reconciliationAccount;
  MyMoneyAccount        m_selectedInvestment;
  MyMoneyInstitution    m_selectedInstitution;
  MyMoneySchedule       m_selectedSchedule;
  QValueList<MyMoneyPayee>  m_selectedPayees;
  QValueList<MyMoneyBudget> m_selectedBudget;
  QValueList<KMyMoneyRegister::SelectedTransaction> m_selectedTransactions;

  QValueList<KMyMoneyRegister::SelectedTransaction> m_editTransactions;
  KMyMoneyRegister::SelectedTransaction             m_matchTransaction;

  // This is Auto Saving related
  bool                  m_autoSaveEnabled;
  QTimer*               m_autoSaveTimer;
  int                   m_autoSavePeriod;
  bool                  m_inAutoSaving;

  // Pointer to the combo box used for key selection during
  // File/Save as
  KComboBox*            m_saveEncrypted;

  // pointer to the current transaction editor
  TransactionEditor*    m_transactionEditor;

  // Reconciliation dialog
  KEndingBalanceDlg*    m_endingBalanceDlg;

  // id's that need to be remembered
  QCString              m_accountGoto, m_payeeGoto;
};

extern  KMyMoney2App *kmymoney2;


#endif // KMYMONEY2_H
