/***************************************************************************
                          mymoneyprice  -  description
                             -------------------
    begin                : Sun Nov 21 2004
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

#ifndef MYMONEYPRICE_H
#define MYMONEYPRICE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qcstring.h>
#include <qstring.h>
#include <qdatetime.h>
#include <qpair.h>
#include <qmap.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/mymoneymoney.h>

/**
  * @author Thomas Baumgart
  */

/**
  * This class represents an exchange rate of a security, currency or commodity
  * based on another security, currency or commodity for a specific date.
  * The term security is used in this class as a placeholder for all
  * those previously mentioned items.
  * In general, the other security is a currency.
  *
  * The securities and the rate form the following equation:
  *
  * @code
  *
  *   toSecurity = rate * fromSecurity
  *
  * @endcode
  *
  * Using the @p price() member function, one can retrieve the price based
  * upon the @p toSecurity or the @p fromSecurity.
  */
class MyMoneyPrice
{
public:
  MyMoneyPrice();
  MyMoneyPrice(const QCString& from, const QCString& to, const QDate& date, const MyMoneyMoney& rate, const QString& source = QString());
  virtual ~MyMoneyPrice();

  /**
    * This method returns the price information based on the
    * security referenced by id. If id is empty (default), the
    * price is returned based on the fromSecurity. If this price
    * object is invalid (see isValid()) MyMoneyMoney(1,1) is returned.
    *
    * @param id return price to be the multiplicator of the security
    *           given by @p id. Defaults to @p fromSecurity.
    *
    * @return returns the exchange rate (price) as MyMoneyMoney object.
    *
    * If @p is not empty and does not match either security ids of this price
    * an exception will be thrown.
    */
  const MyMoneyMoney rate(const QCString& id = QCString()) const;

  const QDate date(void) const { return m_date; };
  const QString source(void) const { return m_source; };
  const QCString from(void) const { return m_fromSecurity; };
  const QCString to(void) const { return m_toSecurity; };

  /**
    * Check whether the object is valid or not. An MyMoneyPrice object
    * is valid if the date is valid and both security ids are set. In case
    * of an invalid object, price() always returns 1.
    *
    * @retval true if price object is valid
    * @retval false if price object is not valid
    */
  const bool isValid(void) const;

private:
  QCString      m_fromSecurity;
  QCString      m_toSecurity;
  QDate         m_date;
  MyMoneyMoney  m_rate;
  QString       m_source;
};


typedef QPair<QCString, QCString> MyMoneySecurityPair;
typedef QMap<QDate, MyMoneyPrice> MyMoneyPriceEntries;
typedef QMap<MyMoneySecurityPair, MyMoneyPriceEntries> MyMoneyPriceList;

#endif
