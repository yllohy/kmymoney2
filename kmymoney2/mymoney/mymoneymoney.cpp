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

const QString MyMoneyMoney::formatMoney(/*QString locale="C", bool addPrefixPostfix=false*/void) const
{
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

    if (bNegative)
    {
      if (right < 10)
        return QString("-%1.0%2").arg((long)left).arg(right);
      else if (right == 0)
        return QString("-%1.00").arg((long)left);
      else
        return QString("-%1.%2").arg((long)left).arg(right);
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
  }
  else
    return QString("0.00");
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
