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

extern const MyMoneyMoney moCheckingOpen;
extern const MyMoneyMoney moCreditOpen;
extern const MyMoneyMoney moConverterCheckingOpen;
extern const MyMoneyMoney moConverterCreditOpen;
extern const MyMoneyMoney moZero;
extern const MyMoneyMoney moSolo;
extern const MyMoneyMoney moParent1;
extern const MyMoneyMoney moParent2;
extern const MyMoneyMoney moParent;
extern const MyMoneyMoney moChild;
extern const MyMoneyMoney moThomas;
extern const MyMoneyMoney moNoPayee;

extern QCString acAsset;
extern QCString acLiability;
extern QCString acExpense;
extern QCString acIncome;
extern QCString acChecking;
extern QCString acCredit;
extern QCString acSolo;
extern QCString acParent;
extern QCString acChild;
extern QCString acSecondChild;
extern QCString acGrandChild1;
extern QCString acGrandChild2;
extern QCString acForeign;
extern QCString acCanChecking;
extern QCString acJpyChecking;
extern QCString acCanCash;
extern QCString acJpyCash;
extern QCString inBank;
extern QCString eqStock1;
extern QCString eqStock2;
extern QCString acInvestment;
extern QCString acStock1;
extern QCString acStock2;
extern QCString acDividends;
extern QCString acInterest;

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

class BudgetEntryHelper
{
private:
  QDate m_date;
  QCString m_categoryid;
  bool m_applytosub;
  MyMoneyMoney m_amount;
    
public:
  BudgetEntryHelper( void ): m_applytosub(false) {} 
  BudgetEntryHelper( const QDate& _date, const QCString& _categoryid, bool _applytosub, const MyMoneyMoney& _amount ): m_date(_date), m_categoryid(_categoryid), m_applytosub(_applytosub), m_amount(_amount) {}
};

class BudgetHelper: public QValueList<BudgetEntryHelper>
{
  MyMoneyMoney budgetAmount( const QDate& _date, const QCString& _categoryid, bool& _applytosub );
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
