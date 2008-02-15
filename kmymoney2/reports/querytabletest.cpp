/***************************************************************************
                          querytabletest.cpp
                          -------------------
    copyright            : (C) 2002 by Thomas Baumgart
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

#include <qvaluelist.h>
#include <qvaluevector.h>
#include <qfile.h>

#include <kdebug.h>
#include <kdeversion.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include "reportstestcommon.h"
#include "querytabletest.h"

#define private public
#include "querytable.h"
#undef private

#include "../mymoney/mymoneyaccount.h"
#include "../mymoney/mymoneysecurity.h"
#include "../mymoney/mymoneyprice.h"
#include "../mymoney/storage/mymoneystoragedump.h"
#include "../mymoney/mymoneyreport.h"
#include "../mymoney/mymoneystatement.h"
#include "../mymoney/storage/mymoneystoragexml.h"

using namespace reports;
using namespace test;

QueryTableTest::QueryTableTest()
{
}

void QueryTableTest::setUp () {

  storage = new MyMoneySeqAccessMgr;
  file = MyMoneyFile::instance();
  file->attachStorage(storage);
  MyMoneyFileTransaction ft;
  file->addCurrency(MyMoneySecurity("CAD", "Canadian Dollar",        "C$"));
  file->addCurrency(MyMoneySecurity("USD", "US Dollar",              "$"));
  file->addCurrency(MyMoneySecurity("JPY", "Japanese Yen",           QChar(0x00A5), 100, 1));
  file->addCurrency(MyMoneySecurity("GBP", "British Pound",           "#"));
  file->setBaseCurrency(file->currency("USD"));

  MyMoneyPayee payeeTest("Test Payee");
  file->addPayee(payeeTest);
  MyMoneyPayee payeeTest2("Thomas Baumgart");
  file->addPayee(payeeTest2);

  acAsset = (MyMoneyFile::instance()->asset().id());
  acLiability = (MyMoneyFile::instance()->liability().id());
  acExpense = (MyMoneyFile::instance()->expense().id());
  acIncome = (MyMoneyFile::instance()->income().id());
  acChecking = makeAccount(QString("Checking Account"),MyMoneyAccount::Checkings,moCheckingOpen,QDate(2004,5,15),acAsset);
  acCredit = makeAccount(QString("Credit Card"),MyMoneyAccount::CreditCard,moCreditOpen,QDate(2004,7,15),acLiability);
  acSolo = makeAccount(QString("Solo"),MyMoneyAccount::Expense,0,QDate(2004,1,11),acExpense);
  acParent = makeAccount(QString("Parent"),MyMoneyAccount::Expense,0,QDate(2004,1,11),acExpense);
  acChild = makeAccount(QString("Child"),MyMoneyAccount::Expense,0,QDate(2004,2,11),acParent);
  acForeign = makeAccount(QString("Foreign"),MyMoneyAccount::Expense,0,QDate(2004,1,11),acExpense);

  MyMoneyInstitution i("Bank of the World","","","","","","");
  file->addInstitution(i);
  inBank = i.id();
  ft.commit();
}

void QueryTableTest::tearDown ()
{
  file->detachStorage(storage);
  delete storage;
}

void QueryTableTest::testQueryBasics()
{
  try
  {
    TransactionHelper t1q1( QDate(2004,1,1), MyMoneySplit::ActionWithdrawal, moSolo, acChecking, acSolo );
    TransactionHelper t2q1( QDate(2004,2,1), MyMoneySplit::ActionWithdrawal, moParent1, acCredit, acParent );
    TransactionHelper t3q1( QDate(2004,3,1), MyMoneySplit::ActionWithdrawal, moParent2, acCredit, acParent );
    TransactionHelper t4y1( QDate(2004,11,7), MyMoneySplit::ActionWithdrawal, moChild, acCredit, acChild );

    TransactionHelper t1q2( QDate(2004,4,1), MyMoneySplit::ActionWithdrawal, moSolo, acChecking, acSolo );
    TransactionHelper t2q2( QDate(2004,5,1), MyMoneySplit::ActionWithdrawal, moParent1, acCredit, acParent );
    TransactionHelper t3q2( QDate(2004,6,1), MyMoneySplit::ActionWithdrawal, moParent2, acCredit, acParent );
    TransactionHelper t4q2( QDate(2004,11,7), MyMoneySplit::ActionWithdrawal, moChild, acCredit, acChild );

    TransactionHelper t1y2( QDate(2005,1,1), MyMoneySplit::ActionWithdrawal, moSolo, acChecking, acSolo );
    TransactionHelper t2y2( QDate(2005,5,1), MyMoneySplit::ActionWithdrawal, moParent1, acCredit, acParent );
    TransactionHelper t3y2( QDate(2005,9,1), MyMoneySplit::ActionWithdrawal, moParent2, acCredit, acParent );
    TransactionHelper t4y2( QDate(2004,11,7), MyMoneySplit::ActionWithdrawal, moChild, acCredit, acChild );

    unsigned cols;

    MyMoneyReport filter;
    filter.setRowType( MyMoneyReport::eCategory );
    cols = MyMoneyReport::eQCnumber | MyMoneyReport::eQCpayee | MyMoneyReport::eQCaccount;
    filter.setQueryColumns( static_cast<MyMoneyReport::EQueryColumns>(cols) ); //
    filter.setName("Transactions by Category");
    XMLandback(filter);
    QueryTable qtbl_1(filter);

    writeTabletoHTML(qtbl_1,"Transactions by Category.html");
    CPPUNIT_ASSERT(qtbl_1.m_transactions.count() == 12);
    CPPUNIT_ASSERT(qtbl_1.m_transactions[0]["categorytype"]=="Expense");
    CPPUNIT_ASSERT(qtbl_1.m_transactions[0]["category"]=="Parent");
    CPPUNIT_ASSERT(qtbl_1.m_transactions[0]["postdate"]=="2004-02-01");
    CPPUNIT_ASSERT(qtbl_1.m_transactions[11]["categorytype"]=="Expense");
    CPPUNIT_ASSERT(qtbl_1.m_transactions[11]["category"]=="Solo");
    CPPUNIT_ASSERT(qtbl_1.m_transactions[11]["postdate"]=="2005-01-01");

    QString html = qtbl_1.renderHTML();
    CPPUNIT_ASSERT( searchHTML(html,i18n("Total")+" Parent") == -(moParent1 + moParent2) * 3 );
    CPPUNIT_ASSERT( searchHTML(html,i18n("Total")+" Parent: Child") == -(moChild) * 3 );
    CPPUNIT_ASSERT( searchHTML(html,i18n("Total")+" Solo") == -(moSolo) * 3 );
    CPPUNIT_ASSERT( searchHTML(html,i18n("Total")+" Expense") == -(moParent1 + moParent2 + moSolo + moChild) * 3 );
    CPPUNIT_ASSERT( searchHTML(html,i18n("Grand Total")) == -(moParent1 + moParent2 + moSolo + moChild) * 3 + moCheckingOpen + moCreditOpen );
    filter.setRowType( MyMoneyReport::eTopCategory );
    cols = MyMoneyReport::eQCnumber | MyMoneyReport::eQCpayee | MyMoneyReport::eQCaccount;
    filter.setQueryColumns( static_cast<MyMoneyReport::EQueryColumns>(cols) ); //
    filter.setName("Transactions by Top Category");
    XMLandback(filter);
    QueryTable qtbl_2(filter);

    writeTabletoHTML(qtbl_2,"Transactions by Top Category.html");
    CPPUNIT_ASSERT(qtbl_2.m_transactions.count() == 12);
    CPPUNIT_ASSERT(qtbl_2.m_transactions[0]["categorytype"]=="Expense");
    CPPUNIT_ASSERT(qtbl_2.m_transactions[0]["topcategory"]=="Parent");
    CPPUNIT_ASSERT(qtbl_2.m_transactions[0]["postdate"]=="2004-02-01");
    CPPUNIT_ASSERT(qtbl_2.m_transactions[8]["categorytype"]=="Expense");
    CPPUNIT_ASSERT(qtbl_2.m_transactions[8]["topcategory"]=="Parent");
    CPPUNIT_ASSERT(qtbl_2.m_transactions[8]["postdate"]=="2005-09-01");
    CPPUNIT_ASSERT(qtbl_2.m_transactions[11]["categorytype"]=="Expense");
    CPPUNIT_ASSERT(qtbl_2.m_transactions[11]["topcategory"]=="Solo");
    CPPUNIT_ASSERT(qtbl_2.m_transactions[11]["postdate"]=="2005-01-01");

    html = qtbl_2.renderHTML();
    CPPUNIT_ASSERT( searchHTML(html,i18n("Total")+" Parent") == -(moParent1 + moParent2 + moChild) * 3 );
    CPPUNIT_ASSERT( searchHTML(html,i18n("Total")+" Solo") == -(moSolo) * 3 );
    CPPUNIT_ASSERT( searchHTML(html,i18n("Total")+" Expense") == -(moParent1 + moParent2 + moSolo + moChild) * 3 );
    CPPUNIT_ASSERT( searchHTML(html,i18n("Grand Total")) == -(moParent1 + moParent2 + moSolo + moChild) * 3 + moCheckingOpen + moCreditOpen);

    filter.setRowType( MyMoneyReport::eAccount );
    filter.setName("Transactions by Account");
    cols = MyMoneyReport::eQCnumber | MyMoneyReport::eQCpayee | MyMoneyReport::eQCcategory;
    filter.setQueryColumns( static_cast<MyMoneyReport::EQueryColumns>(cols) ); //
    XMLandback(filter);
    QueryTable qtbl_3(filter);
    
    QString testDate = qtbl_3.m_transactions[14]["postdate"];

    writeTabletoHTML(qtbl_3,"Transactions by Account.html");
#if 1
    CPPUNIT_ASSERT(qtbl_3.m_transactions.count() == 16);
    CPPUNIT_ASSERT(qtbl_3.m_transactions[1]["account"]=="Checking Account");
    CPPUNIT_ASSERT(qtbl_3.m_transactions[1]["category"]=="Solo");
    CPPUNIT_ASSERT(qtbl_3.m_transactions[1]["postdate"]=="2004-01-01");
    CPPUNIT_ASSERT(qtbl_3.m_transactions[15]["account"]=="Credit Card");
    CPPUNIT_ASSERT(qtbl_3.m_transactions[15]["category"]=="Parent");
    CPPUNIT_ASSERT(qtbl_3.m_transactions[15]["postdate"]=="2005-09-01");
#else
    CPPUNIT_ASSERT(qtbl_3.m_transactions.count() == 12);
    CPPUNIT_ASSERT(qtbl_3.m_transactions[0]["account"]=="Checking Account");
    CPPUNIT_ASSERT(qtbl_3.m_transactions[0]["category"]=="Solo");
    CPPUNIT_ASSERT(qtbl_3.m_transactions[0]["postdate"]=="2004-01-01");
    CPPUNIT_ASSERT(qtbl_3.m_transactions[11]["account"]=="Credit Card");
    CPPUNIT_ASSERT(qtbl_3.m_transactions[11]["category"]=="Parent");
    CPPUNIT_ASSERT(qtbl_3.m_transactions[11]["postdate"]=="2005-09-01");
#endif

    html = qtbl_3.renderHTML();
    CPPUNIT_ASSERT( searchHTML(html,i18n("Total")+" Checking Account") == -(moSolo) * 3 + moCheckingOpen);
    CPPUNIT_ASSERT( searchHTML(html,i18n("Total")+" Credit Card") == -(moParent1 + moParent2 + moChild) * 3 + moCreditOpen );
    CPPUNIT_ASSERT( searchHTML(html,i18n("Grand Total")) == -(moParent1 + moParent2 + moSolo + moChild) * 3 + moCheckingOpen + moCreditOpen);

    filter.setRowType( MyMoneyReport::ePayee );
    filter.setName("Transactions by Payee");
    cols = MyMoneyReport::eQCnumber | MyMoneyReport::eQCmemo | MyMoneyReport::eQCcategory;
    filter.setQueryColumns( static_cast<MyMoneyReport::EQueryColumns>(cols) ); //
    XMLandback(filter);
    QueryTable qtbl_4(filter);

    writeTabletoHTML(qtbl_4,"Transactions by Payee.html");
    CPPUNIT_ASSERT(qtbl_4.m_transactions.count() == 12);
    CPPUNIT_ASSERT(qtbl_4.m_transactions[0]["payee"]=="Test Payee");
    CPPUNIT_ASSERT(qtbl_4.m_transactions[0]["category"]=="Solo");
    CPPUNIT_ASSERT(qtbl_4.m_transactions[0]["postdate"]=="2004-01-01");
    CPPUNIT_ASSERT(qtbl_4.m_transactions[8]["payee"]=="Test Payee");
    CPPUNIT_ASSERT(qtbl_4.m_transactions[8]["category"]=="Parent: Child");
    CPPUNIT_ASSERT(qtbl_4.m_transactions[8]["postdate"]=="2004-11-07");
    CPPUNIT_ASSERT(qtbl_4.m_transactions[11]["payee"]=="Test Payee");
    CPPUNIT_ASSERT(qtbl_4.m_transactions[11]["category"]=="Parent");
    CPPUNIT_ASSERT(qtbl_4.m_transactions[11]["postdate"]=="2005-09-01");

    html = qtbl_4.renderHTML();
    CPPUNIT_ASSERT( searchHTML(html,i18n("Total")+" Test Payee") == -(moParent1 + moParent2 + moSolo + moChild) * 3 );
    CPPUNIT_ASSERT( searchHTML(html,i18n("Grand Total")) == -(moParent1 + moParent2 + moSolo + moChild) * 3 + moCheckingOpen + moCreditOpen);

    filter.setRowType( MyMoneyReport::eMonth );
    filter.setName("Transactions by Month");
    cols = MyMoneyReport::eQCnumber | MyMoneyReport::eQCpayee | MyMoneyReport::eQCcategory;
    filter.setQueryColumns( static_cast<MyMoneyReport::EQueryColumns>(cols) ); //
    XMLandback(filter);
    QueryTable qtbl_5(filter);

    writeTabletoHTML(qtbl_5,"Transactions by Month.html");
    CPPUNIT_ASSERT(qtbl_5.m_transactions.count() == 12);
    CPPUNIT_ASSERT(qtbl_5.m_transactions[0]["payee"]=="Test Payee");
    CPPUNIT_ASSERT(qtbl_5.m_transactions[0]["category"]=="Solo");
    CPPUNIT_ASSERT(qtbl_5.m_transactions[0]["postdate"]=="2004-01-01");
    CPPUNIT_ASSERT(qtbl_5.m_transactions[8]["payee"]=="Test Payee");
    CPPUNIT_ASSERT(qtbl_5.m_transactions[8]["category"]=="Parent: Child");
    CPPUNIT_ASSERT(qtbl_5.m_transactions[8]["postdate"]=="2004-11-07");
    CPPUNIT_ASSERT(qtbl_5.m_transactions[11]["payee"]=="Test Payee");
    CPPUNIT_ASSERT(qtbl_5.m_transactions[11]["category"]=="Parent");
    CPPUNIT_ASSERT(qtbl_5.m_transactions[11]["postdate"]=="2005-09-01");

    html = qtbl_5.renderHTML();
    CPPUNIT_ASSERT( searchHTML(html,i18n("Total")+" Month of 2004-01-01") == -moSolo );
    CPPUNIT_ASSERT( searchHTML(html,i18n("Total")+" Month of 2004-11-01") == -(moChild) * 3 );
    CPPUNIT_ASSERT( searchHTML(html,i18n("Total")+" Month of 2004-05-01") == -moParent1 + moCheckingOpen );
    CPPUNIT_ASSERT( searchHTML(html,i18n("Grand Total")) == -(moParent1 + moParent2 + moSolo + moChild) * 3 + moCheckingOpen + moCreditOpen);

    filter.setRowType( MyMoneyReport::eWeek );
    filter.setName("Transactions by Week");
    cols = MyMoneyReport::eQCnumber | MyMoneyReport::eQCpayee | MyMoneyReport::eQCcategory;
    filter.setQueryColumns( static_cast<MyMoneyReport::EQueryColumns>(cols) ); //
    XMLandback(filter);
    QueryTable qtbl_6(filter);

    writeTabletoHTML(qtbl_6,"Transactions by Week.html");
    CPPUNIT_ASSERT(qtbl_6.m_transactions.count() == 12);
    CPPUNIT_ASSERT(qtbl_6.m_transactions[0]["payee"]=="Test Payee");
    CPPUNIT_ASSERT(qtbl_6.m_transactions[0]["category"]=="Solo");
    CPPUNIT_ASSERT(qtbl_6.m_transactions[0]["postdate"]=="2004-01-01");
    CPPUNIT_ASSERT(qtbl_6.m_transactions[11]["payee"]=="Test Payee");
    CPPUNIT_ASSERT(qtbl_6.m_transactions[11]["category"]=="Parent");
    CPPUNIT_ASSERT(qtbl_6.m_transactions[11]["postdate"]=="2005-09-01");

    html = qtbl_6.renderHTML();
    CPPUNIT_ASSERT( searchHTML(html,i18n("Total")+" Week of 2003-12-29") == -moSolo );
    CPPUNIT_ASSERT( searchHTML(html,i18n("Total")+" Week of 2004-11-01") == -(moChild) * 3 );
    CPPUNIT_ASSERT( searchHTML(html,i18n("Total")+" Week of 2005-08-29") == -moParent2 );
    CPPUNIT_ASSERT( searchHTML(html,i18n("Grand Total")) == -(moParent1 + moParent2 + moSolo + moChild) * 3 + moCheckingOpen + moCreditOpen);
  }
  catch(MyMoneyException *e)
  {
    CPPUNIT_FAIL(e->what());
    delete e;
  }

  // Test querytable::TableRow::operator> and operator==

  QueryTable::TableRow low;
  low["first"] = "A";
  low["second"] = "B";
  low["third"] = "C";

  QueryTable::TableRow high;
  high["first"] = "A";
  high["second"] = "C";
  high["third"] = "B";

  QueryTable::TableRow::setSortCriteria("first,second,third");
  CPPUNIT_ASSERT( low < high );
  CPPUNIT_ASSERT( low <= high );
  CPPUNIT_ASSERT( high > low );
  CPPUNIT_ASSERT( high <= high );
  CPPUNIT_ASSERT( high == high );
}

void QueryTableTest::testCashFlowAnalysis()
{
  //
  // Test IRR calculations
  //

  CashFlowList list;

  list += CashFlowListItem( QDate(2004,5,3),1000.0 );
  list += CashFlowListItem( QDate(2004,5,20),59.0 );
  list += CashFlowListItem( QDate(2004,6,3),14.0 );
  list += CashFlowListItem( QDate(2004,6,24),92.0 );
  list += CashFlowListItem( QDate(2004,7,6),63.0 );
  list += CashFlowListItem( QDate(2004,7,25),15.0 );
  list += CashFlowListItem( QDate(2004,8,5),92.0 );
  list += CashFlowListItem( QDate(2004,9,2),18.0 );
  list += CashFlowListItem( QDate(2004,9,21),5.0 );
  list += CashFlowListItem( QDate(2004,10,16),-2037.0 );

  MyMoneyMoney IRR(list.IRR(),1000);

  CPPUNIT_ASSERT( IRR == MyMoneyMoney(1676,1000) );

  list.pop_back();
  list += CashFlowListItem( QDate(2004,10,16),-1358.0 );

  IRR = MyMoneyMoney( list.IRR(), 1000 );

  CPPUNIT_ASSERT( IRR.isZero() );
}

void QueryTableTest::testAccountQuery()
{
  try
  {
    QString htmlcontext = QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\">\n<html><head><link rel=\"stylesheet\" type=\"text/css\" href=\"html/kmymoney2.css\"></head><body>\n%1\n</body></html>\n");

    //
    // No transactions, opening balances only
    //

    MyMoneyReport filter;
    filter.setRowType( MyMoneyReport::eInstitution );
    filter.setName("Accounts by Institution (No transactions)");
    XMLandback(filter);
    QueryTable qtbl_1(filter);

    writeTabletoHTML(qtbl_1,"Accounts by Institution (No transactions).html");
    CPPUNIT_ASSERT(qtbl_1.m_transactions.count() == 2);
    CPPUNIT_ASSERT(qtbl_1.m_transactions[0]["account"]=="Checking Account");
    CPPUNIT_ASSERT(MyMoneyMoney(qtbl_1.m_transactions[0]["value"])==moCheckingOpen);
    CPPUNIT_ASSERT(qtbl_1.m_transactions[0]["equitytype"].isEmpty());
    CPPUNIT_ASSERT(qtbl_1.m_transactions[1]["account"]=="Credit Card");
    CPPUNIT_ASSERT(MyMoneyMoney(qtbl_1.m_transactions[1]["value"])==moCreditOpen);
    CPPUNIT_ASSERT(qtbl_1.m_transactions[1]["equitytype"].isEmpty());

    QString html = qtbl_1.renderHTML();
    CPPUNIT_ASSERT( searchHTML(html,i18n("Total")+" None") == moCheckingOpen+moCreditOpen );
    CPPUNIT_ASSERT( searchHTML(html,i18n("Grand Total")) == moCheckingOpen+moCreditOpen );

    //
    // Adding in transactions
    //

    TransactionHelper t1q1( QDate(2004,1,1), MyMoneySplit::ActionWithdrawal, moSolo, acChecking, acSolo );
    TransactionHelper t2q1( QDate(2004,2,1), MyMoneySplit::ActionWithdrawal, moParent1, acCredit, acParent );
    TransactionHelper t3q1( QDate(2004,3,1), MyMoneySplit::ActionWithdrawal, moParent2, acCredit, acParent );
    TransactionHelper t4y1( QDate(2004,11,7), MyMoneySplit::ActionWithdrawal, moChild, acCredit, acChild );

    TransactionHelper t1q2( QDate(2004,4,1), MyMoneySplit::ActionWithdrawal, moSolo, acChecking, acSolo );
    TransactionHelper t2q2( QDate(2004,5,1), MyMoneySplit::ActionWithdrawal, moParent1, acCredit, acParent );
    TransactionHelper t3q2( QDate(2004,6,1), MyMoneySplit::ActionWithdrawal, moParent2, acCredit, acParent );
    TransactionHelper t4q2( QDate(2004,11,7), MyMoneySplit::ActionWithdrawal, moChild, acCredit, acChild );

    TransactionHelper t1y2( QDate(2005,1,1), MyMoneySplit::ActionWithdrawal, moSolo, acChecking, acSolo );
    TransactionHelper t2y2( QDate(2005,5,1), MyMoneySplit::ActionWithdrawal, moParent1, acCredit, acParent );
    TransactionHelper t3y2( QDate(2005,9,1), MyMoneySplit::ActionWithdrawal, moParent2, acCredit, acParent );
    TransactionHelper t4y2( QDate(2004,11,7), MyMoneySplit::ActionWithdrawal, moChild, acCredit, acChild );

    filter.setRowType( MyMoneyReport::eInstitution );
    filter.setName("Accounts by Institution (With Transactions)");
    XMLandback(filter);
    QueryTable qtbl_2(filter);

    CPPUNIT_ASSERT(qtbl_2.m_transactions.count() == 2);
    CPPUNIT_ASSERT(qtbl_2.m_transactions[0]["account"]=="Checking Account");
    CPPUNIT_ASSERT(MyMoneyMoney(qtbl_2.m_transactions[0]["value"])==(moCheckingOpen-moSolo*3));
    CPPUNIT_ASSERT(qtbl_2.m_transactions[1]["account"]=="Credit Card");
    CPPUNIT_ASSERT(MyMoneyMoney(qtbl_2.m_transactions[1]["value"])==(moCreditOpen-(moParent1 + moParent2 + moChild) * 3));

    html = qtbl_2.renderHTML();
    CPPUNIT_ASSERT( searchHTML(html,i18n("Grand Total")) == moCheckingOpen+moCreditOpen-(moParent1 + moParent2 + moSolo + moChild) * 3 );

    //
    // Account TYPES
    //

    filter.setRowType( MyMoneyReport::eAccountType );
    filter.setName("Accounts by Type");
    XMLandback(filter);
    QueryTable qtbl_3(filter);

    CPPUNIT_ASSERT(qtbl_3.m_transactions.count() == 2);
    CPPUNIT_ASSERT(qtbl_3.m_transactions[0]["account"]=="Checking Account");
    CPPUNIT_ASSERT(MyMoneyMoney(qtbl_3.m_transactions[0]["value"])==(moCheckingOpen-moSolo*3));
    CPPUNIT_ASSERT(qtbl_3.m_transactions[1]["account"]=="Credit Card");
    CPPUNIT_ASSERT(MyMoneyMoney(qtbl_3.m_transactions[1]["value"])==(moCreditOpen-(moParent1 + moParent2 + moChild) * 3));

    html = qtbl_3.renderHTML();
    CPPUNIT_ASSERT( searchHTML(html,i18n("Total")+" "+i18n("Checking")) == moCheckingOpen-moSolo*3 );
    CPPUNIT_ASSERT( searchHTML(html,i18n("Total")+" "+i18n("Credit Card")) == moCreditOpen-(moParent1 + moParent2 + moChild) * 3 );
    CPPUNIT_ASSERT( searchHTML(html,i18n("Grand Total")) == moCheckingOpen+moCreditOpen-(moParent1 + moParent2 + moSolo + moChild) * 3 );
  }
  catch(MyMoneyException *e)
  {
    CPPUNIT_FAIL(e->what());
    delete e;
  }
}

void QueryTableTest::testInvestment(void)
{
  try
  {
  // Equities
  eqStock1 = makeEquity("Stock1","STK1");
  eqStock2 = makeEquity("Stock2","STK2");

  // Accounts
  acInvestment = makeAccount("Investment",MyMoneyAccount::Investment,moZero,QDate(2004,1,1),acAsset);
  acStock1 = makeAccount("Stock 1",MyMoneyAccount::Stock,moZero,QDate(2004,1,1),acInvestment,eqStock1);
  acStock2 = makeAccount("Stock 2",MyMoneyAccount::Stock,moZero,QDate(2004,1,1),acInvestment,eqStock2);
  acDividends = makeAccount("Dividends",MyMoneyAccount::Income,moZero,QDate(2004,1,1),acIncome);
  acInterest = makeAccount("Interest",MyMoneyAccount::Income,moZero,QDate(2004,1,1),acIncome);

  // Transactions
  //                         Date             Action                              Shares  Price   Stock     Asset       Income
  InvTransactionHelper s1b1( QDate(2004,2,1), MyMoneySplit::ActionBuyShares,      1000.00, 100.00, acStock1, acChecking, QCString() );
  InvTransactionHelper s1b2( QDate(2004,3,1), MyMoneySplit::ActionBuyShares,      1000.00, 110.00, acStock1, acChecking, QCString() );
  InvTransactionHelper s1s1( QDate(2004,4,1), MyMoneySplit::ActionBuyShares,      -200.00, 120.00, acStock1, acChecking, QCString() );
  InvTransactionHelper s1s2( QDate(2004,5,1), MyMoneySplit::ActionBuyShares,      -200.00, 100.00, acStock1, acChecking, QCString() );
  InvTransactionHelper s1r1( QDate(2004,6,1), MyMoneySplit::ActionReinvestDividend, 50.00, 100.00, acStock1, QCString(), acDividends );
  InvTransactionHelper s1r2( QDate(2004,7,1), MyMoneySplit::ActionReinvestDividend, 50.00,  80.00, acStock1, QCString(), acDividends );
  InvTransactionHelper s1c1( QDate(2004,8,1), MyMoneySplit::ActionDividend,         10.00, 100.00, acStock1, acChecking, acDividends );
  InvTransactionHelper s1c2( QDate(2004,9,1), MyMoneySplit::ActionDividend,         10.00, 120.00, acStock1, acChecking, acDividends );
  InvTransactionHelper s1y1( QDate(2004,9,15), MyMoneySplit::ActionYield,           10.00, 110.00, acStock1, acChecking, acInterest );

  makeEquityPrice( eqStock1, QDate(2004,10,1), 100.00 );

  //
  // Investment Transactions Report
  //

  MyMoneyReport invtran_r(
      MyMoneyReport::eTopAccount,
      MyMoneyReport::eQCaction|MyMoneyReport::eQCshares|MyMoneyReport::eQCprice,
      MyMoneyTransactionFilter::userDefined,
      false,
      i18n("Investment Transactions"),
      i18n("Test Report")
    );
  invtran_r.setDateFilter(QDate(2004,1,1),QDate(2004,12,31));
  invtran_r.setInvestmentsOnly(true);
  XMLandback(invtran_r);
  QueryTable invtran(invtran_r);

#if 1
  writeTabletoHTML(invtran,"investment_transactions_test.html");
  CPPUNIT_ASSERT(invtran.m_transactions.count()==17);
  CPPUNIT_ASSERT(MyMoneyMoney(invtran.m_transactions[2]["value"])==MyMoneyMoney(100000.00));
  CPPUNIT_ASSERT(MyMoneyMoney(invtran.m_transactions[3]["value"])==MyMoneyMoney(110000.00));
  CPPUNIT_ASSERT(MyMoneyMoney(invtran.m_transactions[4]["value"])==MyMoneyMoney(-24000.00));
  CPPUNIT_ASSERT(MyMoneyMoney(invtran.m_transactions[5]["value"])==MyMoneyMoney(-20000.00));
  CPPUNIT_ASSERT(MyMoneyMoney(invtran.m_transactions[6]["value"])==MyMoneyMoney(  5000.00));
  CPPUNIT_ASSERT(MyMoneyMoney(invtran.m_transactions[7]["value"])==MyMoneyMoney(  4000.00));
  // need to fix these... fundamentally different from the original test
  //CPPUNIT_ASSERT(MyMoneyMoney(invtran.m_transactions[8]["value"])==MyMoneyMoney( -1000.00));
  //CPPUNIT_ASSERT(MyMoneyMoney(invtran.m_transactions[11]["value"])==MyMoneyMoney( -1200.00));
  //CPPUNIT_ASSERT(MyMoneyMoney(invtran.m_transactions[14]["value"])==MyMoneyMoney( -1100.00));

  CPPUNIT_ASSERT(MyMoneyMoney(invtran.m_transactions[2]["price"])==MyMoneyMoney(100.00));
  CPPUNIT_ASSERT(MyMoneyMoney(invtran.m_transactions[4]["price"])==MyMoneyMoney(120.00));
  CPPUNIT_ASSERT(MyMoneyMoney(invtran.m_transactions[6]["price"])==MyMoneyMoney(100.00));
  CPPUNIT_ASSERT(MyMoneyMoney(invtran.m_transactions[8]["price"])==MyMoneyMoney(  0.00));
  CPPUNIT_ASSERT(MyMoneyMoney(invtran.m_transactions[11]["price"])==MyMoneyMoney(  0.00));

  CPPUNIT_ASSERT(MyMoneyMoney(invtran.m_transactions[3]["shares"])==MyMoneyMoney(1000.00));
  CPPUNIT_ASSERT(MyMoneyMoney(invtran.m_transactions[5]["shares"])==MyMoneyMoney(-200.00));
  CPPUNIT_ASSERT(MyMoneyMoney(invtran.m_transactions[7]["shares"])==MyMoneyMoney(  50.00));
  CPPUNIT_ASSERT(MyMoneyMoney(invtran.m_transactions[9]["shares"])==MyMoneyMoney(   0.00));
  CPPUNIT_ASSERT(MyMoneyMoney(invtran.m_transactions[12]["shares"])==MyMoneyMoney(   0.00));

  CPPUNIT_ASSERT(invtran.m_transactions[2]["action"]=="Buy");
  CPPUNIT_ASSERT(invtran.m_transactions[4]["action"]=="Sell");
  CPPUNIT_ASSERT(invtran.m_transactions[6]["action"]=="Reinvest");
  CPPUNIT_ASSERT(invtran.m_transactions[8]["action"]=="Dividend");
  CPPUNIT_ASSERT(invtran.m_transactions[15]["action"]=="Yield");
#else
  CPPUNIT_ASSERT(invtran.m_transactions.count()==9);
  CPPUNIT_ASSERT(MyMoneyMoney(invtran.m_transactions[0]["value"])==MyMoneyMoney(100000.00));
  CPPUNIT_ASSERT(MyMoneyMoney(invtran.m_transactions[1]["value"])==MyMoneyMoney(110000.00));
  CPPUNIT_ASSERT(MyMoneyMoney(invtran.m_transactions[2]["value"])==MyMoneyMoney(-24000.00));
  CPPUNIT_ASSERT(MyMoneyMoney(invtran.m_transactions[3]["value"])==MyMoneyMoney(-20000.00));
  CPPUNIT_ASSERT(MyMoneyMoney(invtran.m_transactions[4]["value"])==MyMoneyMoney(  5000.00));
  CPPUNIT_ASSERT(MyMoneyMoney(invtran.m_transactions[5]["value"])==MyMoneyMoney(  4000.00));
  CPPUNIT_ASSERT(MyMoneyMoney(invtran.m_transactions[6]["value"])==MyMoneyMoney( -1000.00));
  CPPUNIT_ASSERT(MyMoneyMoney(invtran.m_transactions[7]["value"])==MyMoneyMoney( -1200.00));
  CPPUNIT_ASSERT(MyMoneyMoney(invtran.m_transactions[8]["value"])==MyMoneyMoney( -1100.00));

  CPPUNIT_ASSERT(MyMoneyMoney(invtran.m_transactions[0]["price"])==MyMoneyMoney(100.00));
  CPPUNIT_ASSERT(MyMoneyMoney(invtran.m_transactions[2]["price"])==MyMoneyMoney(120.00));
  CPPUNIT_ASSERT(MyMoneyMoney(invtran.m_transactions[4]["price"])==MyMoneyMoney(100.00));
  CPPUNIT_ASSERT(MyMoneyMoney(invtran.m_transactions[6]["price"])==MyMoneyMoney(  0.00));
  CPPUNIT_ASSERT(MyMoneyMoney(invtran.m_transactions[8]["price"])==MyMoneyMoney(  0.00));

  CPPUNIT_ASSERT(MyMoneyMoney(invtran.m_transactions[1]["shares"])==MyMoneyMoney(1000.00));
  CPPUNIT_ASSERT(MyMoneyMoney(invtran.m_transactions[3]["shares"])==MyMoneyMoney(-200.00));
  CPPUNIT_ASSERT(MyMoneyMoney(invtran.m_transactions[5]["shares"])==MyMoneyMoney(  50.00));
  CPPUNIT_ASSERT(MyMoneyMoney(invtran.m_transactions[7]["shares"])==MyMoneyMoney(   0.00));
  CPPUNIT_ASSERT(MyMoneyMoney(invtran.m_transactions[8]["shares"])==MyMoneyMoney(   0.00));

  CPPUNIT_ASSERT(invtran.m_transactions[0]["action"]=="Buy");
  CPPUNIT_ASSERT(invtran.m_transactions[2]["action"]=="Sell");
  CPPUNIT_ASSERT(invtran.m_transactions[4]["action"]=="Reinvest");
  CPPUNIT_ASSERT(invtran.m_transactions[6]["action"]=="Dividend");
  CPPUNIT_ASSERT(invtran.m_transactions[8]["action"]=="Yield");
#endif

  QString html = invtran.renderHTML();
#if 1
  // i think this is the correct amount. different treatment of dividend and yield
  CPPUNIT_ASSERT( searchHTML(html,i18n("Total Stock 1")) == MyMoneyMoney(175000.00) );
  CPPUNIT_ASSERT( searchHTML(html,i18n("Grand Total")) == MyMoneyMoney(175000.00) );
#else
  CPPUNIT_ASSERT( searchHTML(html,i18n("Total Stock 1")) == MyMoneyMoney(171700.00) );
  CPPUNIT_ASSERT( searchHTML(html,i18n("Grand Total")) == MyMoneyMoney(171700.00) );
#endif

  //
  // Investment Performance Report
  //

  MyMoneyReport invhold_r(
    MyMoneyReport::eAccountByTopAccount,
    MyMoneyReport::eQCperformance,
    MyMoneyTransactionFilter::userDefined,
    false,
    i18n("Investment Performance by Account"),
    i18n("Test Report")
  );
  invhold_r.setDateFilter(QDate(2004,1,1),QDate(2004,10,1));
  invhold_r.setInvestmentsOnly(true);
  XMLandback(invhold_r);
  QueryTable invhold(invhold_r);

  CPPUNIT_ASSERT(invhold.m_transactions.count()==2);
  CPPUNIT_ASSERT(MyMoneyMoney(invhold.m_transactions[0]["return"])==MyMoneyMoney("669/10000"));
  CPPUNIT_ASSERT(MyMoneyMoney(invhold.m_transactions[0]["buys"])==MyMoneyMoney(210000.00));
  CPPUNIT_ASSERT(MyMoneyMoney(invhold.m_transactions[0]["sells"])==MyMoneyMoney(-44000.00));
  CPPUNIT_ASSERT(MyMoneyMoney(invhold.m_transactions[0]["reinvestincome"])==MyMoneyMoney(9000.00));
  CPPUNIT_ASSERT(MyMoneyMoney(invhold.m_transactions[0]["cashincome"])==MyMoneyMoney(3300.00));
  CPPUNIT_ASSERT(MyMoneyMoney(invhold.m_transactions[0]["shares"])==MyMoneyMoney(1700.00));
  CPPUNIT_ASSERT(MyMoneyMoney(invhold.m_transactions[0]["price"])==MyMoneyMoney(100.00));
  CPPUNIT_ASSERT(MyMoneyMoney(invhold.m_transactions[1]["return"]).isZero());

  html = invhold.renderHTML();
  CPPUNIT_ASSERT( searchHTML(html,i18n("Grand Total")) == MyMoneyMoney(170000.00) );

#if 0
  // Dump file & reports
  QFile g( "investmentkmy.xml" );
  g.open( IO_WriteOnly );
  MyMoneyStorageXML xml;
  IMyMoneyStorageFormat& interface = xml;
  interface.writeFile(&g, dynamic_cast<IMyMoneySerialize*> (MyMoneyFile::instance()->storage()));
  g.close();

  invtran.dump("invtran.html","<html><head></head><body>%1</body></html>");
  invhold.dump("invhold.html","<html><head></head><body>%1</body></html>");
#endif

  }
  catch(MyMoneyException *e)
  {
    CPPUNIT_FAIL(e->what());
    delete e;
  }
}

// vim:cin:si:ai:et:ts=2:sw=2:
