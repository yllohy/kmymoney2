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

MyMoneyStorageXML::MyMoneyStorageXML()// : xmlpp::SaxParser(false)
{
  m_pStorage            = NULL;
}

MyMoneyStorageXML::~MyMoneyStorageXML()
{
  
}

//Function to read in the file, send to XML parser.
void MyMoneyStorageXML::readFile(QIODevice* pDevice, IMyMoneySerialize* storage)
{
  //QTextStream stream(pDevice);
  //QString strEntireFile = stream.read();
  //qDebug("XMLREADER: entire file is %s\n", strEntireFile.data());
    
  QDomDocument *pDoc = new QDomDocument;
  if(pDoc->setContent(pDevice, FALSE))
  {
    QDomElement rootElement = pDoc->documentElement();
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
            readUserInformation(pDoc, childElement, storage);
          }
          else if(QString("INSTITUTIONS") == childElement.tagName())
          {
            readInstitutions(pDoc, childElement, storage);
          }
          else if(QString("PAYEES") == childElement.tagName())
          {
            //readPayees(pDoc, childElement, storage);
          }
          else if(QString("ACCOUNTS") == childElement.tagName())
          {
            //readAccounts(pDoc, childElement, storage);
          }
          else if(QString("TRANSACTIONS") == childElement.tagName())
          {
            //readTransactions(pDoc, childElement, storage);
          }
          else if(QString("KEYVALPAIRS") == childElement.tagName())
          {

          }
          else if(QString("SCHEDULES") == childElement.tagName())
          {
            //readSchedules(pDoc, childElement, storage);
          }
        }
        child = child.nextSibling();
      }
    }
  }
  else
  {
    throw new MYMONEYEXCEPTION("File was not parsable!");
  }
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

void MyMoneyStorageXML::writeUserInformation(QDomDocument *pDoc, QDomElement& userInfo, IMyMoneySerialize* storage)
{
  userInfo.setAttribute(QString("name"), storage->userName());
  userInfo.setAttribute(QString("email"), storage->userEmail());

  QDomElement address = pDoc->createElement("ADDRESS");
  address.setAttribute(QString("street"), storage->userStreet());
  address.setAttribute(QString("city"), storage->userTown());
  address.setAttribute(QString("county"), storage->userCounty());
  address.setAttribute(QString("zipcode"), storage->userPostcode());
  address.setAttribute(QString("telephone"), storage->userTelephone());

  userInfo.appendChild(address);
}

void MyMoneyStorageXML::readUserInformation(QDomDocument* pDoc, QDomElement userElement, IMyMoneySerialize* storage)
{
  storage->setUserName(userElement.attribute(QString("name")));
  storage->setUserEmail(userElement.attribute(QString("email")));

  QDomElement addressNode = findChildElement(QString("ADDRESS"), userElement);
  if(!addressNode.isNull())
  {
    storage->setUserStreet(addressNode.attribute(QString("street")));
    storage->setUserTown(addressNode.attribute(QString("city")));
    storage->setUserCounty(addressNode.attribute(QString("county")));
    storage->setUserPostcode(addressNode.attribute(QString("zipcode")));
    storage->setUserTelephone(addressNode.attribute(QString("telephone")));
  }
}

void MyMoneyStorageXML::writeFile(QIODevice* qf, IMyMoneySerialize* storage)
{
  qDebug("XMLWRITER: Starting file write");
  QDomDocument *pDoc = new QDomDocument("KMYMONEY-FILE");
  QDomProcessingInstruction instruct = pDoc->createProcessingInstruction(QString("xml"), QString("version=\"1.0\" encoding=\"utf-8\""));
  pDoc->appendChild(instruct);

  QDomElement mainElement = pDoc->createElement("KMYMONEY-FILE");
  pDoc->appendChild(mainElement);
  
  QDomElement userInfo = pDoc->createElement("USER");
  writeUserInformation(pDoc, userInfo, storage);
  mainElement.appendChild(userInfo);

  QDomElement institutions = pDoc->createElement("INSTITUTIONS");
  writeInstitutions(pDoc, institutions, storage);
  mainElement.appendChild(institutions);

  QDomElement payees = pDoc->createElement("PAYEES");
  writePayees(pDoc, payees, storage);
  mainElement.appendChild(payees);

  QDomElement accounts = pDoc->createElement("ACCOUNTS");
  writeAccounts(pDoc, accounts, storage);
  mainElement.appendChild(accounts);

  QDomElement transactions = pDoc->createElement("TRANSACTIONS");
  writeTransactions(pDoc, transactions, storage);
  mainElement.appendChild(transactions);

  QDomElement keyvalpairs = pDoc->createElement("KEYVALPAIRS");
  //writeGlobalKeyValuePairs(pDoc, keyvalpairs, storage);
  mainElement.appendChild(keyvalpairs);
  
  QDomElement schedules = pDoc->createElement("SCHEDULES");
  writeSchedules(pDoc, schedules, storage);
  mainElement.appendChild(schedules);

  QTextStream stream(qf);
  //stream.setEncoding(QTextStream::Locale);
  QString temp = pDoc->toString();
  stream << temp.data();

  qDebug("File contains %s", temp.data());
  
  delete pDoc;
  pDoc = NULL;
}

void MyMoneyStorageXML::readInstitutions(QDomDocument *pDoc, QDomElement& institutions, IMyMoneySerialize* storage)
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
        storage->loadInstitution(inst);

        id = extractId(inst.id().data());
        if(id > storage->institutionId())
        {
          storage->loadInstitutionId(id);
        }
        
        //return childElement;
      }
    }
    child = child.nextSibling();
  }
}

MyMoneyInstitution MyMoneyStorageXML::readInstitution(const QDomElement& institution)
{
  MyMoneyInstitution i;
  QCString id;
  Q_INT32 version;
  QString tmp_s;

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

  /*QCStringList list;
  s >> list;
  QCStringList::ConstIterator it;
  for(it = list.begin(); it != list.end(); ++it)
    i.addAccountId(*it);
                                        */
  return MyMoneyInstitution(id, i);
  
}

void MyMoneyStorageXML::writeInstitution(QDomDocument *pDoc, QDomElement& institution, const MyMoneyInstitution& i)
{
  institution.setAttribute(QString("id"), i.id());
  institution.setAttribute(QString("name"), i.name());
  institution.setAttribute(QString("manager"), i.manager());
  institution.setAttribute(QString("sortcode"), i.sortcode());

  QDomElement address = pDoc->createElement("ADDRESS");
  address.setAttribute(QString("street"), i.street());
  address.setAttribute(QString("city"), i.city());
  address.setAttribute(QString("zip"), i.postcode());
  address.setAttribute(QString("state"), i.town());
  address.setAttribute(QString("telephone"), i.telephone());

  institution.appendChild(address);
}

void MyMoneyStorageXML::writeInstitutions(QDomDocument *pDoc, QDomElement& institutions, IMyMoneySerialize* storage)
{
  QValueList<MyMoneyInstitution> list;
  QValueList<MyMoneyInstitution>::ConstIterator it;

  //set the nextid attribute of the INSTITUTIONS element.
  institutions.setAttribute(QString("nextid"), list.count());
  
  list = storage->institutionList();
  for(it = list.begin(); it != list.end(); ++it)
  {
    QDomElement institution = pDoc->createElement("INSTITUTION");
    writeInstitution(pDoc, institution, *it);
    institutions.appendChild(institution);
  }
}

void MyMoneyStorageXML::writePayees(QDomDocument *pDoc, QDomElement& payees, IMyMoneySerialize* storage)
{
  QValueList<MyMoneyPayee> list;
  QValueList<MyMoneyPayee>::ConstIterator it;

  list = storage->payeeList();
  payees.setAttribute(QString("nextid"), list.count());
                      
  for(it = list.begin(); it != list.end(); ++it)
  {
    QDomElement payee = pDoc->createElement("PAYEE");
    writePayee(pDoc, payee, *it);
    payees.appendChild(payee);
  }
}

void MyMoneyStorageXML::writePayee(QDomDocument *pDoc, QDomElement& payee, const MyMoneyPayee& p)
{
  payee.setAttribute(QString("name"), p.name());
  payee.setAttribute(QString("id"), p.id());
  payee.setAttribute(QString("reference"), p.reference());

  QDomElement address = pDoc->createElement("ADDRESS");
  address.setAttribute(QString("street"), p.address());
  address.setAttribute(QString("zipcode"), p.postcode());
  address.setAttribute(QString("city"), p.city());
  address.setAttribute(QString("state"), p.state());
  address.setAttribute(QString("telephone"), p.telephone());

  payee.appendChild(address);
}

