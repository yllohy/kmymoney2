/***************************************************************************
                          querytable.h  -  description
                             -------------------
    begin                : Fri Jul 23 2004
    copyright            : (C) 2000-2004 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
                           Ace Jones <ace.j@hotpop.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QUERYTABLE_H
#define QUERYTABLE_H

// ----------------------------------------------------------------------------
// QT Includes
#include <qstringlist.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes
#include "../mymoney/mymoneyreport.h"

namespace reports {

/**
  * Calculates a query of information about the transaction database.
  *
  * This is a middle-layer class, between the UI and the engine.  The 
  * MyMoneyReport class holds only the CONFIGURATION parameters.  This 
  * class actually does the work of retrieving the data from the engine
  * and formatting it for the user.
  *
  * @author Ace Jones
  *
  * @short
**/

class QueryTable
{
public:
  QueryTable(const MyMoneyReport&);
  QString renderHTML( void ) const;
  QString renderCSV( void ) const;
  void dump( const QString& file, const QString& context=QString() ) const;
protected:
  void render( QString&, QString& ) const;
public:
  class TableRow: public QMap<QString,QString>
  {
  public:
    bool operator<( const TableRow& ) const;
    static void setSortCriteria( const QString& _criteria ) { m_sortCriteria = QStringList::split(",",_criteria); }
  private:
    static QStringList m_sortCriteria;
  };

private:
  QValueList<TableRow> m_transactions;
  const MyMoneyReport& m_config;
  QString m_group;
  QString m_columns;
  QString m_subtotal;

};

}

#endif // QUERYREPORT_H
