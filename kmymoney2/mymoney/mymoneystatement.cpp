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

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneystatement.h"

void MyMoneyStatement::write(QDomElement& _root,QDomDocument* _doc) const
{
  QDomElement e = _doc->createElement("STATEMENT");
  _root.appendChild(e);

  e.setAttribute("verson","1.0");
  e.setAttribute("accountname", m_strAccountName);
  e.setAttribute("accountnumber", m_strAccountNumber);
  e.setAttribute("currency", m_strCurrency);
  e.setAttribute("begindate", m_dateBegin.toString(Qt::ISODate));
  e.setAttribute("enddate", m_dateEnd.toString(Qt::ISODate));
  e.setAttribute("closingbalance", QString::number(m_moneyClosingBalance));
  
  // iterate over transactions, and add each one
  QValueList<Transaction>::const_iterator it_t = m_listTransactions.begin();
  while ( it_t != m_listTransactions.end() )
  {
    QDomElement p = _doc->createElement("TRANSACTION");
    p.setAttribute("dateposted", (*it_t).m_datePosted.toString(Qt::ISODate));
    p.setAttribute("payee", (*it_t).m_strPayee);
    p.setAttribute("memo", (*it_t).m_strMemo);
    p.setAttribute("number", (*it_t).m_strNumber);
    p.setAttribute("amount", QString::number((*it_t).m_moneyAmount));
    e.appendChild(p);
    
    ++it_t;
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
    
    QDomNode child = _e.firstChild();
    while(!child.isNull() && child.isElement())
    {
      QDomElement c = child.toElement();
      MyMoneyStatement::Transaction t;

      t.m_datePosted = QDate::fromString(c.attribute("dateposted"),Qt::ISODate);
      t.m_moneyAmount = c.attribute("amount").toDouble();
      t.m_strMemo = c.attribute("memo");
      t.m_strNumber = c.attribute("number");
      t.m_strPayee = c.attribute("payee");
      
      m_listTransactions += t;
      child = child.nextSibling();
    }    
  }
  
  return result;
}

