/***************************************************************************
                          mymoneycurrency.h  -  description
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

#ifndef MYMONEYCURRENCY_H
#define MYMONEYCURRENCY_H

#ifndef MYMONEYEQUITY_H
#include "mymoneyequity.h"
#endif

/**
  * @author Thomas Baumgart
  */

class MyMoneyCurrency : public MyMoneyEquity
{
public:
  MyMoneyCurrency();
  MyMoneyCurrency(const QCString& id, const QString& name, const QString& symbol = QString(), const int partsPerUnit = 100, const int smallestCashFraction = 100, const int smallestAccountFraction = 0);
  MyMoneyCurrency(const MyMoneyEquity& r);
  ~MyMoneyCurrency();

  const int partsPerUnit(void) const { return m_partsPerUnit; };
  const int smallestCashFraction(void) const { return m_smallestCashFraction; };

  void setPartsPerUnit(const int ppu) { m_partsPerUnit = ppu; };
  void setSmallestCashFraction(const int sf) { m_smallestCashFraction = sf; };

private:
  int     m_partsPerUnit;
  int     m_smallestCashFraction;
};

#endif
