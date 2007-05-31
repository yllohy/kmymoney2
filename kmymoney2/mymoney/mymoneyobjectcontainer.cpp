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

MyMoneyObjectContainer::MyMoneyObjectContainer()
{
}

MyMoneyObjectContainer::~MyMoneyObjectContainer()
{
  clear();
}

void MyMoneyObjectContainer::clear(IMyMoneyStorage* storage)
{
  // delete all objects
  QMap<QCString, MyMoneyObject const *>::const_iterator it;
  for(it = m_map.begin(); it != m_map.end(); ++it) {
    delete (*it);
  }

  // then delete the pointers to them
  m_map.clear();

  if(storage)
    m_storage = storage;
}

void MyMoneyObjectContainer::clear(const QCString& id)
{
  QMap<QCString, MyMoneyObject const *>::iterator it;
  it = m_map.find(id);
  if(it != m_map.end()) {
    delete (*it);
    m_map.erase(it);
  }
}

#define listMethod(a, T) \
void MyMoneyObjectContainer::a(QValueList<T>& list) \
{ \
  QMap<QCString, const MyMoneyObject*>::const_iterator it; \
  for(it = m_map.begin(); it != m_map.end(); ++it) { \
    const T* node = dynamic_cast<const T*>(*it); \
    if(node) { \
      list.append(*node); \
    } \
  } \
}

#define preloadMethod(a, T) \
void MyMoneyObjectContainer::preload##a(const QValueList<T>& list) \
{ \
  QValueList<T>::const_iterator it; \
  for(it = list.begin(); it != list.end(); ++it) { \
    delete m_map[(*it).id()]; \
    m_map[(*it).id()] = new T(*it); \
  } \
}

#if 0
#define objectAccessMethod(a, T) \
const T& MyMoneyObjectContainer::a(const QCString& id) \
{ \
  static T nullElement; \
  QMap<QCString, MyMoneyObject const *>::const_iterator it; \
  it = m_map.find(id); \
  if(it == m_map.end()) { \
    /* not found, need to load from engine */ \
    try { \
      const T& x = m_storage->a(id); \
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
#endif

#define objectAccessMethod(a, T) \
const T& MyMoneyObjectContainer::a(const QCString& id) \
{ \
  static T nullElement; \
  if(id.isEmpty()) \
    return nullElement; \
  QMap<QCString, MyMoneyObject const *>::const_iterator it; \
  it = m_map.find(id); \
  if(it == m_map.end()) { \
    /* not found, need to load from engine */ \
    const T& x = m_storage->a(id); \
    m_map[id] = new T(x); \
    return dynamic_cast<const T&>(*m_map[id]); \
  } \
  return dynamic_cast<const T&>(*(*it)); \
}

objectAccessMethod(account, MyMoneyAccount)
objectAccessMethod(payee, MyMoneyPayee)
objectAccessMethod(security, MyMoneySecurity)
objectAccessMethod(institution, MyMoneyInstitution)

preloadMethod(Account, MyMoneyAccount)
preloadMethod(Payee, MyMoneyPayee)
preloadMethod(Institution, MyMoneyInstitution)
preloadMethod(Security, MyMoneySecurity)

listMethod(account, MyMoneyAccount);
listMethod(payee, MyMoneyPayee);
listMethod(institution, MyMoneyInstitution);

#include "mymoneyobjectcontainer.moc"
