/***************************************************************************
                          mymoneyequity.cpp  -  description
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

#include "mymoneyutils.h"
#include "mymoneyequity.h"

MyMoneyEquity::MyMoneyEquity()
{
	m_equityType = ETYPE_NONE;
}

MyMoneyEquity::MyMoneyEquity(const QCString& id, const MyMoneyEquity& equity)
{
  m_id = id;
  *this = equity;
}

MyMoneyEquity::~MyMoneyEquity()
{
}

void MyMoneyEquity::editPriceHistory(QDate& date,MyMoneyMoney& money)
{
  m_priceHistory[date] = money;
}

void MyMoneyEquity::addPriceHistory(QDate& date, MyMoneyMoney& money)
{
  m_priceHistory[date] = money;
}

void MyMoneyEquity::removePriceHistory(QDate& date)
{
  equity_price_history::Iterator i = m_priceHistory.find(date);
  if(i != m_priceHistory.end())
  {
    m_priceHistory.erase(i);
  }
}
  

#if 0
/** No descriptions */
void MyMoneyEquity::setEquityType(const String& str)
{
	if(str.size())
	{
		if(!str.find(i18n("Stock")))
		{
			setEquityType(ETYPE_STOCK);
		}
		else if(!str.find(i18n("Mutual Fund")))
		{
			setEquityType(ETYPE_MUTUALFUND);
		}
		else if(!str.find(i18n("Bond")))
		{
			setEquityType(ETYPE_BOND);
		}
		else
		{
			setEquityType(ETYPE_NONE);
		}
	}
}
#endif
