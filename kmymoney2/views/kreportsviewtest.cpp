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
#include <qdom.h>
#include <qfile.h>

#include <kdebug.h>
#include <kdeversion.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include "kreportsviewtest.h"

#define private public
#include "pivottable.h"
#include "querytable.h"
#undef private
using namespace reports;

#include "../mymoney/mymoneysecurity.h"
#include "../mymoney/mymoneyprice.h"
#include "../mymoney/storage/mymoneystoragedump.h"
#include "../mymoney/mymoneyreport.h"
#include "../mymoney/mymoneystatement.h"
#include "../mymoney/storage/mymoneystoragexml.h"

namespace reports {

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
    // price = MyMoneyFile::instance()->currency(currencyid).price(_date);
    price = MyMoneyFile::instance()->price(currencyid, QCString(), _date).rate();
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

InvTransactionHelper::InvTransactionHelper( const QDate& _date, const QCString& _action, MyMoneyMoney _shares, MyMoneyMoney _price, const QCString& _stockaccountid, const QCString& _transferid, const QCString& _categoryid )
{
  MyMoneyFile* file = MyMoneyFile::instance();
  MyMoneyAccount stockaccount = file->account(_stockaccountid);
  MyMoneyMoney value = _shares * _price;
  
  setPostDate(_date);

  setCommodity("USD");
  MyMoneySplit s1;
  s1.setValue(value);
  s1.setAccountId(_stockaccountid);

  if ( _action == MyMoneySplit::ActionReinvestDividend )
  {
    s1.setShares(_shares);
    s1.setAction(MyMoneySplit::ActionReinvestDividend);

    MyMoneySplit s2;
    s2.setAccountId(_categoryid);
    s2.setShares(-value);
    s2.setValue(-value);
    addSplit(s2);
  }
  else if ( _action == MyMoneySplit::ActionDividend )
  {
    s1.setAccountId(_categoryid);
    s1.setShares(-value);
    s1.setValue(-value);
    
    // Split 2 will be the zero-amount investment split that serves to
    // mark this transaction as a cash dividend and note which stock account
    // it belongs to.
    MyMoneySplit s2;
    s2.setValue(0);
    s2.setShares(0);
    s2.setAction(MyMoneySplit::ActionDividend);
    s2.setAccountId(_stockaccountid);
    addSplit(s2);

    MyMoneySplit s3;
    s3.setAccountId(_transferid);
    s3.setShares(value);
    s3.setValue(value);
    addSplit(s3);
  }
  else if ( _action == MyMoneySplit::ActionBuyShares )
  {
    s1.setShares(_shares);
    s1.setAction(MyMoneySplit::ActionBuyShares);
  
    MyMoneySplit s3;
    s3.setAccountId(_transferid);
    s3.setShares(-value);
    s3.setValue(-value);
    addSplit(s3);
  }
  addSplit(s1);

  //kdDebug(2) << "created transaction, now adding..." << endl;
  
  file->addTransaction(*this);

  //kdDebug(2) << "updating price..." << endl;
    
  // update the price, while we're here
  QCString stockid = stockaccount.currencyId();
  QCString basecurrencyid = file->baseCurrency().id();
  MyMoneyPrice price = file->price( stockid, basecurrencyid, _date, true );
  if ( !price.isValid() )
  {
    MyMoneyPrice newprice( stockid, basecurrencyid, _date, _price, "test" );  
    file->addPrice(newprice);
  }

  //kdDebug(2) << "successfully added " << id() << endl;
}

} // end namespace reports

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

void makePrice(const QCString& _currencyid, const QDate& _date, const MyMoneyMoney& _price )
{
  MyMoneyFile* file = MyMoneyFile::instance();
  MyMoneySecurity curr = file->currency(_currencyid);
  MyMoneyPrice price(file->baseCurrency().id(), _currencyid, _date, _price, "test");
  file->addPrice(price);
}

QCString makeEquity(const QString& _name, const QString& _symbol )
{
  MyMoneySecurity equity;

  equity.setName( _name );
  equity.setTradingSymbol( _symbol );
  equity.setSmallestAccountFraction( 1000 );
  equity.setSecurityType( MyMoneySecurity::SECURITY_NONE /*MyMoneyEquity::ETYPE_STOCK*/ );
  MyMoneyFile::instance()->addSecurity( equity );
 
  return equity.id();
}

void makeEquityPrice(const QCString& _id, const QDate& _date, const MyMoneyMoney& _price )
{
  MyMoneyFile* file = MyMoneyFile::instance();
  QCString basecurrencyid = file->baseCurrency().id();
  MyMoneyPrice price = file->price( _id, basecurrencyid, _date, true );
  if ( !price.isValid() )
  {
    MyMoneyPrice newprice( _id, basecurrencyid, _date, _price, "test" );  
    file->addPrice(newprice);
  }
}