void MyMoneyStorageXML::writeAccounts(QDomDocument *pDoc, QDomElement& accounts, IMyMoneySerialize* storage)
{
  QValueList<MyMoneyAccount> list;
  QValueList<MyMoneyAccount>::ConstIterator it;

  list = storage->accountList();

  accounts.setAttribute(QString("nextid"), list.count() + 4);

  QDomElement asset, liability, expense, income;

  asset = pDoc->createElement("ACCOUNT");
  writeAccount(pDoc, asset, storage->asset());
  accounts.appendChild(asset);

  liability = pDoc->createElement("ACCOUNT");
  writeAccount(pDoc, liability, storage->liability());
  accounts.appendChild(liability);

  expense = pDoc->createElement("ACCOUNT");
  writeAccount(pDoc, expense, storage->expense());
  expense.appendChild(liability);

  income = pDoc->createElement("ACCOUNT");
  writeAccount(pDoc, income, storage->income());
  accounts.appendChild(income);

  //signalProgress(0, list.count(), QObject::tr("Saving accounts..."));
  int i = 0;
  for(it = list.begin(); it != list.end(); ++it, ++i)
  {
    QDomElement account = pDoc->createElement("ACCOUNT");
    writeAccount(pDoc, account, *it);
    accounts.appendChild(account);

    //signalProgress(i, 0);
  }
}

void MyMoneyStorageXML::writeAccount(QDomDocument *pDoc, QDomElement& account, const MyMoneyAccount& p)
{   
  account.setAttribute(QString("parentaccount"), p.parentAccountId());
  account.setAttribute(QString("lastreconciled"), p.lastReconciliationDate().toString());
  account.setAttribute(QString("lastmodified"), p.lastModified().toString());
  account.setAttribute(QString("institution"), p.institutionId());
  account.setAttribute(QString("opened"), p.openingDate().toString());
  account.setAttribute(QString("number"), p.number());
  account.setAttribute(QString("openingbalance"), p.openingBalance().formatMoney());
  account.setAttribute(QString("type"), p.accountType());  
  account.setAttribute(QString("id"), p.id());
  account.setAttribute(QString("name"), p.name());
  account.setAttribute(QString("description"), p.description());

  //Add in subaccount information, if this account has subaccounts.
  if(p.accountCount())
  {
    QDomElement subAccounts = pDoc->createElement("SUBACCOUNTS");
    QCStringList accountList = p.accountList();
    QCStringList::Iterator it;
    for(it = accountList.begin(); it != accountList.end(); ++it)
    {
      QDomElement temp = pDoc->createElement("SUBACCOUNT");
      temp.setAttribute(QString("id"), (*it));
      subAccounts.appendChild(temp);
    }

    account.appendChild(subAccounts);
  }
  
  writeKeyValuePairs(pDoc, account, p.pairs());
 
}

void MyMoneyStorageXML::writeKeyValuePairs(QDomDocument *pDoc, QDomElement& account, const QMap<QCString, QString> pairs)
{
  QMap<QCString, QString>::const_iterator it;
  QDomElement keyPair = pDoc->createElement("KEYVALPAIRS");
  for(it = pairs.begin(); it != pairs.end(); ++it)
  {
    QDomElement pair = pDoc->createElement("PAIR");
    pair.setAttribute(QString("Key"), it.key());
    pair.setAttribute(QString("Value"), it.data());
    keyPair.appendChild(pair);
  }

  account.appendChild(keyPair);
}

void MyMoneyStorageXML::writeTransactions(QDomDocument *pDoc, QDomElement& transactions, IMyMoneySerialize* storage)
{
  QValueList<MyMoneyTransaction> list;
  QValueList<MyMoneyTransaction>::ConstIterator it;
  MyMoneyTransactionFilter filter;

  list = storage->transactionList(filter);
  transactions.setAttribute(QString("nextid"), list.count());
  
  //signalProgress(0, list.count(), QObject::tr("Saving transactions..."));

  int i = 0;
  for(it = list.begin(); it != list.end(); ++it, ++i)
  {
    QDomElement tx = pDoc->createElement("TRANSACTION");
    writeTransaction(pDoc, tx, *it);
    transactions.appendChild(tx);
    //signalProgress(i, 0);
  }
}

void MyMoneyStorageXML::writeTransaction(QDomDocument *pDoc, QDomElement& transaction, const MyMoneyTransaction& tx)
{
  transaction.setAttribute(QString("postdate"), tx.postDate().toString());
  transaction.setAttribute(QString("memo"), tx.memo());
  transaction.setAttribute(QString("entrydate"), tx.entryDate().toString());

  QDomElement splits = pDoc->createElement("SPLITS");
  QValueList<MyMoneySplit> splitList = tx.splits();
  splits.setAttribute(QString("nextid"), tx.splitCount());

  writeSplits(pDoc, splits, splitList);

  transaction.appendChild(splits);
}

void MyMoneyStorageXML::writeSplits(QDomDocument *pDoc, QDomElement& splits, const QValueList<MyMoneySplit> splitList)
{
  QValueList<MyMoneySplit>::const_iterator it;
  for(it = splitList.begin(); it != splitList.end(); ++it)
  {
    QDomElement split = pDoc->createElement("SPLIT");
    writeSplit(pDoc, split, (*it));
    splits.appendChild(split);
  }
}

void MyMoneyStorageXML::writeSplit(QDomDocument *pDoc, QDomElement& splitElement, const MyMoneySplit& split)
{
  splitElement.setAttribute(QString("payee"), split.payeeId());
  splitElement.setAttribute(QString("reconciledate"), split.reconcileDate().toString());
  splitElement.setAttribute(QString("action"), split.action());
  splitElement.setAttribute(QString("reconcileflag"), split.reconcileFlag());
  splitElement.setAttribute(QString("value"), split.value().formatMoney());
  splitElement.setAttribute(QString("memo"), split.memo());
  splitElement.setAttribute(QString("id"), split.id());
  splitElement.setAttribute(QString("account"), split.accountId());
}

void MyMoneyStorageXML::writeSchedules(QDomDocument *pDoc, QDomElement& scheduled, IMyMoneySerialize* storage)
{
  QValueList<MyMoneySchedule> list;
  QValueList<MyMoneySchedule>::ConstIterator it;

  list = storage->scheduleList();
  scheduled.setAttribute(QString("nextid"), list.count());
  
  for(it = list.begin(); it != list.end(); ++it)
  {
    QDomElement schedTx = pDoc->createElement("SCHEDULED_TX");
    writeSchedule(pDoc, schedTx, *it);
    scheduled.appendChild(schedTx);
  }
}

void MyMoneyStorageXML::writeSchedule(QDomDocument *pDoc, QDomElement& scheduledTx, const MyMoneySchedule& tx)
{
  scheduledTx.setAttribute(QString("name"), tx.name());
  scheduledTx.setAttribute(QString("type"), tx.type());
  scheduledTx.setAttribute(QString("occurence"), tx.occurence());
  scheduledTx.setAttribute(QString("paymentType"), tx.paymentType());
  scheduledTx.setAttribute(QString("startDate"), tx.startDate().toString());
  scheduledTx.setAttribute(QString("endDate"), tx.endDate().toString());
  scheduledTx.setAttribute(QString("fixed"), tx.isFixed());
  scheduledTx.setAttribute(QString("autoEnter"), tx.autoEnter());
  scheduledTx.setAttribute(QString("id"), tx.id());
  scheduledTx.setAttribute(QString("lastPayment"), tx.lastPayment().toString());

  //store the payment history for this scheduled task.
  QValueList<QDate> payments = tx.recordedPayments();
  QValueList<QDate>::Iterator it;
  QDomElement paymentsElement = pDoc->createElement("PAYMENTS");
  paymentsElement.setAttribute(QString("count"), payments.count());
  for (it=payments.begin(); it!=payments.end(); ++it)
  {
    QDomElement paymentEntry = pDoc->createElement("PAYMENT");
    paymentEntry.setAttribute(QString("date"), (*it).toString());
    paymentsElement.appendChild(paymentEntry);
  }
  scheduledTx.appendChild(paymentsElement);

  //store the transaction data for this task.
  QDomElement transactionElement = pDoc->createElement("TRANSACTION");
  writeTransaction(pDoc, transactionElement, tx.transaction());
  scheduledTx.appendChild(transactionElement);
}

