/***************************************************************************
                          mymoneystoragexmlcollection.cpp  -  description
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

// ----------------------------------------------------------------------------
// QT Includes
#include <qglobal.h>

// ----------------------------------------------------------------------------
// Project Includes
#include "config.h" 
#include "mymoneystoragexmlcallback.h"

#if HAVE_LIBXMLPP

MyMoneyStorageXMLCallback::MyMoneyStorageXMLCallback()
{
  m_parseState          = PARSE_STATE_UNKNOWN;
  m_previousParseState  = PARSE_STATE_UNKNOWN;
}

MyMoneyStorageXMLCallback::~MyMoneyStorageXMLCallback()
{
}

void MyMoneyStorageXMLCallback::start_document(void)
{
  qDebug("XMLREADER:  start_document() called");
}

void MyMoneyStorageXMLCallback::end_document(void)
{
  qDebug("XMLREADER:  end_document() called");
}

void MyMoneyStorageXMLCallback::start_element(const std::string &n, const XMLPropertyMap &p)
{
  if(m_parseState == PARSE_STATE_UNKNOWN)
  {
//    cout << "Changing state, element name = " << n.data() << endl;
//    cout << "length = " << n.size() << endl;
    if(!n.find("CURRENCY"))
    {
//      m_parseState = PARSE_ACCOUNTS;
    }
  }

  if(m_parseState != PARSE_STATE_UNKNOWN)
  {
    for(XMLPropertyMap::const_iterator i = p.begin(); i != p.end(); ++i)
    {

    }
  }
}

void MyMoneyStorageXMLCallback::end_element(const std::string &n)
{
  m_parseState = m_previousParseState;
}

void MyMoneyStorageXMLCallback::characters(const std::string &s)
{
  //cout << "Character data = " << s.data() << endl;
  //cout << "length = " << s.size() << endl;
}

void MyMoneyStorageXMLCallback::comment(const std::string &s)
{

}

void MyMoneyStorageXMLCallback::warning(const std::string &s)
{
}

void MyMoneyStorageXMLCallback::error(const std::string &s)
{

}

void MyMoneyStorageXMLCallback::fatal_error(const std::string &s)
{
}

void MyMoneyStorageXMLCallback::ChangeParseState(eParseState state)
{
  m_previousParseState = m_parseState;
  m_parseState = state;
}

#endif // HAVE_LIBXMLPP
