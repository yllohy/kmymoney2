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

MyMoneyTransaction::MyMoneyTransaction()
{
}

MyMoneyTransaction::MyMoneyTransaction(const long id, transactionMethod method, const QString& number, const QString& memo,
                     const MyMoneyMoney& amount, const QDate& date, const QString& categoryMajor, const QString& categoryMinor, const QString& atmName,
                     const QString& fromTo, const QString& bankFrom, const QString& bankTo, stateE state)
{
  m_id=id;
  m_number = number;
  m_memo = memo;
  m_method = method;
  m_amount = amount;
  m_date = date;
  m_categoryMajor = categoryMajor;
  m_categoryMinor = categoryMinor;
  m_atmBankName = atmName;
  m_payee = fromTo;
  m_accountFrom = bankFrom;
  m_accountTo = bankTo;
  m_state = state;
  m_index=0;
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
  m_id = right.m_id;
  m_number = right.m_number;
  m_memo = right.m_memo;
  m_amount = right.m_amount;
  m_date = right.m_date;
  m_method = right.m_method;
  m_categoryMajor = right.m_categoryMajor;
  m_categoryMinor = right.m_categoryMinor;
  m_atmBankName = right.m_atmBankName;
  m_payee = right.m_payee;
  m_accountFrom = right.m_accountFrom;
  m_accountTo = right.m_accountTo;
  m_state = right.m_state;
  m_index = right.m_index;
}

MyMoneyTransaction& MyMoneyTransaction::operator = (const MyMoneyTransaction& right)
{
  m_id = right.m_id;
  m_number = right.m_number;
  m_memo = right.m_memo;
  m_amount = right.m_amount;
  m_date = right.m_date;
  m_method = right.m_method;
  m_categoryMajor = right.m_categoryMajor;
  m_categoryMinor = right.m_categoryMinor;
  m_atmBankName = right.m_atmBankName;
  m_payee = right.m_payee;
  m_accountFrom = right.m_accountFrom;
  m_accountTo = right.m_accountTo;
  m_state = right.m_state;
  m_index = right.m_index;
  return *this;
}

bool MyMoneyTransaction::operator == (const MyMoneyTransaction& right)
{
  if ( (m_id == right.m_id) &&
      (m_number == right.m_number) &&
      (m_memo == right.m_memo) &&
      (m_amount == right.m_amount) &&
      (m_date == right.m_date) &&
      (m_method == right.m_method) &&
      (m_categoryMajor == right.m_categoryMajor) &&
      (m_categoryMinor == right.m_categoryMinor) &&
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
