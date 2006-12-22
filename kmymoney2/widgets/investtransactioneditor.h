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
  Q_OBJECT
public:
  InvestTransactionEditor();
  InvestTransactionEditor(TransactionEditorContainer* regForm, MyMoneyObjectContainer* objects, KMyMoneyRegister::Transaction* item, const QValueList<KMyMoneyRegister::SelectedTransaction>& list, const QDate& lastPostDate);
  virtual ~InvestTransactionEditor();

  virtual bool enterTransactions(QCString&);

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

  void setSplits(const MyMoneySplit& assetAccountSplit, const QValueList<MyMoneySplit>& interestSplits, const QValueList<MyMoneySplit>& feeSplits);

  virtual bool fixTransactionCommodity(const MyMoneyAccount& account) { return true; }

protected slots:
  void slotCreateSecurity(const QString& name, QCString& id);
  void slotCreateFeeCategory(const QString& name, QCString& id);
  void slotCreateInterestCategory(const QString& name, QCString& id);

  int slotEditInterestSplits(void);
  int slotEditFeeSplits(void);

  void slotUpdateActivity(KMyMoneyRegister::investTransactionTypeE);
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

  MyMoneyMoney subtotal(const QValueList<MyMoneySplit>& splits) const;

private:
  MyMoneySplit                              m_assetAccountSplit;
  QValueList<MyMoneySplit>                  m_interestSplits;
  QValueList<MyMoneySplit>                  m_feeSplits;
  MyMoneySecurity                           m_security;
  MyMoneySecurity                           m_currency;
  InvestTransactionEditorPrivate*           d;
};

#endif
