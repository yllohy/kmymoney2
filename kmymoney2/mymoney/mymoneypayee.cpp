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
#include "mymoneyutils.h"
#include <kmymoney/mymoneyexception.h>

MyMoneyPayee MyMoneyPayee::null = MyMoneyPayee();

MyMoneyPayee::MyMoneyPayee()
{
}

MyMoneyPayee::MyMoneyPayee(const QCString& id, const MyMoneyPayee& payee)
{
  *this = payee;
  m_id = id;
}

MyMoneyPayee::MyMoneyPayee(const QString& name, const QString& address,
        const QString& city, const QString& state, const QString& postcode,
        const QString& telephone, const QString& email)
{
  m_name      = name;
  m_address   = address;
  m_city      = city;
  m_state     = state;
  m_postcode  = postcode;
  m_telephone = telephone;
  m_email     = email;
}

MyMoneyPayee::MyMoneyPayee(const QDomElement& node) :
  MyMoneyObject(node)
{
  if("PAYEE" != node.tagName())
    throw new MYMONEYEXCEPTION("Node was not PAYEE");

  m_name = node.attribute("name");
  m_reference = node.attribute("reference");
  m_email = node.attribute("email");

  QDomNodeList nodeList = node.elementsByTagName("ADDRESS");
  if(nodeList.count() == 0) {
    QString msg = QString("No ADDRESS in payee %1").arg(m_name);
    throw new MYMONEYEXCEPTION(msg);
  }

  QDomElement addrNode = nodeList.item(0).toElement();
  m_address = addrNode.attribute("street");
  m_city = addrNode.attribute("city");
  m_postcode = addrNode.attribute("postcode");
  m_state = addrNode.attribute("state");
  m_telephone = addrNode.attribute("telephone");
}

MyMoneyPayee::~MyMoneyPayee()
{
}

MyMoneyPayee::MyMoneyPayee(const MyMoneyPayee& right) :
  MyMoneyObject(right)
{
  *this = right;
}

#if 0
QDataStream &operator<<(QDataStream &s, const MyMoneyPayee &payee)
{
  return s << payee.m_name
    << payee.m_address
    << payee.m_city
    << payee.m_state
    << payee.m_postcode
    << payee.m_telephone
    << payee.m_email;
}

QDataStream &operator>>(QDataStream &s, MyMoneyPayee &payee)
{
  return s >> payee.m_name
    >> payee.m_address
    >> payee.m_city
    >> payee.m_state
    >> payee.m_postcode
    >> payee.m_telephone
    >> payee.m_email;
}
#endif

const bool MyMoneyPayee::operator == (const MyMoneyPayee& right) const
{
  return (MyMoneyObject::operator==(right) &&
      ((m_name.length() == 0 && right.m_name.length() == 0) || (m_name == right.m_name)) &&
      ((m_address.length() == 0 && right.m_address.length() == 0) || (m_address == right.m_address)) &&
      ((m_city.length() == 0 && right.m_city.length() == 0) || (m_city == right.m_city)) &&
      ((m_state.length() == 0 && right.m_state.length() == 0) || (m_state == right.m_state)) &&
      ((m_postcode.length() == 0 && right.m_postcode.length() == 0) || (m_postcode == right.m_postcode)) &&
      ((m_telephone.length() == 0 && right.m_telephone.length() == 0) || (m_telephone == right.m_telephone)) &&
      ((m_email.length() == 0 && right.m_email.length() == 0) || (m_email == right.m_email)) &&
      ((m_reference.length() == 0 && right.m_reference.length() == 0) || (m_reference == right.m_reference)) );
}

void MyMoneyPayee::writeXML(QDomDocument& document, QDomElement& parent) const
{
  QDomElement el = document.createElement("PAYEE");

  el.setAttribute("name", m_name);
  el.setAttribute("id", m_id);
  el.setAttribute("reference", m_reference);
  el.setAttribute("email", m_email);

  QDomElement address = document.createElement("ADDRESS");
  address.setAttribute("street", m_address);
  address.setAttribute("city", m_city);
  address.setAttribute("postcode", m_postcode);
  address.setAttribute("state", m_state);
  address.setAttribute("telephone", m_telephone);

  el.appendChild(address);

  parent.appendChild(el);
}

bool MyMoneyPayee::hasReferenceTo(const QCString& id) const
{
  // the payee does not reference any other object
  return false;
}
