/***************************************************************************
                          reportstestcommon.h
                          -------------------
    copyright            : (C) 2002-2005 by Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
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

#ifndef REPORTSTESTCOMMON_H
#define REPORTSTESTCOMMON_H
 
#include <qvaluelist.h>
class QDomDocument;

#include "../mymoney/mymoneyaccount.h"
#include "../mymoney/mymoneytransaction.h"
#include "../mymoney/mymoneymoney.h"
class MyMoneyReport;

namespace reports {
class PivotTable;
class QueryTable;
}

namespace test {

class TransactionHelper: public MyMoneyTransaction
{
private:
  QCString m_id;
public:
  TransactionHelper( const QDate& _date, const QCString& _action, MyMoneyMoney _value, const QCString& _accountid, const QCString& _categoryid, const QCString& _currencyid = QCString(), const QString& _payee="Test Payee" );
  ~TransactionHelper();
  void update(void);
protected:
  TransactionHelper(void) {}
};

class InvTransactionHelper: public TransactionHelper
{
public:
  InvTransactionHelper( const QDate& _date, const QCString& _action, MyMoneyMoney _shares, MyMoneyMoney _value, const QCString& _stockaccountid, const QCString& _transferid, const QCString& _categoryid );
};

extern QCString makeAccount( const QString& _name, MyMoneyAccount::accountTypeE _type, MyMoneyMoney _balance, const QDate& _open, const QCString& _parent, QCString _currency="" );
extern void makePrice(const QCString& _currencyid, const QDate& _date, const MyMoneyMoney& _price );
QCString makeEquity(const QString& _name, const QString& _symbol );
extern void makeEquityPrice(const QCString& _id, const QDate& _date, const MyMoneyMoney& _price );
extern void writeRCFtoXMLDoc( const MyMoneyReport& filter, QDomDocument* doc );
extern void writeTabletoHTML( const reports::PivotTable& table, const QString& _filename = QString() );
extern void writeTabletoHTML( const reports::QueryTable& table, const QString& _filename = QString() );
extern void writeTabletoCSV( const reports::PivotTable& table, const QString& _filename = QString() );
extern void writeTabletoCSV( const reports::QueryTable& table, const QString& _filename = QString() );
extern void writeRCFtoXML( const MyMoneyReport& filter, const QString& _filename = QString() );
extern bool readRCFfromXMLDoc( QValueList<MyMoneyReport>& list, QDomDocument* doc );
extern bool readRCFfromXML( QValueList<MyMoneyReport>& list, const QString& filename );
extern void XMLandback( MyMoneyReport& filter );
extern MyMoneyMoney searchHTML(const QString& _html, const QString& _search);

} // end namespace test

#endif // REPORTSTESTCOMMON_H

// vim:cin:si:ai:et:ts=2:sw=2:
