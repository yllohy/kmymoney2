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

#ifdef _COMPILE_XML
#if HAVE_LIBXMLPP

// ----------------------------------------------------------------------------
// QT Includes

#include <qfile.h>

// ----------------------------------------------------------------------------
// Third party Includes

#include <libxml++-1.0/libxml++/libxml++.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneystoragexml.h"
#include "../../kmymoneyutils.h"

using namespace xmlpp;

MyMoneyStorageXML::MyMoneyStorageXML()// : xmlpp::SaxParser(false)
{
  m_pStorage            = NULL;
  m_pCurrentInstitution = NULL;
  m_pCurrentPayee       = NULL;
  m_pCurrentAccount     = NULL;
  m_pCurrentTx          = NULL;
  m_pCurrentSplit       = NULL;
}

MyMoneyStorageXML::~MyMoneyStorageXML()
{
  
}

//Function to read in the file, send to XML parser.
void MyMoneyStorageXML::readFile(QIODevice* pDevice, IMyMoneySerialize* storage)
{
  if(pDevice && storage)
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
  }
}

void MyMoneyStorageXML::writeFile(QIODevice* qf, IMyMoneySerialize* storage)
{
  QTextStream stream(qf);
  qDebug("XMLWRITER: not implemented yet!");
}

void MyMoneyStorageXML::on_start_document(void)
{
  qDebug("XMLREADER:  start_document() called");
  ChangeParseState(PARSE_NEXTIDS);
}

void MyMoneyStorageXML::on_end_document(void)
{
  qDebug("XMLREADER:  end_document() called");
}

void MyMoneyStorageXML::on_start_element(const std::string &n, const AttributeMap& p)
{
  qDebug("XMLREADER:  start_element called, %s", n.data());

  std::string strTemp;

  if(!n.find("USER"))
  {
    strTemp = getPropertyValue(std::string("name"), p);
    qDebug("XMLREADER:  user name is %s", strTemp.data());

    if(m_pStorage)
    {
      m_pStorage->setUserName(QString(strTemp.data()));
    }
      
    ChangeParseState(PARSE_USERINFO);
  }
  else if(!n.find("ADDRESS"))
  {
    getAddress(p);
  }
  else if(!n.find("INSTITUTIONS"))
  {
    ChangeParseState(PARSE_INSTITUTIONS);
  }
  else if(!n.find("INSTITUTION"))
  {
    qDebug("XMLREADER: Parsing an institution");

    if(getCurrentParseState() == PARSE_INSTITUTIONS)
    {
      ChangeParseState(PARSE_INSTITUTION);
      
      if(!m_pCurrentInstitution)
      {
        qDebug("XMLREADER: creating new institution object to use.");
        m_pCurrentInstitution = new MyMoneyInstitution;
      }

      if(m_pCurrentInstitution)
      {
        getInstitutionDetails(m_pCurrentInstitution, p);
        qDebug("XMLREADER: Main information for institution object in place");
      }
    }
  }
  else if(!n.find("PAYEES"))
  {
    ChangeParseState(PARSE_PAYEES);
  }
  else if(!n.find("PAYEE"))
  {
    if(getCurrentParseState() == PARSE_PAYEES)
    {
      qDebug("XMLREADER: Parsing a new Payee entry");
      
      ChangeParseState(PARSE_PAYEE);

      if(!m_pCurrentPayee)
      {
        m_pCurrentPayee = new MyMoneyPayee;
      }

      if(m_pCurrentPayee)
      {
        qDebug("XMLREADER: Filling out information for the new payee");
        getPayeeDetails(m_pCurrentPayee, p);
        qDebug("XMLREADER: Done filling out information for the new payee");
      }
    }
  }
  else if(!n.find("ACCOUNTS"))
  {
    qDebug("XMLREADER: Parsing Accounts");
    ChangeParseState(PARSE_ACCOUNTS);
  }
  else if(!n.find("ACCOUNT"))
  {
    qDebug("XMLREADER: Parsing an Account");
    if(getCurrentParseState() == PARSE_ACCOUNTS)
    {
      ChangeParseState(PARSE_ACCOUNT);

      if(!m_pCurrentAccount)
      {
        qDebug("XMLREADER: Creating new account object to fill in.");
        m_pCurrentAccount = new MyMoneyAccount;
      }

      if(m_pCurrentAccount)
      {
        getAccountDetails(m_pCurrentAccount, p);
      }
    }
  }
  else if(!n.find("TRANSACTIONS"))
  {
    qDebug("XMLREADER: Parsing Transactions for current account");
    ChangeParseState(PARSE_TRANSACTIONS);
  }
  else if(!n.find("TRANSACTION") && getCurrentParseState() == PARSE_TRANSACTIONS)
  {
    m_pCurrentTx = new MyMoneyTransaction();
    getTransactionDetails(p);
  }
  else if(!n.find("SPLITS"))
  {

  }
  else if(!n.find("SPLIT"))
  {

  }
  else
  {
    qDebug("XMLREADER: Unsupported XML tag found, %s", n.data());
  }
}

std::string MyMoneyStorageXML::getPropertyValue(std::string str, const AttributeMap& p)
{
  AttributeMap::const_iterator i = p.find(str.data());
  if(i != p.end())
  {
    return (*i).second;
  }

  return std::string("");
}

void MyMoneyStorageXML::on_end_element(const std::string &n)
{
  qDebug("XMLREADER:  on_end_element called, %s", n.data());
  
  if(!n.find("INSTITUTION"))
  {
    if(m_pCurrentInstitution)
    {
      qDebug("XMLREADER:  Adding institution to the list of institutions, name = %s", m_pCurrentInstitution->name().data());
      m_pStorage->addInstitution(*m_pCurrentInstitution);
      delete m_pCurrentInstitution;
      m_pCurrentInstitution = NULL;
    }
  }
  else if(!n.find("PAYEE"))
  {
    if(m_pCurrentPayee)
    {
      qDebug("XMLREADER:  Adding Payee to the list of Payees, name = %s", m_pCurrentPayee->name().data());
      m_pStorage->addPayee(*m_pCurrentPayee);
      delete m_pCurrentPayee;
      m_pCurrentPayee = NULL;
    }
  }
  else if(!n.find("ACCOUNT"))
  {
    if(m_pCurrentAccount)
    {
      qDebug("XMLREADER:  Adding account to the list of Accoutns, name=%s", m_pCurrentAccount->name().data());
      m_pStorage->newAccount(*m_pCurrentAccount);
      delete m_pCurrentAccount;
      m_pCurrentAccount = NULL;
    }
  }
  else if(!n.find("TRANSACTION"))
  {
    if(m_pCurrentTx)
    {
      qDebug("XMLREADER:  Adding Transaction to the list of Transactions");
      m_pStorage->addTransaction(*m_pCurrentTx);
      delete m_pCurrentTx;
      m_pCurrentTx = NULL;
    }
  }
  
  m_parseState = m_previousParseState;
}

void MyMoneyStorageXML::on_characters(const std::string &s)
{
  //qDebug("XMLREADER:  Character data = %s", s.data());
}

void MyMoneyStorageXML::on_comment(const std::string &s)
{

}

void MyMoneyStorageXML::on_warning(const std::string &s)
{
}

void MyMoneyStorageXML::on_error(const std::string &s)
{

}

void MyMoneyStorageXML::on_fatal_error(const std::string &s)
{
}

void MyMoneyStorageXML::ChangeParseState(eParseState state)
{
  m_previousParseState = m_parseState;
  m_parseState = state;
}

void MyMoneyStorageXML::getTransactionDetails(const AttributeMap& p)
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

void MyMoneyStorageXML::getAddress(const AttributeMap& p)
{
  std::string strTemp;
  if(getCurrentParseState() == PARSE_USERINFO && m_pStorage)
  {
    qDebug("XMLREADER: Parsing address for User info");

    strTemp = getPropertyValue(std::string("street"), p);
    m_pStorage->setUserStreet(QString(strTemp.data()));

    strTemp = getPropertyValue(std::string("city"), p);
    m_pStorage->setUserTown(QString(strTemp.data()));

    strTemp = getPropertyValue(std::string("state"), p);
    m_pStorage->setUserCounty(QString(strTemp.data()));

    strTemp = getPropertyValue(std::string("zip"), p);
    m_pStorage->setUserPostcode(QString(strTemp.data()));
  }
  else if(getCurrentParseState() == PARSE_INSTITUTION && m_pCurrentInstitution)
  {
    qDebug("XMLREADER: Parsing address for Institution info");

    strTemp = getPropertyValue(std::string("street"), p);
    m_pCurrentInstitution->setStreet(QString(strTemp.data()));

    strTemp = getPropertyValue(std::string("city"), p);
    m_pCurrentInstitution->setCity(QString(strTemp.data()));

    strTemp = getPropertyValue(std::string("state"), p);
    m_pCurrentInstitution->setTown(QString(strTemp.data()));

    strTemp = getPropertyValue(std::string("zip"), p);
    m_pCurrentInstitution->setPostcode(QString(strTemp.data()));
  }
  else if(getCurrentParseState() == PARSE_PAYEE && m_pCurrentPayee)
  {
    qDebug("XMLREADER: Parsing address for Payee info");

    strTemp = getPropertyValue(std::string("street"), p);
    m_pCurrentPayee->setAddress(QString(strTemp.data()));

    strTemp = getPropertyValue(std::string("city"), p);
    m_pCurrentPayee->setCity(QString(strTemp.data()));

    strTemp = getPropertyValue(std::string("state"), p);
    m_pCurrentPayee->setState(QString(strTemp.data()));

    strTemp = getPropertyValue(std::string("zip"), p);
    m_pCurrentPayee->setPostcode(QString(strTemp.data()));
  }
}

void MyMoneyStorageXML::writeInstitution(QTextStream&s, const MyMoneyInstitution& i)
{
  /*Q_INT32 tmp;
  tmp = 1;    // version
  s << tmp;

  s << i.id();
  s << i.name();
  s << i.city();
  s << i.street();
  s << i.postcode();
  s << i.telephone();
  s << i.manager();
  s << i.sortcode();
  s << i.accountList();   */
}

void MyMoneyStorageXML::writeInstitutions(QTextStream& s, IMyMoneySerialize* storage)
{
  Q_INT32 tmp;
  QValueList<MyMoneyInstitution> list;
  QValueList<MyMoneyInstitution>::ConstIterator it;

  //tmp = 1;      // version
  //s << tmp;

  /*list = storage->institutionList();
  s << list.count();
  for(it = list.begin(); it != list.end(); ++it) {
    writeInstitution(s, *it);
  }     */
}

void writeFileBeginning(QTextStream& s)
{
  
}

void writePayees(QTextStream& s, IMyMoneySerialize* storage)
{

}

void writePayee(QTextStream& s, const MyMoneyPayee& p)
{

}

void writeAccounts(QTextStream& s, IMyMoneySerialize* storage)
{

}

void writeAccount(QTextStream& s, const MyMoneyAccount& p)
{

}

void writeTransactions(QTextStream& s, IMyMoneySerialize* storage)
{
              
}

void writeTransaction(QTextStream& s, const MyMoneyTransaction& tx)
{

}

void writeSplits(QTextStream& s, const MyMoneyTransaction& tx)
{                 

}

void writeSplit(QTextStream& s, const MyMoneySplit& split)
{

}

#endif // HAVE_LIBXMLPP
#endif
