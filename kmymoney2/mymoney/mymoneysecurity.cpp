/***************************************************************************
                          mymoneysecurity.cpp  -  description
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

#include "mymoneysecurity.h"
#include "mymoneyexception.h"

MyMoneySecurity::MyMoneySecurity() :
  m_securityType(SECURITY_NONE),
  m_smallestAccountFraction(100),
  m_smallestCashFraction(100),
  m_partsPerUnit(100)
{
}

MyMoneySecurity::MyMoneySecurity(const QCString& id, const QString& name, const QString& symbol, const int partsPerUnit, const int smallestCashFraction, const int smallestAccountFraction) :
  MyMoneyObject(id),
  m_name(name),
  m_securityType(SECURITY_CURRENCY)
{
  if(symbol.isEmpty())
    m_tradingSymbol = id;
  else
    m_tradingSymbol = symbol;

  m_partsPerUnit = partsPerUnit;
  m_smallestCashFraction = smallestCashFraction;
  if(smallestAccountFraction)
    m_smallestAccountFraction = smallestAccountFraction;
  else
    m_smallestAccountFraction = smallestCashFraction;
}

MyMoneySecurity::MyMoneySecurity(const QCString& id, const MyMoneySecurity& equity)
{
  *this = equity;
  m_id = id;
}

MyMoneySecurity::~MyMoneySecurity()
{
}

const bool MyMoneySecurity::operator == (const MyMoneySecurity& r) const
{
  return (m_id == r.m_id)
      && (m_name == r.m_name)
      && (m_tradingSymbol == r.m_tradingSymbol)
      && (m_tradingMarket == r.m_tradingMarket)
      && (m_tradingSymbol == r.m_tradingSymbol)
      && (m_tradingCurrency == r.m_tradingCurrency)
      && (m_securityType == r.m_securityType)
      && (m_smallestAccountFraction == r.m_smallestAccountFraction)
      && (m_smallestCashFraction == r.m_smallestCashFraction)
      && (m_partsPerUnit == r.m_partsPerUnit)
      && this->MyMoneyKeyValueContainer::operator == (r);

}

bool MyMoneySecurity::hasReferenceTo(const QCString& id) const
{
  return (id == m_tradingCurrency);
}

void MyMoneySecurity::writeXML(QDomDocument& document, QDomElement& parent) const
{
  QDomElement el;
  if(isCurrency())
    el = document.createElement("CURRENCY");
  else
    el = document.createElement("SECURITY");

  el.setAttribute("name", m_name);
  el.setAttribute("symbol", m_tradingSymbol);
  el.setAttribute("type", static_cast<int>(m_securityType));
  el.setAttribute("id", id());
  el.setAttribute("saf", m_smallestAccountFraction);
  if(isCurrency()) {
    el.setAttribute("ppu", m_partsPerUnit);
    el.setAttribute("scf", m_smallestCashFraction);
  } else {
    el.setAttribute("trading-currency", m_tradingCurrency);
    el.setAttribute("trading-market", m_tradingMarket);
  }

  //Add in Key-Value Pairs for securities.
  MyMoneyKeyValueContainer::writeXML(document, el);

  parent.appendChild(el);
}

void MyMoneySecurity::readXML(const QDomElement& node)
{
  if(("SECURITY" != node.tagName())
  && ("EQUITY" != node.tagName())
  && ("CURRENCY" != node.tagName()))
    throw new MYMONEYEXCEPTION("Node was not SECURITY or CURRENCY");

  m_id = QCStringEmpty(node.attribute("id"));
  Q_ASSERT(m_id.size());
  setName(QStringEmpty(node.attribute("name")));
  setTradingSymbol(QStringEmpty(node.attribute("symbol")));
  setSecurityType(static_cast<eSECURITYTYPE>(node.attribute("type").toInt()));
  setSmallestAccountFraction(node.attribute("saf").toInt());

  if(isCurrency()) {
    setPartsPerUnit(node.attribute("ppu").toInt());
    setSmallestCashFraction(node.attribute("scf").toInt());
  } else {
    setTradingCurrency(QCStringEmpty(node.attribute("trading-currency")));
    setTradingMarket(QStringEmpty(node.attribute("trading-market")));
  }

  // Process any key value pair
  QDomNodeList nodeList = node.elementsByTagName("KEYVALUEPAIRS");
  if(nodeList.count() > 0) {
    MyMoneyKeyValueContainer::readXML(nodeList.item(0).toElement());
  }
}

#if 0
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

const bool MyMoneyEquity::hasPrice(const QDate& date, const bool exact) const
{
  bool result = false;
  QMap<QDate, MyMoneyMoney>::ConstIterator it;

  if(exact) {
    result = (m_priceHistory.find(date) != m_priceHistory.end()) ? true : false;
  } else {
    it = m_priceHistory.begin();
    if(it != m_priceHistory.end()) {
      if(it.key() <= date)
        result = true;
    }
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
