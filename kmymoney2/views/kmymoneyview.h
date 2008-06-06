/***************************************************************************
                          kmymoneyview.h
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef KMYMONEYVIEW_H
#define KMYMONEYVIEW_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qwidget.h>
class QVBox;
class QFile;
class QVBoxLayout;

// ----------------------------------------------------------------------------
// KDE Includes

#include <kpopupmenu.h>
#include <kjanuswidget.h>

#include <kurl.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/mymoneyaccount.h>
#include <kmymoney/mymoneyinstitution.h>
#include <kmymoney/mymoneytransaction.h>
#include <kmymoney/mymoneyscheduled.h>
#include <kmymoney/mymoneysecurity.h>
#include <kmymoney/selectedtransaction.h>

class KHomeView;
class KAccountsView;
class KCategoriesView;
class KInstitutionsView;
class KPayeesView;
class KBudgetView;
class KScheduledView;
class KGlobalLedgerView;
class IMyMoneyStorageFormat;
class MyMoneyTransaction;
class KInvestmentView;
class KReportsView;
class KMyMoneyViewBase;
class MyMoneyReport;
class TransactionEditor;
class KForecastView;

/**
  * This class represents the view of the MyMoneyFile which contains
  * Banks/Accounts/Transactions, Recurring transactions (or Bills & Deposits)
  * and scripts (yet to be implemented).  Each different aspect of the file
  * is represented by a tab within the view.
  *
  * @author Michael Edwardes 2001 Copyright 2000-2001
  *
  * @short Handles the view of the MyMoneyFile.
  */
class KMyMoneyView : public KJanusWidget
{
  Q_OBJECT

public:
  enum viewID {
    HomeView = 0,
    AccountsView,
    InstitutionsView,
    SchedulesView,
    CategoriesView,
    PayeesView,
    LedgersView,
    InvestmentsView,
    ReportsView,
    BudgetView,
    ForecastView
  };
  // file actions for plugin
  enum fileActions {
    preOpen, postOpen, preSave, postSave, preClose, postClose
  };

private:
  enum menuID {
    AccountNew = 1,
    AccountOpen,
    AccountReconcile,
    AccountEdit,
    AccountDelete,
    AccountOnlineMap,
    AccountOnlineUpdate,
    AccountOfxConnect,
    CategoryNew
  };

  typedef enum storageTypeE { // not used but keep for future implementation
    Memory = 0,
    Database
  } _storageType;

  KHomeView *m_homeView;
  KAccountsView *m_accountsView;
  KInstitutionsView *m_institutionsView;
  KCategoriesView *m_categoriesView;
  KPayeesView *m_payeesView;
  KBudgetView *m_budgetView;
  KScheduledView *m_scheduledView;
  KGlobalLedgerView *m_ledgerView;
  KInvestmentView *m_investmentView;
  KReportsView* m_reportsView;
  KForecastView* m_forecastView;

  QVBox* m_homeViewFrame;
  QVBox* m_accountsViewFrame;
  QVBox* m_institutionsViewFrame;
  QVBox* m_categoriesViewFrame;
  QVBox* m_payeesViewFrame;
  QVBox* m_budgetViewFrame;
  QVBox* m_scheduleViewFrame;
  QVBox* m_ledgerViewFrame;
  QVBox* m_investmentViewFrame;
  QVBox* m_reportsViewFrame;
  QVBox* m_forecastViewFrame;

  bool m_inConstructor;

  bool m_fileOpen;

  int  m_fmode;

  // bool m_bankRightClick;
  // MyMoneyInstitution m_accountsInstitution;

  // Keep a note of the file type
  typedef enum _fileTypeE {
    KmmBinary = 0, // native, binary
    KmmXML,        // native, XML
    KmmDbFile,     // database used as file, pretty much redundant now
    KmmDbSingleUser,    // database with some data maintained in memory
    KmmDbMultiUser,   // 'proper' database support not implemented
    /* insert new native file types above this line */
    MaxNativeFileType,
    /* and non-native types below */
    GncXML         // Gnucash XML
  }fileTypeE;
  fileTypeE m_fileType;

private:
  void ungetString(QIODevice *qfile, char * buf, int len);

  /**
    * This method creates the currency @p curr if it does not exist and
    * @p create is @p true. If the currency already exists, it checks
    * if the name is equal. If it is not, the name of the object in the
    * engine is updated to the name passed with @p curr.
    *
    * @param curr MyMoneySecurity to be checked
    * @param create If true and currency does not exist it will be created
                    If false currency will not be created even if it does not exist
    */
  void loadDefaultCurrency(const MyMoneySecurity& curr, const bool create);

