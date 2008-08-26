/***************************************************************************
                          mymoneyobjectcontainer.h
                             -------------------
    copyright            : (C) 2006 by Thomas Baumgart
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

#ifndef MYMONEYOBJECTCONTAINER_H
#define MYMONEYOBJECTCONTAINER_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// ----------------------------------------------------------------------------
// QT Includes

#include <qobject.h>
#include <qcstring.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/export.h>
#include <kmymoney/mymoneytransaction.h>
#include <kmymoney/mymoneyaccount.h>
#include <kmymoney/mymoneyinstitution.h>
#include <kmymoney/mymoneypayee.h>
#include <kmymoney/mymoneyobject.h>
#include <kmymoney/mymoneysecurity.h>
#include <kmymoney/imymoneystorage.h>

/**
  * @author Thomas Baumgart
  */

/**
  * This class represents a generic container for all MyMoneyObject derived objects.
  */
class KMYMONEY_EXPORT MyMoneyObjectContainer : public QObject
{
  Q_OBJECT
public:
  MyMoneyObjectContainer();
  ~MyMoneyObjectContainer();

  const MyMoneyAccount& account(const QCString& id);
  const MyMoneyPayee& payee(const QCString& id);
  const MyMoneySecurity& security(const QCString& id);
  const MyMoneyInstitution& institution(const QCString& id);
  const MyMoneySchedule& schedule(const QCString& id);

  void account(QValueList<MyMoneyAccount>& list);
  void payee(QValueList<MyMoneyPayee>& list);
  void institution(QValueList<MyMoneyInstitution>& list);

  void preloadAccount(const QValueList<MyMoneyAccount>& list);
  void preloadPayee(const QValueList<MyMoneyPayee>& list);
  void preloadInstitution(const QValueList<MyMoneyInstitution>& list);
  void preloadSecurity(const QValueList<MyMoneySecurity>& list);
  void preloadSchedule(const QValueList<MyMoneySchedule>& list);

  void preloadAccount(const MyMoneyAccount& account);
  void preloadSecurity(const MyMoneySecurity& security);
  void preloadPayee(const MyMoneyPayee& payee);
  void preloadInstitution(const MyMoneyInstitution& institution);

  void clear(const QCString& id);
  void clear(IMyMoneyStorage* storage = 0);

  const MyMoneyAccount& accountByName(const QString& name) const;

  /**
   * This method refreshes an already existing object in the container
   * with a copy from the engine. The object is identified by its @a id.
   * If the object is unknown or the @a id is empty, nothing is done.
   */
  void refresh(const QCString& id);

private:
  void assignFraction(MyMoneyAccount* acc);

private:
  QMap<QCString, MyMoneyObject const *>  m_map;
  IMyMoneyStorage*                       m_storage;
};

#endif


