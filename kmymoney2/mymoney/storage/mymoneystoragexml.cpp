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

using namespace xmlpp;

MyMoneyStorageXML::MyMoneyStorageXML()
{
  m_pStorage            = NULL;
  m_pCurrentInstitution = NULL;
  m_pCurrentPayee       = NULL;
  m_pCurrentAccount     = NULL;
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

void MyMoneyStorageXML::on_start_element(const std::string &n, const Element::AttributeMap& p)
{
  qDebug("XMLREADER:  start_element called, %s", n.data());

  std::string strTemp;

  if(!n.find("NEXTIDS"))
  {

  }
  else if(!n.find("USER"))
  {
    strTemp = getPropertyValue(std::string("name"), p);

    if(m_pStorage)
    {
      m_pStorage->setUserName(QString(strTemp.data()));
    }
      
    ChangeParseState(PARSE_USERINFO);
  }
  else if(!n.find("ADDRESS"))
  {
    if(getCurrentParseState() == PARSE_USERINFO)
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
    else if(getCurrentParseState() == PARSE_INSTITUTION)
    {
      qDebug("XMLREADER: Parsing address for Institution info");
    
      if(m_pCurrentInstitution)
      {
        strTemp = getPropertyValue(std::string("street"), p);
        m_pCurrentInstitution->setStreet(QString(strTemp.data()));

        strTemp = getPropertyValue(std::string("city"), p);
        m_pCurrentInstitution->setCity(QString(strTemp.data()));

        strTemp = getPropertyValue(std::string("state"), p);
        m_pCurrentInstitution->setTown(QString(strTemp.data()));

        strTemp = getPropertyValue(std::string("zip"), p);
        m_pCurrentInstitution->setPostcode(QString(strTemp.data()));
      }
    }
    else if(getCurrentParseState() == PARSE_PAYEE)
    {
      qDebug("XMLREADER: Parsing address for Payee info");

      if(m_pCurrentPayee)
      {
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
        strTemp = getPropertyValue(std::string("id"), p);
        m_pCurrentInstitution->setId(QString(strTemp.data()));
        
        strTemp = getPropertyValue(std::string("name"), p);
        m_pCurrentInstitution->setName(QString(strTemp.data()));

        strTemp = getPropertyValue(std::string("manager"), p);
        m_pCurrentInstitution->setManager(QString(strTemp.data()));

        strTemp = getPropertyValue(std::string("sortcode"), p);
        m_pCurrentInstitution->setSortcode(QString(strTemp.data()));

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
        
        strTemp = getPropertyValue(std::string("name"), p);
        m_pCurrentPayee->setName(QString(strTemp.data()));

        strTemp = getPropertyValue(std::string("id"), p);
        m_pCurrentPayee->setId(QCString(strTemp.data()));

        strTemp = getPropertyValue(std::string("ref"), p);
        m_pCurrentPayee->setReference(QString(strTemp.data()));

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
        strTemp = getPropertyValue(std::string("id"), p);
        m_pCurrentAccount->setAccountId(QCString(strTemp.data()));
        
        // The type of account specified must match up with one of the types, or this file should be treated as invalid.
        MyMoneyAccount::accountTypeE acctype;
        strTemp = getPropertyValue(std::string("type"), p);
        acctype = KMyMoneyUtils::stringToAccountType(QString(strTemp.data()));
        m_pCurrentAccount->setAccountType(acctype);
        
        strTemp = getPropertyValue(std::string("name"), p);
        m_pCurrentAccount->setName(QString(strTemp.data()));

        strTemp = getPropertyValue(std::string("description"), p);
        m_pCurrentAccount->setDescription(QString(strTemp.data()));

        strTemp = getPropertyValue(std::string("institution"), p);
        m_pCurrentAccount->setInstitutionId(QCString(strTemp.data()));

        strTemp = getPropertyValue(std::string("number"), p);
        m_pCurrentAccount->setNumber(QString(strTemp.data()));

        strTemp = getPropertyValue(std::string("opened"), p);
        QDate openingDate = QDate::fromString(QString(strTemp.data()), Qt::ISODate);
        m_pCurrentAccount->setOpeningDate(openingDate);

        strTemp = getPropertyValue(std::string("openingbalance"), p);
        MyMoneyMoney openBalance(QString(strTemp.data()));
        m_pCurrentAccount->setOpeningBalance(openBalance);

        strTemp = getPropertyValue(std::string("lastmodified"), p);
        QDate lastModified = QDate::fromString(QString(strTemp.data()), Qt::ISODate);
        m_pCurrentAccount->setLastModified(lastModified);

        strTemp = getPropertyValue(std::string("lastreconciled"), p);
        QDate lastReconciled = QDate::fromString(QString(strTemp.data()), Qt::ISODate);
        m_pCurrentAccount->setLastReconciliationDate(lastReconciled);
      }
    }
  }
}

std::string MyMoneyStorageXML::getPropertyValue(std::string str, const Element::AttributeMap& p)
{
  //for(XMLPropertyMap::const_iterator i = p.begin(); i != p.end(); ++i)
  //{
  //  qDebug("XMLPropertyMap str=%s, first=%s",str.data(), ((*i).first).data());
  //}
  Element::AttributeMap::const_iterator i = p.find(str.data());
  if(i != p.end())
  {
    const Attribute* pProperty = (*i).second;
    return pProperty->get_value();
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
  m_parseState = m_previousParseState;
}

void MyMoneyStorageXML::on_characters(const std::string &s)
{
  //qDebug("XMLREADER:  Character data = %s", s.data());
  //qDebug("   length = %d", s.size());

  /*if(m_pStorage)
  {
    const QString strData(s.data());

    if(getCurrentParseState() == PARSE_USERINFO_ADDRESS)
    {
      if(m_addressParseState == ADDRESS_STREET)
      {
        m_pStorage->setUserStreet(strData);
      }
      else if(m_addressParseState == ADDRESS_CITY)
      {
        m_pStorage->setUserTown(strData);
      }
      else if(m_addressParseState == ADDRESS_STATE)
      {
        m_pStorage->setUserCounty(strData);
      }
      else if(m_addressParseState == ADDRESS_ZIPCODE)
      {
        m_pStorage->setUserPostcode(strData);
      }
      else if(m_addressParseState == ADDRESS_TELEPHONE)
      {
        m_pStorage->setUserTelephone(strData);
      }
    }
  } */
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

void MyMoneyStorageXML::parseNextIDS(const std::string &n, const Element::AttributeMap& p)
{
  std::string strValue;
  
  if(!n.find("ACCOUNTID"))
  {
    strValue = getPropertyValue(std::string("value"), p);
  }
  else if(!n.find("INSTITUTIONID"))
  {

  }
	else if(!n.find("TRANSACTIONID"))
  {

  }
	else if(!n.find("PAYEEID"))
  {

  }
  else
  {

  }
}


#endif // HAVE_LIBXMLPP
#endif