  /**
    *
    */
  void loadAncientCurrency(const QCString& id, const QString& name, const QString& sym, const QDate& date, const MyMoneyMoney& rate, const QCString& newId, const int partsPerUnit = 100, const int smallestCashFraction = 100, const int smallestAccountFraction = 0);

  /**
    * if no base currency is defined, start the dialog and force it to be set
    */
  void selectBaseCurrency(void);

  /**
    * This method attaches an empty storage object to the MyMoneyFile
    * object. It calls removeStorage() to remove a possibly attached
    * storage object.
    */
  void newStorage(storageTypeE = Memory);

  /**
    * This method removes an attached storage from the MyMoneyFile
    * object.
    */
  void removeStorage(void);

  void viewAccountList(const QCString& selectAccount); // Show the accounts view

  static void progressCallback(int current, int total, const QString&);

  /**
    */
  void fixFile_0(void);
  void fixFile_1(void);

  /**
    */
  void fixLoanAccount_0(MyMoneyAccount acc);

  /**
    */
  void fixTransactions_0(void);
  void fixSchedule_0(MyMoneySchedule sched);
  void fixDuplicateAccounts_0(MyMoneyTransaction& t);

  void createSchedule(MyMoneySchedule s, MyMoneyAccount& a);

  void checkAccountName(const MyMoneyAccount& acc, const QString& name) const;

public:
  /**
    * The constructor for KMyMoneyView. Just creates all the tabs for the
    * different aspects of the MyMoneyFile.
    */
  KMyMoneyView(QWidget *parent=0, const char *name=0);

  /**
    * Destructor
    */
  ~KMyMoneyView();

  /**
    * Makes sure that a MyMoneyFile is open and has been created succesfully.
    *
    * @return Whether the file is open and initialised
    */
  bool fileOpen(void);

  /**
    * Closes the open MyMoneyFile and frees all the allocated memory, I hope !
    */
  void closeFile(void);


  /**
    * Calls MyMoneyFile::readAllData which reads a MyMoneyFile into appropriate
    * data structures in memory.  The return result is examined to make sure no
    * errors occured whilst parsing.
    *
    * @param url The URL to read from.
    *            If no protocol is specified, file:// is assumed.
    *
    * @return Whether the read was successfull.
    */
  bool readFile(const KURL& url);

  /**
    * Saves the data into permanent storage using the XML format.
    *
    * @param url The URL to save into.
    *            If no protocol is specified, file:// is assumed.
    * @param keyList QString containing a comma separated list of keys
    *            to be used for encryption. If @p keyList is empty,
    *            the file will be saved unencrypted (the default)
    *
    * @retval false save operation failed
    * @retval true save operation was successful
    */
  const bool saveFile(const KURL& url, const QString& keyList = QString());
  /**
   * Saves the data into permanent storage on a new or empty SQL database.
   *
   * @param url The pseudo of tyhe database
   *
   * @retval false save operation failed
   * @retval true save operation was successful
   */
  //const bool saveDatabase(const KURL& url); This no longer relevant
  /**
   * Saves the data into permanent storage on a new or empty SQL database.
   *
   * @param url The pseudo URL of the database
   *
   * @retval false save operation failed
   * @retval true save operation was successful
   */
  const bool saveAsDatabase(const KURL& url);

  /**
    * Call this to find out if the currently open file is native KMM
    *
    * @retval true file is native
    * @retval false file is foreign
    */
  const bool isNativeFile() { return (m_fileOpen && (m_fileType < MaxNativeFileType)); }

  /**
   * Call this to find out if the currently open file is a sql database
   *
   * @retval true file is database
   * @retval false file is serial
   */
  const bool isDatabase()
    { return (m_fileOpen && ((m_fileType == KmmDbFile) || (m_fileType == KmmDbSingleUser) || (m_fileType == KmmDbMultiUser))); }

  /**
   * Call this to find out if the currently open file is a SQL database
    * opened in sync mode (i.e. updates writen to database as they happen)
   *
   * @retval true file is synchronous
   * @retval false file is asynchronous
   */
  const bool isSyncDatabase()
  { return (m_fileOpen && ((m_fileType == KmmDbSingleUser) || (m_fileType == KmmDbMultiUser))); }

  /**
    * Call this to see if the MyMoneyFile contains any unsaved data.
    *
    * @retval true if any data has been modified but not saved
    * @retval false otherwise
    */
  bool dirty(void);

