/***************************************************************************
                          mymoneystoragexml.cpp  -  description
                             -------------------
    begin                : Thu Oct 24 2002
    copyright            : (C) 2000-2002 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
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

#include "mymoneystoragexml.h"
#include "../../kmymoneyutils.h"
#include "../mymoneyreport.h"
#include "../mymoneyinstitution.h"

unsigned int MyMoneyStorageXML::fileVersionRead = 0;
unsigned int MyMoneyStorageXML::fileVersionWrite = 0;

MyMoneyStorageXML::MyMoneyStorageXML()
{
  m_storage = NULL;
  m_doc     = NULL;
}

MyMoneyStorageXML::~MyMoneyStorageXML()
{

}

//Function to read in the file, send to XML parser.
void MyMoneyStorageXML::readFile(QIODevice* pDevice, IMyMoneySerialize* storage)
{
  Q_CHECK_PTR(storage);
  Q_CHECK_PTR(pDevice);
  if(!storage)
  {
    return;
  }
  m_storage = storage;

  m_doc = new QDomDocument;
  Q_CHECK_PTR(m_doc);
  if(m_doc->setContent(pDevice, FALSE))
  {
    QDomElement rootElement = m_doc->documentElement();
    if(!rootElement.isNull())
    {
      // extract the base currency
      QDomNode child = rootElement.firstChild();
      while(!child.isNull() && child.isElement() && m_baseCurrencyId.isEmpty()) {
        QDomElement childElement = child.toElement();
        if(QString("KEYVALUEPAIRS") == childElement.tagName()) {
          m_storage->setPairs(readKeyValuePairs(childElement));
          m_baseCurrencyId = m_storage->pairs()["kmm-baseCurrency"];
        }
        child = child.nextSibling();
      }

      // qDebug("XMLREADER: Root element of this file is %s\n", rootElement.tagName().data());

      child = rootElement.firstChild();
      while(!child.isNull() && child.isElement())
      {
        QDomElement childElement = child.toElement();
        // qDebug("XMLREADER: Processing child node %s", childElement.tagName().data());
        if(QString("FILEINFO") == childElement.tagName())
        {
          readFileInformation(childElement);
        }
        else if(QString("USER") == childElement.tagName())
        {
          readUserInformation(childElement);
        }
        else if(QString("INSTITUTIONS") == childElement.tagName())
        {
          readInstitutions(childElement);
        }
        else if(QString("PAYEES") == childElement.tagName())
        {
          readPayees(childElement);
        }
        else if(QString("ACCOUNTS") == childElement.tagName())
        {
          readAccounts(childElement);
        }
        else if(QString("TRANSACTIONS") == childElement.tagName())
        {
          readTransactions(childElement);
        }
        else if(QString("SCHEDULES") == childElement.tagName())
        {
          readSchedules(childElement);
        }
        else if(QString("EQUITIES") == childElement.tagName()
             || QString("SECURITIES") == childElement.tagName())
        {
          readSecurities(childElement);
        }
        else if(QString("CURRENCIES") == childElement.tagName())
        {
          readCurrencies(childElement);
        }
        else if(QString("PRICES") == childElement.tagName())
        {
          readPrices(childElement);
        }
        else if(QString("REPORTS") == childElement.tagName())
        {
          readReports(childElement);
        }
        child = child.nextSibling();
      }
    }

    delete m_doc;
    m_doc = NULL;

    // this seems to be nonsense, but it clears the dirty flag
    // as a side-effect.
    m_storage->setLastModificationDate(m_storage->lastModificationDate());
    m_storage = NULL;

    //hides the progress bar.
    signalProgress(-1, -1);
  }
  else
  {
    delete m_doc;
    m_doc = NULL;
    throw new MYMONEYEXCEPTION("File was not parsable!");
  }
}

void MyMoneyStorageXML::writeFile(QIODevice* qf, IMyMoneySerialize* storage)
{
  Q_CHECK_PTR(qf);
  Q_CHECK_PTR(storage);
  if(!storage)
  {
    return;
  }
  m_storage = storage;

  // qDebug("XMLWRITER: Starting file write");
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

  QDomElement equities = m_doc->createElement("SECURITIES");
  writeSecurities(equities);
  mainElement.appendChild(equities);

  QDomElement currencies = m_doc->createElement("CURRENCIES");
  writeCurrencies(currencies);
  mainElement.appendChild(currencies);

  QDomElement prices = m_doc->createElement("PRICES");
  writePrices(prices);
  mainElement.appendChild(prices);

  QDomElement reports = m_doc->createElement("REPORTS");
  writeReports(reports);
  mainElement.appendChild(reports);

  QTextStream stream(qf);
#if KDE_IS_VERSION(3,2,0)
  stream.setEncoding(QTextStream::UnicodeUTF8);
  stream << m_doc->toString();
#else
  //stream.setEncoding(QTextStream::Locale);
  QCString temp = m_doc->toCString();
  stream << temp.data();
#endif

  // qDebug("File contains %s", temp.data());

  delete m_doc;
  m_doc = NULL;

  //hides the progress bar.
  signalProgress(-1, -1);

  // this seems to be nonsense, but it clears the dirty flag
  // as a side-effect.
  m_storage->setLastModificationDate(m_storage->lastModificationDate());

  m_storage = NULL;
}

void MyMoneyStorageXML::readFileInformation(QDomElement fileInfo)
{
  signalProgress(0, 3, QObject::tr("Loading file information..."));

  QDomElement temp = findChildElement(QString("CREATION_DATE"), fileInfo);
  QString strDate = QStringEmpty(temp.attribute("date"));
  m_storage->setCreationDate(getDate(strDate));
  signalProgress(1, 0);

  temp = findChildElement(QString("LAST_MODIFIED_DATE"), fileInfo);
  strDate = QStringEmpty(temp.attribute("date"));
  m_storage->setLastModificationDate(getDate(strDate));
  signalProgress(2, 0);

  temp = findChildElement(QString("VERSION"), fileInfo);
  QString strVersion = QStringEmpty(temp.attribute("id"));
  fileVersionRead = strVersion.toUInt(NULL, 16);
  fileVersionWrite = fileVersionRead;
  signalProgress(3, 0);
}

void MyMoneyStorageXML::writeFileInformation(QDomElement& fileInfo)
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

void MyMoneyStorageXML::writeUserInformation(QDomElement& userInfo)
{
  MyMoneyPayee user = m_storage->user();
  userInfo.setAttribute(QString("name"), user.name());
  userInfo.setAttribute(QString("email"), user.email());

  QDomElement address = m_doc->createElement("ADDRESS");
  address.setAttribute(QString("street"), user.address());
  address.setAttribute(QString("city"), user.city());
  address.setAttribute(QString("county"), user.state());
  address.setAttribute(QString("zipcode"), user.postcode());
  address.setAttribute(QString("telephone"), user.telephone());

  userInfo.appendChild(address);
}

void MyMoneyStorageXML::readUserInformation(QDomElement userElement)
{
  signalProgress(0, 1, QObject::tr("Loading user information..."));

  MyMoneyPayee user;
  user.setName(QStringEmpty(userElement.attribute(QString("name"))));
  user.setEmail(QStringEmpty(userElement.attribute(QString("email"))));

  QDomElement addressNode = findChildElement(QString("ADDRESS"), userElement);
  if(!addressNode.isNull())
  {
    user.setAddress(QStringEmpty(addressNode.attribute(QString("street"))));
    user.setCity(QStringEmpty(addressNode.attribute(QString("city"))));
    user.setState(QStringEmpty(addressNode.attribute(QString("county"))));
    user.setPostcode(QStringEmpty(addressNode.attribute(QString("zipcode"))));
    user.setTelephone(QStringEmpty(addressNode.attribute(QString("telephone"))));
  }

  m_storage->setUser(user);
  signalProgress(1, 0);
}

void MyMoneyStorageXML::readInstitutions(QDomElement& institutions)
{
  unsigned long id = 0;
  QDomNode child = institutions.firstChild();
  uint nCount = getChildCount(institutions);
  int x = 0;
  signalProgress(0, nCount, QObject::tr("Loading institutions..."));

  while(!child.isNull())
  {
    if(child.isElement())
    {
      QDomElement childElement = child.toElement();
      if(QString("INSTITUTION") == childElement.tagName())
      {
        MyMoneyInstitution inst;
        inst.readXML(childElement);

        //tell the storage objects we have a new institution.
        m_storage->loadInstitution(inst);

        id = extractId(inst.id().data());
        if(id > m_storage->institutionId())
        {
          m_storage->loadInstitutionId(id);
        }

        //return childElement;
      }

      signalProgress(x++, 0);
    }
    child = child.nextSibling();
  }
}

void MyMoneyStorageXML::writeInstitutions(QDomElement& institutions)
{
  const QValueList<MyMoneyInstitution> list = m_storage->institutionList();
  QValueList<MyMoneyInstitution>::ConstIterator it;

  for(it = list.begin(); it != list.end(); ++it)
    writeInstitution(institutions, *it);
}

void MyMoneyStorageXML::writeInstitution(QDomElement& institution, const MyMoneyInstitution& i)
{
  i.writeXML(*m_doc, institution);
}

void MyMoneyStorageXML::readPayees(QDomElement& payees)
{
  unsigned long id = 0;
  QDomNode child = payees.firstChild();
  int x = 0;
  signalProgress(0, getChildCount(payees), QObject::tr("Loading payees..."));

  while(!child.isNull())
  {
    if(child.isElement())
    {
      QDomElement childElement = child.toElement();
      if(QString("PAYEE") == childElement.tagName())
      {
        MyMoneyPayee p;
        p.readXML(childElement);

        //tell the storage objects we have a new institution.
        m_storage->loadPayee(p);

        id = extractId(p.id().data());
        if(id > m_storage->payeeId())
          m_storage->loadPayeeId(id);

        //return childElement;
      }
    }
    child = child.nextSibling();
    signalProgress(x++, 0);
  }
}

void MyMoneyStorageXML::writePayees(QDomElement& payees)
{
  const QValueList<MyMoneyPayee> list = m_storage->payeeList();
  QValueList<MyMoneyPayee>::ConstIterator it;

  for(it = list.begin(); it != list.end(); ++it)
    writePayee(payees, *it);
}

void MyMoneyStorageXML::writePayee(QDomElement& payee, const MyMoneyPayee& p)
{
  p.writeXML(*m_doc, payee);
}

void MyMoneyStorageXML::readAccounts(QDomElement& accounts)
{
  unsigned long id = 0;
  QDomNode child = accounts.firstChild();
  int x = 0;
  signalProgress(0, getChildCount(accounts), QObject::tr("Loading accounts..."));

  while(!child.isNull() && child.isElement())
  {
    QDomElement childElement = child.toElement();
    if(QString("ACCOUNT") == childElement.tagName())
    {
      // make sure the account has a currency
      if(childElement.attribute("currency").isEmpty()) {
        childElement.setAttribute("currency", m_baseCurrencyId);
      }
      MyMoneyAccount account;
      account.readXML(childElement);

      //tell the storage objects we have a new account.
      m_storage->loadAccount(account);

      id = extractId(account.id().data());
      if(id > m_storage->accountId())
      {
        m_storage->loadAccountId(id);
      }
    }

    signalProgress(x++, 0);

    child = child.nextSibling();
  }
}

void MyMoneyStorageXML::writeAccounts(QDomElement& accounts)
{
  const QValueList<MyMoneyAccount> list = m_storage->accountList();
  QValueList<MyMoneyAccount>::ConstIterator it;

  writeAccount(accounts, m_storage->asset());
  writeAccount(accounts, m_storage->liability());
  writeAccount(accounts, m_storage->expense());
  writeAccount(accounts, m_storage->income());
  writeAccount(accounts, m_storage->equity());

  signalProgress(0, list.count(), QObject::tr("Saving accounts..."));
  int i = 0;
  for(it = list.begin(); it != list.end(); ++it, ++i) {
    writeAccount(accounts, *it);
    signalProgress(i, 0);
  }
}

void MyMoneyStorageXML::writeAccount(QDomElement& account, const MyMoneyAccount& p)
{
  p.writeXML(*m_doc, account);
}

void MyMoneyStorageXML::readTransactions(QDomElement& transactions)
{
  unsigned long id = 0;
  QDomNode child = transactions.firstChild();
  int x = 0;
  signalProgress(0, getChildCount(transactions), QObject::tr("Loading transactions..."));

  while(!child.isNull() && child.isElement())
  {
    QDomElement childElement = child.toElement();
    if(QString("TRANSACTION") == childElement.tagName())
    {
      MyMoneyTransaction transaction;
      transaction.readXML(childElement);

      //tell the storage objects we have a new institution.
      m_storage->loadTransaction(transaction);

      id = extractId(transaction.id().data());
      if(id > m_storage->transactionId())
      {
        m_storage->loadTransactionId(id);
      }
    }
    child = child.nextSibling();
    signalProgress(x++, 0);
  }
}

void MyMoneyStorageXML::writeTransactions(QDomElement& transactions)
{
  MyMoneyTransactionFilter filter;
  const QValueList<MyMoneyTransaction> list = m_storage->transactionList(filter);
  QValueList<MyMoneyTransaction>::ConstIterator it;

  signalProgress(0, list.count(), QObject::tr("Saving transactions..."));

  int i = 0;
  for(it = list.begin(); it != list.end(); ++it, ++i)
  {
    writeTransaction(transactions, *it);
    signalProgress(i, 0);
  }
}

void MyMoneyStorageXML::writeTransaction(QDomElement& transaction, const MyMoneyTransaction& tx)
{
  tx.writeXML(*m_doc, transaction);
}

void MyMoneyStorageXML::readSchedules(QDomElement& schedules)
{
  unsigned long id = 0;
  QDomNode child = schedules.firstChild();
  int x = 0;
  signalProgress(0, getChildCount(schedules), QObject::tr("Loading schedules..."));

  while(!child.isNull() && child.isElement())
  {
    QDomElement childElement = child.toElement();
    if(QString("SCHEDULED_TX") == childElement.tagName())
    {
      MyMoneySchedule schedule;
      schedule.readXML(childElement);

      //tell the storage objects we have a new schedule.
      m_storage->loadSchedule(schedule);

      id = extractId(schedule.id().data());
      if(id > m_storage->scheduleId())
      {
        m_storage->loadScheduleId(id);
      }
    }
    child = child.nextSibling();
    signalProgress(x++, 0);
  }
}

void MyMoneyStorageXML::writeSchedules(QDomElement& scheduled)
{
  const QValueList<MyMoneySchedule> list = m_storage->scheduleList();
  QValueList<MyMoneySchedule>::ConstIterator it;

  for(it = list.begin(); it != list.end(); ++it)
  {
    this->writeSchedule(scheduled, *it);
  }
}

void MyMoneyStorageXML::writeSchedule(QDomElement& scheduledTx, const MyMoneySchedule& tx)
{
  tx.writeXML(*m_doc, scheduledTx);
}

void MyMoneyStorageXML::writeSecurities(QDomElement& equities)
{
  const QValueList<MyMoneySecurity> securityList = m_storage->securityList();
  if(securityList.size())
  {
    for(QValueList<MyMoneySecurity>::ConstIterator it = securityList.begin(); it != securityList.end(); ++it)
    {
      QDomElement security = m_doc->createElement("SECURITY");
      writeSecurity(security, (*it));
      equities.appendChild(security);
    }
  }
}

void MyMoneyStorageXML::writeSecurity(QDomElement& securityElement, const MyMoneySecurity& security)
{
  securityElement.setAttribute(QString("name"), security.name());
  securityElement.setAttribute(QString("symbol"), security.tradingSymbol());
  securityElement.setAttribute(QString("type"), static_cast<int>(security.securityType()));
  securityElement.setAttribute(QString("id"), security.id());
  securityElement.setAttribute(QString("saf"), security.smallestAccountFraction());
  if(!security.isCurrency())
    securityElement.setAttribute(QString("trading-currency"), security.tradingCurrency());

  //Add in Key-Value Pairs for security
  QDomElement keyValPairs = writeKeyValuePairs(security.pairs());
  securityElement.appendChild(keyValPairs);
}

void MyMoneyStorageXML::readSecurities(QDomElement& securities)
{
  unsigned long id = 0;
  QDomNode child = securities.firstChild();
  while(!child.isNull() && child.isElement())
  {
    QDomElement childElement = child.toElement();
    if(QString("EQUITY") == childElement.tagName()
    || QString("SECURITY") == childElement.tagName())
    {
      MyMoneySecurity security = readSecurity(childElement);

      //tell the storage objects we have a new security object.
      m_storage->loadSecurity(security);

      id = extractId(security.id().data());
      if(id > m_storage->securityId())
      {
        m_storage->loadSecurityId(id);
      }
    }
    child = child.nextSibling();
  }
}

void MyMoneyStorageXML::writeCurrencies(QDomElement& currencies)
{
  const QValueList<MyMoneySecurity> currencyList = m_storage->currencyList();
  if(currencyList.size())
  {
    for(QValueList<MyMoneySecurity>::ConstIterator it = currencyList.begin(); it != currencyList.end(); ++it)
    {
      QDomElement currency = m_doc->createElement("CURRENCY");
      writeCurrency(currency, (*it));
      currencies.appendChild(currency);
    }
  }
}

void MyMoneyStorageXML::writeCurrency(QDomElement& currencyElement, const MyMoneySecurity& currency)
{
  writeSecurity(currencyElement, currency);
  currencyElement.setAttribute(QString("ppu"), currency.partsPerUnit());
  currencyElement.setAttribute(QString("scf"), currency.smallestCashFraction());
  currencyElement.setAttribute(QString("saf"), currency.smallestAccountFraction());
}

void MyMoneyStorageXML::readCurrencies(QDomElement& currencies)
{
  QDomNode child = currencies.firstChild();
  while(!child.isNull() && child.isElement())
  {
    QDomElement childElement = child.toElement();
    if(QString("CURRENCY") == childElement.tagName())
    {
      MyMoneySecurity currency = readCurrency(childElement);

      //tell the storage objects we have a new currency object.
      m_storage->loadCurrency(currency);
    }
    child = child.nextSibling();
  }
}

const MyMoneySecurity MyMoneyStorageXML::readCurrency(QDomElement& currencyElement)
{
  MyMoneySecurity c(readSecurity(currencyElement));

  c.setPartsPerUnit(currencyElement.attribute(QString("ppu")).toInt());
  c.setSmallestCashFraction(currencyElement.attribute(QString("scf")).toInt());
  c.setSmallestAccountFraction(currencyElement.attribute(QString("saf")).toInt());

  return c;
}

MyMoneySecurity MyMoneyStorageXML::readSecurity(QDomElement& securityElement)
{
  QCString id;
  MyMoneySecurity e;

  e.setName(QStringEmpty(securityElement.attribute(QString("name"))));
  e.setTradingSymbol(QStringEmpty(securityElement.attribute(QString("symbol"))));
  e.setSecurityType(static_cast<MyMoneySecurity::eSECURITYTYPE>(securityElement.attribute(QString("type")).toInt()));
  // some old versions used to store type NONE
  // let's default this to stock
  if(e.securityType() == MyMoneySecurity::SECURITY_NONE)
    e.setSecurityType(MyMoneySecurity::SECURITY_STOCK);

  e.setTradingCurrency(QCStringEmpty(securityElement.attribute(QString("trading-currency"))));
  if(e.tradingCurrency().isEmpty()) {
    e.setTradingCurrency(m_baseCurrencyId);
  }

  int saf = securityElement.attribute(QString("saf")).toInt();
  if(saf == 0)
    saf = 100;
  e.setSmallestAccountFraction(saf);

  id = QStringEmpty(securityElement.attribute(QString("id")));

  // FIXME: keep the history part for some time. After 0.8 this can be wiped out
  QDomElement history = findChildElement(QString("HISTORY"), securityElement);
  if(!history.isNull() && history.isElement())
  {
    QDomNode child = history.firstChild();
    while(!child.isNull())
    {
      if(child.isElement())
      {
        QDomElement childElement = child.toElement();
        if(QString("ENTRY") == childElement.tagName())
        {
          QDate date = getDate(QStringEmpty(childElement.attribute(QString("date"))));
          MyMoneyMoney money(QStringEmpty(childElement.attribute(QString("price"))));
          // e.addPriceHistory(date, money);
          MyMoneyPrice price(id, m_baseCurrencyId, date, money);
          m_storage->addPrice(price);
        }
      }
      child = child.nextSibling();
    }
  }

  /////////////////////////////////////////////////////////////////////////////////////////
  //  Process any KeyValue pairs information found inside the security entry.
  QDomElement keyValPairs = findChildElement(QString("KEYVALUEPAIRS"), securityElement);
  if(!keyValPairs.isNull() && keyValPairs.isElement())
  {
    e.setPairs(readKeyValuePairs(keyValPairs));
  }

  return MyMoneySecurity(id, e);
}

void MyMoneyStorageXML::readReports(QDomElement& reports)
{
  QDomNode child = reports.firstChild();
  unsigned progress = 0;
  signalProgress(0, getChildCount(reports), QObject::tr("Loading reports..."));

  while(!child.isNull())
  {
    if(child.isElement())
    {
      MyMoneyReport report;
      if ( report.read(child.toElement() ) )
      {
        m_storage->loadReport(report);

        unsigned long id = extractId(report.id());
        if(id > m_storage->reportId())
          m_storage->loadReportId(id);
      }
    }
    signalProgress(progress++, 0);
    child = child.nextSibling();
  }
}

void MyMoneyStorageXML::writeReports(QDomElement& parent)
{
  const QValueList<MyMoneyReport> list = m_storage->reportList();
  QValueList<MyMoneyReport>::ConstIterator it;

  signalProgress(0, list.count(), QObject::tr("Saving reports..."));
  unsigned i = 0;
  for(it = list.begin(); it != list.end(); ++it)
  {
    (*it).writeXML(*m_doc, parent);
    signalProgress(++i, 0);
  }
}

const unsigned long MyMoneyStorageXML::extractId(const QCString& txt) const
{
  int pos;
  unsigned long rc = 0;

  pos = txt.find(QRegExp("\\d+"), 0);
  if(pos != -1) {
    rc = atol(txt.mid(pos));
  }
  return rc;
}

QDate MyMoneyStorageXML::getDate(const QString& strText) const
{
  QDate date;
  if(strText.length())
  {
    QDate date = QDate::fromString(strText, Qt::ISODate);
    if(!date.isNull() && date.isValid())
    {
      return date;
    }
    else
    {
      return QDate();
    }
  }

  return date;
}

QString MyMoneyStorageXML::getString(const QDate& date) const
{
  QString str("");
  if(!date.isNull() && date.isValid())
  {
    str = date.toString(Qt::ISODate);
  }

  return str;
}

QDomElement MyMoneyStorageXML::findChildElement(const QString& name, const QDomElement& root)
{
  QDomNode child = root.firstChild();
  while(!child.isNull())
  {
    if(child.isElement())
    {
      QDomElement childElement = child.toElement();
      if(name == childElement.tagName())
      {
        return childElement;
      }
    }

    child = child.nextSibling();
  }
  return QDomElement();
}

QDomElement MyMoneyStorageXML::writeKeyValuePairs(const QMap<QCString, QString> pairs)
{
  if(m_doc)
  {
    QDomElement keyValPairs = m_doc->createElement("KEYVALUEPAIRS");

    QMap<QCString, QString>::const_iterator it;
    for(it = pairs.begin(); it != pairs.end(); ++it)
    {
      QDomElement pair = m_doc->createElement("PAIR");
      pair.setAttribute(QString("key"), it.key());
      pair.setAttribute(QString("value"), it.data());
      keyValPairs.appendChild(pair);
    }
    return keyValPairs;
  }
  return QDomElement();
}

QMap<QCString, QString> MyMoneyStorageXML::readKeyValuePairs(QDomElement& element)
{
  QMap<QCString, QString> pairs;
  QDomNode child = element.firstChild();
  while(!child.isNull() && child.isElement())
  {
    QDomElement childElement = child.toElement();
    if(QString("PAIR") == childElement.tagName())
    {
      QCString key = QCString(childElement.attribute(QString("key")));
      QString value = childElement.attribute(QString("value"));

      pairs.insert(key, value);
    }

    child = child.nextSibling();
  }

  return pairs;
}

void MyMoneyStorageXML::readPrices(QDomElement& prices)
{
  QDomNode child = prices.firstChild();
  int x = 0;
  signalProgress(0, getChildCount(prices), QObject::tr("Loading pricetable ..."));

  while(!child.isNull())
  {
    if(child.isElement())
    {
      QDomElement childElement = child.toElement();
      if(QString("PRICEPAIR") == childElement.tagName())
      {
        readPricePair(childElement);
      }
    }
    child = child.nextSibling();
    signalProgress(x++, 0);
  }
  signalProgress(-1, -1);
}

void MyMoneyStorageXML::readPricePair(const QDomElement& pricePair)
{
  QCString from = QCString(pricePair.attribute(QString("from")));
  QCString to = QCString(pricePair.attribute(QString("to")));
  QDomNode child = pricePair.firstChild();

  while(!child.isNull())
  {
    if(child.isElement())
    {
      QDomElement childElement = child.toElement();
      if(QString("PRICE") == childElement.tagName())
      {
        MyMoneyPrice p = readPrice(from, to, childElement);

        //tell the storage objects we have a new price.
        m_storage->addPrice(p);
      }
    }
    child = child.nextSibling();
  }
}

const MyMoneyPrice MyMoneyStorageXML::readPrice(const QCString& from, const QCString& to, const QDomElement& price)
{
  QDate date;
  MyMoneyMoney rate;
  QString source;

  date = QDate::fromString(price.attribute(QString("date")), Qt::ISODate);
  rate = MyMoneyMoney(price.attribute(QString("price")));
  source = price.attribute(QString("source"));

  //create actual object to return to add into the engine's list of objects.
  return MyMoneyPrice(from, to, date, rate, source);
}

void MyMoneyStorageXML::writePrices(QDomElement& prices)
{
  const MyMoneyPriceList list = m_storage->priceList();
  MyMoneyPriceList::ConstIterator it;

  for(it = list.begin(); it != list.end(); ++it)
  {
    QDomElement price = m_doc->createElement("PRICEPAIR");
    price.setAttribute(QString("from"), it.key().first);
    price.setAttribute(QString("to"), it.key().second);
    writePricePair(price, *it);
    prices.appendChild(price);
  }
}

void MyMoneyStorageXML::writePricePair(QDomElement& price, const MyMoneyPriceEntries& p)
{
  MyMoneyPriceEntries::ConstIterator it;
  for(it = p.begin(); it != p.end(); ++it) {
    QDomElement entry = m_doc->createElement("PRICE");
    writePrice(entry, *it);
    price.appendChild(entry);
  }
}

void MyMoneyStorageXML::writePrice(QDomElement& price, const MyMoneyPrice& p)
{
  price.setAttribute(QString("date"), p.date().toString(Qt::ISODate));
  price.setAttribute(QString("price"), p.rate().toString());
  price.setAttribute(QString("source"), p.source());
}

void MyMoneyStorageXML::setProgressCallback(void(*callback)(int, int, const QString&))
{
  m_progressCallback = callback;
}

void MyMoneyStorageXML::signalProgress(int current, int total, const QString& msg)
{
  if(m_progressCallback != 0)
    (*m_progressCallback)(current, total, msg);
}

const uint MyMoneyStorageXML::getChildCount(const QDomElement& element) const
{
  QDomNodeList tempList = element.childNodes();
  return tempList.count();
}

/*!
    This convenience function returns all of the remaining data in the
    device.

    @note It's copied from the original Qt sources and modified to
          fix a problem with KFilterDev that does not correctly return
          atEnd() status in certain circumstances which caused our
          application to lock at startup.
*/
QByteArray QIODevice::readAll()
{
  if ( isDirectAccess() ) {
    // we know the size
    int n = size()-at(); // ### fix for 64-bit or large files?
    int totalRead = 0;
    QByteArray ba( n );
    char* c = ba.data();
    while ( n ) {
      int r = readBlock( c, n );
      if ( r < 0 )
        return QByteArray();
      n -= r;
      c += r;
      totalRead += r;
      // If we have a translated file, then it is possible that
      // we read less bytes than size() reports
      if ( atEnd() ) {
        ba.resize( totalRead );
        break;
      }
    }
    return ba;
  } else {
    // read until we reach the end
    const int blocksize = 512;
    int nread = 0;
    QByteArray ba;
    int r = 1;
    while ( !atEnd() && r != 0) {
      ba.resize( nread + blocksize );
      r = readBlock( ba.data()+nread, blocksize );
      if ( r < 0 )
        return QByteArray();
      nread += r;
    }
    ba.resize( nread );
    return ba;
  }
}

