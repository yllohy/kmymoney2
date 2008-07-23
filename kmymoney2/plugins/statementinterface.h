/***************************************************************************
                          statementinterface.h
                             -------------------
    begin                : Wed Jan 5 2005
    copyright            : (C) 2005 Thomas Baumgart
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

#ifndef STATEMENTINTERFACE_H
#define STATEMENTINTERFACE_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// ----------------------------------------------------------------------------
// QT Includes

#include <qobject.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/mymoneystatement.h>
#include <kmymoney/mymoneyaccount.h>
#include <kmymoney/export.h>

namespace KMyMoneyPlugin {

/**
  * This abstract class represents the interface to import statements
  * into the KMyMoney application
  */
class KMYMONEY_EXPORT StatementInterface : public QObject {
  Q_OBJECT

public:
  StatementInterface(QObject* parent, const char* name = 0);
  ~StatementInterface() {}

  /**
    * This method imports a MyMoneyStatement into the engine
    */
  virtual bool import(const MyMoneyStatement& s) = 0;

  /**
   * This method returns the account for a given @a key - @a value pair.
   * If the account is not found in the list of accounts, MyMoneyAccount()
   * is returned.
   */
  virtual const MyMoneyAccount& account(const QString& key, const QString& value) const = 0;

  /**
   */
  virtual void setAccountOnlineParameters(const MyMoneyAccount& acc, const MyMoneyKeyValueContainer& kvps) const = 0;

};

}; // namespace
#endif
