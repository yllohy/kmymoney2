/***************************************************************************
                          pivottabletest.cpp
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

#include <qvaluelist.h>
#include <qvaluevector.h>
#include <qdom.h>
#include <qfile.h>

#include <kdebug.h>
#include <kdeversion.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kstandarddirs.h>

// DOH, mmreport.h uses this without including it!!
#include "../mymoney/mymoneyaccount.h"

#include "../mymoney/mymoneysecurity.h"
#include "../mymoney/mymoneyprice.h"
#include "../mymoney/mymoneyreport.h"
#include "../mymoney/mymoneystatement.h"
#include "../mymoney/storage/mymoneystoragedump.h"
#include "../mymoney/storage/mymoneystoragexml.h"

#define private public
#include "../reports/pivottable.h"
#undef private

#include "reportstestcommon.h"
#include "pivottabletest.h"

using namespace reports;
using namespace test;

PivotTableTest::PivotTableTest()
{
}

void PivotTableTest::setUp ()
{
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

  acSecondChild = makeAccount(QString("Second Child"),MyMoneyAccount::Expense,0,QDate(2004,2,11),acParent);
  acGrandChild1 = makeAccount(QString("Grand Child 1"),MyMoneyAccount::Expense,0,QDate(2004,2,11),acChild);
  acGrandChild2 = makeAccount(QString("Grand Child 2"),MyMoneyAccount::Expense,0,QDate(2004,2,11),acChild);

  MyMoneyInstitution i("Bank of the World","","","","","","");
  file->addInstitution(i);
  inBank = i.id();
  ft.commit();
}

void PivotTableTest::tearDown ()
{
  file->detachStorage(storage);
  delete storage;
}

void PivotTableTest::testNetWorthSingle()
{
  try
  {
    MyMoneyReport filter;
    filter.setRowType( MyMoneyReport::eAssetLiability );
    filter.setDateFilter(QDate(2004,1,1),QDate(2004,7,1).addDays(-1));
    XMLandback(filter);
    PivotTable networth_f(filter);
    writeTabletoCSV(networth_f);

    CPPUNIT_ASSERT(networth_f.m_grid["Asset"]["Checking Account"][acChecking][5]==moCheckingOpen);
    CPPUNIT_ASSERT(networth_f.m_grid["Asset"]["Checking Account"][acChecking][6]==moCheckingOpen);
    CPPUNIT_ASSERT(networth_f.m_grid["Asset"]["Checking Account"].m_total[5]==moCheckingOpen);
    CPPUNIT_ASSERT(networth_f.m_grid.m_total[0]==moZero);
    CPPUNIT_ASSERT(networth_f.m_grid.m_total[4]==moZero);
    CPPUNIT_ASSERT(networth_f.m_grid.m_total[5]==moCheckingOpen);
    CPPUNIT_ASSERT(networth_f.m_grid.m_total[6]==moCheckingOpen);
  }
  catch(MyMoneyException *e)
  {
    CPPUNIT_FAIL(e->what());
    delete e;
  }
}

void PivotTableTest::testNetWorthOfsetting()
{
  // Test the net worth report to make sure it picks up the opening balance for two
  // accounts opened during the period of the report, one asset & one liability.  Test
  // that it calculates the totals correctly.

  MyMoneyReport filter;
  filter.setRowType( MyMoneyReport::eAssetLiability );
  filter.setDateFilter(QDate(2004,1,1),QDate(2005,1,1).addDays(-1));
  XMLandback(filter);
  PivotTable networth_f( filter );
  CPPUNIT_ASSERT(networth_f.m_grid["Liability"]["Credit Card"][acCredit][7]==-moCreditOpen);
  CPPUNIT_ASSERT(networth_f.m_grid.m_total[0]==moZero);
  CPPUNIT_ASSERT(networth_f.m_grid.m_total[12]==moCheckingOpen+moCreditOpen);

}

void PivotTableTest::testNetWorthOpeningPrior()
{
  // Test the net worth report to make sure it's picking up opening balances PRIOR to
  // the period of the report.

  MyMoneyReport filter;
  filter.setRowType( MyMoneyReport::eAssetLiability );
  filter.setDateFilter(QDate(2005,8,1),QDate(2005,12,31));
  filter.setName("Net Worth Opening Prior 1");
  XMLandback(filter);
  PivotTable networth_f( filter );
  writeTabletoCSV(networth_f);

  CPPUNIT_ASSERT(networth_f.m_grid["Liability"]["Credit Card"].m_total[0]==-moCreditOpen);
  CPPUNIT_ASSERT(networth_f.m_grid["Asset"]["Checking Account"].m_total[0]==moCheckingOpen);
  CPPUNIT_ASSERT(networth_f.m_grid.m_total[0]==moCheckingOpen+moCreditOpen);
  CPPUNIT_ASSERT(networth_f.m_grid.m_total[1]==moCheckingOpen+moCreditOpen);

  // Test the net worth report to make sure that transactions prior to the report
  // period are included in the opening balance

  TransactionHelper t1( QDate(2004,11,7), MyMoneySplit::ActionWithdrawal, moParent1, acCredit, acParent );
  TransactionHelper t2( QDate(2004,11,7), MyMoneySplit::ActionWithdrawal, moParent2, acCredit, acParent );
  TransactionHelper t3( QDate(2004,11,7), MyMoneySplit::ActionWithdrawal, moChild, acChecking, acChild );

  filter.setName("Net Worth Opening Prior 2");
  PivotTable networth_f2( filter );
  writeTabletoCSV(networth_f2);
  CPPUNIT_ASSERT(networth_f2.m_grid["Liability"]["Credit Card"].m_total[0]==-moCreditOpen+moParent);
  CPPUNIT_ASSERT(networth_f2.m_grid["Asset"]["Checking Account"].m_total[0]==moCheckingOpen-moChild);
  CPPUNIT_ASSERT(networth_f2.m_grid.m_total[0]==moCheckingOpen+moCreditOpen-moChild-moParent);
}

void PivotTableTest::testNetWorthDateFilter()
{
  // Test a net worth report whose period is prior to the time any accounts are open,
  // so the report should be zero.

  MyMoneyReport filter;
  filter.setRowType( MyMoneyReport::eAssetLiability );
  filter.setDateFilter(QDate(2004,1,1),QDate(2004,2,1).addDays(-1));
  XMLandback(filter);
  PivotTable networth_f( filter );
  CPPUNIT_ASSERT(networth_f.m_grid.m_total[1]==moZero);

}

void PivotTableTest::testSpendingEmpty()
{
  // test a spending report with no entries

  MyMoneyReport filter;
  filter.setRowType( MyMoneyReport::eExpenseIncome );
  XMLandback(filter);
  PivotTable spending_f1( filter );
  CPPUNIT_ASSERT(spending_f1.m_grid.m_total.m_total==moZero);

  filter.setDateFilter(QDate(2004,9,1),QDate(2005,1,1).addDays(-1));
  PivotTable spending_f2( filter );
  CPPUNIT_ASSERT(spending_f2.m_grid.m_total.m_total==moZero);
}

void PivotTableTest::testSingleTransaction()
{
  // Test a single transaction
  TransactionHelper t( QDate(2004,10,31), MyMoneySplit::ActionWithdrawal,moSolo, acChecking, acSolo );

  MyMoneyReport filter;
  filter.setRowType( MyMoneyReport::eExpenseIncome );
  filter.setDateFilter(QDate(2004,9,1),QDate(2005,1,1).addDays(-1));
  filter.setName("Spending with Single Transaction.html");
  XMLandback(filter);
  PivotTable spending_f( filter );
  writeTabletoHTML(spending_f,"Spending with Single Transaction.html");

  CPPUNIT_ASSERT(spending_f.m_grid["Expense"]["Solo"][acSolo][2]==moSolo);
  CPPUNIT_ASSERT(spending_f.m_grid["Expense"]["Solo"].m_total[2]==moSolo);
  CPPUNIT_ASSERT(spending_f.m_grid["Expense"]["Solo"].m_total[1]==moZero);
  CPPUNIT_ASSERT(spending_f.m_grid.m_total[2]==(-moSolo));
  CPPUNIT_ASSERT(spending_f.m_grid.m_total.m_total==(-moSolo));

  filter.clear();
  filter.setRowType(MyMoneyReport::eAssetLiability);
  filter.setDateFilter(QDate(2004,9,1),QDate(2005,1,1).addDays(-1));
  XMLandback(filter);
  PivotTable networth_f( filter );
  CPPUNIT_ASSERT(networth_f.m_grid["Asset"]["Checking Account"].m_total[2]==(moCheckingOpen-moSolo) );
}

void PivotTableTest::testSubAccount()
{
  // Test a sub-account with a value, under an account with a value

  TransactionHelper t1( QDate(2004,11,7), MyMoneySplit::ActionWithdrawal, moParent1, acCredit, acParent );
  TransactionHelper t2( QDate(2004,11,7), MyMoneySplit::ActionWithdrawal, moParent2, acCredit, acParent );
  TransactionHelper t3( QDate(2004,11,7), MyMoneySplit::ActionWithdrawal, moChild, acCredit, acChild );

  MyMoneyReport filter;
  filter.setRowType( MyMoneyReport::eExpenseIncome );
  filter.setDateFilter(QDate(2004,9,1),QDate(2005,1,1).addDays(-1));
  filter.setShowSubAccounts(true);
  filter.setName("Spending with Sub-Account");
  XMLandback(filter);
  PivotTable spending_f( filter );
  writeTabletoHTML(spending_f,"Spending with Sub-Account.html");

  CPPUNIT_ASSERT(spending_f.m_grid["Expense"]["Parent"][acParent][3]==moParent);
  CPPUNIT_ASSERT(spending_f.m_grid["Expense"]["Parent"][acChild][3]==moChild);
  CPPUNIT_ASSERT(spending_f.m_grid["Expense"]["Parent"].m_total[3]==moParent+moChild);
  CPPUNIT_ASSERT(spending_f.m_grid["Expense"]["Parent"].m_total[2]==moZero);
  CPPUNIT_ASSERT(spending_f.m_grid["Expense"]["Parent"].m_total.m_total==moParent+moChild);
  CPPUNIT_ASSERT(spending_f.m_grid.m_total[3]==(-moParent-moChild));
  CPPUNIT_ASSERT(spending_f.m_grid.m_total.m_total==(-moParent-moChild));

  filter.clear();
  filter.setRowType(MyMoneyReport::eAssetLiability);
  filter.setDateFilter(QDate(2004,9,1),QDate(2005,1,1).addDays(-1));
  filter.setName("Net Worth with Sub-Account");
  XMLandback(filter);
  PivotTable networth_f( filter );
  writeTabletoHTML(networth_f,"Net Worth with Sub-Account.html");
  CPPUNIT_ASSERT(networth_f.m_grid["Liability"]["Credit Card"].m_total[3]==moParent+moChild-moCreditOpen );
  CPPUNIT_ASSERT(networth_f.m_grid.m_total[4] == -moParent-moChild+moCreditOpen+moCheckingOpen );

}

void PivotTableTest::testFilterIEvsIE()
{
  // Test that removing an income/spending account will remove the entry from an income/spending report
  TransactionHelper t1( QDate(2004,10,31), MyMoneySplit::ActionWithdrawal, moSolo, acChecking, acSolo );
  TransactionHelper t2( QDate(2004,11,7), MyMoneySplit::ActionWithdrawal, moParent1, acCredit, acParent );
  TransactionHelper t3( QDate(2004,11,7), MyMoneySplit::ActionWithdrawal, moChild, acCredit, acChild );

  MyMoneyReport filter;
  filter.setRowType( MyMoneyReport::eExpenseIncome );
  filter.setDateFilter(QDate(2004,9,1),QDate(2005,1,1).addDays(-1));
  filter.addCategory(acChild);
  filter.addCategory(acSolo);
  XMLandback(filter);
  PivotTable spending_f( filter );

  CPPUNIT_ASSERT(spending_f.m_grid["Expense"]["Parent"].m_total[3]==moChild);
  CPPUNIT_ASSERT(spending_f.m_grid["Expense"].m_total[2]==moSolo);
  CPPUNIT_ASSERT(spending_f.m_grid.m_total.m_total==-moSolo-moChild);

}

void PivotTableTest::testFilterALvsAL()
{
  // Test that removing an asset/liability account will remove the entry from an asset/liability report
  TransactionHelper t1( QDate(2004,10,31), MyMoneySplit::ActionWithdrawal, moSolo, acChecking, acSolo );
  TransactionHelper t2( QDate(2004,11,7), MyMoneySplit::ActionWithdrawal, moParent1, acCredit, acParent );
  TransactionHelper t3( QDate(2004,11,7), MyMoneySplit::ActionWithdrawal, moChild, acCredit, acChild );

  MyMoneyReport filter;
  filter.setRowType( MyMoneyReport::eAssetLiability );
  filter.setDateFilter(QDate(2004,9,1),QDate(2005,1,1).addDays(-1));
  filter.addAccount(acChecking);
  filter.addCategory(acChild);
  filter.addCategory(acSolo);
  XMLandback(filter);
  PivotTable networth_f( filter );
  CPPUNIT_ASSERT(networth_f.m_grid.m_total[3] == -moSolo+moCheckingOpen );
}

void PivotTableTest::testFilterALvsIE()
{
  // Test that removing an asset/liability account will remove the entry from an income/spending report
  TransactionHelper t1( QDate(2004,10,31), MyMoneySplit::ActionWithdrawal, moSolo, acChecking, acSolo );
  TransactionHelper t2( QDate(2004,11,7), MyMoneySplit::ActionWithdrawal, moParent1, acCredit, acParent );
  TransactionHelper t3( QDate(2004,11,7), MyMoneySplit::ActionWithdrawal, moChild, acCredit, acChild );

  MyMoneyReport filter;
  filter.setRowType( MyMoneyReport::eExpenseIncome );
  filter.setDateFilter(QDate(2004,9,1),QDate(2005,1,1).addDays(-1));
  filter.addAccount(acChecking);
  CPPUNIT_ASSERT(file->transactionList(filter).count() == 1);

  XMLandback(filter);
  PivotTable spending_f( filter );

  CPPUNIT_ASSERT(spending_f.m_grid["Expense"].m_total[3]==moZero);
  CPPUNIT_ASSERT(spending_f.m_grid["Expense"].m_total[2]==moSolo);
  CPPUNIT_ASSERT(spending_f.m_grid.m_total.m_total==-moSolo);
}

void PivotTableTest::testFilterAllvsIE()
{
  // Test that removing an asset/liability account AND an income/expense
  // category will remove the entry from an income/spending report
  TransactionHelper t1( QDate(2004,10,31), MyMoneySplit::ActionWithdrawal, moSolo, acChecking, acSolo );
  TransactionHelper t2( QDate(2004,11,7), MyMoneySplit::ActionWithdrawal, moParent1, acCredit, acParent );
  TransactionHelper t3( QDate(2004,11,7), MyMoneySplit::ActionWithdrawal, moChild, acCredit, acChild );

  MyMoneyReport filter;
  filter.setRowType( MyMoneyReport::eExpenseIncome );
  filter.setDateFilter(QDate(2004,9,1),QDate(2005,1,1).addDays(-1));
  filter.addAccount(acCredit);
  filter.addCategory(acChild);
  PivotTable spending_f( filter );

  CPPUNIT_ASSERT(spending_f.m_grid["Expense"].m_total[2]==moZero);
  CPPUNIT_ASSERT(spending_f.m_grid["Expense"].m_total[3]==moChild);
  CPPUNIT_ASSERT(spending_f.m_grid.m_total.m_total==-moChild);
}

void PivotTableTest::testFilterBasics()
{
  // Test that the filters are operating the way that the reports expect them to
  TransactionHelper t1( QDate(2004,10,31), MyMoneySplit::ActionWithdrawal, moSolo, acChecking, acSolo );
  TransactionHelper t2( QDate(2004,11,7), MyMoneySplit::ActionWithdrawal, moParent1, acCredit, acParent );
  TransactionHelper t3( QDate(2004,11,7), MyMoneySplit::ActionWithdrawal, moParent2, acCredit, acParent );
  TransactionHelper t4( QDate(2004,11,7), MyMoneySplit::ActionWithdrawal, moChild, acCredit, acChild );

  MyMoneyTransactionFilter filter;
  filter.clear();
  filter.setDateFilter(QDate(2004,9,1),QDate(2005,1,1).addDays(-1));
  filter.addCategory(acSolo);
  filter.setReportAllSplits(false);
  filter.setConsiderCategory(true);

  CPPUNIT_ASSERT(file->transactionList(filter).count() == 1);

  filter.addCategory(acParent);

  CPPUNIT_ASSERT(file->transactionList(filter).count() == 3);

  filter.addAccount(acChecking);

  CPPUNIT_ASSERT(file->transactionList(filter).count() == 1);

  filter.clear();
  filter.setDateFilter(QDate(2004,9,1),QDate(2005,1,1).addDays(-1));
  filter.addCategory(acParent);
  filter.addAccount(acCredit);
  filter.setReportAllSplits(false);
  filter.setConsiderCategory(true);

  CPPUNIT_ASSERT(file->transactionList(filter).count() == 2);
}

void PivotTableTest::testMultipleCurrencies()
{
  MyMoneyMoney moCanOpening( 0.0 );
  MyMoneyMoney moJpyOpening( 0.0 );
  MyMoneyMoney moCanPrice( 0.75 );
  MyMoneyMoney moJpyPrice( 0.010 );
  MyMoneyMoney moJpyPrice2( 0.011 );
  MyMoneyMoney moJpyPrice3( 0.014 );
  MyMoneyMoney moJpyPrice4( 0.0395 );
  MyMoneyMoney moCanTransaction( 100.0 );
  MyMoneyMoney moJpyTransaction( 100.0 );

  QCString acCanChecking = makeAccount(QString("Canadian Checking"),MyMoneyAccount::Checkings,moCanOpening,QDate(2003,11,15),acAsset,"CAD");
  QCString acJpyChecking = makeAccount(QString("Japanese Checking"),MyMoneyAccount::Checkings,moJpyOpening,QDate(2003,11,15),acAsset,"JPY");
  QCString acCanCash = makeAccount(QString("Canadian"),MyMoneyAccount::Expense,0,QDate(2004,2,11),acForeign,"CAD");
  QCString acJpyCash = makeAccount(QString("Japanese"),MyMoneyAccount::Expense,0,QDate(2004,2,11),acForeign,"JPY");

  makePrice("CAD",QDate(2004,1,1),MyMoneyMoney(moCanPrice));
  makePrice("JPY",QDate(2004,1,1),MyMoneyMoney(moJpyPrice));
  makePrice("JPY",QDate(2004,5,1),MyMoneyMoney(moJpyPrice2));
  makePrice("JPY",QDate(2004,6,30),MyMoneyMoney(moJpyPrice3));
  makePrice("JPY",QDate(2004,7,15),MyMoneyMoney(moJpyPrice4));

  TransactionHelper t1( QDate(2004,2,20), MyMoneySplit::ActionWithdrawal,MyMoneyMoney(moJpyTransaction), acJpyChecking, acJpyCash, "JPY" );
  TransactionHelper t2( QDate(2004,3,20), MyMoneySplit::ActionWithdrawal,MyMoneyMoney(moJpyTransaction), acJpyChecking, acJpyCash, "JPY" );
  TransactionHelper t3( QDate(2004,4,20), MyMoneySplit::ActionWithdrawal,MyMoneyMoney(moJpyTransaction), acJpyChecking, acJpyCash, "JPY" );
  TransactionHelper t4( QDate(2004,2,20), MyMoneySplit::ActionWithdrawal,MyMoneyMoney(moCanTransaction), acCanChecking, acCanCash, "CAD" );
  TransactionHelper t5( QDate(2004,3,20), MyMoneySplit::ActionWithdrawal,MyMoneyMoney(moCanTransaction), acCanChecking, acCanCash, "CAD" );
  TransactionHelper t6( QDate(2004,4,20), MyMoneySplit::ActionWithdrawal,MyMoneyMoney(moCanTransaction), acCanChecking, acCanCash, "CAD" );

#if 0
  QFile g( "multicurrencykmy.xml" );
  g.open( IO_WriteOnly );
  MyMoneyStorageXML xml;
  IMyMoneyStorageFormat& interface = xml;
  interface.writeFile(&g, dynamic_cast<IMyMoneySerialize*> (MyMoneyFile::instance()->storage()));
  g.close();
#endif

  MyMoneyReport filter;
  filter.setRowType( MyMoneyReport::eExpenseIncome );
  filter.setDateFilter(QDate(2004,1,1),QDate(2005,1,1).addDays(-1));
  filter.setShowSubAccounts(true);
  filter.setConvertCurrency(true);
  filter.setName("Multiple Currency Spending Rerport (with currency conversion)");
  XMLandback(filter);

  PivotTable spending_f( filter );

  writeTabletoCSV(spending_f);

  // test single foreign currency
  CPPUNIT_ASSERT(spending_f.m_grid["Expense"]["Foreign"][acCanCash][2]==(moCanTransaction*moCanPrice));
  CPPUNIT_ASSERT(spending_f.m_grid["Expense"]["Foreign"][acCanCash][3]==(moCanTransaction*moCanPrice));
  CPPUNIT_ASSERT(spending_f.m_grid["Expense"]["Foreign"][acCanCash][4]==(moCanTransaction*moCanPrice));

  // test multiple foreign currencies under a common parent
  CPPUNIT_ASSERT(spending_f.m_grid["Expense"]["Foreign"][acJpyCash][2]==(moJpyTransaction*moJpyPrice));
  CPPUNIT_ASSERT(spending_f.m_grid["Expense"]["Foreign"][acJpyCash][3]==(moJpyTransaction*moJpyPrice));
  CPPUNIT_ASSERT(spending_f.m_grid["Expense"]["Foreign"][acJpyCash][4]==(moJpyTransaction*moJpyPrice));
  CPPUNIT_ASSERT(spending_f.m_grid["Expense"]["Foreign"].m_total[2]==(moJpyTransaction*moJpyPrice + moCanTransaction*moCanPrice));
  CPPUNIT_ASSERT(spending_f.m_grid["Expense"]["Foreign"].m_total.m_total==(moJpyTransaction*moJpyPrice + moCanTransaction*moCanPrice + moJpyTransaction*moJpyPrice + moCanTransaction*moCanPrice + moJpyTransaction*moJpyPrice + moCanTransaction*moCanPrice));

  // Test the report type where we DO NOT convert the currency
  filter.setConvertCurrency(false);
  filter.setShowSubAccounts(true);
  filter.setName("Multiple Currency Spending Report (WITHOUT currency conversion)");
  XMLandback(filter);
  PivotTable spending_fnc( filter );
  writeTabletoCSV(spending_fnc);

  CPPUNIT_ASSERT(spending_fnc.m_grid["Expense"]["Foreign"][acCanCash][2]==(moCanTransaction));
  CPPUNIT_ASSERT(spending_fnc.m_grid["Expense"]["Foreign"][acCanCash][3]==(moCanTransaction));
  CPPUNIT_ASSERT(spending_fnc.m_grid["Expense"]["Foreign"][acCanCash][4]==(moCanTransaction));
  CPPUNIT_ASSERT(spending_fnc.m_grid["Expense"]["Foreign"][acJpyCash][2]==(moJpyTransaction));
  CPPUNIT_ASSERT(spending_fnc.m_grid["Expense"]["Foreign"][acJpyCash][3]==(moJpyTransaction));
  CPPUNIT_ASSERT(spending_fnc.m_grid["Expense"]["Foreign"][acJpyCash][4]==(moJpyTransaction));

  filter.setConvertCurrency(true);
  filter.clear();
  filter.setName("Multiple currency net worth");
  filter.setRowType(MyMoneyReport::eAssetLiability);
  filter.setDateFilter(QDate(2004,1,1),QDate(2005,1,1).addDays(-1));
  XMLandback(filter);
  PivotTable networth_f( filter );
  writeTabletoCSV(networth_f);

  // test single foreign currency
  CPPUNIT_ASSERT(networth_f.m_grid["Asset"]["Canadian Checking"][acCanChecking][1]==(moCanOpening*moCanPrice));
  CPPUNIT_ASSERT(networth_f.m_grid["Asset"]["Canadian Checking"][acCanChecking][2]==((moCanOpening-moCanTransaction)*moCanPrice));
  CPPUNIT_ASSERT(networth_f.m_grid["Asset"]["Canadian Checking"][acCanChecking][3]==((moCanOpening-moCanTransaction-moCanTransaction)*moCanPrice));
  CPPUNIT_ASSERT(networth_f.m_grid["Asset"]["Canadian Checking"][acCanChecking][4]==((moCanOpening-moCanTransaction-moCanTransaction-moCanTransaction)*moCanPrice));
  CPPUNIT_ASSERT(networth_f.m_grid["Asset"]["Canadian Checking"][acCanChecking][12]==((moCanOpening-moCanTransaction-moCanTransaction-moCanTransaction)*moCanPrice));

  // test Stable currency price, fluctuating account balance
  CPPUNIT_ASSERT(networth_f.m_grid["Asset"]["Japanese Checking"][acJpyChecking][1]==(moJpyOpening*moJpyPrice));
  CPPUNIT_ASSERT(networth_f.m_grid["Asset"]["Japanese Checking"][acJpyChecking][2]==((moJpyOpening-moJpyTransaction)*moJpyPrice));
  CPPUNIT_ASSERT(networth_f.m_grid["Asset"]["Japanese Checking"][acJpyChecking][3]==((moJpyOpening-moJpyTransaction-moJpyTransaction)*moJpyPrice));
  CPPUNIT_ASSERT(networth_f.m_grid["Asset"]["Japanese Checking"][acJpyChecking][4]==((moJpyOpening-moJpyTransaction-moJpyTransaction-moJpyTransaction)*moJpyPrice));

  // test Fluctuating currency price, stable account balance
  CPPUNIT_ASSERT(networth_f.m_grid["Asset"]["Japanese Checking"][acJpyChecking][5]==((moJpyOpening-moJpyTransaction-moJpyTransaction-moJpyTransaction)*moJpyPrice2));
  CPPUNIT_ASSERT(networth_f.m_grid["Asset"]["Japanese Checking"][acJpyChecking][6]==((moJpyOpening-moJpyTransaction-moJpyTransaction-moJpyTransaction)*moJpyPrice3));
  CPPUNIT_ASSERT(networth_f.m_grid["Asset"]["Japanese Checking"][acJpyChecking][7]==((moJpyOpening-moJpyTransaction-moJpyTransaction-moJpyTransaction)*moJpyPrice4));

  // test multiple currencies totalled up
  CPPUNIT_ASSERT(networth_f.m_grid["Asset"].m_total[4]==((moCanOpening-moCanTransaction-moCanTransaction-moCanTransaction)*moCanPrice)+((moJpyOpening-moJpyTransaction-moJpyTransaction-moJpyTransaction)*moJpyPrice));
  CPPUNIT_ASSERT(networth_f.m_grid["Asset"].m_total[5]==((moCanOpening-moCanTransaction-moCanTransaction-moCanTransaction)*moCanPrice)+((moJpyOpening-moJpyTransaction-moJpyTransaction-moJpyTransaction)*moJpyPrice2)+moCheckingOpen);

}

void PivotTableTest::testAdvancedFilter()
{
  // test more advanced filtering capabilities

  // amount
  {
    TransactionHelper t1( QDate(2004,11,7), MyMoneySplit::ActionWithdrawal, moParent1, acCredit, acParent );
    TransactionHelper t2( QDate(2004,11,7), MyMoneySplit::ActionWithdrawal, moChild, acCredit, acChild );

    MyMoneyReport filter;
    filter.setRowType( MyMoneyReport::eExpenseIncome );
    filter.setDateFilter(QDate(2004,1,1),QDate(2005,1,1).addDays(-1));
    filter.setAmountFilter(moChild,moChild);
    XMLandback(filter);
    PivotTable spending_f( filter );
    CPPUNIT_ASSERT(spending_f.m_grid.m_total.m_total==-moChild);
  }

  // payee (specific)
  {
    TransactionHelper t1( QDate(2004,10,31), MyMoneySplit::ActionWithdrawal, moSolo, acChecking, acSolo );
    TransactionHelper t2( QDate(2004,11,7), MyMoneySplit::ActionWithdrawal, moParent1, acCredit, acParent );
    TransactionHelper t3( QDate(2004,11,7), MyMoneySplit::ActionWithdrawal, moChild, acCredit, acChild );
    TransactionHelper t4( QDate(2004,11,7), MyMoneySplit::ActionWithdrawal, moThomas, acCredit, acParent, QCString(), "Thomas Baumgart" );

    MyMoneyReport filter;
    filter.setRowType( MyMoneyReport::eExpenseIncome );
    filter.setDateFilter(QDate(2004,1,1),QDate(2005,1,1).addDays(-1));
    filter.addPayee(MyMoneyFile::instance()->payeeByName("Thomas Baumgart").id());
    filter.setName("Spending with Payee Filter");
    XMLandback(filter);
    PivotTable spending_f( filter );
    writeTabletoHTML(spending_f,"Spending with Payee Filter.html");

    CPPUNIT_ASSERT(spending_f.m_grid["Expense"]["Parent"][acParent][11]==moThomas);
    CPPUNIT_ASSERT(spending_f.m_grid.m_total.m_total==-moThomas);
  }
  // payee (no payee)
  {
    TransactionHelper t1( QDate(2004,10,31), MyMoneySplit::ActionWithdrawal, moSolo, acChecking, acSolo );
    TransactionHelper t2( QDate(2004,11,7), MyMoneySplit::ActionWithdrawal, moParent1, acCredit, acParent );
    TransactionHelper t3( QDate(2004,11,7), MyMoneySplit::ActionWithdrawal, moChild, acCredit, acChild );
    TransactionHelper t4( QDate(2004,11,7), MyMoneySplit::ActionWithdrawal, moNoPayee, acCredit, acParent, QCString(), QString() );

    MyMoneyReport filter;
    filter.setRowType( MyMoneyReport::eExpenseIncome );
    filter.setDateFilter(QDate(2004,1,1),QDate(2005,1,1).addDays(-1));
    filter.addPayee(QCString());
    XMLandback(filter);
    PivotTable spending_f( filter );
    CPPUNIT_ASSERT(spending_f.m_grid["Expense"]["Parent"][acParent][11]==moNoPayee);
    CPPUNIT_ASSERT(spending_f.m_grid.m_total.m_total==-moNoPayee);
  }

  // text
  {
    TransactionHelper t1( QDate(2004,10,31), MyMoneySplit::ActionWithdrawal, moSolo, acChecking, acSolo );
    TransactionHelper t2( QDate(2004,11,7), MyMoneySplit::ActionWithdrawal, moParent1, acCredit, acParent );
    TransactionHelper t3( QDate(2004,11,7), MyMoneySplit::ActionWithdrawal, moChild, acCredit, acChild );
    TransactionHelper t4( QDate(2004,11,7), MyMoneySplit::ActionWithdrawal, moThomas, acCredit, acParent, QCString(), "Thomas Baumgart" );

    MyMoneyReport filter;
    filter.setRowType( MyMoneyReport::eExpenseIncome );
    filter.setDateFilter(QDate(2004,1,1),QDate(2005,1,1).addDays(-1));
    filter.setTextFilter(QRegExp("Thomas"));
    XMLandback(filter);
    PivotTable spending_f( filter );
  }

  // type (payment, deposit, transfer)
  {
    TransactionHelper t1( QDate(2004,1,1), MyMoneySplit::ActionWithdrawal, moSolo, acChecking, acSolo );
    TransactionHelper t2( QDate(2004,2,1), MyMoneySplit::ActionDeposit, -moParent1, acCredit, acParent );
    TransactionHelper t3( QDate(2004,11,1), MyMoneySplit::ActionTransfer, moChild, acCredit, acChecking );

    MyMoneyReport filter;
    filter.setRowType( MyMoneyReport::eExpenseIncome );
    filter.addType(MyMoneyTransactionFilter::payments);
    XMLandback(filter);
    PivotTable spending_f( filter );

    CPPUNIT_ASSERT(spending_f.m_grid.m_total.m_total == -moSolo);

    filter.clear();
    filter.addType(MyMoneyTransactionFilter::deposits);
    XMLandback(filter);
    PivotTable spending_f2( filter );

    CPPUNIT_ASSERT(spending_f2.m_grid.m_total.m_total == moParent1);

    filter.clear();
    filter.addType(MyMoneyTransactionFilter::transfers);
    XMLandback(filter);
    PivotTable spending_f3( filter );

    CPPUNIT_ASSERT(spending_f3.m_grid.m_total.m_total == moZero);

    filter.setRowType(MyMoneyReport::eAssetLiability);
    filter.setDateFilter( QDate(2004,1,1), QDate(2004,12,31) );
    XMLandback(filter);
    PivotTable networth_f4( filter );

    CPPUNIT_ASSERT(networth_f4.m_grid["Asset"].m_total[11] == moCheckingOpen + moChild);
    CPPUNIT_ASSERT(networth_f4.m_grid["Liability"].m_total[11] == - moCreditOpen + moChild);
    CPPUNIT_ASSERT(networth_f4.m_grid.m_total[10] == moCheckingOpen + moCreditOpen);
    CPPUNIT_ASSERT(networth_f4.m_grid.m_total[11] == moCheckingOpen + moCreditOpen);
  }

  // state (reconciled, cleared, not)
  {
    TransactionHelper t1( QDate(2004,1,1), MyMoneySplit::ActionWithdrawal, moSolo, acChecking, acSolo );
    TransactionHelper t2( QDate(2004,2,1), MyMoneySplit::ActionWithdrawal, moParent1, acCredit, acParent );
    TransactionHelper t3( QDate(2004,3,1), MyMoneySplit::ActionWithdrawal, moParent2, acCredit, acParent );
    TransactionHelper t4( QDate(2004,4,1), MyMoneySplit::ActionWithdrawal, moChild, acCredit, acChild );

    QValueList<MyMoneySplit> splits = t1.splits();
    splits[0].setReconcileFlag(MyMoneySplit::Cleared);
    splits[1].setReconcileFlag(MyMoneySplit::Cleared);
    t1.modifySplit(splits[0]);
    t1.modifySplit(splits[1]);
    t1.update();

    splits.clear();
    splits = t2.splits();
    splits[0].setReconcileFlag(MyMoneySplit::Reconciled);
    splits[1].setReconcileFlag(MyMoneySplit::Reconciled);
    t2.modifySplit(splits[0]);
    t2.modifySplit(splits[1]);
    t2.update();

    MyMoneyReport filter;
    filter.setRowType( MyMoneyReport::eExpenseIncome );
    filter.setDateFilter(QDate(2004,1,1),QDate(2005,1,1).addDays(-1));
    filter.addState(MyMoneyTransactionFilter::cleared);
    XMLandback(filter);
    PivotTable spending_f( filter );

    CPPUNIT_ASSERT(spending_f.m_grid.m_total.m_total==-moSolo);

    filter.addState(MyMoneyTransactionFilter::reconciled);
    XMLandback(filter);
    PivotTable spending_f2( filter );

    CPPUNIT_ASSERT(spending_f2.m_grid.m_total.m_total==-moSolo-moParent1);

    filter.clear();
    filter.addState(MyMoneyTransactionFilter::notReconciled);
    XMLandback(filter);
    PivotTable spending_f3( filter );

    CPPUNIT_ASSERT(spending_f3.m_grid.m_total.m_total==-moChild-moParent2);
  }

  // number
  {
    TransactionHelper t1( QDate(2004,10,31), MyMoneySplit::ActionWithdrawal, moSolo, acChecking, acSolo );
    TransactionHelper t2( QDate(2004,11,7), MyMoneySplit::ActionWithdrawal, moParent1, acCredit, acParent );
    TransactionHelper t3( QDate(2004,11,7), MyMoneySplit::ActionWithdrawal, moParent2, acCredit, acParent );
    TransactionHelper t4( QDate(2004,11,7), MyMoneySplit::ActionWithdrawal, moChild, acCredit, acChild );

    QValueList<MyMoneySplit> splits = t1.splits();
    splits[0].setNumber("1");
    splits[1].setNumber("1");
    t1.modifySplit(splits[0]);
    t1.modifySplit(splits[1]);
    t1.update();

    splits.clear();
    splits = t2.splits();
    splits[0].setNumber("2");
    splits[1].setNumber("2");
    t2.modifySplit(splits[0]);
    t2.modifySplit(splits[1]);
    t2.update();

    splits.clear();
    splits = t3.splits();
    splits[0].setNumber("3");
    splits[1].setNumber("3");
    t3.modifySplit(splits[0]);
    t3.modifySplit(splits[1]);
    t3.update();

    splits.clear();
    splits = t2.splits();
    splits[0].setNumber("4");
    splits[1].setNumber("4");
    t4.modifySplit(splits[0]);
    t4.modifySplit(splits[1]);
    t4.update();

    MyMoneyReport filter;
    filter.setRowType( MyMoneyReport::eExpenseIncome );
    filter.setDateFilter(QDate(2004,1,1),QDate(2005,1,1).addDays(-1));
    filter.setNumberFilter("1","3");
    XMLandback(filter);
    PivotTable spending_f( filter );
    CPPUNIT_ASSERT(spending_f.m_grid.m_total.m_total==-moSolo-moParent1-moParent2);
  }

  // blank dates
  {
    TransactionHelper t1y1( QDate(2003,10,1), MyMoneySplit::ActionWithdrawal, moSolo, acChecking, acSolo );
    TransactionHelper t2y1( QDate(2003,11,1), MyMoneySplit::ActionWithdrawal, moParent1, acCredit, acParent );
    TransactionHelper t3y1( QDate(2003,12,1), MyMoneySplit::ActionWithdrawal, moParent2, acCredit, acParent );

    TransactionHelper t1y2( QDate(2004,4,1), MyMoneySplit::ActionWithdrawal, moSolo, acChecking, acSolo );
    TransactionHelper t2y2( QDate(2004,5,1), MyMoneySplit::ActionWithdrawal, moParent1, acCredit, acParent );
    TransactionHelper t3y2( QDate(2004,6,1), MyMoneySplit::ActionWithdrawal, moParent2, acCredit, acParent );

    TransactionHelper t1y3( QDate(2005,1,1), MyMoneySplit::ActionWithdrawal, moSolo, acChecking, acSolo );
    TransactionHelper t2y3( QDate(2005,5,1), MyMoneySplit::ActionWithdrawal, moParent1, acCredit, acParent );
    TransactionHelper t3y3( QDate(2005,9,1), MyMoneySplit::ActionWithdrawal, moParent2, acCredit, acParent );

    MyMoneyReport filter;
    filter.setRowType( MyMoneyReport::eExpenseIncome );
    filter.setDateFilter(QDate(),QDate(2004,7,1));
    XMLandback(filter);
    PivotTable spending_f( filter );
    CPPUNIT_ASSERT(spending_f.m_grid.m_total.m_total==-moSolo-moParent1-moParent2-moSolo-moParent1-moParent2);

    filter.clear();
    XMLandback(filter);
    PivotTable spending_f2( filter );
    CPPUNIT_ASSERT(spending_f2.m_grid.m_total.m_total==-moSolo-moParent1-moParent2-moSolo-moParent1-moParent2-moSolo-moParent1-moParent2);

  }

}

void PivotTableTest::testColumnType()
{
  // test column type values of other than 'month'

  TransactionHelper t1q1( QDate(2004,1,1), MyMoneySplit::ActionWithdrawal, moSolo, acChecking, acSolo );
  TransactionHelper t2q1( QDate(2004,2,1), MyMoneySplit::ActionWithdrawal, moParent1, acCredit, acParent );
  TransactionHelper t3q1( QDate(2004,3,1), MyMoneySplit::ActionWithdrawal, moParent2, acCredit, acParent );

  TransactionHelper t1q2( QDate(2004,4,1), MyMoneySplit::ActionWithdrawal, moSolo, acChecking, acSolo );
  TransactionHelper t2q2( QDate(2004,5,1), MyMoneySplit::ActionWithdrawal, moParent1, acCredit, acParent );
  TransactionHelper t3q2( QDate(2004,6,1), MyMoneySplit::ActionWithdrawal, moParent2, acCredit, acParent );

  TransactionHelper t1y2( QDate(2005,1,1), MyMoneySplit::ActionWithdrawal, moSolo, acChecking, acSolo );
  TransactionHelper t2y2( QDate(2005,5,1), MyMoneySplit::ActionWithdrawal, moParent1, acCredit, acParent );
  TransactionHelper t3y2( QDate(2005,9,1), MyMoneySplit::ActionWithdrawal, moParent2, acCredit, acParent );

  MyMoneyReport filter;
  filter.setRowType( MyMoneyReport::eExpenseIncome );
  filter.setDateFilter(QDate(2003,12,31),QDate(2005,12,31));
  filter.setRowType( MyMoneyReport::eExpenseIncome );
  filter.setColumnType(MyMoneyReport::eBiMonths);
  XMLandback(filter);
  PivotTable spending_b( filter );

  CPPUNIT_ASSERT(spending_b.m_grid.m_total[1] == moZero);
  CPPUNIT_ASSERT(spending_b.m_grid.m_total[2] == -moParent1-moSolo);
  CPPUNIT_ASSERT(spending_b.m_grid.m_total[3] == -moParent2-moSolo);
  CPPUNIT_ASSERT(spending_b.m_grid.m_total[4] == -moParent);
  CPPUNIT_ASSERT(spending_b.m_grid.m_total[5] == moZero);
  CPPUNIT_ASSERT(spending_b.m_grid.m_total[6] == moZero);
  CPPUNIT_ASSERT(spending_b.m_grid.m_total[7] == moZero);
  CPPUNIT_ASSERT(spending_b.m_grid.m_total[8] == -moSolo);
  CPPUNIT_ASSERT(spending_b.m_grid.m_total[9] == moZero);
  CPPUNIT_ASSERT(spending_b.m_grid.m_total[10] == -moParent1);
  CPPUNIT_ASSERT(spending_b.m_grid.m_total[11] == moZero);
  CPPUNIT_ASSERT(spending_b.m_grid.m_total[12] == -moParent2);
  CPPUNIT_ASSERT(spending_b.m_grid.m_total[13] == moZero);

  filter.setColumnType(MyMoneyReport::eQuarters);
  XMLandback(filter);
  PivotTable spending_q( filter );

  CPPUNIT_ASSERT(spending_q.m_grid.m_total[1] == moZero);
  CPPUNIT_ASSERT(spending_q.m_grid.m_total[2] == -moSolo-moParent);
  CPPUNIT_ASSERT(spending_q.m_grid.m_total[3] == -moSolo-moParent);
  CPPUNIT_ASSERT(spending_q.m_grid.m_total[4] == moZero);
  CPPUNIT_ASSERT(spending_q.m_grid.m_total[5] == moZero);
  CPPUNIT_ASSERT(spending_q.m_grid.m_total[6] == -moSolo);
  CPPUNIT_ASSERT(spending_q.m_grid.m_total[7] == -moParent1);
  CPPUNIT_ASSERT(spending_q.m_grid.m_total[8] == -moParent2);
  CPPUNIT_ASSERT(spending_q.m_grid.m_total[9] == moZero);

  filter.setRowType( MyMoneyReport::eAssetLiability );
  filter.setName( "Net Worth by Quarter" );
  XMLandback(filter);
  PivotTable networth_q( filter );
  writeTabletoHTML( networth_q, "Net Worth by Quarter.html" );

  CPPUNIT_ASSERT(networth_q.m_grid.m_total[1] == moZero);
  CPPUNIT_ASSERT(networth_q.m_grid.m_total[2] == -moSolo-moParent);
  CPPUNIT_ASSERT(networth_q.m_grid.m_total[3] == -moSolo-moParent-moSolo-moParent+moCheckingOpen);
  CPPUNIT_ASSERT(networth_q.m_grid.m_total[4] == -moSolo-moParent-moSolo-moParent+moCheckingOpen+moCreditOpen);
  CPPUNIT_ASSERT(networth_q.m_grid.m_total[5] == -moSolo-moParent-moSolo-moParent+moCheckingOpen+moCreditOpen);
  CPPUNIT_ASSERT(networth_q.m_grid.m_total[6] == -moSolo-moSolo-moParent-moSolo-moParent+moCheckingOpen+moCreditOpen);
  CPPUNIT_ASSERT(networth_q.m_grid.m_total[7] == -moParent1-moSolo-moSolo-moParent-moSolo-moParent+moCheckingOpen+moCreditOpen);
  CPPUNIT_ASSERT(networth_q.m_grid.m_total[8] == -moParent2-moParent1-moSolo-moSolo-moParent-moSolo-moParent+moCheckingOpen+moCreditOpen);
  CPPUNIT_ASSERT(networth_q.m_grid.m_total[9] == -moParent2-moParent1-moSolo-moSolo-moParent-moSolo-moParent+moCheckingOpen+moCreditOpen);

  filter.setRowType( MyMoneyReport::eExpenseIncome );
  filter.setColumnType(MyMoneyReport::eYears);
  XMLandback(filter);
  PivotTable spending_y( filter );

  CPPUNIT_ASSERT(spending_y.m_grid.m_total[1] == moZero);
  CPPUNIT_ASSERT(spending_y.m_grid.m_total[2] == -moSolo-moParent-moSolo-moParent);
  CPPUNIT_ASSERT(spending_y.m_grid.m_total[3] == -moSolo-moParent);
  CPPUNIT_ASSERT(spending_y.m_grid.m_total.m_total == -moSolo-moParent-moSolo-moParent-moSolo-moParent);

  filter.setRowType( MyMoneyReport::eAssetLiability );
  XMLandback(filter);
  PivotTable networth_y( filter );

  CPPUNIT_ASSERT(networth_y.m_grid.m_total[1] == moZero);
  CPPUNIT_ASSERT(networth_y.m_grid.m_total[2] == -moSolo-moParent-moSolo-moParent+moCheckingOpen+moCreditOpen);
  CPPUNIT_ASSERT(networth_y.m_grid.m_total[3] == -moSolo-moParent-moSolo-moParent-moSolo-moParent+moCheckingOpen+moCreditOpen);

  // Test days-based reports

  TransactionHelper t1d1( QDate(2004,7,1), MyMoneySplit::ActionWithdrawal, moSolo, acChecking, acSolo );
  TransactionHelper t2d1( QDate(2004,7,1), MyMoneySplit::ActionWithdrawal, moParent1, acCredit, acParent );
  TransactionHelper t3d1( QDate(2004,7,5), MyMoneySplit::ActionWithdrawal, moParent2, acCredit, acParent );

  TransactionHelper t1d2( QDate(2004,7,14), MyMoneySplit::ActionWithdrawal, moSolo, acChecking, acSolo );
  TransactionHelper t2d2( QDate(2004,7,15), MyMoneySplit::ActionWithdrawal, moParent1, acCredit, acParent );
  TransactionHelper t3d2( QDate(2004,7,20), MyMoneySplit::ActionWithdrawal, moParent2, acCredit, acParent );

  TransactionHelper t1d3( QDate(2004,8,2), MyMoneySplit::ActionWithdrawal, moSolo, acChecking, acSolo );
  TransactionHelper t2d3( QDate(2004,8,3), MyMoneySplit::ActionWithdrawal, moParent1, acCredit, acParent );
  TransactionHelper t3d3( QDate(2004,8,4), MyMoneySplit::ActionWithdrawal, moParent2, acCredit, acParent );

  filter.setDateFilter(QDate(2004,7,2),QDate(2004,7,14));
  filter.setRowType( MyMoneyReport::eExpenseIncome );
  filter.setColumnType(MyMoneyReport::eMonths);
  filter.setColumnsAreDays(true);

  XMLandback(filter);
  PivotTable spending_days( filter );
  writeTabletoHTML(spending_days,"Spending by Days.html");

  CPPUNIT_ASSERT(spending_days.m_grid.m_total[4] == -moParent2);
  CPPUNIT_ASSERT(spending_days.m_grid.m_total[13] == -moSolo);
  CPPUNIT_ASSERT(spending_days.m_grid.m_total.m_total == -moSolo-moParent2);

  unsigned save_dayweekstart = KGlobal::locale()->weekStartDay();
  KGlobal::locale()->setWeekStartDay(2);

  filter.setDateFilter(QDate(2004,7,2),QDate(2004,8,1));
  filter.setRowType( MyMoneyReport::eExpenseIncome );
  filter.setColumnType(static_cast<MyMoneyReport::EColumnType>(7));
  filter.setColumnsAreDays(true);

  XMLandback(filter);
  PivotTable spending_weeks( filter );
  writeTabletoHTML(spending_weeks,"Spending by Weeks.html");

  KGlobal::locale()->setWeekStartDay(save_dayweekstart);

  CPPUNIT_ASSERT(spending_weeks.m_grid.m_total[0] == moZero);
  CPPUNIT_ASSERT(spending_weeks.m_grid.m_total[1] == -moParent2);
  CPPUNIT_ASSERT(spending_weeks.m_grid.m_total[2] == moZero);
  CPPUNIT_ASSERT(spending_weeks.m_grid.m_total[3] == -moSolo-moParent1);
  CPPUNIT_ASSERT(spending_weeks.m_grid.m_total[4] == -moParent2);
  CPPUNIT_ASSERT(spending_weeks.m_grid.m_total[5] == moZero);
  CPPUNIT_ASSERT(spending_weeks.m_grid.m_total.m_total == -moSolo-moParent-moParent2);


}

void PivotTableTest::testInvestment(void)
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

  makeEquityPrice( eqStock1, QDate(2004,10,1), 100.00 );

  //
  // Net Worth Report (with investments)
  //

  MyMoneyReport networth_r;
  networth_r.setRowType( MyMoneyReport::eAssetLiability );
  networth_r.setDateFilter(QDate(2004,1,1),QDate(2004,12,31).addDays(-1));
  XMLandback(networth_r);
  PivotTable networth(networth_r);

  networth.dump("networth_i.html");

  CPPUNIT_ASSERT(networth.m_grid["Asset"]["Investment"].m_total[1]==moZero);
  // 1000 shares @ $100.00
  CPPUNIT_ASSERT(networth.m_grid["Asset"]["Investment"].m_total[2]==MyMoneyMoney(100000.0));
  // 2000 shares @ $110.00
  CPPUNIT_ASSERT(networth.m_grid["Asset"]["Investment"].m_total[3]==MyMoneyMoney(220000.0));
  // 1800 shares @ $120.00
  CPPUNIT_ASSERT(networth.m_grid["Asset"]["Investment"].m_total[4]==MyMoneyMoney(216000.0));
  // 1600 shares @ $100.00
  CPPUNIT_ASSERT(networth.m_grid["Asset"]["Investment"].m_total[5]==MyMoneyMoney(160000.0));
  // 1650 shares @ $100.00
  CPPUNIT_ASSERT(networth.m_grid["Asset"]["Investment"].m_total[6]==MyMoneyMoney(165000.0));
  // 1700 shares @ $ 80.00
  CPPUNIT_ASSERT(networth.m_grid["Asset"]["Investment"].m_total[7]==MyMoneyMoney(136000.0));
  // 1700 shares @ $100.00
  CPPUNIT_ASSERT(networth.m_grid["Asset"]["Investment"].m_total[8]==MyMoneyMoney(170000.0));
  // 1700 shares @ $120.00
  CPPUNIT_ASSERT(networth.m_grid["Asset"]["Investment"].m_total[9]==MyMoneyMoney(204000.0));
  // 1700 shares @ $100.00
  CPPUNIT_ASSERT(networth.m_grid["Asset"]["Investment"].m_total[10]==MyMoneyMoney(170000.0));

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

void PivotTableTest::testBudget(void)
{

  // 1. Budget on A, transations on A
  {
    BudgetHelper budget;
    budget += BudgetEntryHelper( QDate(2006,1,1), acSolo, false, 100.0 );

    MyMoneyReport report(MyMoneyReport::eBudgetActual,MyMoneyReport::eMonths,MyMoneyTransactionFilter::yearToDate,
      false,"Yearly Budgeted vs. Actual","Default Report");
    PivotTable table(report);
  }

  // 2. Budget on B, not applying to sub accounts, transactions on B and B:1
  {
    BudgetHelper budget;
    budget += BudgetEntryHelper( QDate(2006,1,1), acParent, false, 100.0 );
    MyMoneyReport report(MyMoneyReport::eBudgetActual,MyMoneyReport::eMonths,MyMoneyTransactionFilter::yearToDate,
      false,"Yearly Budgeted vs. Actual","Default Report");
    PivotTable table(report);
  }

  //  - Both B and B:1 totals should show up
  //  - B actuals compare against B budget
  //  - B:1 actuals compare against 0

  // 3. Budget on C, applying to sub accounts, transactions on C and C:1 and C:1:a
  {
    BudgetHelper budget;
    budget += BudgetEntryHelper( QDate(2006,1,1), acParent, true, 100.0 );
    MyMoneyReport report(MyMoneyReport::eBudgetActual,MyMoneyReport::eMonths,MyMoneyTransactionFilter::yearToDate,
      false,"Yearly Budgeted vs. Actual","Default Report");
    PivotTable table(report);
  }

  //  - Only C totals show up, not C:1 or C:1:a totals
  //  - C + C:1 totals compare against C budget

  // 4. Budget on D, not applying to sub accounts, budget on D:1 not applying, budget on D:2 applying.  Transactions on D, D:1, D:2, D:2:a, D:2:b
  {
    BudgetHelper budget;
    budget += BudgetEntryHelper( QDate(2006,1,1), acParent, false, 100.0 );
    budget += BudgetEntryHelper( QDate(2006,1,1), acChild, false, 100.0 );
    budget += BudgetEntryHelper( QDate(2006,1,1), acSecondChild, true, 100.0 );
    MyMoneyReport report(MyMoneyReport::eBudgetActual,MyMoneyReport::eMonths,MyMoneyTransactionFilter::yearToDate,
      false,"Yearly Budgeted vs. Actual","Default Report");
    PivotTable table(report);
  }

  //  - Totals for D, D:1, D:2 show up.  D:2:a and D:2:b do not
  //  - D actuals (only) compare against D budget
  //  - Ditto for D:1
  //  - D:2 acutals and children compare against D:2 budget

  // 5. Budget on E, no transactions on E
  {
    BudgetHelper budget;
    budget += BudgetEntryHelper( QDate(2006,1,1), acSolo, false, 100.0 );
    MyMoneyReport report(MyMoneyReport::eBudgetActual,MyMoneyReport::eMonths,MyMoneyTransactionFilter::yearToDate,
      false,"Yearly Budgeted vs. Actual","Default Report");
    PivotTable table(report);
  }
}

void PivotTableTest::testCellAddValue(void)
{
  PivotTable::TCell a;
  CPPUNIT_ASSERT(a == MyMoneyMoney(0,1));
  CPPUNIT_ASSERT(a.m_stockSplit == MyMoneyMoney(1,1));
  CPPUNIT_ASSERT(a.m_postSplit == MyMoneyMoney(0,1));
  CPPUNIT_ASSERT(a.formatMoney() == MyMoneyMoney(0,1).formatMoney());

  PivotTable::TCell b(MyMoneyMoney(13,10));
  CPPUNIT_ASSERT(b == MyMoneyMoney(13,10));
  CPPUNIT_ASSERT(b.m_stockSplit == MyMoneyMoney(1,1));
  CPPUNIT_ASSERT(b.m_postSplit == MyMoneyMoney(0,1));
  CPPUNIT_ASSERT(b.formatMoney() == MyMoneyMoney(13,10).formatMoney());

  PivotTable::TCell s(b);
  CPPUNIT_ASSERT(s == MyMoneyMoney(13,10));
  CPPUNIT_ASSERT(s.m_stockSplit == MyMoneyMoney(1,1));
  CPPUNIT_ASSERT(s.m_postSplit == MyMoneyMoney(0,1));
  CPPUNIT_ASSERT(s.formatMoney() == MyMoneyMoney(13,10).formatMoney());

  s = PivotTable::TCell::stockSplit(MyMoneyMoney(1,2));
  CPPUNIT_ASSERT(s == MyMoneyMoney(0,1));
  CPPUNIT_ASSERT(s.m_stockSplit == MyMoneyMoney(1,2));
  CPPUNIT_ASSERT(s.m_postSplit == MyMoneyMoney(0,1));
  CPPUNIT_ASSERT(s.formatMoney() == MyMoneyMoney(0,1).formatMoney());
  
  a += MyMoneyMoney(1,1);
  a += MyMoneyMoney(2,1);
  CPPUNIT_ASSERT(a == MyMoneyMoney(3,1));
  CPPUNIT_ASSERT(a.m_stockSplit == MyMoneyMoney(1,1));
  CPPUNIT_ASSERT(a.m_postSplit == MyMoneyMoney(0,1));
  CPPUNIT_ASSERT(a.formatMoney() == MyMoneyMoney(3,1).formatMoney());

  a += s;
  CPPUNIT_ASSERT(a == MyMoneyMoney(3,1));
  CPPUNIT_ASSERT(a.m_stockSplit == MyMoneyMoney(1,2));
  CPPUNIT_ASSERT(a.m_postSplit == MyMoneyMoney(0,1));
  CPPUNIT_ASSERT(a.formatMoney() == MyMoneyMoney(15,10).formatMoney());

  a += MyMoneyMoney(3,1);
  a += MyMoneyMoney(3,1);
  CPPUNIT_ASSERT(a == MyMoneyMoney(3,1));
  CPPUNIT_ASSERT(a.m_stockSplit == MyMoneyMoney(1,2));
  CPPUNIT_ASSERT(a.m_postSplit == MyMoneyMoney(6,1));
  CPPUNIT_ASSERT(a.formatMoney() == MyMoneyMoney(75,10).formatMoney());
}

void PivotTableTest::testCellAddCell(void)
{
  PivotTable::TCell a,b;

  a += MyMoneyMoney(3,1);
  a += PivotTable::TCell::stockSplit(MyMoneyMoney(2,1));
  a += MyMoneyMoney(4,1);

  CPPUNIT_ASSERT(a == MyMoneyMoney(3,1));
  CPPUNIT_ASSERT(a.m_stockSplit == MyMoneyMoney(2,1));
  CPPUNIT_ASSERT(a.m_postSplit == MyMoneyMoney(4,1));
  CPPUNIT_ASSERT(a.formatMoney() == MyMoneyMoney(10,1).formatMoney());

  b += MyMoneyMoney(4,1);
  b += PivotTable::TCell::stockSplit(MyMoneyMoney(4,1));
  b += MyMoneyMoney(16,1);
  
  CPPUNIT_ASSERT(b == MyMoneyMoney(4,1));
  CPPUNIT_ASSERT(b.m_stockSplit == MyMoneyMoney(4,1));
  CPPUNIT_ASSERT(b.m_postSplit == MyMoneyMoney(16,1));
  CPPUNIT_ASSERT(b.formatMoney() == MyMoneyMoney(32,1).formatMoney());

  a += b;

  CPPUNIT_ASSERT(a == MyMoneyMoney(3,1));
  CPPUNIT_ASSERT(a.m_stockSplit == MyMoneyMoney(8,1));
  CPPUNIT_ASSERT(a.m_postSplit == MyMoneyMoney(48,1));
  CPPUNIT_ASSERT(a.formatMoney() == MyMoneyMoney(72,1).formatMoney());
}

void PivotTableTest::testCellRunningSum(void)
{
  PivotTable::TCell a;
  MyMoneyMoney runningSum(12,10);

  a += MyMoneyMoney(3,1);
  a += PivotTable::TCell::stockSplit(MyMoneyMoney(125,100));
  a += MyMoneyMoney(134,10);

  CPPUNIT_ASSERT(a.m_stockSplit != MyMoneyMoney(1,1));
  CPPUNIT_ASSERT(a.m_postSplit != MyMoneyMoney(0,1));

  runningSum = a.calculateRunningSum(runningSum);

  CPPUNIT_ASSERT(runningSum == MyMoneyMoney(1865,100));
  CPPUNIT_ASSERT(a.formatMoney() == MyMoneyMoney(1865,100).formatMoney());
  CPPUNIT_ASSERT(a.m_stockSplit == MyMoneyMoney(1,1));
  CPPUNIT_ASSERT(a.m_postSplit == MyMoneyMoney(0,1));
}

// vim:cin:si:ai:et:ts=2:sw=2:
