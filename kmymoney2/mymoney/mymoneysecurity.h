/***************************************************************************
                          mymoneysecurity.h  -  description
                             -------------------
    begin                : Tue Jan 29 2002
    copyright            : (C) 2000-2002 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MYMONEYSECURITY_H
#define MYMONEYSECURITY_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// ----------------------------------------------------------------------------
// QT Includes

#include <qdatetime.h>
#include <qmap.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/export.h>
#include <kmymoney/mymoneymoney.h>
#include <kmymoney/mymoneyutils.h>
#include <kmymoney/mymoneykeyvaluecontainer.h>

/**
  * Class that holds all the required information about a security that the user
  * has entered information about. A security can be a stock, a mutual fund, a bond
  * or a currency.
  *
  * @author Kevin Tambascio
  */
class KMYMONEY_EXPORT MyMoneySecurity : public MyMoneyKeyValueContainer
{
public:
  MyMoneySecurity();
  MyMoneySecurity(const QCString& id, const MyMoneySecurity& equity);
  MyMoneySecurity(const QCString& id, const QString& name, const QString& symbol = QString(), const int partsPerUnit = 100, const int smallestCashFraction = 100, const int smallestAccountFraction = 0);
  virtual ~MyMoneySecurity();

  /**
    * This operator tests for equality of two MyMoneySecurity objects
    */
  const bool operator == (const MyMoneySecurity&) const;

  /**
    * This operator tests for inequality of this MyMoneySecurity object
    * and the one passed by @p r
    *
    * @param r the right side of the comparison
    */
  const bool operator != (const MyMoneySecurity& r) const { return !(*this == r); }

public:
  typedef enum {
    SECURITY_STOCK,
    SECURITY_MUTUALFUND,
    SECURITY_BOND,
    SECURITY_CURRENCY,
    SECURITY_NONE
  } eSECURITYTYPE;

  const QCString id() const  { return m_id; }

  const QString  name() const                 { return m_name; }
  void           setName(const String& str)   { m_name = str; }

  const QString   tradingSymbol() const               { return m_tradingSymbol; }
  void            setTradingSymbol(const String& str) { m_tradingSymbol = str; }

  const eSECURITYTYPE securityType() const                 { return m_securityType; }
  void          setSecurityType(const eSECURITYTYPE& s)   { m_securityType = s; }
  const bool    isCurrency(void) const { return m_securityType == SECURITY_CURRENCY; };

  const QString tradingMarket() const  { return m_tradingMarket; }
  void          setTradingMarket(const QString& str) { m_tradingMarket = str; }

  const QCString  tradingCurrency(void) const { return m_tradingCurrency; };
  void            setTradingCurrency(const QCString& str) { m_tradingCurrency = str; };

  const int smallestAccountFraction(void) const { return m_smallestAccountFraction; };
  void setSmallestAccountFraction(const int sf) { m_smallestAccountFraction = sf; };

  const int partsPerUnit(void) const { return m_partsPerUnit; };
  const int smallestCashFraction(void) const { return m_smallestCashFraction; };

  void setPartsPerUnit(const int ppu) { m_partsPerUnit = ppu; };
  void setSmallestCashFraction(const int sf) { m_smallestCashFraction = sf; };

protected:
  QCString              m_id;
  QString               m_name;
  QString               m_tradingSymbol;
  QString               m_tradingMarket;
  QCString              m_tradingCurrency;
  eSECURITYTYPE         m_securityType;
  int                   m_smallestAccountFraction;
  int                   m_smallestCashFraction;
  int                   m_partsPerUnit;
};

#endif
