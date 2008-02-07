/***************************************************************************
                          mymoneyforecasttest.cpp
                          -------------------
    copyright            : (C) 2007 by Alvaro Soliverez
    email                : asoliverez@gmail.com
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

#include "mymoneyforecasttest.h"

#include <kmymoney/mymoneyexception.h>

#include "../kmymoneyglobalsettings.h"
#include "../mymoney/storage/mymoneystoragedump.h"
#include "../mymoney/storage/mymoneystoragexml.h"
#include "../reports/reportstestcommon.h"


using namespace test;

MyMoneyForecastTest::MyMoneyForecastTest()
{
  this->moT1 = MyMoneyMoney(57,1);
  this->moT2 = MyMoneyMoney(63,1);
  this->moT3 = MyMoneyMoney(84,1);
  this->moT4 = MyMoneyMoney(62,1);
  this->moT5 = MyMoneyMoney(104,1);
}


void MyMoneyForecastTest::setUp () {
  
  //all this has been taken from pivottabletest.cpp, by Thomas Baumgart and Ace Jones
  
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
  MyMoneyPayee payeeTest2("Alvaro Soliverez");
  file->addPayee(payeeTest2);

  acAsset = (MyMoneyFile::instance()->asset().id());
  acLiability = (MyMoneyFile::instance()->liability().id());
  acExpense = (MyMoneyFile::instance()->expense().id());
  acIncome = (MyMoneyFile::instance()->income().id());
  acChecking = makeAccount(QString("Checking Account"),MyMoneyAccount::Checkings,moCheckingOpen,QDate(2004,5,15),acAsset, "USD");
  acCredit = makeAccount(QString("Credit Card"),MyMoneyAccount::CreditCard,moCreditOpen,QDate(2004,7,15),acLiability, "USD");
  acSolo = makeAccount(QString("Solo"),MyMoneyAccount::Expense,0,QDate(2004,1,11),acExpense, "USD");
  acParent = makeAccount(QString("Parent"),MyMoneyAccount::Expense,0,QDate(2004,1,11),acExpense, "USD");
  acChild = makeAccount(QString("Child"),MyMoneyAccount::Expense,0,QDate(2004,2,11),acParent, "USD");
  acForeign = makeAccount(QString("Foreign"),MyMoneyAccount::Expense,0,QDate(2004,1,11),acExpense, "USD");
  acInvestment = makeAccount("Investment",MyMoneyAccount::Investment,moZero,QDate(2004,1,1),acAsset, "USD");

  acSecondChild = makeAccount(QString("Second Child"),MyMoneyAccount::Expense,0,QDate(2004,2,11),acParent, "USD");
  acGrandChild1 = makeAccount(QString("Grand Child 1"),MyMoneyAccount::Expense,0,QDate(2004,2,11),acChild, "USD");
  acGrandChild2 = makeAccount(QString("Grand Child 2"),MyMoneyAccount::Expense,0,QDate(2004,2,11),acChild, "USD");
  
  //this account added to have an account to test opening date calculations
  acCash = makeAccount(QString("Cash"),MyMoneyAccount::Cash,moCreditOpen,QDate::currentDate().addDays(-2),acAsset, "USD");
  

  MyMoneyInstitution i("Bank of the World","","","","","","");
  file->addInstitution(i);
  inBank = i.id();
  ft.commit();

}

void MyMoneyForecastTest::tearDown () {
    file->detachStorage(storage);
    delete storage;
}

void MyMoneyForecastTest::testEmptyConstructor() {
	MyMoneyForecast a;
  MyMoneyAccount b;
  
  int f = a.forecastBalance(b, QDate::currentDate());

	CPPUNIT_ASSERT(f == 0);
	CPPUNIT_ASSERT(!a.isForecastAccount(b));
  CPPUNIT_ASSERT(a.forecastBalance(b, QDate::currentDate()) == MyMoneyMoney(0,1));
  CPPUNIT_ASSERT(a.daysToMinimumBalance(b) == -1);
  CPPUNIT_ASSERT(a.daysToZeroBalance(b) == -2);
  CPPUNIT_ASSERT(a.forecastDays() == 1);
  CPPUNIT_ASSERT(a.accountsCycle() == 1);
  CPPUNIT_ASSERT(a.forecastCycles() == 1);
  CPPUNIT_ASSERT(a.historyDays() == 1);
}


void MyMoneyForecastTest::testDoForecastInit() {
  MyMoneyForecast a;
  
  a.doForecast();
  /*
  //check the illegal argument validation  
  try {
    KMyMoneyGlobalSettings::setForecastDays(-10);
    a.doForecast();
  }
  catch (MyMoneyException *e)
  {
    delete e;
    CPPUNIT_FAIL("Unexpected exception");
  }
  try {
    KMyMoneyGlobalSettings::setForecastAccountCycle(-20);
      a.doForecast();
    }
    catch (MyMoneyException *e) {
      delete e;
      CPPUNIT_FAIL("Unexpected exception");
  }
  try {
    KMyMoneyGlobalSettings::setForecastCycles(-10);
    a.doForecast();
  }
  catch (MyMoneyException *e) {
    delete e;
    CPPUNIT_FAIL("Unexpected exception");
  }
  
  try {
    KMyMoneyGlobalSettings::setForecastAccountCycle(0);
    a.doForecast();
  }
  catch (MyMoneyException *e) {
    delete e;
    CPPUNIT_FAIL("Unexpected exception");
  }
  try {
    KMyMoneyGlobalSettings::setForecastDays(0);
    KMyMoneyGlobalSettings::setForecastCycles(0);
    KMyMoneyGlobalSettings::setForecastAccountCycle(0);
    a.doForecast();
  }
  catch (MyMoneyException *e) {
    delete e;
    CPPUNIT_ASSERT("Unexpected exception");
  }*/
}
  
