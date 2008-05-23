/***************************************************************************
                          mymoneyprice  -  description
                             -------------------
    begin                : Sun Nov 21 2004
    copyright            : (C) 2000-2004 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/**
  * @author Thomas Baumgart
  */

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyprice.h"
#include "mymoneyexception.h"

MyMoneyPrice::MyMoneyPrice() :
  m_date(QDate())
{
}

MyMoneyPrice::MyMoneyPrice(const QCString& from, const QCString& to, const QDate& date, const MyMoneyMoney& rate, const QString& source) :
  m_fromSecurity(from),
  m_toSecurity(to),
  m_date(date),
  m_rate(rate),
  m_source(source)
{
  if(!m_rate.isZero())
    m_invRate = MyMoneyMoney(1,1) / m_rate;
  else
    qDebug("Price with zero value created");
}

MyMoneyPrice::~MyMoneyPrice()
{
}

const MyMoneyMoney MyMoneyPrice::rate(const QCString& id) const
{
  static MyMoneyMoney dummyPrice(1,1);

  if(!isValid())
    return dummyPrice;

  if(id.isEmpty() || id == m_toSecurity)
    return m_rate;
  if(id == m_fromSecurity)
    return m_invRate;

  QString msg = QString("Unknown security id %1 for price info %2/%3.").arg(id).arg(m_fromSecurity).arg(m_toSecurity);
  throw new MYMONEYEXCEPTION(msg);
}

const bool MyMoneyPrice::isValid(void) const
{
  return (m_date.isValid() && !m_fromSecurity.isEmpty() && !m_toSecurity.isEmpty());
}

// Equality operator
const bool MyMoneyPrice::operator == (const MyMoneyPrice &right) const
{
  return ((m_date == right.m_date) &&
      (m_rate == right.m_rate) &&
      ((m_fromSecurity.length() == 0 && right.m_fromSecurity.length() == 0) || (m_fromSecurity == right.m_fromSecurity)) &&
      ((m_toSecurity.length() == 0 && right.m_toSecurity.length() == 0) || (m_toSecurity == right.m_toSecurity)) &&
      ((m_source.length() == 0 && right.m_source.length() == 0) || (m_source == right.m_source)));
}

bool MyMoneyPrice::hasReferenceTo(const QCString& id) const
{
  return (id == m_fromSecurity) || (id == m_toSecurity);
}
