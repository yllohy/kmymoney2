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
  QDomDocument *pDoc = new QDomDocument;
  if(pDoc->setContent(pDevice, FALSE))
  {
/*    readUserInformation(pDoc, storage);
    readInstitutions(pDoc, storage);
    readPayees(pDoc, storage);
    readAccounts(pDoc, storage);
    readTransactions(pDoc, storage);
    readSchedules(pDoc, storage);     */
  }
  else
  {
    throw new MYMONEYEXCEPTION("File was not parsable!");
  }
  /*if(pDevice && storage)
  {
    m_pStorage = storage;

    //reads the contents of the entire file into this buffer.
    QTextStream stream(pDevice);
    QString strEntireFile = stream.read();
    qDebug("XMLREADER: entire file is %s\n", strEntireFile.data());
    try
    {
      parse_memory(strEntireFile);
    }
    catch(xmlpp::parse_error* e)
    {
      qDebug("XMLREADER: EXCEPTION while parsing buffer");
    }

    //don't use this pointer after the function has exited...
    m_pStorage = NULL;

    //qDebug("XMLREADER: %ld total file size", totalSize);
  } */
  
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
  //writePayees(pDoc, payees, storage);
  mainElement.appendChild(payees);

  QDomElement accounts = pDoc->createElement("ACCOUNTS");
  writeAccounts(pDoc, accounts, storage);
  mainElement.appendChild(accounts);

  QDomElement transactions = pDoc->createElement("TRANSACTIONS");
  //writeTransactions(pDoc, transactions, storage);
  mainElement.appendChild(transactions);

  QDomElement schedules = pDoc->createElement("SCHEDULES");
  //writeSchedules(pDoc, schedules, storage);
  mainElement.appendChild(schedules);

  QDataStream stream(qf);
  //stream.setEncoding(QTextStream::Locale);
  QString temp = pDoc->toString();
  stream << temp.data();

  qDebug("File contains %s", temp.data());
  
  delete pDoc;
  pDoc = NULL;
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
  Q_INT32 tmp;
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

}

void MyMoneyStorageXML::writePayee(QDomDocument *pDoc, QDomElement& payee, const MyMoneyPayee& p)
{

}

void MyMoneyStorageXML::writeAccounts(QDomDocument *pDoc, QDomElement& accounts, IMyMoneySerialize* storage)
{
  Q_INT32 tmp;
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

}

void MyMoneyStorageXML::writeTransaction(QDomDocument *pDoc, QDomElement& transaction, const MyMoneyTransaction& tx)
{

}

void MyMoneyStorageXML::writeSplits(QDomDocument *pDoc, QDomElement& splits, IMyMoneySerialize* storage)
{

}

void MyMoneyStorageXML::writeSplit(QDomDocument *pDoc, QDomElement& splitElement, const MyMoneySplit& split)
{

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

