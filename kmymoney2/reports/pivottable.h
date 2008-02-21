/***************************************************************************
                          pivottable.h
                             -------------------
    begin                : Sat May 22 2004
    copyright            : (C) 2004-2005 by Ace Jones
    email                : <ace.j@hotpop.com>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef PIVOTTABLE_H
#define PIVOTTABLE_H

// ----------------------------------------------------------------------------
// QT Includes
#include <qmap.h>
#include <qvaluelist.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes
#include "../mymoney/mymoneyfile.h"
#include "../mymoney/mymoneyreport.h"
#include "reporttable.h"
#include "reportaccount.h"

namespace reports {

/**
  * Calculates a 'pivot table' of information about the transaction database.
  * Based on pivot tables in MS Excel, and implemented as 'Data Pilot' in
  * OpenOffice.Org Calc.
  *
  *              | Month,etc
  * -------------+------------
  * Expense Type | Sum(Value)
  *   Category   |
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
class PivotTable : public ReportTable
{
public:
  /**
    * Create a Pivot table style report
    *
    * @param _config_f The configuration parameters for this report
    */
    PivotTable( const MyMoneyReport& _config_f );

  /**
    * Render the report to an HTML stream.
    *
    * @return QString HTML string representing the report
    */
    QString renderHTML( void ) const;
  /**
    * Render the report to a comma-separated-values stream.
    *
    * @return QString CSV string representing the report
    */
    QString renderCSV( void ) const;

  /**
    * Render the report to a graphical chart
    *
    * Currently, this is not implemented, but it's here as a stub for an
    * ambitious chart-writer.
    *
    * @param view The KReportChartView into which to draw the chart.
    */
    void drawChart( KReportChartView& view ) const;

  /**
    * Dump the report's HTML to a file
    *
    * @param file The filename to dump into
    */
    void dump( const QString& file, const QString& context=QString()) const;

protected:
  void init(void);    // used for debugging the constructor

