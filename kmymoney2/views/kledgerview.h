/***************************************************************************
                          kledgerview.h  -  description
                             -------------------
    begin                : Sun Jul 14 2002
    copyright            : (C) 2000-2002 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KLEDGERVIEW_H
#define KLEDGERVIEW_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qwidget.h>
#include <qtable.h>
#include <qvaluelist.h>
#include <qvaluevector.h>
#include <qtimer.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>

// ----------------------------------------------------------------------------
// Project Includes

class kMyMoneyRegister;
class kMyMoneyTransactionForm;
class kMyMoneyPayee;
class kMyMoneyCategory;
class kMyMoneyEdit;
class kMyMoneyLineEdit;
class kMyMoneyDateInput;

#include "../mymoney/mymoneyaccount.h"
#include "../mymoney/mymoneytransaction.h"
#include "../mymoney/mymoneyobserver.h"


/**
  *@author Thomas Baumgart
  *
  * @todo Add account (hierarchy) upon new category
  */

/**
  * This class is used to store a sorted vector of pointers to
  * the transactions that are visible in a ledger view. When the
  * vector is created, the sort method is set to SortPostDate.
  * The sort type can be changed using the method setSortType().
  */
class KTransactionPtrVector : public QPtrVector<MyMoneyTransaction> {
public:
  /**
    * This enumerator defines the possible sort methods.
    * Possible values are:
    *
    */
  enum TransactionSortE {
    SortEntryDate = 0,      /**< Sort the vector so that the transactions appear sorted
                              *  according to their entry date
                              */
    SortPostDate,           /**< Sort the vector so that the transactions appear sorted
                              *     according to their post date
                              */
  };

  KTransactionPtrVector() { m_sortType = SortPostDate; };
  ~KTransactionPtrVector() {};

  /**
    * This method is used to set a different sort type.
    * The vector is resorted. See KTransactionPtrVector::TransactionSortE
    * for possible values.
    */
  void setSortType(const TransactionSortE type);

protected:
  int compareItems(KTransactionPtrVector::Item d1, KTransactionPtrVector::Item d2);

private:
  TransactionSortE  m_sortType;
};

/**
  * This class is the abstract base class for all ledger views. Common functionality
  * for all ledger views. The specifics of the ledger view depend on the account type
  * of the account which ledger is shown. The following specific ledger views are
  * available:
  *
  * @li KLedgerViewCheckings
  */
class KLedgerView : public QWidget, MyMoneyObserver  {
   Q_OBJECT

public:
  enum transactionTypeE {
    Check = 0,
    Deposit,
    Transfer,
    Withdrawal,
    ATM
  };

	KLedgerView(QWidget *parent=0, const char *name=0);
	virtual ~KLedgerView();

  void setCurrentAccount(const QCString& accountId);
  void update(const QCString& accountId);

  MyMoneyTransaction* const transaction(const int idx) const;
  const MyMoneyMoney& balance(const int idx) const;
  const QCString accountId(void) { return m_account.id(); }

  /**
    * This method is used to convert a string used in the
    * context of the user interface to an action string
    * used in the mymoney engine
    *
    * @param action QString reference of action string
    * @return QCString with normalized action string
    */
  const QCString str2action(const QString& action) const;

  /**
    * This method is used to convert an action string used
    * within the engine to a form usable on the user interface.
    *
    * @param action QCString reference to normalized action string
    * @return QString with translated action string
    */
  const QString action2str(const QCString& action) const;

  /**
    * This method is called to fill the transaction form
    * with the data of the currently selected transaction
    * in m_register. It must be overridden by any derived
    * class.
    */
  virtual void fillForm(void) = 0;

  /**
    * This method is used to select a specific transaction
    *
    * @param id const reference to the ID of the transaction to be selected
    */
  void selectTransaction(const QCString& id);

public slots:
  /**
    * refresh the current view
    */
  virtual void refreshView(void);
  virtual void slotRegisterClicked(int row, int col, int button, const QPoint &mousePos);

  /**
    * Called when the user changes the visibility
    * setting of the transaction form
    */
  virtual void slotShowTransactionForm(bool show);

  /**
    * Called when the payee field has been changed
    *
    * @param name const reference to the name of the payee
    */
  virtual void slotPayeeChanged(const QString &name);

  /**
    * Called when the memo field has been changed
    *
    * @param memo const reference to the new memo text
    */
  virtual void slotMemoChanged(const QString &memo);

