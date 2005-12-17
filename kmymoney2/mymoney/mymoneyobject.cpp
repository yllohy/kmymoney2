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
#include "mymoneyutils.h"
#include "mymoneyexception.h"

MyMoneyObject::MyMoneyObject(const QCString& id)
{
  m_id = id;
}

MyMoneyObject::MyMoneyObject(const QDomElement& el, const bool forceId)
{
  m_id = QCStringEmpty(el.attribute("id"));
  if(m_id.size() == 0 && forceId)
    throw new MYMONEYEXCEPTION("Node has no ID");
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
