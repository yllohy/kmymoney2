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
public:
  /**
    * Contains a single row in the table.
    *
    * Each column is a key/value pair, both strings.  This class is just
    * a QMap with the added ability to specify which columns you'd like to
    * use as a sort key when you qHeapSort a list of these TableRows
    */
  class TableRow: public QMap<QString,QString>
  {
  public:
    bool operator<( const TableRow& ) const;
    bool operator<=( const TableRow& ) const;
    bool operator>( const TableRow& ) const;
    bool operator==( const TableRow& ) const;
    
    static void setSortCriteria( const QString& _criteria ) { m_sortCriteria = QStringList::split(",",_criteria); }
  private:
    static QStringList m_sortCriteria;
  };

protected:
  void render( QString&, QString& ) const;
  void constructAccountTable(void);
  void constructTransactionTable(void);

private:
  QValueList<TableRow> m_transactions;
  const MyMoneyReport& m_config;
  QString m_group;
  QString m_columns;
  QString m_subtotal;
  QString m_summarize;
  QString m_propagate;

};

//
// Cash Flow analysis tools for investment reports
//

class CashFlowListItem
{
public:
  CashFlowListItem(void) {}
  CashFlowListItem( const QDate& _date, const MyMoneyMoney& _value ): m_date(_date), m_value(_value) {}
  bool operator<( const CashFlowListItem _second ) const { return m_date < _second.m_date; }
  bool operator<=( const CashFlowListItem _second ) const { return m_date <= _second.m_date; }
  bool operator>( const CashFlowListItem _second ) const { return m_date > _second.m_date; }
  const QDate& date( void ) const { return m_date; }
  const MyMoneyMoney& value( void ) const { return m_value; }
  MyMoneyMoney NPV( double _rate ) const;

  static void setToday( const QDate& _today ) { m_sToday = _today; }

private:
  QDate m_date;
  MyMoneyMoney m_value;

  static QDate m_sToday;
};

class CashFlowList: public QValueList<CashFlowListItem>
{
public:
  CashFlowList(void) {}
  CashFlowList(const QValueList<MyMoneyTransaction>&, const CashFlowListItem& _opening, const CashFlowListItem& _current) {}
  MyMoneyMoney NPV(double rate);
  double IRR(void);
  MyMoneyMoney total(void) const;
  void dumpDebug(void) const;
protected:
  const CashFlowListItem& mostRecent(void) const;
};

}

#endif // QUERYREPORT_H
