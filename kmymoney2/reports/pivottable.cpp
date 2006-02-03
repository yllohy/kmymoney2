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

const unsigned PivotTable::TOuterGroup::m_kDefaultSortOrder = 100;

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

  // if this is a months-based report
  if (! m_config_f.isColumnsAreDays())
  {
    // strip out the 'days' component of the begin and end dates.
    // we're only using these variables to contain year and month.
    m_beginDate =  QDate( m_beginDate.year(), m_beginDate.month(), 1 );
    m_endDate = QDate( m_endDate.year(), m_endDate.month(), 1 );
  }
  
  m_numColumns = columnValue(m_endDate) - columnValue(m_beginDate) + 2;

  //
  // Initialize outer groups of the grid
  //
  if ( m_config_f.rowType() == MyMoneyReport::eAssetLiability )
  {
    m_grid.insert(accountTypeToString(MyMoneyAccount::Asset),TOuterGroup(m_numColumns));
    m_grid.insert(accountTypeToString(MyMoneyAccount::Liability),TOuterGroup(m_numColumns,TOuterGroup::m_kDefaultSortOrder,true));
  }
  else
  {
    m_grid.insert(accountTypeToString(MyMoneyAccount::Income),TOuterGroup(m_numColumns,TOuterGroup::m_kDefaultSortOrder-2));
    m_grid.insert(accountTypeToString(MyMoneyAccount::Expense),TOuterGroup(m_numColumns,TOuterGroup::m_kDefaultSortOrder-1,true));
  }

  //
  // Initialize grid totals
  //

  m_grid.m_total = TGridRow(m_numColumns);
  
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
  QValueList<MyMoneyTransaction> transactions;
  try {
    transactions = file->transactionList(m_config_f);
  } catch(MyMoneyException *e) {
    qDebug("ERR: %s thrown in %s(%ld)", e->what().data(), e->file().data(), e->line());
    throw e;
  }
  DEBUG_OUTPUT(QString("Found %1 matching transactions").arg(transactions.count()));

  // Include scheduled transactions if required
  if ( m_config_f.isIncludingSchedules() )
  {
    // Create a custom version of the report filter, excluding date
    // We'll use this to compare the transaction against
    MyMoneyTransactionFilter schedulefilter(m_config_f);
    schedulefilter.setDateFilter(QDate(),QDate());

    // Get the real dates from the config filter
    QDate configbegin, configend;
    m_config_f.validDateRange(configbegin, configend);

    QValueList<MyMoneySchedule> schedules = file->scheduleList();
    QValueList<MyMoneySchedule>::const_iterator it_schedule = schedules.begin();
    while ( it_schedule != schedules.end() )
    {
      // If the transaction meets the filter
      MyMoneyTransaction tx = (*it_schedule).transaction();
      if ( schedulefilter.match(tx,file->storage()) )
      {
        // Get the dates when a payment will be made within the report window
        QDate nextpayment = (*it_schedule).nextPayment(configbegin);
        if ( nextpayment.isValid() )
        {
          // Add one transaction for each date
          QValueList<QDate> paymentdates = (*it_schedule).paymentDates(nextpayment,configend);
          QValueList<QDate>::const_iterator it_date = paymentdates.begin();
          while ( it_date != paymentdates.end() )
          {
            tx.setPostDate(*it_date);

            // ???? Does this violate an assumption that transactions are sorted
            // by date?? (ace)
            transactions += tx;

            DEBUG_OUTPUT(QString("Added transaction for schedule %1 on %2").arg((*it_schedule).id()).arg((*it_date).toString()));

            ++it_date;
          }
        }
      }

      ++it_schedule;
    }
  }
  
  // whether asset & liability transactions are actually to be considered
  // transfers
  bool al_transfers = ( m_config_f.rowType() == MyMoneyReport::eExpenseIncome ) && ( m_config_f.isIncludingTransfers() );
  
  QValueList<MyMoneyTransaction>::const_iterator it_transaction = transactions.begin();
  unsigned colofs = columnValue(m_beginDate) - 1;
  while ( it_transaction != transactions.end() )
  {
    QDate postdate = (*it_transaction).postDate();
    unsigned column = columnValue(postdate) - colofs;

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
        MyMoneyAccount::accountTypeE type = splitAccount.accountGroup();
        QString outergroup = accountTypeToString(type);
        
        // Except in the case of transfers on an income/expense report
        if ( al_transfers && ( type == MyMoneyAccount::Asset || type == MyMoneyAccount::Liability ) )
        {
          outergroup = i18n("Transfers");
          value = -value;
        }
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

  if ( m_config_f.columnPitch() > 1 )
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
    unsigned sourcemonth = columnValue(m_beginDate);
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
  
  // if this is a days-based report
  if (m_config_f.isColumnsAreDays())
  {
    if ( columnpitch == 1 )
    {
      QDate columnDate = m_beginDate;
      unsigned column = 1;
      while ( column++ < m_numColumns )
      {
        QString heading = QDate::shortMonthName(columnDate.month()) + " " + QString::number(columnDate.day());
        columnDate = columnDate.addDays(1);
        m_columnHeadings.append( heading);
      }
    }
    else
    {
      QDate columnDate = m_beginDate;
      unsigned column = 1;
      while ( column++ < m_numColumns )
      {
        QDate columnEndDate = columnDate.addDays(columnpitch).addDays(-1);
        if ( columnEndDate > m_endDate )
          columnEndDate = m_endDate;
      
        QString heading = QDate::shortMonthName(columnDate.month()) + "&nbsp;" + QString::number(columnDate.day()) + " - " + QDate::shortMonthName(columnEndDate.month()) + "&nbsp;" + QString::number(columnEndDate.day());
        
        columnDate = columnDate.addDays(columnpitch);
        m_columnHeadings.append( heading);
      }
    }
  }
  
  // else it's a months-based report
  else
  {
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
        unsigned column = columnValue(opendate) - columnValue(m_beginDate) + 1;
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
        unsigned column = 1;
        while ( column < m_numColumns )
        {
          if ( it_row.data().count() <= column )
            throw new MYMONEYEXCEPTION(QString("Column %1 out of grid range (%2) in PivotTable::convertToBaseCurrency").arg(column).arg(it_row.data().count()));

          QDate valuedate = columnDate(column);
          double conversionfactor = it_row.key().baseCurrencyPrice(valuedate).toDouble();
          double oldval = it_row.data()[column].toDouble();
          double value = oldval * conversionfactor;
          it_row.data()[column] = MyMoneyMoney( value );

          DEBUG_OUTPUT_IF(conversionfactor != 1.0 ,QString("Factor of %1, value was %2, now %3").arg(conversionfactor).arg(DEBUG_SENSITIVE(oldval)).arg(DEBUG_SENSITIVE(it_row.data()[column].toDouble())));

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
        unsigned column = 1;
        while ( column < m_numColumns )
        {
          if ( it_row.data().count() <= column )
            throw new MYMONEYEXCEPTION(QString("Column %1 out of grid range (%2) in PivotTable::convertToDeepCurrency").arg(column).arg(it_row.data().count()));

          QDate valuedate = columnDate(column);
          double conversionfactor = it_row.key().deepCurrencyPrice(valuedate).toDouble();
          double oldval = it_row.data()[column].toDouble();
          double value = oldval * conversionfactor;
          it_row.data()[column] = MyMoneyMoney( value );

          DEBUG_OUTPUT_IF(conversionfactor != 1.0 ,QString("Factor of %1, value was %2, now %3").arg(conversionfactor).arg(DEBUG_SENSITIVE(oldval)).arg(DEBUG_SENSITIVE(it_row.data()[column].toDouble())));

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

    bool invert_total = (*it_outergroup).m_inverted;
    unsigned column = 0;
    while ( column < m_numColumns )
    {
      if ( m_grid.m_total.count() <= column )
        throw new MYMONEYEXCEPTION(QString("Column %1 out of grid range (%2) in PivotTable::calculateTotals, grid totals").arg(column).arg((*it_innergroup).m_total.count()));

      MyMoneyMoney value = (*it_outergroup).m_total[column];
      (*it_outergroup).m_total.m_total += value;
      
      if ( invert_total )
        value = -value;

      m_grid.m_total[column] += value;

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

void PivotTable::assignCell( const QString& outergroup, const ReportAccount& row, unsigned column, MyMoneyMoney value, bool budget )
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

  // Determine whether the value should be inverted before being placed in the row
  if ( m_grid[outergroup].m_inverted )
    value = -value;
    
  // Add the value to the grid cell
  if ( budget )
    m_grid[outergroup][innergroup][row].m_budget[column] += value;
  else
    m_grid[outergroup][innergroup][row][column] += value;

}

void PivotTable::createRow( const QString& outergroup, const ReportAccount& row, bool recursive )
{
  DEBUG_ENTER(__PRETTY_FUNCTION__);

  // Determine the inner group from the top-most parent account
  QString innergroup( row.topParentName() );

  if ( ! m_grid.contains(outergroup) )
  {
    DEBUG_OUTPUT(QString("Adding group [%1]").arg(outergroup));
    m_grid[outergroup] = TOuterGroup(m_numColumns);
  }

  if ( ! m_grid[outergroup].contains(innergroup) )
  {
    DEBUG_OUTPUT(QString("Adding group [%1][%2]").arg(outergroup).arg(innergroup));
    m_grid[outergroup][innergroup] = TInnerGroup(m_numColumns); 
  }
  
  if ( ! m_grid[outergroup][innergroup].contains(row) )
  {
    DEBUG_OUTPUT(QString("Adding row [%1][%2][%3]").arg(outergroup).arg(innergroup).arg(row.debugName()));
    m_grid[outergroup][innergroup][row] = TGridRowPair(m_numColumns);

    if ( recursive && !row.isTopLevel() )
        createRow( outergroup, row.parent(), recursive );
  }
}

unsigned PivotTable::columnValue(const QDate& _date) const
{
  if (m_config_f.isColumnsAreDays())
    return (QDate().daysTo(_date));
  else
    return (_date.year() * 12 + _date.month()); 
}

QDate PivotTable::columnDate(int column) const
{
  if (m_config_f.isColumnsAreDays())
    return m_beginDate.addDays( m_config_f.columnPitch() * column - 1 ); 
  else
    return m_beginDate.addMonths( m_config_f.columnPitch() * column ).addDays(-1); 
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

  // Hardcoded exampled for a budget vs. actual report
  if (m_config_f.rowType() == MyMoneyReport::eBudgetActual )
  {
    result += QString("<div class=\"subtitle\">Budget 2006 for 2006</div>");
    result += QString("\n\n<table class=\"report\" cellspacing=\"0\">");
    result += QString("<tbody>\n");
    result += QString("<tr class=\"itemheader\">\n");
    result += QString("<th>Account</th>\n");
    result += QString("<th colspan=\"2\">Jan<br></th>\n");
    result += QString("<th colspan=\"2\">Feb<br></th>\n");
    result += QString("<th colspan=\"2\">Total<br></th>\n");
    result += QString("<th colspan=\"2\">Difference<br></th>\n");
    result += QString("</td>\n");
    result += QString("</tr>\n");
    result += QString("<tr>\n");
    result += QString("<td style=\"vertical-align: top; font-weight: bold;\"><br>\n");
    result += QString("</td>\n");
    result += QString("<td style=\"vertical-align: top; font-weight: bold;\">Budget<br>\n");
    result += QString("</td>\n");
    result += QString("<td style=\"vertical-align: top; font-weight: bold;\">Actual<br>\n");
    result += QString("</td>\n");
    result += QString("<td style=\"vertical-align: top; font-weight: bold;\">Budget<br>\n");
    result += QString("</td>\n");
    result += QString("<td style=\"vertical-align: top; font-weight: bold;\">Actual<br>\n");
    result += QString("</td>\n");
    result += QString("<td style=\"vertical-align: top; font-weight: bold;\">Budget<br>\n");
    result += QString("</td>\n");
    result += QString("<td style=\"vertical-align: top; font-weight: bold;\">Actual<br>\n");
    result += QString("</td>\n");
    result += QString("<td style=\"vertical-align: top; font-weight: bold;\"><br>\n");
    result += QString("</td>\n");
    result += QString("</tr>\n");
    result += QString("<tr>\n");
    result += QString("<td class=\"left\" colspan=\"1\">Income<br>\n");
    result += QString("</td>\n");
    result += QString("<td colspan=\"1\" style=\"vertical-align: top; font-weight: normal;\">100.00<br>\n");
    result += QString("</td>\n");
    result += QString("<td colspan=\"1\" style=\"vertical-align: top; font-weight: normal;\">200.00<br>\n");
    result += QString("</td>\n");
    result += QString("<td style=\"vertical-align: top; font-weight: normal;\">200.00<br>\n");
    result += QString("</td>\n");
    result += QString("<td style=\"vertical-align: top; font-weight: normal;\">100.00<br>\n");
    result += QString("</td>\n");
    result += QString("<td style=\"vertical-align: top; font-weight: normal;\">300.00<br>\n");
    result += QString("</td>\n");
    result += QString("<td style=\"vertical-align: top; font-weight: normal;\">300.00<br>\n");
    result += QString("</td>\n");
    result += QString("<td\n");
    result += QString("style=\"vertical-align: top; font-weight: normal;\">0.00<br>\n");
    result += QString("</td>\n");
    result += QString("</tr>\n");
    result += QString("<tr class=\"row-even\" id=\"topparent\">\n");
    result += QString("<td class=\"left2\">Salary<br>\n");
    result += QString("</td>\n");
    result += QString("<td>400.00<br>\n");
    result += QString("</td>\n");
    result += QString("<td>100.00<br>\n");
    result += QString("</td>\n");
    result += QString("<td>100.00<br>\n");
    result += QString("</td>\n");
    result += QString("<td style=\"vertical-align: top;\">100.00<br>\n");
    result += QString("</td>\n");
    result += QString("<td style=\"vertical-align: top;\">500.00<br>\n");
    result += QString("</td>\n");
    result += QString("<td style=\"vertical-align: top;\">400.00<br>\n");
    result += QString("</td>\n");
    result += QString("<td style=\"vertical-align: top; color: rgb(255, 0, 0);\">(100.00)<br>\n");
    result += QString("</td>\n");
    result += QString("</tr>\n");
    result += QString("<tr class=\"row-even\" id=\"topparent\">\n");
    result += QString("<td style=\"text-align: left; font-weight: normal;\" class=\"left2\">Interest<br>\n");
    result += QString("</td>\n");
    result += QString("<td colspan=\"1\" style=\"vertical-align: top; font-weight: normal;\">100.00<br>\n");
    result += QString("</td>\n");
    result += QString("<td colspan=\"1\" style=\"vertical-align: top; font-weight: normal;\">200.00<br>\n");
    result += QString("</td>\n");
    result += QString("<td style=\"vertical-align: top; font-weight: normal;\">100.00<br>\n");
    result += QString("</td>\n");
    result += QString("<td style=\"vertical-align: top; font-weight: normal;\">300.00<br>\n");
    result += QString("</td>\n");
    result += QString("<td style=\"vertical-align: top; font-weight: normal;\">200.00<br>\n");
    result += QString("</td>\n");
    result += QString("<td style=\"vertical-align: top; font-weight: normal;\">500.00<br>\n");
    result += QString("</td>\n");
    result += QString("<td style=\"vertical-align: top; color: rgb(255, 0, 0);\"><span\n");
    result += QString("style=\"font-weight: normal; color: rgb(0, 0, 0);\">300.00</span><br>\n");
    result += QString("</td>\n");
    result += QString("</tr>\n");
    result += QString("<tr class=\"row-even\" id=\"subtotal\">\n");
    result += QString("<td style=\"font-weight: bold;\" class=\"left\">&nbsp;&nbsp;Total</td>\n");
    result += QString("<td style=\"vertical-align: top; font-weight: bold;\">600.00<br>\n");
    result += QString("</td>\n");
    result += QString("<td style=\"vertical-align: top; font-weight: bold;\">500.00<br>\n");
    result += QString("</td>\n");
    result += QString("<td style=\"vertical-align: top; font-weight: bold;\">400.00<br>\n");
    result += QString("</td>\n");
    result += QString("<td style=\"vertical-align: top; font-weight: bold;\">500.00<br>\n");
    result += QString("</td>\n");
    result += QString("<td style=\"vertical-align: top; font-weight: bold;\">1000.00<br>\n");
    result += QString("</td>\n");
    result += QString("<td style=\"vertical-align: top; font-weight: bold;\">1200.00<br>\n");
    result += QString("</td>\n");
    result += QString("<td\n");
    result += QString("style=\"vertical-align: top; color: rgb(0, 0, 0); font-weight: bold;\">&nbsp;<br>\n");
    result += QString("</td>\n");
    result += QString("</tr>\n");
    result += QString("<tr class=\"row-even\" id=\"topparent\">\n");
    result += QString("<td style=\"font-weight: bold;\" class=\"left2\">Difference<br>\n");
    result += QString("</td>\n");
    result += QString("<td style=\"text-align: center; font-weight: bold;\" colspan=\"2\"\n");
    result += QString("rowspan=\"1\"><span style=\"color: rgb(255, 0, 0);\">(100.00)</span><br>\n");
    result += QString("</td>\n");
    result += QString("<td style=\"text-align: center; font-weight: bold;\" colspan=\"2\"\n");
    result += QString("rowspan=\"1\">100.00<br>\n");
    result += QString("</td>\n");
    result += QString("<td colspan=\"2\" rowspan=\"1\"\n");
    result += QString("style=\"vertical-align: top; text-align: center; font-weight: bold;\">200.00<br>\n");
    result += QString("</td>\n");
    result += QString("<td\n");
    result += QString("style=\"vertical-align: top; color: rgb(0, 0, 0); font-weight: bold;\"><br>\n");
    result += QString("</td>\n");
    result += QString("</tr>\n");
    result += QString("<tr>\n");
    result += QString("<td style=\"vertical-align: top;\"><br>\n");
    result += QString("</td>\n");
    result += QString("<td style=\"vertical-align: top;\"><br>\n");
    result += QString("</td>\n");
    result += QString("<td style=\"vertical-align: top;\"><br>\n");
    result += QString("</td>\n");
    result += QString("<td style=\"vertical-align: top;\"><br>\n");
    result += QString("</td>\n");
    result += QString("<td style=\"vertical-align: top;\"><br>\n");
    result += QString("</td>\n");
    result += QString("<td style=\"vertical-align: top;\"><br>\n");
    result += QString("</td>\n");
    result += QString("<td style=\"vertical-align: top;\"><br>\n");
    result += QString("</td>\n");
    result += QString("<td style=\"vertical-align: top;\"><br>\n");
    result += QString("</td>\n");
    result += QString("</tr>\n");
    result += QString("<tr>\n");
    result += QString("<td style=\"vertical-align: top; text-align: left;\">Expense<br>\n");
    result += QString("</td>\n");
    result += QString("<td style=\"vertical-align: top;\">200.00<br>\n");
    result += QString("</td>\n");
    result += QString("<td style=\"vertical-align: top;\">100.00<br>\n");
    result += QString("</td>\n");
    result += QString("<td style=\"vertical-align: top;\">100.00<br>\n");
    result += QString("</td>\n");
    result += QString("<td style=\"vertical-align: top;\">200.00<br>\n");
    result += QString("</td>\n");
    result += QString("<td style=\"vertical-align: top;\">300.00<br>\n");
    result += QString("</td>\n");
    result += QString("<td style=\"vertical-align: top;\">300.00<br>\n");
    result += QString("</td>\n");
    result += QString("<td style=\"vertical-align: top;\">0.00<br>\n");
    result += QString("</td>\n");
    result += QString("</tr>\n");
    result += QString("<tr>\n");
    result += QString("<td style=\"vertical-align: top; text-align: left;\" class=\"left2\">Groceries<br>\n");
    result += QString("</td>\n");
    result += QString("<td style=\"vertical-align: top;\">100.00<br>\n");
    result += QString("</td>\n");
    result += QString("<td style=\"vertical-align: top;\">400.00<br>\n");
    result += QString("</td>\n");
    result += QString("<td style=\"vertical-align: top;\">100.00<br>\n");
    result += QString("</td>\n");
    result += QString("<td style=\"vertical-align: top;\">100.00<br>\n");
    result += QString("</td>\n");
    result += QString("<td style=\"vertical-align: top;\">400.00<br>\n");
    result += QString("</td>\n");
    result += QString("<td style=\"vertical-align: top;\">500.00<br>\n");
    result += QString("</td>\n");
    result += QString("<td style=\"vertical-align: top;\">100.00<br>\n");
    result += QString("</td>\n");
    result += QString("</tr>\n");
    result += QString("<tr>\n");
    result += QString("<td style=\"vertical-align: top; text-align: left;\" class=\"left2\">&nbsp;&nbsp;&nbsp;\n");
    result += QString("Phone<br>\n");
    result += QString("</td>\n");
    result += QString("<td style=\"vertical-align: top;\">600.00<br>\n");
    result += QString("</td>\n");
    result += QString("<td style=\"vertical-align: top;\">100.00<br>\n");
    result += QString("</td>\n");
    result += QString("<td style=\"vertical-align: top;\">100.00<br>\n");
    result += QString("</td>\n");
    result += QString("<td style=\"vertical-align: top;\">500.00<br>\n");
    result += QString("</td>\n");
    result += QString("<td style=\"vertical-align: top;\">700.00<br>\n");
    result += QString("</td>\n");
    result += QString("<td style=\"vertical-align: top;\">600.00<br>\n");
    result += QString("</td>\n");
    result += QString("<td style=\"vertical-align: top; color: rgb(255, 0, 0);\">(100.00)<br>\n");
    result += QString("</td>\n");
    result += QString("</tr>\n");
    result += QString("<tr>\n");
    result += QString("<td\n");
    result += QString("<tr class=\"row-even\" id=\"subtotal\">\n");
    result += QString("<td style=\"font-weight: bold;\" class=\"left\">&nbsp;&nbsp;Total</td>\n");
    result += QString("<td style=\"vertical-align: top; font-weight: bold;\">900.00<br>\n");
    result += QString("</td>\n");
    result += QString("<td style=\"vertical-align: top; font-weight: bold;\">600.00<br>\n");
    result += QString("</td>\n");
    result += QString("<td style=\"vertical-align: top; font-weight: bold;\">300.00<br>\n");
    result += QString("</td>\n");
    result += QString("<td style=\"vertical-align: top; font-weight: bold;\">800.00<br>\n");
    result += QString("</td>\n");
    result += QString("<td style=\"vertical-align: top; font-weight: bold;\">1400.00<br>\n");
    result += QString("</td>\n");
    result += QString("<td style=\"vertical-align: top; font-weight: bold;\">1400.00<br>\n");
    result += QString("</td>\n");
    result += QString("<td style=\"vertical-align: top; font-weight: bold;\">&nbsp;<br>\n");
    result += QString("</td>\n");
    result += QString("</tr>\n");
    result += QString("<tr>\n");
    result += QString("<tr class=\"row-even\" id=\"topparent\">\n");
    result += QString("<td style=\"vertical-align: top; font-weight: bold;\">Difference<br>\n");
    result += QString("</td>\n");
    result += QString("<td colspan=\"2\" rowspan=\"1\"\n");
    result += QString("style=\"vertical-align: top; font-weight: bold; text-align: center;\">300.00<br>\n");
    result += QString("</td>\n");
    result += QString("<td colspan=\"2\" rowspan=\"1\"\n");
    result += QString("style=\"vertical-align: top; font-weight: bold; text-align: center;\"><span\n");
    result += QString("style=\"color: rgb(255, 0, 0);\">(500.00)</span><br>\n");
    result += QString("</td>\n");
    result += QString("<td colspan=\"2\" rowspan=\"1\"\n");
    result += QString("style=\"vertical-align: top; font-weight: bold; text-align: center;\">0.00<br>\n");
    result += QString("</td>\n");
    result += QString("<td style=\"vertical-align: top; font-weight: bold;\"><br>\n");
    result += QString("</td>\n");
    result += QString("</tr>\n");
    result += QString("<tr class=\"spacer\"><td>&nbsp;</td></tr>\n");
    result += QString("<tr class=\"reportfooter\"><td class=\"left\">Net Profit</td>");
    result += QString("<td style=\"vertical-align: top; font-weight: bold; color: rgb(255, 0, 0);\">(300.00)</td>\n");
    result += QString("<td style=\"vertical-align: top; font-weight: bold; color: rgb(255, 0, 0);\">(100.00)</td><br>\n");
    result += QString("<td style=\"vertical-align: top; font-weight: bold; color: rgb(0, 0, 0);\">100.00</td><br>\n");
    result += QString("<td style=\"vertical-align: top; font-weight: bold; color: rgb(255, 0, 0);\">(300.00)<br>\n");
    result += QString("</td>\n");
    result += QString("<td style=\"vertical-align: top; font-weight: bold; color: rgb(255, 0, 0);\">(400.00)<br></td>\n");
    result += QString("<td\n");
    result += QString("style=\"vertical-align: top; font-weight: bold; color: rgb(255, 0, 0);\">(200.00)<br>\n");
    result += QString("</td>\n");
    result += QString("<td style=\"vertical-align: top; font-weight: bold;\"><br>\n");
    result += QString("</td>\n");
    result += QString("</tr>\n");
    result += QString("<tr>\n");
    result += QString("<td style=\"vertical-align: top; font-weight: bold;\">Difference<br>\n");
    result += QString("</td>\n");
    result += QString("<td colspan=\"2\" rowspan=\"1\"\n");
    result += QString("style=\"vertical-align: top; font-weight: bold; text-align: center;\">200.00<br>\n");
    result += QString("</td>\n");
    result += QString("<td colspan=\"2\" rowspan=\"1\"\n");
    result += QString("style=\"vertical-align: top; font-weight: bold; text-align: center; color: rgb(255, 0, 0);\">(400.00)<br>\n");
    result += QString("</td>\n");
    result += QString("<td colspan=\"2\" rowspan=\"1\"\n");
    result += QString("style=\"vertical-align: top; font-weight: bold; text-align: center;\">200.00<br>\n");
    result += QString("</td>\n");
    result += QString("<td style=\"vertical-align: top; font-weight: bold;\"><br>\n");
    result += QString("</td>\n");
    result += QString("</tr>\n");
    result += QString("<tr class=\"spacer\"><td>&nbsp;</td></tr>\n");
    result += QString("<tr class=\"spacer\"><td>&nbsp;</td></tr>\n");
    result += QString("</tbody>\n");
    result += QString("</table>\n");

    return result;
  }
  // Hardcoded exampled for a budget report
  else if (m_config_f.rowType() == MyMoneyReport::eBudget )
  {
    result += QString("<div class=\"subtitle\">Budget 2006 for 2006</div>");
    result += QString("\n\n<table class=\"report\" cellspacing=\"0\">");
    result += QString("<tbody>");
    result += QString("<tr class=\"itemheader\">");
    result += QString("<th>Account</th>");
    result += QString("<th colspan=\"1\">Jan<br>");
    result += QString("</th>");
    result += QString("<th colspan=\"1\">Feb<br>");
    result += QString("</th>");
    result += QString("<th colspan=\"1\">Total<br>");
    result += QString("</th>");
    result += QString("</tr>");
    result += QString("<tr>");
    result += QString("<td class=\"left\" colspan=\"1\">Income<br>");
    result += QString("</td>");
    result += QString("<td colspan=\"1\" style=\"vertical-align: top; font-weight: normal;\">100.00<br>");
    result += QString("</td>");
    result += QString("<td style=\"vertical-align: top; font-weight: normal;\">200.00<br>");
    result += QString("</td>");
    result += QString("<td style=\"vertical-align: top; font-weight: normal;\">300.00<br>");
    result += QString("</td>");
    result += QString("</tr>");
    result += QString("<tr class=\"row-even\" id=\"topparent\">");
    result += QString("<td class=\"left2\">Salary<br>");
    result += QString("</td>");
    result += QString("<td>400.00<br>");
    result += QString("</td>");
    result += QString("<td>100.00<br>");
    result += QString("</td>");
    result += QString("<td style=\"vertical-align: top;\">500.00<br>");
    result += QString("</td>");
    result += QString("</tr>");
    result += QString("<tr class=\"row-even\" id=\"topparent\">");
    result += QString("<td style=\"text-align: left; font-weight: normal;\" class=\"left2\">Interest<br>");
    result += QString("</td>");
    result += QString("<td colspan=\"1\" style=\"vertical-align: top; font-weight: normal;\">100.00<br>");
    result += QString("</td>");
    result += QString("<td style=\"vertical-align: top; font-weight: normal;\">100.00<br>");
    result += QString("</td>");
    result += QString("<td style=\"vertical-align: top; font-weight: normal;\">200.00<br>");
    result += QString("</td>");
    result += QString("</tr>");
    result += QString("<tr class=\"row-even\" id=\"subtotal\">");
    result += QString("<td style=\"font-weight: bold;\" class=\"left\">&nbsp;&nbsp;Total</td>");
    result += QString("<td style=\"vertical-align: top; font-weight: bold;\">600.00<br>");
    result += QString("</td>");
    result += QString("<td style=\"vertical-align: top; font-weight: bold;\">400.00<br>");
    result += QString("</td>");
    result += QString("<td style=\"vertical-align: top; font-weight: bold;\">1000.00<br>");
    result += QString("</td>");
    result += QString("</tr>");
    result += QString("<tr>");
    result += QString("<td style=\"vertical-align: top;\"><br>");
    result += QString("</td>");
    result += QString("<td style=\"vertical-align: top;\"><br>");
    result += QString("</td>");
    result += QString("<td style=\"vertical-align: top;\"><br>");
    result += QString("</td>");
    result += QString("<td style=\"vertical-align: top;\"><br>");
    result += QString("</td>");
    result += QString("</tr>");
    result += QString("<tr>");
    result += QString("<td style=\"vertical-align: top; text-align: left;\">Expense<br>");
    result += QString("</td>");
    result += QString("<td style=\"vertical-align: top;\">200.00<br>");
    result += QString("</td>");
    result += QString("<td style=\"vertical-align: top;\">100.00<br>");
    result += QString("</td>");
    result += QString("<td style=\"vertical-align: top;\">300.00<br>");
    result += QString("</td>");
    result += QString("</tr>");
    result += QString("<tr>");
    result += QString("<td style=\"vertical-align: top; text-align: left;\" class=\"left2\">Groceries<br>");
    result += QString("</td>");
    result += QString("<td style=\"vertical-align: top;\">100.00<br>");
    result += QString("</td>");
    result += QString("<td style=\"vertical-align: top;\">100.00<br>");
    result += QString("</td>");
    result += QString("<td style=\"vertical-align: top;\">200.00<br>");
    result += QString("</td>");
    result += QString("</tr>");
    result += QString("<tr>");
    result += QString("<td style=\"vertical-align: top; text-align: left;\" class=\"left2\">&nbsp;&nbsp;&nbsp;");
    result += QString("Phone<br>");
    result += QString("</td>");
    result += QString("<td style=\"vertical-align: top;\">600.00<br>");
    result += QString("</td>");
    result += QString("<td style=\"vertical-align: top;\">100.00<br>");
    result += QString("</td>");
    result += QString("<td style=\"vertical-align: top;\">700.00<br>");
    result += QString("</td>");
    result += QString("</tr>");
    result += QString("<tr class=\"row-even\" id=\"subtotal\">");
    result += QString("<td style=\"font-weight: bold;\" class=\"left\">&nbsp;&nbsp;Total</td>");
    result += QString("<td style=\"vertical-align: top; font-weight: bold;\">300.00<br>");
    result += QString("</td>");
    result += QString("<td style=\"vertical-align: top; font-weight: bold;\">900.00<br>");
    result += QString("</td>");
    result += QString("<td style=\"vertical-align: top; font-weight: bold;\">1200.00<br>");
    result += QString("</td>");
    result += QString("</tr>");
    result += QString("<tr class=\"spacer\"><td>&nbsp;</td></tr>\n");
    result += QString("<tr class=\"reportfooter\"><td class=\"left\">Net Profit</td>");
    result += QString("<td style=\"vertical-align: top; font-weight: bold; color: rgb(255, 0, 0);\">(300.00)</td>\n");
    result += QString("<td style=\"vertical-align: top; font-weight: bold; color: rgb(255, 0, 0);\">(100.00)</td><br>\n");
    result += QString("<td style=\"vertical-align: top; font-weight: bold; color: rgb(0, 0, 0);\">100.00</td><br>\n");
    result += QString("</td>\n");
    result += QString("</tr>\n");
    result += QString("</tbody>");
    result += QString("</table>");
    result += QString("</body>");
    result += QString("</table>\n");

    return result;
  }

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

    // Need to sort the outergroups.  They can't always be sorted by name.  So we create a list of
    // map iterators, and sort that.  Then we'll iterate through the map iterators and use those as
    // before.
    //
    // I hope this doesn't bog the performance of reports, given that we're copying the entire report
    // data.  If this is a perf hit, we could change to storing outergroup pointers, I think.
    QValueList<TOuterGroup> outergroups;
    TGrid::const_iterator it_outergroup_map = m_grid.begin();
    while ( it_outergroup_map != m_grid.end() )
    {
      outergroups.push_back(it_outergroup_map.data());
      
      // copy the name into the outergroup, because we will now lose any association with
      // the map iterator
      outergroups.back().m_displayName = it_outergroup_map.key();

      ++it_outergroup_map;
    }
    qHeapSort(outergroups);
    
    QValueList<TOuterGroup>::const_iterator it_outergroup = outergroups.begin();
    while ( it_outergroup != outergroups.end() )
    {
      //
      // Outer Group Header
      //

      result += QString("<tr class=\"sectionheader\"><td class=\"left\"%1>%2</td></tr>\n").arg(colspan).arg((*it_outergroup).m_displayName);

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
        result += QString("<tr class=\"sectionfooter\"><td class=\"left\">%1&nbsp;%2</td>").arg(i18n("Total")).arg((*it_outergroup).m_displayName);
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

#ifdef HAVE_KDCHART
void PivotTable::drawChart( KReportChartView& _view ) const
{

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
            _view.params().setLegendText( rownum-1, it_row.key().name() );

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
          _view.params().setLegendText( rownum-1, it_innergroup.key() );

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
        _view.params().setLegendText( rownum-1, it_outergroup.key() );

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
      _view.params().setLegendText( 0, i18n("Total") );

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
}
#else
void PivotTable::drawChart( KReportChartView& ) const { }
#endif

} // namespace
// vim:cin:si:ai:et:ts=2:sw=2:
