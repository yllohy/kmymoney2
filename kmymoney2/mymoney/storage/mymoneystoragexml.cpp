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
  xmlpp::XMLParser<MyMoneyStorageXMLCallback> *parser = new xmlpp::XMLParser<MyMoneyStorageXMLCallback>;
  if(parser)
  {
    //cout << "Able to create XML++ parser" << endl;
  }
}

MyMoneyStorageXML::~MyMoneyStorageXML()
{
  
}

void MyMoneyStorageXML::readStream(QDataStream& s, IMyMoneySerialize* storage)
{
  if(storage)
  {

  }
}

void MyMoneyStorageXML::readOldFormat(QDataStream& s, IMyMoneySerialize* storage)
{
  readStream(s, storage);
}

void MyMoneyStorageXML::readNewFormat(QDataStream& s, IMyMoneySerialize* storage)
{
  readStream(s, storage);
}

void MyMoneyStorageXML::addCategory(IMyMoneySerialize* storage,QMap<QString, QCString>& categories,
                                    const QString& majorName, const QString& minorName,
                                    const MyMoneyAccount::accountTypeE type)
{

}

#endif // HAVE_LIBXMLPP
