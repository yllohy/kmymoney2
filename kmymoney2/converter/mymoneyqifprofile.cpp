/***************************************************************************
                          mymoneyqifprofile.cpp  -  description
                             -------------------
    begin                : Tue Dec 24 2002
    copyright            : (C) 2002 by Thomas Baumgart
    email                : thb@net-bembel.de
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

#include <kglobal.h>
#include <kconfig.h>
#include <klocale.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyqifprofile.h"
#include "../mymoney/mymoneyexception.h"
#include "../mymoney/mymoneymoney.h"

MyMoneyQifProfile::MyMoneyQifProfile()
  : m_isDirty(false)
{
  clear();
}

MyMoneyQifProfile::MyMoneyQifProfile(const QString& name)
{
  loadProfile(name);
}

MyMoneyQifProfile::~MyMoneyQifProfile()
{
}

void MyMoneyQifProfile::clear(void)
{
  m_dateFormat = "%d.%m.%yyyy";
  m_apostropheFormat = "2000-2099";
  m_valueMode = "";
  m_filterScriptImport = "";
  m_filterScriptExport = "";

  m_decimal.clear();
  m_decimal['$'] =
  m_decimal['Q'] =
  m_decimal['T'] =
  m_decimal['O'] =
  m_decimal['I'] = KGlobal::locale()->monetaryDecimalSymbol()[0];

  m_thousands.clear();
  m_thousands['$'] =
  m_thousands['Q'] =
  m_thousands['T'] =
  m_thousands['O'] =
  m_thousands['I'] = KGlobal::locale()->monetaryThousandsSeparator()[0];

  m_openingBalanceText = "Opening Balance";
  m_voidMark = "VOID ";
  m_accountDelimiter = "[";

  m_profileName = "";
  m_profileDescription = "";
  m_profileType = "Bank";
}

void MyMoneyQifProfile::loadProfile(const QString& name)
{
  KConfig* config = KGlobal::config();
  config->setGroup(name);

  clear();

  m_profileName = name;
  m_profileDescription = config->readEntry("Description", m_profileDescription);
  m_profileType = config->readEntry("Type", m_profileType);
  m_dateFormat = config->readEntry("DateFormat", m_dateFormat);
  m_apostropheFormat = config->readEntry("ApostropheFormat", m_apostropheFormat);
  m_accountDelimiter = config->readEntry("AccountDelimiter", m_accountDelimiter);
  m_openingBalanceText = config->readEntry("OpeningBalance", m_openingBalanceText);
  m_voidMark = config->readEntry("VoidMark", m_voidMark);
  m_filterScriptImport = config->readEntry("FilterScriptImport", m_filterScriptImport);
  m_filterScriptExport = config->readEntry("FilterScriptExport", m_filterScriptExport);

  // make sure, we remove any old stuff for now
  config->deleteEntry("FilterScript");
  
  QString tmp = QString(m_decimal['Q']) + m_decimal['T'] + m_decimal['I'] +
                m_decimal['$'] + m_decimal['O'];
  tmp = config->readEntry("Decimal", tmp);
  m_decimal['Q'] = tmp[0];
  m_decimal['T'] = tmp[1];
  m_decimal['I'] = tmp[2];
  m_decimal['$'] = tmp[3];
  m_decimal['O'] = tmp[4];

  tmp = QString(m_thousands['Q']) + m_thousands['T'] + m_thousands['I'] +
                m_thousands['$'] + m_thousands['O'];
  tmp = config->readEntry("Thousand", tmp);
  m_thousands['Q'] = tmp[0];
  m_thousands['T'] = tmp[1];
  m_thousands['I'] = tmp[2];
  m_thousands['$'] = tmp[3];
  m_thousands['O'] = tmp[4];

  m_isDirty = false;
}

void MyMoneyQifProfile::saveProfile(void)
{
  if(m_isDirty == true) {
    KConfig* config = KGlobal::config();
    config->setGroup(m_profileName);

    config->writeEntry("Description", m_profileDescription);
    config->writeEntry("Type", m_profileType);
    config->writeEntry("DateFormat", m_dateFormat);
    config->writeEntry("ApostropheFormat", m_apostropheFormat);
    config->writeEntry("AccountDelimiter", m_accountDelimiter);
    config->writeEntry("OpeningBalance", m_openingBalanceText);
    config->writeEntry("VoidMark", m_voidMark);
    config->writeEntry("FilterScriptImport", m_filterScriptImport);
    config->writeEntry("FilterScriptExport", m_filterScriptExport);

    QString tmp;

    tmp = QString(m_decimal['Q']) + m_decimal['T'] + m_decimal['I'] +
                  m_decimal['$'] + m_decimal['O'];
    config->writeEntry("Decimal", tmp);
    tmp = QString(m_thousands['Q']) + m_thousands['T'] + m_thousands['I'] + 
                m_thousands['$'] + m_thousands['O'];
    config->writeEntry("Thousand", tmp);
  }
  m_isDirty = false;
}

void MyMoneyQifProfile::setProfileName(const QString& name)
{
  if(m_profileName != name)
    m_isDirty = true;

  m_profileName = name;
}

void MyMoneyQifProfile::setProfileDescription(const QString& desc)
{
  if(m_profileDescription != desc)
    m_isDirty = true;

  m_profileDescription = desc;
}

void MyMoneyQifProfile::setProfileType(const QString& type)
{
  if(m_profileType != type)
    m_isDirty = true;
  m_profileType = type;
}

void MyMoneyQifProfile::setDateFormat(const QString& dateFormat)
{
  if(m_dateFormat != dateFormat)
    m_isDirty = true;

  m_dateFormat = dateFormat;
}

void MyMoneyQifProfile::setApostropheFormat(const QString& apostropheFormat)
{
  if(m_apostropheFormat != apostropheFormat)
    m_isDirty = true;

  m_apostropheFormat = apostropheFormat;
}

void MyMoneyQifProfile::setAmountDecimal(const QChar& def, const QChar& chr)
{
  QChar ch(chr);
  if(ch == QChar())
    ch = ' ';

  if(m_decimal[def] != ch)
    m_isDirty = true;

  m_decimal[def] = ch;
}

void MyMoneyQifProfile::setAmountThousands(const QChar& def, const QChar& chr)
{
  QChar ch(chr);
  if(ch == QChar())
    ch = ' ';

  if(m_thousands[def] != ch)
    m_isDirty = true;

  m_thousands[def] = ch;
}

const QChar MyMoneyQifProfile::amountDecimal(const QChar& def) const
{
  QChar chr = m_decimal[def];
  return chr;
}

const QChar MyMoneyQifProfile::amountThousands(const QChar& def) const
{
  QChar chr = m_thousands[def];
  return chr;
}

void MyMoneyQifProfile::setAccountDelimiter(const QString& delim)
{
  QString txt(delim);

  if(txt.isEmpty())
     txt = " ";
  else if(txt[0] != '[')
    txt = "[";

  if(m_accountDelimiter[0] != txt[0])
    m_isDirty = true;
  m_accountDelimiter = txt[0];
}

void MyMoneyQifProfile::setOpeningBalanceText(const QString& txt)
{
  if(m_openingBalanceText != txt)
    m_isDirty = true;
  m_openingBalanceText = txt;
}

void MyMoneyQifProfile::setVoidMark(const QString& txt)
{
  if(m_voidMark != txt)
    m_isDirty = true;
  m_voidMark = txt;
}

const QString MyMoneyQifProfile::accountDelimiter(void) const
{
  QString rc;

  switch(m_accountDelimiter[0]) {
    case ' ':
      rc = "  ";
      break;
    default:
      rc = "[]";
      break;
  }
  return rc;
}

const QString MyMoneyQifProfile::date(const QDate& datein) const
{
  const char* format = m_dateFormat.latin1();
  QString buffer;
  QChar delim;
  int maskLen;
  char maskChar;

  while(*format) {
    switch(*format) {
      case '%':
        maskLen = 0;
        maskChar = *++format;
        while(*format && *format == maskChar) {
          ++maskLen;
          ++format;
        }

        switch(maskChar) {
          case 'd':
            if(delim)
              buffer += delim;
            buffer += QString::number(datein.day());
            break;

          case 'm':
            if(delim)
              buffer += delim;
            if(maskLen == 3)
              buffer += datein.shortMonthName(datein.month());
            else
              buffer += QString::number(datein.month());
            break;

          case 'y':
            if(maskLen == 2) {
              buffer += twoDigitYear(delim, datein.year());
            } else {
              if(delim)
                buffer += delim;
              buffer += QString::number(datein.year());
            }
            break;
          default:
            throw new MYMONEYEXCEPTION("Invalid char in QifProfile date field");
            break;
        }
        delim = 0;
        break;

      default:
        if(delim)
          buffer += delim;
        delim = *format++;
        break;
    }
  }
  return buffer;
}

const QDate MyMoneyQifProfile::date(const QString& datein) const
{
  QString scannedParts[3];
  QString scannedDelim[2];
  QString formatParts[3];
  QString formatDelim[2];
  int part;
  int delim;
  unsigned int i;

  part = -1;
  delim = 0;
  for(i = 0; i < m_dateFormat.length(); ++i) {
    if(m_dateFormat[i] == '%') {
      ++part;
      if(part == 3) {
        qWarning("MyMoneyQifProfile::date(const QString& datein) Too many parts in date format");
        return QDate();
      }
      ++i;
    }
    switch(m_dateFormat[i].latin1()) {
      case 'm':
      case 'd':
      case 'y':
        formatParts[part] += m_dateFormat[i];
        break;
      case '/':
      case '.':
        if(delim == 2) {
          qWarning("MyMoneyQifProfile::date(const QString& datein) Too many delimiters in date format");
          return QDate();
        }
        formatDelim[delim] = m_dateFormat[i];
        ++delim;
        break;
      default:
        qWarning("MyMoneyQifProfile::date(const QString& datein) Invalid char in date format");
        return QDate();
    }
  }

  
  part = 0;
  delim = 0;
  bool prevWasChar = false;
  for(i = 0; i < datein.length(); ++i) {
    switch(datein[i].latin1()) {
      case '/':
      case '.':
      case '\'':
        if(delim == 2) {
          qWarning("MyMoneyQifProfile::date(const QString& datein) Too many delimiters in date field");
          return QDate();
        }
        scannedDelim[delim] = datein[i];
        ++delim;
        ++part;
        prevWasChar = false;
        break;
        
      default:
        if(prevWasChar && datein[i].isDigit()) {
          ++part;
          prevWasChar = false;
        }
        if(datein[i].isLetter())
          prevWasChar = true;
        scannedParts[part] += datein[i];
        break;
    }
  }

  int day, mon, yr;
  bool ok;
  for(i = 0; i < 2; ++i) {
    if(scannedDelim[i] != formatDelim[i]
    && scannedDelim[i] != QChar('\'')) {
      qWarning("MyMoneyQifProfile::date(const QString& datein) Invalid delimiter '%s' when '%s' was expected",
        scannedDelim[i].latin1(), formatDelim[i].latin1());
      return QDate();
    }
  }
  
  QString msg("No warning!");
  for(i = 0; i < 3; ++i) {
    switch(formatParts[i][0].latin1()) {
      case 'd':
        day = scannedParts[i].toUInt(&ok);
        msg = "Invalid numeric character in day string";
        break;
      case 'm':
        if(formatParts[i].length() != 3) {
          mon = scannedParts[i].toUInt(&ok);
          msg = "Invalid numeric character in month string";
        } else {
          for(i = 1; i <= 12; ++i) {
            if(QDate::shortMonthName(i).lower() == formatParts[i].lower()) {
              mon = i;
              ok = true;
              break;
            }
          }
          if(i == 13) {
            msg = "Unknown month '" + scannedParts[i] + "'";
          }
        }
        break;
      case 'y':
        ok = false;
        if(scannedParts[i].length() == formatParts[i].length()) {
          yr = scannedParts[i].toUInt(&ok);
          msg = "Invalid numeric character in month string";
          if(yr < 100) {      // two digit year info
            if(i > 1) {
              ok = true;
              if(scannedDelim[i-1] == QChar('\'')) {
                if(m_apostropheFormat == "1900-1949") {
                  if(yr < 50)
                    yr += 1900;
                  else
                    yr += 2000;
                } else if(m_apostropheFormat == "1900-1999") {
                  yr += 1900;
                } else if(m_apostropheFormat == "2000-2099") {
                  yr += 2000;
                } else {
                  msg = "Unsupported apostropheFormat!";
                  ok = false;
                }
              } else {
                if(m_apostropheFormat == "1900-1949") {
                  if(yr < 50)
                    yr += 2000;
                  else
                    yr += 1900;
                } else if(m_apostropheFormat == "1900-1999") {
                  yr += 2000;
                } else if(m_apostropheFormat == "2000-2099") {
                  yr += 1900;
                } else {
                  msg = "Unsupported apostropheFormat!";
                  ok = false;
                }
              }
            } else {
              msg = "Year as first parameter is not supported!";
            }
          } else if(yr < 1900) {
              msg = "Year not in range < 100 or >= 1900!";
          } else {
            ok = true;
          }
        } else {
          msg = "Length of year does not match expected length.";
        }
        break;
    }
    if(!ok) {
      qWarning(QString("MyMoneyQifProfile::date(const QString& datein) ") + msg.latin1());
      return QDate();
    }
  }
  return QDate(yr, mon, day);
}

const QString MyMoneyQifProfile::twoDigitYear(const QChar delim, int yr) const
{
  QChar realDelim = delim;
  QString buffer;

  if(delim) {
    if((m_apostropheFormat == "1900-1949" && yr <= 1949)
    || (m_apostropheFormat == "1900-1999" && yr <= 1999)
    || (m_apostropheFormat == "2000-2099" && yr >= 2000))
      realDelim = '\'';
    buffer += realDelim;
  }
  yr -= 1900;
  if(yr > 100)
    yr -= 100;

  if(yr < 10)
    buffer += "0";

  buffer += QString::number(yr);
  return buffer;  
}

const QString MyMoneyQifProfile::value(const QChar& def, const MyMoneyMoney& valuein) const
{
  unsigned char _decimalSeparator;
  unsigned char _thousandsSeparator;
  QString res;

  _decimalSeparator = MyMoneyMoney::decimalSeparator();
  _thousandsSeparator = MyMoneyMoney::thousandSeparator();

  MyMoneyMoney::setDecimalSeparator(amountDecimal(def));
  MyMoneyMoney::setThousandSeparator(amountThousands(def));
  res = valuein.formatMoney();

  MyMoneyMoney::setDecimalSeparator(_decimalSeparator);
  MyMoneyMoney::setThousandSeparator(_thousandsSeparator);

  return res;
}

const MyMoneyMoney MyMoneyQifProfile::value(const QChar& def, const QString& valuein) const
{
  unsigned char _decimalSeparator;
  unsigned char _thousandsSeparator;
  MyMoneyMoney res;

  _decimalSeparator = MyMoneyMoney::decimalSeparator();
  _thousandsSeparator = MyMoneyMoney::thousandSeparator();

  MyMoneyMoney::setDecimalSeparator(amountDecimal(def));
  MyMoneyMoney::setThousandSeparator(amountThousands(def));

  res = MyMoneyMoney(valuein);

  MyMoneyMoney::setDecimalSeparator(_decimalSeparator);
  MyMoneyMoney::setThousandSeparator(_thousandsSeparator);

  return res;
}

void MyMoneyQifProfile::setFilterScriptImport(const QString& script)
{
  if(m_filterScriptImport != script)
    m_isDirty = true;
    
  m_filterScriptImport = script;
}

void MyMoneyQifProfile::setFilterScriptExport(const QString& script)
{
  if(m_filterScriptExport != script)
    m_isDirty = true;

  m_filterScriptExport = script;
}
