/***************************************************************************
                          querytable.h
                         -------------------
    begin                : Fri Jul 23 2004
    copyright            : (C) 2004-2005 by Ace Jones
    email                :  acejones@users.sourceforge.net
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

class ReportAccount;

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
  void constructPerformanceRow( const ReportAccount& account, TableRow& result, const MyMoneyMoney& ) const;

private:
  QValueList<TableRow> m_transactions;
  const MyMoneyReport& m_config;
  QString m_group;
  /**
    * Comma-separated list of columns to place BEFORE the subtotal column
    */
  QString m_columns;
  /**
    * Name of the subtotal column
    */
  QString m_subtotal;
  /**
    * Comma-separated list of columns to place AFTER the subtotal column
    */
  QString m_postcolumns;
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
  MyMoneyMoney NPV(double rate) const;
  double IRR(void) const;
  MyMoneyMoney total(void) const;
  void dumpDebug(void) const;
protected:
  const CashFlowListItem& mostRecent(void) const;
};

}

#endif // QUERYREPORT_H