//test that it forecasts correctly with transactions in the period of forecast 
void MyMoneyForecastTest::testDoForecast() {
  //set up environment
  MyMoneyForecast a;
  
  MyMoneyAccount a_checking = file->account(acChecking);
  MyMoneyAccount a_credit = file->account(acCredit);
  
  TransactionHelper t1( QDate::currentDate().addDays(-1), MyMoneySplit::ActionWithdrawal, this->moT1, acChecking, acSolo);
  TransactionHelper t2( QDate::currentDate().addDays(-1), MyMoneySplit::ActionDeposit, -(this->moT2), acCredit, acParent);
  TransactionHelper t3( QDate::currentDate().addDays(-1), MyMoneySplit::ActionTransfer, this->moT1, acCredit, acChecking);
  
  
  KMyMoneyGlobalSettings::setForecastMethod(1);
  KMyMoneyGlobalSettings::setForecastDays(3);
  KMyMoneyGlobalSettings::setForecastAccountCycle(1);
  KMyMoneyGlobalSettings::setForecastCycles(1);
  a.doForecast();

  //checking didnt have balance variations, so the forecast should be equal to the current balance
  MyMoneyMoney b_checking = file->balance(a_checking.id(), QDate::currentDate());
  
  CPPUNIT_ASSERT(a.forecastBalance(a_checking, QDate::currentDate().addDays(1))==b_checking);
  CPPUNIT_ASSERT(a.forecastBalance(a_checking, QDate::currentDate().addDays(2))==b_checking);
  CPPUNIT_ASSERT(a.forecastBalance(a_checking, QDate::currentDate().addDays(3))==b_checking);
  CPPUNIT_ASSERT(a.forecastBalance(a_checking, QDate::currentDate())==b_checking);
  
  //credit had a variation so the forecast should be different for each day
  MyMoneyMoney b_credit = file->balance(a_credit.id(), QDate::currentDate());
  
  CPPUNIT_ASSERT(a.forecastBalance(a_credit, 0) == b_credit);
  CPPUNIT_ASSERT(a.forecastBalance(a_credit, QDate::currentDate().addDays(1)) == (b_credit+(moT2-moT1)));
  CPPUNIT_ASSERT(a.forecastBalance(a_credit, QDate::currentDate().addDays(2)) == (b_credit+((moT2-moT1)*2)));
  CPPUNIT_ASSERT(a.forecastBalance(a_credit, QDate::currentDate().addDays(3)) == b_credit+((moT2-moT1)*3));
  
  //insert transactions outside the forecast period. The calculation should be the same.
  TransactionHelper t4( QDate::currentDate().addDays(-2), MyMoneySplit::ActionDeposit, -moT2, acCredit, acParent );
  TransactionHelper t5( QDate::currentDate().addDays(-10), MyMoneySplit::ActionDeposit, -moT2, acCredit, acParent );
  
  KMyMoneyGlobalSettings::setForecastMethod(1);
  KMyMoneyGlobalSettings::setForecastDays(3);
  KMyMoneyGlobalSettings::setForecastAccountCycle(1);
  KMyMoneyGlobalSettings::setForecastCycles(1);
  a.doForecast();
  //check forecast
  b_credit = file->balance(a_credit.id(), QDate::currentDate());
  MyMoneyMoney b_credit_1_exp =  (b_credit+((moT2-moT1)));
  MyMoneyMoney b_credit_2 = a.forecastBalance(a_credit, QDate::currentDate().addDays(2));
  MyMoneyMoney b_credit_2_exp =  (b_credit+((moT2-moT1)*2));
  CPPUNIT_ASSERT(a.forecastBalance(a_credit, QDate::currentDate())==file->balance(a_credit.id(), QDate::currentDate()));
  CPPUNIT_ASSERT(a.forecastBalance(a_credit, QDate::currentDate().addDays(1))==b_credit+(moT2-moT1));
  CPPUNIT_ASSERT(a.forecastBalance(a_credit, QDate::currentDate().addDays(2))==b_credit+((moT2-moT1)*2));
  CPPUNIT_ASSERT(a.forecastBalance(a_credit, QDate::currentDate().addDays(3))==b_credit+((moT2-moT1)*3));
}

