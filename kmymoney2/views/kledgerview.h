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
#include <qwidgetlist.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>

// ----------------------------------------------------------------------------
// Project Includes

class kMyMoneyRegister;
class kMyMoneyTransactionForm;
class kMyMoneyTransactionFormTable;
class kMyMoneyPayee;
class kMyMoneyCategory;
class kMyMoneyEdit;
class kMyMoneyLineEdit;
class kMyMoneyDateInput;
class KPushButton;

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
  *
  * Each KLedgerView is devided into three parts:
  *
  * - a @b register specific to the account type
  * - a set of @b buttons to control the dialog
  * - a @b transaction-form specific to the account type
  *
  * If the specific account provides multiple transaction types, the
  * form can additionaly provide a @b tab. This is handled by the specific
  * implementation of the ledger view. See KLedgerViewCheckings for an
  * example of this feature.
  *
  * The register is provided by a derived class of the kMyMoneyRegister
  * widget matching the account type (e.g. kMyMoneyRegisterCheckings).
  *
  * The buttons allow to select the following functions for the selected
  * transaction:
  *
  * - enter new transaction (New)
  * - start editing new transaction (Edit)
  * - cancel editing (Cancel)
  * - end editing (Enter)
  * - selecting additional functions (More)
  *
  * The buttons are provided by the kMyMoneyTransactionForm class.
  *
  * The form is based on an invisible QTable widget. This allows an aligned
  * arrangement of the fields and also a different visual appearance for
  * read-only and edit mode. The maintenance of the table object, it's size,
  * layout and contents has to be performed by the derived class.
  *
  * This class provides the member m_tabOrderWidgets to support a specific
  * form of the tab order for the keyboard focus while editing a transaction.
  * It has to be filled and maintained by the derived class.
  */
class KLedgerView : public QWidget, MyMoneyObserver  {
  Q_OBJECT

  friend class kMyMoneyTransactionFormTable;

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

  /**
    * This method is called by KGlobalLedgerView::selectAccount to set
    * the current account to @p accountId. It calls
    * loadAccount() to do the actual loading of data from the engine.
    * If will also register
    * this object as an observer with the engine's account represented by
    * @p accountId which causes update() to be called whenever the engine's
    * representation of the account changes.
    *
    * Any previously registered observer will be detached.
    *
    * @param accountId id of the account to be loaded.
    * @see update
    */
  void setCurrentAccount(const QCString& accountId);

  /**
    * This is the observer function called by the MyMoneyFile notify
    * method when the account has been changed. It forces a reloadAccount() of the
    * account into the ledger.
    *
    * @param accountId id of the account to be updated. This is same id
    *                  as requested for setCurrentAccount().
    */
  void update(const QCString& accountId);

  /**
    * This method returns a pointer to the transaction data
    * in the ledger of this account. The transaction is identified
    * by the parameter @p idx.
    *
    * @param idx index into ledger starting at 0
    * @return pointer to MyMoneyTransaction object representing the
    *         selected transaction. If idx is out of bounds,
    *         0 will be returned.
    */
  MyMoneyTransaction* const transaction(const int idx) const;

  /**
    * This method returns the balance of any visible transaction
    * in the ledger of this account. The balance depends on filters
    * and is automatically calculated when any view option is changed
    * (e.g. filters, sort order, etc.)
    *
    * @param idx index into the ledger starting at 0
    * @return Value of the balance for the account after the selected
    *         transaction has been taken into account. If idx is out
    *         of bounds, 0 will be returned as value.
    */
  const MyMoneyMoney& balance(const int idx) const;

  /**
    * This method returns the id of the account that is currently
    * shown by this widget.
    *
    * @return const QCString containing the account's id.
    */
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
    * @param showHotkey If true, the hotkey will be part of the returned
    *                   value. If false, the hotkey will not be shown.
    * @return QString with translated action string
    */
  const QString action2str(const QCString& action, const bool showHotkey = false) const;

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
    * This method refreshes the current view. This includes reading the
    * configuration options for the filters, the register and rebuilding
    * the filters by calling filterTransactions().
    */
  virtual void refreshView(void);

  /**
    *
    */
  virtual void slotRegisterClicked(int row, int col, int button, const QPoint &mousePos);

  /**
    * This method selects the next transaction if not
    * at the end of the ledger.
    */
  virtual void slotNextTransaction(void);

  /**
    * This method selects the previous transaction if not
    * at the beginning of the ledger.
    */
  virtual void slotPreviousTransaction(void);

  /**
    * Called when the user changes the visibility
    * setting of the transaction form
    */
  virtual void slotShowTransactionForm(bool show);

  /**
    * Called when the payee field has been changed.
    * m_transaction and m_split will be updated accordingly.
    *
    * @param name const reference to the name of the payee
    */
  virtual void slotPayeeChanged(const QString &name);

  /**
    * Called when the memo field has been changed.
    * m_transaction and m_split will be updated accordingly.
    *
    * @param memo const reference to the new memo text
    */
  virtual void slotMemoChanged(const QString &memo);

  /**
    * Called when the category field has been changed.
    * m_transaction and m_split will be updated accordingly.
    *
    * @param name const reference to the name of the category
    */
  virtual void slotCategoryChanged(const QString& category);

  /**
    * Called when the amount field has been changed by the user.
    * m_transaction and m_split will be updated accordingly.
    *
    * @param amount const reference to the amount value
    */
  virtual void slotAmountChanged(const QString& amount);

