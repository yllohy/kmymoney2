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

#include "config.h"

#ifndef KMYMONEYVIEW_H
#define KMYMONEYVIEW_H

// ----------------------------------------------------------------------------
// QT Includes
#include <qwidget.h>
#include <qmessagebox.h>
#include <qvbox.h>
class QFile;

// ----------------------------------------------------------------------------
// KDE Includes
#include <kpopupmenu.h>
#include <kjanuswidget.h>

#include <kurl.h>

// ----------------------------------------------------------------------------
// Project Includes
#include "../mymoney/mymoneyaccount.h"
//#include "kmainview.h"
#include "../dialogs/kreconciledlg.h"
#include "../dialogs/kfindtransactiondlg.h"
#include "../dialogs/knewaccountwizard.h"
//#include "kscheduleview.h"
#include "../dialogs/kcsvprogressdlg.h"

#include "kbanksview.h"
#include "ktransactionview.h"
#include "khomeview.h"
#include "kcategoriesview.h"
#include "kpayeesview.h"
#include "kscheduledview.h"
#include "kinvestmentview.h"
#include "kmymoneyfile.h"
#include "kgloballedgerview.h"

#include "../mymoney/storage/mymoneyseqaccessmgr.h"
class IMyMoneyStorageFormat;

/**
  * This class represents the view of the MyMoneyFile which contains
  * Banks/Accounts/Transactions, Recurring transactions (or Bills & Deposits)
  * and scripts (yet to be implemented).  Each different aspect of the file
  * is represented by a tab within the view.
  *
  * @author Michael Edwardes 2001 Copyright 2000-2001
  * $Id: kmymoneyview.h,v 1.23 2002/11/17 09:00:51 ipwizard Exp $
  *
  * @short Handles the view of the MyMoneyFile.
**/
class KMyMoneyView : public KJanusWidget {
   Q_OBJECT


public:
  enum viewType { None=0, BankList=1, TransactionList=2, InvestmentList=3 };
  enum viewShowing { AccountsView, HomeView, PayeeView, CategoryView, ScheduledView, AccountView };

private:
  KHomeView *m_homeView;
  KAccountsView *accountsView;
  // KTransactionView *transactionView;
  KCategoriesView *m_categoriesView;
  KPayeesView *m_payeesView;
  KScheduledView *m_scheduledView;
  KInvestmentView *m_investmentView;
  KNewAccountWizard *m_newAccountWizard;
  KGlobalLedgerView *m_ledgerView;

  viewType m_showing;
  viewShowing m_realShowing;

  KMyMoneyFile *m_file;  // The interface to the file
  //MyMoneySeqAccessMgr *m_storage;

/*
  bool m_inReconciliation;  // True if the reconciliaton dialog needs updating when the user adds/deletes transactions
  bool m_reconcileInited;  // True if a reconciliation has already been completed this execution
  KReconcileDlg *reconcileDlg;  // These exists during app run time ?
*/
//  KFindTransactionDlg *transactionFindDlg;
//  KImportDlg       *importDlg;

  KPopupMenu* m_accountMenu;
  KPopupMenu* m_bankMenu;
  KPopupMenu* m_rightMenu;
  
  
  // The schedule view
//  KScheduleView *m_scheduledView;
  /**  */
  //QList<MyMoneyTransaction> m_transactionList;

  void loadDefaultCategories(void);  // Loads catgegories from default_categories.dat
  // Parses a line in the default categories file
  bool parseDefaultCategory(QString& line, bool& income, QString& name, QStringList& minors);
  void viewAccountList(const QCString& selectAccount); // Show the accounts view

