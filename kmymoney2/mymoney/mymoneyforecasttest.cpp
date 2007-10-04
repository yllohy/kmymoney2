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
  
  int f = a.getForecastBalance(b, QDate::currentDate());

	CPPUNIT_ASSERT(f == 0);
	CPPUNIT_ASSERT(!a.isForecastAccount(b));
  CPPUNIT_ASSERT(a.getForecastBalance(b, QDate::currentDate()) == MyMoneyMoney(0,1));
  CPPUNIT_ASSERT(a.daysToMinimumBalance(b) == -1);
  CPPUNIT_ASSERT(a.daysToZeroBalance(b) == -2);
  CPPUNIT_ASSERT(a.getForecastDays() == 1);
  CPPUNIT_ASSERT(a.getAccountsCycle() == 1);
  CPPUNIT_ASSERT(a.getForecastCycles() == 1);
  CPPUNIT_ASSERT(a.getHistoryDays() == 1);
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
  
  CPPUNIT_ASSERT(a.getForecastBalance(a_checking, QDate::currentDate().addDays(1))==b_checking);
  CPPUNIT_ASSERT(a.getForecastBalance(a_checking, QDate::currentDate().addDays(2))==b_checking);
  CPPUNIT_ASSERT(a.getForecastBalance(a_checking, QDate::currentDate().addDays(3))==b_checking);
  CPPUNIT_ASSERT(a.getForecastBalance(a_checking, QDate::currentDate())==b_checking);
  
  //credit had a variation so the forecast should be different for each day
  MyMoneyMoney b_credit = file->balance(a_credit.id(), QDate::currentDate());
  
  CPPUNIT_ASSERT(a.getForecastBalance(a_credit, 0) == b_credit);
  CPPUNIT_ASSERT(a.getForecastBalance(a_credit, QDate::currentDate().addDays(1)) == (b_credit+(moT2-moT1)));
  CPPUNIT_ASSERT(a.getForecastBalance(a_credit, QDate::currentDate().addDays(2)) == (b_credit+((moT2-moT1)*2)));
  CPPUNIT_ASSERT(a.getForecastBalance(a_credit, QDate::currentDate().addDays(3)) == b_credit+((moT2-moT1)*3));
  
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
  MyMoneyMoney b_credit_2 = a.getForecastBalance(a_credit, QDate::currentDate().addDays(2));
  MyMoneyMoney b_credit_2_exp =  (b_credit+((moT2-moT1)*2));
  CPPUNIT_ASSERT(a.getForecastBalance(a_credit, QDate::currentDate())==file->balance(a_credit.id(), QDate::currentDate()));
  CPPUNIT_ASSERT(a.getForecastBalance(a_credit, QDate::currentDate().addDays(1))==b_credit+(moT2-moT1));
  CPPUNIT_ASSERT(a.getForecastBalance(a_credit, QDate::currentDate().addDays(2))==b_credit+((moT2-moT1)*2));
  CPPUNIT_ASSERT(a.getForecastBalance(a_credit, QDate::currentDate().addDays(3))==b_credit+((moT2-moT1)*3));
}

void MyMoneyForecastTest::testGetForecastAccountList()
{
  MyMoneyForecast a;
  MyMoneyAccount a_checking = file->account(acChecking);
  MyMoneyAccount a_parent = file->account(acParent);
  QValueList<MyMoneyAccount> b;
  
  b = a.getForecastAccountList();
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
  CPPUNIT_ASSERT(a.getForecastBalance(a_checking, QDate::currentDate().addDays(-1))==MyMoneyMoney(0,1));
  CPPUNIT_ASSERT(a.getForecastBalance(a_checking, QDate::currentDate().addDays(-10))==MyMoneyMoney(0,1));
  CPPUNIT_ASSERT(a.getForecastBalance(a_checking, -1)==MyMoneyMoney(0,1));
  CPPUNIT_ASSERT(a.getForecastBalance(a_checking, -100)==MyMoneyMoney(0,1));
  
  //test a date outside the forecast days
  CPPUNIT_ASSERT(a.getForecastBalance(a_checking, QDate::currentDate().addDays(4))==MyMoneyMoney(0,1));
  CPPUNIT_ASSERT(a.getForecastBalance(a_checking, 4)==MyMoneyMoney(0,1));
  CPPUNIT_ASSERT(a.getForecastBalance(a_checking, QDate::currentDate().addDays(10))==MyMoneyMoney(0,1));
  CPPUNIT_ASSERT(a.getForecastBalance(a_checking, 10)==MyMoneyMoney(0,1));
  
  //test it returns valid results
  MyMoneyMoney b_credit = file->balance(a_credit.id(), QDate::currentDate());
  CPPUNIT_ASSERT(a.getForecastBalance(a_credit, QDate::currentDate())==file->balance(a_credit.id(), QDate::currentDate()));
  CPPUNIT_ASSERT(a.getForecastBalance(a_credit, QDate::currentDate().addDays(1))==b_credit+(moT2-moT1));
  CPPUNIT_ASSERT(a.getForecastBalance(a_credit, QDate::currentDate().addDays(2))==b_credit+((moT2-moT1)*2));
  CPPUNIT_ASSERT(a.getForecastBalance(a_credit, QDate::currentDate().addDays(3))==b_credit+((moT2-moT1)*3));
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
  CPPUNIT_ASSERT(a.getForecastBalance(a_cash, QDate::currentDate())==b_cash);
  CPPUNIT_ASSERT(a.getForecastBalance(a_cash, QDate::currentDate().addDays(1))==b_cash+moT1);
  CPPUNIT_ASSERT(a.getForecastBalance(a_cash, QDate::currentDate().addDays(2))==b_cash+moT1+moT2);
  CPPUNIT_ASSERT(a.getForecastBalance(a_cash, QDate::currentDate().addDays(3))==b_cash+moT1+moT2+moT3);
  
  //TODO test for schedules should be added  
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
  
  MyMoneyMoney fCash = a.getForecastBalance(a_Cash, QDate::currentDate().addDays(2));
  
  CPPUNIT_ASSERT(a.daysToZeroBalance(a_Cash) == 2);
  
  //test it does not warn when it will be outside of the forecast period
  

  
}


