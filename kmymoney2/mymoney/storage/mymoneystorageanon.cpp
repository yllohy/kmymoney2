/***************************************************************************
                          mymoneystorageanon.cpp
                             -------------------
    begin                : Thu Oct 24 2002
    copyright            : (C) 2000-2002 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
                           Ace Jones <acejones@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "config.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <qfile.h>
#include <qdom.h>
#include <qmap.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include "kdecompat.h"

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneystorageanon.h"
#include "../../kmymoneyutils.h"
#include "../mymoneyreport.h"
#include "../mymoneyinstitution.h"

unsigned int MyMoneyStorageANON::fileVersionRead = 0;
unsigned int MyMoneyStorageANON::fileVersionWrite = 0;

QStringList MyMoneyStorageANON::zKvpNoModify = QStringList::split(",","kmm-baseCurrency,PreferredAccount,Tax,fixed-interest,interest-calculation,payee,schedule,term");
QStringList MyMoneyStorageANON::zKvpXNumber = QStringList::split(",","final-payment,loan-amount,periodic-payment");


MyMoneyStorageANON::MyMoneyStorageANON()
{
  m_storage = NULL;
  m_doc     = NULL;
}

MyMoneyStorageANON::~MyMoneyStorageANON()
{
}

void MyMoneyStorageANON::readFile(QIODevice* , IMyMoneySerialize* )
{
  throw new MYMONEYEXCEPTION("Cannot read a file through MyMoneyStorageANON!!");
}

void MyMoneyStorageANON::writeFile(QIODevice* qf, IMyMoneySerialize* storage)
{
  Q_CHECK_PTR(qf);
  Q_CHECK_PTR(storage);
  if(!storage)
  {
    return;
  }
  m_storage = storage;

  m_doc = new QDomDocument("KMYMONEY-FILE");
  Q_CHECK_PTR(m_doc);
  QDomProcessingInstruction instruct = m_doc->createProcessingInstruction(QString("xml"), QString("version=\"1.0\" encoding=\"utf-8\""));
  m_doc->appendChild(instruct);

  QDomElement mainElement = m_doc->createElement("KMYMONEY-FILE");
  m_doc->appendChild(mainElement);

  QDomElement fileInfo = m_doc->createElement("FILEINFO");
  writeFileInformation(fileInfo);
  mainElement.appendChild(fileInfo);

  QDomElement userInfo = m_doc->createElement("USER");
  writeUserInformation(userInfo);
  mainElement.appendChild(userInfo);

  QDomElement institutions = m_doc->createElement("INSTITUTIONS");
  writeInstitutions(institutions);
  mainElement.appendChild(institutions);

  QDomElement payees = m_doc->createElement("PAYEES");
  writePayees(payees);
  mainElement.appendChild(payees);

  QDomElement accounts = m_doc->createElement("ACCOUNTS");
  writeAccounts(accounts);
  mainElement.appendChild(accounts);

  QDomElement transactions = m_doc->createElement("TRANSACTIONS");
  writeTransactions(transactions);
  mainElement.appendChild(transactions);

  QDomElement keyvalpairs = writeKeyValuePairs(m_storage->pairs());
  mainElement.appendChild(keyvalpairs);

  QDomElement schedules = m_doc->createElement("SCHEDULES");
  writeSchedules(schedules);
  mainElement.appendChild(schedules);

  QDomElement equities = m_doc->createElement("EQUITIES");
  writeEquities(equities);
  mainElement.appendChild(equities);

  QDomElement currencies = m_doc->createElement("CURRENCIES");
  writeCurrencies(currencies);
  mainElement.appendChild(currencies);

  QDomElement reports = m_doc->createElement("REPORTS");
  writeReports(reports);
  mainElement.appendChild(reports);

  QTextStream stream(qf);
  stream.setEncoding(QTextStream::UnicodeUTF8);
  stream << m_doc->toString();

  delete m_doc;
  m_doc = NULL;
  m_storage = NULL;
}

void MyMoneyStorageANON::writeFileInformation(QDomElement& fileInfo)
{
  QDomElement creationDate = m_doc->createElement("CREATION_DATE");
  creationDate.setAttribute(QString("date"), getString(m_storage->creationDate()));
  fileInfo.appendChild(creationDate);

  QDomElement lastModifiedDate = m_doc->createElement("LAST_MODIFIED_DATE");
  lastModifiedDate.setAttribute(QString("date"), getString(m_storage->lastModificationDate()));
  fileInfo.appendChild(lastModifiedDate);

  QDomElement version = m_doc->createElement("VERSION");

  //if we haven't written a file yet, write using the default version.
  if(!fileVersionWrite)
  {
    fileVersionWrite = VERSION_0_60_XML;
  }
  QString strVersion;
  strVersion.setNum(fileVersionWrite, 16);
  version.setAttribute(QString("id"), strVersion);
  fileInfo.appendChild(version);
}

void MyMoneyStorageANON::writeUserInformation(QDomElement& userInfo)
{
  userInfo.setAttribute(QString("name"), hideString(m_storage->userName()));
  userInfo.setAttribute(QString("email"), hideString(m_storage->userEmail()));

  QDomElement address = m_doc->createElement("ADDRESS");
  address.setAttribute(QString("street"), hideString(m_storage->userStreet()));
  address.setAttribute(QString("city"), hideString(m_storage->userTown()));
  address.setAttribute(QString("county"), hideString(m_storage->userCounty()));
  address.setAttribute(QString("zipcode"), hideString(m_storage->userPostcode()));
  address.setAttribute(QString("telephone"), hideString(m_storage->userTelephone()));

  userInfo.appendChild(address);
}

void MyMoneyStorageANON::writeInstitutions(QDomElement& institutions)
{
  QValueList<MyMoneyInstitution> list;
  QValueList<MyMoneyInstitution>::ConstIterator it;

  list = m_storage->institutionList();
  for(it = list.begin(); it != list.end(); ++it)
  {
    QDomElement institution = m_doc->createElement("INSTITUTION");
    writeInstitution(institution, *it);
    institutions.appendChild(institution);
  }
}

void MyMoneyStorageANON::writeInstitution(QDomElement& institution, const MyMoneyInstitution& i)
{
  institution.setAttribute(QString("id"), i.id());
  institution.setAttribute(QString("name"), i.id());
  institution.setAttribute(QString("manager"), hideString(i.manager()));
  institution.setAttribute(QString("sortcode"), hideString(i.sortcode()));

  QDomElement address = m_doc->createElement("ADDRESS");
  address.setAttribute(QString("street"), hideString(i.street()));
  address.setAttribute(QString("city"), hideString(i.city()));
  address.setAttribute(QString("zip"), hideString(i.postcode()));
  address.setAttribute(QString("state"), hideString(i.town()));
  address.setAttribute(QString("telephone"), hideString(i.telephone()));

  institution.appendChild(address);

  QDomElement accounts = m_doc->createElement("ACCOUNTIDS");
  QCStringList tempAccountList = i.accountList();
  for(QCStringList::const_iterator it = tempAccountList.begin(); it != tempAccountList.end(); ++it)
  {
    QDomElement temp = m_doc->createElement("ACCOUNTID");
    temp.setAttribute(QString("id"), (*it));
    accounts.appendChild(temp);
  }

  institution.appendChild(accounts);
  
  QMap<QCString,QString> s = i.ofxConnectionSettings().pairs();
  if ( s.count() )
  {
    QDomElement ofxsettings = m_doc->createElement("OFXSETTINGS");
    QMap<QCString,QString>::const_iterator it_key = s.begin();
    while ( it_key != s.end() )
    {
      ofxsettings.setAttribute(it_key.key(), hideString(it_key.data()));
      ++it_key;    
    }
  
    institution.appendChild(ofxsettings);
  }    
}


void MyMoneyStorageANON::writePayees(QDomElement& payees)
{
  QValueList<MyMoneyPayee> list;
  QValueList<MyMoneyPayee>::ConstIterator it;

  list = m_storage->payeeList();

  for(it = list.begin(); it != list.end(); ++it)
  {
    QDomElement payee = m_doc->createElement("PAYEE");
    writePayee(payee, *it);
    payees.appendChild(payee);
  }
}

void MyMoneyStorageANON::writePayee(QDomElement& payee, const MyMoneyPayee& p)
{
  payee.setAttribute(QString("name"), p.id());
  payee.setAttribute(QString("id"), p.id());
  payee.setAttribute(QString("reference"), hideString(p.reference()));

  QDomElement address = m_doc->createElement("ADDRESS");
  address.setAttribute(QString("street"), hideString(p.address()));
  address.setAttribute(QString("city"), hideString(p.city()));
  address.setAttribute(QString("postcode"), hideString(p.postcode()));
  address.setAttribute(QString("state"), hideString(p.state()));
  address.setAttribute(QString("telephone"), hideString(p.telephone()));

  payee.appendChild(address);
}

void MyMoneyStorageANON::writeAccounts(QDomElement& accounts)
{
  QValueList<MyMoneyAccount> list;
  QValueList<MyMoneyAccount>::ConstIterator it;

  list = m_storage->accountList();

  QDomElement asset, liability, expense, income;

  asset = m_doc->createElement("ACCOUNT");
  writeAccount(asset, m_storage->asset());
  accounts.appendChild(asset);

  liability = m_doc->createElement("ACCOUNT");
  writeAccount(liability, m_storage->liability());
  accounts.appendChild(liability);

  expense = m_doc->createElement("ACCOUNT");
  writeAccount(expense, m_storage->expense());
  accounts.appendChild(expense);

  income = m_doc->createElement("ACCOUNT");
  writeAccount(income, m_storage->income());
  accounts.appendChild(income);

  //signalProgress(0, list.count(), QObject::tr("Saving accounts..."));
  int i = 0;
  for(it = list.begin(); it != list.end(); ++it, ++i)
  {
    QDomElement account = m_doc->createElement("ACCOUNT");
    writeAccount(account, *it);
    accounts.appendChild(account);

    //signalProgress(i, 0);
  }
}

void MyMoneyStorageANON::writeAccount(QDomElement& account, const MyMoneyAccount& p)
{
  account.setAttribute(QString("parentaccount"), p.parentAccountId());
  account.setAttribute(QString("lastreconciled"), getString(p.lastReconciliationDate()));
  account.setAttribute(QString("lastmodified"), getString(p.lastModified()));
  account.setAttribute(QString("institution"), p.institutionId());
  account.setAttribute(QString("opened"), getString(p.openingDate()));
  account.setAttribute(QString("number"), hideString(p.number()));
  account.setAttribute(QString("openingbalance"), hideNumber(p.openingBalance()).toString());
  account.setAttribute(QString("type"), p.accountType());
  account.setAttribute(QString("id"), p.id());
  account.setAttribute(QString("name"), p.id());
  account.setAttribute(QString("description"), hideString(p.description()));
  if(!p.currencyId().isEmpty())
    account.setAttribute(QString("currency"), p.currencyId());

  //Add in subaccount information, if this account has subaccounts.
  if(p.accountCount())
  {
    QDomElement subAccounts = m_doc->createElement("SUBACCOUNTS");
    QCStringList accountList = p.accountList();
    QCStringList::Iterator it;
    for(it = accountList.begin(); it != accountList.end(); ++it)
    {
      QDomElement temp = m_doc->createElement("SUBACCOUNT");
      temp.setAttribute(QString("id"), (*it));
      subAccounts.appendChild(temp);
    }

    account.appendChild(subAccounts);
  }

  //Add in Key-Value Pairs for accounts.
  QDomElement keyValPairs = writeKeyValuePairs(p.pairs());
  account.appendChild(keyValPairs);
}

void MyMoneyStorageANON::writeTransactions(QDomElement& transactions)
{
  QValueList<MyMoneyTransaction> list;
  QValueList<MyMoneyTransaction>::ConstIterator it;
  MyMoneyTransactionFilter filter;

  list = m_storage->transactionList(filter);

  //signalProgress(0, list.count(), QObject::tr("Saving transactions..."));

  int i = 0;
  for(it = list.begin(); it != list.end(); ++it, ++i)
  {
    QDomElement tx = m_doc->createElement("TRANSACTION");
    writeTransaction(tx, *it);
    transactions.appendChild(tx);
    //signalProgress(i, 0);
  }
}

void MyMoneyStorageANON::writeTransaction(QDomElement& transaction, const MyMoneyTransaction& tx)
{
  transaction.setAttribute(QString("id"), tx.id());
  transaction.setAttribute(QString("postdate"), getString(tx.postDate()));
  transaction.setAttribute(QString("memo"), tx.id());
  transaction.setAttribute(QString("entrydate"), getString(tx.entryDate()));
  transaction.setAttribute(QString("commodity"), tx.commodity());
  transaction.setAttribute(QString("bankid"), hideString(tx.bankID()));

  QDomElement splits = m_doc->createElement("SPLITS");
  QValueList<MyMoneySplit> splitList = tx.splits();

  writeSplits(splits, splitList,tx.id());
  transaction.appendChild(splits);

  //Add in Key-Value Pairs for transactions.
  QDomElement keyValPairs = writeKeyValuePairs(tx.pairs());
  transaction.appendChild(keyValPairs);
}


void MyMoneyStorageANON::writeSchedules(QDomElement& scheduled)
{
  QValueList<MyMoneySchedule> list;
  QValueList<MyMoneySchedule>::ConstIterator it;

  list = m_storage->scheduleList();

  for(it = list.begin(); it != list.end(); ++it)
  {
    QDomElement schedTx = m_doc->createElement("SCHEDULED_TX");
    writeSchedule(schedTx, *it);
    scheduled.appendChild(schedTx);
  }
}


void MyMoneyStorageANON::writeSchedule(QDomElement& scheduledTx, const MyMoneySchedule& tx)
{
  scheduledTx.setAttribute(QString("name"), tx.id());
  scheduledTx.setAttribute(QString("type"), tx.type());
  scheduledTx.setAttribute(QString("occurence"), tx.occurence());
  scheduledTx.setAttribute(QString("paymentType"), tx.paymentType());
  scheduledTx.setAttribute(QString("startDate"), getString(tx.startDate()));
  scheduledTx.setAttribute(QString("endDate"), getString(tx.endDate()));
  scheduledTx.setAttribute(QString("fixed"), tx.isFixed());
  scheduledTx.setAttribute(QString("autoEnter"), tx.autoEnter());
  scheduledTx.setAttribute(QString("id"), tx.id());
  scheduledTx.setAttribute(QString("lastPayment"), getString(tx.lastPayment()));
  scheduledTx.setAttribute(QString("weekendOption"), tx.weekendOption());

  //store the payment history for this scheduled task.
  QValueList<QDate> payments = tx.recordedPayments();
  QValueList<QDate>::Iterator it;
  QDomElement paymentsElement = m_doc->createElement("PAYMENTS");
  paymentsElement.setAttribute(QString("count"), payments.count());
  for (it=payments.begin(); it!=payments.end(); ++it)
  {
    QDomElement paymentEntry = m_doc->createElement("PAYMENT");
    paymentEntry.setAttribute(QString("date"), getString(*it));
    paymentsElement.appendChild(paymentEntry);
  }
  scheduledTx.appendChild(paymentsElement);

  //store the transaction data for this task.
  QDomElement transactionElement = m_doc->createElement("TRANSACTION");
  writeTransaction(transactionElement, tx.transaction());
  scheduledTx.appendChild(transactionElement);
}

void MyMoneyStorageANON::writeSplits(QDomElement& splits, const QValueList<MyMoneySplit> splitList, const QCString& transactionId)
{
  //FIXME: Get the splits to balance out

  QValueList<MyMoneySplit>::const_iterator it;
  for(it = splitList.begin(); it != splitList.end(); ++it)
  {
    QDomElement split = m_doc->createElement("SPLIT");
    writeSplit(split, (*it), transactionId);
    splits.appendChild(split);
  }
}

void MyMoneyStorageANON::writeSplit(QDomElement& splitElement, const MyMoneySplit& split,const QCString& transactionId)
{
  splitElement.setAttribute(QString("payee"), split.payeeId());
  splitElement.setAttribute(QString("reconciledate"), getString(split.reconcileDate()));
  splitElement.setAttribute(QString("action"), split.action());
  splitElement.setAttribute(QString("reconcileflag"), split.reconcileFlag());
  
  MyMoneyMoney price = split.shares() / split.value();
  MyMoneyMoney hidevalue = hideNumber(split.value());
  MyMoneyMoney hideshares = (hidevalue * price).convert();
  
  splitElement.setAttribute(QString("value"), hidevalue.toString());
  splitElement.setAttribute(QString("shares"), hideshares.toString());
  splitElement.setAttribute(QString("memo"), QString(transactionId) + "/" + QString(split.id()));
  splitElement.setAttribute(QString("id"), split.id());
  splitElement.setAttribute(QString("account"), split.accountId());
  splitElement.setAttribute(QString("number"), hideString(split.number()));
}

void MyMoneyStorageANON::writeEquities(QDomElement& equities)
{
  QValueList<MyMoneyEquity> equityList = m_storage->equityList();
  if(equityList.size())
  {
    for(QValueList<MyMoneyEquity>::Iterator it = equityList.begin(); it != equityList.end(); ++it)
    {
      QDomElement equity = m_doc->createElement("EQUITY");
      writeEquity(equity, (*it));
      equities.appendChild(equity);
    }
  }
}

void MyMoneyStorageANON::writeEquity(QDomElement& equityElement, const MyMoneyEquity& equity)
{
  equityElement.setAttribute(QString("name"), equity.id());
  equityElement.setAttribute(QString("symbol"), hideString(equity.tradingSymbol()));
  equityElement.setAttribute(QString("type"), static_cast<int>(equity.equityType()));
  equityElement.setAttribute(QString("id"), equity.id());
  equityElement.setAttribute(QString("saf"), equity.smallestAccountFraction());

  QDomElement history = m_doc->createElement("HISTORY");

  equity_price_history priceHistory = equity.priceHistory();
  if(priceHistory.size())
  {
    for(equity_price_history::ConstIterator it = priceHistory.begin(); it != priceHistory.end(); ++it)
    {
      QDomElement entry = m_doc->createElement("ENTRY");
      entry.setAttribute(QString("date"), getString(it.key()));
      entry.setAttribute(QString("price"), hideNumber(it.data()).toString());
      history.appendChild(entry);
    }
  }

  equityElement.appendChild(history);
}

void MyMoneyStorageANON::writeCurrencies(QDomElement& currencies)
{
  QValueList<MyMoneyCurrency> currencyList = m_storage->currencyList();
  if(currencyList.size())
  {
    for(QValueList<MyMoneyCurrency>::Iterator it = currencyList.begin(); it != currencyList.end(); ++it)
    {
      QDomElement currency = m_doc->createElement("CURRENCY");
      writeCurrency(currency, (*it));
      currencies.appendChild(currency);
    }
  }

}

void MyMoneyStorageANON::writeCurrency(QDomElement& currencyElement, const MyMoneyCurrency& currency)
{
  writeEquity(currencyElement, currency);
  currencyElement.setAttribute(QString("ppu"), currency.partsPerUnit());
  currencyElement.setAttribute(QString("scf"), currency.smallestCashFraction());
  currencyElement.setAttribute(QString("saf"), currency.smallestAccountFraction());
}

void MyMoneyStorageANON::writeReports(QDomElement& parent) const
{
  QValueList<MyMoneyReport> list = m_storage->reportList();
  QValueList<MyMoneyReport>::ConstIterator it;

  for(it = list.begin(); it != list.end(); ++it)
  {
    QDomElement child = m_doc->createElement("REPORT");
    (*it).write(child, m_doc, true);
    parent.appendChild(child);
  }
}

QString MyMoneyStorageANON::getString(const QDate& date) const
{
  QString str("");
  if(!date.isNull() && date.isValid())
  {
    str = date.toString(Qt::ISODate);
  }

  return str;
}

QDomElement MyMoneyStorageANON::writeKeyValuePairs(const QMap<QCString, QString> pairs)
{
  if(m_doc)
  {
    QDomElement keyValPairs = m_doc->createElement("KEYVALUEPAIRS");

    QMap<QCString, QString>::const_iterator it;
    for(it = pairs.begin(); it != pairs.end(); ++it)
    {
      QDomElement pair = m_doc->createElement("PAIR");
      pair.setAttribute(QString("key"), it.key());
      
      if ( zKvpXNumber.contains( it.key() ) || it.key().left(3)=="ir-" )
        pair.setAttribute(QString("value"), hideNumber(MyMoneyMoney(it.data())).toString());
      else if ( zKvpNoModify.contains( it.key() ) )
        pair.setAttribute(QString("value"), it.data());
      else
        pair.setAttribute(QString("value"), hideString(it.data()));
      keyValPairs.appendChild(pair);
    }
    return keyValPairs;
  }
  return QDomElement();
}

void MyMoneyStorageANON::setProgressCallback(void(*callback)(int, int, const QString&))
{
  m_progressCallback = callback;
}

void MyMoneyStorageANON::signalProgress(int current, int total, const QString& msg)
{
  if(m_progressCallback != 0)
    (*m_progressCallback)(current, total, msg);
}
QString MyMoneyStorageANON::hideString(const QString& _in)
{
  return QString(_in).fill('x');
}

MyMoneyMoney MyMoneyStorageANON::hideNumber(const MyMoneyMoney& _in)
{
  MyMoneyMoney result;
  static MyMoneyMoney counter = MyMoneyMoney(100,100);
  
  // preserve sign
  if ( _in.isNegative() )
    result = MyMoneyMoney(-1);
  else
    result = MyMoneyMoney(1);
  
  result = result * counter;
  counter += MyMoneyMoney("10/100");
    
  // preserve > 1000
  if ( _in >= MyMoneyMoney(1000) )
    result = result * MyMoneyMoney(1000);
  if ( _in <= MyMoneyMoney(-1000) )
    result = result * MyMoneyMoney(1000);
  
  return result.convert();
}

// vim:cin:si:ai:et:ts=2:sw=2:
