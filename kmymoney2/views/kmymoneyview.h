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
#include <qmessagebox.h>
#include <qvbox.h>
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

class KHomeView;
class KAccountsView;
class KCategoriesView;
class KPayeesView;
class KScheduledView;
class KNewAccountWizard;
class KGlobalLedgerView;
class IMyMoneyStorageFormat;
class MyMoneyTransaction;
class KInvestmentView;
class KReportsView;
class KMyMoneyViewBase;
class KFindTransactionDlg;

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
class KMyMoneyView : public KJanusWidget {
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
    ReportsView
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

  KHomeView *m_homeView;
  KAccountsView *m_accountsView;
  KAccountsView *m_institutionsView;
  KCategoriesView *m_categoriesView;
  KPayeesView *m_payeesView;
  KScheduledView *m_scheduledView;
  KGlobalLedgerView *m_ledgerView;
  KInvestmentView *m_investmentView;
  KReportsView* m_reportsView;
  KFindTransactionDlg* m_searchDlg;

  QVBox* m_homeViewFrame;
  QVBox* m_accountsViewFrame;
  QVBox* m_institutionsViewFrame;
  QVBox* m_categoriesViewFrame;
  QVBox* m_payeesViewFrame;
  QVBox* m_scheduleViewFrame;
  QVBox* m_ledgerViewFrame;
  QVBox* m_investmentViewFrame;
  QVBox* m_reportsViewFrame;

  bool m_fileOpen;

  KPopupMenu* m_accountMenu;
  KPopupMenu* m_bankMenu;
  KPopupMenu* m_rightMenu;

  bool m_bankRightClick;
  MyMoneyInstitution m_accountsInstitution;
  // Keep a note of the file type
  typedef enum _fileTypeE {
    KmmBinary = 0, // native, binary
    KmmXML,        // native, XML
    /* insert new native file types above this line */
    MaxNativeFileType,
    /* and non-native types below */
    GncXML
  }fileTypeE;
  fileTypeE m_fileType;
  
private:
  void ungetString(QIODevice *qfile, char * buf, int len);

  /**
    * This method preloads all known currencies into the engine.
    */
  void loadDefaultCurrencies(void);

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

  void loadAncientCurrencies(void);

  /**
    * if no base currency is defined, start the dialog and force it to be set
    */
  void selectBaseCurrency(void);

  /**
    * This method attaches an empty storage object to the MyMoneyFile
    * object. It calls removeStorage() to remove a possibly attached
    * storage object.
    */
  void newStorage(void);

  /**
    * This method removes an attached storage from the MyMoneyFile
    * object.
    */
  void removeStorage(void);

  void viewAccountList(const QCString& selectAccount); // Show the accounts view

  static void progressCallback(int current, int total, const QString&);

  /**
    */
  void fixFile(void);

  /**
    */
  void fixLoanAccount(MyMoneyAccount acc);

  /**
    * This method converts a possible opening balance held with @p
    * account into a transaction between the account in question and
    * the opening balances account for the currency of @p account.
    * @p account is updated. If the opening balance is 0, then nothing
    * is changed.
    *
    * @param account reference to the account object to be checked
    */
  void fixOpeningBalance(MyMoneyAccount& account);

  /**
    */
  void fixTransactions(void);
  void fixSchedule(MyMoneySchedule sched);
  void fixDuplicateAccounts(MyMoneyTransaction& t);

  void createSchedule(MyMoneySchedule s, MyMoneyAccount& a);

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
    *
    * @retval false save operation failed
    * @retval true save operation was successful
    */
  const bool saveFile(const KURL& url);
  /**
    * Call this to find out if the currently open file is native KMM
    *
    * @retval true file is native
    * @retval false file is foreign
    */
  const bool isNativeFile() { return (m_fileType < MaxNativeFileType);};

  /**
    * Call this to see if the MyMoneyFile contains any unsaved data.
    *
    * @retval true if any data has been modified but not saved
    * @retval false otherwise
    */
  bool dirty(void);

  /**
    * Creates a new file first making sure that one isn't open already.  Opens
    * up a KNewFileDlg to input the new details.
    *
    * @param createEmtpyFile if false (default) the user will be asked for personal
    *                        information and will be offered to load default income
    *                        and expense accounts. If true, a completely empty file
    *                        will be created without user interaction.
    * @retval false User pressed Cancel button
    * @retval true User pressed Ok
    *
    * @see MyMoneyFile
    */
  bool newFile(const bool createEmtpyFile = false);

  /**
    * Brings up a dialog that displays information about the user who created
    * the MyMoneyFile if set.
    *
    * @see KNewFileDlg
    */
  void viewPersonal(void);

  /**
    * Moves the view up from transaction to Bank/Account view.
    */
  void viewUp(void);

  /**
    * Utility method to retrieve the currently selected account name.
    *
    * @return The currently selected account name.
    */
  QString currentAccountName(void);