/*void MyMoneyStorageXML::getTransactionDetails(const AttributeMap& p)
{
  if(m_pCurrentTx)
  {
    std::string strTemp;
   // strTemp = getPropertyValue(std::string("id"), p);
   // m_pCurrentTx->setId(QString(strTemp.data()));

    strTemp = getPropertyValue(std::string("entrydate"), p);
    QDate entryDate = QDate::fromString(QString(strTemp.data()), Qt::ISODate);
    m_pCurrentTx->setEntryDate(entryDate);

    strTemp = getPropertyValue(std::string("postdate"), p);
    QDate postDate = QDate::fromString(QString(strTemp.data()), Qt::ISODate);
    m_pCurrentTx->setPostDate(postDate);
  }
}

void MyMoneyStorageXML::getPayeeDetails(MyMoneyPayee* pCurrentPayee, const AttributeMap& p)
{
  std::string strTemp;
  strTemp = getPropertyValue(std::string("name"), p);
  pCurrentPayee->setName(QString(strTemp.data()));

  strTemp = getPropertyValue(std::string("id"), p);
  pCurrentPayee->setId(QCString(strTemp.data()));

  strTemp = getPropertyValue(std::string("reference"), p);
  pCurrentPayee->setReference(QString(strTemp.data()));
}

void MyMoneyStorageXML::getAccountDetails(MyMoneyAccount* pCurrentAccount, const AttributeMap& p)
{
  std::string strTemp;
  strTemp = getPropertyValue(std::string("id"), p);
  pCurrentAccount->setAccountId(QCString(strTemp.data()));

  // The type of account specified must match up with one of the types, or this file should be treated as invalid.
  MyMoneyAccount::accountTypeE acctype;
  strTemp = getPropertyValue(std::string("type"), p);
  acctype = KMyMoneyUtils::stringToAccountType(QString(strTemp.data()));
  pCurrentAccount->setAccountType(acctype);

  strTemp = getPropertyValue(std::string("name"), p);
  pCurrentAccount->setName(QString(strTemp.data()));

  strTemp = getPropertyValue(std::string("description"), p);
  pCurrentAccount->setDescription(QString(strTemp.data()));

  strTemp = getPropertyValue(std::string("institution"), p);
  pCurrentAccount->setInstitutionId(QCString(strTemp.data()));

  strTemp = getPropertyValue(std::string("number"), p);
  pCurrentAccount->setNumber(QString(strTemp.data()));

  strTemp = getPropertyValue(std::string("opened"), p);
  QDate openingDate = QDate::fromString(QString(strTemp.data()), Qt::ISODate);
  pCurrentAccount->setOpeningDate(openingDate);

  strTemp = getPropertyValue(std::string("openingbalance"), p);
  MyMoneyMoney openBalance(QString(strTemp.data()));
  pCurrentAccount->setOpeningBalance(openBalance);

  strTemp = getPropertyValue(std::string("lastmodified"), p);
  QDate lastModified = QDate::fromString(QString(strTemp.data()), Qt::ISODate);
  pCurrentAccount->setLastModified(lastModified);

  strTemp = getPropertyValue(std::string("lastreconciled"), p);
  QDate lastReconciled = QDate::fromString(QString(strTemp.data()), Qt::ISODate);
  pCurrentAccount->setLastReconciliationDate(lastReconciled);
}

void MyMoneyStorageXML::getInstitutionDetails(MyMoneyInstitution* pInstitution, const AttributeMap& p)
{
  std::string strTemp;
  strTemp = getPropertyValue(std::string("id"), p);
  m_pCurrentInstitution->setId(QString(strTemp.data()));

  strTemp = getPropertyValue(std::string("name"), p);
  m_pCurrentInstitution->setName(QString(strTemp.data()));

  strTemp = getPropertyValue(std::string("manager"), p);
  m_pCurrentInstitution->setManager(QString(strTemp.data()));

  strTemp = getPropertyValue(std::string("sortcode"), p);
  m_pCurrentInstitution->setSortcode(QString(strTemp.data()));
}
      */

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

