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
#include "mymoney_config.h"

#include "mymoneyfile.h"

MyMoneyBank::MyMoneyBank()
{
//  m_accounts.setAutoDelete(true);
	m_parent=0;
}

MyMoneyBank::MyMoneyBank(MyMoneyFile *parent, const QString& name, const QString& sortCode, const QString& city, const QString& street,
  const QString& postcode, const QString& telephone, const QString& manager)
{
//  m_accounts.setAutoDelete(true);
	m_parent = parent;
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
    const QString& description, const QDate openingDate, const MyMoneyMoney openingBal, const QDate& lastReconcile)
{
  MyMoneyAccount *account = new MyMoneyAccount(this, name, number, type, description, openingDate, openingBal, lastReconcile);
  m_accounts.append(account);
	if (m_parent)
		m_parent->setDirty(true);
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

  if (findAccountPosition(account, pos)) {
		if (m_parent)
			m_parent->setDirty(true);
    return m_accounts.remove(pos);
	}
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
	m_parent = right.m_parent;
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
	m_parent = right.m_parent;
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

bool MyMoneyBank::readAllData(int version, QDataStream& stream)
{
  // Ignore version for now because we don't need it.
  stream >> m_name
    >> m_city
    >> m_street
    >> m_postcode
    >> m_telephone
    >> m_manager
    >> m_sortCode;
  return true;
}

void MyMoneyBank::setName(const QString& name) { m_name = name; if (m_parent) m_parent->setDirty(true); }
void MyMoneyBank::setCity(const QString& val) { m_city = val; if (m_parent) m_parent->setDirty(true); }
void MyMoneyBank::setStreet(const QString& val) { m_street = val; if (m_parent) m_parent->setDirty(true); }
void MyMoneyBank::setPostcode(const QString& val) { m_postcode = val; if (m_parent) m_parent->setDirty(true); }
void MyMoneyBank::setTelephone(const QString& val) { m_telephone = val; if (m_parent) m_parent->setDirty(true); }
void MyMoneyBank::setManager(const QString& val) { m_manager = val; if (m_parent) m_parent->setDirty(true); }
void MyMoneyBank::setSortCode(const QString& val) { m_sortCode = val; if (m_parent) m_parent->setDirty(true); }