void MyMoneyForecastTest::testGetForecastAccountList()
{
  MyMoneyForecast a;
  MyMoneyAccount a_checking = file->account(acChecking);
  MyMoneyAccount a_parent = file->account(acParent);
  QValueList<MyMoneyAccount> b;
  
  b = a.forecastAccountList();
  //check that it contains asset account, but not expense accounts 
  CPPUNIT_ASSERT(b.contains(a_checking));
  CPPUNIT_ASSERT(!b.contains(a_parent));
  
}

void MyMoneyForecastTest::testCalculateAccountTrend()
{
  //set up environment
  TransactionHelper t1( QDate::currentDate().addDays(-3), MyMoneySplit::ActionDeposit, -moT2, acChecking, acSolo );
  MyMoneyAccount a_checking = file->account(acChecking);
    
    //test invalid arguments
  
  try {
    MyMoneyForecast::calculateAccountTrend(a_checking, 0);
  }
  catch (MyMoneyException *e) {
    CPPUNIT_ASSERT(e->what().compare("Illegal arguments when calling calculateAccountTrend. trendDays must be higher than 0") == 0);
    delete e;
  }
  try {
    MyMoneyForecast::calculateAccountTrend(a_checking, -10);
  }
  catch (MyMoneyException *e) {
    CPPUNIT_ASSERT(e->what().compare("Illegal arguments when calling calculateAccountTrend. trendDays must be higher than 0") == 0);
    delete e;
  }
  
  //test that it calculates correctly
  CPPUNIT_ASSERT(MyMoneyForecast::calculateAccountTrend(a_checking ,3) == moT2/MyMoneyMoney(3,1));
  
  //test that it works for all kind of accounts
  MyMoneyAccount a_solo = file->account(acSolo);
  MyMoneyMoney soloTrend = MyMoneyForecast::calculateAccountTrend(a_solo,3);
  MyMoneyMoney soloTrendExp = -moT2/MyMoneyMoney(3,1);
  CPPUNIT_ASSERT(MyMoneyForecast::calculateAccountTrend(a_solo,3) == -moT2/MyMoneyMoney(3,1));
  
  //test that it does not take into account the transactions of the opening date of the account
  MyMoneyAccount a_cash = file->account(acCash);
  TransactionHelper t2( QDate::currentDate().addDays(-2), MyMoneySplit::ActionDeposit, moT2, acCash, acParent );
  TransactionHelper t3( QDate::currentDate().addDays(-1), MyMoneySplit::ActionDeposit, moT1, acCash, acParent );
  CPPUNIT_ASSERT(MyMoneyForecast::calculateAccountTrend(a_cash,3) == -moT1);
   
}

