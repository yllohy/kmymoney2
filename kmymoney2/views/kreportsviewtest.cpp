/***************************************************************************
                          kreportsviewtest.cpp
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

#include "kreportsviewtest.h"

#define private public
#include "pivottable.h"
#undef private
using namespace reports;

#include "../mymoney/mymoneyequity.h"
#include "../mymoney/storage/mymoneystoragedump.h"

class TransactionHelper: public MyMoneyTransaction
{
private:
  QCString m_id;
public:
  TransactionHelper( const QDate& _date, const QCString& _action, MyMoneyMoney _value, const QCString& _accountid, const QCString& _categoryid, const QCString& _currencyid = QCString(), const QString& _payee="Test Payee" );
  ~TransactionHelper();
  void update(void);
};

TransactionHelper::TransactionHelper( const QDate& _date, const QCString& _action, MyMoneyMoney _value, const QCString& _accountid, const QCString& _categoryid, const QCString& _currencyid, const QString& _payee )
{
    bool haspayee = ! _payee.isEmpty();
    MyMoneyPayee payeeTest = MyMoneyFile::instance()->payeeByName(_payee);

    setPostDate(_date);

    QCString currencyid = _currencyid;
    if ( ! currencyid.isEmpty() )
        setCommodity(currencyid);
    else
      currencyid=MyMoneyFile::instance()->baseCurrency().id();

    MyMoneyMoney price;
    MyMoneySplit splitLeft;
    if ( haspayee )
      splitLeft.setPayeeId(payeeTest.id());
    splitLeft.setAction(_action);
    splitLeft.setValue(-_value);
    splitLeft.setShares(-_value); // assumes transaction currency == this account's currency!
    splitLeft.setAccountId(_accountid);
    addSplit(splitLeft);

    MyMoneySplit splitRight;
    if ( haspayee )
      splitRight.setPayeeId(payeeTest.id());
    splitRight.setAction(_action);
    splitRight.setValue(_value);
    price = MyMoneyFile::instance()->currency(currencyid).price(_date);
    splitRight.setShares(_value * price );
    splitRight.setAccountId(_categoryid);
    addSplit(splitRight);

    MyMoneyFile::instance()->addTransaction(*this);
}

TransactionHelper::~TransactionHelper()
{
  MyMoneyFile::instance()->removeTransaction(*this);
}

void TransactionHelper::update(void)
{
  MyMoneyFile::instance()->modifyTransaction(*this);
}

QCString makeAccount( const QString& _name, MyMoneyAccount::accountTypeE _type, MyMoneyMoney _balance, const QDate& _open, const QCString& _parent, QCString _currency="" )
{
  MyMoneyAccount info;

  info.setName(_name);
  info.setAccountType(_type);
  info.setOpeningBalance(_balance);
  info.setOpeningDate(_open);
  if ( _currency != "" )
    info.setCurrencyId(_currency);
  else
    info.setCurrencyId("USD");

  MyMoneyAccount parent = MyMoneyFile::instance()->account(_parent);
  MyMoneyFile::instance()->addAccount( info, parent );

  return info.id();
}

void makePrice(const QCString& _currency, const QDate& _date, const MyMoneyMoney& _price )
{
  MyMoneyCurrency curr = MyMoneyFile::instance()->currency(_currency);
  curr.addPriceHistory(_date,_price);
  MyMoneyFile::instance()->modifyCurrency(curr);
}

KReportsViewTest::KReportsViewTest()
{
}

const MyMoneyMoney moCheckingOpen(1418.0);
const MyMoneyMoney moCreditOpen(-418.0);
const MyMoneyMoney moZero(0.0);
const MyMoneyMoney moSolo(234.12);
const MyMoneyMoney moParent1(88.01);
const MyMoneyMoney moParent2(133.22);
const MyMoneyMoney moParent(moParent1+moParent2);
const MyMoneyMoney moChild(14.00);
const MyMoneyMoney moThomas(5.11);
const MyMoneyMoney moNoPayee(8944.70);

QCString acAsset; 
QCString acLiability;
QCString acExpense;
QCString acChecking;
QCString acCredit;
QCString acSolo;
QCString acParent;
QCString acChild;
QCString acForeign; 

void KReportsViewTest::setUp () {

	storage = new MyMoneySeqAccessMgr;
	file = MyMoneyFile::instance();
	file->attachStorage(storage);

  file->addCurrency(MyMoneyCurrency("CAD", "Canadian Dollar",        "C$"));
  file->addCurrency(MyMoneyCurrency("USD", "US Dollar",              "$"));
  file->addCurrency(MyMoneyCurrency("JPY", "Japanese Yen",           QChar(0x00A5), 100, 1));
  file->addCurrency(MyMoneyCurrency("GBP", "British Pound",           "#"));
  file->setBaseCurrency(file->currency("USD"));

  MyMoneyPayee payeeTest("Test Payee");
  file->addPayee(payeeTest);
  MyMoneyPayee payeeTest2("Thomas Baumgart");
  file->addPayee(payeeTest2);
  
  acAsset = (MyMoneyFile::instance()->asset().id());
  acLiability = (MyMoneyFile::instance()->liability().id());
  acExpense = (MyMoneyFile::instance()->expense().id());
  acChecking = makeAccount(QString("Checking Account"),MyMoneyAccount::Checkings,moCheckingOpen,QDate(2004,5,15),acAsset);
  acCredit = makeAccount(QString("Credit Card"),MyMoneyAccount::CreditCard,moCreditOpen,QDate(2004,7,15),acLiability);
  acSolo = makeAccount(QString("Solo"),MyMoneyAccount::Expense,0,QDate(2004,1,11),acExpense);
  acParent = makeAccount(QString("Parent"),MyMoneyAccount::Expense,0,QDate(2004,1,11),acExpense);
  acChild = makeAccount(QString("Child"),MyMoneyAccount::Expense,0,QDate(2004,2,11),acParent);
  acForeign = makeAccount(QString("Foreign"),MyMoneyAccount::Expense,0,QDate(2004,1,11),acExpense);
  
}

void KReportsViewTest::tearDown ()
{
	file->detachStorage(storage);
	delete file;
	delete storage;
}

void KReportsViewTest::testNetWorthSingle()
{

  ReportConfigurationFilter filter( ReportConfigurationFilter::eAssetLiability );
  filter.setDateFilter(QDate(2004,1,1),QDate(2004,7,1).addDays(-1));
  PivotTable networth_f(filter);

  CPPUNIT_ASSERT(networth_f.m_grid["Asset"]["Checking Account"][acChecking][5]==moCheckingOpen);
  CPPUNIT_ASSERT(networth_f.m_grid["Asset"]["Checking Account"][acChecking][6]==moCheckingOpen);
  CPPUNIT_ASSERT(networth_f.m_grid["Asset"]["Checking Account"].m_total[5]==moCheckingOpen);
  CPPUNIT_ASSERT(networth_f.m_grid.m_total[0]==moZero);
  CPPUNIT_ASSERT(networth_f.m_grid.m_total[4]==moZero);
  CPPUNIT_ASSERT(networth_f.m_grid.m_total[5]==moCheckingOpen);
  CPPUNIT_ASSERT(networth_f.m_grid.m_total[6]==moCheckingOpen);
}

void KReportsViewTest::testNetWorthOfsetting()
{
  // Test the net worth report to make sure it picks up the opening balance for two
  // accounts opened during the period of the report, one asset & one liability.  Test
  // that it calculates the totals correctly.
  
  ReportConfigurationFilter filter( ReportConfigurationFilter::eAssetLiability );
  filter.setDateFilter(QDate(2004,1,1),QDate(2005,1,1).addDays(-1));
  PivotTable networth_f( filter );
  CPPUNIT_ASSERT(networth_f.m_grid["Liability"]["Credit Card"][acCredit][7]==moCreditOpen);
  CPPUNIT_ASSERT(networth_f.m_grid.m_total[0]==moZero);
  CPPUNIT_ASSERT(networth_f.m_grid.m_total[12]==moCheckingOpen+moCreditOpen);

}

void KReportsViewTest::testNetWorthOpeningPrior()
{
  // Test the net worth report to make sure it's picking up opening balances PRIOR to
  // the period of the report.

  ReportConfigurationFilter filter( ReportConfigurationFilter::eAssetLiability );
  filter.setDateFilter(QDate(2004,8,1),QDate(2005,9,1).addDays(-1));
  PivotTable networth_f( filter );

  CPPUNIT_ASSERT(networth_f.m_grid["Liability"]["Credit Card"].m_total[0]==moCreditOpen);
  CPPUNIT_ASSERT(networth_f.m_grid["Asset"]["Checking Account"].m_total[0]==moCheckingOpen);
  CPPUNIT_ASSERT(networth_f.m_grid.m_total[0]==moCheckingOpen+moCreditOpen);
  CPPUNIT_ASSERT(networth_f.m_grid.m_total[1]==moCheckingOpen+moCreditOpen);
}

void KReportsViewTest::testNetWorthDateFilter()
{
  // Test a net worth report whose period is prior to the time any accounts are open,
  // so the report should be zero.
  
  ReportConfigurationFilter filter( ReportConfigurationFilter::eAssetLiability );
  filter.setDateFilter(QDate(2004,1,1),QDate(2004,2,1).addDays(-1));
  PivotTable networth_f( filter );
  CPPUNIT_ASSERT(networth_f.m_grid.m_total[1]==moZero);
  
}

void KReportsViewTest::testSpendingEmpty()
{
  // test a spending report with no entries

  ReportConfigurationFilter filter( ReportConfigurationFilter::eExpenseIncome );
  PivotTable spending_f1( filter );
  CPPUNIT_ASSERT(spending_f1.m_grid.m_total.m_total==moZero);
  
  filter.setDateFilter(QDate(2004,9,1),QDate(2005,1,1).addDays(-1));
  PivotTable spending_f2( filter );
  CPPUNIT_ASSERT(spending_f2.m_grid.m_total.m_total==moZero);
}

void KReportsViewTest::testSingleTransaction()
{
  // Test a single transaction
  TransactionHelper t( QDate(2004,10,31), MyMoneySplit::ActionWithdrawal,moSolo, acChecking, acSolo );

  ReportConfigurationFilter filter( ReportConfigurationFilter::eExpenseIncome );
  filter.setDateFilter(QDate(2004,9,1),QDate(2005,1,1).addDays(-1));
  PivotTable spending_f( filter );

  CPPUNIT_ASSERT(spending_f.m_grid["Expense"]["Solo"][acSolo][2]==(-moSolo));
  CPPUNIT_ASSERT(spending_f.m_grid["Expense"]["Solo"].m_total[2]==(-moSolo));
  CPPUNIT_ASSERT(spending_f.m_grid["Expense"]["Solo"].m_total[1]==moZero);
  CPPUNIT_ASSERT(spending_f.m_grid.m_total[2]==(-moSolo));
  CPPUNIT_ASSERT(spending_f.m_grid.m_total.m_total==(-moSolo));
  
  filter.clear();
  filter.setRowType(ReportConfigurationFilter::eAssetLiability);
  filter.setDateFilter(QDate(2004,9,1),QDate(2005,1,1).addDays(-1));
  PivotTable networth_f( filter );
  CPPUNIT_ASSERT(networth_f.m_grid["Asset"]["Checking Account"].m_total[2]==(moCheckingOpen-moSolo) );
}

void KReportsViewTest::testSubAccount()
{
  // Test a sub-account with a value, under an account with a value
  
  TransactionHelper t1( QDate(2004,11,7), MyMoneySplit::ActionWithdrawal, moParent1, acCredit, acParent );
  TransactionHelper t2( QDate(2004,11,7), MyMoneySplit::ActionWithdrawal, moParent2, acCredit, acParent );
  TransactionHelper t3( QDate(2004,11,7), MyMoneySplit::ActionWithdrawal, moChild, acCredit, acChild );

  ReportConfigurationFilter filter( ReportConfigurationFilter::eExpenseIncome );
  filter.setDateFilter(QDate(2004,9,1),QDate(2005,1,1).addDays(-1));
  PivotTable spending_f( filter );

  CPPUNIT_ASSERT(spending_f.m_grid["Expense"]["Parent"][acParent][3]==(-moParent));
  CPPUNIT_ASSERT(spending_f.m_grid["Expense"]["Parent"][acChild][3]==(-moChild));
  CPPUNIT_ASSERT(spending_f.m_grid["Expense"]["Parent"].m_total[3]==(-moParent-moChild));
  CPPUNIT_ASSERT(spending_f.m_grid["Expense"]["Parent"].m_total[2]==moZero);
  CPPUNIT_ASSERT(spending_f.m_grid["Expense"]["Parent"].m_total.m_total==(-moParent-moChild));
  CPPUNIT_ASSERT(spending_f.m_grid.m_total[3]==(-moParent-moChild));
  CPPUNIT_ASSERT(spending_f.m_grid.m_total.m_total==(-moParent-moChild));
        
  filter.clear();
  filter.setRowType(ReportConfigurationFilter::eAssetLiability);
  filter.setDateFilter(QDate(2004,9,1),QDate(2005,1,1).addDays(-1));
  PivotTable networth_f( filter );
  CPPUNIT_ASSERT(networth_f.m_grid["Liability"]["Credit Card"].m_total[3]==-moParent-moChild+moCreditOpen );
  CPPUNIT_ASSERT(networth_f.m_grid.m_total[4] == -moParent-moChild+moCreditOpen+moCheckingOpen );
  
}

void KReportsViewTest::testFilterIEvsIE()
{
  // Test that removing an income/spending account will remove the entry from an income/spending report
  TransactionHelper t1( QDate(2004,10,31), MyMoneySplit::ActionWithdrawal, moSolo, acChecking, acSolo );
  TransactionHelper t2( QDate(2004,11,7), MyMoneySplit::ActionWithdrawal, moParent1, acCredit, acParent );
  TransactionHelper t3( QDate(2004,11,7), MyMoneySplit::ActionWithdrawal, moChild, acCredit, acChild );
  
  ReportConfigurationFilter filter( ReportConfigurationFilter::eExpenseIncome );
  filter.setDateFilter(QDate(2004,9,1),QDate(2005,1,1).addDays(-1));
  filter.addCategory(acChild);
  filter.addCategory(acSolo);
  PivotTable spending_f( filter );
        
  CPPUNIT_ASSERT(spending_f.m_grid["Expense"]["Parent"].m_total[3]==-moChild);
  CPPUNIT_ASSERT(spending_f.m_grid["Expense"].m_total[2]==-moSolo);
  CPPUNIT_ASSERT(spending_f.m_grid.m_total.m_total==-moSolo-moChild);
  
}
  
void KReportsViewTest::testFilterALvsAL()
{
  // Test that removing an asset/liability account will remove the entry from an asset/liability report
  TransactionHelper t1( QDate(2004,10,31), MyMoneySplit::ActionWithdrawal, moSolo, acChecking, acSolo );
  TransactionHelper t2( QDate(2004,11,7), MyMoneySplit::ActionWithdrawal, moParent1, acCredit, acParent );
  TransactionHelper t3( QDate(2004,11,7), MyMoneySplit::ActionWithdrawal, moChild, acCredit, acChild );
  
  ReportConfigurationFilter filter( ReportConfigurationFilter::eAssetLiability );
  filter.setDateFilter(QDate(2004,9,1),QDate(2005,1,1).addDays(-1));
  filter.addAccount(acChecking);
  filter.addCategory(acChild);
  filter.addCategory(acSolo);
  PivotTable networth_f( filter );
  CPPUNIT_ASSERT(networth_f.m_grid.m_total[3] == -moSolo+moCheckingOpen );
}

void KReportsViewTest::testFilterALvsIE()
{
  // Test that removing an asset/liability account will remove the entry from an income/spending report
  TransactionHelper t1( QDate(2004,10,31), MyMoneySplit::ActionWithdrawal, moSolo, acChecking, acSolo );
  TransactionHelper t2( QDate(2004,11,7), MyMoneySplit::ActionWithdrawal, moParent1, acCredit, acParent );
  TransactionHelper t3( QDate(2004,11,7), MyMoneySplit::ActionWithdrawal, moChild, acCredit, acChild );
  
  ReportConfigurationFilter filter(ReportConfigurationFilter::eExpenseIncome);
  filter.setDateFilter(QDate(2004,9,1),QDate(2005,1,1).addDays(-1));
  filter.addAccount(acChecking);
  CPPUNIT_ASSERT(file->transactionList(filter).count() == 1);

  PivotTable spending_f( filter );

  CPPUNIT_ASSERT(spending_f.m_grid["Expense"].m_total[3]==moZero);
  CPPUNIT_ASSERT(spending_f.m_grid["Expense"].m_total[2]==-moSolo);
  CPPUNIT_ASSERT(spending_f.m_grid.m_total.m_total==-moSolo);
}    

void KReportsViewTest::testFilterAllvsIE()
{
  // Test that removing an asset/liability account AND an income/expense
  // category will remove the entry from an income/spending report
  TransactionHelper t1( QDate(2004,10,31), MyMoneySplit::ActionWithdrawal, moSolo, acChecking, acSolo );
  TransactionHelper t2( QDate(2004,11,7), MyMoneySplit::ActionWithdrawal, moParent1, acCredit, acParent );
  TransactionHelper t3( QDate(2004,11,7), MyMoneySplit::ActionWithdrawal, moChild, acCredit, acChild );
  
  ReportConfigurationFilter filter( ReportConfigurationFilter::eExpenseIncome );
  filter.setDateFilter(QDate(2004,9,1),QDate(2005,1,1).addDays(-1));
  filter.addAccount(acCredit);
  filter.addCategory(acChild);
  PivotTable spending_f( filter );
    
  CPPUNIT_ASSERT(spending_f.m_grid["Expense"].m_total[2]==moZero);
  CPPUNIT_ASSERT(spending_f.m_grid["Expense"].m_total[3]==-moChild);
  CPPUNIT_ASSERT(spending_f.m_grid.m_total.m_total==-moChild);
}    

void KReportsViewTest::testFilterBasics()
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
        
void KReportsViewTest::testMultipleCurrencies()
{
  MyMoneyMoney moCanOpening( 1000.0 );
  MyMoneyMoney moJpyOpening( 1000.0 );
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

  ReportConfigurationFilter filter( ReportConfigurationFilter::eExpenseIncome );
  filter.setDateFilter(QDate(2004,1,1),QDate(2005,1,1).addDays(-1));
  PivotTable spending_f( filter );

  // test single foreign currency
  CPPUNIT_ASSERT(spending_f.m_grid["Expense"]["Foreign"][acCanCash][2]==(-moCanTransaction*moCanPrice));
  CPPUNIT_ASSERT(spending_f.m_grid["Expense"]["Foreign"][acCanCash][3]==(-moCanTransaction*moCanPrice));
  CPPUNIT_ASSERT(spending_f.m_grid["Expense"]["Foreign"][acCanCash][4]==(-moCanTransaction*moCanPrice));

  // test multiple foreign currencies under a common parent
  CPPUNIT_ASSERT(spending_f.m_grid["Expense"]["Foreign"][acJpyCash][2]==(-moJpyTransaction*moJpyPrice));
  CPPUNIT_ASSERT(spending_f.m_grid["Expense"]["Foreign"][acJpyCash][3]==(-moJpyTransaction*moJpyPrice));
  CPPUNIT_ASSERT(spending_f.m_grid["Expense"]["Foreign"][acJpyCash][4]==(-moJpyTransaction*moJpyPrice));
  CPPUNIT_ASSERT(spending_f.m_grid["Expense"]["Foreign"].m_total[2]==(-moJpyTransaction*moJpyPrice-moCanTransaction*moCanPrice));
  CPPUNIT_ASSERT(spending_f.m_grid["Expense"]["Foreign"].m_total.m_total==(-moJpyTransaction*moJpyPrice-moCanTransaction*moCanPrice-moJpyTransaction*moJpyPrice-moCanTransaction*moCanPrice-moJpyTransaction*moJpyPrice-moCanTransaction*moCanPrice));
  
  // Test the report type where we DO NOT convert the currency
  filter.setConvertCurrency(false);
  filter.setShowSubAccounts(true);
  PivotTable spending_fnc( filter );
  
  CPPUNIT_ASSERT(spending_fnc.m_grid["Expense"]["Foreign"][acCanCash][2]==(-moCanTransaction));
  CPPUNIT_ASSERT(spending_fnc.m_grid["Expense"]["Foreign"][acCanCash][3]==(-moCanTransaction));
  CPPUNIT_ASSERT(spending_fnc.m_grid["Expense"]["Foreign"][acCanCash][4]==(-moCanTransaction));
  CPPUNIT_ASSERT(spending_fnc.m_grid["Expense"]["Foreign"][acJpyCash][2]==(-moJpyTransaction));
  CPPUNIT_ASSERT(spending_fnc.m_grid["Expense"]["Foreign"][acJpyCash][3]==(-moJpyTransaction));
  CPPUNIT_ASSERT(spending_fnc.m_grid["Expense"]["Foreign"][acJpyCash][4]==(-moJpyTransaction));
  
  filter.setConvertCurrency(true);
  filter.clear();
  filter.setRowType(ReportConfigurationFilter::eAssetLiability);
  filter.setDateFilter(QDate(2004,1,1),QDate(2005,1,1).addDays(-1));
  PivotTable networth_f( filter );

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

void KReportsViewTest::testAdvancedFilter()
{
  // test more advanced filtering capabilities

  // amount
  {
    TransactionHelper t1( QDate(2004,11,7), MyMoneySplit::ActionWithdrawal, moParent1, acCredit, acParent );
    TransactionHelper t2( QDate(2004,11,7), MyMoneySplit::ActionWithdrawal, moChild, acCredit, acChild );
    ReportConfigurationFilter filter( ReportConfigurationFilter::eExpenseIncome );
    filter.setDateFilter(QDate(2004,1,1),QDate(2005,1,1).addDays(-1));
    filter.setAmountFilter(moChild,moChild);
    PivotTable spending_f( filter );
    CPPUNIT_ASSERT(spending_f.m_grid.m_total.m_total==-moChild);
  }
  
  // payee (specific)
  {
    TransactionHelper t1( QDate(2004,10,31), MyMoneySplit::ActionWithdrawal, moSolo, acChecking, acSolo );
    TransactionHelper t2( QDate(2004,11,7), MyMoneySplit::ActionWithdrawal, moParent1, acCredit, acParent );
    TransactionHelper t3( QDate(2004,11,7), MyMoneySplit::ActionWithdrawal, moChild, acCredit, acChild );
    TransactionHelper t4( QDate(2004,11,7), MyMoneySplit::ActionWithdrawal, moThomas, acCredit, acParent, QCString(), "Thomas Baumgart" );
    ReportConfigurationFilter filter( ReportConfigurationFilter::eExpenseIncome );
    filter.setDateFilter(QDate(2004,1,1),QDate(2005,1,1).addDays(-1));
    filter.addPayee(MyMoneyFile::instance()->payeeByName("Thomas Baumgart").id());
    PivotTable spending_f( filter );
    CPPUNIT_ASSERT(spending_f.m_grid["Expense"]["Parent"][acParent][11]==-moThomas);
    CPPUNIT_ASSERT(spending_f.m_grid.m_total.m_total==-moThomas);
  }
  // payee (no payee)
  {
    TransactionHelper t1( QDate(2004,10,31), MyMoneySplit::ActionWithdrawal, moSolo, acChecking, acSolo );
    TransactionHelper t2( QDate(2004,11,7), MyMoneySplit::ActionWithdrawal, moParent1, acCredit, acParent );
    TransactionHelper t3( QDate(2004,11,7), MyMoneySplit::ActionWithdrawal, moChild, acCredit, acChild );
    TransactionHelper t4( QDate(2004,11,7), MyMoneySplit::ActionWithdrawal, moNoPayee, acCredit, acParent, QCString(), QString() );
    
    ReportConfigurationFilter filter( ReportConfigurationFilter::eExpenseIncome );
    filter.setDateFilter(QDate(2004,1,1),QDate(2005,1,1).addDays(-1));
    filter.addPayee(QCString());
    PivotTable spending_f( filter );
    CPPUNIT_ASSERT(spending_f.m_grid["Expense"]["Parent"][acParent][11]==-moNoPayee);
    CPPUNIT_ASSERT(spending_f.m_grid.m_total.m_total==-moNoPayee);
  }
          
  // text
  {
    TransactionHelper t1( QDate(2004,10,31), MyMoneySplit::ActionWithdrawal, moSolo, acChecking, acSolo );
    TransactionHelper t2( QDate(2004,11,7), MyMoneySplit::ActionWithdrawal, moParent1, acCredit, acParent );
    TransactionHelper t3( QDate(2004,11,7), MyMoneySplit::ActionWithdrawal, moChild, acCredit, acChild );
    TransactionHelper t4( QDate(2004,11,7), MyMoneySplit::ActionWithdrawal, moThomas, acCredit, acParent, QCString(), "Thomas Baumgart" );
    
    ReportConfigurationFilter filter( ReportConfigurationFilter::eExpenseIncome );
    filter.setDateFilter(QDate(2004,1,1),QDate(2005,1,1).addDays(-1));
    filter.setTextFilter(QRegExp("Thomas"));
    PivotTable spending_f( filter );
  }
  
  // type (payment, deposit, transfer)
  {
    TransactionHelper t1( QDate(2004,1,1), MyMoneySplit::ActionWithdrawal, moSolo, acChecking, acSolo );
    TransactionHelper t2( QDate(2004,2,1), MyMoneySplit::ActionDeposit, -moParent1, acCredit, acParent );
    TransactionHelper t3( QDate(2004,11,1), MyMoneySplit::ActionTransfer, moChild, acCredit, acChecking );
  
    ReportConfigurationFilter filter( ReportConfigurationFilter::eExpenseIncome );
    filter.addType(MyMoneyTransactionFilter::payments);
    PivotTable spending_f( filter );
    
    CPPUNIT_ASSERT(spending_f.m_grid.m_total.m_total == -moSolo);
    
    filter.clear();
    filter.addType(MyMoneyTransactionFilter::deposits);
    PivotTable spending_f2( filter );

    CPPUNIT_ASSERT(spending_f2.m_grid.m_total.m_total == moParent1);
    
    filter.clear();
    filter.addType(MyMoneyTransactionFilter::transfers);
    PivotTable spending_f3( filter );

    CPPUNIT_ASSERT(spending_f3.m_grid.m_total.m_total == moZero);
    
    filter.setRowType(ReportConfigurationFilter::eAssetLiability);
    filter.setDateFilter( QDate(2004,1,1), QDate(2004,12,31) );
    PivotTable networth_f4( filter );

    CPPUNIT_ASSERT(networth_f4.m_grid["Asset"].m_total[11] == moCheckingOpen + moChild);
    CPPUNIT_ASSERT(networth_f4.m_grid["Liability"].m_total[11] == moCreditOpen - moChild);
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

    ReportConfigurationFilter filter( ReportConfigurationFilter::eExpenseIncome );
    filter.setDateFilter(QDate(2004,1,1),QDate(2005,1,1).addDays(-1));
    filter.addState(MyMoneyTransactionFilter::cleared);
    PivotTable spending_f( filter );
  
    CPPUNIT_ASSERT(spending_f.m_grid.m_total.m_total==-moSolo);

    filter.addState(MyMoneyTransactionFilter::reconciled);
    PivotTable spending_f2( filter );
    
    CPPUNIT_ASSERT(spending_f2.m_grid.m_total.m_total==-moSolo-moParent1);
    
    filter.clear();
    filter.addState(MyMoneyTransactionFilter::notReconciled);
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
  
    ReportConfigurationFilter filter( ReportConfigurationFilter::eExpenseIncome );
    filter.setDateFilter(QDate(2004,1,1),QDate(2005,1,1).addDays(-1));
    filter.setNumberFilter("1","3");
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
  
    ReportConfigurationFilter filter( ReportConfigurationFilter::eExpenseIncome );
    filter.setDateFilter(QDate(),QDate(2004,7,1));
    PivotTable spending_f( filter );
    CPPUNIT_ASSERT(spending_f.m_grid.m_total.m_total==-moSolo-moParent1-moParent2-moSolo-moParent1-moParent2);
        
    filter.clear();
    PivotTable spending_f2( filter );
    CPPUNIT_ASSERT(spending_f2.m_grid.m_total.m_total==-moSolo-moParent1-moParent2-moSolo-moParent1-moParent2-moSolo-moParent1-moParent2);
    
  }

}

void KReportsViewTest::testColumnType()
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

  ReportConfigurationFilter filter( ReportConfigurationFilter::eExpenseIncome );
  filter.setDateFilter(QDate(2003,12,31),QDate(2005,12,31));
  filter.setRowType( ReportConfigurationFilter::eExpenseIncome );
  filter.setColumnType(ReportConfigurationFilter::eBiMonths);
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
    
  filter.setColumnType(ReportConfigurationFilter::eQuarters);
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
  
  filter.setRowType( ReportConfigurationFilter::eAssetLiability );
  PivotTable networth_q( filter );

  CPPUNIT_ASSERT(networth_q.m_grid.m_total[1] == moZero);
  CPPUNIT_ASSERT(networth_q.m_grid.m_total[2] == -moSolo-moParent);
  CPPUNIT_ASSERT(networth_q.m_grid.m_total[3] == -moSolo-moParent-moSolo-moParent+moCheckingOpen);
  CPPUNIT_ASSERT(networth_q.m_grid.m_total[4] == -moSolo-moParent-moSolo-moParent+moCheckingOpen+moCreditOpen);
  CPPUNIT_ASSERT(networth_q.m_grid.m_total[5] == -moSolo-moParent-moSolo-moParent+moCheckingOpen+moCreditOpen);
  CPPUNIT_ASSERT(networth_q.m_grid.m_total[6] == -moSolo-moSolo-moParent-moSolo-moParent+moCheckingOpen+moCreditOpen);
  CPPUNIT_ASSERT(networth_q.m_grid.m_total[7] == -moParent1-moSolo-moSolo-moParent-moSolo-moParent+moCheckingOpen+moCreditOpen);
  CPPUNIT_ASSERT(networth_q.m_grid.m_total[8] == -moParent2-moParent1-moSolo-moSolo-moParent-moSolo-moParent+moCheckingOpen+moCreditOpen);
  CPPUNIT_ASSERT(networth_q.m_grid.m_total[9] == -moParent2-moParent1-moSolo-moSolo-moParent-moSolo-moParent+moCheckingOpen+moCreditOpen);
  
  filter.setRowType( ReportConfigurationFilter::eExpenseIncome );
  filter.setColumnType(ReportConfigurationFilter::eYears);
  PivotTable spending_y( filter );
  
  CPPUNIT_ASSERT(spending_y.m_grid.m_total[1] == moZero);
  CPPUNIT_ASSERT(spending_y.m_grid.m_total[2] == -moSolo-moParent-moSolo-moParent);
  CPPUNIT_ASSERT(spending_y.m_grid.m_total[3] == -moSolo-moParent);
  CPPUNIT_ASSERT(spending_y.m_grid.m_total.m_total == -moSolo-moParent-moSolo-moParent-moSolo-moParent);

  filter.setRowType( ReportConfigurationFilter::eAssetLiability );
  PivotTable networth_y( filter );

  CPPUNIT_ASSERT(networth_y.m_grid.m_total[1] == moZero);
  CPPUNIT_ASSERT(networth_y.m_grid.m_total[2] == -moSolo-moParent-moSolo-moParent+moCheckingOpen+moCreditOpen);
  CPPUNIT_ASSERT(networth_y.m_grid.m_total[3] == -moSolo-moParent-moSolo-moParent-moSolo-moParent+moCheckingOpen+moCreditOpen);

}

