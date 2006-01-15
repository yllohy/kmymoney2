/***************************************************************************
                         mymoneyofxconnector.cpp
                             -------------------
    begin                : Sat Nov 13 2004
    copyright            : (C) 2002 by Ace Jones
    email                : acejones@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MYMONEYOFXCONNECTOR_H
#define MYMONEYOFXCONNECTOR_H

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#ifdef USE_OFX_DIRECTCONNECT

// ----------------------------------------------------------------------------
// Library Includes

#include <libofx/libofx.h>

// ----------------------------------------------------------------------------
// QT Includes

class QDate;

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "../mymoney/mymoneykeyvaluecontainer.h"

class MyMoneyAccount;
class MyMoneyInstitution;
class MyMoneyTransaction;

/**
@author ace jones
*/
class MyMoneyOfxConnector
{
public:
  MyMoneyOfxConnector(const MyMoneyAccount& _account);
  QString url(void) const;
  const QByteArray statementRequest(const QDate& _dtstart) const;
  const QByteArray MyMoneyOfxConnector::accountInfoRequest(void) const;
  
  const QByteArray statementResponse(const QDate& _dtstart) const;
  
protected:
  QString iban(void) const;
  QString fiorg(void) const;
  QString fiid(void) const;
  QString username(void) const;
  QString password(void) const;
  QString accountnum(void) const;
  AccountType accounttype(void) const;

private:
//   QString m_body;
  const MyMoneyAccount& m_account;
  MyMoneyKeyValueContainer m_fiSettings;
};

#endif // USE_OFX_DIRECTCONNECT
#endif // OFXCONNECTOR_H
