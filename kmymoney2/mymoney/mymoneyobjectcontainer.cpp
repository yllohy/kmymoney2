/***************************************************************************
                          mymoneyobjectcontainer.cpp
                          -------------------
    copyright            : (C) 2006 by Thomas Baumagrt
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

#include <kmymoney/mymoneyobjectcontainer.h>
#include <kmymoney/mymoneyfile.h>

#define CATEGORY_SEPERATOR ":"

MyMoneyObjectContainer::MyMoneyObjectContainer()
{
}

MyMoneyObjectContainer::~MyMoneyObjectContainer()
{
  clear();
}

void MyMoneyObjectContainer::clear(void)
{
  // delete all objects
  QMap<QCString, MyMoneyObject const *>::const_iterator it;
  for(it = m_map.begin(); it != m_map.end(); ++it) {
    delete (*it);
  }

  // then delete the pointers to them
  m_map.clear();
}


#define objectAccessMethod(a, T) \
const T& MyMoneyObjectContainer::a(const QCString& id) \
{ \
  static T nullElement; \
  QMap<QCString, MyMoneyObject const *>::const_iterator it; \
  it = m_map.find(id); \
  if(it == m_map.end()) { \
    /* not found, need to load from engine */ \
    try { \
      const T& x = MyMoneyFile::instance()->a(id); \
      m_map[id] = new T(x); \
      return dynamic_cast<const T&>(*m_map[id]); \
    } catch(MyMoneyException * e) { \
      delete e; \
      qDebug(#T " with id %s not found", id.data()); \
      return nullElement; \
    } \
  } \
  return dynamic_cast<const T&>(*(*it)); \
}

objectAccessMethod(account, MyMoneyAccount)
objectAccessMethod(transaction, MyMoneyTransaction)
objectAccessMethod(payee, MyMoneyPayee)
objectAccessMethod(security, MyMoneySecurity)

const QString MyMoneyObjectContainer::accountToCategory(const QCString& accountId)
{
  MyMoneyAccount acc;
  QString rc;

  acc = account(accountId);
  do {
    if(!rc.isEmpty())
      rc = QString(CATEGORY_SEPERATOR) + rc;
    rc = acc.name() + rc;
    acc = account(acc.parentAccountId());
  } while(!MyMoneyFile::instance()->isStandardAccount(acc.id()));

  return rc;
}

#include "mymoneyobjectcontainer.moc"
