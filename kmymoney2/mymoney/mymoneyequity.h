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
  ~MyMoneyEquity();

public:
  typedef enum {
    ETYPE_STOCK,
    ETYPE_MUTUALFUND,
    ETYPE_BOND,
    ETYPE_NONE
  } eEQUITYTYPE;

  QString   getEquityName()                       { return m_name; }
  void      setEquityName(const String& str)      { m_name = str; }

	QString   getEquitySymbol()                     { return m_symbol; }
	void      setEquitySymbol(const String& str)    { m_symbol = str; }

	eEQUITYTYPE   getEquityType()                       { return m_equityType; }
	void          setEquityType(const eEQUITYTYPE& e)   { m_equityType = e; }

	QString   getEquityMarket()                     { return m_market; }
	void      setEquityMarket(const String& str)    { m_market = str; }

	MyMoneyMoney    getCurrentPrice()                       { return m_currentPrice; }
	void            setCurrentPrice(const MyMoneyMoney& m)  { m_currentPrice = m; }
  /** No descriptions */
  void setEquityType(const String& str);

  const equity_price_history& getConstPriceHistory() const { return m_priceHistory; }

  void  editPriceHistory(QDate& date,MyMoneyMoney& money);
  void  addPriceHistory(QDate& date, MyMoneyMoney& money);
  void  removePriceHistory(QDate& date);
	
private:
	QString m_name;
	QString m_symbol;
	QString m_market;
	eEQUITYTYPE m_equityType;
	MyMoneyMoney m_currentPrice;
  equity_price_history m_priceHistory;
};

#endif