  /**
    * Called when the category field has been changed
    *
    * @param name const reference to the name of the category
    */
  virtual void slotCategoryChanged(const QString& category);

  /**
    * Called when the amount field has been changed by the user
    *
    * @param amount const reference to the amount value
    */
  virtual void slotAmountChanged(const QString& amount);

  /**
    * Called when the nr field has been changed by the user
    *
    * @param nr const reference to the nr
    */
  virtual void slotNrChanged(const QString& nr);

  /**
    * Called when the date field has been changed by the user
    *
    * @param date const reference to the date
    */
  virtual void slotDateChanged(const QDate& date);

  /**
    * Called when the from account field has been changed by the user
    *
    * @param from const reference to the from account name
    */
  virtual void slotFromChanged(const QString& from);

  /**
    * Called when the to field has been changed by the user
    *
    * @param date const reference to the to account name
    */
  virtual void slotToChanged(const QString& to);

  /**
    * Called when a new payee entry has been edited
    * This routine will call the payee dialog and possibly add
    * the payee to the MyMoneyFile object
    */
  virtual void slotNewPayee(const QString& payee);

  /**
    * Called when editing a transaction begins
    */
  virtual void slotStartEdit(void);

  /**
    * Called when editing a transaction is cancelled
    */
  virtual void slotCancelEdit(void);

  /**
    * Called when editing a transaction is done and changes should be stored
    */
  virtual void slotEndEdit(void);

  /**
    * Called when a new transaction should be generated
    */
  virtual void slotNew(void);

protected:
  void reloadAccount(const bool repaint = true);
  void loadAccount(void);
  void filterTransactions(void);
  void sortTransactions(void);
  int transactionType(const MyMoneySplit& split) const;

  virtual void hideEvent(QHideEvent *ev);
  virtual void showWidgets(void) = 0;
  virtual void hideWidgets(void) = 0;

protected:
  /**
    * This member keeps a pointer to the specific register for the account
    */
  kMyMoneyRegister *m_register;

  /**
    * This member keeps a pointer to the specifc form for the account
    */
  kMyMoneyTransactionForm *m_form;

  /**
    * This member holds the date from which on transactions should be shown
    * in the ledger view. See KSettingsDlg where this value can be changed.
    */
  QDate m_dateStart;

  /**
    * This member keeps the account information for the account that is shown
    * in the specific view
    */
  MyMoneyAccount m_account;

  /**
    * This member holds a list of all transactions that belong to the account
    * shown in this view.
    */
  QValueList<MyMoneyTransaction> m_transactionList;

  /**
    * This member holds vector of the balances for each transaction
    */
  QValueVector<MyMoneyMoney> m_balance;

  /**
    * This member keeps a vector of pointers to all visible (filtered)
    * transaction in m_transactionList in sorted order. Sorting is done
    * in KTransactionPtrVector::compareItems
    */
  KTransactionPtrVector m_transactionPtrVector;

  /**
    * This member keeps a pointer to the currently selected transaction
    * It is NULL, if an empty (new) transaction is selected.
    */
  MyMoneyTransaction *m_transactionPtr;

  /**
    * This member keeps a copy of the currently selected transaction
    * during the edit phase. It will be used for all modifications.
    */
  MyMoneyTransaction m_transaction;

  /**
    * This member keeps a copy of the split that references the current
    * account.
    */
  MyMoneySplit m_split;

  /*
   * The following members are pointers to the edit widgets
   */
  kMyMoneyPayee*        m_editPayee;      ///< pointer to payee edit widget
  kMyMoneyCategory*     m_editCategory;   ///< pointer to category edit widget
  kMyMoneyLineEdit*     m_editMemo;       ///< pointer to memo edit widget
  kMyMoneyEdit*         m_editAmount;     ///< pointer to amount edit widget
  kMyMoneyLineEdit*     m_editNr;         ///< pointer to number edit widget
  kMyMoneyDateInput*    m_editDate;       ///< pointer to date edit widget
  kMyMoneyCategory*     m_editFrom;       ///< pointer to 'from account' edit widget
  kMyMoneyCategory*     m_editTo;         ///< pointer to 'to account' edit widget

private:
  QTimer*      m_timer;

private slots:
  void timerDone(void);

signals:
  /**
    * The signal transactionSelected(void) is emitted, when a different
    * transaction than the currently selected transaction is selected by
    * the user.
    */
  void transactionSelected(void);

};

#endif