  /**
    * Close the currently opened file and create an empty new file.
    *
    * @see MyMoneyFile
    */
  void newFile(void);

  /**
    * Moves the view up from transaction to Bank/Account view.
    */
  void viewUp(void);

  /**
    * This method allows to set the enable state of all views (except home view)
    * The argument @p state controls the availability.
    *
    * @param state Controls whether views are disabled @p (0), enabled @p (1) or
    *              enabled/disabled according to an open file @p (-1). The latter
    *              is the default.
    */
  void enableViews(int state = -1);

  KMyMoneyViewBase* addPage(const QString& title, const QString& icon = QString());

  void addWidget(QWidget* w);

  virtual bool showPage(int index);

  /**
    * check if the current view allows to create a transaction
    *
    * @param list list of selected transactions
    * @param tooltip reference to string receiving the tooltip text
    *        which explains why the modify function is not available (in case
    *        of returning @c false)
    *
    * @retval true Yes, view allows to create a transaction (tooltip is not changed)
    * @retval false No, view cannot to create a transaction (tooltip is updated with message)
    */
  bool canCreateTransactions(const KMyMoneyRegister::SelectedTransactions& list, QString& tooltip) const;

  /**
    * check if the current view allows to modify (edit/delete) the selected transactions
    *
    * @param list list of selected transactions
    * @param tooltip reference to string receiving the tooltip text
    *        which explains why the modify function is not available (in case
    *        of returning @c false)
    *
    * @retval true Yes, view allows to edit/delete transactions (tooltip is not changed)
    * @retval false No, view cannot edit/delete transactions (tooltip is updated with message)
    */
  bool canModifyTransactions(const KMyMoneyRegister::SelectedTransactions& list, QString& tooltip) const;

  /**
    * check if the current view allows to edit the selected transactions
    *
    * @param list list of selected transactions
    * @param tooltip reference to string receiving the tooltip text
    *        which explains why the edit function is not available (in case
    *        of returning @c false)
    *
    * @retval true Yes, view allows to enter/edit transactions
    * @retval false No, view cannot enter/edit transactions
    */
  bool canEditTransactions(const KMyMoneyRegister::SelectedTransactions& list, QString& tooltip) const;

  /**
    * check if the current view allows to print something
    *
    * @retval true Yes, view allows to print
    * @retval false No, view cannot print
    */
  bool canPrint(void);

  TransactionEditor* startEdit(const KMyMoneyRegister::SelectedTransactions&);

  bool createNewTransaction(void);

  /**
    * Used to start reconciliation of account @a account. It switches the
    * ledger view into reconciliation mode and updates the view.
    *
    * @param account account which should be reconciled
    * @param endingBalance the ending balance entered for this account
    *
    * @retval true Reconciliation started
    * @retval false Account cannot be reconciled
    */
  bool startReconciliation(const MyMoneyAccount& account, const MyMoneyMoney& endingBalance);

  /**
    * Used to finish reconciliation of account @a account. It switches the
    * ledger view to normal mode and updates the view.
    *
    * @param account account which should be reconciled
    */
  void finishReconciliation(const MyMoneyAccount& account);

  /**
    * This method preloads all known currencies into the engine.
    */
  void loadDefaultCurrencies(void);

  void loadAncientCurrencies(void);

public slots:
  /**
    * This slot writes information about the page passed as argument @p widget
    * in the kmymoney2.rc file so that in can be selected automatically when
    * the application is started again.
    *
    * @param widget pointer to page widget
    */
  void slotRememberPage(QWidget* widget);

  /**
    * Brings up a dialog to change the list(s) settings and saves them into the
    * class KMyMoneySettings (a singleton).
    *
    * @see KListSettingsDlg
    * Refreshs all views. Used e.g. after settings have been changed or
    * data has been loaded from external sources (QIF import).
    **/
  void slotRefreshViews();

  /**
    * Called, whenever the ledger view should pop up and a specific
    * transaction in an account should be shown. If @p transaction
    * is empty, the last transaction should be selected
    *
    * @param acc The ID of the account to be shown
    * @param transaction The ID of the transaction to be selected
    */
  void slotLedgerSelected(const QCString& acc, const QCString& transaction = QCString());

  /**
    * Called, whenever the payees view should pop up and a specific
    * transaction in an account should be shown.
    *
    * @param payeeId The ID of the payee to be shown
    * @param accountId The ID of the account to be shown
    * @param transactionId The ID of the transaction to be selected
    */
  void slotPayeeSelected(const QCString& payeeId, const QCString& accountId, const QCString& transactionId);

