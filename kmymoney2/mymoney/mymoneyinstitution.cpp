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
}

// MyMoneyInstitution::MyMoneyInstitution(MyMoneyFile *file, const QString id, const MyMoneyInstitution& right)
MyMoneyInstitution::MyMoneyInstitution(const QCString id, const MyMoneyInstitution& right)
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
  m_id = QCString();
  m_name = name;
  m_town = town;
  m_street = street;
  m_postcode = postcode;
  m_telephone = telephone;
  m_manager = manager;
  m_sortcode = sortcode;
}

MyMoneyInstitution::~MyMoneyInstitution()
{
}

void MyMoneyInstitution::addAccountId(const QCString& account)
{
  // only add this account if it is not yet presently in the list
  if(m_accountList.contains(account) == 0)
    m_accountList.append(account);
}

QCString MyMoneyInstitution::removeAccountId(const QCString& account)
{
  QCStringList::Iterator pos;
  QCString rc;

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

