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

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyequity.h"

MyMoneyEquity::MyMoneyEquity()
{
  m_equityType = ETYPE_NONE;
}

MyMoneyEquity::MyMoneyEquity(const QCString& id, const MyMoneyEquity& equity)
{
  *this = equity;
  m_id = id;
}

MyMoneyEquity::~MyMoneyEquity()
{
}

void MyMoneyEquity::setPriceHistory(const equity_price_history& history)
{
  m_priceHistory = history;
}

void MyMoneyEquity::editPriceHistory(const QDate& date, const MyMoneyMoney& money)
{
  m_priceHistory[date] = money;
}

void MyMoneyEquity::addPriceHistory(const QDate& date, const MyMoneyMoney& money)
{
  m_priceHistory[date] = money;
}

void MyMoneyEquity::removePriceHistory(const QDate& date)
{
  equity_price_history::Iterator i = m_priceHistory.find(date);
  if(i != m_priceHistory.end())
  {
    m_priceHistory.erase(i);
  }
}

const bool MyMoneyEquity::hasPrice(const QDate& date) const
{
  bool result = false;
  QMap<QDate, MyMoneyMoney>::ConstIterator it;

  it = m_priceHistory.begin();
  if(it != m_priceHistory.end()) {
    if(it.key() <= date)
      result = true;
  }
  return result;
}

const MyMoneyMoney MyMoneyEquity::price(const QDate& date) const
{
  MyMoneyMoney price(1,1);

  QMap<QDate, MyMoneyMoney>::ConstIterator it;
  for(it = m_priceHistory.begin(); it != m_priceHistory.end(); ++it) {
    if(it.key() <= date)
      price = *it;
    else if(it.key() > date)
      break;
  }
  return price;
}

void MyMoneyEquity::appendNewPriceData(const MyMoneyEquity& equity)
{

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
