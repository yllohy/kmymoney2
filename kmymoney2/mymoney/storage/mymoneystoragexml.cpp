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

#if HAVE_LIBXMLPP

// ----------------------------------------------------------------------------
// QT Includes

#include <qfile.h>

// ----------------------------------------------------------------------------
// Third party Includes

#include <xml++.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneystoragexmlcallback.h"
#include "mymoneystoragexml.h"

MyMoneyStorageXML::MyMoneyStorageXML()
{
  m_parser = NULL;
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

  if(pDevice && storage)
  {
    Q_LONG totalSize = 0;
    char buf[1000];
    Q_LONG readSize = 0;
    while(readSize >= 0)
    {
      readSize = pDevice->readBlock((char*)&buf, 1000);
      totalSize += readSize;
      if(readSize > 0)
      {
        qDebug("XMLREADER: chars read");
        std::string parseString(buf);
        m_parser->parse_chunk(parseString);
      }
      else
      {
        m_parser->finish();
        break;
      }
    }

    if(m_parser)
    {
      delete m_parser;
      m_parser = NULL;
    }

    //qDebug("XMLREADER: %n total file size", totalSize);
    
  }
}

void MyMoneyStorageXML::writeFile(QIODevice* qf, IMyMoneySerialize* storage)
{
  qDebug("XMLWRITER: not implemented yet!");
}

bool MyMoneyStorageXML::CreateXMLParser()
{
  if(m_parser)
  {
    qDebug("XMLREADER:  XML++ parser already created");
    return true;
  }
  else
  {
    m_parser = new xmlpp::XMLParser<MyMoneyStorageXMLCallback>;
    if(m_parser)
    {
      qDebug("XMLREADER:  Able to create the XML++ Parser");
      return true;
    }
  }

  qDebug("XMLREADER:  Failed to create XML++ Parser");
  return false;
}

#endif // HAVE_LIBXMLPP

