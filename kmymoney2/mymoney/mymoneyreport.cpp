/***************************************************************************
                          imymoneystoragestream.cpp  -  description
                             -------------------
    begin                : Sun July 4 2004
    copyright            : (C) 2000-2004 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
                           Ace Jones <ace.j@hotpop.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "config.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <qstring.h>
#include <qdom.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyfile.h"
#include "mymoneyreport.h"
 
//ReportContainer gContainer;

const QStringList MyMoneyReport::kRowTypeText = QStringList::split(",","none,assetliability,expenseincome,category,topcategory,account,payee,month,week",true);
const QStringList MyMoneyReport::kColumnTypeText = QStringList::split(",","none,months,bimonths,quarters,,,,,,,,,years",true);
const QStringList MyMoneyReport::kQueryColumnsText = QStringList::split(",","none,number,payee,,category,,,,memo,,,,,,,,account,,,,,,,,,,,,,,,,reconcileflag",true);
const MyMoneyReport::EReportType MyMoneyReport::kTypeArray[] = { eNoReport, ePivotTable, ePivotTable, eQueryTable, eQueryTable, eQueryTable, eQueryTable, eQueryTable, eQueryTable };

// This should live in mymoney/mymoneytransactionfilter.h
static const QStringList kTypeText = QStringList::split(",","all,payments,deposits,transfers,none");
static const QStringList kStateText = QStringList::split(",","all,notreconciled,cleared,reconciled,frozen,none");
static const QStringList kDateLockText = QStringList::split(",", "alldates,untiltoday,currentmonth,currentyear,monthtodate,yeartodate,lastmonth,lastyear,last30days,last3months,last6months,last12months,next30days,next3months,next6months,next12months,userdefined");

void MyMoneyReport::write(QDomElement& e, QDomDocument *doc) const
{
  // No matter what changes, be sure to have a 'type' attribute.  Only change
  // the major type if it becomes impossible to maintain compatability with 
  // older versions of the program as new features are added to the reports.
  // Feel free to change the minor type every time a change is made here.

  if ( m_reportType == ePivotTable )
  {
    e.setAttribute("type","pivottable 1.4");
    e.setAttribute("name", m_name);
    e.setAttribute("comment", m_comment);
    e.setAttribute("group", m_group);
    e.setAttribute("showsubaccounts", m_showSubAccounts);
    e.setAttribute("convertcurrency", m_convertCurrency);
    e.setAttribute("favorite", m_favorite);
    e.setAttribute("rowtype", kRowTypeText[m_rowType]);
    e.setAttribute("columntype", kColumnTypeText[m_columnType]);
    e.setAttribute("id", m_id);
    e.setAttribute("datelock", kDateLockText[m_dateLock]);
  }
  else if ( m_reportType == eQueryTable )
  {
    e.setAttribute("type","querytable 1.3");
    e.setAttribute("name", m_name);
    e.setAttribute("comment", m_comment);
    e.setAttribute("group", m_group);
    e.setAttribute("convertcurrency", m_convertCurrency);
    e.setAttribute("favorite", m_favorite);
    e.setAttribute("rowtype", kRowTypeText[m_rowType]);
    e.setAttribute("id", m_id);
    e.setAttribute("datelock", kDateLockText[m_dateLock]);
    
    QStringList columns;
    unsigned qc = m_queryColumns;
    unsigned it_qc = eQCbegin;
    while ( it_qc != eQCend )
    {
      if ( qc & it_qc )
        columns += kQueryColumnsText[it_qc];
      it_qc *= 2;
    }
    e.setAttribute("querycolumns", columns.join(","));
  }
   
  //
  // Text Filter
  //
    
  QRegExp textfilter;
  if ( textFilter(textfilter))
  {
    QDomElement f = doc->createElement("TEXT");
    f.setAttribute("pattern", textfilter.pattern());
    f.setAttribute("casesensitive", textfilter.caseSensitive());
    f.setAttribute("regex", !textfilter.wildcard());
    e.appendChild(f);
  }

  //
  // Type & State Filters
  //
  QValueList<int> typelist;
  if ( types(typelist) && ! typelist.empty() )
  {
    // iterate over payees, and add each one
    QValueList<int>::const_iterator it_type = typelist.begin();
    while ( it_type != typelist.end() )
    {
      QDomElement p = doc->createElement("TYPE");
      p.setAttribute("type", kTypeText[*it_type]);
      e.appendChild(p);
      
      ++it_type;
    }
  }      

  QValueList<int> statelist;
  if ( states(statelist) && ! statelist.empty() )
  {
    // iterate over payees, and add each one
    QValueList<int>::const_iterator it_state = statelist.begin();
    while ( it_state != statelist.end() )
    {
      QDomElement p = doc->createElement("STATE");
      p.setAttribute("state", kStateText[*it_state]);
      e.appendChild(p);
      
      ++it_state;
    }
  }      
  //
  // Number Filter
  //
    
  QString nrFrom, nrTo;
  if ( numberFilter(nrFrom, nrTo) )
  {
    QDomElement f = doc->createElement("NUMBER");
    f.setAttribute("from", nrFrom);
    f.setAttribute("to", nrTo);
    e.appendChild(f);
  }
  
  //
  // Amount Filter
  //

  MyMoneyMoney from, to;
  if ( amountFilter(from,to) ) // bool getAmountFilter(MyMoneyMoney&,MyMoneyMoney&);
  {
    QDomElement f = doc->createElement("AMOUNT");
    f.setAttribute("from", from.toString());
    f.setAttribute("to", to.toString());
    e.appendChild(f);
  }

  //
  // Payees Filter
  //
  
  QCStringList payeelist;
  if ( payees(payeelist) )
  {
    if ( payeelist.empty() )
    {
      QDomElement p = doc->createElement("PAYEE");
      e.appendChild(p);
    }
    else
    {
      // iterate over payees, and add each one
      QCStringList::const_iterator it_payee = payeelist.begin();
      while ( it_payee != payeelist.end() )
      {
        QDomElement p = doc->createElement("PAYEE");
        p.setAttribute("id", *it_payee);
        e.appendChild(p);
        
        ++it_payee;
      }      
    }
  }

  //
  // Accounts Filter
  //
  
  QCStringList accountlist;
  if ( accounts(accountlist) )
  {
    // iterate over accounts, and add each one
    QCStringList::const_iterator it_account = accountlist.begin();
    while ( it_account != accountlist.end() )
    {
      QDomElement p = doc->createElement("ACCOUNT");
      p.setAttribute("id", *it_account);
      e.appendChild(p);
      
      ++it_account;
    }      
  }

  //
  // Categories Filter
  //
        
  accountlist.clear();
  if ( categories(accountlist) )
  {
    // iterate over accounts, and add each one
    QCStringList::const_iterator it_account = accountlist.begin();
    while ( it_account != accountlist.end() )
    {
      QDomElement p = doc->createElement("CATEGORY");
      p.setAttribute("id", *it_account);
      e.appendChild(p);
      
      ++it_account;
    }      
  }
    
  //
  // Date Filter
  //
    
  if ( m_dateLock == userDefined )
  {
    QDate dateFrom, dateTo;
    if ( dateFilter( dateFrom, dateTo ) )
    {
      QDomElement f = doc->createElement("DATES");
      if ( dateFrom.isValid() )
        f.setAttribute("from", dateFrom.toString(Qt::ISODate));
      if ( dateTo.isValid() )
        f.setAttribute("to", dateTo.toString(Qt::ISODate));
      e.appendChild(f);
    }
  }
}

bool MyMoneyReport::read(const QDomElement& e)
{
  // The goal of this reading method is 100% backward AND 100% forward
  // compatability.  Any report ever created with any version of KMyMoney
  // should be able to be loaded by this method (as long as it's one of the
  // report types supported in this version, of course)
        
  bool result = false;
  
  if ( 
    QString("REPORT") == e.tagName() 
    && 
    (
      (  e.attribute("type").find(QString("pivottable 1.")) == 0 )
      ||
      (  e.attribute("type").find(QString("querytable 1.")) == 0 )
    )
  )      
  {
    result = true;

    int i;
    m_name = e.attribute("name");
    m_comment = e.attribute("comment","Extremely old report");
    
    // Do not load saved versions of the default reports.  In older versions
    // of the file format (pivot 1.2 & query 1.1), we saved the default reports.
    // Now default reports are generated every time, so there's no need to load them.   
    if ( m_comment == "Default Report" )
      result = false;

    m_group = e.attribute("group");
    m_id = e.attribute("id");
    m_showSubAccounts = e.attribute("showsubaccounts","0").toUInt();
    m_convertCurrency = e.attribute("convertcurrency","1").toUInt();
    m_favorite = e.attribute("favorite","0").toUInt();
    
    QString datelockstr = e.attribute("datelock","userdefined");
    // Handle the pivot 1.2/query 1.1 case where the values were saved as
    // numbers
    bool ok = false;
    i = datelockstr.toUInt(&ok);
    if ( !ok )
    {
      i = kDateLockText.findIndex(datelockstr);
      if ( i == -1 )
        i = userDefined;
    }
    setDateFilter( i );
    
    i = kRowTypeText.findIndex(e.attribute("rowtype","expenseincome"));
    if ( i != -1 )
    {
      setRowType( static_cast<ERowType>(i) );
    }
    
    i = kColumnTypeText.findIndex(e.attribute("columntype","months"));      
    if ( i != -1 )
      setColumnType( static_cast<EColumnType>(i) );
    
    unsigned qc = 0;
    QStringList columns = QStringList::split(",",e.attribute("querycolumns","none"));
    QStringList::const_iterator it_column = columns.begin();
    while (it_column != columns.end())
    {
      i = kQueryColumnsText.findIndex(*it_column);
      if ( i != -1 )
        qc |= i;
    
      ++it_column;
    }
    setQueryColumns( static_cast<EQueryColumns>(qc) );

    QDomNode child = e.firstChild();
    while(!child.isNull() && child.isElement())
    {
      QDomElement c = child.toElement();
      if(QString("TEXT") == c.tagName() && c.hasAttribute("pattern"))
      {
        setTextFilter(QRegExp(c.attribute("pattern"),c.attribute("casesensitive","1").toUInt(),!c.attribute("regex","1").toUInt()));
      }
      if(QString("TYPE") == c.tagName() && c.hasAttribute("type"))
      {
        i = kTypeText.findIndex(c.attribute("type"));
        if ( i != -1 )
          addType(i);
      }
      if(QString("STATE") == c.tagName() && c.hasAttribute("state"))
      {
        i = kStateText.findIndex(c.attribute("state"));
        if ( i != -1 )
          addState(i);
      }
      if(QString("NUMBER") == c.tagName())
      {
        setNumberFilter(c.attribute("from"),c.attribute("to"));        	
      }
      if(QString("AMOUNT") == c.tagName())
      {
        setAmountFilter(MyMoneyMoney(c.attribute("from","0/100")),MyMoneyMoney(c.attribute("to","0/100")));        	
      }
      if(QString("DATES") == c.tagName())
      {
	QDate from, to;
	if ( c.hasAttribute("from") )
          from = QDate::fromString(c.attribute("from"),Qt::ISODate);
	if ( c.hasAttribute("to") )
          to = QDate::fromString(c.attribute("to"),Qt::ISODate);
        MyMoneyTransactionFilter::setDateFilter(from,to);
      }
      if(QString("PAYEE") == c.tagName() )
      {
        addPayee(c.attribute("id").latin1());
      }
      if(QString("CATEGORY") == c.tagName() && c.hasAttribute("id"))
      {
        addCategory(c.attribute("id").latin1());
      }
      if(QString("ACCOUNT") == c.tagName() && c.hasAttribute("id"))
      {
        addAccount(c.attribute("id").latin1());
      }
      child = child.nextSibling();
    }
  }
  return result;  
}

// vim:cin:si:ai:et:ts=2:sw=2:
