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
// Third party Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneystoragexml.h"
#include "../../kmymoneyutils.h"

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
  if(!storage)
  {
    return;
  }
  m_storage = storage;
  
  m_doc = new QDomDocument;
  if(m_doc->setContent(pDevice, FALSE))
  {
    QDomElement rootElement = m_doc->documentElement();
    if(!rootElement.isNull())
    {
      qDebug("XMLREADER: Root element of this file is %s\n", rootElement.tagName().data());

      QDomNode child = rootElement.firstChild();
      while(!child.isNull())
      {
        if(child.isElement())
        {
          QDomElement childElement = child.toElement();
          qDebug("XMLREADER: Processing child node %s", childElement.tagName().data());
          if(QString("USER") == childElement.tagName())
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
          else if(QString("KEYVALPAIRS") == childElement.tagName())
          {
            m_storage->setPairs(readKeyValuePairs(childElement));
          }
          else if(QString("SCHEDULES") == childElement.tagName())
          {
            readSchedules(childElement);
          }
        }
        child = child.nextSibling();
      }
    }
    
    delete m_doc;
    m_doc = NULL;
    m_storage = NULL;
  }
  else
  {
    throw new MYMONEYEXCEPTION("File was not parsable!");
  }
}

void MyMoneyStorageXML::writeFile(QIODevice* qf, IMyMoneySerialize* storage)
{
  if(!storage)
  {
    return;
  }
  m_storage = storage;

  qDebug("XMLWRITER: Starting file write");
  m_doc = new QDomDocument("KMYMONEY-FILE");
  QDomProcessingInstruction instruct = m_doc->createProcessingInstruction(QString("xml"), QString("version=\"1.0\" encoding=\"utf-8\""));
  m_doc->appendChild(instruct);

  QDomElement mainElement = m_doc->createElement("KMYMONEY-FILE");
  m_doc->appendChild(mainElement);

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

  QTextStream stream(qf);
  //stream.setEncoding(QTextStream::Locale);
  QCString temp = m_doc->toCString();
  stream << temp.data();

  qDebug("File contains %s", temp.data());

  delete m_doc;
  m_doc = NULL;

  // this seems to be nonsense, but it clears the dirty flag
  // as a side-effect.
  m_storage->setLastModificationDate(m_storage->lastModificationDate());

  m_storage = NULL;
}

void MyMoneyStorageXML::writeUserInformation(QDomElement& userInfo)
{
  userInfo.setAttribute(QString("name"), m_storage->userName());
  userInfo.setAttribute(QString("email"), m_storage->userEmail());

  QDomElement address = m_doc->createElement("ADDRESS");
  address.setAttribute(QString("street"), m_storage->userStreet());
  address.setAttribute(QString("city"), m_storage->userTown());
  address.setAttribute(QString("county"), m_storage->userCounty());
  address.setAttribute(QString("zipcode"), m_storage->userPostcode());
  address.setAttribute(QString("telephone"), m_storage->userTelephone());

  userInfo.appendChild(address);
}

void MyMoneyStorageXML::readUserInformation(QDomElement userElement)
{
  m_storage->setUserName(userElement.attribute(QString("name")));
  m_storage->setUserEmail(userElement.attribute(QString("email")));

  QDomElement addressNode = findChildElement(QString("ADDRESS"), userElement);
  if(!addressNode.isNull())
  {
    m_storage->setUserStreet(addressNode.attribute(QString("street")));
    m_storage->setUserTown(addressNode.attribute(QString("city")));
    m_storage->setUserCounty(addressNode.attribute(QString("county")));
    m_storage->setUserPostcode(addressNode.attribute(QString("zipcode")));
    m_storage->setUserTelephone(addressNode.attribute(QString("telephone")));
  }
}

void MyMoneyStorageXML::readInstitutions(QDomElement& institutions)
{
  unsigned long id = 0;
  QDomNode child = institutions.firstChild();
  while(!child.isNull())
  {
    if(child.isElement())
    {
      QDomElement childElement = child.toElement();
      if(QString("INSTITUTION") == childElement.tagName())
      {
        MyMoneyInstitution inst = readInstitution(childElement);

        //tell the storage objects we have a new institution.
        m_storage->loadInstitution(inst);

        id = extractId(inst.id().data());
        if(id > m_storage->institutionId())
        {
          m_storage->loadInstitutionId(id);
        }
        
        //return childElement;
      }
    }
    child = child.nextSibling();
  }
}

void MyMoneyStorageXML::writeInstitutions(QDomElement& institutions)
{
  QValueList<MyMoneyInstitution> list;
  QValueList<MyMoneyInstitution>::ConstIterator it;

  //set the nextid attribute of the INSTITUTIONS element.
  institutions.setAttribute(QString("nextid"), list.count());

  list = m_storage->institutionList();
  for(it = list.begin(); it != list.end(); ++it)
  {
    QDomElement institution = m_doc->createElement("INSTITUTION");
    writeInstitution(institution, *it);
    institutions.appendChild(institution);
  }
}

MyMoneyInstitution MyMoneyStorageXML::readInstitution(const QDomElement& institution)
{
  MyMoneyInstitution i;
  QCString id;

  i.setName(institution.attribute(QString("name")));
  i.setManager(institution.attribute(QString("manager")));
  i.setSortcode(institution.attribute(QString("sortcode")));
  id = institution.attribute(QString("id"));

  QDomElement address = findChildElement(QString("ADDRESS"), institution);
  if(!address.isNull() && address.isElement())
  {
    i.setStreet(address.attribute(QString("street")));
    i.setCity(address.attribute(QString("city")));
    i.setPostcode(address.attribute(QString("zip")));
    i.setTelephone(address.attribute(QString("telephone")));
  }

  return MyMoneyInstitution(id, i);
}

void MyMoneyStorageXML::writeInstitution(QDomElement& institution, const MyMoneyInstitution& i)
{
  institution.setAttribute(QString("id"), i.id());
  institution.setAttribute(QString("name"), i.name());
  institution.setAttribute(QString("manager"), i.manager());
  institution.setAttribute(QString("sortcode"), i.sortcode());

  QDomElement address = m_doc->createElement("ADDRESS");
  address.setAttribute(QString("street"), i.street());
  address.setAttribute(QString("city"), i.city());
  address.setAttribute(QString("zip"), i.postcode());
  address.setAttribute(QString("state"), i.town());
  address.setAttribute(QString("telephone"), i.telephone());

  institution.appendChild(address);
}

void MyMoneyStorageXML::readPayees(QDomElement& payees)
{
  unsigned long id = 0;
  QDomNode child = payees.firstChild();
  while(!child.isNull())
  {
    if(child.isElement())
    {
      QDomElement childElement = child.toElement();
      if(QString("PAYEE") == childElement.tagName())
      {
        MyMoneyPayee p = readPayee(childElement);

        //tell the storage objects we have a new institution.
        m_storage->loadPayee(p);

        id = extractId(p.id().data());
        if(id > m_storage->payeeId())
          m_storage->loadPayeeId(id);

        //return childElement;
      }
    }
    child = child.nextSibling();
  }
}

void MyMoneyStorageXML::writePayees(QDomElement& payees)
{
  QValueList<MyMoneyPayee> list;
  QValueList<MyMoneyPayee>::ConstIterator it;

  list = m_storage->payeeList();
  payees.setAttribute(QString("nextid"), list.count());

  for(it = list.begin(); it != list.end(); ++it)
  {
    QDomElement payee = m_doc->createElement("PAYEE");
    writePayee(payee, *it);
    payees.appendChild(payee);
  }
}

MyMoneyPayee MyMoneyStorageXML::readPayee(const QDomElement& payee)
{
  MyMoneyPayee p;
  QCString id;

  p.setName(payee.attribute(QString("name")));
  p.setReference(payee.attribute(QString("reference")));
  p.setEmail(payee.attribute(QString("email")));

  id = payee.attribute(QString("id"));

  QDomElement address = findChildElement(QString("ADDRESS"), payee);
  if(!address.isNull() && address.isElement())
  {
    p.setAddress(address.attribute(QString("street")));
    p.setCity(address.attribute(QString("city")));
    p.setPostcode(address.attribute(QString("zip")));
    p.setTelephone(address.attribute(QString("telephone")));
  }

  //create actual object to return to add into the engine's list of objects.
  return MyMoneyPayee(id, p);
}

void MyMoneyStorageXML::writePayee(QDomElement& payee, const MyMoneyPayee& p)
{
  payee.setAttribute(QString("name"), p.name());
  payee.setAttribute(QString("id"), p.id());
  payee.setAttribute(QString("reference"), p.reference());

  QDomElement address = m_doc->createElement("ADDRESS");
  address.setAttribute(QString("street"), p.address());
  address.setAttribute(QString("zipcode"), p.postcode());
  address.setAttribute(QString("city"), p.city());
  address.setAttribute(QString("state"), p.state());
  address.setAttribute(QString("telephone"), p.telephone());

  payee.appendChild(address);
}

void MyMoneyStorageXML::readAccounts(QDomElement& accounts)
{
  unsigned long id = 0;
  QDomNode child = accounts.firstChild();
  while(!child.isNull())
  {
    if(child.isElement())
    {
      QDomElement childElement = child.toElement();
      if(QString("ACCOUNT") == childElement.tagName())
      {
        MyMoneyAccount account = readAccount(childElement);

        //tell the storage objects we have a new institution.
        m_storage->loadAccount(account);

        id = extractId(account.id().data());
        if(id > m_storage->accountId())
        {
          m_storage->loadAccountId(id);
        }
      }
    }
    child = child.nextSibling();
  }
}

void MyMoneyStorageXML::writeAccounts(QDomElement& accounts)
{
  QValueList<MyMoneyAccount> list;
  QValueList<MyMoneyAccount>::ConstIterator it;

  list = m_storage->accountList();

  accounts.setAttribute(QString("nextid"), list.count() + 4);

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

MyMoneyAccount MyMoneyStorageXML::readAccount(const QDomElement& account)
{
  MyMoneyAccount acc;
  QCString id;
  QString tmp;
  
  acc.setName(account.attribute(QString("name")));

  qDebug("Reading information for account %s", acc.name().data());
  
  acc.setParentAccountId(QCString(account.attribute(QString("parentaccount"))));
  acc.setLastModified(getDate(account.attribute(QString("lastmodified"))));
  acc.setLastReconciliationDate(getDate(account.attribute(QString("lastreconciled"))));
  acc.setInstitutionId(QCString(account.attribute(QString("institution"))));
  acc.setNumber(account.attribute(QString("number")));
  acc.setOpeningDate(getDate(account.attribute(QString("opened"))));

  tmp = account.attribute(QString("type"));
  bool bOK = false;
  int type = tmp.toInt(&bOK);
  if(bOK)
  {
    acc.setAccountType(static_cast<MyMoneyAccount::accountTypeE>(type));
  }
  
  acc.setOpeningBalance(MyMoneyMoney(account.attribute(QString("openingbalance"))));
  acc.setDescription(account.attribute(QString("description")));
  
  id = account.attribute(QString("id"));

  qDebug("Account %s has id of %s, type of %d, parent is %s.", acc.name().data(), id.data(), type, acc.parentAccountId().data());

  /////////////////////////////////////////////////////////////////////////////////////////
  //  Process any Sub-Account information found inside the account entry.
  QDomElement subAccounts = findChildElement(QString("SUBACCOUNTS"), account);
  if(!subAccounts.isNull() && subAccounts.isElement())
  {
    QDomNode child = subAccounts.firstChild();
    while(!child.isNull())
    {
      if(child.isElement())
      {
        QDomElement childElement = child.toElement();
        if(QString("SUBACCOUNT") == childElement.tagName())
        {
          acc.addAccountId(QCString(childElement.attribute(QString("id")))); 
        }
      }
      child = child.nextSibling();
    }  
  }

  /////////////////////////////////////////////////////////////////////////////////////////
  //  Process any KeyValue pairs information found inside the account entry.
  QDomElement keyValPairs = findChildElement(QString("KEYVALUEPAIRS"), account);
  if(!keyValPairs.isNull() && keyValPairs.isElement())
  {
    acc.setPairs(readKeyValuePairs(keyValPairs));
  }
  
  return MyMoneyAccount(id, acc);
}

void MyMoneyStorageXML::writeAccount(QDomElement& account, const MyMoneyAccount& p)
{   
  account.setAttribute(QString("parentaccount"), p.parentAccountId());
  account.setAttribute(QString("lastreconciled"), getString(p.lastReconciliationDate()));
  account.setAttribute(QString("lastmodified"), getString(p.lastModified()));
  account.setAttribute(QString("institution"), p.institutionId());
  account.setAttribute(QString("opened"), getString(p.openingDate()));
  account.setAttribute(QString("number"), p.number());
  account.setAttribute(QString("openingbalance"), p.openingBalance().toString());
  account.setAttribute(QString("type"), p.accountType());  
  account.setAttribute(QString("id"), p.id());
  account.setAttribute(QString("name"), p.name());
  account.setAttribute(QString("description"), p.description());

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

void MyMoneyStorageXML::readTransactions(QDomElement& transactions)
{
  unsigned long id = 0;
  QDomNode child = transactions.firstChild();
  while(!child.isNull())
  {
    if(child.isElement())
    {
      QDomElement childElement = child.toElement();
      if(QString("TRANSACTION") == childElement.tagName())
      {
        MyMoneyTransaction transaction = readTransaction(childElement);

        //tell the storage objects we have a new institution.
        m_storage->loadTransaction(transaction);

        id = extractId(transaction.id().data());
        if(id > m_storage->transactionId())
        {
          m_storage->loadTransactionId(id);
        }
      }
    }
    child = child.nextSibling();
  }
}

void MyMoneyStorageXML::writeTransactions(QDomElement& transactions)
{
  QValueList<MyMoneyTransaction> list;
  QValueList<MyMoneyTransaction>::ConstIterator it;
  MyMoneyTransactionFilter filter;

  list = m_storage->transactionList(filter);
  transactions.setAttribute(QString("nextid"), list.count());
  
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

MyMoneyTransaction MyMoneyStorageXML::readTransaction(QDomElement& transaction)
{
  QCString id;
  MyMoneyTransaction t;

  t.setEntryDate(getDate(transaction.attribute(QString("entrydate"))));
  t.setPostDate(getDate(transaction.attribute(QString("postdate"))));
  t.setMemo(transaction.attribute(QString("memo")));

  id = transaction.attribute(QString("id"));

  qDebug("Transaction has id of %s", id.data());


  QDomElement splits = findChildElement(QString("SPLITS"), transaction);
  if(!splits.isNull() && splits.isElement())
  {
    readSplits(t, splits);
  }

  //Process any KeyValue pairs information found inside the transaction entry.
  QDomElement keyValPairs = findChildElement(QString("KEYVALUEPAIRS"), transaction);
  if(!keyValPairs.isNull() && keyValPairs.isElement())
  {
    t.setPairs(readKeyValuePairs(keyValPairs));
  }

  return MyMoneyTransaction(id, t);
}

void MyMoneyStorageXML::writeTransaction(QDomElement& transaction, const MyMoneyTransaction& tx)
{
  transaction.setAttribute(QString("id"), tx.id());
  transaction.setAttribute(QString("postdate"), getString(tx.postDate()));
  transaction.setAttribute(QString("memo"), tx.memo());
  transaction.setAttribute(QString("entrydate"), getString(tx.entryDate()));

  QDomElement splits = m_doc->createElement("SPLITS");
  QValueList<MyMoneySplit> splitList = tx.splits();
  splits.setAttribute(QString("nextid"), tx.splitCount());

  writeSplits(splits, splitList);
  transaction.appendChild(splits);

  //Add in Key-Value Pairs for transactions.
  QDomElement keyValPairs = writeKeyValuePairs(tx.pairs());
  transaction.appendChild(keyValPairs);
}

void MyMoneyStorageXML::readSchedules(QDomElement& schedules)
{
  unsigned long id = 0;
  QDomNode child = schedules.firstChild();
  while(!child.isNull())
  {
    if(child.isElement())
    {
      QDomElement childElement = child.toElement();
      if(QString("SCHEDULED_TX") == childElement.tagName())
      {
        MyMoneySchedule schedule = readSchedule(childElement);

        //tell the storage objects we have a new institution.
        m_storage->loadSchedule(schedule);
        
        id = extractId(schedule.id().data());
        if(id > m_storage->scheduleId())
        {
          m_storage->loadScheduleId(id);
        }
      }
    }
    child = child.nextSibling();
  }
}

void MyMoneyStorageXML::writeSchedules(QDomElement& scheduled)
{
  QValueList<MyMoneySchedule> list;
  QValueList<MyMoneySchedule>::ConstIterator it;

  list = m_storage->scheduleList();
  scheduled.setAttribute(QString("nextid"), list.count());

  for(it = list.begin(); it != list.end(); ++it)
  {
    QDomElement schedTx = m_doc->createElement("SCHEDULED_TX");
    writeSchedule(schedTx, *it);
    scheduled.appendChild(schedTx);
  }
}
 
MyMoneySchedule MyMoneyStorageXML::readSchedule(QDomElement& schedule)
{
  MyMoneySchedule sc;
  sc.setStartDate(getDate(schedule.attribute(QString("startDate"))));
  sc.setAutoEnter(static_cast<bool>(schedule.attribute(QString("autoEnter")).toInt()));
  sc.setLastPayment(getDate(schedule.attribute(QString("lastPayment"))));
  sc.setPaymentType(static_cast<MyMoneySchedule::paymentTypeE>(schedule.attribute(QString("paymentType")).toInt()));
  sc.setEndDate(getDate(schedule.attribute(QString("endDate"))));
  sc.setType(static_cast<MyMoneySchedule::typeE>(schedule.attribute(QString("type")).toInt()));
  sc.setId(QCString(schedule.attribute(QString("id"))));
  sc.setName(schedule.attribute(QString("name")));
  sc.setFixed(static_cast<bool>(schedule.attribute(QString("fixed")).toInt()));
  sc.setOccurence(static_cast<MyMoneySchedule::occurenceE>(schedule.attribute(QString("occurence")).toInt()));

  QDomElement payments = findChildElement(QString("PAYMENTS"), schedule);
  if(!payments.isNull() && payments.isElement())
  {
    QDomNode child = payments.firstChild();
    while(!child.isNull())
    {
      if(child.isElement())
      {
        QDomElement childElement = child.toElement();
        if(QString("PAYMENT") == childElement.tagName())
        {
          QDate date = getDate(childElement.attribute(QString("date")));
          sc.recordPayment(date);
        }
      }
      child = child.nextSibling();
    }
  }

  QDomElement transaction = findChildElement(QString("TRANSACTION"), schedule);
  if(!transaction.isNull() && transaction.isElement())
  {
    MyMoneyTransaction t = readTransaction(transaction);
    sc.setTransaction(t);
  }

  return sc;  
}

void MyMoneyStorageXML::writeSchedule(QDomElement& scheduledTx, const MyMoneySchedule& tx)
{
  scheduledTx.setAttribute(QString("name"), tx.name());
  scheduledTx.setAttribute(QString("type"), tx.type());
  scheduledTx.setAttribute(QString("occurence"), tx.occurence());
  scheduledTx.setAttribute(QString("paymentType"), tx.paymentType());
  scheduledTx.setAttribute(QString("startDate"), getString(tx.startDate()));
  scheduledTx.setAttribute(QString("endDate"), getString(tx.endDate()));
  scheduledTx.setAttribute(QString("fixed"), tx.isFixed());
  scheduledTx.setAttribute(QString("autoEnter"), tx.autoEnter());
  scheduledTx.setAttribute(QString("id"), tx.id());
  scheduledTx.setAttribute(QString("lastPayment"), getString(tx.lastPayment()));

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

void MyMoneyStorageXML::readSplits(MyMoneyTransaction& t, QDomElement& splits)
{
  QDomNode child = splits.firstChild();
  while(!child.isNull())
  {
    if(child.isElement())
    {
      QDomElement childElement = child.toElement();
      if(QString("SPLIT") == childElement.tagName())
      {
        MyMoneySplit split = readSplit(childElement);
        t.addSplit(split);
      }
    }
    child = child.nextSibling();
  }
}

void MyMoneyStorageXML::writeSplits(QDomElement& splits, const QValueList<MyMoneySplit> splitList)
{
  QValueList<MyMoneySplit>::const_iterator it;
  for(it = splitList.begin(); it != splitList.end(); ++it)
  {
    QDomElement split = m_doc->createElement("SPLIT");
    writeSplit(split, (*it));
    splits.appendChild(split);
  }
}

MyMoneySplit MyMoneyStorageXML::readSplit(QDomElement& splitElement)
{
  MyMoneySplit split;
  QString strTmp;
  
  split.setPayeeId(QCString(splitElement.attribute(QString("payee"))));
  split.setReconcileDate(getDate(splitElement.attribute(QString("reconciledate"))));
  split.setAction(QCString(splitElement.attribute(QString("action"))));
  split.setReconcileFlag(static_cast<MyMoneySplit::reconcileFlagE>(splitElement.attribute(QString("reconcileflag")).toInt()));
  split.setMemo(splitElement.attribute(QString("memo")));
  split.setValue(MyMoneyMoney(splitElement.attribute(QString("value"))));
  split.setAccountId(QCString(splitElement.attribute(QString("account"))));
 
  return split;
}

void MyMoneyStorageXML::writeSplit(QDomElement& splitElement, const MyMoneySplit& split)
{
  splitElement.setAttribute(QString("payee"), split.payeeId());
  splitElement.setAttribute(QString("reconciledate"), getString(split.reconcileDate()));
  splitElement.setAttribute(QString("action"), split.action());
  splitElement.setAttribute(QString("reconcileflag"), split.reconcileFlag());
  splitElement.setAttribute(QString("value"), split.value().toString());
  splitElement.setAttribute(QString("memo"), split.memo());
  splitElement.setAttribute(QString("id"), split.id());
  splitElement.setAttribute(QString("account"), split.accountId());
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
  while(!child.isNull())
  {
    if(child.isElement())
    {
      QDomElement childElement = child.toElement();
      if(QString("PAIR") == childElement.tagName())
      {
        QCString key = QCString(childElement.attribute(QString("key")));
        QString value = childElement.attribute(QString("value"));

        pairs.insert(key, value);
      }
    }
    child = child.nextSibling();
  }

  return pairs;
}
