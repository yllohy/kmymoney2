/***************************************************************************
                          kreportsviewtest.cpp
                          -------------------
    copyright            : (C) 2002 by Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
                           Ace Jones <ace.jones@hotpop.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qvaluelist.h"
 
#include "kreportsviewtest.h"

#define private public
#include "pivottable.h"
#undef private
using namespace reports;

#include "../mymoney/mymoneyequity.h"
#include "../mymoney/storage/mymoneystoragedump.h"

KReportsViewTest::KReportsViewTest()
{
}

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
}

void KReportsViewTest::tearDown ()
{
	file->detachStorage(storage);
	delete file;
	delete storage;
}

void makeTransaction( const QDate& _date, const QCString& _action, MyMoneyMoney _value, const QCString& _accountid, const QCString& _categoryid, const QCString& _currencyid ="" )
{
    MyMoneyPayee payeeTest = MyMoneyFile::instance()->payeeByName("Test Payee");

    MyMoneyTransaction transaction;
    transaction.setPostDate(_date);

    QCString currencyid = _currencyid;
    if ( currencyid != "" )
        transaction.setCommodity(currencyid);
    else
      currencyid=MyMoneyFile::instance()->baseCurrency().id();

    MyMoneyMoney price;
    MyMoneySplit splitLeft;
    splitLeft.setPayeeId(payeeTest.id());
    splitLeft.setAction(_action);
    splitLeft.setValue(-_value);
    splitLeft.setShares(-_value); // assumes transaction currency == this account's currency!
    splitLeft.setAccountId(_accountid);
    transaction.addSplit(splitLeft);

    MyMoneySplit splitRight;
    splitRight.setPayeeId(payeeTest.id());
    splitRight.setAction(_action);
    splitRight.setValue(_value);
    price = MyMoneyFile::instance()->currency(currencyid).price(_date);
    splitRight.setShares(_value * price );
    splitRight.setAccountId(_categoryid);
    transaction.addSplit(splitRight);

    MyMoneyFile::instance()->addTransaction(transaction);
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

void KReportsViewTest::testTest()
{
  const MyMoneyMoney moCheckingOpen(1418.0);
  const MyMoneyMoney moCreditOpen(-418.0);
  const MyMoneyMoney moZero(0.0);
  const MyMoneyMoney moSolo(234.12);
  const MyMoneyMoney moParent(88.01);
  const MyMoneyMoney moChild(14.00);

  QCString acAsset (file->asset().id());
  QCString acLiability (file->liability().id());
  QCString acExpense (file->expense().id());
  QCString acChecking = makeAccount(QString("Checking Account"),MyMoneyAccount::Checkings,moCheckingOpen,QDate(2004,5,15),acAsset);
  QCString acCredit = makeAccount(QString("Credit Card"),MyMoneyAccount::CreditCard,moCreditOpen,QDate(2004,7,15),acLiability);
  QCString acSolo = makeAccount(QString("Solo"),MyMoneyAccount::Expense,0,QDate(2004,1,11),acExpense);
  QCString acParent = makeAccount(QString("Parent"),MyMoneyAccount::Expense,0,QDate(2004,1,11),acExpense);
  QCString acChild = makeAccount(QString("Child"),MyMoneyAccount::Expense,0,QDate(2004,2,11),acParent);
  QCString acForeign = makeAccount(QString("Foreign"),MyMoneyAccount::Expense,0,QDate(2004,1,11),acExpense);

  // testNetWorthSingle
  {
    PivotTable networth( ReportConfiguration(ReportConfiguration::eAssetLiability,QDate(2004,1,1),QDate(2004,7,1)) );

  	CPPUNIT_ASSERT(networth.m_grid["Asset"]["Checking Account"][acChecking][5]==moCheckingOpen);
  	CPPUNIT_ASSERT(networth.m_grid["Asset"]["Checking Account"][acChecking][6]==moCheckingOpen);
  	CPPUNIT_ASSERT(networth.m_grid["Asset"]["Checking Account"].m_total[5]==moCheckingOpen);
  	CPPUNIT_ASSERT(networth.m_grid.m_total[0]==moZero);
  	CPPUNIT_ASSERT(networth.m_grid.m_total[4]==moZero);
  	CPPUNIT_ASSERT(networth.m_grid.m_total[5]==moCheckingOpen);
  	CPPUNIT_ASSERT(networth.m_grid.m_total[6]==moCheckingOpen);
  }

  // testNetWorthOfsetting
  {
    PivotTable networth( ReportConfiguration(ReportConfiguration::eAssetLiability,QDate(2004,1,1),QDate(2005,1,1)) );

  	CPPUNIT_ASSERT(networth.m_grid["Liability"]["Credit Card"][acCredit][7]==moCreditOpen);
  	CPPUNIT_ASSERT(networth.m_grid["Liability"]["Credit Card"][acCredit][12]==moCreditOpen);
  	CPPUNIT_ASSERT(networth.m_grid["Liability"]["Credit Card"].m_total[7]==moCreditOpen);
  	CPPUNIT_ASSERT(networth.m_grid["Liability"].m_total[7]==moCreditOpen);
  	CPPUNIT_ASSERT(networth.m_grid["Asset"]["Checking Account"][acChecking][7]==moCheckingOpen);
  	CPPUNIT_ASSERT(networth.m_grid["Asset"]["Checking Account"][acChecking][12]==moCheckingOpen);
  	CPPUNIT_ASSERT(networth.m_grid["Asset"].m_total[12]==moCheckingOpen);
  	CPPUNIT_ASSERT(networth.m_grid.m_total[0]==moZero);
  	CPPUNIT_ASSERT(networth.m_grid.m_total[4]==moZero);
  	CPPUNIT_ASSERT(networth.m_grid.m_total[5]==moCheckingOpen);
  	CPPUNIT_ASSERT(networth.m_grid.m_total[6]==moCheckingOpen);
  	CPPUNIT_ASSERT(networth.m_grid.m_total[7]==moCheckingOpen+moCreditOpen);
  	CPPUNIT_ASSERT(networth.m_grid.m_total[12]==moCheckingOpen+moCreditOpen);
  }

  // testNetWorthOpening Balances
  {
    PivotTable networth( ReportConfiguration(ReportConfiguration::eAssetLiability,QDate(2004,8,1),QDate(2005,9,1)) );


  	CPPUNIT_ASSERT(networth.m_grid["Liability"]["Credit Card"].m_total[0]==moCreditOpen);
  	CPPUNIT_ASSERT(networth.m_grid["Asset"]["Checking Account"].m_total[0]==moCheckingOpen);
  	CPPUNIT_ASSERT(networth.m_grid.m_total[0]==moCheckingOpen+moCreditOpen);
  	CPPUNIT_ASSERT(networth.m_grid.m_total[1]==moCheckingOpen+moCreditOpen);
  }

  // testNetWorthDateFilter
  {
    // This report is looking for the period PRIOR to any transactions or openings, so it
    // should all be moZero.
    PivotTable networth( ReportConfiguration(ReportConfiguration::eAssetLiability,QDate(2004,1,1),QDate(2004,2,1)) );

  	CPPUNIT_ASSERT(networth.m_grid["Liability"]["Credit Card"].m_total[0]==moZero);
  	CPPUNIT_ASSERT(networth.m_grid["Asset"]["Checking Account"].m_total[0]==moZero);
  	CPPUNIT_ASSERT(networth.m_grid.m_total[0]==moZero);
  	CPPUNIT_ASSERT(networth.m_grid.m_total[1]==moZero);
  }

  // testSpendingEmpty
  {
    PivotTable spending( ReportConfiguration(ReportConfiguration::eExpenseIncome,QDate(2004,1,1),QDate(2005,1,1)) );

    CPPUNIT_ASSERT(spending.m_grid.m_total.m_total==moZero);
  }

  // testTransaction
  {
    // Test a single transaction
    makeTransaction( QDate(2004,10,31), MyMoneySplit::ActionWithdrawal,moSolo, acChecking, acSolo );
    PivotTable spending( ReportConfiguration(ReportConfiguration::eExpenseIncome,QDate(2004,9,1),QDate(2005,1,1)) );

  	CPPUNIT_ASSERT(spending.m_grid["Expense"]["Solo"][acSolo][2]==(-moSolo));
  	CPPUNIT_ASSERT(spending.m_grid["Expense"]["Solo"].m_total[2]==(-moSolo));
  	CPPUNIT_ASSERT(spending.m_grid["Expense"]["Solo"].m_total[1]==moZero);
  	CPPUNIT_ASSERT(spending.m_grid.m_total[2]==(-moSolo));
  	CPPUNIT_ASSERT(spending.m_grid.m_total.m_total==(-moSolo));

    PivotTable networth( ReportConfiguration(ReportConfiguration::eAssetLiability,QDate(2004,9,1),QDate(2005,1,1)) );

    CPPUNIT_ASSERT(networth.m_grid["Asset"]["Checking Account"].m_total[2]==(moCheckingOpen-moSolo) );

    // Test a sub-account with a value, under an account with a value
    {
      makeTransaction( QDate(2004,11,7), MyMoneySplit::ActionWithdrawal, moParent, acCredit, acParent );
      makeTransaction( QDate(2004,11,7), MyMoneySplit::ActionWithdrawal, moChild, acCredit, acChild );
      PivotTable spending( ReportConfiguration(ReportConfiguration::eExpenseIncome,QDate(2004,9,1),QDate(2005,1,1)) );

    	CPPUNIT_ASSERT(spending.m_grid["Expense"]["Parent"][acParent][3]==(-moParent));
    	CPPUNIT_ASSERT(spending.m_grid["Expense"]["Parent"][acChild][3]==(-moChild));
    	CPPUNIT_ASSERT(spending.m_grid["Expense"]["Parent"].m_total[3]==(-moParent-moChild));
    	CPPUNIT_ASSERT(spending.m_grid["Expense"]["Parent"].m_total[2]==moZero);
    	CPPUNIT_ASSERT(spending.m_grid["Expense"]["Parent"].m_total.m_total==(-moParent-moChild));
    	CPPUNIT_ASSERT(spending.m_grid.m_total[3]==(-moParent-moChild));
    	CPPUNIT_ASSERT(spending.m_grid.m_total.m_total==(-moParent-moChild-moSolo));

      PivotTable networth( ReportConfiguration(ReportConfiguration::eAssetLiability,QDate(2004,9,1),QDate(2005,1,1)) );

      CPPUNIT_ASSERT(networth.m_grid["Liability"]["Credit Card"].m_total[3]==-moParent-moChild+moCreditOpen );
      CPPUNIT_ASSERT(networth.m_grid.m_total[4] == -moParent-moChild+moCreditOpen-moSolo+moCheckingOpen );
    }

    // Test that removing an income/spending account will remove the entry from an income/spending report
    {
      ReportConfiguration config(ReportConfiguration::eExpenseIncome,QDate(2004,9,1),QDate(2005,1,1));
      config.setIncludesAccount(acChecking,true);
      config.setIncludesAccount(acCredit,true);
      config.setIncludesAccount(acParent,false);
      config.setIncludesAccount(acChild,true);
      config.setIncludesAccount(acSolo,true);
      PivotTable spending( config );

    	CPPUNIT_ASSERT(spending.m_grid["Expense"]["Parent"].m_total[3]==-moChild);
    	CPPUNIT_ASSERT(spending.m_grid["Expense"].m_total[2]==-moSolo);
    	CPPUNIT_ASSERT(spending.m_grid.m_total.m_total==-moSolo-moChild);
    }
    
    // Test that removing an asset/liability account will remove the entry from an asset/liability report
    {
      ReportConfiguration config(ReportConfiguration::eAssetLiability,QDate(2004,9,1),QDate(2005,1,1));
      config.setIncludesAccount(acChecking,true);
      config.setIncludesAccount(acCredit,false);
      config.setIncludesAccount(acParent,true);
      config.setIncludesAccount(acChild,true);
      config.setIncludesAccount(acSolo,true);
      PivotTable networth( config );

      CPPUNIT_ASSERT(networth.m_grid.m_total[3] == -moSolo+moCheckingOpen );
    }

    // Test that removing an asset/liability account will remove the entry from an income/spending report
    {
      ReportConfiguration config(ReportConfiguration::eExpenseIncome,QDate(2004,9,1),QDate(2005,1,1));
      config.setIncludesAccount(acChecking,true);
      config.setIncludesAccount(acCredit,false);
      config.setIncludesAccount(acParent,true);
      config.setIncludesAccount(acChild,true);
      config.setIncludesAccount(acSolo,true);
      PivotTable spending( config );

    	CPPUNIT_ASSERT(spending.m_grid["Expense"].m_total[3]==moZero);
    	CPPUNIT_ASSERT(spending.m_grid["Expense"].m_total[2]==-moSolo);
    	CPPUNIT_ASSERT(spending.m_grid.m_total.m_total==-moSolo);
    }    
    
    /*
    fprintf(stderr,"%s\n", spending.renderHTML(false).latin1() );
    fprintf(stderr,"%s\n", networth.renderHTML(false).latin1() );

    QFile g( "kmymoney2.test.dump" );
    g.open( IO_WriteOnly );
    QDataStream st(&g);
    MyMoneyStorageDump dumper;
    dumper.writeStream(st, dynamic_cast<IMyMoneySerialize*> (MyMoneyFile::instance()->storage()));
    g.close();
    */
  }

  // testMultipleCurrencies
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

    makeTransaction( QDate(2004,2,20), MyMoneySplit::ActionWithdrawal,MyMoneyMoney(moJpyTransaction), acJpyChecking, acJpyCash, "JPY" );
    makeTransaction( QDate(2004,3,20), MyMoneySplit::ActionWithdrawal,MyMoneyMoney(moJpyTransaction), acJpyChecking, acJpyCash, "JPY" );
    makeTransaction( QDate(2004,4,20), MyMoneySplit::ActionWithdrawal,MyMoneyMoney(moJpyTransaction), acJpyChecking, acJpyCash, "JPY" );
    makeTransaction( QDate(2004,2,20), MyMoneySplit::ActionWithdrawal,MyMoneyMoney(moCanTransaction), acCanChecking, acCanCash, "CAD" );
    makeTransaction( QDate(2004,3,20), MyMoneySplit::ActionWithdrawal,MyMoneyMoney(moCanTransaction), acCanChecking, acCanCash, "CAD" );
    makeTransaction( QDate(2004,4,20), MyMoneySplit::ActionWithdrawal,MyMoneyMoney(moCanTransaction), acCanChecking, acCanCash, "CAD" );

    PivotTable spending( ReportConfiguration(ReportConfiguration::eExpenseIncome,QDate(2004,1,1),QDate(2005,1,1)) );

    // test single foreign currency
    CPPUNIT_ASSERT(spending.m_grid["Expense"]["Foreign"][acCanCash][2]==(-moCanTransaction*moCanPrice));
    CPPUNIT_ASSERT(spending.m_grid["Expense"]["Foreign"][acCanCash][3]==(-moCanTransaction*moCanPrice));
    CPPUNIT_ASSERT(spending.m_grid["Expense"]["Foreign"][acCanCash][4]==(-moCanTransaction*moCanPrice));

    // test multiple foreign currencies under a common parent
    CPPUNIT_ASSERT(spending.m_grid["Expense"]["Foreign"][acJpyCash][2]==(-moJpyTransaction*moJpyPrice));
    CPPUNIT_ASSERT(spending.m_grid["Expense"]["Foreign"][acJpyCash][3]==(-moJpyTransaction*moJpyPrice));
    CPPUNIT_ASSERT(spending.m_grid["Expense"]["Foreign"][acJpyCash][4]==(-moJpyTransaction*moJpyPrice));
    CPPUNIT_ASSERT(spending.m_grid["Expense"]["Foreign"].m_total[2]==(-moJpyTransaction*moJpyPrice-moCanTransaction*moCanPrice));
    CPPUNIT_ASSERT(spending.m_grid["Expense"]["Foreign"].m_total.m_total==(-moJpyTransaction*moJpyPrice-moCanTransaction*moCanPrice-moJpyTransaction*moJpyPrice-moCanTransaction*moCanPrice-moJpyTransaction*moJpyPrice-moCanTransaction*moCanPrice));

    PivotTable networth( ReportConfiguration(ReportConfiguration::eAssetLiability,QDate(2004,1,1),QDate(2005,1,1)) );

    // test single foreign currency
    CPPUNIT_ASSERT(networth.m_grid["Asset"]["Canadian Checking"][acCanChecking][1]==(moCanOpening*moCanPrice));
    CPPUNIT_ASSERT(networth.m_grid["Asset"]["Canadian Checking"][acCanChecking][2]==((moCanOpening-moCanTransaction)*moCanPrice));
    CPPUNIT_ASSERT(networth.m_grid["Asset"]["Canadian Checking"][acCanChecking][3]==((moCanOpening-moCanTransaction-moCanTransaction)*moCanPrice));
    CPPUNIT_ASSERT(networth.m_grid["Asset"]["Canadian Checking"][acCanChecking][4]==((moCanOpening-moCanTransaction-moCanTransaction-moCanTransaction)*moCanPrice));
    CPPUNIT_ASSERT(networth.m_grid["Asset"]["Canadian Checking"][acCanChecking][12]==((moCanOpening-moCanTransaction-moCanTransaction-moCanTransaction)*moCanPrice));

    // test Stable currency price, fluctuating account balance
    CPPUNIT_ASSERT(networth.m_grid["Asset"]["Japanese Checking"][acJpyChecking][1]==(moJpyOpening*moJpyPrice));
    CPPUNIT_ASSERT(networth.m_grid["Asset"]["Japanese Checking"][acJpyChecking][2]==((moJpyOpening-moJpyTransaction)*moJpyPrice));
    CPPUNIT_ASSERT(networth.m_grid["Asset"]["Japanese Checking"][acJpyChecking][3]==((moJpyOpening-moJpyTransaction-moJpyTransaction)*moJpyPrice));
    CPPUNIT_ASSERT(networth.m_grid["Asset"]["Japanese Checking"][acJpyChecking][4]==((moJpyOpening-moJpyTransaction-moJpyTransaction-moJpyTransaction)*moJpyPrice));

    // test Fluctuating currency price, stable account balance
    CPPUNIT_ASSERT(networth.m_grid["Asset"]["Japanese Checking"][acJpyChecking][5]==((moJpyOpening-moJpyTransaction-moJpyTransaction-moJpyTransaction)*moJpyPrice2));
    CPPUNIT_ASSERT(networth.m_grid["Asset"]["Japanese Checking"][acJpyChecking][6]==((moJpyOpening-moJpyTransaction-moJpyTransaction-moJpyTransaction)*moJpyPrice3));
    CPPUNIT_ASSERT(networth.m_grid["Asset"]["Japanese Checking"][acJpyChecking][7]==((moJpyOpening-moJpyTransaction-moJpyTransaction-moJpyTransaction)*moJpyPrice4));

    // test multiple currencies totalled up
    CPPUNIT_ASSERT(networth.m_grid["Asset"].m_total[4]==((moCanOpening-moCanTransaction-moCanTransaction-moCanTransaction)*moCanPrice)+((moJpyOpening-moJpyTransaction-moJpyTransaction-moJpyTransaction)*moJpyPrice));
    CPPUNIT_ASSERT(networth.m_grid["Asset"].m_total[5]==((moCanOpening-moCanTransaction-moCanTransaction-moCanTransaction)*moCanPrice)+((moJpyOpening-moJpyTransaction-moJpyTransaction-moJpyTransaction)*moJpyPrice2)+moCheckingOpen);
  }
    
}
 