void MyMoneyForecastTest::testGetForecastBalance()
{
  //set up environment
  MyMoneyForecast a;
  
  TransactionHelper t1( QDate::currentDate().addDays(-1), MyMoneySplit::ActionWithdrawal, this->moT1, acChecking, acSolo);
  TransactionHelper t2( QDate::currentDate().addDays(-1), MyMoneySplit::ActionDeposit, -(this->moT2), acCredit, acParent);
  TransactionHelper t3( QDate::currentDate().addDays(-1), MyMoneySplit::ActionTransfer, this->moT1, acCredit, acChecking);
  
  KMyMoneyGlobalSettings::setForecastMethod(1);
  KMyMoneyGlobalSettings::setForecastDays(3);
  KMyMoneyGlobalSettings::setForecastAccountCycle(1);
  KMyMoneyGlobalSettings::setForecastCycles(1);
  a.doForecast();
  
  MyMoneyAccount a_checking = file->account(acChecking);
  MyMoneyAccount a_credit = file->account(acCredit);
      
  //test invalid arguments
  CPPUNIT_ASSERT(a.forecastBalance(a_checking, QDate::currentDate().addDays(-1))==MyMoneyMoney(0,1));
  CPPUNIT_ASSERT(a.forecastBalance(a_checking, QDate::currentDate().addDays(-10))==MyMoneyMoney(0,1));
  CPPUNIT_ASSERT(a.forecastBalance(a_checking, -1)==MyMoneyMoney(0,1));
  CPPUNIT_ASSERT(a.forecastBalance(a_checking, -100)==MyMoneyMoney(0,1));
  
  //test a date outside the forecast days
  CPPUNIT_ASSERT(a.forecastBalance(a_checking, QDate::currentDate().addDays(4))==MyMoneyMoney(0,1));
  CPPUNIT_ASSERT(a.forecastBalance(a_checking, 4)==MyMoneyMoney(0,1));
  CPPUNIT_ASSERT(a.forecastBalance(a_checking, QDate::currentDate().addDays(10))==MyMoneyMoney(0,1));
  CPPUNIT_ASSERT(a.forecastBalance(a_checking, 10)==MyMoneyMoney(0,1));
  
  //test it returns valid results
  MyMoneyMoney b_credit = file->balance(a_credit.id(), QDate::currentDate());
  CPPUNIT_ASSERT(a.forecastBalance(a_credit, QDate::currentDate())==file->balance(a_credit.id(), QDate::currentDate()));
  CPPUNIT_ASSERT(a.forecastBalance(a_credit, QDate::currentDate().addDays(1))==b_credit+(moT2-moT1));
  CPPUNIT_ASSERT(a.forecastBalance(a_credit, QDate::currentDate().addDays(2))==b_credit+((moT2-moT1)*2));
  CPPUNIT_ASSERT(a.forecastBalance(a_credit, QDate::currentDate().addDays(3))==b_credit+((moT2-moT1)*3));
}

void MyMoneyForecastTest::testIsForecastAccount()
{
  MyMoneyForecast a;

  MyMoneyAccount a_checking = file->account(acChecking);
  MyMoneyAccount a_solo = file->account(acSolo);
  MyMoneyAccount a_investment = file->account(acInvestment);
  
  //test an invalid account
  CPPUNIT_ASSERT(a.isForecastAccount(a_solo)==false);
  CPPUNIT_ASSERT(a.isForecastAccount(a_investment)==false);
  
  //test a valid account
  CPPUNIT_ASSERT(a.isForecastAccount(a_checking)==true);
  
}

void MyMoneyForecastTest::testDoFutureScheduledForecast()
{
   
  
  //set up future transactions
  MyMoneyForecast a;
    
  MyMoneyAccount a_cash = file->account(acCash);
  TransactionHelper t1( QDate::currentDate().addDays(1), MyMoneySplit::ActionDeposit, -moT1, acCash, acParent );
  TransactionHelper t2( QDate::currentDate().addDays(2), MyMoneySplit::ActionDeposit, -moT2, acCash, acParent );
  TransactionHelper t3( QDate::currentDate().addDays(3), MyMoneySplit::ActionDeposit, -moT3, acCash, acParent );
  TransactionHelper t4( QDate::currentDate().addDays(10), MyMoneySplit::ActionDeposit, -moT4, acCash, acParent );
  
  KMyMoneyGlobalSettings::setForecastMethod(0);
  KMyMoneyGlobalSettings::setForecastDays(3);
  KMyMoneyGlobalSettings::setForecastAccountCycle(1);
  KMyMoneyGlobalSettings::setForecastCycles(1);
  a.doForecast();
  
  MyMoneyMoney b_cash = file->balance(a_cash.id(), QDate::currentDate());
  
  //test valid results
  CPPUNIT_ASSERT(a.forecastBalance(a_cash, QDate::currentDate())==b_cash);
  CPPUNIT_ASSERT(a.forecastBalance(a_cash, QDate::currentDate().addDays(1))==b_cash+moT1);
  CPPUNIT_ASSERT(a.forecastBalance(a_cash, QDate::currentDate().addDays(2))==b_cash+moT1+moT2);
  CPPUNIT_ASSERT(a.forecastBalance(a_cash, QDate::currentDate().addDays(3))==b_cash+moT1+moT2+moT3);
  
  //TODO test for schedules should be added  
}

void MyMoneyForecastTest::testScheduleForecast()
{
  //set up schedule environment for testing
  MyMoneyAccount a_cash = file->account(acCash);
  MyMoneyAccount a_parent = file->account(acParent);

  MyMoneyFileTransaction ft;
  MyMoneySchedule sch( "A Name",
                       MyMoneySchedule::TYPE_BILL,
                       MyMoneySchedule::OCCUR_WEEKLY,
                       MyMoneySchedule::STYPE_DIRECTDEBIT,
                       QDate::currentDate().addDays(1),
                                          QDate(),
                                              true,
                                              true);

  //sch.setLastPayment(QDate::currentDate());
  //sch.recordPayment(QDate::currentDate().addDays(1));
  //sch.setId("SCH0001");

  MyMoneyTransaction t;
  t.setPostDate(QDate::currentDate().addDays(1));
  t.setEntryDate(QDate::currentDate().addDays(1));
  //t.setId("T000000000000000001");
  t.setBankID("BID");
  t.setMemo("Wohnung:Miete");
  t.setCommodity("USD");
  t.setValue("key", "value");

  MyMoneySplit s;
  s.setPayeeId("P000001");
  s.setShares(moT2);
  s.setValue(moT2);
  s.setAccountId(a_parent.id());
  s.setBankID("SPID1");
  s.setReconcileFlag(MyMoneySplit::Reconciled);
  t.addSplit(s);

  s.setPayeeId("P000001");
  s.setShares(-moT2);
  s.setValue(-moT2);
  s.setAccountId(a_cash.id());
  s.setBankID("SPID2");
  s.setReconcileFlag(MyMoneySplit::Cleared);
  s.clearId();
  t.addSplit(s);

  sch.setTransaction(t);

  file->addSchedule(sch);
  ft.commit();

  MyMoneyFileTransaction ft3;
  MyMoneySchedule sch3( "A Name1",
                       MyMoneySchedule::TYPE_BILL,
                       MyMoneySchedule::OCCUR_WEEKLY,
                       MyMoneySchedule::STYPE_DIRECTDEBIT,
                       QDate::currentDate().addDays(5),
                                          QDate(),
                                              true,
                                              true);

  //sch.setLastPayment(QDate::currentDate());
  //sch.recordPayment(QDate::currentDate().addDays(1));
  //sch.setId("SCH0001");

  MyMoneyTransaction t3;
  t3.setPostDate(QDate::currentDate().addDays(5));
  t3.setEntryDate(QDate::currentDate().addDays(5));
  //t.setId("T000000000000000001");
  t3.setBankID("BID");
  t3.setMemo("Wohnung:Miete");
  t3.setCommodity("USD");
  t3.setValue("key", "value");

  MyMoneySplit s3;
  s3.setPayeeId("P000001");
  s3.setShares(moT2);
  s3.setValue(moT2);
  s3.setAccountId(a_parent.id());
  s3.setBankID("SPID1");
  s3.setReconcileFlag(MyMoneySplit::Reconciled);
  t3.addSplit(s3);

  s3.setPayeeId("P000001");
  s3.setShares(-moT2);
  s3.setValue(-moT2);
  s3.setAccountId(a_cash.id());
  s3.setBankID("SPID2");
  s3.setReconcileFlag(MyMoneySplit::Cleared);
  s3.clearId();
  t3.addSplit(s3);

  sch3.setTransaction(t3);

  file->addSchedule(sch3);
  ft3.commit();


  MyMoneyFileTransaction ft2;
  MyMoneySchedule sch2( "A Name2",
                       MyMoneySchedule::TYPE_BILL,
                       MyMoneySchedule::OCCUR_WEEKLY,
                       MyMoneySchedule::STYPE_DIRECTDEBIT,
                       QDate::currentDate().addDays(2),
                                          QDate(),
                                              true,
                                              true);

  //sch.setLastPayment(QDate::currentDate());
  //sch.recordPayment(QDate::currentDate().addDays(1));
  //sch.setId("SCH0001");

  MyMoneyTransaction t2;
  t2.setPostDate(QDate::currentDate().addDays(2));
  t2.setEntryDate(QDate::currentDate().addDays(2));
  //t.setId("T000000000000000001");
  t2.setBankID("BID");
  t2.setMemo("Wohnung:Miete");
  t2.setCommodity("USD");
  t2.setValue("key", "value");

  MyMoneySplit s2;
  s2.setPayeeId("P000001");
  s2.setShares(moT1);
  s2.setValue(moT1);
  s2.setAccountId(a_parent.id());
  s2.setBankID("SPID1");
  s2.setReconcileFlag(MyMoneySplit::Reconciled);
  t2.addSplit(s2);

  s2.setPayeeId("P000001");
  s2.setShares(-moT1);
  s2.setValue(-moT1);
  s2.setAccountId(a_cash.id());
  s2.setBankID("SPID2");
  s2.setReconcileFlag(MyMoneySplit::Cleared);
  s2.clearId();
  t2.addSplit(s2);

  sch2.setTransaction(t2);

  file->addSchedule(sch2);

  ft2.commit();

  //run forecast
  MyMoneyForecast a;
  KMyMoneyGlobalSettings::setForecastMethod(0);
  KMyMoneyGlobalSettings::setForecastDays(3);
  KMyMoneyGlobalSettings::setForecastAccountCycle(1);
  KMyMoneyGlobalSettings::setForecastCycles(1);
  a.doForecast();

  //check result for single schedule
  MyMoneyMoney b_cash = file->balance(a_cash.id(), QDate::currentDate());
  MyMoneyMoney b_cash1 = a.forecastBalance(a_cash, QDate::currentDate().addDays(1));

  //test valid results
  CPPUNIT_ASSERT(a.forecastBalance(a_cash, QDate::currentDate())==b_cash);
  CPPUNIT_ASSERT(a.forecastBalance(a_cash, QDate::currentDate().addDays(1))==b_cash-moT2);
  CPPUNIT_ASSERT(a.forecastBalance(a_cash, QDate::currentDate().addDays(2))==b_cash-moT2-moT1);
}


void MyMoneyForecastTest::testDaysToMinimumBalance()
{
  //setup environment   
  MyMoneyForecast a;
    
  MyMoneyAccount a_cash = file->account(acCash);
  MyMoneyAccount a_credit = file->account(acCredit);
  MyMoneyAccount a_parent = file->account(acParent);
  a_cash.setValue("minBalanceAbsolute", "50");
  a_credit.setValue("minBalanceAbsolute", "50");
  TransactionHelper t1( QDate::currentDate().addDays(-1), MyMoneySplit::ActionDeposit, -moT1, acCash, acParent );
  TransactionHelper t2( QDate::currentDate().addDays(2), MyMoneySplit::ActionDeposit, moT2, acCash, acParent );
  TransactionHelper t3( QDate::currentDate().addDays(-1), MyMoneySplit::ActionWithdrawal, -moT1, acCredit, acParent );
  TransactionHelper t4( QDate::currentDate().addDays(4), MyMoneySplit::ActionWithdrawal, moT5, acCredit, acParent );
  
  KMyMoneyGlobalSettings::setForecastMethod(0);
  KMyMoneyGlobalSettings::setForecastDays(3);
  KMyMoneyGlobalSettings::setForecastAccountCycle(1);
  KMyMoneyGlobalSettings::setForecastCycles(1);
  a.doForecast();
  
   
  
  //test invalid arguments
  MyMoneyAccount nullAcc;
  CPPUNIT_ASSERT(a.daysToMinimumBalance(nullAcc) == -1);
  
  //test when not a forecast account
  CPPUNIT_ASSERT(a.daysToMinimumBalance(a_parent) == -1);
  
  //test it warns when inside the forecast period
  CPPUNIT_ASSERT(a.daysToMinimumBalance(a_cash) == 2);
  
  //test it does not warn when it will be outside of the forecast period
  CPPUNIT_ASSERT(a.daysToMinimumBalance(a_credit) == -1);
}
void MyMoneyForecastTest::testDaysToZeroBalance()
{
  //set up environment
  MyMoneyAccount a_Solo = file->account(acSolo);
  MyMoneyAccount a_Cash = file->account(acCash);
  MyMoneyAccount a_Credit = file->account(acCredit);
  
  //MyMoneyFileTransaction ft;
  TransactionHelper t1( QDate::currentDate().addDays(2), MyMoneySplit::ActionWithdrawal, -moT1, acChecking, acSolo );
  TransactionHelper t2( QDate::currentDate().addDays(2), MyMoneySplit::ActionTransfer, (moT5), acCash, acCredit );
  TransactionHelper t3( QDate::currentDate().addDays(2), MyMoneySplit::ActionWithdrawal, (moT5*100), acCredit, acParent );
  //ft.commit();
  
  MyMoneyForecast a;
  KMyMoneyGlobalSettings::setForecastMethod(0);
  KMyMoneyGlobalSettings::setForecastDays(30);
  KMyMoneyGlobalSettings::setForecastAccountCycle(1);
  KMyMoneyGlobalSettings::setForecastCycles(3);
  a.doForecast();

  //test invalid arguments
  MyMoneyAccount nullAcc;
  try {
    a.daysToZeroBalance(nullAcc);
  }
  catch (MyMoneyException *e) {
    delete e;
    CPPUNIT_FAIL("Unexpected exception");
  }
      
  //test when not a forecast account
  MyMoneyAccount a_solo = file->account(acSolo);
  int iSolo = a.daysToZeroBalance(a_Solo);
  
  CPPUNIT_ASSERT(iSolo == -2);
  
  //test it warns when inside the forecast period
  
  MyMoneyMoney fCash = a.forecastBalance(a_Cash, QDate::currentDate().addDays(2));
  
  CPPUNIT_ASSERT(a.daysToZeroBalance(a_Cash) == 2);
  
  //test it does not warn when it will be outside of the forecast period
  
}

