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
#include <qcanvas.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes
#include "../mymoney/mymoneyfile.h"
#include "../mymoney/mymoneyreport.h"

namespace reports {

// define to enable massive debug logging to stderr
// #undef DEBUG_REPORTS
#define DEBUG_REPORTS

#define DEBUG_ENABLED_BY_DEFAULT false

#ifdef DEBUG_REPORTS

// define to filter out account names & transaction amounts
// DO NOT check into CVS with this defined!! It breaks all
// unit tests.
#undef DEBUG_HIDE_SENSITIVE

#define DEBUG_ENTER(x) Tester ___TEST(x)
#define DEBUG_OUTPUT(x) ___TEST.output(x)
#define DEBUG_OUTPUT_IF(x,y) { if (x) ___TEST.output(y); }
#define DEBUG_ENABLE(x) Tester::enable(x)
#define DEBUG_ENABLE_KEY(x) Tester::setEnableKey(x)
#ifdef DEBUG_HIDE_SENSITIVE
#define DEBUG_SENSITIVE(x) QString("hidden")
#else
#define DEBUG_SENSITIVE(x) (x)
#endif

#else

#define DEBUG_ENTER(x)
#define DEBUG_OUTPUT(x)
#define DEBUG_OUTPUT_IF(x,y)
#define DEBUG_ENABLE(x) 
#define DEBUG_SENSITIVE(x)
#endif

class Tester
{
  QString m_methodName;
  static QString m_sTabs;
  static bool m_sEnabled;
  bool m_enabled;
  static QString m_sEnableKey;
public:
  Tester( const QString& _name );
  ~Tester();
  void output( const QString& _text );
  static void enable( bool _e ) { m_sEnabled = _e; }
  static void setEnableKey( const QString& _s ) { m_sEnableKey = _s; }
};

class KReportChartView: public QCanvasView 
{
public:
  QCanvas* m_canvas;
  
  KReportChartView( QWidget* parent, const char* name ): QCanvasView(parent,name) { m_canvas = new QCanvas(parent); setCanvas( m_canvas ); m_canvas->resize(600,600); }
  ~KReportChartView() { if ( m_canvas ) delete m_canvas; }
  static bool implemented(void) { return false; }
};

/**
  * This is a MyMoneyAccount as viewed from the reporting engine.
  *
  * All reporting methods should use ReportAccount INSTEAD OF
  * MyMoneyAccount at all times.
  *
  * The primary functionality this provides is a full chain of account
  * hierarchy that is easy to traverse.  It's needed because the PivotTable
  * grid needs to store and sort by the full account hierarchy, while still
  * having access to the account itself for currency conversion.
  *
  * In addition, several other convenience functions are provided that may
  * be worth moving into MyMoneyAccount at some point.
  *
  * @author Ace Jones
  *
  * @short
**/
class ReportAccount: public MyMoneyAccount
{
private:
  QStringList m_nameHierarchy;

public:
  /**
    * Default constructor
    *
    * Needed to allow this object to be stored in a QMap.
    */
  ReportAccount( void );

  /**
    * Copy constructor
    *
    * Needed to allow this object to be stored in a QMap.
    */
  ReportAccount( const ReportAccount& );

  /**
    * Regular constructor
    *
    * @param accountid Account which this account descriptor should be based off of
    */
  ReportAccount( const QCString& accountid );

  /**
    * Regular constructor
    *
    * @param account Account which this account descriptor should be based off of
    */
  ReportAccount( const MyMoneyAccount& accountid );

  /**
    * @param right The object to compare against
    * @return bool True if this account's fully-qualified hierarchy name
    * is less than that of the given qccount
    */
  bool operator<( const ReportAccount& right ) const;

  /**
    * Returns the price of this account's underlying currency on the indicated date,
    * translated into the account's deep currency
    *
    * There are three differeny currencies in play with a single Account:
    *   - The underlying currency: What currency the account itself is denominated in
    *   - The deep currency: The underlying currency's own underlying currency.  This
    *      is only a factor if the underlying currency of this account IS NOT a
    *      currency itself, but is some other kind of security.  In that case, the
    *      underlying security has its own currency.  The deep currency is the
    *      currency of the underlying security.  On the other hand, if the account
    *      has a currency itself, then the deep currency == the underlying currency,
    *      and this function will return 1.0.
    *   - The base currency: The base currency of the user's overall file
    *
    * @param date The date in question
    * @return MyMoneyMoney The value of the account's currency on that date
    */
  MyMoneyMoney deepCurrencyPrice( const QDate& date ) const;

  /**
    * Returns the price of this account's deep currency on the indicated date,
    * translated into the base currency
    *
    * @param date The date in question
    * @return MyMoneyMoney The value of the account's currency on that date
    */
  MyMoneyMoney baseCurrencyPrice( const QDate& date ) const;

  /**
    * Fetch the trading symbol of this account's deep currency
    *
    * @return QCString The account's currency trading symbol
    */
  QCString currency( void ) const;
    
  /**
    * Determine if this account's deep currency is different from the file's
    * base currency
    *
    * @return bool True if this account is in a foreign currency
    */
  bool isForiegnCurrency( void ) const;
   
  /**
    * The name of only this account.  No matter how deep the hierarchy, this
    * method only returns the last name in the list, which is the engine name]
    * of this account.
    *
    * @return QString The account's name
    */
  QString name( void ) const;

  /**
    * The entire hierarchy of this account descriptor
    * This is similiar to debugName(), however debugName() is not guaranteed
    * to always look pretty, while fullName() is.  So if the user is ever
    * going to see the results, use fullName().
    *
    * @return QString The account's full hierarchy
    */
  QString fullName( void ) const;

  /**
    * The entire hierarchy of this account descriptor, suitable for displaying
    * in debugging output
    *
    * @return QString The account's full hierarchy (suitable for debugging)
    */
  QString debugName( void ) const;

  /**
    * Whether this account is a 'top level' parent account.  This means that
    * it's parent is an account class, like asset, liability, expense or income
    *
    * @return bool True if this account is a top level parent account
    */
  /*inline*/ bool isTopLevel( void ) const;

  /**
    * Returns the name of the top level parent account
    *
    * (See isTopLevel for a definition of 'top level parent')
    *
    * @return QString The name of the top level parent account
    */
  /*inline*/ QString topParentName( void ) const;

  /**
    * Returns a report account containing the top parent account
    *
    * @return ReportAccount The account of the top parent
    */
  ReportAccount topParent( void ) const;
  
  /**
    * Returns a report account containing the immediate parent account
    *
    * @return ReportAccount The account of the immediate parent
    */
  ReportAccount parent( void ) const;
  
  /**
    * Returns the number of accounts in this account's hierarchy.  If this is a
    * Top Category, it returns 1.  If it's parent is a Top Category, returns 2,
    * etc.
    *
    * @return unsigned Hierarchy depth
    */
  unsigned hierarchyDepth( void ) const;

  bool isIncomeExpense(void) const;
  bool isAssetLiability(void) const;

protected:
  /**
    * Calculates the full account hierarchy of this account
    */
  void calculateAccountHierarchy( void );

};

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
class PivotTable
{
public:
  /**
    * Create a Pivot table style report
    *
    * @param MyMoneyReport The configuration parameters for this report
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
    * @return QString CSV string representing the report
    */
    void drawChart( KReportChartView& ) const;
    
  /**
    * Dump the report's HTML to a file
    *
    * @param QString The filename to dump into
    */
    void dump( const QString& file ) const;

private:
  /**
    * The fundamental data construct of this class is a 'grid'.  It is organized as follows:
    *
    * A 'Row' is a row of money values, each column is a month.  The first month corresponds to
    * m_beginDate
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
    class TGridRow: public QValueList<MyMoneyMoney> { public: MyMoneyMoney m_total; };
    class TInnerGroup: public QMap<ReportAccount,TGridRow> { public: TGridRow m_total; };
    class TOuterGroup: public QMap<QString,TInnerGroup> { public: TGridRow m_total; };
    class TGrid: public QMap<QString,TOuterGroup> { public: TGridRow m_total; };

    TGrid m_grid;

    QStringList m_columnHeadings;
    unsigned m_numColumns;
    QDate m_beginDate;
    QDate m_endDate;
    
    MyMoneyReport m_config_f;

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
    */
    inline void assignCell( const QString& outergroup, const ReportAccount& row, unsigned column, MyMoneyMoney value );

  /**
    * Record the opening balances of all qualifying accounts into the grid.
    *
    * For accounts opened before the report period, places the balance into the '0' column.
    * For those opened during the report period, places the balance into the appropriate column
    * for the month when it was opened.
    *
    * @param accounttypes Which account types to include.  Valid values are major account types: MyMoneyAccount::Asset, Liability, Income, Expense.
    */
    void calculateOpeningBalances( void );

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
};

}
#endif
// PIVOTTABLE_H
