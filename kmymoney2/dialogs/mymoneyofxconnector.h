/***************************************************************************
                         mymoneyofxconnector.cpp
                             -------------------
    begin                : Sat Nov 13 2004
    copyright            : (C) 2002 by Ace Jones
    email                : acejones@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MYMONEYOFXCONNECTOR_H
#define MYMONEYOFXCONNECTOR_H

// ----------------------------------------------------------------------------
// QT Includes

class QDate;

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "../mymoney/mymoneykeyvaluecontainer.h"

class MyMoneyAccount;
class MyMoneyInstitution;
class MyMoneyTransaction;

/**
@author ace jones
*/
class MyMoneyOfxConnector
{
  class Tag
  {
  public:
    Tag(const QString& _name): m_name(_name) {}
    Tag& subtag(const Tag& _tag) { m_contents += _tag; return *this; }
    Tag& element(const QString& _name, const QString& _data) { m_contents += "<"+_name+">"+_data+"\r\n"; return *this; }
    Tag& data(const QString& _data) { m_contents += _data; return *this; }
    operator QString(void) const { return QString("<%1>\r\n%2</%3>\r\n").arg(m_name).arg(m_contents).arg(m_name); }
    bool isEmpty(void) const { return m_contents.isEmpty(); }
  private:
    QString m_name;
    QString m_contents;
  };

public:
  MyMoneyOfxConnector(const MyMoneyAccount& _account);
  QString url(void) const;
  const QByteArray statementRequest(const QDate& _dtstart) const;
  const QByteArray statementResponse(const QDate& _dtstart) const;
  
protected:
  Tag investmentRequest(const QDate& _dtstart) const;
  Tag bankStatementRequest(const QDate& _dtstart) const;
  Tag creditCardRequest(const QDate& _dtstart) const; 
  Tag signOn(void) const;
  Tag transaction(const MyMoneyTransaction& _t) const;
  Tag bankStatementResponse(const QDate& _dtstart) const;
  Tag signOnResponse(void) const;
  
  QString iban(void) const;
  QString fiorg(void) const;
  QString fiid(void) const;
  QString username(void) const;
  QString password(void) const;
  QString accountnum(void) const;
  QString accounttype(void) const;
  
  static Tag message(const QString& _msgType, const QString& _trnType, const Tag& _request);
  static Tag messageResponse(const QString& _msgType, const QString& _trnType, const Tag& _response);
  static QString header(void);
  static QString uuid(void);

private:
  QString m_body;
  const MyMoneyAccount& m_account;
  const MyMoneyInstitution& m_institution;
  MyMoneyKeyValueContainer m_fiSettings;
};

#endif // OFXCONNECTOR_H
