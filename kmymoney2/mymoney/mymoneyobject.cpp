/***************************************************************************
                          mymoneyobject.cpp
                          -------------------
    copyright            : (C) 2005 by Thomas Baumagrt
    email                : ipwizard@users.sourceforge.net
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

#include "mymoneyobject.h"

MyMoneyObject::MyMoneyObject(const QCString& id)
{
  m_id = id;
}

MyMoneyObject::MyMoneyObject()
{
}

MyMoneyObject::~MyMoneyObject()
{
}

void MyMoneyObject::setId(const QCString& id)
{
  m_id = id;
}

const bool MyMoneyObject::operator == (const MyMoneyObject& right) const
{
  return m_id == right.m_id;
}

void MyMoneyObject::clearId(void)
{
  m_id = QCString();
}

QString MyMoneyObject::dateToString(const QDate& date) const
{
  if(!date.isNull() && date.isValid()) {
    return date.toString(Qt::ISODate);
  }

  return QString();
}

QDate MyMoneyObject::stringToDate(const QString& str) const
{
  if(str.length()) {
    QDate date = QDate::fromString(str, Qt::ISODate);
    if(!date.isNull() && date.isValid()) {
      return date;
    }
  }
  return QDate();
}

QCString MyMoneyObject::QCStringEmpty(const QString& val) const
{
  if(!val.isEmpty())
    return QCString(val);

  return QCString();
}

QString MyMoneyObject::QStringEmpty(const QString& val) const
{
  if(!val.isEmpty()) {
    return QString(val);
  }
  return QString();
}
