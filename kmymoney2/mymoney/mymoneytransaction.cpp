/***************************************************************************
                          mymoneytransaction.cpp
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes
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
#include "mymoneytransaction.h"
#include "mymoney_config.h"

#include "mymoneyfile.h"
#include "mymoneybank.h"
#include "mymoneyaccount.h"

MyMoneyTransaction::MyMoneyTransaction()
{
	m_parent=0;
}

MyMoneyTransaction::MyMoneyTransaction(MyMoneyAccount *parent, const long id, transactionMethod method, const QString& number, const QString& memo,
                     const MyMoneyMoney& amount, const QDate& date, const QString& categoryMajor, const QString& categoryMinor, const QString& atmName,
                     const QString& fromTo, const QString& bankFrom, const QString& bankTo, stateE state)
  : MyMoneyTransactionBase(memo, amount, categoryMajor, categoryMinor)
{
	m_parent = parent;
  m_id=id;
  m_number = number;
  m_method = method;
  m_date = date;
  m_atmBankName = atmName;
  m_payee = fromTo;
  m_accountFrom = bankFrom;
  m_accountTo = bankTo;
  m_state = state;
  m_index=0;
  m_splitList.setAutoDelete(true);
}

MyMoneyTransaction::~MyMoneyTransaction()
{
}

MyMoneyTransaction::transactionType MyMoneyTransaction::type(void) const
{
  transactionType ltype=Debit;
  switch (m_method) {
    case MyMoneyTransaction::Cheque:
      ltype = Debit;
      break;
    case MyMoneyTransaction::Deposit:
      ltype = Credit;
      break;
    case MyMoneyTransaction::Transfer:
      ltype = Debit;
      break;
    case MyMoneyTransaction::Withdrawal:
      ltype = Debit;
      break;
    case MyMoneyTransaction::ATM:
      ltype = Debit;
      break;
  }
  return ltype;
}

MyMoneyTransaction::MyMoneyTransaction(const MyMoneyTransaction& right)
{
  init(const_cast<MyMoneyTransaction&> (right));
}

MyMoneyTransaction& MyMoneyTransaction::operator = (const MyMoneyTransaction& right)
{
  init(const_cast<MyMoneyTransaction&> (right));
  return *this;
}

void MyMoneyTransaction::init(MyMoneyTransaction& right)
{
  // set list to auto delete mode
  m_splitList.setAutoDelete(true);

  // copy base class members
  MyMoneyTransactionBase::init(right);

  // copy class members
  m_id = right.m_id;
  m_number = right.m_number;
  m_date = right.m_date;
  m_method = right.m_method;
  m_atmBankName = right.m_atmBankName;
  m_payee = right.m_payee;
  m_accountFrom = right.m_accountFrom;
  m_accountTo = right.m_accountTo;
  m_state = right.m_state;
  m_index = right.m_index;
	m_parent = right.m_parent;

  // copy the split list
  MyMoneySplitTransaction* split, *tmp;
  int current;

  current = right.m_splitList.at();
  split = right.firstSplit();
  while(split) {
    tmp = new MyMoneySplitTransaction(*split);
    addSplit(tmp);
    split = right.nextSplit();
  }
  if(current != -1)
    right.m_splitList.at(current);
}

bool MyMoneyTransaction::operator == (const MyMoneyTransaction& right)
{
  if ( (m_id == right.m_id) &&
      (m_number == right.m_number) &&
      (memo() == right.memo()) &&
      (amount() == right.amount()) &&
      (m_date == right.m_date) &&
      (m_method == right.m_method) &&
      (categoryMajor() == right.categoryMajor()) &&
      (categoryMinor() == right.categoryMinor()) &&
      (m_atmBankName == right.m_atmBankName) &&
      (m_payee == right.m_payee) &&
      (m_accountFrom == right.m_accountFrom) &&
      (m_accountTo == right.m_accountTo) &&
      (m_state == right.m_state) ) {
    return true;
  } else
    return false;
}

QDataStream &operator<<(QDataStream &s, const MyMoneyTransaction &trans)
{
  return s << (Q_UINT32)trans.m_id
    << trans.m_number
    << trans.m_payee
    << trans.m_amount
    << trans.m_date
    << (Q_UINT32)trans.m_method
    << trans.m_categoryMajor
    << trans.m_categoryMinor
    << trans.m_atmBankName
    << trans.m_accountFrom
    << trans.m_accountTo
    << trans.m_memo
    << (Q_INT32)trans.m_state;
  // no need to save m_index as its just an internal counter
}

QDataStream &operator>>(QDataStream &s, MyMoneyTransaction &trans)
{
  s >> (Q_UINT32 &)trans.m_id
    >> trans.m_number
    >> trans.m_payee
    >> trans.m_amount
    >> trans.m_date
    >> (Q_UINT32 &)trans.m_method
    >> trans.m_categoryMajor
    >> trans.m_categoryMinor
    >> trans.m_atmBankName
    >> trans.m_accountFrom
    >> trans.m_accountTo
    >> trans.m_memo
    >> (Q_INT32 &)trans.m_state;
    return s;
}

bool MyMoneyTransaction::readAllData(int version, QDataStream& stream)
{
  // ignore version for now.
  stream >> (Q_UINT32 &)m_id
    >> m_number
    >> m_payee
    >> m_amount
    >> m_date
    >> (Q_UINT32 &)m_method
    >> m_categoryMajor
    >> m_categoryMinor
    >> m_atmBankName
    >> m_accountFrom
    >> m_accountTo
    >> m_memo
    >> (Q_INT32 &)m_state;
    return true;
}

void MyMoneyTransaction::setNumber(const QString& val) { m_number = val; setDirty(true); }
void MyMoneyTransaction::setDate(const QDate& date) { m_date = date; setDirty(true); }
void MyMoneyTransaction::setMethod(const transactionMethod method) { m_method = method; setDirty(true); }
void MyMoneyTransaction::setAtmBankName(const QString& val) { m_atmBankName = val; setDirty(true); }
void MyMoneyTransaction::setPayee(const QString& fromTo) { m_payee = fromTo; setDirty(true); }
void MyMoneyTransaction::setAccountFrom(const QString& bankFrom) { m_accountFrom = bankFrom; setDirty(true); }
void MyMoneyTransaction::setAccountTo(const QString& bankTo) { m_accountTo = bankTo; setDirty(true); }
void MyMoneyTransaction::setState(const stateE state) { m_state = state; setDirty(true); }
void MyMoneyTransaction::setIndex(const unsigned int index) { m_index = index; setDirty(true); }

MyMoneyTransaction::transactionMethod MyMoneyTransaction::stringToMethod(const char *method)
{
  if ((strcmp(method, "Deposit"))==0)
    return Deposit;
  else if ((strcmp(method, "Cheque"))==0)
    return Cheque;
  else if ((strcmp(method, "Transfer"))==0)
    return Transfer;
  else if ((strcmp(method, "Withdrawal"))==0)
    return Withdrawal;
  else if ((strcmp(method, "ATM"))==0)
    return ATM;
  qDebug("Invalid transaction method '%s'. Use ATM instead.", method);
  return ATM;
}

void MyMoneyTransaction::setDirty(bool flag)
{
  if (m_parent)
    m_parent->setDirty(flag);
}

MyMoneySplitTransaction* const MyMoneyTransaction::firstSplit(void)
{
  return m_splitList.first();
}

MyMoneySplitTransaction* const MyMoneyTransaction::nextSplit(void)
{
  return m_splitList.next();
}

void MyMoneyTransaction::clearSplitList(void)
{
  m_splitList.clear();
}

void MyMoneyTransaction::addSplit(MyMoneySplitTransaction* const split)
{
  m_splitList.append(split);
  setCategoryMajor("Split");
  setCategoryMinor("");
}

