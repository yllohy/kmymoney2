/***************************************************************************
                          reportstestcommon.cpp
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

namespace test {

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
  // _currencyid is the currency of the transaction, and of the _value
  // both the account and category can have their own currency (athough the category having
  // a foreign currency is not yet supported by the program, the reports will still allow it,
  // so it must be tested.)

    MyMoneyFile* file = MyMoneyFile::instance();
    bool haspayee = ! _payee.isEmpty();
    MyMoneyPayee payeeTest = file->payeeByName(_payee);

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
    price = MyMoneyFile::instance()->price(currencyid, file->account(_accountid).currencyId(),_date).rate();
    splitLeft.setShares(-_value * price);
    splitLeft.setAccountId(_accountid);
    addSplit(splitLeft);

    MyMoneySplit splitRight;
    if ( haspayee )
      splitRight.setPayeeId(payeeTest.id());
    splitRight.setAction(_action);
    splitRight.setValue(_value);
    price = MyMoneyFile::instance()->price(currencyid, file->account(_categoryid).currencyId(),_date).rate();
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
  else if ( _action == MyMoneySplit::ActionDividend || _action == MyMoneySplit::ActionYield )
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
    s2.setAction(_action);
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

void writeTabletoHTML( const QueryTable& table, const QString& _filename = QString() )
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
        if("REPORTS" == childElement.tagName())
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

MyMoneyMoney searchHTML(const QString& _html, const QString& _search)
{
  QRegExp re(QString("%1[<>/td]*([\\-.0-9,]*)").arg(_search));
  re.search(_html);
  QString found = re.cap(1);
  found.remove(',');

  return MyMoneyMoney(found.toDouble());
}

} // end namespace test

// vim:cin:si:ai:et:ts=2:sw=2:
