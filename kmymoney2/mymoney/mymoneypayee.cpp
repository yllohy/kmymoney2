/***************************************************************************
                          mymoneypayee.cpp
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

#include "mymoneypayee.h"

MyMoneyPayee::MyMoneyPayee()
{
}

MyMoneyPayee::MyMoneyPayee(const QCString& id, const MyMoneyPayee& payee)
{
  *this = payee;
  m_id = id;
}

MyMoneyPayee::MyMoneyPayee(const QString& name, const QString address, const QString postcode, const QString telephone, const QString email, const QString city, const QString state)
{
  m_name      = name;
  m_address   = address;
  m_postcode  = postcode;
  m_telephone = telephone;
  m_email     = email;
  m_city      = city;
  m_state     = state;
}

MyMoneyPayee::~MyMoneyPayee()
{
}

MyMoneyPayee::MyMoneyPayee(const MyMoneyPayee& right)
{
  *this = right;
}

QDataStream &operator<<(QDataStream &s, const MyMoneyPayee &payee)
{
  return s << payee.m_name
    << payee.m_address
    << payee.m_postcode
    << payee.m_telephone
    << payee.m_email;
}

QDataStream &operator>>(QDataStream &s, MyMoneyPayee &payee)
{
  return s >> payee.m_name
    >> payee.m_address
    >> payee.m_postcode
    >> payee.m_telephone
    >> payee.m_email;
}

const bool MyMoneyPayee::operator == (const MyMoneyPayee& right) const
{
  return ((m_id == right.m_id) &&
      ((m_address.length() == 0 && right.m_address.length() == 0) || (m_address == right.m_address)) &&
      ((m_email.length() == 0 && right.m_email.length() == 0) || (m_email == right.m_email)) &&
      ((m_name.length() == 0 && right.m_name.length() == 0) || (m_name == right.m_name)) &&
      ((m_postcode.length() == 0 && right.m_postcode.length() == 0) || (m_postcode == right.m_postcode)) &&
      ((m_reference.length() == 0 && right.m_reference.length() == 0) || (m_reference == right.m_reference)) );
}