void writeRCFtoXMLDoc( const MyMoneyReport& filter, QDomDocument* doc )
{
 QDomProcessingInstruction instruct = doc->createProcessingInstruction(QString("xml"), QString("version=\"1.0\" encoding=\"utf-8\""));
  doc->appendChild(instruct);

  QDomElement root = doc->createElement("KMYMONEY-FILE");
  doc->appendChild(root);

  QDomElement reports = doc->createElement("REPORTS");
  root.appendChild(reports);

  QDomElement report = doc->createElement("REPORT");
  filter.write(report,doc);
  reports.appendChild(report);

}

void writeTabletoHTML( const PivotTable& table, const QString& _filename = QString() )
{
  static unsigned filenumber = 1;
  QString filename = _filename;
  if ( filename.isEmpty() )
  {
    filename = QString("report-%1%2.html").arg((filenumber<10)?"0":"").arg(filenumber);
    ++filenumber;
  }

  QFile g( filename );
  g.open( IO_WriteOnly );
  QTextStream(&g) << table.renderHTML();
  g.close();

}

void writeTabletoCSV( const PivotTable& table, const QString& _filename = QString() )
{
  static unsigned filenumber = 1;
  QString filename = _filename;
  if ( filename.isEmpty() )
  {
    filename = QString("report-%1%2.csv").arg((filenumber<10)?"0":"").arg(filenumber);
    ++filenumber;
  }

  QFile g( filename );
  g.open( IO_WriteOnly );
  QTextStream(&g) << table.renderCSV();
  g.close();

}

void writeTabletoCSV( const QueryTable& table, const QString& _filename = QString() )
{
  static unsigned filenumber = 1;
  QString filename = _filename;
  if ( filename.isEmpty() )
  {
    filename = QString("qreport-%1%2.csv").arg((filenumber<10)?"0":"").arg(filenumber);
    ++filenumber;
  }

  QFile g( filename );
  g.open( IO_WriteOnly );
  QTextStream(&g) << table.renderCSV();
  g.close();

}

void writeRCFtoXML( const MyMoneyReport& filter, const QString& _filename = QString() )
{
  static unsigned filenum = 1;
  QString filename = _filename;
  if ( filename.isEmpty() )
    filename = QString("report-%1%2.xml").arg((filenum<10)?"0":"").arg(filenum++);

  QDomDocument* doc = new QDomDocument("KMYMONEY-FILE");
  Q_CHECK_PTR(doc);

  writeRCFtoXMLDoc(filter,doc);

  QFile g( filename );
  g.open( IO_WriteOnly );

  QTextStream stream(&g);
#if KDE_IS_VERSION(3,2,0)
  stream.setEncoding(QTextStream::UnicodeUTF8);
  stream << doc->toString();
#else
  //stream.setEncoding(QTextStream::Locale);
  QCString temp = doc->toCString();
  stream << temp.data();
#endif
  g.close();

  delete doc;
}

bool readRCFfromXMLDoc( QValueList<MyMoneyReport>& list, QDomDocument* doc )
{
  bool result = false;

    QDomElement rootElement = doc->documentElement();
    if(!rootElement.isNull())
    {
      QDomNode child = rootElement.firstChild();
      while(!child.isNull() && child.isElement())
      {
        QDomElement childElement = child.toElement();
        if(QString("REPORTS") == childElement.tagName())
        {
    result = true;
          QDomNode subchild = child.firstChild();
          while(!subchild.isNull() && subchild.isElement())
          {
            MyMoneyReport filter;
      if ( filter.read(subchild.toElement()))
      {
              list += filter;
      }
      subchild = subchild.nextSibling();
         }
       }
       child = child.nextSibling();
      }
    }
  return result;
}

bool readRCFfromXML( QValueList<MyMoneyReport>& list, const QString& filename )
{
  int result = false;
  QFile f( filename );
  f.open( IO_ReadOnly );
  QDomDocument* doc = new QDomDocument;
  if(doc->setContent(&f, FALSE))
  {
    result = readRCFfromXMLDoc(list,doc);
  }
  delete doc;

  return result;

}

void XMLandback( MyMoneyReport& filter )
{
  // this function writes the filter to XML, and then reads
  // it back from XML overwriting the original filter;
  // in all cases, the result should be the same if the read
  // & write methods are working correctly.

  QDomDocument* doc = new QDomDocument("KMYMONEY-FILE");
  Q_CHECK_PTR(doc);

  writeRCFtoXMLDoc(filter,doc);
  QValueList<MyMoneyReport> list;
  if ( readRCFfromXMLDoc(list,doc) && list.count() > 0 )
    filter = list[0];
  else
    throw new MYMONEYEXCEPTION("Failed to load report from XML");

  delete doc;

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
QCString acIncome;
QCString acChecking;
QCString acCredit;
QCString acSolo;
QCString acParent;
QCString acChild;
QCString acForeign;
QCString acCanChecking;
QCString acJpyChecking;
QCString acCanCash;
QCString acJpyCash;
QCString inBank;
QCString eqStock1;
QCString eqStock2;
QCString acInvestment;
QCString acStock1;
QCString acStock2;
QCString acDividends;

void KReportsViewTest::setUp () {

  storage = new MyMoneySeqAccessMgr;
  file = MyMoneyFile::instance();
  file->attachStorage(storage);

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
  MyMoneyKeyValueContainer ofxsettings;
  ofxsettings.setValue("iban","123456789");
  i.setOfxConnectionSettings(ofxsettings);
  file->addInstitution(i);
  inBank = i.id();
}

void KReportsViewTest::tearDown ()
{
  file->detachStorage(storage);
  delete file;
  delete storage;
}

void KReportsViewTest::testNetWorthSingle()
{
  try
  {
    MyMoneyReport filter( MyMoneyReport::eAssetLiability );
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

void KReportsViewTest::testNetWorthOfsetting()
{
  // Test the net worth report to make sure it picks up the opening balance for two
  // accounts opened during the period of the report, one asset & one liability.  Test
  // that it calculates the totals correctly.

  MyMoneyReport filter( MyMoneyReport::eAssetLiability );
  filter.setDateFilter(QDate(2004,1,1),QDate(2005,1,1).addDays(-1));
  XMLandback(filter);
  PivotTable networth_f( filter );
  CPPUNIT_ASSERT(networth_f.m_grid["Liability"]["Credit Card"][acCredit][7]==moCreditOpen);
  CPPUNIT_ASSERT(networth_f.m_grid.m_total[0]==moZero);
  CPPUNIT_ASSERT(networth_f.m_grid.m_total[12]==moCheckingOpen+moCreditOpen);

}

void KReportsViewTest::testNetWorthOpeningPrior()
{
  // Test the net worth report to make sure it's picking up opening balances PRIOR to
  // the period of the report.

  MyMoneyReport filter( MyMoneyReport::eAssetLiability );
  filter.setDateFilter(QDate(2005,8,1),QDate(2005,12,31));
  filter.setName("Net Worth Opening Prior 1");
  XMLandback(filter);
  PivotTable networth_f( filter );
  writeTabletoCSV(networth_f);

  CPPUNIT_ASSERT(networth_f.m_grid["Liability"]["Credit Card"].m_total[0]==moCreditOpen);
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
  CPPUNIT_ASSERT(networth_f2.m_grid["Liability"]["Credit Card"].m_total[0]==moCreditOpen-moParent);
  CPPUNIT_ASSERT(networth_f2.m_grid["Asset"]["Checking Account"].m_total[0]==moCheckingOpen-moChild);
  CPPUNIT_ASSERT(networth_f2.m_grid.m_total[0]==moCheckingOpen+moCreditOpen-moChild-moParent);
}

void KReportsViewTest::testNetWorthDateFilter()
{
  // Test a net worth report whose period is prior to the time any accounts are open,
  // so the report should be zero.

  MyMoneyReport filter( MyMoneyReport::eAssetLiability );
  filter.setDateFilter(QDate(2004,1,1),QDate(2004,2,1).addDays(-1));
  XMLandback(filter);
  PivotTable networth_f( filter );
  CPPUNIT_ASSERT(networth_f.m_grid.m_total[1]==moZero);

}

void KReportsViewTest::testSpendingEmpty()
{
  // test a spending report with no entries

  MyMoneyReport filter( MyMoneyReport::eExpenseIncome );
  XMLandback(filter);
  PivotTable spending_f1( filter );
  CPPUNIT_ASSERT(spending_f1.m_grid.m_total.m_total==moZero);
  writeTabletoCSV(spending_f1);

  filter.setDateFilter(QDate(2004,9,1),QDate(2005,1,1).addDays(-1));
  PivotTable spending_f2( filter );
  CPPUNIT_ASSERT(spending_f2.m_grid.m_total.m_total==moZero);
}

void KReportsViewTest::testSingleTransaction()
{
  // Test a single transaction
  TransactionHelper t( QDate(2004,10,31), MyMoneySplit::ActionWithdrawal,moSolo, acChecking, acSolo );

  MyMoneyReport filter( MyMoneyReport::eExpenseIncome );
  filter.setDateFilter(QDate(2004,9,1),QDate(2005,1,1).addDays(-1));
  XMLandback(filter);
  PivotTable spending_f( filter );

  CPPUNIT_ASSERT(spending_f.m_grid["Expense"]["Solo"][acSolo][2]==(-moSolo));
  CPPUNIT_ASSERT(spending_f.m_grid["Expense"]["Solo"].m_total[2]==(-moSolo));
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

void KReportsViewTest::testSubAccount()
{
  // Test a sub-account with a value, under an account with a value

  TransactionHelper t1( QDate(2004,11,7), MyMoneySplit::ActionWithdrawal, moParent1, acCredit, acParent );
  TransactionHelper t2( QDate(2004,11,7), MyMoneySplit::ActionWithdrawal, moParent2, acCredit, acParent );
  TransactionHelper t3( QDate(2004,11,7), MyMoneySplit::ActionWithdrawal, moChild, acCredit, acChild );

  MyMoneyReport filter( MyMoneyReport::eExpenseIncome );
  filter.setDateFilter(QDate(2004,9,1),QDate(2005,1,1).addDays(-1));
  filter.setShowSubAccounts(true);
  XMLandback(filter);
  PivotTable spending_f( filter );
  writeTabletoCSV(spending_f);

  CPPUNIT_ASSERT(spending_f.m_grid["Expense"]["Parent"][acParent][3]==(-moParent));
  CPPUNIT_ASSERT(spending_f.m_grid["Expense"]["Parent"][acChild][3]==(-moChild));
  CPPUNIT_ASSERT(spending_f.m_grid["Expense"]["Parent"].m_total[3]==(-moParent-moChild));
  CPPUNIT_ASSERT(spending_f.m_grid["Expense"]["Parent"].m_total[2]==moZero);
  CPPUNIT_ASSERT(spending_f.m_grid["Expense"]["Parent"].m_total.m_total==(-moParent-moChild));
  CPPUNIT_ASSERT(spending_f.m_grid.m_total[3]==(-moParent-moChild));
  CPPUNIT_ASSERT(spending_f.m_grid.m_total.m_total==(-moParent-moChild));

  filter.clear();
  filter.setRowType(MyMoneyReport::eAssetLiability);
  filter.setDateFilter(QDate(2004,9,1),QDate(2005,1,1).addDays(-1));
  XMLandback(filter);
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

  MyMoneyReport filter( MyMoneyReport::eExpenseIncome );
  filter.setDateFilter(QDate(2004,9,1),QDate(2005,1,1).addDays(-1));
  filter.addCategory(acChild);
  filter.addCategory(acSolo);
  XMLandback(filter);
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

  MyMoneyReport filter( MyMoneyReport::eAssetLiability );
  filter.setDateFilter(QDate(2004,9,1),QDate(2005,1,1).addDays(-1));
  filter.addAccount(acChecking);
  filter.addCategory(acChild);
  filter.addCategory(acSolo);
  XMLandback(filter);
  PivotTable networth_f( filter );
  CPPUNIT_ASSERT(networth_f.m_grid.m_total[3] == -moSolo+moCheckingOpen );
}

void KReportsViewTest::testFilterALvsIE()
{
  // Test that removing an asset/liability account will remove the entry from an income/spending report
  TransactionHelper t1( QDate(2004,10,31), MyMoneySplit::ActionWithdrawal, moSolo, acChecking, acSolo );
  TransactionHelper t2( QDate(2004,11,7), MyMoneySplit::ActionWithdrawal, moParent1, acCredit, acParent );
  TransactionHelper t3( QDate(2004,11,7), MyMoneySplit::ActionWithdrawal, moChild, acCredit, acChild );

  MyMoneyReport filter(MyMoneyReport::eExpenseIncome);
  filter.setDateFilter(QDate(2004,9,1),QDate(2005,1,1).addDays(-1));
  filter.addAccount(acChecking);
  CPPUNIT_ASSERT(file->transactionList(filter).count() == 1);

  XMLandback(filter);
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

  MyMoneyReport filter( MyMoneyReport::eExpenseIncome );
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

  MyMoneyReport filter( MyMoneyReport::eExpenseIncome );
  filter.setDateFilter(QDate(2004,1,1),QDate(2005,1,1).addDays(-1));
  filter.setShowSubAccounts(true);
  XMLandback(filter);
  PivotTable spending_f( filter );
  writeTabletoCSV(spending_f);

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
  filter.setName("Spending WITHOUT currency conversion");
  XMLandback(filter);
  PivotTable spending_fnc( filter );
  writeTabletoCSV(spending_fnc);

  CPPUNIT_ASSERT(spending_fnc.m_grid["Expense"]["Foreign"][acCanCash][2]==(-moCanTransaction));
  CPPUNIT_ASSERT(spending_fnc.m_grid["Expense"]["Foreign"][acCanCash][3]==(-moCanTransaction));
  CPPUNIT_ASSERT(spending_fnc.m_grid["Expense"]["Foreign"][acCanCash][4]==(-moCanTransaction));
  CPPUNIT_ASSERT(spending_fnc.m_grid["Expense"]["Foreign"][acJpyCash][2]==(-moJpyTransaction));
  CPPUNIT_ASSERT(spending_fnc.m_grid["Expense"]["Foreign"][acJpyCash][3]==(-moJpyTransaction));
  CPPUNIT_ASSERT(spending_fnc.m_grid["Expense"]["Foreign"][acJpyCash][4]==(-moJpyTransaction));

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

void KReportsViewTest::testAdvancedFilter()
{
  // test more advanced filtering capabilities

  // amount
  {
    TransactionHelper t1( QDate(2004,11,7), MyMoneySplit::ActionWithdrawal, moParent1, acCredit, acParent );
    TransactionHelper t2( QDate(2004,11,7), MyMoneySplit::ActionWithdrawal, moChild, acCredit, acChild );
    MyMoneyReport filter( MyMoneyReport::eExpenseIncome );
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
    MyMoneyReport filter( MyMoneyReport::eExpenseIncome );
    filter.setDateFilter(QDate(2004,1,1),QDate(2005,1,1).addDays(-1));
    filter.addPayee(MyMoneyFile::instance()->payeeByName("Thomas Baumgart").id());
    XMLandback(filter);
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

    MyMoneyReport filter( MyMoneyReport::eExpenseIncome );
    filter.setDateFilter(QDate(2004,1,1),QDate(2005,1,1).addDays(-1));
    filter.addPayee(QCString());
    XMLandback(filter);
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

    MyMoneyReport filter( MyMoneyReport::eExpenseIncome );
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

    MyMoneyReport filter( MyMoneyReport::eExpenseIncome );
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

    MyMoneyReport filter( MyMoneyReport::eExpenseIncome );
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

    MyMoneyReport filter( MyMoneyReport::eExpenseIncome );
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

    MyMoneyReport filter( MyMoneyReport::eExpenseIncome );
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

  MyMoneyReport filter( MyMoneyReport::eExpenseIncome );
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
  XMLandback(filter);
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

}

inline int RANDOM( int lo, int hi )
{
  return ( lo + static_cast<int>( static_cast<double>(hi-lo)*rand()/RAND_MAX ) );
}

inline double RANDOM( double lo, double hi )
{
  return ( lo + (hi-lo)*rand()/RAND_MAX );
}

void KReportsViewTest::testXMLWrite()
{
  // test writing a report configuration object to XML.
  // This shouldn't be needed any longer, with the additional of
  // XMLandback() calls everywhere else, but we'll leave it here
  // just in case we want to special-case test a few things.

  MyMoneyReport megafilter( MyMoneyReport::eExpenseIncome );
  megafilter.setDateFilter(QDate(2004,1,1),QDate(2005,1,1).addDays(-1));
  megafilter.setAmountFilter(moZero,moChild);
  megafilter.setTextFilter(QRegExp("Thomas"));
  megafilter.addType(MyMoneyTransactionFilter::payments);
  megafilter.addType(MyMoneyTransactionFilter::deposits);
  megafilter.setNumberFilter("1","3");
  megafilter.addState(MyMoneyTransactionFilter::cleared);
  megafilter.addPayee(MyMoneyFile::instance()->payeeByName("Thomas Baumgart").id());
  megafilter.addAccount(acChecking);
  megafilter.addAccount(acCredit);
  megafilter.addCategory(acSolo);
  megafilter.addCategory(acForeign);
  megafilter.setColumnType(MyMoneyReport::eBiMonths);
  megafilter.setName("Report Saved to XML");
  megafilter.setComment("Good morning, America!");
  megafilter.setShowSubAccounts(false);
  megafilter.setConvertCurrency(false);

  writeRCFtoXML(megafilter,"test2xml.xml");

  QValueList<MyMoneyReport> filters;
  readRCFfromXML( filters, "test2xml.xml" );

  CPPUNIT_ASSERT(filters.count() == 1);
  CPPUNIT_ASSERT(filters[0].name() == "Report Saved to XML");
  CPPUNIT_ASSERT(filters[0].comment() == "Good morning, America!");
  CPPUNIT_ASSERT(filters[0].isShowingSubAccounts() == megafilter.isShowingSubAccounts());
  CPPUNIT_ASSERT(filters[0].isConvertCurrency() == megafilter.isConvertCurrency());
  CPPUNIT_ASSERT(filters[0].rowType() == megafilter.rowType());
  CPPUNIT_ASSERT(filters[0].columnType() == megafilter.columnType());
  // TODO: Add more checks here

  // FIXME: remove this, and make a mymoneystatementtest class.  This is just a
  // temporary place to stash this.

  // Test data generator for investment data
  int transactions = 25;
  QStringList equities = QStringList::split(",","TEST1 Test 1,TEST2 Test 2,TEST3 Test 3");
  QDate date = QDate::currentDate();
  double price = 100.0;
  QValueList<MyMoneyStatement::Transaction::EAction> actions;
  actions.push_back( MyMoneyStatement::Transaction::eaBuy );
  actions.push_back( MyMoneyStatement::Transaction::eaSell );
  actions.push_back( MyMoneyStatement::Transaction::eaReinvestDividend );
  QStringList wordlist = QStringList::split(" ","during the last weeks I was thinking about the requirements for the budget support. I like to wrap them up here and throw them out for further discussion. I am by far not a financial expert and look at the issues as a personal user. If I am missing something, you have more requirements or have other ideas, please feel free to send them to the mailing list. Now's the time to discuss all this. Requirements: What is a budget? In my eyes, a budget is the plan for a certain amount of time (usually one year or one quarter) about your earnings and spendings. So we deal with expense and income accounts exclusively.");

  srand(time(0));
  MyMoneyStatement s;
  s.m_eType = MyMoneyStatement::etInvestment;

  while ( transactions-- )
  {
    static int nextnumber = 1;
    MyMoneyStatement::Transaction t;

    t.m_strSecurity = equities[RANDOM(0,equities.count())];
    t.m_eAction = actions[RANDOM(0,actions.count())];
    t.m_dShares = RANDOM(1000.0,1000000.0)/1000.0;
    if ( t.m_eAction == MyMoneyStatement::Transaction::eaSell )
      t.m_dShares = -t.m_dShares;
    t.m_moneyAmount = price * t.m_dShares;
    price = price * RANDOM(80.0,120.0)/100.0;
    t.m_strBankID = QString::number(RANDOM(0,INT_MAX));
    t.m_datePosted = date;
    date = date.addDays( - RANDOM(0,7) );
    t.m_strNumber = QString::number(nextnumber++);

    int words = RANDOM(1,50);
    while ( words-- )
      t.m_strMemo += wordlist[RANDOM(0,wordlist.count())] + " ";

    s.m_listTransactions += t;
  }

  MyMoneyStatement::writeXMLFile( s, "investments.xml" );
}

inline MyMoneyMoney searchHTML(const QString& _html, const QString& _search)
{
  QRegExp re(QString("%1[<>/td]*([\\-.0-9,]*)").arg(_search));
  re.search(_html);
  QString found = re.cap(1);
  found.remove(',');

  return MyMoneyMoney(found.toDouble());
}

void KReportsViewTest::testQueryBasics()
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
    CPPUNIT_ASSERT( searchHTML(html,i18n("Grand Total")) == -(moParent1 + moParent2 + moSolo + moChild) * 3 );

    filter.setRowType( MyMoneyReport::eTopCategory );
    cols = MyMoneyReport::eQCnumber | MyMoneyReport::eQCpayee | MyMoneyReport::eQCaccount;
    filter.setQueryColumns( static_cast<MyMoneyReport::EQueryColumns>(cols) ); // 
    XMLandback(filter);
    QueryTable qtbl_2(filter);

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
    CPPUNIT_ASSERT( searchHTML(html,i18n("Grand Total")) == -(moParent1 + moParent2 + moSolo + moChild) * 3 );

    filter.setRowType( MyMoneyReport::eAccount );
    filter.setName("Transactions by Account");
    cols = MyMoneyReport::eQCnumber | MyMoneyReport::eQCpayee | MyMoneyReport::eQCcategory;
    filter.setQueryColumns( static_cast<MyMoneyReport::EQueryColumns>(cols) ); // 
    XMLandback(filter);
    QueryTable qtbl_3(filter);

    CPPUNIT_ASSERT(qtbl_3.m_transactions.count() == 12);
    CPPUNIT_ASSERT(qtbl_3.m_transactions[0]["account"]=="Checking Account");
    CPPUNIT_ASSERT(qtbl_3.m_transactions[0]["category"]=="Solo");
    CPPUNIT_ASSERT(qtbl_3.m_transactions[0]["postdate"]=="2004-01-01");
    CPPUNIT_ASSERT(qtbl_3.m_transactions[11]["account"]=="Credit Card");
    CPPUNIT_ASSERT(qtbl_3.m_transactions[11]["category"]=="Parent");
    CPPUNIT_ASSERT(qtbl_3.m_transactions[11]["postdate"]=="2005-09-01");

    html = qtbl_3.renderHTML();
    CPPUNIT_ASSERT( searchHTML(html,i18n("Total")+" Checking Account") == -(moSolo) * 3 );
    CPPUNIT_ASSERT( searchHTML(html,i18n("Total")+" Credit Card") == -(moParent1 + moParent2 + moChild) * 3 );
    CPPUNIT_ASSERT( searchHTML(html,i18n("Grand Total")) == -(moParent1 + moParent2 + moSolo + moChild) * 3 );

    filter.setRowType( MyMoneyReport::ePayee );
    filter.setName("Transactions by Payee");
    cols = MyMoneyReport::eQCnumber | MyMoneyReport::eQCmemo | MyMoneyReport::eQCcategory;
    filter.setQueryColumns( static_cast<MyMoneyReport::EQueryColumns>(cols) ); // 
    XMLandback(filter);
    QueryTable qtbl_4(filter);

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
    CPPUNIT_ASSERT( searchHTML(html,i18n("Grand Total")) == -(moParent1 + moParent2 + moSolo + moChild) * 3 );

    filter.setRowType( MyMoneyReport::eMonth );
    filter.setName("Transactions by Month");
    cols = MyMoneyReport::eQCnumber | MyMoneyReport::eQCpayee | MyMoneyReport::eQCcategory;
    filter.setQueryColumns( static_cast<MyMoneyReport::EQueryColumns>(cols) ); // 
    XMLandback(filter);
    QueryTable qtbl_5(filter);

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
    CPPUNIT_ASSERT( searchHTML(html,i18n("Total")+" Month of 2005-05-01") == -moParent1 );
    CPPUNIT_ASSERT( searchHTML(html,i18n("Grand Total")) == -(moParent1 + moParent2 + moSolo + moChild) * 3 );

    filter.setRowType( MyMoneyReport::eWeek );
    filter.setName("Transactions by Week");
    cols = MyMoneyReport::eQCnumber | MyMoneyReport::eQCpayee | MyMoneyReport::eQCcategory;
    filter.setQueryColumns( static_cast<MyMoneyReport::EQueryColumns>(cols) ); // 
    XMLandback(filter);
    QueryTable qtbl_6(filter);

    CPPUNIT_ASSERT(qtbl_6.m_transactions.count() == 12);
    CPPUNIT_ASSERT(qtbl_6.m_transactions[0]["payee"]=="Test Payee");
    CPPUNIT_ASSERT(qtbl_6.m_transactions[0]["category"]=="Solo");
    CPPUNIT_ASSERT(qtbl_6.m_transactions[0]["postdate"]=="2004-01-01");
    CPPUNIT_ASSERT(qtbl_6.m_transactions[8]["payee"]=="Test Payee");
    CPPUNIT_ASSERT(qtbl_6.m_transactions[8]["category"]=="Parent: Child");
    CPPUNIT_ASSERT(qtbl_6.m_transactions[8]["postdate"]=="2004-11-07");
    CPPUNIT_ASSERT(qtbl_6.m_transactions[11]["payee"]=="Test Payee");
    CPPUNIT_ASSERT(qtbl_6.m_transactions[11]["category"]=="Parent");
    CPPUNIT_ASSERT(qtbl_6.m_transactions[11]["postdate"]=="2005-09-01");

    html = qtbl_6.renderHTML();
    CPPUNIT_ASSERT( searchHTML(html,i18n("Total")+" Week of 2003-12-29") == -moSolo );
    CPPUNIT_ASSERT( searchHTML(html,i18n("Total")+" Week of 2004-11-01") == -(moChild) * 3 );
    CPPUNIT_ASSERT( searchHTML(html,i18n("Total")+" Week of 2005-08-29") == -moParent2 );
    CPPUNIT_ASSERT( searchHTML(html,i18n("Grand Total")) == -(moParent1 + moParent2 + moSolo + moChild) * 3 );
  }
  catch(MyMoneyException *e)
  {
    CPPUNIT_FAIL(e->what());
    delete e;
  }

  // Test querytable::TableRow::operator> and operator==

  reports::QueryTable::TableRow low;
  low["first"] = "A";
  low["second"] = "B";
  low["third"] = "C";

  reports::QueryTable::TableRow high;
  high["first"] = "A";
  high["second"] = "C";
  high["third"] = "B";

  reports::QueryTable::TableRow::setSortCriteria("first,second,third");
  CPPUNIT_ASSERT( low < high );
  CPPUNIT_ASSERT( low <= high );
  CPPUNIT_ASSERT( high > low );
  CPPUNIT_ASSERT( high <= high );
  CPPUNIT_ASSERT( high == high );
}

void KReportsViewTest::testCashFlowAnalysis()
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

void KReportsViewTest::testAccountQuery()
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

    CPPUNIT_ASSERT(qtbl_1.m_transactions.count() == 2);
    CPPUNIT_ASSERT(qtbl_1.m_transactions[0]["account"]=="Checking Account");
    CPPUNIT_ASSERT(qtbl_1.m_transactions[0]["value"]==moCheckingOpen.toString());
    CPPUNIT_ASSERT(qtbl_1.m_transactions[0]["equitytype"].isEmpty());
    CPPUNIT_ASSERT(qtbl_1.m_transactions[1]["account"]=="Credit Card");
    CPPUNIT_ASSERT(qtbl_1.m_transactions[1]["value"]==moCreditOpen.toString());
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
    CPPUNIT_ASSERT(qtbl_2.m_transactions[0]["value"]==(moCheckingOpen-moSolo*3).toString());
    CPPUNIT_ASSERT(qtbl_2.m_transactions[1]["account"]=="Credit Card");
    CPPUNIT_ASSERT(qtbl_2.m_transactions[1]["value"]==(moCreditOpen-(moParent1 + moParent2 + moChild) * 3).toString());

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
    CPPUNIT_ASSERT(qtbl_3.m_transactions[0]["value"]==(moCheckingOpen-moSolo*3).toString());
    CPPUNIT_ASSERT(qtbl_3.m_transactions[1]["account"]=="Credit Card");
    CPPUNIT_ASSERT(qtbl_3.m_transactions[1]["value"]==(moCreditOpen-(moParent1 + moParent2 + moChild) * 3).toString());

    html = qtbl_3.renderHTML();
    CPPUNIT_ASSERT( searchHTML(html,i18n("Total")+" "+i18n("Checkings")) == moCheckingOpen-moSolo*3 );
    CPPUNIT_ASSERT( searchHTML(html,i18n("Total")+" "+i18n("Credit Card")) == moCreditOpen-(moParent1 + moParent2 + moChild) * 3 );
    CPPUNIT_ASSERT( searchHTML(html,i18n("Grand Total")) == moCheckingOpen+moCreditOpen-(moParent1 + moParent2 + moSolo + moChild) * 3 );
  }
  catch(MyMoneyException *e)
  {
    CPPUNIT_FAIL(e->what());
    delete e;
  }
}

void KReportsViewTest::testInvestment(void)
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
  // Investment Transactions Report
  //
  
  MyMoneyReport invtran_r(
      MyMoneyReport::eTopAccount,
      MyMoneyReport::eQCaction|MyMoneyReport::eQCshares|MyMoneyReport::eQCprice,
      MyMoneyTransactionFilter::yearToDate,
      false,
      i18n("Investment Transactions"),
      i18n("Test Report")
    );
  invtran_r.setInvestmentsOnly(true);
  XMLandback(invtran_r);
  QueryTable invtran(invtran_r);

  CPPUNIT_ASSERT(invtran.m_transactions.count()==8);
  CPPUNIT_ASSERT(MyMoneyMoney(invtran.m_transactions[0]["value"])==MyMoneyMoney(100000.00));
  CPPUNIT_ASSERT(MyMoneyMoney(invtran.m_transactions[1]["value"])==MyMoneyMoney(110000.00));
  CPPUNIT_ASSERT(MyMoneyMoney(invtran.m_transactions[2]["value"])==MyMoneyMoney(-24000.00));
  CPPUNIT_ASSERT(MyMoneyMoney(invtran.m_transactions[3]["value"])==MyMoneyMoney(-20000.00));
  CPPUNIT_ASSERT(MyMoneyMoney(invtran.m_transactions[4]["value"])==MyMoneyMoney(  5000.00));
  CPPUNIT_ASSERT(MyMoneyMoney(invtran.m_transactions[5]["value"])==MyMoneyMoney(  4000.00));
  CPPUNIT_ASSERT(MyMoneyMoney(invtran.m_transactions[6]["value"])==MyMoneyMoney( -1000.00));
  CPPUNIT_ASSERT(MyMoneyMoney(invtran.m_transactions[7]["value"])==MyMoneyMoney( -1200.00));

  CPPUNIT_ASSERT(MyMoneyMoney(invtran.m_transactions[0]["price"])==MyMoneyMoney(100.00));
  CPPUNIT_ASSERT(MyMoneyMoney(invtran.m_transactions[2]["price"])==MyMoneyMoney(120.00));
  CPPUNIT_ASSERT(MyMoneyMoney(invtran.m_transactions[4]["price"])==MyMoneyMoney(100.00));
  CPPUNIT_ASSERT(MyMoneyMoney(invtran.m_transactions[6]["price"])==MyMoneyMoney(  0.00));

  CPPUNIT_ASSERT(MyMoneyMoney(invtran.m_transactions[1]["shares"])==MyMoneyMoney(1000.00));
  CPPUNIT_ASSERT(MyMoneyMoney(invtran.m_transactions[3]["shares"])==MyMoneyMoney(-200.00));
  CPPUNIT_ASSERT(MyMoneyMoney(invtran.m_transactions[5]["shares"])==MyMoneyMoney(  50.00));
  CPPUNIT_ASSERT(MyMoneyMoney(invtran.m_transactions[7]["shares"])==MyMoneyMoney(   0.00));

  CPPUNIT_ASSERT(invtran.m_transactions[0]["action"]=="Buy");
  CPPUNIT_ASSERT(invtran.m_transactions[2]["action"]=="Sell");
  CPPUNIT_ASSERT(invtran.m_transactions[4]["action"]=="Reinvest");
  CPPUNIT_ASSERT(invtran.m_transactions[6]["action"]=="Dividend");
        
  QString html = invtran.renderHTML();
  CPPUNIT_ASSERT( searchHTML(html,i18n("Total Stock 1")) == MyMoneyMoney(172800.00) );
  CPPUNIT_ASSERT( searchHTML(html,i18n("Grand Total")) == MyMoneyMoney(172800.00) );
    
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
  CPPUNIT_ASSERT(MyMoneyMoney(invhold.m_transactions[0]["return"])==MyMoneyMoney("567/10000"));
  CPPUNIT_ASSERT(MyMoneyMoney(invhold.m_transactions[0]["buys"])==MyMoneyMoney(210000.00));
  CPPUNIT_ASSERT(MyMoneyMoney(invhold.m_transactions[0]["sells"])==MyMoneyMoney(-44000.00));
  CPPUNIT_ASSERT(MyMoneyMoney(invhold.m_transactions[0]["reinvestincome"])==MyMoneyMoney(9000.00));
  CPPUNIT_ASSERT(MyMoneyMoney(invhold.m_transactions[0]["cashincome"])==MyMoneyMoney(2200.00));
  CPPUNIT_ASSERT(MyMoneyMoney(invhold.m_transactions[0]["shares"])==MyMoneyMoney(1700.00));
  CPPUNIT_ASSERT(MyMoneyMoney(invhold.m_transactions[0]["price"])==MyMoneyMoney(100.00));
  CPPUNIT_ASSERT(MyMoneyMoney(invhold.m_transactions[1]["return"]).isZero());

  html = invhold.renderHTML();
  CPPUNIT_ASSERT( searchHTML(html,i18n("Grand Total")) == MyMoneyMoney(170000.00) );

  //
  // Net Worth Report (with investments)
  //

  MyMoneyReport networth_r( MyMoneyReport::eAssetLiability );
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

//
// testOfxImport: Needs to be moved to converter/something.cpp.  But this
// requires a bunch of AM changes I don't want to make right now.  Will
// move it when done.
//

#include "../dialogs/mymoneyofxconnector.h"
#include "../converter/mymoneyofxstatement.h"

void KReportsViewTest::testOfxImport(void)
{
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
    CPPUNIT_ASSERT(s.m_listTransactions.count() == 4);
    
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
    CPPUNIT_ASSERT(s.m_listTransactions.count() == 4);
    
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
    
}

// vim:cin:si:ai:et:ts=2:sw=2:
