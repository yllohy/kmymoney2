/***************************************************************************
                          mymoneyonlinepriceupdate.cpp
                          -------------------
    copyright            : (C) 2004 by Kevin Tambascio, 2004 by Thomas Baumgart
    email                : ktambascio@users.sourceforge.net, 
                           ipwizard@users.sourceforge.net
    
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
#include <qdatetime.h>
#include <qmap.h>
#include <qstringlist.h>

// ----------------------------------------------------------------------------
// Project Includes
#include "mymoneyonlinepriceupdate.h"

MyMoneyOnlinePriceUpdate::MyMoneyOnlinePriceUpdate()
{
}


MyMoneyOnlinePriceUpdate::~MyMoneyOnlinePriceUpdate()
{
}

void MyMoneyOnlinePriceUpdate::setPerlLocation(const QString& strLoc)
{
  m_strPerlLocation = strLoc;
}

int MyMoneyOnlinePriceUpdate::getQuotes(const QStringList& symbolNames)
{
  return 0;
}

int MyMoneyOnlinePriceUpdate::applyNewQuotes()
{
  return 0;
}

int MyMoneyOnlinePriceUpdate::getLastUpdateDate(const QString& symbolName, QDate& date)
{
  return 0;
}

int MyMoneyOnlinePriceUpdate::getLastValue(const QString& symbolName, MyMoneyMoney& value)
{
  return 0;
}
