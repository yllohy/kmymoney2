/***************************************************************************
                          pivottable.h  -  description
                             -------------------
    begin                : Sat May 22 2004
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

namespace reports {

// define to enable massive debug logging to stderr
#undef DEBUG_REPORTS
//#define DEBUG_REPORTS
#define DEBUG_ENABLED_BY_DEFAULT false

#ifdef DEBUG_REPORTS
#define DEBUG_ENTER(x) Tester ___TEST(x)
#define DEBUG_OUTPUT(x) ___TEST.output(x)
#define DEBUG_OUTPUT_IF(x,y) { if (x) ___TEST.output(y); }
#define DEBUG_ENABLE(x) Tester::enable(x)
#else
#define DEBUG_ENTER(x)
#define DEBUG_OUTPUT(x)
#define DEBUG_OUTPUT_IF(x,y)
#define DEBUG_ENABLE(x) 
#endif

class Tester
{
  QString m_methodName;
  static QString m_sTabs;
  static bool m_sEnabled;
  bool m_enabled;
public:
  Tester( const QString& _name );
  ~Tester();
  void output( const QString& _text );
  static void enable( bool _e ) { m_sEnabled = _e; }
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
protected:
  /**
    * Holds a description of a MyMoneyAccount suitable for use in the PivotTable
    * class.
    *
    * The primary functionality this provides is a full chain of account
    * hierarchy that is easy to traverse.  It's needed because the PivotTable
    * grid needs to store and sort by the full account hierarchy, while still
    * having access to the account itself for currency conversion.
    *
    * @author Ace Jones
    *
    * @short
  **/
  class AccountDescriptor
  {
  private:
    QCString m_account;
    const MyMoneyFile* m_file;
    QStringList m_names;
  
  public:
    /**
      * Default constructor
      *
      * Needed to allow this object to be stored in a QMap.
      */
    AccountDescriptor( void );
  
    /**
      * Copy constructor
      *
      * Needed to allow this object to be stored in a QMap.
      */
    AccountDescriptor( const AccountDescriptor& );
  
    /**
      * Regular constructor
      *
      * @param accountid Account which this account descriptor should be based off of
      */
    AccountDescriptor( const QCString& accountid );
  
    /**
      * @param right The object to compare against
      * @return bool True if this object is less than the given object
      */
    bool operator<( const AccountDescriptor& right ) const;
  
    /**
      * Returns the price of this account's currency on the indicated date,
      * translated into the base currency
      *
      * @param date The date in question
      * @return MyMoneyMoney The value of the account's currency on that date
      */
    MyMoneyMoney currencyPrice( const QDate& date ) const;
  
    /**
      * The bottom-most sub-account of this account descriptor, prefaced with
      * one tab for each parent.  'Tab' is currently implemented as 2 nbsp's.
      *
      * This is suitable for printing in the PivotTable report's HTML
      *
      * Note that this is a fairly specialized function for reporting.  If we
      * need a more general-case display of the full hierarchy, a different
      * method will be needed.
      *
      * @param showcurrency True if the currency of this account should be included. Note this flag is only considered if the account is not in the file's base currency.
      * @param htmlformat True if the spaces should be replaced with &nbsp;'s
      * @return QString The account's full hierarchy (suitable for printing)
      */
    QString htmlTabbedName( bool showcurrency, bool htmlformat=true ) const;
  
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
    inline bool isTopLevel( void ) const;
  
    /**
      * Returns the name of the top level parent account
      *
      * (See isTopLevel for a definition of 'top level parent')
      *
      * @return QString The name of the top level parent account
      */
    inline QString getTopLevel( void ) const;
  
    /**
      * Returns the account descriptor of the immediate parent account
      *
      * @return AccountDescriptor The account descriptor of the immediate parent
      */
    AccountDescriptor getParent( void ) const;
  
  protected:
    /**
      * Calculates the full account hierarchy of this account
      */
    void calculateAccountHierarchy( void );
  
  };

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
    class TInnerGroup: public QMap<AccountDescriptor,TGridRow> { public: TGridRow m_total; };
    class TOuterGroup: public QMap<QString,TInnerGroup> { public: TGridRow m_total; };
    class TGrid: public QMap<QString,TOuterGroup> { public: TGridRow m_total; };

    TGrid m_grid;
    QValueList<MyMoneyAccount::accountTypeE> m_accounttypes;

    QStringList m_columnHeadings;
    unsigned m_numColumns;
    QDate m_beginDate;
    QDate m_endDate;
    
    MyMoneyReport m_config_f;

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
    * @param showSubAccounts Whether to include all sub-accounts in the report (true) or just the top-most parents (false)
    * @return QString HTML string representing the report
    */
    QString renderHTML( void ) const;
    QString renderCSV( void ) const;
    
    void dump( const QString& file ) const;

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
    void createRow( const QString& outergroup, const AccountDescriptor& row, bool recursive );

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
    inline void assignCell( const QString& outergroup, const AccountDescriptor& row, unsigned column, MyMoneyMoney value );

  /**
    * Determines whether this is an income/expense category
    */
    inline static bool isCategory(const MyMoneyAccount&);
    
  /**
    * Determines whether this account is considered in the report
    */
    bool includesAccount( const MyMoneyAccount& account ) const;


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
    
    void collapseColumns(void);
    void calculateColumnHeadings(void);
    
    void accumulateColumn(unsigned destcolumn, unsigned sourcecolumn);
    void clearColumn(unsigned column);
    
};

}
#endif
// PIVOTTABLE_H
