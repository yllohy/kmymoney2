/***************************************************************************
                          mymoneyaccount.cpp
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

#include "mymoneyaccount.h"

MyMoneyAccount::MyMoneyAccount()
{
//  m_transactions.setAutoDelete(true);
  m_lastId = 0L;
  m_accountType = MyMoneyAccount::Current;
  m_lastReconcile = QDate(1950, 1, 1); // After gregorian got implemented in most countries ???
}

MyMoneyAccount::MyMoneyAccount(const QString& name, const QString& number, accountTypeE type, const QString& description, const QDate& lastReconcile)
{
  m_accountName = name;
  m_accountNumber = number;
//  m_transactions.setAutoDelete(true);
  m_lastId=0L;
  m_accountType = type;
  m_description = description;
  m_lastReconcile = lastReconcile;
}

MyMoneyAccount::~MyMoneyAccount()
{
}

MyMoneyMoney MyMoneyAccount::balance(void) const
{
  // Recalculate the balance each time it is requested
  MyMoneyMoney balance;

  QListIterator<MyMoneyTransaction> it(m_transactions);
  for ( ; it.current(); ++it )
 {
    MyMoneyTransaction *trans = it.current();
    if (trans->type()==MyMoneyTransaction::Credit)
      balance += trans->amount();
    else
      balance -= trans->amount();
  }

  return balance;
}

MyMoneyTransaction* MyMoneyAccount::transaction(const MyMoneyTransaction& transaction)
{
  unsigned int pos;
  if (findTransactionPosition(transaction, pos)) {
    return m_transactions.at(pos);
  }
  return 0;
}

void MyMoneyAccount::clear(void)
{
  m_transactions.clear();
}

MyMoneyTransaction* MyMoneyAccount::transactionFirst(void)
{
  return m_transactions.first();
}

MyMoneyTransaction* MyMoneyAccount::transactionNext(void)
{
  return m_transactions.next();
}

MyMoneyTransaction* MyMoneyAccount::transactionLast(void)
{
  return m_transactions.last();
}

MyMoneyTransaction* MyMoneyAccount::transactionAt(int index)
{
 	return m_transactions.at();
}

unsigned int MyMoneyAccount::transactionCount(void) const
{
  return m_transactions.count();
}

bool MyMoneyAccount::removeCurrentTransaction(unsigned int pos)
{
  return m_transactions.remove(pos);
}

bool MyMoneyAccount::removeTransaction(const MyMoneyTransaction& transaction)
{
  unsigned int pos;
  if (findTransactionPosition(transaction, pos))
    return m_transactions.remove(pos);
  return false;
}

bool MyMoneyAccount::addTransaction(MyMoneyTransaction::transactionMethod methodType, const QString& number, const QString& memo,
  const MyMoneyMoney& amount, const QDate& date, const QString& categoryMajor, const QString& categoryMinor, const QString& atmName,
  const QString& fromTo, const QString& bankFrom, const QString& bankTo, MyMoneyTransaction::stateE state)
{
  MyMoneyTransaction *transaction = new MyMoneyTransaction(m_lastId++, methodType, number,
    memo, amount, date, categoryMajor, categoryMinor, atmName,
    fromTo, bankFrom, bankTo, state);


  if (m_transactions.isEmpty()) {
    m_transactions.append(transaction);
    return true;
  }
  int idx=0;

  // Sort on date
  QListIterator<MyMoneyTransaction> it(m_transactions);
  for ( ; it.current(); ++it, idx++ ) {
    MyMoneyTransaction *trans = it.current();
    if (trans->date() < date)
      continue;
    else if (trans->date()==date)
      continue;
    else
      break;
  }

  m_transactions.insert(idx,transaction);

  return true;
}

bool MyMoneyAccount::findTransactionPosition(const MyMoneyTransaction& transaction, unsigned int& pos)
{
  int k=0;

  QListIterator<MyMoneyTransaction> it(m_transactions);
  for (k=0; it.current(); ++it, k++) {
    if (*it.current() == transaction) {
      pos=k;
      return true;
    }
  }
  pos=k;
  return false;
}

bool MyMoneyAccount::operator == (const MyMoneyAccount& right)
{
  if ( (m_accountName == right.m_accountName) ) {
    if (m_accountNumber == right.m_accountNumber) {
      if (m_accountType == right.m_accountType) {
        if (m_description == right.m_description) {
          if (m_lastReconcile == right.m_lastReconcile) {
            return true;
          }
        }
      }
    }
  }
  return false;
}

MyMoneyAccount::MyMoneyAccount(const MyMoneyAccount& right)
{
  m_accountName = right.m_accountName;
  m_accountNumber = right.m_accountNumber;
  m_accountType = right.m_accountType;
  m_lastId = right.m_lastId;
  m_description = right.m_description;
  m_lastReconcile = right.m_lastReconcile;
  m_balance = right.m_balance;
  m_transactions.clear();
  m_transactions = right.m_transactions;
}

MyMoneyAccount& MyMoneyAccount::operator = (const MyMoneyAccount& right)
{
  m_accountName = right.m_accountName;
  m_accountNumber = right.m_accountNumber;
  m_accountType = right.m_accountType;
  m_lastId = right.m_lastId;
  m_description = right.m_description;
  m_lastReconcile = right.m_lastReconcile;
  m_balance = right.m_balance;
  m_transactions.clear();
  m_transactions = right.m_transactions;
  return *this;
}

QDataStream &operator<<(QDataStream &s, const MyMoneyAccount &account)
{
  return s << account.m_accountName
    << account.m_description
    << account.m_accountNumber
    << (Q_INT32)account.m_accountType
    << account.m_lastReconcile;
}

QDataStream &operator>>(QDataStream &s, MyMoneyAccount &account)
{
  return s >> account.m_accountName
    >> account.m_description
    >> account.m_accountNumber
    >> (Q_INT32 &)account.m_accountType
    >> account.m_lastReconcile;
}
