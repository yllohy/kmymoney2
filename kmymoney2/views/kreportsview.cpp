/***************************************************************************
                          kreportsview.cpp  -  description
                             -------------------
    begin                : Sat Mar 27 2004
    copyright            : (C) 2000-2004 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
                           Ace Jones <ace.jones@hotpop.com>
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
#include <qlayout.h>
#include <qdatetime.h>
#include <qregexp.h>
#include <qdragobject.h>
#include <qclipboard.h>
#include <qapplication.h>
#include <qprinter.h>
#include <qpainter.h>
#include <qfile.h>

// ----------------------------------------------------------------------------
// KDE Includes
#include <kglobal.h>
#include <klocale.h>
#if QT_VERSION > 300
#include <kstandarddirs.h>
#else
#include <kstddirs.h>
#endif

#include <khtmlview.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kfiledialog.h>

// ----------------------------------------------------------------------------
// Project Includes
#include "kreportsview.h"
#include "../mymoney/mymoneyfile.h"
#include "../kmymoneyutils.h"

#define VIEW_LEDGER         "ledger"
#define VIEW_SCHEDULE       "schedule"
#define VIEW_WELCOME        "welcome"
#define VIEW_HOME           "home"
#define VIEW_REPORTS        "reports"

// define to enable massive debug logging to stderr
#undef DEBUG_REPORTS
//#define DEBUG_REPORTS

#ifdef DEBUG_REPORTS
#define DEBUG_ENTER(x) Tester ___TEST(x)
#define DEBUG_OUTPUT(x) ___TEST.output(x)
#define DEBUG_OUTPUT_IF(x,y) { if (x) ___TEST.output(y); }
#else
#define DEBUG_ENTER(x)
#define DEBUG_OUTPUT(x)
#define DEBUG_OUTPUT_IF(x,y)
#endif

class Tester
{
  QString m_methodName;
  static QString m_sTabs;
public:
  Tester( const QString& _name );
  ~Tester();
  void output( const QString& _text );

};

QString Tester::m_sTabs;

Tester::Tester( const QString& _name ): m_methodName( _name )
{
  qDebug( "%s%s(): ENTER", m_sTabs.latin1(), m_methodName.latin1() );
  m_sTabs.append("  ");
}

Tester::~Tester()
{
  m_sTabs.remove(0,2);
  qDebug( "%s%s(): EXIT", m_sTabs.latin1(), m_methodName.latin1() );
}

void Tester::output( const QString& _text )
{
  qDebug( "%s%s(): %s", m_sTabs.latin1(), m_methodName.latin1(), _text.latin1() );
}

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
    * @return QString The account's full hierarchy (suitable for printing)
    */
  QString htmlTabbedName( void ) const;

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

AccountDescriptor::AccountDescriptor( void )
{
  DEBUG_ENTER("AccountDescriptor::AccountDescriptor( void )");
  m_file = MyMoneyFile::instance();

}

AccountDescriptor::AccountDescriptor( const AccountDescriptor& copy ):
  m_account( copy.m_account ), m_names( copy.m_names )
{
  // NOTE: I implemented the copy constructor solely for debugging reasons
  
  DEBUG_ENTER("AccountDescriptor::AccountDescriptor( const AccountDescriptor&  )");
  m_file = MyMoneyFile::instance();
  
}

AccountDescriptor::AccountDescriptor( const QCString& accountid ): m_account( accountid )
{
  DEBUG_ENTER("AccountDescriptor::AccountDescriptor( account )");
  m_file = MyMoneyFile::instance();
  calculateAccountHierarchy();
}

void AccountDescriptor::calculateAccountHierarchy( void )
{
  DEBUG_ENTER("AccountDescriptor::calculateAccountHierarchy");
  
  QCString resultid = m_account;
  QCString parentid = m_file->account(resultid).parentAccountId();
  m_names.prepend(m_file->account(resultid).name());

  while (!m_file->isStandardAccount(parentid))
  {
    // take on the identity of our parent
    resultid = parentid;

    // and try again
    parentid = m_file->account(resultid).parentAccountId();
    m_names.prepend(m_file->account(resultid).name());
  }
}

MyMoneyMoney AccountDescriptor::currencyPrice(const QDate& date) const
{
  DEBUG_ENTER("AccountDescriptor::currencyPrice");
  
  MyMoneyMoney value(1.0);

  MyMoneyAccount account = m_file->account(m_account);
  
  if(account.currencyId() != m_file->baseCurrency().id()) {
    MyMoneyCurrency currency = m_file->currency(account.currencyId());
    value = currency.price(date);

    DEBUG_OUTPUT(QString("Converting %1 to %2, price on %3 is %4").arg(m_file->currency(account.currencyId()).name()).arg(m_file->baseCurrency().name()).arg(date.toString()).arg(value.toDouble()));
  }

  return value;
}

bool AccountDescriptor::operator<(const AccountDescriptor& second) const
{
  DEBUG_ENTER("AccountDescriptor::operator<");

  bool result = false;
  bool haveresult = false;
  QStringList::const_iterator it_first = m_names.begin();
  QStringList::const_iterator it_second = second.m_names.begin();
  while ( it_first != m_names.end() )
  {
    // The first string is longer than the second, but otherwise identical
    if ( it_second == second.m_names.end() )
    {
      result = false;
      haveresult = true;
      break;
    }

    if ( (*it_first) < (*it_second) )
    {
      result = true;
      haveresult = true;
      break;
    }

    ++it_first;
    ++it_second;
  }

  // The second string is longer than the first, but otherwise identical
  if ( !haveresult && ( it_second != second.m_names.end() ) )
    result = true;

  return result;
}

QString AccountDescriptor::htmlTabbedName( void ) const
{
  QString result = m_names.back();
  int tabs = m_names.size() - 1;
  while ( tabs-- )
    result.prepend("&nbsp;&nbsp;");
  result.replace(QRegExp(" "),QString("&nbsp;"));

  return result;
}

QString AccountDescriptor::debugName( void ) const
{
  return m_names.join("|");
}

bool AccountDescriptor::isTopLevel( void ) const
{
  return ( m_names.size() == 1 );
}

AccountDescriptor AccountDescriptor::getParent( void ) const
{
  return AccountDescriptor( m_file->account(m_account).parentAccountId() );
}

QString AccountDescriptor::getTopLevel( void ) const
{
  return m_names.first();
}

/**
  * Holds a 'pivot table' of information about the transaction database.
  * Based on pivot tables in MS Excel, and implemented as 'Data Pilot' in
  * OpenOffice.Org Calc.
  *
  *              | Month
  * -------------+------------
  * Expense Type | Sum(Value)
  *   Category   |
  *
  * @author Ace Jones
  *
  * @short
**/
class PivotTable
{
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
    typedef QValueList<MyMoneyMoney> TGridRow;
    typedef QMap<AccountDescriptor,TGridRow> TInnerGroup;
    typedef QMap<QString,TInnerGroup> TOuterGroup;
    typedef QMap<QString,TOuterGroup> TGrid;
    TGrid m_grid;

    QStringList m_columnHeadings;
    bool m_displayRowTotals;
    int m_numColumns;
    QDate m_beginDate;
    QDate m_endDate;

public:
  /**
    * Create a Pivot table style report
    *
    * @param file Pointer to the current MyMoneyFile
    * @param begindate First date of the reporting range (only month and year are used, day is assumed to be '1').
    * @param enddate One day past the reporting range (only month and year are used, day is assumed to be '1').
    * @param accounttypes Which account types to include.  Valid values are major account types: MyMoneyAccount::Asset, Liability, Income, Expense.
    * @param runningsum Whether the table entries should be sums of all transactions to date (true) or only for the single month (false)
    */
    PivotTable( const MyMoneyFile* file, const QDate& begindate, const QDate& enddate, const QValueList<MyMoneyAccount::accountTypeE>& accounttypes, bool runningsum );

  /**
    * Render the report to an HTML stream.
    *
    * @param showSubAccounts Whether to include all sub-accounts in the report (true) or just the top-most parents (false)
    * @return QString HTML string representing the report
    */
    QString renderHTML( bool showSubAccounts ) const;

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
    inline void assignCell( const QString& outergroup, const AccountDescriptor& row, int column, MyMoneyMoney value );

