/***************************************************************************
                          mymoneyinstitution.cpp
                          -------------------
    copyright            : (C) 2001 by Michael Edwardes,
                               2002-2005 by Thomas Baumgart
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
#include <kmymoney/mymoneyexception.h>

MyMoneyInstitution::MyMoneyInstitution()
{
}

MyMoneyInstitution::MyMoneyInstitution(const QCString id, const MyMoneyInstitution& right) :
  MyMoneyObject(id)
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
  clearId();
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
  if (MyMoneyObject::operator==(right) &&
      ((m_name.length() == 0 && right.m_name.length() == 0) || (m_name == right.m_name)) &&
      ((m_town.length() == 0 && right.m_town.length() == 0) || (m_town == right.m_town)) &&
      ((m_street.length() == 0 && right.m_street.length() == 0) || (m_street == right.m_street)) &&
      ((m_postcode.length() == 0 && right.m_postcode.length() == 0) || (m_postcode == right.m_postcode)) &&
      ((m_telephone.length() == 0 && right.m_telephone.length() == 0) || (m_telephone == right.m_telephone)) &&
      ((m_sortcode.length() == 0 && right.m_sortcode.length() == 0) || (m_sortcode == right.m_sortcode)) &&
      ((m_manager.length() == 0 && right.m_manager.length() == 0) || (m_manager == right.m_manager)) &&
      (m_accountList == right.m_accountList) ) {
    return true;
  } else
    return false;
}

void MyMoneyInstitution::writeXML(QDomDocument& document, QDomElement& parent) const
{
  QDomElement el = document.createElement("INSTITUTION");

  el.setAttribute(QString("id"), m_id);
  el.setAttribute(QString("name"), m_name);
  el.setAttribute(QString("manager"), m_manager);
  el.setAttribute(QString("sortcode"), m_sortcode);

  QDomElement address = document.createElement("ADDRESS");
  address.setAttribute(QString("street"), m_street);
  address.setAttribute(QString("city"), m_town);
  address.setAttribute(QString("zip"), m_postcode);
  address.setAttribute(QString("telephone"), m_telephone);
  el.appendChild(address);


  QDomElement accounts = document.createElement("ACCOUNTIDS");
  for(QCStringList::ConstIterator it = accountList().begin(); it != accountList().end(); ++it){
    QDomElement temp = document.createElement("ACCOUNTID");
    temp.setAttribute(QString("id"), (*it));
    accounts.appendChild(temp);
  }
  el.appendChild(accounts);

  if(m_ofxConnectionSettings.pairs().count()) {
    QDomElement ofxsettings = document.createElement("OFXSETTINGS");
    QMap<QCString,QString>::const_iterator it_key = m_ofxConnectionSettings.pairs().begin();
    while ( it_key != m_ofxConnectionSettings.pairs().end() ) {
      ofxsettings.setAttribute(it_key.key(), it_key.data());
      ++it_key;
    }
    el.appendChild(ofxsettings);
  }
  parent.appendChild(el);
}

void MyMoneyInstitution::readXML(const QDomElement& node)
{
  if(QString("INSTITUTION") != node.tagName())
    throw new MYMONEYEXCEPTION("Node was not INSTITUTION");

  m_id = QCStringEmpty(node.attribute(QString("id")));
  Q_ASSERT(m_id.size());

  m_sortcode = node.attribute(QString("sortcode"));
  m_name = node.attribute("name");
  m_manager = node.attribute("manager");

  QDomNodeList nodeList = node.elementsByTagName(QString("ADDRESS"));
  if(nodeList.count() == 0) {
    QString msg = QString("No ADDRESS in institution %1").arg(m_name);
    throw new MYMONEYEXCEPTION(msg);
  }

  QDomElement addrNode = nodeList.item(0).toElement();
  m_street = addrNode.attribute("street");
  m_town = addrNode.attribute("city");
  m_postcode = addrNode.attribute("zip");
  m_telephone = addrNode.attribute("telephone");

  m_accountList.clear();

  nodeList = node.elementsByTagName(QString("ACCOUNTIDS"));
  if(nodeList.count() > 0) {
    nodeList = nodeList.item(0).toElement().elementsByTagName(QString("ACCOUNTID"));
    for(int i = 0; i < nodeList.count(); ++i) {
      m_accountList << QCString(nodeList.item(i).toElement().attribute(QString("id")));
    }
  }

  m_ofxConnectionSettings = MyMoneyKeyValueContainer();

  nodeList = node.elementsByTagName(QString("OFXSETTINGS"));
  if(nodeList.count() > 0) {
    QDomNamedNodeMap attributes = nodeList.item(0).toElement().attributes();
    for(int i = 0; i < attributes.count(); ++i) {
      const QDomAttr& it_attr = attributes.item(i).toAttr();
      m_ofxConnectionSettings.setValue(it_attr.name().utf8(), it_attr.value());
    }
  }
}

