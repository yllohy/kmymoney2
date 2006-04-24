/***************************************************************************
                          mymoneystatement.cpp
                          -------------------
    begin                : Mon Aug 30 2004
    copyright            : (C) 2000-2004 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
                           Ace Jones <acejones@users.sourceforge.net>
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

#include <qdom.h>
#include <qstringlist.h>
#include <qfile.h>
#include <qtextstream.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "../../kdecompat.h"
#include "mymoneystatement.h"

const QStringList kAccountTypeTxt = QStringList::split(",","none,checkings,savings,investment,creditcard,invalid");
const QStringList kActionText = QStringList::split(",","none,buy,sell,reinvestdividend,cashdividend,invalid");

void MyMoneyStatement::write(QDomElement& _root,QDomDocument* _doc) const
{
  QDomElement e = _doc->createElement("STATEMENT");
  _root.appendChild(e);

  e.setAttribute("verson","1.1");
  e.setAttribute("accountname", m_strAccountName);
  e.setAttribute("accountnumber", m_strAccountNumber);
  e.setAttribute("currency", m_strCurrency);
  e.setAttribute("begindate", m_dateBegin.toString(Qt::ISODate));
  e.setAttribute("enddate", m_dateEnd.toString(Qt::ISODate));
  e.setAttribute("closingbalance", QString::number(m_moneyClosingBalance));
  e.setAttribute("type", kAccountTypeTxt[m_eType]);

  // iterate over transactions, and add each one
  QValueList<Transaction>::const_iterator it_t = m_listTransactions.begin();
  while ( it_t != m_listTransactions.end() )
  {
    QDomElement p = _doc->createElement("TRANSACTION");
    p.setAttribute("dateposted", (*it_t).m_datePosted.toString(Qt::ISODate));
    p.setAttribute("payee", (*it_t).m_strPayee);
    p.setAttribute("memo", (*it_t).m_strMemo);
    p.setAttribute("number", (*it_t).m_strNumber);
    p.setAttribute("amount", QString::number((*it_t).m_moneyAmount,'f',20));
    p.setAttribute("bankid", (*it_t).m_strBankID);

    if (m_eType == etInvestment)
    {
      p.setAttribute("shares", QString::number((*it_t).m_dShares,'f', 10));
      p.setAttribute("action", kActionText[(*it_t).m_eAction]);
      p.setAttribute("security", (*it_t).m_strSecurity);
    }

    e.appendChild(p);

    ++it_t;
  }

  // iterate over prices, and add each one
  QValueList<Price>::const_iterator it_p = m_listPrices.begin();
  while ( it_p != m_listPrices.end() )
  {
    QDomElement p = _doc->createElement("PRICE");
    p.setAttribute("dateposted", (*it_p).m_date.toString(Qt::ISODate));
    p.setAttribute("security", (*it_p).m_strSecurity);
    p.setAttribute("amount", QString::number((*it_p).m_moneyAmount));

    e.appendChild(p);

    ++it_p;
  }

  // iterate over securities, and add each one
  QValueList<Security>::const_iterator it_s = m_listSecurities.begin();
  while ( it_s != m_listSecurities.end() )
  {
    QDomElement p = _doc->createElement("SECURITY");
    p.setAttribute("name", (*it_s).m_strName);
    p.setAttribute("symbol", (*it_s).m_strSymbol);
    p.setAttribute("id", (*it_s).m_strId);

    e.appendChild(p);

    ++it_s;
  }

}

bool MyMoneyStatement::read(const QDomElement& _e)
{
  bool result = false;

  if ( _e.tagName() == "STATEMENT" )
  {
    result = true;

    m_strAccountName = _e.attribute("accountname");
    m_strAccountNumber = _e.attribute("accountnumber");
    m_strCurrency = _e.attribute("currency");
    m_dateBegin = QDate::fromString(_e.attribute("begindate"),Qt::ISODate);
    m_dateEnd = QDate::fromString(_e.attribute("enddate"),Qt::ISODate);
    m_moneyClosingBalance = _e.attribute("closingbalance").toDouble();

    int i = kAccountTypeTxt.findIndex(_e.attribute("type",kAccountTypeTxt[1]));
    if ( i != -1 )
      m_eType = static_cast<EType>(i);

    QDomNode child = _e.firstChild();
    while(!child.isNull() && child.isElement())
    {
      QDomElement c = child.toElement();

      if ( c.tagName() == "TRANSACTION" )
      {
        MyMoneyStatement::Transaction t;

        t.m_datePosted = QDate::fromString(c.attribute("dateposted"),Qt::ISODate);
        t.m_moneyAmount = c.attribute("amount").toDouble();
        t.m_strMemo = c.attribute("memo");
        t.m_strNumber = c.attribute("number");
        t.m_strPayee = c.attribute("payee");
        t.m_strBankID = c.attribute("bankid");

        if (m_eType == etInvestment)
        {
          t.m_dShares = c.attribute("shares").toDouble();
          t.m_strSecurity = c.attribute("security");
          int i = kActionText.findIndex(c.attribute("action",kActionText[1]));
          if ( i != -1 )
            t.m_eAction = static_cast<Transaction::EAction>(i);
        }

        m_listTransactions += t;
      }
      else if ( c.tagName() == "PRICE" )
      {
        MyMoneyStatement::Price p;

        p.m_date = QDate::fromString(c.attribute("dateposted"), Qt::ISODate);
        p.m_strSecurity = c.attribute("security");
        p.m_moneyAmount = c.attribute("amount").toDouble();

        m_listPrices += p;
      }
      else if ( c.tagName() == "SECURITY" )
      {
        MyMoneyStatement::Security s;

        s.m_strName = c.attribute("name");
        s.m_strSymbol = c.attribute("symbol");
        s.m_strId = c.attribute("id");

        m_listSecurities += s;
      }
      child = child.nextSibling();
    }
  }

  return result;
}

bool MyMoneyStatement::isStatementFile(const QString& _filename)
{
  // filename is a statement file if it contains the tag "<KMYMONEY2-STATEMENT>" somewhere.
  bool result = false;

  QFile f( _filename );
  if ( f.open( IO_ReadOnly ) )
  {
    QTextStream ts( &f );

    while ( !ts.atEnd() && !result )
      if ( ts.readLine().contains("<KMYMONEY-STATEMENT>",false) )
        result = true;

    f.close();
  }

  return result;
}

void MyMoneyStatement::writeXMLFile( const MyMoneyStatement& _s, const QString& _filename )
{
  static unsigned filenum = 1;
  QString filename = _filename;
  if ( filename.isEmpty() ) {
    filename = QString("statement-%1%2.xml").arg((filenum<10)?"0":"").arg(filenum);
    filenum++;
  }

  QDomDocument* doc = new QDomDocument("KMYMONEY-STATEMENT");
  Q_CHECK_PTR(doc);

  //writeStatementtoXMLDoc(_s,doc);
  QDomProcessingInstruction instruct = doc->createProcessingInstruction(QString("xml"), QString("version=\"1.0\" encoding=\"utf-8\""));
  doc->appendChild(instruct);
  QDomElement eroot = doc->createElement("KMYMONEY-STATEMENT");
  doc->appendChild(eroot);
  _s.write(eroot,doc);

  QFile g( filename );
  g.open( IO_WriteOnly );

  QTextStream stream(&g);
#if KDE_IS_VERSION(3,2,0)
  stream.setEncoding(QTextStream::UnicodeUTF8);
  stream << doc->toString();
#else
  //stream.setEncoding(QTextStream::Locale);
  QCString temp = doc->toCString();
  stream << temp.data();
#endif
  g.close();

  delete doc;
}

bool MyMoneyStatement::readXMLFile( MyMoneyStatement& _s, const QString& _filename )
{
  bool result = false;
  QFile f( _filename );
  f.open( IO_ReadOnly );
  QDomDocument* doc = new QDomDocument;
  if(doc->setContent(&f, FALSE))
  {
    QDomElement rootElement = doc->documentElement();
    if(!rootElement.isNull())
    {
      QDomNode child = rootElement.firstChild();
      while(!child.isNull() && child.isElement())
      {
        result = true;
        QDomElement childElement = child.toElement();
        _s.read(childElement);

        child = child.nextSibling();
      }
    }
  }
  delete doc;

  return result;
}
// vim:cin:si:ai:et:ts=2:sw=2:
