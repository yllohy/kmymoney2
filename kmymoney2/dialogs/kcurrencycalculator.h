/***************************************************************************
                          kcurrencycalculator.h  -  description
                             -------------------
    begin                : Thu Apr 8 2004
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

#ifndef KCURRENCYCALCULATOR_H
#define KCURRENCYCALCULATOR_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qwidget.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kcurrencycalculatordecl.h"
#include "../mymoney/mymoneyfile.h"

/**
  * @author Thomas Baumgart
  */

class KCurrencyCalculator : public KCurrencyCalculatorDecl
{
  Q_OBJECT
  
public:
  /**
    * @param from the @p from currency
    * @param to   the @p to currency
    * @param value the value to be converted
    * @param shares the number of foreign currency units
    * @param date the date when the conversion took place
    * @param resultFraction the smallest fraction of the result (default 100)
    *
    * @note @p value must not be 0!
    */ 
  KCurrencyCalculator(const MyMoneyCurrency& from, const MyMoneyCurrency& to, const MyMoneyMoney& value, const MyMoneyMoney& shares, const QDate& date, const int resultFraction = 100, QWidget *parent=0, const char *name=0);
  ~KCurrencyCalculator();

  const MyMoneyMoney price(void) const;

protected slots:
  void slotSetFromTo(void);
  void slotSetToFrom(void);
  void slotUpdateResult(const QString& txt);
  void slotUpdateRate(const QString& txt);
  virtual void accept();

private:
  MyMoneyCurrency     m_fromCurrency;
  MyMoneyCurrency     m_toCurrency;
  MyMoneyCurrency*    m_updateCurrency;
  MyMoneyMoney        m_result;
  MyMoneyMoney        m_value;
  QDate               m_date;
  int                 m_sign;
  int                 m_resultFraction;
};

#endif