  /**
    * Record the opening balances of all qualifying accounts into the grid.
    *
    * For accounts opened before the report period, places the balance into the '0' column.
    * For those opened during the report period, places the balance into the appropriate column
    * for the month when it was opened.
    *
    * @param accounttypes Which account types to include.  Valid values are major account types: MyMoneyAccount::Asset, Liability, Income, Expense.
    */
    void calculateOpeningBalances( const QValueList<MyMoneyAccount::accountTypeE>& accounttypes );

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
    * Convert each value in the grid to the base currency
    *
    */
    void convertToBaseCurrency( void );
};

PivotTable::PivotTable( const MyMoneyFile* file, const QDate& begindate, const QDate& enddate, const QValueList<MyMoneyAccount::accountTypeE>& accounttypes, bool runningsum )
{
  DEBUG_ENTER("PivotTable::PivotTable");

  //
  // Initialize member variables
  //

  // strip out the 'days' component of the begin and end dates.
  // we're only concerned about the month & year.
  m_beginDate =  QDate( begindate.year(), begindate.month(), 1 );
  m_endDate = QDate( enddate.year(), enddate.month(), 1 );

  m_displayRowTotals = !runningsum;

  //
  // Determine column headings
  //

  // one column for the opening balance
  m_columnHeadings.append( "Opening" );

  // one for each month in the date range
  QDate columndate = m_beginDate;
  while ( columndate < m_endDate )
  {
    m_columnHeadings.append( QDate::shortMonthName(columndate.month()) );
    columndate = columndate.addMonths( 1 );
  }

  m_numColumns =  m_columnHeadings.size();

  //
  // Get opening balances
  // (for running sum reports only)
  //

  if ( runningsum )
    calculateOpeningBalances( accounttypes );

  //
  // Populate all transactions into the row/column pivot grid
  //

  MyMoneyTransactionFilter filter;
  filter.setReportAllSplits(false);
  filter.setDateFilter(m_beginDate, m_endDate);
  QValueList<MyMoneyTransaction> transactions = file->transactionList(filter);
  QValueList<MyMoneyTransaction>::const_iterator it_transaction = transactions.begin();
  int colofs = m_beginDate.year() * 12 + m_beginDate.month() - 1;

  while ( it_transaction != transactions.end() )
  {
    QDate postdate = (*it_transaction).postDate();
    int column = postdate.year() * 12 + postdate.month() - colofs;

    QValueList<MyMoneySplit> splits = (*it_transaction).splits();
    QValueList<MyMoneySplit>::const_iterator it_split = splits.begin();
    while ( it_split != splits.end() )
    {
      MyMoneyAccount splitAccount = file->account((*it_split).accountId());
      // only reverse sign for expense/income splits
      MyMoneyMoney reverse((splitAccount.accountGroup() == MyMoneyAccount::Income) |
                           (splitAccount.accountGroup() == MyMoneyAccount::Expense) ? -1 : 1, 1);

      // only include this item if its account group is included in this report
      if ( accounttypes.contains(splitAccount.accountGroup()) )
      {
        // reverse sign to match common notation for cash flow direction
        MyMoneyMoney value = (*it_split).value((*it_transaction).commodity(), splitAccount.currencyId())*reverse;

        // the outer group is the account class (major account type)
        QString outergroup = KMyMoneyUtils::accountTypeToString(splitAccount.accountGroup());

        // the row itself is the account
        AccountDescriptor row( splitAccount.id() );

        // add the value to its correct position in the pivot table
        assignCell( outergroup, row, column, value );

      }
      ++it_split;
    }
    ++it_transaction;
  }

  //
  // Calculate the running sums
  // (for running sum reports only)
  //

  if ( runningsum )
    calculateRunningSums();

  //
  // Convert all values to the base currency
  //

  convertToBaseCurrency();

}

void PivotTable::calculateOpeningBalances( const QValueList<MyMoneyAccount::accountTypeE>& accounttypes )
{
  DEBUG_ENTER("PivotTable::calculateOpeningBalances");

  MyMoneyFile* file = MyMoneyFile::instance();

  QValueList<MyMoneyAccount> accounts = file->accountList();
  QValueList<MyMoneyAccount>::const_iterator it_account = accounts.begin();

  while ( it_account != accounts.end() )
  {
    // only include this item if its account group is included in this report
    if ( accounttypes.contains((*it_account).accountGroup()) )
    {
      // extract the balance of the account for the given begin date
      MyMoneyMoney value = file->balance((*it_account).id(), m_beginDate.addDays(-1));

      // place into the 'opening' column...
      int column = 0;

      // unless the account was opened after the start of the report period
      QDate opendate = (*it_account).openingDate();
      if ( opendate >= m_beginDate )
      {
        if ( opendate < m_endDate ) {
          // in which case it should be placed in the correct column
          column = opendate.year() * 12 + opendate.month() - m_beginDate.year() * 12 - m_beginDate.month() + 1;

        } else
          // or disregarded if the account was opened after the end of the
          // report period
          value = 0;
      }

      // the row group is the account class (major account type)
      QString outergroup = KMyMoneyUtils::accountTypeToString((*it_account).accountGroup());

      // the row itself is the account
      AccountDescriptor row( (*it_account).id() );

      // add the value to its correct position in the pivot table
      assignCell( outergroup, row, column, value );

    }
    ++it_account;
  }
}

void PivotTable::calculateRunningSums( void )
{
  DEBUG_ENTER("PivotTable::calculateRunningSums");

  TGrid::iterator it_outergroup = m_grid.begin();
  while ( it_outergroup != m_grid.end() )
  {
    TOuterGroup::iterator it_innergroup = (*it_outergroup).begin();
    while ( it_innergroup != (*it_outergroup).end() )
    {
      TInnerGroup::iterator it_row = (*it_innergroup).begin();
      while ( it_row != (*it_innergroup).end() )
      {
        MyMoneyMoney runningsum = it_row.data()[0];
        int column = 1;
        while ( column < m_numColumns )
        {
          runningsum = ( it_row.data()[column] += runningsum );

          ++column;
        }
        ++it_row;
      }
      ++it_innergroup;
    }
    ++it_outergroup;
  }
}

void PivotTable::convertToBaseCurrency( void )
{
  DEBUG_ENTER("PivotTable::convertToBaseCurrency");

  TGrid::iterator it_outergroup = m_grid.begin();
  while ( it_outergroup != m_grid.end() )
  {
    TOuterGroup::iterator it_innergroup = (*it_outergroup).begin();
    while ( it_innergroup != (*it_outergroup).end() )
    {
      TInnerGroup::iterator it_row = (*it_innergroup).begin();
      while ( it_row != (*it_innergroup).end() )
      {
        QDate valuedate = m_beginDate.addMonths(1).addDays(-1);
        int column = 1;
        while ( column < m_numColumns )
        {
          // (acejones) Would be nice to have
          // MyMoneyMoney& MyMoneyMoney::operator*=(const MyMoneyMoney&)
          // because then we could just do the following:
          // it_row.data()[column] *= it_row.key().currencyPrice(valuedate);
          double conversionfactor = it_row.key().currencyPrice(valuedate).toDouble();
          double oldval = it_row.data()[column].toDouble();
          double value = oldval * conversionfactor;
          it_row.data()[column] = MyMoneyMoney( value );

          //DEBUG_OUTPUT_IF(static_cast<int>(conversionfactor*1000) != 1000,QString("Factor of %1, value was %2, now %3").arg(conversionfactor).arg(oldval).arg(it_row.data()[column].toDouble()));
          DEBUG_OUTPUT_IF(conversionfactor != 1.0 ,QString("Factor of %1, value was %2, now %3").arg(conversionfactor).arg(oldval).arg(it_row.data()[column].toDouble()));

          // Move to the end of the next month
          valuedate = valuedate.addDays(1).addMonths(1).addDays(-1);

          ++column;
        }
        ++it_row;
      }
      ++it_innergroup;
    }
    ++it_outergroup;
  }
}

void PivotTable::assignCell( const QString& outergroup, const AccountDescriptor& row, int column, MyMoneyMoney value )
{
  DEBUG_ENTER("PivotTable::assignCell");
  DEBUG_OUTPUT(QString("Parameters: %1,%2,%3,%4").arg(outergroup).arg(row.debugName()).arg(column).arg(value.toDouble()));

  // ensure the row already exists (and its parental hierarchy)
  createRow( outergroup, row, true );

  // Determine the inner group from the top-most parent account
  QString innergroup( row.getTopLevel() );

  m_grid[outergroup][innergroup][row][column] += value;
}

void PivotTable::createRow( const QString& outergroup, const AccountDescriptor& row, bool recursive )
{
  DEBUG_ENTER("PivotTable::createRow");

  // Determine the inner group from the top-most parent account
  QString innergroup( row.getTopLevel() );

  // fill the row list with blanks if it doesn't already exist.
  if ( m_grid[outergroup][innergroup][row].isEmpty() )
  {
    DEBUG_OUTPUT(QString("m_grid[%1][%2][%3].insert(%4)").arg(outergroup).arg(innergroup).arg(row.debugName()).arg(m_numColumns));

    m_grid[outergroup][innergroup][row].insert( m_grid[outergroup][innergroup][row].end(), m_numColumns, 0 );

    if ( recursive && !row.isTopLevel() )
        createRow( outergroup, row.getParent(), recursive );
  }
}

QString PivotTable::renderHTML( bool showSubAccounts ) const
{
  DEBUG_ENTER("PivotTable::renderHTML");

  QString colspan = QString(" colspan=\"%1\"").arg(m_numColumns + 1 + (m_displayRowTotals ? 1 : 0) );

  //
  // Table Header
  //

  QString result = QString("\n\n<table class=\"report\" cellspacing=\"0\">"
      "<tr class=\"itemheader\">\n<th>%1</th>").arg(i18n("Account"));

  int column = 1;
  while ( column < m_numColumns )
  {
    result += QString("<th>%1</th>").arg(m_columnHeadings[column]);
    ++column;
  }

  if ( m_displayRowTotals )
    result += QString("<th>%1</th>").arg(i18n("Total"));

  result += "</tr>\n";

  // calculate the column grand totals along the way
  QValueList<MyMoneyMoney> columngrandtotal;
  columngrandtotal.insert( columngrandtotal.end(), m_numColumns, MyMoneyMoney() );

  //
  // Outer groups
  //

  // iterate over outer groups
  TGrid::const_iterator it_outergroup = m_grid.begin();
  while ( it_outergroup != m_grid.end() )
  {
    //
    // Outer Group Header
    //

    result += QString("<tr class=\"sectionheader\"><td class=\"left\"%1>%2</td></tr>\n").arg(colspan).arg(it_outergroup.key());

    QValueList<MyMoneyMoney> columntotal;
    columntotal.insert( columntotal.end(), m_numColumns, MyMoneyMoney() );

    //
    // Inner Groups
    //

    TOuterGroup::const_iterator it_innergroup = (*it_outergroup).begin();
    int rownum = 0;
    while ( it_innergroup != (*it_outergroup).end() )
    {
      QValueList<MyMoneyMoney> innercolumntotal;
      innercolumntotal.insert( innercolumntotal.end(), m_numColumns, MyMoneyMoney() );

      //
      // Rows
      //

      QString innergroupdata;
      TInnerGroup::const_iterator it_row = (*it_innergroup).begin();
      while ( it_row != (*it_innergroup).end() )
      {
        //
        // Columns
        //

        QString rowdata;
        int column = 1;
        MyMoneyMoney rowtotal;
        while ( column < m_numColumns )
        {
          MyMoneyMoney value = it_row.data()[column];
          innercolumntotal[column] += value;
          rowtotal += value;
          rowdata += QString("<td>%1</td>").arg(value.formatMoney());

          ++column;
        }

        //
        // Row Total
        //
        if ( m_displayRowTotals )
          rowdata += QString("<td>%1</td>").arg(rowtotal.formatMoney());

        //
        // Row Header
        //

        AccountDescriptor rowname = it_row.key();

        innergroupdata += QString("<tr class=\"row-%1\"%2><td class=\"left\"%3>%4</td>")
          .arg(rownum & 0x01 ? "even" : "odd")
          .arg(rowname.isTopLevel() ? " id=\"topparent\"" : "")
          .arg(rowtotal == 0 ? colspan : "")
          .arg(rowname.htmlTabbedName());

        if ( rowtotal != 0 )
          innergroupdata += rowdata;

        innergroupdata += "</tr>\n";

        ++it_row;
      }

      //
      // Inner Row Group Totals
      //

      if ( showSubAccounts && ((*it_innergroup).size() > 1 ))
      {
        // Print the individual rows
        result += innergroupdata;

        // Print the TOTALS row
        result += QString("<tr class=\"row-%1\" id=\"subtotal\"><td class=\"left\">&nbsp;&nbsp;%2</td>")
          .arg(rownum & 0x01 ? "even" : "odd")
          .arg(i18n("Total"));
      }
      else
      {
        // this is an INDIVIDUAL ACCOUNT row
        AccountDescriptor rowname = (*it_innergroup).begin().key();
        result += QString("<tr class=\"row-%1\"%2><td class=\"left\">%3</td>")
          .arg(rownum & 0x01 ? "even" : "odd")
          .arg(showSubAccounts ? "id=\"solo\"" : "" )
          .arg(rowname.htmlTabbedName());
      }

      int column = 1;
      MyMoneyMoney innergrouptotal;
      while ( column < m_numColumns )
      {
        MyMoneyMoney value = innercolumntotal[column];
        columntotal[column] += value;
        innergrouptotal += value;
        result += QString("<td>%1</td>").arg(value.formatMoney());

        ++column;
      }

      if ( m_displayRowTotals )
        result += QString("<td>%1</td>").arg(innergrouptotal.formatMoney());

      result += "</tr>\n";

      ++rownum;
      ++it_innergroup;
    }

    //
    // Outer Row Group Totals
    //

    result += QString("<tr class=\"sectionfooter\"><td class=\"left\">%1&nbsp;%2</td>").arg(i18n("Total")).arg(it_outergroup.key());
    int column = 1;
    MyMoneyMoney outergrouptotal;
    while ( column < m_numColumns )
    {
      MyMoneyMoney value = columntotal[column];
      columngrandtotal[column] += value;
      outergrouptotal += value;
      result += QString("<td>%1</td>").arg(value.formatMoney());

      ++column;
    }

    if ( m_displayRowTotals )
      result += QString("<td>%1</td>").arg(outergrouptotal.formatMoney());

    result += "</tr>\n";

    ++it_outergroup;
  }

  //
  // Report Totals
  //

  result += QString("<tr class=\"spacer\"><td>&nbsp;</td></tr>\n");
  result += QString("<tr class=\"reportfooter\"><td class=\"left\">%1</td>").arg(i18n("Grand Total"));
  int totalcolumn = 1;
  MyMoneyMoney grandtotal;
  while ( totalcolumn < m_numColumns )
  {
    MyMoneyMoney value = columngrandtotal[totalcolumn];
    grandtotal += value;
    result += QString("<td>%1</td>").arg(value.formatMoney());

    ++totalcolumn;
  }

  if ( m_displayRowTotals )
    result += QString("<td>%1</td>").arg(grandtotal.formatMoney());

  result += "</tr>\n";

  result += QString("<tr class=\"spacer\"><td>&nbsp;</td></tr>\n");
  result += QString("<tr class=\"spacer\"><td>&nbsp;</td></tr>\n");
  result += "</table>\n";

  return result;
}

KReportsView::KReportsView(QWidget *parent, const char *name )
 : QWidget(parent,name)
{
  m_boolShowSubAccounts = true;

  m_qvboxlayoutPage = new QVBoxLayout(this);
  m_qvboxlayoutPage->setSpacing( 6 );
  m_qvboxlayoutPage->setMargin( 11 );

  m_part = new KHTMLPart(this, "htmlpart_km2");
  m_qvboxlayoutPage->addWidget(m_part->view());
  QString language = KGlobal::locale()->language();
  QString country = KGlobal::locale()->country();

  connect(m_part->browserExtension(), SIGNAL(openURLRequest(const KURL&, const KParts::URLArgs&)),
          this, SLOT(slotOpenURL(const KURL&, const KParts::URLArgs&)));
}

KReportsView::~KReportsView()
{
}

void KReportsView::show()
{
  slotRefreshView();
  QWidget::show();
  emit signalViewActivated();
}

const QString KReportsView::createTable(const QString& links) const
{
  DEBUG_ENTER("KReportsView::createTable()");
  
  QString filename = KGlobal::dirs()->findResource("appdata", "html/kmymoney2.css");
  QString header = QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\">\n") +
    QString("<html><head><link rel=\"stylesheet\" type=\"text/css\" href=\"%1\">").arg(filename);
  header += "</head><body>\n";

  QString footer = "</body></html>\n";

  int currentyear = QDate::currentDate().year();
  QValueList<MyMoneyAccount::accountTypeE> accounttypes;

  QString html;
  html += header;
  html += QString("<h2 class=\"report\">%1</h2>\n").arg(i18n("Monthly Income & Expenses"));
  html += QString("<div class=\"subtitle\">") + i18n("All currencies converted to %1")
                                .arg(MyMoneyFile::instance()->baseCurrency().name())
       + QString("</div>\n");
  html += QString("<div class=\"gap\">&nbsp;</div>\n");

  accounttypes.clear();
  accounttypes.append(MyMoneyAccount::Expense);
  accounttypes.append(MyMoneyAccount::Income);  
  PivotTable expensereport( MyMoneyFile::instance(), QDate(currentyear,1,1),QDate(currentyear+1,1,1),accounttypes, false );
  html += expensereport.renderHTML(m_boolShowSubAccounts);

  html += QString("<h2 class=\"report\">%1</h2>\n").arg(i18n("Net Worth Over Time"));
  html += QString("<div class=\"subtitle\">\n") + i18n("All currencies converted to %1")
                                .arg(MyMoneyFile::instance()->baseCurrency().name())
       + QString("</div>");
  html += QString("<div class=\"gap\">&nbsp;</div>\n");

  accounttypes.clear();
  accounttypes.append(MyMoneyAccount::Asset);
  accounttypes.append(MyMoneyAccount::Liability);
  PivotTable networthreport( MyMoneyFile::instance(), QDate(currentyear,1,1),QDate(currentyear+1,1,1),accounttypes, true );
  html += networthreport.renderHTML(m_boolShowSubAccounts);
    
  html += links;
  html += footer;

  return html;
}

void KReportsView::slotRefreshView(void)
{
  QString links;

  if ( m_boolShowSubAccounts )
    links += linkfull(VIEW_REPORTS, QString("?format=topparents"),i18n("Hide Subaccounts"));
  else
    links += linkfull(VIEW_REPORTS, QString("?format=subaccounts"),i18n("Show Subaccounts"));
  links += "&nbsp;|&nbsp;";
  links += linkfull(VIEW_REPORTS, QString("?command=copy"),i18n("Copy to Clipboard"));
  links += "&nbsp;|&nbsp;";
  links += linkfull(VIEW_REPORTS, QString("?command=save"),i18n("Save to File"));
  
  m_part->begin();
  m_part->write(createTable(links));
  m_part->end();
}

void KReportsView::slotPrintView(void)
{
  KHTMLPart part(this, "htmlpart_km2");

  part.begin();
  part.write(createTable());
  part.end();

  part.view()->print();
}

void KReportsView::slotCopyView(void)
{
  QTextDrag* pdrag =  new QTextDrag( createTable() );
  pdrag->setSubtype("html");
  QApplication::clipboard()->setData(pdrag);
}

void KReportsView::slotSaveView(void)
{
  QString newName=KFileDialog::getSaveFileName(KGlobalSettings::documentPath(),
                                               i18n("*.html|HTML files\n""*.*|All files"), this, i18n("Save as..."));

  if(!newName.isEmpty())
  {
    if(newName.findRev('.') == -1)
      newName.append(".html");

    QFile file( newName );
    if ( file.open( IO_WriteOnly ) ) {
      QTextStream stream( &file );
      stream <<  createTable();
      file.close();
    }
  }
}

const QString KReportsView::linkfull(const QString& view, const QString& query, const QString& label)
{
  return QString("<a href=\"/") + view + query + "\">" + label + "</a>";
}

void KReportsView::slotOpenURL(const KURL &url, const KParts::URLArgs& /* args */)
{
  QString view = url.fileName(false);
  QCString id = url.queryItem("id").data();
  QCString command = url.queryItem("command").data();
  QCString format = url.queryItem("format").data();
  
  if(view == VIEW_REPORTS) {

      if ( format == "subaccounts" )
        m_boolShowSubAccounts = true;
      else if ( format == "topparents" )
        m_boolShowSubAccounts = false;
      else
        qDebug("Unknown format '%s' in KReportsView::slotOpenURL()", static_cast<const char*>(format));

      if ( command.isEmpty() )
        slotRefreshView();
      else if ( command == "print" )
        slotPrintView();
      else if ( command == "copy" )
        slotCopyView();
      else if ( command == "save" )
        slotSaveView();
      else
        qDebug("Unknown command '%s' in KReportsView::slotOpenURL()", static_cast<const char*>(command));

  } else {
    qDebug("Unknown view '%s' in KReportsView::slotOpenURL()", view.latin1());
  }
}