void MyMoneyForecastTest::testSkipOpeningDate()
{
  //set up environment
  MyMoneyForecast a;
  
  TransactionHelper t1( QDate::currentDate().addDays(-2), MyMoneySplit::ActionWithdrawal, this->moT1, acCash, acSolo);
  TransactionHelper t2( QDate::currentDate().addDays(-1), MyMoneySplit::ActionWithdrawal, this->moT2, acCash, acSolo);
  
  KMyMoneyGlobalSettings::setForecastMethod(1);
  KMyMoneyGlobalSettings::setForecastDays(3);
  KMyMoneyGlobalSettings::setForecastAccountCycle(2);
  KMyMoneyGlobalSettings::setForecastCycles(1);
  a.doForecast();
  
  MyMoneyAccount a_cash = file->account(acCash);
  
  //test it has no variation because it skipped the variation of the opening date
  MyMoneyMoney b_cash = file->balance(a_cash.id(), QDate::currentDate());
  CPPUNIT_ASSERT(a.forecastBalance(a_cash, QDate::currentDate())==b_cash);
  CPPUNIT_ASSERT(a.forecastBalance(a_cash, QDate::currentDate().addDays(1))==b_cash);
  CPPUNIT_ASSERT(a.forecastBalance(a_cash, QDate::currentDate().addDays(2))==b_cash-moT2);
  CPPUNIT_ASSERT(a.forecastBalance(a_cash, QDate::currentDate().addDays(3))==b_cash-moT2);
}

void MyMoneyForecastTest::testAccountMinimumBalanceDateList() {
  
  //set up environment
  MyMoneyForecast a;
  
  TransactionHelper t1( QDate::currentDate().addDays(-2), MyMoneySplit::ActionWithdrawal, this->moT1, acCash, acSolo);
  TransactionHelper t2( QDate::currentDate().addDays(-1), MyMoneySplit::ActionWithdrawal, this->moT2, acCash, acSolo);
  
  KMyMoneyGlobalSettings::setForecastMethod(1);
  KMyMoneyGlobalSettings::setForecastDays(6);
  KMyMoneyGlobalSettings::setForecastAccountCycle(2);
  KMyMoneyGlobalSettings::setForecastCycles(3);
  KMyMoneyGlobalSettings::setBeginForecastDay(QDate::currentDate().addDays(1).day());
  a.doForecast();
  
  MyMoneyAccount a_cash = file->account(acCash);
  
  //test 
  QValueList<QDate> dateList;
  dateList = a.accountMinimumBalanceDateList(a_cash);
  
  QValueList<QDate>::iterator it = dateList.begin();
  
  QDate minDate = *it;
  
  CPPUNIT_ASSERT(minDate==QDate::currentDate().addDays(2));
  it++;
  minDate = *it;
  CPPUNIT_ASSERT(minDate==QDate::currentDate().addDays(4));
  it++;
  minDate = *it;
  CPPUNIT_ASSERT(minDate==QDate::currentDate().addDays(6));
  
}

void MyMoneyForecastTest::testAccountMaximumBalanceDateList() {
    //set up environment
  MyMoneyForecast a;
  
  TransactionHelper t1( QDate::currentDate().addDays(-2), MyMoneySplit::ActionWithdrawal, this->moT1, acCash, acSolo);
  TransactionHelper t2( QDate::currentDate().addDays(-1), MyMoneySplit::ActionWithdrawal, this->moT2, acCash, acSolo);
  
  KMyMoneyGlobalSettings::setForecastMethod(1);
  KMyMoneyGlobalSettings::setForecastDays(6);
  KMyMoneyGlobalSettings::setForecastAccountCycle(2);
  KMyMoneyGlobalSettings::setForecastCycles(3);
  KMyMoneyGlobalSettings::setBeginForecastDay(QDate::currentDate().addDays(1).day());
  a.doForecast();
  
  MyMoneyAccount a_cash = file->account(acCash);
  
  //test 
  QValueList<QDate> dateList;
  dateList = a.accountMaximumBalanceDateList(a_cash);
  
  QValueList<QDate>::iterator it = dateList.begin();
  
  QDate maxDate = *it;
  
  CPPUNIT_ASSERT(maxDate==QDate::currentDate().addDays(1));
  it++;
  maxDate = *it;
  CPPUNIT_ASSERT(maxDate==QDate::currentDate().addDays(3));
  it++;
  maxDate = *it;
  CPPUNIT_ASSERT(maxDate==QDate::currentDate().addDays(5));
  
  
}