private:
  /**
    * The fundamental data construct of this class is a 'grid'.  It is organized as follows:
    *
    * A 'Row' is a row of money values, each column is a month.  The first month corresponds to
    * m_beginDate.
    *
    * A 'Row Pair' is two rows of money values.  Each column is the SAME month.  One row is the
    * 'actual' values for the period, the other row is the 'budgetted' values for the same
    * period.  For ease of implementation, a Row Pair is implemented as a Row which contains
    * another Row.  The inherited Row is the 'actual', the contained row is the 'Budget'.
    *
    * An 'Inner Group' contains a rows for each subordinate account within a single top-level
    * account.  It also contains a mapping from the account descriptor for the subordinate account
    * to its row data.  So if we have an Expense account called "Computers", with sub-accounts called
    * "Hardware", "Software", and "Peripherals", there will be one Inner Group for "Computers"
    * which contains three Rows.
    *
    * An 'Outer Group' contains Inner Row Groups for all the top-level accounts in a given
    * account class.  Account classes are Expense, Income, Asset, Liability.  In the case above,
    * the "Computers" Inner Group is contained within the "Expense" Outer Group.
    *
    * A 'Grid' is the set of all Outer Groups contained in this report.
    *
    */
    class TCell: public MyMoneyMoney
    {
    public:
      TCell() : m_stockSplit(MyMoneyMoney(1,1)), m_cellUsed(false) {};
      TCell(const MyMoneyMoney& value) : MyMoneyMoney(value), m_stockSplit(MyMoneyMoney(1,1)), m_cellUsed(false) {}
      static TCell stockSplit(const MyMoneyMoney& factor);
      TCell operator += (const TCell& right);
      TCell operator += (const MyMoneyMoney& value);
      const QString formatMoney(int fraction, bool showThousandSeparator = true) const;
      const QString formatMoney(const QString& currency, const int prec, bool showThousandSeparator = true) const;
      MyMoneyMoney calculateRunningSum(const MyMoneyMoney& runningSum);
      MyMoneyMoney cellBalance(const MyMoneyMoney& _balance);
      bool isUsed(void) const { return m_cellUsed; }
    private:
      MyMoneyMoney m_stockSplit;
      MyMoneyMoney m_postSplit;
      bool m_cellUsed;
    };
    class TGridRow: public QValueList<TCell>
    {
    public:
      TGridRow( unsigned _numcolumns = 0 )
      {
        if ( _numcolumns )
          insert( end(), _numcolumns, TCell() );
      }
      MyMoneyMoney m_total;
    };
    class TGridRowPair: public TGridRow
    {
    public:
      TGridRowPair( unsigned _numcolumns = 0 ): TGridRow(_numcolumns), m_budget(_numcolumns) {}
      TGridRow m_budget;
    };
    class TInnerGroup: public QMap<ReportAccount,TGridRowPair>
    {
    public:
      TInnerGroup( unsigned _numcolumns = 0 ): m_total(_numcolumns) {}

      TGridRowPair m_total;
    };
    class TOuterGroup: public QMap<QString,TInnerGroup>
    {
    public:
      TOuterGroup( unsigned _numcolumns = 0, unsigned _sort=m_kDefaultSortOrder, bool _inverted=false): m_total(_numcolumns), m_inverted(_inverted), m_sortOrder(_sort) {}
      int operator<( const TOuterGroup& _right )
      {
        if ( m_sortOrder != _right.m_sortOrder )
          return m_sortOrder < _right.m_sortOrder;
        else
          return m_displayName < _right.m_displayName;
      }
      TGridRowPair m_total;

      // An inverted outergroup means that all values placed in subordinate rows
      // should have their sign inverted from typical cash-flow notation.  Also it
      // means that when the report is summed, the values should be inverted again
      // so that the grand total is really "non-inverted outergroup MINUS inverted outergroup".
      bool m_inverted;

      // The localized name of the group for display in the report. Outergoups need this
      // independently, because they will lose their association with the TGrid when the
      // report is rendered.
      QString m_displayName;

      // lower numbers sort toward the top of the report. defaults to 100, which is a nice
      // middle-of-the-road value
      unsigned m_sortOrder;

      // default sort order
      static const unsigned m_kDefaultSortOrder;
    };
    class TGrid: public QMap<QString,TOuterGroup> { public: TGridRowPair m_total; };

    TGrid m_grid;

    QStringList m_columnHeadings;
    unsigned m_numColumns;
    QDate m_beginDate;
    QDate m_endDate;
    bool m_runningSumsCalculated;

    // For budget-vs-actual reports only, maps each account to the account which holds
    // the budget for it.  If an account is not contained in this map, it is not included
    // in the budget.
    QMap<QCString,QCString> m_budgetMap;

    MyMoneyReport m_config_f;

    /**
      * This method returns the formatted value of @a amount with
      * a possible @a currencySymbol added and @a prec fractional digits.
      * @a currencySymbol defaults to be empty and @a prec defaults to 2.
      *
      * If @a amount is negative the formatted value is enclosed in an
      * HTML font tag to modify the color to reflect the user settings for
      * negtive numbers.
      *
      * Example: 1.23 is returned as '1.23' whereas -1.23 is returned as
      *          '<font color="rgb($red,$green,$blue)">-1.23</font>' with
      *          $red, $green and $blue being the actual value for the
      *          chosen color.
      */
    QString coloredAmount(const MyMoneyMoney& amount, const QString& currencySymbol = QString(), int prec = 2 ) const;

    /**
      * This method returns the difference between a @a budgeted and an @a
      * actual amount. The calculation is based on the type of the
      * @a repAccount. The difference value returned is calculated as follows:
      *
      * If @a repAccount is of type MyMoneyAccount::Income
      *
      * @code
      *      diff = actual - budgeted
      * @endcode
      *
      * If @a repAccount is of type MyMoneyAccount::Expense
      *
      * @code
      *      diff = budgeted - actual
      * @endcode
      *
      * In all other cases, 0 is returned.
      */
    MyMoneyMoney calculateBudgetDiff( const ReportAccount& repAccount, const TCell& budgeted, const TCell& actual ) const;

