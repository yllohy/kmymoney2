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

unsigned char MyMoneyMoney::_thousandSeparator = ',';
unsigned char MyMoneyMoney::_decimalSeparator = '.';
MyMoneyMoney::fileVersionE MyMoneyMoney::_fileVersion = MyMoneyMoney::FILE_4_BYTE_VALUE;

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

void MyMoneyMoney::setFileVersion(fileVersionE version)
{
  _fileVersion = version;
}

MyMoneyMoney::MyMoneyMoney(const QString& pszAmountInPence)
{
  m_64Value = 0;

  QString res = pszAmountInPence;
  int pos;
  while((pos = res.find(_thousandSeparator)) != -1)
    res.remove(pos, 1);

  if((pos = res.find(_decimalSeparator)) != -1) {
    // make sure, we have exactly two digits of fractional part

    // truncate, if too long
    if((res.length() - pos - 1) > 2) {
      res = res.left(pos+3);
    }

    // append 0's until enough
    while((res.length() - pos - 1) < 2) {
      res += '0';
    }
    // now remove the decimal symbol
    res.remove(pos, 1);
  }

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
    if (right < 0){
      right = -right;
      bNegative=true;
    }
    if (left < 0) {
      left = -left;
      bNegative=true;
    }

    res = QString("%1").arg((long)left);
    int pos = res.length();
    while(0 < (pos -= 3))
      res.insert(pos, thousandSeparator());
    QString format;

    res += decimalSeparator();
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
    res = QString("0")+QChar(decimalSeparator())+"00";

  return res;
}

QDataStream &operator<<(QDataStream &s, const MyMoneyMoney &money)
{
  // We WILL lose data here if the user has more than 2 billion pounds :-(
  // QT defined it here as long:
  // qglobal.h:typedef long          Q_INT64;

  switch(MyMoneyMoney::_fileVersion) {
    case MyMoneyMoney::FILE_4_BYTE_VALUE:
      if(money.m_64Value & 0xffffffff00000000LL)
        qWarning("Lost data while writing out MyMoneyMoney object using deprecated 4 byte writer");

      s << static_cast<Q_INT32> (money.m_64Value & 0xffffffff);
      break;

    default:
      qDebug("Unknown file version while writing MyMoneyMoney object! Use FILE_8_BYTE_VALUE");
      // tricky fall through here

    case MyMoneyMoney::FILE_8_BYTE_VALUE:
      s << static_cast<Q_INT32> (money.m_64Value >> 32);
      s << static_cast<Q_INT32> (money.m_64Value & 0xffffffff);
      break;
  }
  return s;
}

QDataStream &operator>>(QDataStream &s, MyMoneyMoney &money)
{
  Q_INT32 tmp;
  switch(MyMoneyMoney::_fileVersion) {
    case MyMoneyMoney::FILE_4_BYTE_VALUE:
      s >> tmp;
      money.m_64Value = static_cast<signed64> (tmp);
      break;

    default:
      qDebug("Unknown file version while writing MyMoneyMoney object! FILE_8_BYTE_VALUE assumed");
      // tricky fall through here

    case MyMoneyMoney::FILE_8_BYTE_VALUE:
      s >> tmp;
      money.m_64Value = static_cast<signed64> (tmp);
      money.m_64Value <<= 32;
      s >> tmp;
      money.m_64Value |= static_cast<signed64> (tmp);
      break;
  }
  return s;
}
