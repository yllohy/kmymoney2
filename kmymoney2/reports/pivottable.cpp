/***************************************************************************
                          pivottable.cpp
                             -------------------
    begin                : Mon May 17 2004
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
#include <qdom.h>

// ----------------------------------------------------------------------------
// KDE Includes
// This is just needed for i18n().  Once I figure out how to handle i18n
// without using this macro directly, I'll be freed of KDE dependency.  This
// is a minor problem because we use these terms when rendering to HTML,
// and a more major problem because we need it to translate account types
// (e.g. MyMoneyAccount::Checkings) into their text representation.  We also
// use that text representation in the core data structure of the report. (Ace)

#include <klocale.h>

// ----------------------------------------------------------------------------
// Project Includes
#include "pivottable.h"
#include "reportdebug.h"
#include "kreportchartview.h"

namespace reports {

const QString accountTypeToString(const MyMoneyAccount::accountTypeE accountType)
{
  QString returnString;

  switch (accountType)
  {
    case MyMoneyAccount::Checkings:
      returnString = i18n("Checkings");
      break;
    case MyMoneyAccount::Savings:
      returnString = i18n("Savings");
      break;
    case MyMoneyAccount::CreditCard:
      returnString = i18n("Credit Card");
      break;
    case MyMoneyAccount::Cash:
      returnString = i18n("Cash");
      break;
    case MyMoneyAccount::Loan:
      returnString = i18n("Loan");
      break;
    case MyMoneyAccount::CertificateDep:
      returnString = i18n("Certificate of Deposit");
      break;
    case MyMoneyAccount::Investment:
      returnString = i18n("Investment");
      break;
    case MyMoneyAccount::MoneyMarket:
      returnString = i18n("Money Market");
      break;
    case MyMoneyAccount::Asset:
      returnString = i18n("Asset");
      break;
    case MyMoneyAccount::Liability:
      returnString = i18n("Liability");
      break;
    case MyMoneyAccount::Currency:
      returnString = i18n("Currency");
      break;
    case MyMoneyAccount::Income:
      returnString = i18n("Income");
      break;
    case MyMoneyAccount::Expense:
      returnString = i18n("Expense");
      break;
    case MyMoneyAccount::AssetLoan:
      returnString = i18n("Investment Loan");
      break;
    case MyMoneyAccount::Stock:
      returnString = i18n("Stock");
      break;
    case MyMoneyAccount::Equity:
      returnString = i18n("Equity");
      break;
    default:
      returnString = i18n("Unknown");
  }

  return returnString;
}

QString Debug::m_sTabs;
bool Debug::m_sEnabled = DEBUG_ENABLED_BY_DEFAULT;
QString Debug::m_sEnableKey;

Debug::Debug( const QString& _name ): m_methodName( _name ), m_enabled( m_sEnabled )
{
  if (!m_enabled && _name == m_sEnableKey)
    m_enabled = true;

  if (m_enabled)
  {
    qDebug( "%s%s(): ENTER", m_sTabs.latin1(), m_methodName.latin1() );
    m_sTabs.append("--");
  }
}

Debug::~Debug()
{
  if ( m_enabled )
  {
    m_sTabs.remove(0,2);
    qDebug( "%s%s(): EXIT", m_sTabs.latin1(), m_methodName.latin1() );

    if (m_methodName == m_sEnableKey)
      m_enabled = false;
  }
}

void Debug::output( const QString& _text )
{
  if ( m_enabled )
    qDebug( "%s%s(): %s", m_sTabs.latin1(), m_methodName.latin1(), _text.latin1() );
}

//////////////////////////////////////////////////////////////////////

PivotTable::PivotTable( const MyMoneyReport& _config_f ):
  m_config_f( _config_f )
{
  DEBUG_ENTER(__PRETTY_FUNCTION__);

  //
  // Initialize locals
  //

  MyMoneyFile* file = MyMoneyFile::instance();

  //
  // Initialize member variables
  //

  m_config_f.validDateRange( m_beginDate, m_endDate );

  // strip out the 'days' component of the begin and end dates.
  // we're only using these variables to contain year and month.
  m_beginDate =  QDate( m_beginDate.year(), m_beginDate.month(), 1 );
  m_endDate = QDate( m_endDate.year(), m_endDate.month(), 1 );

  m_numColumns = m_endDate.year() * 12 + m_endDate.month() - m_beginDate.year() * 12 - m_beginDate.month() + 2;

  //
  // Get opening balances
  // (for running sum reports only)
  //

  if ( m_config_f.isRunningSum() )
    calculateOpeningBalances();

  //
  // Populate all transactions into the row/column pivot grid
  //

  m_config_f.setReportAllSplits(false);
  m_config_f.setConsiderCategory(true);
  QValueList<MyMoneyTransaction> transactions = file->transactionList(m_config_f);
  QValueList<MyMoneyTransaction>::const_iterator it_transaction = transactions.begin();
  unsigned colofs = m_beginDate.year() * 12 + m_beginDate.month() - 1;

  DEBUG_OUTPUT(QString("Found %1 matching transactions").arg(transactions.count()));
  while ( it_transaction != transactions.end() )
  {
    QDate postdate = (*it_transaction).postDate();
    unsigned column = postdate.year() * 12 + postdate.month() - colofs;

    QValueList<MyMoneySplit> splits = (*it_transaction).splits();
    QValueList<MyMoneySplit>::const_iterator it_split = splits.begin();
    while ( it_split != splits.end() )
    {
      ReportAccount splitAccount = (*it_split).accountId();

      // Each split must be further filtered, because if even one split matches,
      // the ENTIRE transaction is returned with all splits (even non-matching ones)
      if ( m_config_f.includes( splitAccount ) )
      {
        // reverse sign to match common notation for cash flow direction, only for expense/income splits
        MyMoneyMoney reverse(splitAccount.isIncomeExpense() ? -1 : 1, 1);

        // retrieve the value in the account's underlying currency
        MyMoneyMoney value = (*it_split).shares() * reverse;

        // the outer group is the account class (major account type)
        QString outergroup = accountTypeToString(splitAccount.accountGroup());

        // add the value to its correct position in the pivot table
        assignCell( outergroup, splitAccount, column, value );

      }
      ++it_split;
    }

    ++it_transaction;
  }

  //
  // Collapse columns to match column type
  //

  if ( m_config_f.columnType() != MyMoneyReport::eMonths )
    collapseColumns();

  //
  // Calculate the running sums
  // (for running sum reports only)
  //

  if ( m_config_f.isRunningSum() )
    calculateRunningSums();

  //
  // Convert all values to the deep currency
  //

  convertToDeepCurrency();

  //
  // Convert all values to the base currency
  //

  if ( m_config_f.isConvertCurrency() )
    convertToBaseCurrency();

  //
  // Determine column headings
  //

  calculateColumnHeadings();

  //
  // Calculate row and column totals
  //

  calculateTotals();

}

void PivotTable::collapseColumns(void)
{
  DEBUG_ENTER(__PRETTY_FUNCTION__);

  unsigned columnpitch = m_config_f.columnPitch();
  if ( columnpitch != 1 )
  {
    unsigned sourcemonth = m_beginDate.year() * 12 + m_beginDate.month();
    unsigned sourcecolumn = 1;
    unsigned destcolumn = 1;
    while ( sourcecolumn < m_numColumns )
    {
      if ( sourcecolumn != destcolumn )
      {
#if 0
        // TODO: Clean up this rather inefficient kludge. We really should jump by an entire
        // destcolumn at a time on RS reports, and calculate the proper sourcecolumn to use,
        // allowing us to clear and accumulate only ONCE per destcolumn
        if ( m_config_f.isRunningSum() )
          clearColumn(destcolumn);
#endif
        accumulateColumn(destcolumn,sourcecolumn);
      }

      ++sourcecolumn;

      // increment the source month, and test if it crosses a segment boundary AND it's still a valid column
      if ( ( ((sourcemonth-1)/columnpitch) != ((++sourcemonth-1)/columnpitch) ) && sourcecolumn < m_numColumns )
      {
        ++destcolumn;
        if ( sourcecolumn != destcolumn )
          clearColumn(destcolumn);
      }
    }
    m_numColumns = destcolumn + 1;
  }
}

void PivotTable::accumulateColumn(unsigned destcolumn, unsigned sourcecolumn)
{
  DEBUG_ENTER(__PRETTY_FUNCTION__);
  DEBUG_OUTPUT(QString("From Column %1 to %2").arg(sourcecolumn).arg(destcolumn));

  // iterate over outer groups
  TGrid::iterator it_outergroup = m_grid.begin();
  while ( it_outergroup != m_grid.end() )
  {
    // iterate over inner groups
    TOuterGroup::iterator it_innergroup = (*it_outergroup).begin();
    while ( it_innergroup != (*it_outergroup).end() )
    {
      // iterator over rows
      TInnerGroup::iterator it_row = (*it_innergroup).begin();
      while ( it_row != (*it_innergroup).end() )
      {
        if ( (*it_row).count() <= sourcecolumn )
          throw new MYMONEYEXCEPTION(QString("Sourcecolumn %1 out of grid range (%2) in PivotTable::accumulateColumn").arg(sourcecolumn).arg((*it_row).count()));
        if ( (*it_row).count() <= destcolumn )
          throw new MYMONEYEXCEPTION(QString("Destcolumn %1 out of grid range (%2) in PivotTable::accumulateColumn").arg(sourcecolumn).arg((*it_row).count()));

        (*it_row)[destcolumn] += (*it_row)[sourcecolumn];
        ++it_row;
      }

      ++it_innergroup;
    }
    ++it_outergroup;
  }
}

void PivotTable::clearColumn(unsigned column)
{
  DEBUG_ENTER(__PRETTY_FUNCTION__);
  DEBUG_OUTPUT(QString("Column %1").arg(column));

  // iterate over outer groups
  TGrid::iterator it_outergroup = m_grid.begin();
  while ( it_outergroup != m_grid.end() )
  {
    // iterate over inner groups
    TOuterGroup::iterator it_innergroup = (*it_outergroup).begin();
    while ( it_innergroup != (*it_outergroup).end() )
    {
      // iterator over rows
      TInnerGroup::iterator it_row = (*it_innergroup).begin();
      while ( it_row != (*it_innergroup).end() )
      {
        if ( (*it_row).count() <= column )
          throw new MYMONEYEXCEPTION(QString("Column %1 out of grid range (%2) in PivotTable::accumulateColumn").arg(column).arg((*it_row).count()));

        (*it_row++)[column] = MyMoneyMoney(0,1);
      }

      ++it_innergroup;
    }
    ++it_outergroup;
  }
}

void PivotTable::calculateColumnHeadings(void)
{
  DEBUG_ENTER(__PRETTY_FUNCTION__);

  // one column for the opening balance
  m_columnHeadings.append( "Opening" );

  unsigned columnpitch = m_config_f.columnPitch();
  if ( columnpitch == 12 )
  {
    unsigned year = m_beginDate.year();
    unsigned column = 1;
    while ( column++ < m_numColumns )
      m_columnHeadings.append(QString::number(year++));
  }
  else
  {
    unsigned year = m_beginDate.year();
    bool includeyear = ( m_beginDate.year() != m_endDate.year() );
    unsigned segment = ( m_beginDate.month() - 1 ) / columnpitch;
    unsigned column = 1;
    while ( column++ < m_numColumns )
    {
      QString heading = QDate::shortMonthName(1+segment*columnpitch);
      if ( columnpitch != 1 )
        heading += "-" + QDate::shortMonthName((1+segment)*columnpitch);
      if ( includeyear )
        heading += " " + QString::number(year);
      m_columnHeadings.append( heading);
      if ( ++segment >= 12/columnpitch )
      {
        segment -= 12/columnpitch;
        ++year;
      }
    }
  }
}

void PivotTable::calculateOpeningBalances( void )
{
  DEBUG_ENTER(__PRETTY_FUNCTION__);

  // First, determine the inclusive dates of the report.  Normally, that's just
  // the begin & end dates of m_config_f.  However, if either of those dates are
  // blank, we need to use m_beginDate and/or m_endDate instead.
  QDate from = m_config_f.fromDate();
  QDate to = m_config_f.toDate();
  if ( ! from.isValid() )
    from = m_beginDate;
  if ( ! to.isValid() )
    to = m_endDate;

  MyMoneyFile* file = MyMoneyFile::instance();

  const QValueList<MyMoneyAccount>& accounts = file->accountList();

  QValueList<MyMoneyAccount>::const_iterator it_account = accounts.begin();

  while ( it_account != accounts.end() )
  {
    ReportAccount account = *it_account;

    // only include this item if its account group is included in this report
    // and if the report includes this account
    if ( m_config_f.includes( *it_account ) )
    {
      DEBUG_OUTPUT(QString("Includes account %1").arg(account.name()));

      // the row group is the account class (major account type)
      QString outergroup = accountTypeToString(account.accountGroup());

      // extract the balance of the account for the given begin date, which is
      // the opening balance plus the sum of all transactions prior to the begin
      // date

      // this is in the underlying currency
      MyMoneyMoney value = file->balance(account.id(), from.addDays(-1));

      // remove the opening balance from the figure, if necessary
      QDate opendate = account.openingDate();
      if ( opendate >= from )
        value -= account.openingBalance();

      // place into the 'opening' column...
      assignCell( outergroup, account, 0, value );

      if ( ( opendate >= from ) && ( opendate <= to ) )
      {
        // get the opening value
        MyMoneyMoney value = account.openingBalance();
        // place in the correct column
        unsigned column = opendate.year() * 12 + opendate.month() - m_beginDate.year() * 12 - m_beginDate.month() + 1;
        assignCell( outergroup, account, column, value );
      }
    }
    else
    {
      DEBUG_OUTPUT(QString("DOES NOT INCLUDE account %1").arg(account.name()));
    }

    ++it_account;
  }
}


void PivotTable::calculateRunningSums( void )
{
  DEBUG_ENTER(__PRETTY_FUNCTION__);

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
        unsigned column = 1;
        while ( column < m_numColumns )
        {
          if ( it_row.data().count() <= column )
            throw new MYMONEYEXCEPTION(QString("Column %1 out of grid range (%2) in PivotTable::calculateRunningSums").arg(column).arg(it_row.data().count()));

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
  DEBUG_ENTER(__PRETTY_FUNCTION__);

  TGrid::iterator it_outergroup = m_grid.begin();
  while ( it_outergroup != m_grid.end() )
  {
    TOuterGroup::iterator it_innergroup = (*it_outergroup).begin();
    while ( it_innergroup != (*it_outergroup).end() )
    {
      TInnerGroup::iterator it_row = (*it_innergroup).begin();
      while ( it_row != (*it_innergroup).end() )
      {
        QDate valuedate = m_beginDate.addMonths( m_config_f.columnPitch() ).addDays(-1);
        unsigned column = 1;
        while ( column < m_numColumns )
        {
          if ( it_row.data().count() <= column )
            throw new MYMONEYEXCEPTION(QString("Column %1 out of grid range (%2) in PivotTable::convertToBaseCurrency").arg(column).arg(it_row.data().count()));

          double conversionfactor = it_row.key().baseCurrencyPrice(valuedate).toDouble();
          double oldval = it_row.data()[column].toDouble();
          double value = oldval * conversionfactor;
          it_row.data()[column] = MyMoneyMoney( value );

          DEBUG_OUTPUT_IF(conversionfactor != 1.0 ,QString("Factor of %1, value was %2, now %3").arg(conversionfactor).arg(DEBUG_SENSITIVE(oldval)).arg(DEBUG_SENSITIVE(it_row.data()[column].toDouble())));

          // Move to the end of the next period
          valuedate = valuedate.addDays(1).addMonths( m_config_f.columnPitch() ).addDays(-1);

          ++column;
        }
        ++it_row;
      }
      ++it_innergroup;
    }
    ++it_outergroup;
  }
}

void PivotTable::convertToDeepCurrency( void )
{
  DEBUG_ENTER(__PRETTY_FUNCTION__);

  TGrid::iterator it_outergroup = m_grid.begin();
  while ( it_outergroup != m_grid.end() )
  {
    TOuterGroup::iterator it_innergroup = (*it_outergroup).begin();
    while ( it_innergroup != (*it_outergroup).end() )
    {
      TInnerGroup::iterator it_row = (*it_innergroup).begin();
      while ( it_row != (*it_innergroup).end() )
      {
        QDate valuedate = m_beginDate.addMonths( m_config_f.columnPitch() ).addDays(-1);
        unsigned column = 1;
        while ( column < m_numColumns )
        {
          if ( it_row.data().count() <= column )
            throw new MYMONEYEXCEPTION(QString("Column %1 out of grid range (%2) in PivotTable::convertToDeepCurrency").arg(column).arg(it_row.data().count()));

          double conversionfactor = it_row.key().deepCurrencyPrice(valuedate).toDouble();
          double oldval = it_row.data()[column].toDouble();
          double value = oldval * conversionfactor;
          it_row.data()[column] = MyMoneyMoney( value );

          DEBUG_OUTPUT_IF(conversionfactor != 1.0 ,QString("Factor of %1, value was %2, now %3").arg(conversionfactor).arg(DEBUG_SENSITIVE(oldval)).arg(DEBUG_SENSITIVE(it_row.data()[column].toDouble())));

          // Move to the end of the next period
          valuedate = valuedate.addDays(1).addMonths( m_config_f.columnPitch() ).addDays(-1);

          ++column;
        }
        ++it_row;
      }
      ++it_innergroup;
    }
    ++it_outergroup;
  }
}

void PivotTable::calculateTotals( void )
{
  m_grid.m_total.insert( m_grid.m_total.end(), m_numColumns, MyMoneyMoney() );

  //
  // Outer groups
  //

  // iterate over outer groups
  TGrid::iterator it_outergroup = m_grid.begin();
  while ( it_outergroup != m_grid.end() )
  {
    (*it_outergroup).m_total.insert( (*it_outergroup).m_total.end(), m_numColumns, MyMoneyMoney() );

    //
    // Inner Groups
    //

    TOuterGroup::iterator it_innergroup = (*it_outergroup).begin();
    while ( it_innergroup != (*it_outergroup).end() )
    {
      (*it_innergroup).m_total.insert( (*it_innergroup).m_total.end(), m_numColumns, MyMoneyMoney() );

      //
      // Rows
      //

      TInnerGroup::iterator it_row = (*it_innergroup).begin();
      while ( it_row != (*it_innergroup).end() )
      {
        //
        // Columns
        //

        unsigned column = 0;
        while ( column < m_numColumns )
        {
          if ( it_row.data().count() <= column )
            throw new MYMONEYEXCEPTION(QString("Column %1 out of grid range (%2) in PivotTable::calculateTotals, row columns").arg(column).arg(it_row.data().count()));
          if ( (*it_innergroup).m_total.count() <= column )
            throw new MYMONEYEXCEPTION(QString("Column %1 out of grid range (%2) in PivotTable::calculateTotals, inner group totals").arg(column).arg((*it_innergroup).m_total.count()));

          MyMoneyMoney value = it_row.data()[column];
          (*it_innergroup).m_total[column] += value;
          (*it_row).m_total += value;

          ++column;
        }
        ++it_row;
      }

      //
      // Inner Row Group Totals
      //

      unsigned column = 0;
      while ( column < m_numColumns )
      {
        if ( (*it_innergroup).m_total.count() <= column )
          throw new MYMONEYEXCEPTION(QString("Column %1 out of grid range (%2) in PivotTable::calculateTotals, inner group totals").arg(column).arg((*it_innergroup).m_total.count()));
        if ( (*it_outergroup).m_total.count() <= column )
          throw new MYMONEYEXCEPTION(QString("Column %1 out of grid range (%2) in PivotTable::calculateTotals, outer group totals").arg(column).arg((*it_innergroup).m_total.count()));

        MyMoneyMoney value = (*it_innergroup).m_total[column];
        (*it_outergroup).m_total[column] += value;
        (*it_innergroup).m_total.m_total += value;

        ++column;
      }

      ++it_innergroup;
    }

    //
    // Outer Row Group Totals
    //

    unsigned column = 0;
    while ( column < m_numColumns )
    {
      if ( m_grid.m_total.count() <= column )
        throw new MYMONEYEXCEPTION(QString("Column %1 out of grid range (%2) in PivotTable::calculateTotals, grid totals").arg(column).arg((*it_innergroup).m_total.count()));

      MyMoneyMoney value = (*it_outergroup).m_total[column];
      m_grid.m_total[column] += value;
      (*it_outergroup).m_total.m_total += value;

      ++column;
    }

    ++it_outergroup;
  }

  //
  // Report Totals
  //

  unsigned totalcolumn = 0;
  while ( totalcolumn < m_numColumns )
  {
    if ( m_grid.m_total.count() <= totalcolumn )
      throw new MYMONEYEXCEPTION(QString("Total column %1 out of grid range (%2) in PivotTable::calculateTotals, grid totals").arg(totalcolumn).arg(m_grid.m_total.count()));

    MyMoneyMoney value = m_grid.m_total[totalcolumn];
    m_grid.m_total.m_total += value;

    ++totalcolumn;
  }

}

void PivotTable::assignCell( const QString& outergroup, const ReportAccount& row, unsigned column, MyMoneyMoney value )
{
  DEBUG_ENTER(__PRETTY_FUNCTION__);
  DEBUG_OUTPUT(QString("Parameters: %1,%2,%3,%4").arg(outergroup).arg(row.debugName()).arg(column).arg(DEBUG_SENSITIVE(value.toDouble())));

  // ensure the row already exists (and its parental hierarchy)
  createRow( outergroup, row, true );

  // Determine the inner group from the top-most parent account
  QString innergroup( row.topParentName() );

  if ( m_numColumns <= column )
    throw new MYMONEYEXCEPTION(QString("Column %1 out of m_numColumns range (%2) in PivotTable::assignCell").arg(column).arg(m_numColumns));
  if ( m_grid[outergroup][innergroup][row].count() <= column )
    throw new MYMONEYEXCEPTION(QString("Column %1 out of grid range (%2) in PivotTable::assignCell").arg(column).arg(m_grid[outergroup][innergroup][row].count()));

  // Add the value to the grid cell
  m_grid[outergroup][innergroup][row][column] += value;

}

void PivotTable::createRow( const QString& outergroup, const ReportAccount& row, bool recursive )
{
  DEBUG_ENTER(__PRETTY_FUNCTION__);

  // Determine the inner group from the top-most parent account
  QString innergroup( row.topParentName() );

  // fill the row list with blanks if it doesn't already exist.
  if ( m_grid[outergroup][innergroup][row].isEmpty() )
  {
    DEBUG_OUTPUT(QString("m_grid[%1][%2][%3].insert(%4)").arg(outergroup).arg(innergroup).arg(row.debugName()).arg(m_numColumns));

    m_grid[outergroup][innergroup][row].insert( m_grid[outergroup][innergroup][row].end(), m_numColumns, 0 );

    m_grid[outergroup][innergroup].m_total.insert( m_grid[outergroup][innergroup].m_total.end(), m_numColumns, 0 );
    m_grid[outergroup].m_total.insert( m_grid[outergroup].m_total.end(), m_numColumns, 0 );
    m_grid.m_total.insert( m_grid.m_total.end(), m_numColumns, 0 );

    if ( recursive && !row.isTopLevel() )
        createRow( outergroup, row.parent(), recursive );
  }
}

QString PivotTable::renderCSV( void ) const
{
  DEBUG_ENTER(__PRETTY_FUNCTION__);

  char saveseparator = MyMoneyMoney::thousandSeparator();
  MyMoneyMoney::setThousandSeparator('\0');

  //
  // Report Title
  //

  QString result = QString("\"Report: %1\"\n").arg(m_config_f.name());
  if ( m_config_f.isConvertCurrency() )
    result += i18n("All currencies converted to %1\n").arg(MyMoneyFile::instance()->baseCurrency().name());
  else
    result += i18n("All values shown in %1 unless otherwise noted\n").arg(MyMoneyFile::instance()->baseCurrency().name());

  //
  // Table Header
  //

  result += i18n("Account");

  unsigned column = 1;
  while ( column < m_numColumns )
    result += QString(",%1").arg(QString(m_columnHeadings[column++]));

  if ( m_config_f.isShowingRowTotals() )
    result += QString(",%1").arg(i18n("Total"));

  result += "\n";

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

    result += it_outergroup.key() + "\n";

    //
    // Inner Groups
    //

    TOuterGroup::const_iterator it_innergroup = (*it_outergroup).begin();
    unsigned rownum = 0;
    while ( it_innergroup != (*it_outergroup).end() )
    {
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
        unsigned column = 1;
        while ( column < m_numColumns )
          rowdata += QString(",\"%1\"").arg(it_row.data()[column++].formatMoney());

        if ( m_config_f.isShowingRowTotals() )
          rowdata += QString(",\"%1\"").arg((*it_row).m_total.formatMoney());

        //
        // Row Header
        //

        ReportAccount rowname = it_row.key();

        innergroupdata += "\"" + QString().fill(' ',rowname.hierarchyDepth() - 1) + rowname.name();

        if (m_config_f.isConvertCurrency() || !rowname.isForiegnCurrency() )
          innergroupdata += QString(" (%1)").arg(rowname.currencyId());

        innergroupdata += "\"";

        if ( ! (*it_row).m_total.isZero() )
          innergroupdata += rowdata;

        innergroupdata += "\n";

        ++it_row;
      }

      //
      // Inner Row Group Totals
      //

      bool finishrow = true;
      if ( m_config_f.isShowingSubAccounts() && ((*it_innergroup).size() > 1 ))
      {
        // Print the individual rows
        result += innergroupdata;

        if ( m_config_f.isShowingColumnTotals() )
        {
          // Start the TOTALS row
          result += i18n("Total");
        }
        else
          finishrow = false;
      }
      else
      {
        // Start the single INDIVIDUAL ACCOUNT row
        ReportAccount rowname = (*it_innergroup).begin().key();

        result += "\"" + QString().fill(' ',rowname.hierarchyDepth() - 1) + rowname.name();
        if (m_config_f.isConvertCurrency() || !rowname.isForiegnCurrency() )
          result += QString(" (%1)").arg(rowname.currencyId());
        result += "\"";

      }

      // Finish the row started above, unless told not to
      if ( finishrow )
      {
        unsigned column = 1;
        while ( column < m_numColumns )
          result += QString(",\"%1\"").arg((*it_innergroup).m_total[column++].formatMoney());

        if (  m_config_f.isShowingRowTotals() )
          result += QString(",\"%1\"").arg((*it_innergroup).m_total.m_total.formatMoney());

        result += "\n";
      }

      ++rownum;
      ++it_innergroup;
    }

    //
    // Outer Row Group Totals
    //

    if ( m_config_f.isShowingColumnTotals() )
    {
      result += QString("%1 %2").arg(i18n("Total")).arg(it_outergroup.key());
      unsigned column = 1;
      while ( column < m_numColumns )
        result += QString(",\"%1\"").arg((*it_outergroup).m_total[column++].formatMoney());

      if (  m_config_f.isShowingRowTotals() )
        result += QString(",\"%1\"").arg((*it_outergroup).m_total.m_total.formatMoney());

      result += "\n";
    }
    ++it_outergroup;
  }

  //
  // Report Totals
  //

  if ( m_config_f.isShowingColumnTotals() )
  {
    result += i18n("Grand Total");
    unsigned totalcolumn = 1;
    while ( totalcolumn < m_numColumns )
      result += QString(",\"%1\"").arg(m_grid.m_total[totalcolumn++].formatMoney());

    if (  m_config_f.isShowingRowTotals() )
      result += QString(",\"%1\"").arg(m_grid.m_total.m_total.formatMoney());

    result += "\n";
  }

  MyMoneyMoney::setThousandSeparator(saveseparator);

  return result;
}

QString PivotTable::renderHTML( void ) const
{
  DEBUG_ENTER(__PRETTY_FUNCTION__);

  QString colspan = QString(" colspan=\"%1\"").arg(m_numColumns + 1 + (m_config_f.isShowingRowTotals() ? 1 : 0) );

  //
  // Report Title
  //

  QString result = QString("<h2 class=\"report\">%1</h2>\n").arg(m_config_f.name());
  result += QString("<div class=\"subtitle\">");
  if ( m_config_f.isConvertCurrency() )
    result += i18n("All currencies converted to %1").arg(MyMoneyFile::instance()->baseCurrency().name());
  else
    result += i18n("All values shown in %1 unless otherwise noted").arg(MyMoneyFile::instance()->baseCurrency().name());
  result += QString("</div>\n");
  result += QString("<div class=\"gap\">&nbsp;</div>\n");

  //
  // Table Header
  //

  result += QString("\n\n<table class=\"report\" cellspacing=\"0\">"
      "<tr class=\"itemheader\">\n<th>%1</th>").arg(i18n("Account"));

  unsigned column = 1;
  while ( column < m_numColumns )
    result += QString("<th>%1</th>").arg(QString(m_columnHeadings[column++]).replace(QRegExp(" "),"<br>"));

  if ( m_config_f.isShowingRowTotals() )
    result += QString("<th>%1</th>").arg(i18n("Total"));

  result += "</tr>\n";

  // Skip the body of the report if the report only calls for totals to be shown
  if ( m_config_f.detailLevel() != MyMoneyReport::eDetailTotal )
  {
  
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
  
      // Skip the inner groups if the report only calls for outer group totals to be shown
      if ( m_config_f.detailLevel() != MyMoneyReport::eDetailGroup )
      {
      
        //
        // Inner Groups
        //
    
        TOuterGroup::const_iterator it_innergroup = (*it_outergroup).begin();
        unsigned rownum = 0;
        while ( it_innergroup != (*it_outergroup).end() )
        {
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
            unsigned column = 1;
            while ( column < m_numColumns )
              rowdata += QString("<td>%1</td>").arg(it_row.data()[column++].formatMoney());
    
            if ( m_config_f.isShowingRowTotals() )
              rowdata += QString("<td>%1</td>").arg((*it_row).m_total.formatMoney());
    
            //
            // Row Header
            //
    
            ReportAccount rowname = it_row.key();
    
            innergroupdata += QString("<tr class=\"row-%1\"%2><td%3 class=\"left%4\">%5%6</td>")
              .arg(rownum & 0x01 ? "even" : "odd")
              .arg(rowname.isTopLevel() ? " id=\"topparent\"" : "")
              .arg((*it_row).m_total.isZero() ? colspan : "")
              .arg(rowname.hierarchyDepth() - 1)
              .arg(rowname.name().replace(QRegExp(" "), "&nbsp;"))
              .arg((m_config_f.isConvertCurrency() || !rowname.isForiegnCurrency() )?QString():QString(" (%1)").arg(rowname.currency()));
    
            if ( !(*it_row).m_total.isZero() )
              innergroupdata += rowdata;
    
            innergroupdata += "</tr>\n";
    
            ++it_row;
          }
    
          //
          // Inner Row Group Totals
          //
    
          bool finishrow = true;
          if ( m_config_f.isShowingSubAccounts() && ((*it_innergroup).size() > 1 ))
          {
            // Print the individual rows
            result += innergroupdata;
    
            if ( m_config_f.isShowingColumnTotals() )
            {
              // Start the TOTALS row
              result += QString("<tr class=\"row-%1\" id=\"subtotal\"><td class=\"left\">&nbsp;&nbsp;%2</td>")
                .arg(rownum & 0x01 ? "even" : "odd")
                .arg(i18n("Total"));
            }
            else
              finishrow = false;
          }
          else
          {
            // Start the single INDIVIDUAL ACCOUNT row
            // FIXME: There is a bit of a bug here with class=leftX.  There's only a finite number
            // of classes I can define in the .CSS file, and the user can theoretically nest deeper.
            // The right solution is to use style=Xem, and calculate X.  Let's see if anyone complains
            // first :)  Also applies to the row header case above.
            ReportAccount rowname = (*it_innergroup).begin().key();
            result += QString("<tr class=\"row-%1\"%2><td class=\"left%3\">%4%5</td>")
              .arg(rownum & 0x01 ? "even" : "odd")
              .arg( m_config_f.isShowingSubAccounts() ? "id=\"solo\"" : "" )
              .arg(rowname.hierarchyDepth() - 1)
              .arg(rowname.name().replace(QRegExp(" "), "&nbsp;"))
              .arg((m_config_f.isConvertCurrency() || !rowname.isForiegnCurrency() )?QString():QString(" (%1)").arg(rowname.currency()));
          }
    
          // Finish the row started above, unless told not to
          if ( finishrow )
          {
            unsigned column = 1;
            while ( column < m_numColumns )
              result += QString("<td>%1</td>").arg((*it_innergroup).m_total[column++].formatMoney());
    
            if (  m_config_f.isShowingRowTotals() )
              result += QString("<td>%1</td>").arg((*it_innergroup).m_total.m_total.formatMoney());
    
            result += "</tr>\n";
          }
    
          ++rownum;
          ++it_innergroup;
          
        } // end while iterating on the inner groups
        
      } // end if detail level is not "group"
  
      //
      // Outer Row Group Totals
      //
  
      if ( m_config_f.isShowingColumnTotals() )
      {
        result += QString("<tr class=\"sectionfooter\"><td class=\"left\">%1&nbsp;%2</td>").arg(i18n("Total")).arg(it_outergroup.key());
        unsigned column = 1;
        while ( column < m_numColumns )
          result += QString("<td>%1</td>").arg((*it_outergroup).m_total[column++].formatMoney());
  
        if (  m_config_f.isShowingRowTotals() )
          result += QString("<td>%1</td>").arg((*it_outergroup).m_total.m_total.formatMoney());
  
        result += "</tr>\n";
      }
      
      ++it_outergroup;
      
    } // end while iterating on the outergroups
    
  } // end if detail level is not "total"
  
  //
  // Report Totals
  //

  if ( m_config_f.isShowingColumnTotals() )
  {
    result += QString("<tr class=\"spacer\"><td>&nbsp;</td></tr>\n");
    result += QString("<tr class=\"reportfooter\"><td class=\"left\">%1</td>").arg(i18n("Grand Total"));
    unsigned totalcolumn = 1;
    while ( totalcolumn < m_numColumns )
      result += QString("<td>%1</td>").arg(m_grid.m_total[totalcolumn++].formatMoney());

    if (  m_config_f.isShowingRowTotals() )
      result += QString("<td>%1</td>").arg(m_grid.m_total.m_total.formatMoney());

    result += "</tr>\n";
  }

  result += QString("<tr class=\"spacer\"><td>&nbsp;</td></tr>\n");
  result += QString("<tr class=\"spacer\"><td>&nbsp;</td></tr>\n");
  result += "</table>\n";

  return result;
}

void PivotTable::dump( const QString& file ) const
{
  QFile g( file );
  g.open( IO_WriteOnly );
  QTextStream(&g) << renderHTML();
  g.close();
}

void PivotTable::drawChart( KReportChartView& _view ) const
{
#ifdef HAVE_KDCHART

  _view.params().setAxisShowGrid(0,m_config_f.isChartGridLines());
  _view.params().setAxisShowGrid(1,m_config_f.isChartGridLines());
  _view.params().setPrintDataValues(m_config_f.isChartDataLabels());
  
  // whether to limit the chart to use series totals only.  Used for reports which only
  // show one dimension (pie).
  bool seriestotals = false;
  
  // whether series (rows) are accounts (true) or months (false). This causes a lot
  // of complexity in the charts.  The problem is that circular reports work best with
  // an account in a COLUMN, while line/bar prefer it in a ROW.  
  bool accountseries = true;
  
  switch( m_config_f.chartType() )
  {
  case MyMoneyReport::eChartNone:
  case MyMoneyReport::eChartEnd:
  case MyMoneyReport::eChartLine:
    _view.params().setChartType( KDChartParams::Line );
    _view.params().setAxisDatasets( 0,0 );
    break;
  case MyMoneyReport::eChartBar:
    _view.params().setChartType( KDChartParams::Bar );
    _view.params().setBarChartSubType( KDChartParams::BarNormal );
    break;
  case MyMoneyReport::eChartStackedBar:
    _view.params().setChartType( KDChartParams::Bar );
    _view.params().setBarChartSubType( KDChartParams::BarStacked );
    break;
  case MyMoneyReport::eChartPie:
    _view.params().setChartType( KDChartParams::Pie );
    _view.params().setThreeDPies( true );
    accountseries = false;
    seriestotals = true;
    break;
  case MyMoneyReport::eChartRing:
    _view.params().setChartType( KDChartParams::Ring );
    _view.params().setRelativeRingThickness( true );
    accountseries = false;
    break;
  }
  
  //
  // In KDChart parlance, a 'series' (or row) is an account (or accountgroup, etc)
  // and an 'item' (or column) is a month
  //
  unsigned r;
  unsigned c;
  if ( accountseries )
  {
    r = 1;
    c = m_numColumns;
  }
  else
  {
    c = 1;
    r = m_numColumns;
  }
  KDChartTableData data( r,c );
  
  // Set up X axis labels (ie "abscissa" to use the technical term)
  QStringList& abscissaNames = _view.abscissaNames();
  abscissaNames.clear();
  if ( accountseries )
  {
    unsigned column = 1;
    while ( column < m_numColumns )
      abscissaNames += m_columnHeadings[column++];
  }
  else
  {
    // we will set these up while putting in the chart values.
  }
  
  switch ( m_config_f.detailLevel() )
  {
    case MyMoneyReport::eDetailNone:
    case MyMoneyReport::eDetailEnd:
    case MyMoneyReport::eDetailAll:
    {
      unsigned rownum = 1;  
      
      // iterate over outer groups
      TGrid::const_iterator it_outergroup = m_grid.begin();
      while ( it_outergroup != m_grid.end() )
      {
    
        // iterate over inner groups
        TOuterGroup::const_iterator it_innergroup = (*it_outergroup).begin();
        while ( it_innergroup != (*it_outergroup).end() )
        {
          //
          // Rows
          //
    
          QString innergroupdata;
          TInnerGroup::const_iterator it_row = (*it_innergroup).begin();
          while ( it_row != (*it_innergroup).end() )
          {
            if ( accountseries )
              _view.params().setLegendText( rownum-1, it_row.key().name() );
            else
              abscissaNames += it_row.key().name();
            
            //
            // Columns
            //
    
            if ( seriestotals )
            {
                if ( accountseries )
                  data.setCell( rownum-1, 0, it_row.data().m_total.toDouble() );
                else
                  data.setCell( 0, rownum-1, it_row.data().m_total.toDouble() );
            }
            else
            {
              unsigned column = 1;
              while ( column < m_numColumns )
              {
                if ( accountseries )
                  data.setCell( rownum-1, column-1, it_row.data()[column].toDouble() );
                else
                  data.setCell( column-1, rownum-1, it_row.data()[column].toDouble() );
                ++column;
              }
            }            
            // TODO: This is inefficient. Really we should total up how many rows
            // there will be and allocate it all at once.
            if ( accountseries )
              data.expand( ++rownum, m_numColumns );
            else
              data.expand( m_numColumns, ++rownum );
    
            ++it_row;
          }
          ++it_innergroup;
        }
        ++it_outergroup;  
      }
      
      if ( accountseries )
        data.expand( rownum-1, m_numColumns );
      else
        data.expand( m_numColumns, rownum-1 );
      
    }
    break;    
        
    case MyMoneyReport::eDetailTop:
    {
      unsigned rownum = 1;  
      
      // iterate over outer groups
      TGrid::const_iterator it_outergroup = m_grid.begin();
      while ( it_outergroup != m_grid.end() )
      {
    
        // iterate over inner groups
        TOuterGroup::const_iterator it_innergroup = (*it_outergroup).begin();
        while ( it_innergroup != (*it_outergroup).end() )
        {
          if ( accountseries )
            _view.params().setLegendText( rownum-1, it_innergroup.key() );
          else
            abscissaNames += it_innergroup.key();
          
          //
          // Columns
          //
  
          if ( seriestotals )
          {
            if ( accountseries )
              data.setCell( rownum-1, 0, (*it_innergroup).m_total.m_total.toDouble() );
            else
              data.setCell( 0, rownum-1, (*it_innergroup).m_total.m_total.toDouble() );
          }
          else
          {
            unsigned column = 1;
            while ( column < m_numColumns )
            {
              if ( accountseries )
                data.setCell( rownum-1, column-1, (*it_innergroup).m_total[column].toDouble() );
              else
                data.setCell( column-1, rownum-1, (*it_innergroup).m_total[column].toDouble() );
              ++column;
            }
          }
            
          // TODO: This is inefficient. Really we should total up how many rows
          // there will be and allocate it all at once.
          if ( accountseries )
            data.expand( ++rownum, m_numColumns );
          else
            data.expand( m_numColumns, ++rownum );
                              
          ++it_innergroup;
        }
        ++it_outergroup;
      }
      if ( accountseries )
        data.expand( rownum-1, m_numColumns );
      else
        data.expand( m_numColumns, rownum-1 );

    }
    break;
          
    case MyMoneyReport::eDetailGroup:
    {
      unsigned rownum = 1;  
      
      // iterate over outer groups
      TGrid::const_iterator it_outergroup = m_grid.begin();
      while ( it_outergroup != m_grid.end() )
      {
        if ( accountseries )    
          _view.params().setLegendText( rownum-1, it_outergroup.key() );
        else
          abscissaNames += it_outergroup.key();
          
        //
        // Columns
        //

        if ( seriestotals )
        {
          if ( accountseries )
            data.setCell( rownum-1, 0, (*it_outergroup).m_total.m_total.toDouble() );
          else
            data.setCell( 0, rownum-1, (*it_outergroup).m_total.m_total.toDouble() );
        }
        else
        {
          unsigned column = 1;
          while ( column < m_numColumns )
          {
            if ( accountseries )
              data.setCell( rownum-1, column-1, (*it_outergroup).m_total[column].toDouble() );
            else
              data.setCell( column-1, rownum-1, (*it_outergroup).m_total[column].toDouble() );
            ++column;
          }
        }
          
        // TODO: This is inefficient. Really we should total up how many rows
        // there will be and allocate it all at once.
        if ( accountseries )
          data.expand( ++rownum, m_numColumns );
        else
          data.expand( m_numColumns, ++rownum );
                  
        ++it_outergroup;  
      }
      if ( accountseries )
        data.expand( rownum-1, m_numColumns );
      else
        data.expand( m_numColumns, rownum-1 );
    }
    break;
          
    case MyMoneyReport::eDetailTotal:
    {
      if ( accountseries )
        _view.params().setLegendText( 0, i18n("Total") );
      else
        abscissaNames += i18n("Total");

      if ( seriestotals )
      {
        if ( accountseries )
          data.setCell( 0, 0, m_grid.m_total.m_total.toDouble() );
        else
          data.setCell( 0, 0, m_grid.m_total.m_total.toDouble() );
      }
      else
      {
        // For now, just the totals
        unsigned totalcolumn = 1;
        while ( totalcolumn < m_numColumns )
        {
          if ( accountseries )
            data.setCell( 0, totalcolumn-1, m_grid.m_total[totalcolumn].toDouble() );
          else
            data.setCell( totalcolumn-1, 0, m_grid.m_total[totalcolumn].toDouble() );
          ++totalcolumn;
        }
      }
    }
    break;
  }
      
  _view.setNewData(data);
  _view.refreshLabels();

#endif
}

} // namespace
