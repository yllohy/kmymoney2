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

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif
 
// ----------------------------------------------------------------------------
// System Includes

#if HAVE_LIBUUID
#include <uuid/uuid.h>
#endif

// ----------------------------------------------------------------------------
// QT Includes

#include <qdatetime.h>
#include <qregexp.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kdebug.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "../mymoney/mymoneyfile.h"
#include "../mymoney/mymoneyaccount.h"
#include "../mymoney/mymoneyinstitution.h"
#include "../mymoney/mymoneykeyvaluecontainer.h"
#include "mymoneyofxconnector.h"

MyMoneyOfxConnector::MyMoneyOfxConnector(const MyMoneyAccount& _account):
  m_account(_account),
  m_institution( MyMoneyFile::instance()->institution( _account.institutionId() ) )  
{
  m_fiSettings = m_institution.ofxConnectionSettings();
}

QString MyMoneyOfxConnector::iban(void) const { return m_fiSettings.value("iban"); }
QString MyMoneyOfxConnector::fiorg(void) const { return m_fiSettings.value("fiorg"); }
QString MyMoneyOfxConnector::fiid(void) const { return m_fiSettings.value("fiid"); }
QString MyMoneyOfxConnector::username(void) const { return m_fiSettings.value("user"); }
QString MyMoneyOfxConnector::password(void) const { return m_fiSettings.value("password"); }
QString MyMoneyOfxConnector::accountnum(void) const { return m_account.number(); }
QString MyMoneyOfxConnector::url(void) const { return m_fiSettings.value("url"); }

QString MyMoneyOfxConnector::accounttype(void) const 
{ 
  QString result = "BANK";
  
  switch( m_account.accountType() )
  {
  case MyMoneyAccount::Investment:
    result = "INV";
    break;
  case MyMoneyAccount::CreditCard:
    result = "CC";
    break;
  default:
    break;
  }
  
  // This is a bit of a personalized hack.  Sometimes we may want to override the
  // ofx type for an account.  For now, I will stash it in the notes!
  
  QRegExp rexp("OFXTYPE:([A-Z]*)");
  if ( rexp.search(m_account.description()) != -1 )
  {
    result = rexp.cap(1);
    kdDebug(2) << "MyMoneyOfxConnector::accounttype() overriding to " << result << endl;
  }
  
  return result;
}

const QByteArray MyMoneyOfxConnector::statementRequest(const QDate& _dtstart) const
{
  QString request;
  
  if ( accounttype()=="CC" )
    request = header() + Tag("OFX").subtag(signOn()).subtag(creditCardRequest(_dtstart));
  else if ( accounttype()=="INV" )
    request = header() + Tag("OFX").subtag(signOn()).subtag(investmentRequest(_dtstart));
  else
    request = header() + Tag("OFX").subtag(signOn()).subtag(bankStatementRequest(_dtstart));
 
  // remove the trailing zero
  QByteArray result = request.utf8();
  result.truncate(result.size()-1);
  
  return result; 
}

QString MyMoneyOfxConnector::uuid(void)
{
#if HAVE_LIBUUID
  uuid_t uuid;
  char buffer[37];
  uuid_generate(uuid);
  uuid_unparse(uuid,buffer);
  
  return QString(buffer).upper();
#else
  kdDebug(2) << "MyMoneyOfxConnector::uuid(): Warning: Program has been compiled without libuuid.  Unable to generate UUID's for this connection.  Some banks will accept OFX connectios without UUID's, others will not." << endl;
  return "000";
#endif
}

MyMoneyOfxConnector::Tag MyMoneyOfxConnector::message(const QString& _msgType, const QString& _trnType, const Tag& _request)
{
  return Tag(_msgType+"MSGSRQV1")
    .subtag(Tag(_trnType+"TRNRQ")
      .element("TRNUID",uuid())
      .element("CLTCOOKIE","1")
      .subtag(_request));
}

MyMoneyOfxConnector::Tag MyMoneyOfxConnector::investmentRequest(const QDate& _dtstart) const
{
  QString dtnow_string = QDateTime::currentDateTime().toString(Qt::ISODate).remove(QRegExp("[^0-9]"));
  QString dtstart_string = _dtstart.toString(Qt::ISODate).remove(QRegExp("[^0-9]"));

  return message("INVSTMT","INVSTMT",Tag("INVSTMTRQ")
    .subtag(Tag("INVACCTFROM").element("BROKERID", fiorg()).element("ACCTID", accountnum())) 
    .subtag(Tag("INCTRAN").element("DTSTART",dtstart_string).element("INCLUDE","Y"))
    .element("INCOO","Y")
    .subtag(Tag("INCPOS").element("DTASOF", dtnow_string).element("INCLUDE","Y"))
    .element("INCBAL","Y"));   
}

MyMoneyOfxConnector::Tag MyMoneyOfxConnector::bankStatementRequest(const QDate& _dtstart) const
{
  QString dtstart_string = _dtstart.toString(Qt::ISODate).remove(QRegExp("[^0-9]"));

  return message("BANK","STMT",Tag("STMTRQ")
    .subtag(Tag("BANKACCTFROM").element("BANKID", iban()).element("ACCTID", accountnum()).element("ACCTTYPE", "CHECKING"))
    .subtag(Tag("INCTRAN").element("DTSTART",dtstart_string).element("INCLUDE","Y")));
}

MyMoneyOfxConnector::Tag MyMoneyOfxConnector::creditCardRequest(const QDate& _dtstart) const
{
  QString dtstart_string = _dtstart.toString(Qt::ISODate).remove(QRegExp("[^0-9]"));

  return message("CREDITCARD","CCSTMT",Tag("CCSTMTRQ")
    .subtag(Tag("CCACCTFROM").element("ACCTID",accountnum()))
    .subtag(Tag("INCTRAN").element("DTSTART",dtstart_string).element("INCLUDE","Y")));
}

QString MyMoneyOfxConnector::header(void)
{
  return QString("OFXHEADER:100\r\n"
                 "DATA:OFXSGML\r\n"
                 "VERSION:102\r\n"
                 "SECURITY:NONE\r\n"
                 "ENCODING:USASCII\r\n"
                 "CHARSET:1252\r\n"
                 "COMPRESSION:NONE\r\n"
                 "OLDFILEUID:NONE\r\n"
                 "NEWFILEUID:%1\r\n"
                 "\r\n").arg(uuid());
}

MyMoneyOfxConnector::Tag MyMoneyOfxConnector::signOn(void) const
{
  QString dtnow_string = QDateTime::currentDateTime().toString(Qt::ISODate).remove(QRegExp("[^0-9]"));

  Tag fi("FI");
  fi.element("ORG",fiorg());
  if ( !fiid().isEmpty() )
    fi.element("FID",fiid());

  return Tag("SIGNONMSGSRQV1")
    .subtag(Tag("SONRQ")
      .element("DTCLIENT",dtnow_string)
      .element("USERID",username())
      .element("USERPASS",password())
      .element("LANGUAGE","ENG")
      .subtag(fi)
      .element("APPID","QWIN")
      .element("APPVER","1100"));
}
