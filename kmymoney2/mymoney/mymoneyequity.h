/***************************************************************************
                          mymoneyequity.h  -  description
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

#ifndef MYMONEYEQUITY_H
#define MYMONEYEQUITY_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qdatetime.h>
#include <qmap.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "../mymoney/mymoneymoney.h"
#include "../mymoney/mymoneyutils.h"

typedef QMap<QDate,MyMoneyMoney> equity_price_history;

/**
  * Class that holds all the required information about an equity that the user
  * has entered information about.
  *
  * @author Kevin Tambascio
  */
class MyMoneyEquity
{
public:
  MyMoneyEquity();
  MyMoneyEquity(const QCString& id, const MyMoneyEquity& equity);
  virtual ~MyMoneyEquity();

public:
  typedef enum {
    ETYPE_STOCK,
    ETYPE_MUTUALFUND,
    ETYPE_BOND,
    ETYPE_CURRENCY,
    ETYPE_NONE
  } eEQUITYTYPE;

  QCString  id() const  { return m_id; }

  QString   name() const                 { return m_name; }
  void      setName(const String& str)      { m_name = str; }

  QString   tradingSymbol() const               { return m_symbol; }
  void      setTradingSymbol(const String& str)    { m_symbol = str; }

  eEQUITYTYPE   equityType() const                 { return m_equityType; }
  void          setEquityType(const eEQUITYTYPE& e)   { m_equityType = e; }

  QString    tradingMarket() const  { return m_tradingMarket; }
  void       setTradingMarket(const QString& str) { m_tradingMarket = str; }

  const int smallestAccountFraction(void) const { return m_smallestAccountFraction; };
  void setSmallestAccountFraction(const int sf) { m_smallestAccountFraction = sf; };

  /** No descriptions */
  //virtual void setEquityType(const String& str);

  virtual const equity_price_history& priceHistory() const { return m_priceHistory; }
  virtual void setPriceHistory(const equity_price_history& history);

  virtual void  editPriceHistory(const QDate& date,const MyMoneyMoney& money);
  virtual void  addPriceHistory(const QDate& date, const MyMoneyMoney& money);
  virtual void  removePriceHistory(const QDate& date);

  /**
    * This method takes an equity object that has some price data, and inserts it
    * into 'this' object.
    * @param equity The object to extract price data from.
    *
    * @return void
    *
    */
  virtual void  appendNewPriceData(const MyMoneyEquity& equity);

  /**
    * This method is used to retrieve a price for a specific date
    * from the history. If there is no price for this date, the last
    * known price is used. If no price information
    * is available, 1.0 will be returned as price.
    *
    * If the price history contains the following entries
    *
    * -# 01.01.2004   12.50
    * -# 05.01.2004   12.80
    *
    * then the following return values are given
    *
    * @code
    *
    *   price(02.01.2004) == 12.50
    *   price(31.12.2003) ==  1.00
    *   price(05.01.2004) == 12.80
    *
    * @endcode
    *
    * @param date the date for which the price should be returned (default = today)
    *
    * @return price found as MyMoneyMoney object
    */
  const MyMoneyMoney price(const QDate& date = QDate::currentDate()) const;

  /**
    * This method returns, if a price information for this currency
    * is available for a specific date. If the parameter @p exact is
    * @p false (default), then an older price information is also treated
    * for the date.
    *
    * If the price history contains the following entries
    *
    * -# 01.01.2004   12.50
    * -# 05.01.2004   12.80
    *
    * then the following return values are given
    *
    * @code
    *
    *   hasPrice(02.01.2004, false) == true
    *   hasPrice(31.12.2003, false) == false
    *   hasPrice(02.01.2004, true)  == false
    *   hasPrice(05.01.2004, true)  == true
    *
    * @endcode
    *
    * @param date The date for which a price info is needed (default today)
    * @param exact If @p true, date must match existing price info, otherwise
    *              a previous price information would also match.
    *
    * @retval false no price info available
    * @retval true price info is available
    */
  const bool hasPrice(const QDate& date = QDate::currentDate(), const bool exact = false) const;

protected:
  QCString              m_id;
  QString               m_name;
  QString               m_symbol;
  QString               m_tradingMarket;
  eEQUITYTYPE           m_equityType;
  equity_price_history  m_priceHistory;
  int                   m_smallestAccountFraction;

};

#endif
