/***************************************************************************
                             transactioneditor.h
                             ----------
    begin                : Wed Jun 07 2006
    copyright            : (C) 2006 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef TRANSACTIONEDITOR_H
#define TRANSACTIONEDITOR_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qobject.h>
#include <qwidgetlist.h>

// ----------------------------------------------------------------------------
// KDE Includes


// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/mymoneyscheduled.h>
#include <kmymoney/transactioneditorcontainer.h>
#include <kmymoney/register.h>

class KCurrencyExchange;
class KMyMoneyCategory;

class TransactionEditor : public QObject
{
  Q_OBJECT
public:
  TransactionEditor() {};
  TransactionEditor(TransactionEditorContainer* regForm, KMyMoneyRegister::Transaction* item, const QValueList<KMyMoneyRegister::SelectedTransaction>& list, const QDate& lastPostDate);
  virtual ~TransactionEditor();

  /**
    * This method is used as a helper because virtual methods cannot be
    * called within a constructor. Thus setup() should be called immediately
    * after a TransactionEditor() object or one of its derivatives is
    * constructed. The parameter @a account identifies the account that
    * is currently opened in the calling ledger view.
    *
    * This account will not be included in category sets. The default is
    * no account so all will be shown. I have no idea anymore, what I
    * tried to say with the first sentence above. :(  Maybe this is crap.
    *
    * @param tabOrderWidgets QWidgetList which will be filled with the pointers
    *                        to the editWidgets in their tab order
    * @param account account that is currently shown in the calling ledger view
    * @param action default action (defaults to ActionNone).
    */
  void setup(QWidgetList& tabOrderWidgets, const MyMoneyAccount& account = MyMoneyAccount(), KMyMoneyRegister::Action action = KMyMoneyRegister::ActionNone);

  virtual bool enterTransactions(QCString&);

  /**
    * This method creates a transaction based on the contents of the current widgets,
    * the splits in m_split in single selection mode or an existing transaction/split
    * and the contents of the widgets in multi selection mode.
    *
    * The split referencing the current account is returned as the first split in the
    * transaction's split list.
    *
    * @param t reference to created transaction
    * @param torig the original transaction
    * @param sorig the original split
    * @param skipPriceDialog if @p true the user will not be requested for price information
    *                        (defaults to @p false)
    *
    * @return @p false if aborted by user, @p true otherwise
    *
    * @note Usually not used directly. If unsure, use enterTransactions() instead.
    */
  virtual bool createTransaction(MyMoneyTransaction& t, const MyMoneyTransaction& torig, const MyMoneySplit& sorig, bool skipPriceDialog = false) = 0;

  /**
    * This method returns information about the completeness of the data
    * entered. This can be used to control the availability of the
    * 'Enter transaction' action.
    *
    * @retval true if entering the transaction into the engine
    * @retval false if not enough information is present to enter the
    * transaction into the engine
    *
    * @sa transactionDataSufficient()
    */
  virtual bool isComplete(void) const = 0;

  /**
    * This method returns information if the editor is started with multiple transactions
    * being selected or not.
    *
    * @retval false only a single transaction was selected when the editor was started
    * @retval true multiple transactions were selected when the editor was started
    */
  virtual bool isMultiSelection(void) const { return m_transactions.count() > 1; }

  virtual bool fixTransactionCommodity(const MyMoneyAccount& account);

  virtual bool canAssignNumber(void) const;
  virtual void assignNumber(void);

  /**
    * Returns a pointer to the widget that should receive
    * the focus after the editor has been started.
    */
  virtual QWidget* firstWidget(void) const = 0;

  /**
    * Returns a pointer to a widget by name
    */
  QWidget* haveWidget(const QString& name) const;

  void setTransaction(const MyMoneyTransaction& t, const MyMoneySplit& s);

public slots:
  void slotReloadEditWidgets(void);

  /**
    * The default implementation returns QDialog::Rejected
    */
  virtual int slotEditSplits(void);

  /**
    * Modify the account which the transaction should be based on. The
    * initial value for the account is passed during setup().
    *
    * @param id of the account to be used
    */
  void slotUpdateAccount(const QCString& id);

protected:
  virtual void createEditWidgets(void) = 0;
  virtual void loadEditWidgets(KMyMoneyRegister::Action action = KMyMoneyRegister::ActionNone) = 0;
  void setupCategoryWidget(KMyMoneyCategory* category, const QValueList<MyMoneySplit>& splits, QCString& categoryId, const char* splitEditSlot, bool allowObjectCreation = true);

protected slots:
  void slotUpdateButtonState(void);
  void slotUpdateAccount(void);
  void slotNumberChanged(const QString&);

