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


/**
	* Class that holds all the required information about an equity that the user
	* has entered information about.
	*
  *@author Kevin Tambascio
  */

#include "mymoneymoney.h"
#include "mymoneyutils.h"

#include <qdatetime.h>
#include <qmap.h>

typedef QMap<QDate,MyMoneyMoney> equity_price_history;

class MyMoneyEquity
{
public: 
  MyMoneyEquity();
  MyMoneyEquity(const QCString& id, const MyMoneyEquity& equity);
  ~MyMoneyEquity();

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

  
  /** No descriptions */
  void setEquityType(const String& str);

  const equity_price_history& priceHistory() const { return m_priceHistory; }
  void setPriceHistory(const equity_price_history& history);

  void  editPriceHistory(QDate& date,MyMoneyMoney& money);
  void  addPriceHistory(QDate& date, MyMoneyMoney& money);
  void  removePriceHistory(QDate& date);

  /**
    * This method is used to retrieve a price for a specific date
    * from the history. If there is no price for this date, the last
    * known price is used. If no price information
    * is available, 1.0 will be returned as price.
    *
    * @param date the date for which the price should be returned (default = today)
    *
    * @return price found as MyMoneyMoney object
    */
  const MyMoneyMoney price(const QDate& date = QDate::currentDate()) const;

  /**
    * This method returns, if a price information for this currency
    * is available for a specific date.
    *
    * @param date The date for which a price info is needed (default today)
    *
    * @retval false no price info available
    * @retval true price info is available
    */
  const bool hasPrice(const QDate& date = QDate::currentDate()) const;

protected:
  QCString m_id;
  QString m_name;
	QString m_symbol;
	eEQUITYTYPE m_equityType;
  equity_price_history m_priceHistory;
};

#endif