  // Some utility functions for the KFindTransactionDlg calls in doTransactionSearch()
  bool checkTransactionDates(const MyMoneyTransaction *transaction, const bool enabled, const QDate start, const QDate end);
  bool checkTransactionAmount(const MyMoneyTransaction *transaction, const bool enabled, const QString id, const MyMoneyMoney amount);
  bool checkTransactionCredit(const MyMoneyTransaction *transaction, const bool enabled, const QString id);
  bool checkTransactionStatus(const MyMoneyTransaction *transaction, const bool enabled, const QString id);
  bool checkTransactionDescription(const MyMoneyTransaction *transaction, const bool enabled, const QString description, const bool isRegExp);
  bool checkTransactionNumber(const MyMoneyTransaction *transaction, const bool enabled, const QString number, const bool isRegExp);
  bool checkTransactionPayee(const MyMoneyTransaction *transaction, const bool enabled, const QString payee, const bool isRegExp);

  bool checkTransactionCategory(const MyMoneyTransaction *transaction, const bool enabled, const QString category);

public:
  /**
    * The constructor for KMyMoneyView. Just creates all the tabs for the
    * different aspects of the MyMoneyFile.
  **/
  KMyMoneyView(QWidget *parent=0, const char *name=0);
  
  /**
    * Destructor
  **/
  ~KMyMoneyView();
  
  /**
    * Makes sure that a MyMoneyFile is open and has been created succesfully.
    *
    * @return Whether the file is open and initialised
  **/
  bool fileOpen(void);

  /**
    * Closes the open MyMoneyFile and frees all the allocated memory, I hope !
  **/
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
  **/
  bool readFile(const KURL& url);

  /**
    * Calls MyMoneyFile::saveAllData which saves all the data structures in memory
    * into the file specified by filename.
    *
    * @param url The URL to save into.
    *            If no protocol is specified, file:// is assumed.
    *
  **/
  void saveFile(const KURL& url);

  /**
    * Call this to see if the MyMoneyFile contains any unsaved data.
    *
    * @return TRUE if any data has been modified but not saved.
  **/
  bool dirty(void);

  /**
    * Creates a new file first making sure that one isn't open already.  Opens
    * up a KNewFileDlg to input the new details.
    *
    * @see MyMoneyFile
  **/
  void newFile(void);

  /**
    * Brings up a dialog that displays information about the user who created
    * the MyMoneyFile if set.
    *
    * @see KNewFileDlg
  **/
  void viewPersonal(void);

  /**
    * Moves the view up from transaction to Bank/Account view.
  **/
  void viewUp(void);

  /**
    * Utility method to retrieve the currently selected bank name.
    *
    * @return The currently selected bank name.
  **/
  //QString currentBankName(void);

  /**
    * Utility method to retrieve the currently selected account name.
    *
    * @return The currently selected account name.
  **/
  QString currentAccountName(void);


  void memoryDump();

public slots:
  /**
    * Brings up a dialog to change the list(s) settings and saves them into the
    * class KMyMoneySettings (a singleton).
    *
    * @see KListSettingsDlg
  **/
  void settingsLists();

  /**
    * Brings up a dialog to let the user search for specific transaction(s).  It then
    * opens a results window to display those transactions.
  **/
  void accountFind();

  /**
    * Called whenever the user 'executes' an account. This operation opens the account
    * and shows the register view.
    *
    * @param account The account which was 'executed'.
  **/
  void slotAccountDoubleClick(void);

  //void slotAccountDoubleClick(const MyMoneyAccount);


  /**
    * Called whenever the user wishes to create a new bank.  Brings up the input
    * dialog and saves the information.  It then enables the banks view.
    *
    * @see KBanksView
    * @see KNewBankDlg
    * @see MyMoneyFile
    * @see MyMoneyBank
  **/
  void slotBankNew(void);

  /**
    * Called whenever the user wishes to create a new account.  Brings up the input
    * dialog and saves the information.
    *
    * @see KBanksView
    * @see KNewAccountDlg

    * @see MyMoneyFile
    * @see MyMoneyAccount
  **/
  void slotAccountNew(void);

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
  **/
  void slotAccountReconcile(void);

