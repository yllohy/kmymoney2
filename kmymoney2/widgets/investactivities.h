/***************************************************************************
                             investactivities.h
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

#ifndef INVESTACTIVITIES_H
#define INVESTACTIVITIES_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/register.h>
#include <kmymoney/investtransactioneditor.h>

namespace Invest {

class Activity
{
public:
  virtual KMyMoneyRegister::investTransactionTypeE type(void) const = 0;
  virtual void showWidgets(void) const = 0;
  virtual bool isComplete(void) const = 0;
  virtual bool createTransaction(MyMoneyTransaction& t, MyMoneySplit& s0, MyMoneySplit& assetAccountSplit, QValueList<MyMoneySplit>& feeSplits, QValueList<MyMoneySplit>& interestSplits, MyMoneySecurity& security, MyMoneySecurity& currency) = 0;

protected:
  Activity(InvestTransactionEditor* editor) { m_parent = editor; }
  QWidget* haveWidget(const QString& name) const { return m_parent->haveWidget(name); }
  bool haveAssetAccount(void) const;
  bool haveFees(bool optional = false) const { return haveCategoryAndAmount("fee-account", "fee-amount", optional); }
  bool haveInterest(bool optional = false) const { return haveCategoryAndAmount("interest-account", "interest-amount", optional); }
  bool haveShares(void) const;
  bool havePrice(void) const;
  bool isMultiSelection(void) const { return m_parent->isMultiSelection(); }

private:
  bool haveCategoryAndAmount(const QString& category, const QString& amount, bool optional) const;

protected:
  InvestTransactionEditor*    m_parent;
};

class Buy : public Activity
{
public:
  Buy(InvestTransactionEditor* editor) : Activity(editor) {}
  virtual KMyMoneyRegister::investTransactionTypeE type(void) const { return KMyMoneyRegister::BuyShares; }
  virtual void showWidgets(void) const;
  virtual bool isComplete(void) const;
  virtual bool createTransaction(MyMoneyTransaction& t, MyMoneySplit& s0, MyMoneySplit& assetAccountSplit, QValueList<MyMoneySplit>& feeSplits, QValueList<MyMoneySplit>& interestSplits, MyMoneySecurity& security, MyMoneySecurity& currency);
};

class Sell : public Activity
{
public:
  Sell(InvestTransactionEditor* editor) : Activity(editor) {}
  virtual KMyMoneyRegister::investTransactionTypeE type(void) const { return KMyMoneyRegister::SellShares; }
  virtual void showWidgets(void) const;
  virtual bool isComplete(void) const;
  virtual bool createTransaction(MyMoneyTransaction& t, MyMoneySplit& s0, MyMoneySplit& assetAccountSplit, QValueList<MyMoneySplit>& feeSplits, QValueList<MyMoneySplit>& interestSplits, MyMoneySecurity& security, MyMoneySecurity& currency);
};

class Div : public Activity
{
public:
  Div(InvestTransactionEditor* editor) : Activity(editor) {}
  virtual KMyMoneyRegister::investTransactionTypeE type(void) const { return KMyMoneyRegister::Dividend; }
  virtual void showWidgets(void) const;
  virtual bool isComplete(void) const;
  virtual bool createTransaction(MyMoneyTransaction& t, MyMoneySplit& s0, MyMoneySplit& assetAccountSplit, QValueList<MyMoneySplit>& feeSplits, QValueList<MyMoneySplit>& interestSplits, MyMoneySecurity& security, MyMoneySecurity& currency);
};

class Reinvest : public Activity
{
public:
  Reinvest(InvestTransactionEditor* editor) : Activity(editor) {}
  virtual KMyMoneyRegister::investTransactionTypeE type(void) const { return KMyMoneyRegister::ReinvestDividend; }
  virtual void showWidgets(void) const;
  virtual bool isComplete(void) const;
  virtual bool createTransaction(MyMoneyTransaction& t, MyMoneySplit& s0, MyMoneySplit& assetAccountSplit, QValueList<MyMoneySplit>& feeSplits, QValueList<MyMoneySplit>& interestSplits, MyMoneySecurity& security, MyMoneySecurity& currency);
};

class Add : public Activity
{
public:
  Add(InvestTransactionEditor* editor) : Activity(editor) {}
  virtual KMyMoneyRegister::investTransactionTypeE type(void) const { return KMyMoneyRegister::AddShares; }
  virtual void showWidgets(void) const;
  virtual bool isComplete(void) const;
  virtual bool createTransaction(MyMoneyTransaction& t, MyMoneySplit& s0, MyMoneySplit& assetAccountSplit, QValueList<MyMoneySplit>& feeSplits, QValueList<MyMoneySplit>& interestSplits, MyMoneySecurity& security, MyMoneySecurity& currency);
};

class Remove : public Activity
{
public:
  Remove(InvestTransactionEditor* editor) : Activity(editor) {}
  virtual KMyMoneyRegister::investTransactionTypeE type(void) const { return KMyMoneyRegister::RemoveShares; }
  virtual void showWidgets(void) const;
  virtual bool isComplete(void) const;
  virtual bool createTransaction(MyMoneyTransaction& t, MyMoneySplit& s0, MyMoneySplit& assetAccountSplit, QValueList<MyMoneySplit>& feeSplits, QValueList<MyMoneySplit>& interestSplits, MyMoneySecurity& security, MyMoneySecurity& currency);
};

class Split : public Activity
{
public:
  Split(InvestTransactionEditor* editor) : Activity(editor) {}
  virtual KMyMoneyRegister::investTransactionTypeE type(void) const { return KMyMoneyRegister::SplitShares; }
  virtual void showWidgets(void) const;
  virtual bool isComplete(void) const;
  virtual bool createTransaction(MyMoneyTransaction& t, MyMoneySplit& s0, MyMoneySplit& assetAccountSplit, QValueList<MyMoneySplit>& feeSplits, QValueList<MyMoneySplit>& interestSplits, MyMoneySecurity& security, MyMoneySecurity& currency);
};

} // namespace Invest



#endif // INVESTACTIVITIES_H