signals:
  /**
    * This signal is sent out by the destructor to inform other entities
    * that editing has been finished. The parameter @a t contains the list
    * of transactions that were processed.
    */
  void finishEdit(const QValueList<KMyMoneyRegister::SelectedTransaction>& t);

  /**
    * This signal is sent out whenever enough data is present to enter the
    * transaction into the ledger. This signal can be used to control the
    * KAction which implements entering the transaction.
    *
    * @sa isComplete()
    *
    * @param state @a true if enough data is present, @a false otherwise.
    */
  void transactionDataSufficient(bool state);

  /**
    * This signal is sent out, when a new payee needs to be created
    * @sa KMyMoneyCombo::createItem()
    *
    * @param txt The name of the payee to be created
    * @param id A connected slot should store the id of the created object in this variable
    */
  void createPayee(const QString& txt, QCString& id);

  /**
    * This signal is sent out, when a new category needs to be created
    * Depending on the setting of either a payment or deposit, the parent
    * account will be preset to Expense or Income.
    *
    * @param account reference to account info. Will be filled by called slot
    * @param parent reference to parent account
    */
  void createCategory(MyMoneyAccount& account, const MyMoneyAccount& parent);

  /**
    * This signal is sent out, when a new security (e.g. stock )needs to be created
    * @a Parent should be the investment account under which the security account
    * will be created.
    *
    * @param account reference to account info. Will be filled by called slot
    * @param parent reference to parent account
    */
  void createSecurity(MyMoneyAccount& account, const MyMoneyAccount& parent);

  /**
    * Signal is emitted, if any of the widgets enters (@a state equals @a true)
    *  or leaves (@a state equals @a false) object creation mode.
    *
    * @param state Enter (@a true) or leave (@a false) object creation
    */
  void objectCreation(bool state);

  void statusMsg(const QString& txt);

  void statusProgress(int cnt, int base);

  /**
    * This signal is sent out for each newly added transaction
    *
    * @param date the post date of the newly created transaction
    */
  void lastPostDateUsed(const QDate& date);

  /**
    * This signal is sent out, if the user decides to schedule the transaction @a t
    * rather then adding it to the ledger right away.
    */
  void scheduleTransaction(const MyMoneyTransaction& t, MyMoneySchedule::occurenceE occurence);

protected:
  QValueList<MyMoneySplit>                          m_splits;
  QValueList<KMyMoneyRegister::SelectedTransaction> m_transactions;
  TransactionEditorContainer*                       m_regForm;
  KMyMoneyRegister::Transaction*                    m_item;
  KMyMoneyRegister::QWidgetContainer                m_editWidgets;
  MyMoneyAccount                                    m_account;
  MyMoneyTransaction                                m_transaction;
  MyMoneySplit                                      m_split;
  QDate                                             m_lastPostDate;
  QMap<QCString, MyMoneyMoney>                      m_priceInfo;
  KMyMoneyRegister::Action                          m_initialAction;
  bool                                              m_openEditSplits;
};


class StdTransactionEditor : public TransactionEditor
{
  Q_OBJECT
public:
  StdTransactionEditor();
  StdTransactionEditor(TransactionEditorContainer* regForm, KMyMoneyRegister::Transaction* item, const QValueList<KMyMoneyRegister::SelectedTransaction>& list, const QDate& lastPostDate);

  bool isComplete(void) const;
  QWidget* firstWidget(void) const;

  /**
    * This method creates a transaction based on the contents of the current widgets,
    * the splits in m_split in single selection mode or an existing transaction/split
    * and the contents of the widgets in multi selection mode.
    *
    * The split referencing the current account is returned as the first split in the
    * transaction's split list.
    *
    * @param t reference to created transaction
    * @param torig the original transaction
    * @param sorig the original split
    * @param skipPriceDialog if @p true the user will not be requested for price information
    *                        (defaults to @p false)
    *
    * @return @p false if aborted by user, @p true otherwise
    *
    * @note Usually not used directly. If unsure, use enterTransactions() instead.
    */
  bool createTransaction(MyMoneyTransaction& t, const MyMoneyTransaction& torig, const MyMoneySplit& sorig, bool skipPriceDialog = false);

public slots:
  void slotReloadEditWidgets(void);
  int slotEditSplits(void);

protected slots:
  void slotUpdatePayment(const QString&);
  void slotUpdateDeposit(const QString&);
  void slotUpdateAmount(const QString&);
  void slotUpdateCategory(const QCString&);
  void slotUpdatePayee(const QCString&);
  void slotUpdateCashFlow(KMyMoneyRegister::CashFlowDirection);
  void slotCreateCategory(const QString&, QCString&);
  void slotUpdateAction(int action);

protected:
  /**
    * This method creates all necessary widgets for this transaction editor.
    * All signals will be connected to the relevant slots.
    */
  void createEditWidgets(void);

  /**
    * This method (re-)loads the widgets with the transaction information
    * contained in @a m_transaction and @a m_split.
    *
    * @param action preset the edit wigdets for @a action if no transaction
    *               is present
    */
  void loadEditWidgets(KMyMoneyRegister::Action action = KMyMoneyRegister::ActionNone);

  void setupCategoryWidget(QCString&);
  void updateAmount(const MyMoneyMoney& value);
  bool isTransfer(const QCString& accId1, const QCString& accId2) const;

  void checkPayeeInSplit(MyMoneySplit& s, const QCString& payeeId);

  /**
    * This method fills the editor widgets with the last transaction
    * that can be found for payee @a payeeId in the account @a m_account.
    */
  void autoFill(const QCString& payeeId);

  /**
    * Extracts the amount of the transaction from the widgets depending
    * if form or register based input method is used.
    * Returns if an amount has been found in @a update.
    *
    * @param update pointer to update information flag
    * @return amount of transaction (deposit positive, payment negative)
    */
  MyMoneyMoney amountFromWidget(bool* update = 0) const;

  /**
    * Create or update a VAT split
    */
  void updateVAT(bool amountChanged = true);

  MyMoneyMoney removeVatSplit(void);

  /**
    * This method adds a VAT split to transaction @a tr if necessary.
    *
    * @param tr transaction that the split should be added to
    * @param amount Amount to be used for the calculation. Depending upon the
    *               setting of the resp. category, this value is treated as
    *               either gross or net value.
    * @retval false VAT split has not been added
    * @retval true VAT split has been added
    */
  bool addVatSplit(MyMoneyTransaction& tr, const MyMoneyMoney& amount);

private:
  MyMoneyMoney        m_shares;
  bool                m_inUpdateVat;
};


#endif
