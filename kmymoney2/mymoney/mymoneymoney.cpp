/***************************************************************************
                          mymoneymymoney.cpp  -  description
                             -------------------
    begin                : Thu Feb 21 2002
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
#include "mymoneymoney.h"

static unsigned char _thousandSeparator = ',';
static unsigned char _decimalSeparator = '.';

void MyMoneyMoney::setThousandSeparator(const unsigned char separator)
{
  _thousandSeparator = separator;
}

unsigned char MyMoneyMoney::thousandSeparator(void)
{
  return _thousandSeparator;
}

void MyMoneyMoney::setDecimalSeparator(const unsigned char separator)
{
  _decimalSeparator = separator;
}

unsigned char MyMoneyMoney::decimalSeparator(void)
{
  return _decimalSeparator;
}

MyMoneyMoney::MyMoneyMoney(const QString& pszAmountInPence)
{
  m_64Value = 0;

  QString res = pszAmountInPence;
  int pos;
  while((pos = res.find(_thousandSeparator)) != -1)
    res.remove(pos, 1);
  if((pos = res.find(_decimalSeparator)) != -1)
    res.remove(pos, 1);

  if(res.length() > 0)
    m_64Value = atoll( res );
}

const QString MyMoneyMoney::formatMoney(/*QString locale="C", bool addPrefixPostfix=false*/void) const
{
  QString res;

  // Once we really support multiple currencies then this method will
  // be much better than using KGlobal::locale()->formatMoney.
  if (m_64Value != 0)
  {
    bool bNegative=false;
    signed64 left = (signed64)m_64Value/100;
    short right = (short)m_64Value-(left*100);
    if (right<0)
    {
      right = -right;
      left = -left;
      bNegative=true;
    }

    res = QString("%1").arg((long)left);
    int pos = res.length();
    while(0 < (pos -= 3))
      res.insert(pos, _thousandSeparator);
    QString format;

    res += _decimalSeparator;
    if (bNegative)
      res.insert(0, '-');

    if (right < 10)
      res += QString("0%1").arg(right);
    else if(right == 0)
      res += "00";
    else
      res += QString("%1").arg(right);
/*
    }
    else
    {
      if (right < 10)
        return QString("%1.0%2").arg((long)left).arg(right);
      else if (right == 0)
        return QString("%1.00").arg((long)left);
      else
        return QString("%1.%2").arg((long)left).arg(right);
    }
*/
  }
  else
    res = QString("0")+QChar(_decimalSeparator)+"00";

  return res;
}

QDataStream &operator<<(QDataStream &s, const MyMoneyMoney &money)
{
  // We WILL lose data here if the user has more than 2 billion pounds :-(
  // QT defined it here as long:
  // qglobal.h:typedef long          Q_INT64;

  return s << (Q_INT64)money.m_64Value;
}

QDataStream &operator>>(QDataStream &s, MyMoneyMoney &money)
{
//  Uncomment to read in a kmymoney2 <= 0.5.0 file.
//  double d_amount;
//  s >> d_amount;
//  money.m_64Value = static_cast<signed64>(100*static_cast<double>(d_amount));
//  return s;

  Q_INT64 amount;
  s >> amount;
  money = static_cast<Q_INT64>(amount);
  return s;
}