void MyMoneyForecastTest::testAccountAverageBalance() {
    //set up environment
  MyMoneyForecast a;
  
  TransactionHelper t1( QDate::currentDate().addDays(-2), MyMoneySplit::ActionWithdrawal, this->moT1, acCash, acSolo);
  TransactionHelper t2( QDate::currentDate().addDays(-1), MyMoneySplit::ActionWithdrawal, this->moT2, acCash, acSolo);
  
  KMyMoneyGlobalSettings::setForecastMethod(1);
  KMyMoneyGlobalSettings::setForecastDays(3);
  KMyMoneyGlobalSettings::setForecastAccountCycle(2);
  KMyMoneyGlobalSettings::setForecastCycles(1);
  KMyMoneyGlobalSettings::setBeginForecastDay(0);
  a.doForecast();
  
  MyMoneyAccount a_cash = file->account(acCash);
  
  //test
  MyMoneyMoney b_cash1 = a.forecastBalance(a_cash, QDate::currentDate().addDays(1));
  MyMoneyMoney b_cash2 = a.forecastBalance(a_cash, QDate::currentDate().addDays(2));
  MyMoneyMoney b_cash3 = a.forecastBalance(a_cash, QDate::currentDate().addDays(3));
  
  MyMoneyMoney average = (b_cash1 + b_cash2 +b_cash3)/MyMoneyMoney(3,1);
  
  
  CPPUNIT_ASSERT(a.accountAverageBalance(a_cash)==average);
}

void MyMoneyForecastTest::testBeginForecastDate() {
    //set up environment
  MyMoneyForecast a;
  QDate beginDate;
  int beginDay;
  
  KMyMoneyGlobalSettings::setForecastMethod(1);
  KMyMoneyGlobalSettings::setForecastDays(90);
  KMyMoneyGlobalSettings::setForecastAccountCycle(14);
  KMyMoneyGlobalSettings::setForecastCycles(3);
  KMyMoneyGlobalSettings::setBeginForecastDay(0);
  a.doForecast();
  
  //test when using old method without begin day
  CPPUNIT_ASSERT(QDate::currentDate() == a.beginForecastDate());

  //setup begin to last day of month
  KMyMoneyGlobalSettings::setBeginForecastDay(31);
  beginDay = KMyMoneyGlobalSettings::beginForecastDay();
  a.doForecast();

  //test
  if(QDate::currentDate().day() < beginDay)
  {
    if(QDate::currentDate().daysInMonth() < beginDay)
      beginDay = QDate::currentDate().daysInMonth();
    
    beginDate = QDate(QDate::currentDate().year(), QDate::currentDate().month(), beginDay);
    
    CPPUNIT_ASSERT(beginDate == a.beginForecastDate());
  }
  
  //setup begin day to same date
  KMyMoneyGlobalSettings::setBeginForecastDay(QDate::currentDate().day());
  beginDay = KMyMoneyGlobalSettings::beginForecastDay();
  a.doForecast();

  CPPUNIT_ASSERT(QDate::currentDate() == a.beginForecastDate());
  
  //setup to first day of month with small interval
  KMyMoneyGlobalSettings::setBeginForecastDay(1);
  KMyMoneyGlobalSettings::setForecastAccountCycle(1);
  beginDay = KMyMoneyGlobalSettings::beginForecastDay();
  a.doForecast();
  
  //test
  if(QDate::currentDate() == a.beginForecastDate()) {
    CPPUNIT_ASSERT(QDate::currentDate() == a.beginForecastDate());
  } else {
    beginDay = ((((QDate::currentDate().day() - beginDay)/a.accountsCycle()) + 1) * a.accountsCycle()) + beginDay;
    beginDate = QDate(QDate::currentDate().year(), QDate::currentDate().month(), beginDay);
    
    CPPUNIT_ASSERT(beginDate == a.beginForecastDate());
  }
  
  //setup to test when current date plus cycle equals begin day
  KMyMoneyGlobalSettings::setForecastAccountCycle(14);
  beginDay = QDate::currentDate().addDays(14).day();
  KMyMoneyGlobalSettings::setBeginForecastDay(beginDay);
  beginDate = QDate::currentDate().addDays(14);
  a.doForecast();
  
  //test
  CPPUNIT_ASSERT(beginDate == a.beginForecastDate());
  
  //setup to test when the begin day will be next month
  KMyMoneyGlobalSettings::setBeginForecastDay(1);
  KMyMoneyGlobalSettings::setForecastAccountCycle(40);
  a.doForecast();
  
  beginDate = QDate(QDate::currentDate().addMonths(1).year(), QDate::currentDate().addMonths(1).month(), 1);
    
  //test
  CPPUNIT_ASSERT(beginDate == a.beginForecastDate());
}

