/***************************************************************************
                          mymoneyqifprofile.h  -  description
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

#ifndef MYMONEYQIFPROFILE_H
#define MYMONEYQIFPROFILE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qobject.h>
#include <qstring.h>
#include <qcstring.h>
class QDate;

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class MyMoneyMoney;

/**
  * @author Thomas Baumgart
  */

class MyMoneyQifProfile : public QObject
{
  Q_OBJECT

public: 
  MyMoneyQifProfile();
  MyMoneyQifProfile(const QString& name);
  ~MyMoneyQifProfile();

  const QString& profileName(void) const { return m_profileName; };
  void setProfileName(const QString& name);

  void loadProfile(const QString& name);
  void saveProfile(void);

  const QDate date(const QString& datein) const;
  const QString date(const QDate& datein) const;

  const MyMoneyMoney value(const QChar& def, const QString& valuein) const;
  const QString value(const QChar& def, const MyMoneyMoney& valuein) const;

  const QString& dateFormat(void) const { return m_dateFormat; };
  const QString& apostropheFormat(void) const { return m_apostropheFormat; };
  const QChar amountDecimal(const QChar& def) const;
  const QChar amountThousands(const QChar& def) const;
  const QString& profileDescription(void) const { return m_profileDescription; };
  const QString& profileType(void) const { return m_profileType; };
  const QString& openingBalanceText(void) const { return m_openingBalanceText; };
  const QString accountDelimiter(void) const;
  const QString& voidMark(void) const { return m_voidMark; };
  const QString& filterScriptImport(void) const { return m_filterScriptImport; };
  const QString& filterScriptExport(void) const { return m_filterScriptExport; };

  /**
    * This method presets the member variables with the default values.
    */
  void clear(void);

  /**
    * This method is used to determine, if a profile has been changed or not
    */
  const bool isDirty(void) const { return m_isDirty; };

public slots:
  void setProfileDescription(const QString& desc);
  void setProfileType(const QString& type);
  void setDateFormat(const QString& dateFormat);
  void setApostropheFormat(const QString& apostropheFormat);
  void setAmountDecimal(const QChar& def, const QChar& chr);
  void setAmountThousands(const QChar& def, const QChar& chr);
  void setAccountDelimiter(const QString& delim);
  void setOpeningBalanceText(const QString& text);
  void setVoidMark(const QString& txt);
  void setFilterScriptImport(const QString& txt);
  void setFilterScriptExport(const QString& txt);

private:
  const QString twoDigitYear(const QChar delim, int yr) const;

private:
  bool      m_isDirty;
  QString   m_profileName;
  QString   m_profileDescription;
  QString   m_dateFormat;
  QString   m_apostropheFormat;
  QString   m_valueMode;
  QString   m_profileType;
  QString   m_openingBalanceText;
  QString   m_voidMark;
  QString   m_accountDelimiter;
  QString   m_filterScriptImport;
  QString   m_filterScriptExport;
  QMap<QChar, QChar> m_decimal;
  QMap<QChar, QChar> m_thousands;
};

#endif
