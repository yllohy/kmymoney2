/***************************************************************************
                          mymoneyinstitution.cpp
                          -------------------
    copyright            : (C) 2001 by Michael Edwardes,
                               2002 by Thomas Baumgart
    email                : mte@users.sourceforge.net,
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

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyinstitution.h"

MyMoneyInstitution::MyMoneyInstitution()
{
	m_file = 0;

  m_id =
  m_name =
  m_town =
  m_street =
  m_postcode =
  m_telephone =
  m_manager =
  m_sortcode = "";
}

// MyMoneyInstitution::MyMoneyInstitution(MyMoneyFile *file, const QString id, const MyMoneyInstitution& right)
MyMoneyInstitution::MyMoneyInstitution(const QString id, const MyMoneyInstitution& right)
{
  *this = right;
  m_id = id;
}

MyMoneyInstitution::MyMoneyInstitution(const QString& name,
                         const QString& town,
                         const QString& street,
                         const QString& postcode,
                         const QString& telephone,
                         const QString& manager,
                         const QString& sortcode)
{
	m_file = 0;
  m_id = "";
  m_name = name;
  m_town = town;
  m_street = street;
  m_postcode = postcode;
  m_telephone = telephone;
  m_manager = manager;
  m_sortcode = sortcode;
}

#if 0
MyMoneyInstitution& MyMoneyInstitution::operator = (const MyMoneyInstitution &right)
{
  m_file = right.m_file;
  m_id = right.m_id;
  m_name = right.m_name;
  m_town = right.m_town;
  m_street = right.m_street;
  m_postcode = right.m_postcode;
  m_telephone = right.m_telephone;
  m_manager = right.m_manager;
  m_sortcode = right.m_sortcode;
  m_accountList = right.m_accountList;
  return *this;
}
#endif

MyMoneyInstitution::~MyMoneyInstitution()
{
}

void MyMoneyInstitution::addAccountId(const QString& account)
{
  // only add this account if it is not yet presently in the list
  if(m_accountList.contains(account) == 0)
    m_accountList.append(account);
}

QString MyMoneyInstitution::removeAccountId(const QString& account)
{
  QStringList::Iterator pos;
  QString rc = "";

  pos = m_accountList.find(account);
  if(pos != m_accountList.end()) {
    m_accountList.remove(pos);
    rc = account;
  }
  return rc;
}

bool MyMoneyInstitution::operator == (const MyMoneyInstitution& right) const
{
  if ((m_id == right.m_id) &&
      (m_file == right.m_file) &&
      (m_name == right.m_name) &&
      (m_town == right.m_town) &&
      (m_street == right.m_street) &&
      (m_postcode == right.m_postcode) &&
      (m_telephone == right.m_telephone) &&
      (m_sortcode == right.m_sortcode) &&
      (m_manager == right.m_manager) &&
      (m_accountList == right.m_accountList) ) {
    return true;
  } else
    return false;
}

/*
QDataStream& operator << (QDataStream &s, const MyMoneyInstitution &institution)
{
  if(MyMoneyFile::fileVersion(MyMoneyFile::Writing) >= VERSION_0_5_0) {
    s << Q_INT8(0);
    s << institution.m_id;
  }

  return s << institution.m_name
    << institution.m_town
    << institution.m_street
    << institution.m_postcode
    << institution.m_telephone
    << institution.m_manager
    << institution.m_sortcode;
}

QDataStream& operator >> (QDataStream &s, MyMoneyInstitution &institution)
{
  Q_UINT8  objVersion;

  institution.m_file = 0;
  institution.m_id = "";

  if(MyMoneyFile::fileVersion(MyMoneyFile::Reading) >= VERSION_0_5_0) {
    s >> objVersion;
    s >> institution.m_id;
  }

  return s >> institution.m_name
    >> institution.m_town
    >> institution.m_street
    >> institution.m_postcode
    >> institution.m_telephone
    >> institution.m_manager
    >> institution.m_sortcode;
}
*/
