/***************************************************************************
                          mymoneyonlinepriceupdate.h
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

#ifndef MYMONEYONLINEPRICEUPDATE_H
#define MYMONEYONLINEPRICEUPDATE_H

#include "mymoneymoney.h"
#include <qdatetime.h>
#include <qmap.h>
#include <qstringlist.h>

typedef struct {
  QString symbolName;
  MyMoneyMoney value;
  QDate lastUpdated;
  int error;
} OnlineUpdateStruct;

#define SUCCESS                         0x0
#define SUCCESS_WITH_ERRORS             0x1
#define ERROR_SYMBOL_NAME_NOT_FOUND     0x100
#define ERROR_PERL_NOT_FOUND            0x101
#define ERROR_NO_CONNECTION             0x102

class MyMoneyOnlinePriceUpdate{
public:
    MyMoneyOnlinePriceUpdate();
    ~MyMoneyOnlinePriceUpdate();

    ///Sets the location of the perl binary for our use.
    void setPerlLocation(const QString& strLoc);
    
    ///Retrieves quotes using
    int getQuotes(const QStringList& symbolNames);
    
    ///Tells this object to update price histories of MyMoneyEquity objects.
    int applyNewQuotes();
    
    ///Retrieves the date of the last update for this symbol name.
    int getLastUpdateDate(const QString& symbolName, QDate& date);
    
    ///Retrieves the price returned from the quote engine.  
    int getLastValue(const QString& symbolName, MyMoneyMoney& value);
    
private:
    QString m_strPerlLocation;
    QMap<QString, OnlineUpdateStruct> m_data;
};

#endif