protected:
  /**
    * Creates a row in the grid if it doesn't already exist
    *
    * Downsteam assignment functions will assume that this row already
    * exists, so this function creates a row of the needed length populated
    * with zeros.
    *
    * @param outergroup The outer row group
    * @param row The row itself
    * @param recursive Whether to also recursively create rows for our parent accounts
    */
    void createRow( const QString& outergroup, const ReportAccount& row, bool recursive );

  /**
    * Assigns a value into the grid
    *
    * Adds the given value to the value which already exists at the specified grid position
    *
    * @param outergroup The outer row group
    * @param row The row itself
    * @param column The column
    * @param value The value to be added in
    * @param budget Whether this is a budget value (@p true) or an actual
    *               value (@p false). Defaults to @p false.
    * @param stockSplit Wheter this is a stock split (@p true) or an actual
    *                   value (@p false). Defaults to @p false.
    */
    inline void assignCell( const QString& outergroup, const ReportAccount& row, unsigned column, MyMoneyMoney value, bool budget = false, bool stockSplit = false );

  /**
    * Create a row for each included account. This is used when
    * the config parameter isIncludingUnusedAccount() is true
    */
    void createAccountRows(void);

  /**
    * Record the opening balances of all qualifying accounts into the grid.
    *
    * For accounts opened before the report period, places the balance into the '0' column.
    * For those opened during the report period, places the balance into the appropriate column
    * for the month when it was opened.
    */
    void calculateOpeningBalances( void );

  /**
    * Calculate budget mapping
    *
    * For budget-vs-actual reports, this creates a mapping between each account
    * in the user's hierarchy and the account where the budget is held for it.
    * This is needed because the user can budget on a given account for that
    * account and all its descendants.  Also if NO budget is placed on the
    * account or any of its parents, the account is not included in the map.
    */
    void calculateBudgetMapping( void );

  /**
    * Calculate the running sums.
    *
    * After calling this method, each cell of the report will contain the running sum of all
    * the cells in its row in this and earlier columns.
    *
    * For example, consider a row with these values:
    *   01 02 03 04 05 06 07 08 09 10
    *
    * After calling this function, the row will look like this:
    *   01 03 06 10 15 21 28 36 45 55
    */
    void calculateRunningSums( void );
    void calculateRunningSums( TInnerGroup::iterator& it_row);

  /**
    * Calculate the row and column totals
    *
    * This function will set the m_total members of all the TGrid objects.  Be sure the values are
    * all converted to the base currency first!!
    *
    */
    void calculateTotals( void );

  /**
    * Convert each value in the grid to the base currency
    *
    */
    void convertToBaseCurrency( void );

  /**
    * Convert each value in the grid to the account/category's deep currency
    *
    * See AccountDescriptor::deepCurrencyPrice() for a description of 'deep' currency
    *
    */
    void convertToDeepCurrency( void );

  /**
    * Turn month-long columns into larger time periods if needed
    *
    * For example, consider a row with these values:
    *   01 02 03 04 05 06 07 08 09 10
    *
    * If the column pitch is 3 (i.e. quarterly), after calling this function,
    * the row will look like this:
    *   06 15 26 10
    */
    void collapseColumns(void);

  /**
    * Determine the proper column headings based on the time periods covered by each column
    *
    */
    void calculateColumnHeadings(void);

  /**
    * Helper methods for collapseColumns
    *
    */
    void accumulateColumn(unsigned destcolumn, unsigned sourcecolumn);
    void clearColumn(unsigned column);

  /**
    * Calculate the column of a given date.  This is the absolute column in a
    * hypothetical report that covers all of known time.  In reality an actual
    * report will be a subset of that.
    *
    * @param _date The date
    */
    unsigned columnValue(const QDate& _date) const;

  /**
    * Calculate the date of the last day covered by a given column.
    *
    * @param column The column
    */
    QDate columnDate(int column) const;

  /**
    * Returns the balance of a given cell. Throws an exception once calculateRunningSums() has been run.
    */
    MyMoneyMoney cellBalance(const QString& outergroup, const ReportAccount& _row, unsigned column, bool budget);

};


}
#endif
// PIVOTTABLE_H
// vim:cin:si:ai:et:ts=2:sw=2:
