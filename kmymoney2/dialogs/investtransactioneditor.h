/***************************************************************************
                             investtransactioneditor.h
                             ----------
    begin                : Fri Dec 15 2006
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

#ifndef INVESTMENTTRANSACTIONEDITOR_H
#define INVESTMENTTRANSACTIONEDITOR_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes


// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/transactioneditor.h>

class InvestTransactionEditorPrivate;

class InvestTransactionEditor : public TransactionEditor
{
  friend class InvestTransactionEditorPrivate;

  Q_OBJECT
public:
  InvestTransactionEditor();
  InvestTransactionEditor(TransactionEditorContainer* regForm, KMyMoneyRegister::InvestTransaction* item, const QValueList<KMyMoneyRegister::SelectedTransaction>& list, const QDate& lastPostDate);
  virtual ~InvestTransactionEditor();

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
  virtual bool isComplete(void) const;

  virtual QWidget* firstWidget(void) const;

  virtual bool fixTransactionCommodity(const MyMoneyAccount& /* account */) { return true; }

  void totalAmount(MyMoneyMoney& amount) const;

  static void dissectTransaction(const MyMoneyTransaction& transaction, const MyMoneySplit& split, MyMoneySplit& assetAccountSplit, QValueList<MyMoneySplit>& feeSplits, QValueList<MyMoneySplit>& interestSplits, MyMoneySecurity& security, MyMoneySecurity& currency, MyMoneySplit::investTransactionTypeE& transactionType);

  bool setupPrice(const MyMoneyTransaction& t, MyMoneySplit& split);

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
    *
    * @param skipPriceDialog if @p true the user will not be requested for price information
    *                        (defaults to @p false)
    *
    * @return @p false if aborted by user, @p true otherwise
    *
    * @note Usually not used directly. If unsure, use enterTransactions() instead.
    */
  bool createTransaction(MyMoneyTransaction& t, const MyMoneyTransaction& torig, const MyMoneySplit& sorig, bool skipPriceDialog = false);

protected slots:
  void slotCreateSecurity(const QString& name, QCString& id);
  void slotCreateFeeCategory(const QString& name, QCString& id);
  void slotCreateInterestCategory(const QString& name, QCString& id);

  int slotEditInterestSplits(void);
  int slotEditFeeSplits(void);

  void slotUpdateActivity(MyMoneySplit::investTransactionTypeE);
  void slotUpdateSecurity(const QCString& stockId);
  void slotUpdateInterestCategory(const QCString& id);
  void slotUpdateInterestVisibility(const QString&);
  void slotUpdateFeeCategory(const QCString& id);
  void slotUpdateFeeVisibility(const QString&);
  void slotUpdateTotalAmount(void);

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

  void activityFactory(MyMoneySplit::investTransactionTypeE type);

  MyMoneyMoney subtotal(const QValueList<MyMoneySplit>& splits) const;

private:

private:
  MyMoneySplit                              m_assetAccountSplit;
  QValueList<MyMoneySplit>                  m_interestSplits;
  QValueList<MyMoneySplit>                  m_feeSplits;
  MyMoneySecurity                           m_security;
  MyMoneySecurity                           m_currency;
  MyMoneySplit::investTransactionTypeE      m_transactionType;
  InvestTransactionEditorPrivate* const     d;
};

#endif