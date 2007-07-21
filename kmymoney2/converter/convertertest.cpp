/***************************************************************************
                          convertertest.cpp
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

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

#include "convertertest.h"

// uses helper functions from reports tests
#include "../reports/reportstestcommon.h"
using namespace test;

#include "../mymoney/mymoneysecurity.h"
#include "../mymoney/mymoneyprice.h"
#include "../mymoney/storage/mymoneystoragedump.h"
#include "../mymoney/mymoneyreport.h"
#include "../mymoney/mymoneystatement.h"
#include "../mymoney/storage/mymoneystoragexml.h"
#include "../dialogs/mymoneyofxconnector.h"
#include "../converter/mymoneyofxstatement.h"

#define private public
#include "../converter/webpricequote.h"
#undef private

ConverterTest::ConverterTest()
{
}

using namespace convertertest;

void ConverterTest::setUp () {

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
  acChecking = makeAccount("Checking Account",MyMoneyAccount::Checkings,moConverterCheckingOpen,QDate(2004,5,15),acAsset);
  acCredit = makeAccount("Credit Card",MyMoneyAccount::CreditCard,moConverterCreditOpen,QDate(2004,7,15),acLiability);
  acSolo = makeAccount("Solo",MyMoneyAccount::Expense,0,QDate(2004,1,11),acExpense);
  acParent = makeAccount("Parent",MyMoneyAccount::Expense,0,QDate(2004,1,11),acExpense);
  acChild = makeAccount("Child",MyMoneyAccount::Expense,0,QDate(2004,2,11),acParent);
  acForeign = makeAccount("Foreign",MyMoneyAccount::Expense,0,QDate(2004,1,11),acExpense);
  
  MyMoneyInstitution i("Bank of the World","","","","","","");
  file->addInstitution(i);
  inBank = i.id();
  ft.commit();
}

void ConverterTest::tearDown ()
{
  file->detachStorage(storage);
  delete storage;
}

void ConverterTest::testOfxImport(void)
{
// These tests don't work now that the OFX importing logic is in a plugin.
#if 0
  // Of course, the TRUE test will be to import these back through 
  // MyMoneyStatementReader into transactions, HOWEVER, that class currently
  // has UI dependencies, so it's a task for the future.

  //
  // Bank Statement
  //

  try
  {
    MyMoneyFile* file = MyMoneyFile::instance();
    
    TransactionHelper t1q1( QDate(2004,1,1), MyMoneySplit::ActionWithdrawal, moSolo, acChecking, acSolo );
    TransactionHelper t2q1( QDate(2004,2,1), MyMoneySplit::ActionWithdrawal, moParent1, acChecking, acParent );
    TransactionHelper t3q1( QDate(2004,3,1), MyMoneySplit::ActionWithdrawal, moParent2, acChecking, acParent );
    TransactionHelper t4y1( QDate(2004,11,7), MyMoneySplit::ActionWithdrawal, moChild, acChecking, acChild, QCString(), "Thomas Baumgart" );
    
    t1q1.setMemo("Memo");
    t1q1.update();
  
    MyMoneyAccount a = file->account(acChecking);
    a.setNumber(a.id());
    a.setInstitutionId(inBank);
  
    QByteArray ofxResponse = MyMoneyOfxConnector(a).statementResponse(QDate(2004,1,1));
    
    QFile ofxfile("bank.ofx");
    if ( ofxfile.open( IO_WriteOnly) )
    {
      QTextStream(&ofxfile) << QString(ofxResponse);
      ofxfile.close();
    }
    else
      CPPUNIT_FAIL("Could not open bank.ofx for writing");    
      
    MyMoneyOfxStatement os("bank.ofx");
  
    CPPUNIT_ASSERT(os.isValid());
    CPPUNIT_ASSERT(os.count() == 1);
    
    MyMoneyStatement& s = os.back();
    
    CPPUNIT_ASSERT(s.m_strAccountNumber == QString("123456789  %1").arg(a.id()));
    CPPUNIT_ASSERT(s.m_strAccountName == QString("Bank account %1").arg(a.id()));
    CPPUNIT_ASSERT(s.m_dateBegin == QDate(2004,1,1) );
    CPPUNIT_ASSERT(s.m_dateEnd == QDate::currentDate() );
    CPPUNIT_ASSERT(s.m_eType == MyMoneyStatement::etCheckings );
    CPPUNIT_ASSERT(s.m_strCurrency == "USD" );
    CPPUNIT_ASSERT(s.m_listTransactions.count() == 5);
    
    MyMoneyStatement::Transaction& t1 = s.m_listTransactions.front();
    MyMoneyStatement::Transaction& t4 = s.m_listTransactions.back();
    
    CPPUNIT_ASSERT(t1.m_strPayee == "Test Payee" );
    CPPUNIT_ASSERT(t1.m_strMemo == "Memo" );
    CPPUNIT_ASSERT(MyMoneyMoney(t1.m_moneyAmount) == -moSolo );
    CPPUNIT_ASSERT(t1.m_datePosted == t1q1.postDate() );
    CPPUNIT_ASSERT(t1.m_strBankID == QString("ID %1").arg(t1q1.id()) );
    
    CPPUNIT_ASSERT(t4.m_strPayee == "Thomas Baumgart" );
    CPPUNIT_ASSERT(t4.m_strMemo == "Thomas Baumgart" );
    CPPUNIT_ASSERT(MyMoneyMoney(t4.m_moneyAmount) == -moChild );
    CPPUNIT_ASSERT(t4.m_datePosted == t4y1.postDate() );
    CPPUNIT_ASSERT(t4.m_strBankID == QString("ID %1").arg(t4y1.id()) );
  }
  catch(MyMoneyException *e) 
  {
    CPPUNIT_FAIL(e->what());
    delete e;
  }

  //
  // Credit Card Statement
  //

  try
  {
    MyMoneyFile* file = MyMoneyFile::instance();
    
    TransactionHelper t1q1( QDate(2004,1,1), MyMoneySplit::ActionWithdrawal, moSolo, acCredit, acSolo );
    TransactionHelper t2q1( QDate(2004,2,1), MyMoneySplit::ActionWithdrawal, moParent1, acCredit, acParent );
    TransactionHelper t3q1( QDate(2004,3,1), MyMoneySplit::ActionWithdrawal, moParent2, acCredit, acParent );
    TransactionHelper t4y1( QDate(2004,11,7), MyMoneySplit::ActionWithdrawal, moChild, acCredit, acChild, QCString(), "Thomas Baumgart" );
  
    MyMoneyAccount a = file->account(acCredit);
    a.setNumber(a.id());
    a.setInstitutionId(inBank);
  
    QByteArray ofxResponse = MyMoneyOfxConnector(a).statementResponse(QDate(2004,1,1));
    
    QFile ofxfile("cc.ofx");
    if ( ofxfile.open( IO_WriteOnly) )
    {
      QTextStream(&ofxfile) << QString(ofxResponse);
      ofxfile.close();
    }
    else
      CPPUNIT_FAIL("Could not open cc.ofx for writing");    
      
    MyMoneyOfxStatement os("cc.ofx");
    MyMoneyStatement::writeXMLFile(os.back(),"cc.xml");
  
    CPPUNIT_ASSERT(os.isValid());
    CPPUNIT_ASSERT(os.count() == 1);
    
    MyMoneyStatement& s = os.back();
    
    CPPUNIT_ASSERT(s.m_strAccountNumber == QString("%1 ").arg(a.id()));
    CPPUNIT_ASSERT(s.m_strAccountName == QString("Credit card %1").arg(a.id()));
    CPPUNIT_ASSERT(s.m_dateBegin == QDate(2004,1,1) );
    CPPUNIT_ASSERT(s.m_dateEnd == QDate::currentDate() );
    CPPUNIT_ASSERT(s.m_eType == MyMoneyStatement::etCreditCard );
    CPPUNIT_ASSERT(s.m_strCurrency == "USD" );
    CPPUNIT_ASSERT(s.m_listTransactions.count() == 5);
    
    MyMoneyStatement::Transaction& t1 = s.m_listTransactions.front();
    MyMoneyStatement::Transaction& t4 = s.m_listTransactions.back();
    
    CPPUNIT_ASSERT(t1.m_strPayee == "Test Payee" );
    CPPUNIT_ASSERT(t1.m_strMemo == "Test Payee" );
    CPPUNIT_ASSERT(MyMoneyMoney(t1.m_moneyAmount) == -moSolo );
    CPPUNIT_ASSERT(t1.m_datePosted == t1q1.postDate() );
    CPPUNIT_ASSERT(t1.m_strBankID == QString("ID %1").arg(t1q1.id()) );
    
    CPPUNIT_ASSERT(t4.m_strPayee == "Thomas Baumgart" );
    CPPUNIT_ASSERT(t4.m_strMemo == "Thomas Baumgart" );
    CPPUNIT_ASSERT(MyMoneyMoney(t4.m_moneyAmount) == -moChild );
    CPPUNIT_ASSERT(t4.m_datePosted == t4y1.postDate() );
    CPPUNIT_ASSERT(t4.m_strBankID == QString("ID %1").arg(t4y1.id()) );
  }
  catch(MyMoneyException *e) 
  {
    CPPUNIT_FAIL(e->what());
    delete e;
  }

  //
  // Investment Statement
  //

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
    //                         Date             Action                              Shares    Price   Stock     Asset       Income
    InvTransactionHelper s1b1( QDate(2004,2,1), MyMoneySplit::ActionBuyShares,      1000.00, 100.00, acStock1, acChecking, QCString() );
    InvTransactionHelper s1b2( QDate(2004,3,1), MyMoneySplit::ActionBuyShares,      1000.00, 110.00, acStock1, acChecking, QCString() );
    InvTransactionHelper s1s1( QDate(2004,4,1), MyMoneySplit::ActionBuyShares,      -200.00, 120.00, acStock1, acChecking, QCString() );
    InvTransactionHelper s1s2( QDate(2004,5,1), MyMoneySplit::ActionBuyShares,      -200.00, 100.00, acStock1, acChecking, QCString() );
    InvTransactionHelper s1r1( QDate(2004,6,1), MyMoneySplit::ActionReinvestDividend, 50.00, 100.00, acStock1, QCString(), acDividends );
    InvTransactionHelper s1r2( QDate(2004,7,1), MyMoneySplit::ActionReinvestDividend, 50.00,  80.00, acStock1, QCString(), acDividends );
    InvTransactionHelper s1c1( QDate(2004,8,1), MyMoneySplit::ActionDividend,         10.00, 100.00, acStock1, acChecking, acDividends );
    InvTransactionHelper s1c2( QDate(2004,9,1), MyMoneySplit::ActionDividend,         10.00, 120.00, acStock1, acChecking, acDividends );

    MyMoneyAccount a = file->account(acInvestment);
    a.setNumber(a.id());
    a.setInstitutionId(inBank);
  
    QByteArray ofxResponse = MyMoneyOfxConnector(a).statementResponse(QDate(2004,1,1));
    
    QFile ofxfile("inv.ofx");
    if ( ofxfile.open( IO_WriteOnly) )
    {
      QTextStream(&ofxfile) << QString(ofxResponse);
      ofxfile.close();
    }
    else
      CPPUNIT_FAIL("Could not open inv.ofx for writing");    
      
    MyMoneyOfxStatement os("inv.ofx");
    MyMoneyStatement::writeXMLFile(os.back(),"inv.xml");
  
    CPPUNIT_ASSERT(os.isValid());
    CPPUNIT_ASSERT(os.count() == 1);
    
    MyMoneyStatement& s = os.back();
  
    // For some reason, libofx doesn't seem to like the dtstart/end parameters
    // from the OFX file, so they don't come through here.
//     CPPUNIT_ASSERT(s.m_dateBegin == QDate(2004,1,1) );
//     CPPUNIT_ASSERT(s.m_dateEnd == QDate::currentDate() );
    CPPUNIT_ASSERT(s.m_eType == MyMoneyStatement::etInvestment );
    CPPUNIT_ASSERT(s.m_listTransactions.count() == 8);
    
    MyMoneyStatement::Transaction& t1 = s.m_listTransactions[1];
    MyMoneyStatement::Transaction& t3 = s.m_listTransactions[3];
    MyMoneyStatement::Transaction& t5 = s.m_listTransactions[5];
    MyMoneyStatement::Transaction& t7 = s.m_listTransactions[7];
    
    CPPUNIT_ASSERT(MyMoneyMoney(t1.m_dShares) == MyMoneyMoney(1000,1) );
    CPPUNIT_ASSERT(MyMoneyMoney(t1.m_moneyAmount) == MyMoneyMoney(110000,1) );
    CPPUNIT_ASSERT(t1.m_datePosted == s1b2.postDate() );
    CPPUNIT_ASSERT(t1.m_strBankID == QString("ID %1").arg(s1b2.id()) );
    CPPUNIT_ASSERT(t1.m_eAction == MyMoneyStatement::Transaction::eaBuy );
    
    CPPUNIT_ASSERT(MyMoneyMoney(t3.m_dShares) == MyMoneyMoney(-200,1) );
    CPPUNIT_ASSERT(MyMoneyMoney(t3.m_moneyAmount) == MyMoneyMoney(-20000,1) );
    CPPUNIT_ASSERT(t3.m_datePosted == s1s2.postDate() );
    CPPUNIT_ASSERT(t3.m_strBankID == QString("ID %1").arg(s1s2.id()) );
    CPPUNIT_ASSERT(t3.m_eAction == MyMoneyStatement::Transaction::eaSell );

    CPPUNIT_ASSERT(MyMoneyMoney(t5.m_dShares) == MyMoneyMoney(50,1) );
    CPPUNIT_ASSERT(MyMoneyMoney(t5.m_moneyAmount) == MyMoneyMoney(4000,1) );
    CPPUNIT_ASSERT(t5.m_datePosted == s1r2.postDate() );
    CPPUNIT_ASSERT(t5.m_strBankID == QString("ID %1").arg(s1r2.id()) );
    CPPUNIT_ASSERT(t5.m_eAction == MyMoneyStatement::Transaction::eaReinvestDividend );

    CPPUNIT_ASSERT(MyMoneyMoney(t7.m_dShares).isZero() );
    CPPUNIT_ASSERT(MyMoneyMoney(t7.m_moneyAmount) == MyMoneyMoney(-1200,1) );
    CPPUNIT_ASSERT(t7.m_datePosted == s1c2.postDate() );
    CPPUNIT_ASSERT(t7.m_strBankID == QString("ID %1").arg(s1c2.id()) );
    CPPUNIT_ASSERT(t7.m_eAction == MyMoneyStatement::Transaction::eaCashDividend );
  }
  catch(MyMoneyException *e) 
  {
    CPPUNIT_FAIL(e->what());
    delete e;
  }
  
  //
  // Multiple Accounts
  //

  // Unfortunately, I'm not even clear what this LOOKS like :-(
#endif    
}

void ConverterTest::testWebQuotes()
{
#ifdef PERFORM_ONLINE_TESTS
  try
  {
    WebPriceQuote q;
    QuoteReceiver qr(&q);
    
    q.launch("DIS");
    
//    kdDebug(2) << "ConverterTest::testWebQuotes(): quote for " << q.m_symbol << " on " << qr.m_date.toString() << " is " << qr.m_price.toString() << " errors(" << qr.m_errors.count() << "): " << qr.m_errors.join(" /// ") << endl;
    
    // No errors allowed
    CPPUNIT_ASSERT(qr.m_errors.count() == 0);
    
    // Quote date should be within the last week, or something bad is going on.
    CPPUNIT_ASSERT(qr.m_date <= QDate::currentDate());
    CPPUNIT_ASSERT(qr.m_date >= QDate::currentDate().addDays(-7));
    
    // Quote value should at least be positive
    CPPUNIT_ASSERT(qr.m_price.isPositive());
    
    q.launch("MF8AAUKS.L","Yahoo UK");
    
//    kdDebug(2) << "ConverterTest::testWebQuotes(): quote for " << q.m_symbol << " on " << qr.m_date.toString() << " is " << qr.m_price.toString() << " errors(" << qr.m_errors.count() << "): " << qr.m_errors.join(" /// ") << endl;
        
    CPPUNIT_ASSERT(qr.m_errors.count() == 0);
    CPPUNIT_ASSERT(qr.m_date <= QDate::currentDate().addDays(1));
    CPPUNIT_ASSERT(qr.m_date >= QDate::currentDate().addDays(-7));
    CPPUNIT_ASSERT(qr.m_price.isPositive());
    
    q.launch("EUR > USD","Yahoo Currency");

//    kdDebug(2) << "ConverterTest::testWebQuotes(): quote for " << q.m_symbol << " on " << qr.m_date.toString() << " is " << qr.m_price.toString() << " errors(" << qr.m_errors.count() << "): " << qr.m_errors.join(" /// ") << endl;
    
    CPPUNIT_ASSERT(qr.m_errors.count() == 0);
    CPPUNIT_ASSERT(qr.m_date <= QDate::currentDate().addDays(1));
    CPPUNIT_ASSERT(qr.m_date >= QDate::currentDate().addDays(-7));
    CPPUNIT_ASSERT(qr.m_price.isPositive());
  
    q.launch("50492","Globe & Mail");

//    kdDebug(2) << "ConverterTest::testWebQuotes(): quote for " << q.m_symbol << " on " << qr.m_date.toString() << " is " << qr.m_price.toString() << " errors(" << qr.m_errors.count() << "): " << qr.m_errors.join(" /// ") << endl;
    
    CPPUNIT_ASSERT(qr.m_errors.count() == 0);
    CPPUNIT_ASSERT(qr.m_date <= QDate::currentDate().addDays(1));
    CPPUNIT_ASSERT(qr.m_date >= QDate::currentDate().addDays(-7));
    CPPUNIT_ASSERT(qr.m_price.isPositive());
  
    q.launch("TDB647","MSN.CA");

//    kdDebug(2) << "ConverterTest::testWebQuotes(): quote for " << q.m_symbol << " on " << qr.m_date.toString() << " is " << qr.m_price.toString() << " errors(" << qr.m_errors.count() << "): " << qr.m_errors.join(" /// ") << endl;
    
    CPPUNIT_ASSERT(qr.m_errors.count() == 0);
    CPPUNIT_ASSERT(qr.m_date <= QDate::currentDate().addDays(1));
    CPPUNIT_ASSERT(qr.m_date >= QDate::currentDate().addDays(-7));
    CPPUNIT_ASSERT(qr.m_price.isPositive());
  
  }
  catch (MyMoneyException* e)
  {
    CPPUNIT_FAIL(e->what());
  }
#endif
}

void ConverterTest::testDateFormat()
{
  try
  {
    MyMoneyDateFormat format("%mm-%dd-%yyyy");
  
    CPPUNIT_ASSERT(format.convertString("1-5-2005") == QDate(2005,1,5));
    CPPUNIT_ASSERT(format.convertString("jan-15-2005") == QDate(2005,1,15));
    CPPUNIT_ASSERT(format.convertString("august-25-2005") == QDate(2005,8,25));

    format = MyMoneyDateFormat("%mm/%dd/%yy");

    CPPUNIT_ASSERT(format.convertString("1/5/05") == QDate(2005,1,5));
    CPPUNIT_ASSERT(format.convertString("jan/15/05") == QDate(2005,1,15));
    CPPUNIT_ASSERT(format.convertString("august/25/05") == QDate(2005,8,25));

    format = MyMoneyDateFormat("%d\\.%m\\.%yy");

    CPPUNIT_ASSERT(format.convertString("1.5.05") == QDate(2005,5,1));
    CPPUNIT_ASSERT(format.convertString("15.jan.05") == QDate(2005,1,15));
    CPPUNIT_ASSERT(format.convertString("25.august.05") == QDate(2005,8,25));

    format = MyMoneyDateFormat("%yyyy\\\\%dddd\\\\%mmmmmmmmmmm");
  
    CPPUNIT_ASSERT(format.convertString("2005\\31\\12") == QDate(2005,12,31));
    CPPUNIT_ASSERT(format.convertString("2005\\15\\jan") == QDate(2005,1,15));
    CPPUNIT_ASSERT(format.convertString("2005\\25\\august") == QDate(2005,8,25));
  
    format = MyMoneyDateFormat("%m %dd, %yyyy");
  
    CPPUNIT_ASSERT(format.convertString("jan 15, 2005") == QDate(2005,1,15));
    CPPUNIT_ASSERT(format.convertString("august 25, 2005") == QDate(2005,8,25));
    CPPUNIT_ASSERT(format.convertString("january 1st, 2005") == QDate(2005,1,1));

    format = MyMoneyDateFormat("%m %d %y");
  
    CPPUNIT_ASSERT(format.convertString("12/31/50",false,2000) == QDate(1950,12,31));
    CPPUNIT_ASSERT(format.convertString("1/1/90",false,2000) == QDate(1990,1,1));
    CPPUNIT_ASSERT(format.convertString("december 31st, 5",false) == QDate(2005,12,31));
  }
  catch (MyMoneyException* e)
  {
    CPPUNIT_FAIL(e->what());
  }
}

// vim:cin:si:ai:et:ts=2:sw=2:
