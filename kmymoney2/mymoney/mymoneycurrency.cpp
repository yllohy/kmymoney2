/***************************************************************************
                          mymoneycurrency.cpp  -  description
                             -------------------
    begin                : Fri Mar 19 2004
    copyright            : (C) 2000-2004 by Michael Edwardes
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


#include "mymoneycurrency.h"

MyMoneyCurrency::MyMoneyCurrency()
{
  setEquityType(ETYPE_CURRENCY);
  m_partsPerUnit = 100;
  m_smallestCashFraction = 100;
  m_smallestAccountFraction = 100;
  // m_symbol = QChar(0x00A4);         // general currency symbol
  m_symbol = QString();
}

MyMoneyCurrency::MyMoneyCurrency(const QCString& id, const QString& name, const QString& symbol, const int partsPerUnit, const int smallestCashFraction, const int smallestAccountFraction)
{
  setEquityType(ETYPE_CURRENCY);
  m_id = id;
  m_name = name;

  if(symbol.isEmpty())
    m_symbol = id;
  else
    m_symbol = symbol;

  m_partsPerUnit = partsPerUnit;
  m_smallestCashFraction = smallestCashFraction;
  if(smallestAccountFraction)
    m_smallestAccountFraction = smallestAccountFraction;
  else
    m_smallestAccountFraction = smallestCashFraction;
}

MyMoneyCurrency::~MyMoneyCurrency()
{
}

MyMoneyCurrency::MyMoneyCurrency(const MyMoneyEquity& r)
{
  MyMoneyEquity* that = static_cast<MyMoneyEquity*> (this);
  *that = r;

  m_partsPerUnit = 100;
  m_smallestCashFraction = 100;
  m_smallestAccountFraction = 100;
}
