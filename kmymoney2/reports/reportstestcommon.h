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

extern QString acAsset;
extern QString acLiability;
extern QString acExpense;
extern QString acIncome;
extern QString acChecking;
extern QString acCredit;
extern QString acSolo;
extern QString acParent;
extern QString acChild;
extern QString acSecondChild;
extern QString acGrandChild1;
extern QString acGrandChild2;
extern QString acForeign;
extern QString acCanChecking;
extern QString acJpyChecking;
extern QString acCanCash;
extern QString acJpyCash;
extern QString inBank;
extern QString eqStock1;
extern QString eqStock2;
extern QString acInvestment;
extern QString acStock1;
extern QString acStock2;
extern QString acDividends;
extern QString acInterest;
extern QString acTax;
extern QString acCash;

class TransactionHelper: public MyMoneyTransaction
{
private:
  QString m_id;
public:
  TransactionHelper( const QDate& _date, const QString& _action, MyMoneyMoney _value, const QString& _accountid, const QString& _categoryid, const QString& _currencyid = QString(), const QString& _payee="Test Payee" );
  ~TransactionHelper();
  void update(void);
protected:
  TransactionHelper(void) {}
};

class InvTransactionHelper: public TransactionHelper
{
public:
  InvTransactionHelper( const QDate& _date, const QString& _action, MyMoneyMoney _shares, MyMoneyMoney _value, const QString& _stockaccountid, const QString& _transferid, const QString& _categoryid );
  void init( const QDate& _date, const QString& _action, MyMoneyMoney _shares, MyMoneyMoney _value, const QString& _stockaccountid, const QString& _transferid, const QString& _categoryid );
};

class BudgetEntryHelper
{
private:
  QDate m_date;
  QString m_categoryid;
  bool m_applytosub;
  MyMoneyMoney m_amount;

public:
  BudgetEntryHelper( void ): m_applytosub(false) {}
  BudgetEntryHelper( const QDate& _date, const QString& _categoryid, bool _applytosub, const MyMoneyMoney& _amount ): m_date(_date), m_categoryid(_categoryid), m_applytosub(_applytosub), m_amount(_amount) {}
};

class BudgetHelper: public QValueList<BudgetEntryHelper>
{
  MyMoneyMoney budgetAmount( const QDate& _date, const QString& _categoryid, bool& _applytosub );
};

extern QString makeAccount( const QString& _name, MyMoneyAccount::accountTypeE _type, MyMoneyMoney _balance, const QDate& _open, const QString& _parent, QString _currency="", bool _taxReport = false );
extern void makePrice(const QString& _currencyid, const QDate& _date, const MyMoneyMoney& _price );
QString makeEquity(const QString& _name, const QString& _symbol );
extern void makeEquityPrice(const QString& _id, const QDate& _date, const MyMoneyMoney& _price );
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
