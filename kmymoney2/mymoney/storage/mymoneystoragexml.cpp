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

#include "mymoneyxmlparser.h"
//#include "mymoneystoragexmlcallback.h"
#include "mymoneystoragexml.h"

using namespace xmlpp;

MyMoneyStorageXML::MyMoneyStorageXML()
{
  //m_parser = NULL;
  m_pStorage = NULL;
}

MyMoneyStorageXML::~MyMoneyStorageXML()
{
  
}

//Function to read in the file, send to XML parser.
void MyMoneyStorageXML::readFile(QIODevice* pDevice, IMyMoneySerialize* storage)
{
  if(!CreateXMLParser())
  {
    return;
  }
         
  //
  //  Need to use the file object and read it at 1000 bytes at a time.  We need to send the
  //  XML file to the SAX interface in chunks not too big to waste memory, but not too small to
  //  still provide good speed.  The ideal number of bytes read should be determined by profiling.
  //
  if(pDevice && storage)
  {
    m_pStorage = storage;
    Q_LONG totalSize = 0;
    char buf[1000];
    Q_LONG readSize = 0;
    while(readSize >= 0)
    {
      readSize = pDevice->readBlock((char*)&buf, 1000);
      totalSize += readSize;
      if(readSize > 0)
      {
        qDebug("XMLREADER: %ld chars read", readSize);
        std::string parseString(buf);
        try
        {
          parse_memory(parseString);
        }
        catch(xmlpp::parse_error* e)
        {
          qDebug("XMLREADER: EXCEPTION while parsing buffer");
        }
        //m_parser->parse_chunk(parseString);
      }
      else
      {
        //m_parser->finish();
        break;
      }
    }

    /*if(m_parser)
    {
      delete m_parser;
      m_parser = NULL;
    }*/

    //don't use this pointer after the function has exited...
    m_pStorage = NULL;

    qDebug("XMLREADER: %ld total file size", totalSize);
    
  }
}

void MyMoneyStorageXML::writeFile(QIODevice* qf, IMyMoneySerialize* storage)
{
  qDebug("XMLWRITER: not implemented yet!");
}

bool MyMoneyStorageXML::CreateXMLParser()
{
  return true;
  /*if(m_parser)
  {
    qDebug("XMLREADER:  XML++ parser already created");
    return true;
  }
  else
  {
    m_parser = new MyMoneyXMLParser;//xmlpp::XMLParser<MyMoneyStorageXMLCallback>;
    if(m_parser)
    {
      //m_callback = new MyMoneyStorageXMLCallback(this);
      m_parser->setParserCallback(this);
      
      qDebug("XMLREADER:  Able to create the XML++ Parser");
      return true;
    }
  }

  qDebug("XMLREADER:  Failed to create XML++ Parser");   */
  //return false;
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

  

  if(!n.find("NEXTIDS"))
  {

  }
  if(!n.find("USER"))
  {
    ChangeParseState(PARSE_USERINFO);
    if(m_pStorage)
    {
      std::string strUserName = getPropertyValue(std::string("name"), p);
      m_pStorage->setUserName(QString(strUserName.data()));
    }
  }

  if(getCurrentParseState() == PARSE_NEXTIDS)
  {
      parseNextIDS(n, p);
  }
  else if(getCurrentParseState() == PARSE_USERINFO)
  {

  }
  
  /*if(m_parseState == PARSE_USERINFO)
  {
    if(!n.find("ADDRESS"))
    {
      ChangeParseState(PARSE_USERINFO_ADDRESS);
    }
  }
  else if(m_parseState == PARSE_USERINFO_ADDRESS)
  {
    if(!n.find("STREET"))
    {
      ChangeParseState(PARSE_USERINFO_ADDRESS_STREET);
    }
    else if(!n.find("CITY"))
    {
      ChangeParseState(PARSE_USERINFO_ADDRESS_CITY);
    }
    else if(!n.find("STATE"))
    {
      ChangeParseState(PARSE_USERINFO_ADDRESS_STATE);
    }
    else if(!n.find("ZIPCODE"))
    {
      ChangeParseState(PARSE_USERINFO_ADDRESS_ZIPCODE);
    }
    else if(!n.find("COUNTY"))
    {
      ChangeParseState(PARSE_USERINFO_ADDRESS_COUNTY);
    }
    else if(!n.find("COUNTRY"))
    {
      ChangeParseState(PARSE_USERINFO_ADDRESS_COUNTRY);
    }
    else if(!n.find("TELEPHONE"))
    {
      ChangeParseState(PARSE_USERINFO_ADDRESS_TELEPHONE);
    }
  }


  if(m_parseState == PARSE_USERINFO)
  {
    qDebug("Changing state, element name = %s", n.data());
    qDebug("length = %d", n.size());
    //if(!n.find("CURRENCY"))
    //{
//      m_parseState = PARSE_ACCOUNTS;
    //}
  }

  if(m_parseState != PARSE_STATE_UNKNOWN)
  {
    //for(XMLPropertyMap::const_iterator i = p.begin(); i != p.end(); ++i)
    //{

    //}
  }    */
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
  m_parseState = m_previousParseState;
}

void MyMoneyStorageXML::on_characters(const std::string &s)
{
  qDebug("XMLREADER:  Character data = %s", s.data());
  qDebug("   length = %d", s.size());

  if(m_pStorage)
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
  }
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