  /**
    * Called, whenever the schedule view should pop up and a specific
    * schedule should be shown.
    *
    * @param schedule The ID of the schedule to be shown
    */
  void slotScheduleSelected(const QCString& schedule);

  /**
    * Called, whenever the report view should pop up and a specific
    * report should be shown.
    *
    * @param reportid The ID of the report to be shown
    */
  void slotShowReport(const QCString& reportid);

  /**
    * Same as the above, but the caller passes in an actual report
    * definition to be shown.
    *
    * @param report The report to be shown
    */
  void slotShowReport(const MyMoneyReport& report);

  /**
    * This slot prints the current view.
    */
  void slotPrintView(void);

  /**
    * This slot switches the view to present the home page
    */
  void slotShowHomePage(void) { showPage(0); }

protected slots:
  /**
    * Called when the user changes the detail
    * setting of the transaction register
    *
    * @param detailed if true, the register is shown with all details
    */
  void slotShowTransactionDetail(bool detailed);

  /**
   * eventually replace this with KMyMoney2App::slotCurrencySetBase(void).
   * it contains the same code
   *
   * @deprecated
   */
  void slotSetBaseCurrency(const MyMoneySecurity& baseCurrency);

private:
  /**
   * This method is called from readFile to open a database file which
   * is to be processed in 'proper' database mode, i.e. in-place updates
   *
   * @param dbaseURL pseudo-KURL representation of database
   *
   * @retval true Database opened successfully
   * @retval false Could not open or read database
   */
  bool openDatabase (const KURL& dbaseURL);
  /**
   * This method is used after a file or database has been
   * read into storage, and performs various initialization tasks
   *
   * @retval true all went okay
   * @retval false an exception occurred during this process
   */
  bool initializeStorage();
  /**
    * This method is used by saveFile() to store the data
    * either directly in the destination file if it is on
    * the local file system or in a temporary file when
    * the final destination is reached over a network
    * protocol (e.g. FTP)
    *
    * @param qf pointer to QFile representing the opened file
    * @param writer pointer to the formatter
    * @param plaintext whether to override any compression & encryption settings
    * @param keyList QString containing a comma separated list of keys to be used for encryption
    *            If @p keyList is empty, the file will be saved unencrypted
    *
    * @note This method will close the file when it is written.
    */
  void saveToLocalFile(QFile* qf, IMyMoneyStorageFormat* writer, bool plaintext=false, const QString& keyList = QString());

  /**
    * Internal method used by slotAccountNew() and slotAccountCategory().
    */
  void accountNew(const bool createCategory);

signals:
  /**
    * This signal is emitted whenever a view is selected.
    * The parameter @p view is identified as one of KMyMoneyView::viewID.
    */
  void viewActivated(int view);

  void accountSelectedForContextMenu(const MyMoneyAccount& acc);

  void viewStateChanged(bool enabled);
   /**
     * This signal is emitted to inform the kmmFile plugin when various file actions
     * occur. The Action parameter distinguishes between them.
     */
  void kmmFilePlugin (unsigned int action);

  /**
    * Signal is emitted when reconciliation starts or ends. In case of end,
    * @a account is MyMoneyAccount()
    *
    * @param account account for which reconciliation starts or MyMoneyAccount()
    *                if reconciliation ends.
    * @param endingBalance collected ending balance when reconciliation starts
    *                0 otherwise
    */
  void reconciliationStarts(const MyMoneyAccount& account, const MyMoneyMoney& endingBalance);

};

class KMyMoneyTitleLabel;

/**
  * This class is an abstract base class that all specific views
  * should be based on.
  */
class KMyMoneyViewBase : public QWidget
{
  Q_OBJECT
public:
  KMyMoneyViewBase(QWidget* parent, const char *name, const QString& title);
  virtual ~KMyMoneyViewBase() {};

  void setTitle(const QString& title);
  QVBoxLayout* layout(void) const { return m_viewLayout; }
  void addWidget(QWidget* w);

  /**
    * This method is used to edit the currently selected transactions
    * The default implementation returns @p false which signals to the caller, that
    * the view was not capable to edit the transactions.
    *
    * @retval false view was not capable to edit transactions
    * @retval true view was capable to edit the transactions and did so
    */
  bool editTransactions(const QValueList<MyMoneyTransaction>& transactions) const { Q_UNUSED(transactions)  return false; }

protected:
  KMyMoneyTitleLabel*    m_titleLabel;
  QVBoxLayout*           m_viewLayout;
};
#endif