  /**
    * Called when the payment field has been changed by the user.
    * m_transaction and m_split will be updated accordingly.
    *
    * @param amount const reference to the amount value
    */
  virtual void slotPaymentChanged(const QString& amount);

  /**
    * Called when the deposit field has been changed by the user.
    * m_transaction and m_split will be updated accordingly.
    *
    * @param amount const reference to the amount value
    */
  virtual void slotDepositChanged(const QString& amount);

  /**
    * Called when the nr field has been changed by the user.
    * m_transaction and m_split will be updated accordingly.
    *
    * @param nr const reference to the nr
    */
  virtual void slotNrChanged(const QString& nr);

  /**
    * Called when the date field has been changed by the user.
    * m_transaction and m_split will be updated accordingly.
    *
    * @param date const reference to the date
    */
  virtual void slotDateChanged(const QDate& date);

  /**
    * Called when the from account field has been changed by the user.
    * m_transaction and m_split will be updated accordingly.
    *
    * @param from const reference to the from account name
    */
  virtual void slotFromChanged(const QString& from);

  /**
    * Called when the to field has been changed by the user.
    * m_transaction and m_split will be updated accordingly.
    *
    * @param date const reference to the to account name
    */
  virtual void slotToChanged(const QString& to);

  /**
    * Called when a new payee entry has been edited
    * This routine will call the payee dialog and possibly add
    * the payee to the MyMoneyFile object.
    *
    * @param payee const reference to the payee's name
    */
  virtual void slotNewPayee(const QString& payee);

  /**
    * Called when editing a transaction begins.
    * This will ensure, that the transaction is visible in the
    * register, the state of the buttons is updated and that
    * the edit widgets will be shown (see showWidgets()).
    */
  virtual void slotStartEdit(void);

  /**
    * Called when editing a transaction is cancelled.
    * This will ensure, that the edit widgets are removed
    * from the screen (see hideWidgets()) and that the state of
    * the buttons is updated. The transaction is not written
    * back to the engine. All changes made during the last
    * call to slotStartEdit are lost.
    */
  virtual void slotCancelEdit(void);

  /**
    * Called when editing a transaction is done and changes should be stored.
    * This will ensure, that
    * @li the changes made to the transaction are logically correct,
    * @li the changes are written back to the engine (see MyMoneyFile::modifyTransaction()
    *     and MyMoneyFile::addTransaction())
    * @li the state of the buttons is updated
    * @li the edit widgets are removed (see hideWidget())
    */
  virtual void slotEndEdit(void);

  /**
    * Called when a new transaction should be generated.
    * This will select the 'new' transaction (last line of the register)
    * fill the form, update button states and show the edit widgets
    * (see showWidgets()).
    */
  virtual void slotNew(void);

protected:
  /**
    * This method reloads the account data from the engine, refreshes
    * the view using refreshView() and repaints the register if not
    * suppressed by an argument. If repainting is requested, the transaction
    * form is also updated using fillForm().
    *
    * @param repaint If true, the register is repainted with the new
    *                the new values, if false, repainting is suppressed.
    *                The default is true.
    */
  void reloadAccount(const bool repaint = true);

  /**
    * This method reloads the account data using reloadAccount(), selects
    * the last transaction in the current register as the current
    * transaction and fills the form with the data of this transaction
    * using fillForm().
    */
  void loadAccount(void);

  /**
    * This method clears the m_TransactionPtrVector and rebuilds and sorts
    * it according to the current settings. Once the m_transactionPtrVector
    * is available, it rebuilds the m_balance vector.
    */
  void filterTransactions(void);

  /**
    * This method converts the actions strings contained in a split
    * (e.g. MyMoneySplit::ActionATM) into the internal used numeric values.
    *
    * @param const reference to the split
    * @return KLedgerView::transactionTypeE value of the action
    */
  int transactionType(const MyMoneySplit& split) const;

  /**
    * This method handles the focus of the keyboard. When in edit mode
    * (m_editDate widget is visible) the keyboard focus is handled
    * according to the widgets that are referenced in m_tabOrderWidgets.
    * If not in edit mode, the base class functionality is provided.
    *
    * @param next true if forward-tab, false if backward-tab was
    *             pressed by the user
    */
  virtual bool focusNextPrevChild(bool next);

  /**
    * This method is called when the edit widgets for the transaction
    * should be shown and editing a transaction starts. Since it's pure
    * virtual, it must be provided by the derived classes.
    */
  virtual void showWidgets(void) = 0;

  /**
    * This method is called when the edit widgets for the transaction
    * should be hidden and editing a transaction ends. Since it's pure
    * virtual, it must be provided by the derived classes.
    */
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
    * This member keeps the visibility status of the transaction form.
    */
  bool m_transactionFormActive;

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
  kMyMoneyEdit*         m_editPayment;    ///< pointer to payment edit widget
  kMyMoneyEdit*         m_editDeposit;    ///< pointer to deposit edit widget
  kMyMoneyLineEdit*     m_editNr;         ///< pointer to number edit widget
  kMyMoneyDateInput*    m_editDate;       ///< pointer to date edit widget
  kMyMoneyCategory*     m_editFrom;       ///< pointer to 'from account' edit widget
  kMyMoneyCategory*     m_editTo;         ///< pointer to 'to account' edit widget
  KPushButton*          m_editSplit;      ///< pointer to split button

  /**
    * This member keeps the tab order for the above widgets
    */
  QWidgetList   m_tabOrderWidgets;

  /**
    * This member contains the current setting of the ledger lens
    */
  bool          m_ledgerLens;

private:
  QTimer*       m_timer;

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
