/***************************************************************************
                          mymoneyaccount.cpp
                          -------------------
    copyright            : (C) 2000 by Michael Edwardes
                           (C) 2002 by Thomas Baumagrt
    email                : mte@users.sourceforge.net
                           ipwizard@users.sourceforge.net
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
#include "mymoneyfile.h"

#include <iostream>

MyMoneyAccount::MyMoneyAccount()
{
  m_file = 0;
  m_openingBalance = 0;
  m_institution = "";
  m_id = "";
  m_name = "";
  m_description = "";
  m_number = "";
  m_parentAccount = "";
  m_accountType = UnknownAccountType;
}

MyMoneyAccount::~MyMoneyAccount()
{
}

MyMoneyAccount::MyMoneyAccount(const QString& id, const MyMoneyAccount& right)
{
  *this = right;
  m_id = id;
}

void MyMoneyAccount::setName(const QString& name)
{
  m_name = name;
}

void MyMoneyAccount::setNumber(const QString& number)
{
  m_number = number;
}

void MyMoneyAccount::setDescription(const QString& desc)
{
  m_description = desc;
}

void MyMoneyAccount::setInstitutionId(const QString& id)
{
  m_institution = id;
}

void MyMoneyAccount::setLastModified(const QDate& date)
{
  m_lastModified = date;
}

void MyMoneyAccount::setOpeningDate(const QDate& date)
{
  m_openingDate = date;
}

void MyMoneyAccount::setOpeningBalance(const MyMoneyMoney& balance)
{
  MyMoneyMoney diff;

  diff = balance - m_openingBalance;

  if(diff != 0) {
    QValueList<MyMoneyAccount::Transaction>::Iterator it_t;
    for(it_t = m_transactionList.begin(); it_t != m_transactionList.end(); ++it_t) {
      MyMoneyAccount::Transaction t((*it_t).transactionID(), (*it_t).balance()+diff);
      *it_t = t;
    }
  }
  m_openingBalance = balance;
}

void MyMoneyAccount::setLastReconciliationDate(const QDate& date)
{
  m_lastReconciliationDate = date;
}

void MyMoneyAccount::setParentAccountId(const QString& parent)
{
  m_parentAccount = parent;
}

void MyMoneyAccount::setAccountType(const accountTypeE type)
{
  m_accountType = type;
}

const QValueList<MyMoneyAccount::Transaction>& MyMoneyAccount::transactionList(void) const
{
  return m_transactionList;
}

/*
const MyMoneyAccount::Transaction& MyMoneyAccount::transaction(const QString id) const
{
  QValueList<MyMoneyAccount::Transaction>::ConstIterator it;

  for(it = m_transactionList.begin(); it != m_transactionList.end(); ++it) {
    if((*it).transactionID() == id)
      return *it;
  }
  throw new MYMONEYEXCEPTION("Invalid transaction id");
}
*/

const MyMoneyAccount::Transaction& MyMoneyAccount::transaction(const int idx) const
{
  if(idx >= 0 && idx < static_cast<int> (m_transactionList.count()))
    return m_transactionList[idx];
  throw new MYMONEYEXCEPTION("Invalid transaction index");
}

void MyMoneyAccount::addTransaction(const MyMoneyAccount::Transaction& val)
{
  MyMoneyAccount::Transaction v(val.transactionID(), balance() + val.balance());
  m_transactionList += v;
}

void MyMoneyAccount::clearTransactions(void)
{
  m_transactionList.clear();
}

const MyMoneyMoney MyMoneyAccount::balance(void) const
{
  MyMoneyMoney result(0);

  if(m_transactionList.count() > 0)
    result = m_transactionList.back().balance();
  return result;
}

void MyMoneyAccount::addAccountId(const QString& account)
{
  if(!m_accountList.contains(account))
    m_accountList += account;
}

void MyMoneyAccount::removeAccountId(const QString& account)
{
  QStringList::Iterator it;

  it = m_accountList.find(account);
  if(it != m_accountList.end())
    m_accountList.remove(it);
}