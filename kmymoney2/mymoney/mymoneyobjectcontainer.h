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
#include <kmymoney/mymoneypayee.h>
#include <kmymoney/mymoneyobject.h>
#include <kmymoney/mymoneysecurity.h>

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

  const MyMoneyTransaction& transaction(const QCString& id);
  const MyMoneyAccount& account(const QCString& id);
  const QString accountToCategory(const QCString& accountId);
  const MyMoneyPayee& payee(const QCString& id);
  const MyMoneySecurity& security(const QCString& id);

public slots:
  void clear(void);

private:
  QMap<QCString, MyMoneyObject const *>  m_map;
};

#endif


