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

#include <xml++.h>
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
void MyMoneyStorageXML::readStream(QDataStream& s, IMyMoneySerialize* storage)
{
  if(!CreateXMLParser())
  {
    return;
  }

  //
  //  QDataStream is a wrapper around a real file object.  QDataStream provides a nice interface,
  //  but we need raw access to the file, to read it at 1000 bytes at a time.  We need to send the
  //  XML file to the SAX interface in chunks not too big to waste memory, but not too small to
  //  still provide good speed.  The ideal number of bytes read should be determined by profiling.
  //
  QIODevice *pDevice = s.device();
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

void MyMoneyStorageXML::addCategory(IMyMoneySerialize* storage,QMap<QString, QCString>& categories,
                                    const QString& majorName, const QString& minorName,
                                    const MyMoneyAccount::accountTypeE type)
{

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

