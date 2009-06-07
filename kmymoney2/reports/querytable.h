/***************************************************************************
                          querytable.h
                         -------------------
    begin                : Fri Jul 23 2004
    copyright            : (C) 2004-2005 by Ace Jones
                           (C) 2007 Sascha Pfau
    email                :  acejones@users.sourceforge.net
                            MrPeacock@gmail.com
 ***************************************************************************/
 
/****************************************************************************
  Contains code from the func_xirr and related methods of financial.cpp 
  - KOffice 1.6 by Sascha Pfau.  Sascha agreed to relicense those methods under
  GPLv2 or later.
*****************************************************************************/

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
#include "listtable.h"

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

class QueryTable : public ListTable
{
  public:
    QueryTable(const MyMoneyReport&);
    void init(void);

  protected:
    void constructAccountTable(void);
    void constructTransactionTable(void);
    void constructPerformanceRow( const ReportAccount& account, TableRow& result ) const;
    void constructSplitsTable(void);

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
  const QDate& today( void ) const { return m_sToday; }

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

    /**
     * Function: XIRR
     *
     * Compute the internal rate of return for a non-periodic series of cash flows.
     *
     * XIRR ( Values; Dates; [ Guess = 0.1 ] )
     **/
    double calculateXIRR ( void ) const;

  protected:
    CashFlowListItem mostRecent(void) const;

  private:
        /**
   * helper: xirrResult
   *
   * args[0] = values
   * args[1] = dates
         **/
    double xirrResult ( double& rate ) const;

        /**
     *
     * helper: xirrResultDerive
     *
     * args[0] = values
     * args[1] = dates
         **/
    double xirrResultDerive ( double& rate ) const;
};

}

#endif // QUERYREPORT_H
