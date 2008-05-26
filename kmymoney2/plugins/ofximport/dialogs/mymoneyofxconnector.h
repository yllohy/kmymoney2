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
#include "config.h"
#endif


#ifdef USE_OFX_DIRECTCONNECT

// ----------------------------------------------------------------------------
// Library Includes

#include <libofx/libofx.h>

// if OFX has a major version number defined, we'll take it
// if not, we assume 0.8.3. 0.8.3 was the last version w/o version number info
#ifdef LIBOFX_MAJOR_VERSION
  #define LIBOFX_VERSION KDE_MAKE_VERSION(LIBOFX_MAJOR_VERSION, LIBOFX_MINOR_VERSION, LIBOFX_MICRO_VERSION)
#else
  #define LIBOFX_VERSION KDE_MAKE_VERSION(0,8,3)
#endif
#define LIBOFX_IS_VERSION(a,b,c) (LIBOFX_VERSION >= KDE_MAKE_VERSION(a,b,c))

// ----------------------------------------------------------------------------
// QT Includes

class QDate;

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/mymoneykeyvaluecontainer.h>

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
  const QByteArray accountInfoRequest(void) const;

  const QByteArray statementResponse(const QDate& _dtstart) const;

protected:
  QString iban(void) const;
  QString fiorg(void) const;
  QString fiid(void) const;
  QString username(void) const;
  QString password(void) const;
  QString accountnum(void) const;
#if LIBOFX_IS_VERSION(0,9,0)
  OfxAccountData::AccountType accounttype(void) const;
#else
  AccountType accounttype(void) const;
#endif

private:
//   QString m_body;
  const MyMoneyAccount& m_account;
  MyMoneyKeyValueContainer m_fiSettings;
};

#endif // USE_OFX_DIRECTCONNECT
#endif // OFXCONNECTOR_H