  /**
    * utility method to suspend/activate updates of the MyMoney engine on
    * all views. This is used to speed up operations with lot's of updates
    * of engine data in a short time (e.g. importing data, creating a
    * new file).
    *
    * @param suspend Suspend updates or not. Possible values are
    *
    * @li true updates are suspended
    * @li false updates will be performed immediately
    */
  void suspendUpdate(const bool suspend);

  void memoryDump();

  /**
    * This method allows to set the enable state of all views (except home view)
    * The argument @p state controls the availability.
    *
    * @param state Controls whether views are disabled @p (0), enabled @p (1) or
    *              enabled/disabled according to an open file @p (-1). The latter
    *              is the default.
    */
  void enableViews(int state = -1);

  KPopupMenu* accountContextMenu(void) const { return m_accountMenu; };

  KMyMoneyViewBase* addPage(const QString& title, const QPixmap& pixmap = QPixmap());

  void addWidget(QWidget* w);

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
    * Brings up a dialog to let the user search for specific transaction(s).  It then
    * opens a results window to display those transactions.
    */
  void slotFindTransaction();

  /**
    * Called whenever the user 'executes' an account. This operation opens the account
    * and shows the register view.
    **/
  void slotAccountDoubleClick(void);

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
  void slotReportSelected(const QCString& reportid);

  /**
    * Called whenever the user wishes to create a new bank.  Brings up the input
    * dialog and saves the information.  It then enables the banks view.
    *
    * @see KBanksView
    * @see KNewBankDlg
    * @see MyMoneyFile
    * @see MyMoneyInstitution
    */
  void slotBankNew(void);

  /**
    * Called whenever the user wishes to create a new account.  Brings up the input
    * dialog and saves the information.
    *
    * @see KBanksView
    * @see KNewAccountDlg
    * @see MyMoneyFile
    * @see MyMoneyAccount
    */
  void slotAccountNew(void);

  /**
    * Called whenever the user wishes to create a new account by right clicking on
    * an institution.  Brings up the input dialog and saves the information.
    *
    * This exists so that the institution can be pre-set in the account wizard.
    *
    * @see KBanksView
    * @see KNewAccountDlg
    * @see MyMoneyFile
    * @see MyMoneyAccount
    */
  void slotBankAccountNew(void);

  /**
    * Called whenever the user wishes to create a new category.  Brings up the input
    * dialog and saves the information.
    *
    * @see KBanksView
    * @see KNewAccountDlg
    * @see MyMoneyFile
    * @see MyMoneyAccount
    */
  void slotCategoryNew(void);

  /**
    * Called whenever the user wishes to reconcile the open account.  It first get some
    * required input and then opens the reconciliation dialog.  The user can edit transactions
    * as normal in the main view because this dialog is modeless and is updated whenever the
    * transaction list is changed.
    *
    * @see KEndingBalanceDlg
    * @see KReconcileDlg
    * @see KMyMoneyFile
    * @see MyMoneyAccount
    */
  void slotAccountReconcile(void);

  // Not implemented, not documented!

  void slotAccountImportAscii(void);
  void slotAccountExportAscii(void);

  /**
    * This slot cancels any edit activity in any view. It will
    * be called e.g. before entering the settings dialog.
    */
  void slotCancelEdit(void) const;

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
    * Called whenever the user right clicks on an account.  It brings up
    * a context menu.  TODO: move the context menu into kmymoney2ui.rc, move
    * this method into KBanksView, remove the param inList.
    */
  void slotAccountRightMouse();
  void slotBankRightMouse();
  void slotRightMouse();

  /**
    * Called by the context menu created in slotAccountRightMouse.  Brings up
    * a dialog which allows the user to edit the account details.  TODO: move this
    * method into KBanksView.
    */
  void slotAccountEdit();

  /**
    * Called by the context menu created in slotAccountRightMouse.  Deletes the currently
    * selected account. TODO: move this method into KBanksView.
    */
  void slotAccountDelete();

  /**
    * Called by the context menu created in slotAccountRightMouse.  Connects to the users
    * bank via OFX.  Only valid if ofxConnectionSettings have been set in the institution.
    */
  void slotAccountOfxConnect();

  /**
    * Called by the context menu created in slotBankRightMouse.  Brings up
    * a dialog which allows the user to edit the bank details.  TODO: move this
    * method into KBanksView.
    */
  void slotBankEdit();

  /**
    * Called by the context menu created in slotBankRightMouse.  Deletes the currently
    * selected bank. TODO: move this method into KBanksView.
    */
  void slotBankDelete();

  /**
    * Destroys the search dialog
    */
  void slotCloseSearchDialog(void);

  /**
    * Called when the user changes the detail
    * setting of the transaction register
    *
    * @param detailed if true, the register is shown with all details
    */
  void slotShowTransactionDetail(bool detailed);

private:
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
    *
    * @note This method will close the file when it is written.
    */
  void saveToLocalFile(QFile* qf, IMyMoneyStorageFormat* writer, bool plaintext=false);

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
};

class kMyMoneyTitleLabel;

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

protected:
  kMyMoneyTitleLabel*    m_titleLabel;
  QVBoxLayout*           m_viewLayout;
};
#endif
