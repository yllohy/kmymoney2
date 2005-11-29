/***************************************************************************
                          mymoneykeyvaluecontainer.cpp
                             -------------------
    begin                : Sun Nov 10 2002
    copyright            : (C) 2002-2005 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
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

#include <kmymoney/mymoneykeyvaluecontainer.h>
#include <kmymoney/mymoneyexception.h>

MyMoneyKeyValueContainer::MyMoneyKeyValueContainer()
{
}

MyMoneyKeyValueContainer::~MyMoneyKeyValueContainer()
{
}

const QString& MyMoneyKeyValueContainer::value(const QCString& key) const
{
  QMap<QCString, QString>::ConstIterator it;

  it = m_kvp.find(key);
  if(it != m_kvp.end())
    return (*it);
  return QString::null;
}

void MyMoneyKeyValueContainer::setValue(const QCString& key, const QString& value)
{
  m_kvp[key] = value;
}


void MyMoneyKeyValueContainer::setPairs(const QMap<QCString, QString>& list)
{
  m_kvp = list;
}

void MyMoneyKeyValueContainer::deletePair(const QCString& key)
{
  QMap<QCString, QString>::Iterator it;

  it = m_kvp.find(key);
  if(it != m_kvp.end())
    m_kvp.remove(it);
}

void MyMoneyKeyValueContainer::clear(void)
{
  m_kvp.clear();
}

const bool MyMoneyKeyValueContainer::operator == (const MyMoneyKeyValueContainer& right) const
{
  QMap<QCString, QString>::ConstIterator it_a, it_b;

  it_a = m_kvp.begin();
  it_b = right.m_kvp.begin();

  while(it_a != m_kvp.end() && it_b != right.m_kvp.end()) {
    if(it_a.key() != it_b.key()
    || (((*it_a).length() != 0 || (*it_b).length() != 0) && *it_a != *it_b))
      return false;
    ++it_a;
    ++it_b;
  }

  return (it_a == m_kvp.end() && it_b == right.m_kvp.end());
}

void MyMoneyKeyValueContainer::writeXML(QDomDocument& document, QDomElement& parent) const
{
  QDomElement el = document.createElement("KEYVALUEPAIRS");

  QMap<QCString, QString>::ConstIterator it;
  for(it = m_kvp.begin(); it != m_kvp.end(); ++it)
  {
    QDomElement pair = document.createElement("PAIR");
    pair.setAttribute(QString("key"), it.key());
    pair.setAttribute(QString("value"), it.data());
    el.appendChild(pair);
  }

  parent.appendChild(el);
}

void MyMoneyKeyValueContainer::readXML(const QDomElement& node)
{
  if(QString("KEYVALUEPAIRS") != node.tagName())
    throw new MYMONEYEXCEPTION("Node was not KEYVALUEPAIRS");

  m_kvp.clear();

  QDomNodeList nodeList = node.elementsByTagName(QString("PAIR"));
  for(unsigned int i = 0; i < nodeList.count(); ++i) {
    const QDomElement& el(nodeList.item(i).toElement());
    m_kvp[QCString(el.attribute(QString("key")))] = el.attribute(QString("value"));
  }
}
