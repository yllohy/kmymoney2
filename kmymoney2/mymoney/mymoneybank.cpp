/***************************************************************************
                          mymoneybank.cpp
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

#include "mymoneybank.h"

MyMoneyBank::MyMoneyBank()
{
//  m_accounts.setAutoDelete(true);
}

MyMoneyBank::MyMoneyBank(const QString& name, const QString& sortCode, const QString& city, const QString& street,
  const QString& postcode, const QString& telephone, const QString& manager)
{
//  m_accounts.setAutoDelete(true);
  m_name = name;
  m_city = city;
  m_street = street;
  m_postcode = postcode;
  m_telephone = telephone;
  m_manager = manager;
  m_sortCode = sortCode;
}

MyMoneyBank::~MyMoneyBank()
{
}

void MyMoneyBank::clear(void)
{
  m_name = QString::null;

  QListIterator<MyMoneyAccount> it(m_accounts);
  for ( ; it.current(); ++it )
    it.current()->clear();

  m_accounts.clear();
}

bool MyMoneyBank::newAccount(const QString& name, const QString& number, MyMoneyAccount::accountTypeE type,
  const QString& description, const QDate& lastReconcile)
{
  MyMoneyAccount *account = new MyMoneyAccount(name, number, type, description, lastReconcile);
  m_accounts.append(account);
  return true;
}

MyMoneyAccount* MyMoneyBank::account(const MyMoneyAccount& account)
{
  unsigned int pos;

  if (findAccountPosition(account, pos)) {
    return m_accounts.at(pos);
  }
  return 0;
}

MyMoneyAccount* MyMoneyBank::accountFirst(void)
{
  return m_accounts.first();
}

MyMoneyAccount* MyMoneyBank::accountNext(void)
{
  return m_accounts.next();
}

MyMoneyAccount* MyMoneyBank::accountLast(void)
{
  return m_accounts.last();
}

unsigned int MyMoneyBank::accountCount(void)
{
  return m_accounts.count();
}

bool MyMoneyBank::removeAccount(const MyMoneyAccount& account)
{
  unsigned int pos;

  if (findAccountPosition(account, pos))
    return m_accounts.remove(pos);
  return false;
}

bool MyMoneyBank::findAccountPosition(const MyMoneyAccount& account, unsigned int& pos)
{
  int k=0;
  QListIterator<MyMoneyAccount> it(m_accounts);
  for (k=0; it.current(); ++it, k++) {
    if (*it.current() == account) {
      pos=k;
      return true;
    }
  }
  pos=k;
  return false;
}

MyMoneyBank::MyMoneyBank(const MyMoneyBank& right)
{
  m_name = right.m_name;
  m_city = right.m_city;
  m_street = right.m_street;
  m_postcode = right.m_postcode;
  m_telephone = right.m_telephone;
  m_manager = right.m_manager;
  m_accounts.clear();
  m_accounts = right.m_accounts;
  m_sortCode = right.m_sortCode;
}

MyMoneyBank& MyMoneyBank::operator = (const MyMoneyBank& right)
{
  m_name = right.m_name;
  m_city = right.m_city;
  m_street = right.m_street;
  m_postcode = right.m_postcode;
  m_telephone = right.m_telephone;
  m_manager = right.m_manager;
  m_accounts.clear();
  m_accounts = right.m_accounts;
  m_sortCode = right.m_sortCode;
  return *this;
}

bool MyMoneyBank::operator == (const MyMoneyBank& right)
{
  if ( (m_name == right.m_name) &&
      (m_city == right.m_city) &&
      (m_street == right.m_street) &&
      (m_postcode == right.m_postcode) &&
      (m_telephone == right.m_telephone) &&
      (m_sortCode == right.m_sortCode) &&
      (m_manager == right.m_manager) ) {
    return true;
  } else
    return false;
}

QDataStream &operator<<(QDataStream &s, const MyMoneyBank &bank)
{
  return s << bank.m_name
    << bank.m_city
    << bank.m_street
    << bank.m_postcode
    << bank.m_telephone
    << bank.m_manager
    << bank.m_sortCode;
}

QDataStream &operator>>(QDataStream &s, MyMoneyBank &bank)
{
  return s >> bank.m_name
    >> bank.m_city
    >> bank.m_street
    >> bank.m_postcode
    >> bank.m_telephone
    >> bank.m_manager
    >> bank.m_sortCode;
}