  // Not implemented, not documented!
  void slotAccountImportAscii(void);
  void slotAccountExportAscii(void);

//  void slotAccountImportQIF(void);
//  void slotAccountExportQIF(void);

protected slots:
  void viewTransactionList(void);  // Show the transaction view

  /**
    * This slot is called whenever the transaction list is changed and is used
    * in the reconciliation process to update the view.
    *
    * @see slotAccountReconcile.
  **/
//  void slotTransactionListChanged();

  /**
    * Called whenever the user right clicks on an account.  It brings up
    * a context menu.  TODO: move the context menu into kmymoney2ui.rc, move
    * this method into KBanksView, remove the param inList.
    *
    * @param account The account which has been clicked on.
    * @param inList Whether the click was on an account (REDUNDANT).
  **/
  void slotAccountRightMouse();
  void slotBankRightMouse();
  void slotRightMouse();



  /**
    * Called by the context menu created in slotAccountRightMouse.  Brings up
    * a dialog which allows the user to edit the account details.  TODO: move this
    * method into KBanksView.
  **/
  void slotAccountEdit();

  /**
    * Called by the context menu created in slotAccountRightMouse.  Deletes the currently
    * selected account. TODO: move this method into KBanksView.
  **/
  void slotAccountDelete();

  /**
    * Called by the context menu created in slotBankRightMouse.  Brings up
    * a dialog which allows the user to edit the bank details.  TODO: move this
    * method into KBanksView.
  **/
  void slotBankEdit();

  /**
    * Called by the context menu created in slotBankRightMouse.  Deletes the currently
    * selected bank. TODO: move this method into KBanksView.
  **/
  void slotBankDelete();

  /**
    * This is connected to KReconcileDlg::reconcileFinished in slotAccountReconcile.
    * It is called when the user has finished the reconciliation process.
    *
    * @param success Whether the user successfully reconciled the account.
  **/
  void slotReconcileFinished(bool success);

  /**
    * Brings up the find transaction dialog and shows the results.
    *
    * @see KFindTransactionDlg
    * @see KTFindResultsDlg
  **/
  void doTransactionSearch();

  /**
    * Called when the user clicks on the homepage button.
    *
    * @see KHomeView
  **/
  void slotActivatedHomePage();


  /**
    * Called when the user clicks on the accounts button
    *
    * @see KBanksView
  **/
  void slotActivatedAccountsView();

  void slotActivatedScheduledView();

  void slotActivatedCategoriesView();

  void slotActivatedPayeeView();

  void slotActivatedAccountView();

  /**
    * Called when the user changes the visibility
    * setting of the transaction form
    *
    * @param show if true, the transaction form is shown
    */
  void slotShowTransactionForm(bool show);


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
    *
    * @note This method will close the file when it is written.
    */
  void saveToLocalFile(QFile* qf, IMyMoneyStorageFormat* writer);

signals:
  /**
    * This signal is emitted whenever the bank actions needs enabling or disabling.
    * KMyMoney2App connects to this signal and does the actual enabling.
    *
    * @param enable Whether to enable to actions.
  **/
  //void bankOperations(bool);

  /**
    * This signal is emitted whenever the account actions needs enabling or disabling.
    * KMyMoney2App connects to this signal and does the actual enabling.
    *
    * @param enable Whether to enable to actions.
  **/
  //void accountOperations(bool);

  /**
    * This signal is emitted whenever the file actions needs enabling or disabling.
    * KMyMoney2App connects to this signal and does the actual enabling.
    *
    * @param enable Whether to enable to actions.
  **/
  //void fileOperations(bool);


  /**
    * This signal is emitted whenever the transaction actions needs enabling or disabling.
    * KMyMoney2App connects to this signal and does the actual enabling.
    *
    * @param enable Whether to enable to actions.
  **/
  //void transactionOperations(bool);

  void signalEnableKMyMoneyOperations(bool);


  void signalHomeView();
  void signalAccountsView();
  void signalScheduledView();
  void signalCategoryView();
  void signalPayeeView();
  void signalAccountView();
};

#endif
