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
// This is just needed for i18n() and weekStartDay().
// Once I figure out how to handle i18n
// without using this macro directly, I'll be freed of KDE dependency.  This
// is a minor problem because we use these terms when rendering to HTML,
// and a more major problem because we need it to translate account types
// (e.g. MyMoneyAccount::Checkings) into their text representation.  We also
// use that text representation in the core data structure of the report. (Ace)

#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>
#include <kcalendarsystem.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "pivottable.h"
#include "reportdebug.h"
#include "kreportchartview.h"
#include "../kmymoneyglobalsettings.h"

#include <kmymoney/kmymoneyutils.h>

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

PivotTable::TCell PivotTable::TCell::operator += (const TCell& right)
{
  const MyMoneyMoney& r = static_cast<const MyMoneyMoney&>(right);
  *this += r;
  m_postSplit = m_postSplit * right.m_stockSplit;
  m_stockSplit = m_stockSplit * right.m_stockSplit;
  m_postSplit += right.m_postSplit;
  m_cellUsed |= right.m_cellUsed;
  return *this;
}

PivotTable::TCell PivotTable::TCell::operator += (const MyMoneyMoney& value)
{
  m_cellUsed |= !value.isZero();
  if(m_stockSplit != MyMoneyMoney(1,1))
    m_postSplit += value;
  else
    MyMoneyMoney::operator += (value);
  return *this;
}

PivotTable::TCell PivotTable::TCell::stockSplit(const MyMoneyMoney& factor)
{
  TCell s;
  s.m_stockSplit = factor;
  return s;
}

const QString PivotTable::TCell::formatMoney(const QString& currency, const int prec, bool showThousandSeparator) const
{
  // construct the result
  MyMoneyMoney res = (*this * m_stockSplit) + m_postSplit;
  return res.formatMoney(currency, prec, showThousandSeparator);
}

MyMoneyMoney PivotTable::TCell::calculateRunningSum(const MyMoneyMoney& runningSum)
{
  MyMoneyMoney::operator += (runningSum);
  MyMoneyMoney::operator = ((*this * m_stockSplit) + m_postSplit);
  m_postSplit = MyMoneyMoney(0,1);
  m_stockSplit = MyMoneyMoney(1,1);
  return *this;
}

MyMoneyMoney PivotTable::TCell::cellBalance(const MyMoneyMoney& _balance)
{
  MyMoneyMoney balance(_balance);
  balance += *this;
  balance = (balance * m_stockSplit) + m_postSplit;
  return balance;
}

//////////////////////////////////////////////////////////////////////

PivotTable::PivotTable( const MyMoneyReport& _config_f ):
  m_runningSumsCalculated(false),
  m_config_f( _config_f )
{
  init();
}

void PivotTable::init(void)
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
    m_grid.insert(accountTypeToString(MyMoneyAccount::Liability),TOuterGroup(m_numColumns,TOuterGroup::m_kDefaultSortOrder,true /* inverted */));
  }
  else
  {
    m_grid.insert(accountTypeToString(MyMoneyAccount::Income),TOuterGroup(m_numColumns,TOuterGroup::m_kDefaultSortOrder-2));
    m_grid.insert(accountTypeToString(MyMoneyAccount::Expense),TOuterGroup(m_numColumns,TOuterGroup::m_kDefaultSortOrder-1,true /* inverted */));
    //
    // Create rows for income/expense reports with all accounts included
    //
    if(m_config_f.isIncludingUnusedAccounts())
      createAccountRows();
  }

  //
  // Initialize grid totals
  //

  m_grid.m_total = TGridRowPair(m_numColumns);

  //
  // Get opening balances
  // (for running sum reports only)
  //

  if ( m_config_f.isRunningSum() )
    calculateOpeningBalances();

  //
  // Calculate budget mapping
  // (for budget-vs-actual reports only)
  //
  if ( m_config_f.hasBudget())
    calculateBudgetMapping();

  //
  // Populate all transactions into the row/column pivot grid
  //

  QValueList<MyMoneyTransaction> transactions;
  m_config_f.setReportAllSplits(false);
  m_config_f.setConsiderCategory(true);
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
      if (!(*it_schedule).isFinished() && schedulefilter.match(tx, file->storage()) )
      {
        // Keep the id of the schedule with the transaction so that
        // we can do the autocalc later on in case of a loan payment
        tx.setValue("kmm-schedule-id", (*it_schedule).id());

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

    MyMoneyTransaction tx = (*it_transaction);

    // check if we need to call the autocalculation routine
    if(tx.isLoanPayment() && tx.hasAutoCalcSplit() && (tx.value("kmm-schedule-id").length() > 0)) {
      // make sure to consider any autocalculation for loan payments
      MyMoneySchedule sched = file->schedule(QCString(tx.value("kmm-schedule-id")));
      const MyMoneySplit& split = tx.amortizationSplit();
      if(!split.id().isEmpty()) {
        ReportAccount splitAccount = file->account(split.accountId());
        MyMoneyAccount::accountTypeE type = splitAccount.accountGroup();
        QString outergroup = accountTypeToString(type);

        QMap<QCString, MyMoneyMoney> balances;
        balances[splitAccount.id()] = cellBalance(outergroup, splitAccount, column, false);

        KMyMoneyUtils::calculateAutoLoan(sched, tx, balances);
      }
    }

    QValueList<MyMoneySplit> splits = tx.splits();
    QValueList<MyMoneySplit>::const_iterator it_split = splits.begin();
    while ( it_split != splits.end() )
    {
      ReportAccount splitAccount = (*it_split).accountId();

      // Each split must be further filtered, because if even one split matches,
      // the ENTIRE transaction is returned with all splits (even non-matching ones)
      if ( m_config_f.includes( splitAccount ) && m_config_f.match(&(*it_split), MyMoneyFile::instance()->storage()))
      {
        // reverse sign to match common notation for cash flow direction, only for expense/income splits
        MyMoneyMoney reverse(splitAccount.isIncomeExpense() ? -1 : 1, 1);

        MyMoneyMoney value;
        // the outer group is the account class (major account type)
        MyMoneyAccount::accountTypeE type = splitAccount.accountGroup();
        QString outergroup = accountTypeToString(type);

        value = (*it_split).shares();
        bool stockSplit = tx.isStockSplit();
        if(!stockSplit) {
          // retrieve the value in the account's underlying currency
          if(value != MyMoneyMoney::autoCalc) {
            value = value * reverse;
          } else {
            qDebug("PivotTable::PivotTable(): This must not happen");
            value = MyMoneyMoney();  // keep it 0 so far
          }

          // Except in the case of transfers on an income/expense report
          if ( al_transfers && ( type == MyMoneyAccount::Asset || type == MyMoneyAccount::Liability ) )
          {
            outergroup = i18n("Transfers");
            value = -value;
          }
        }
        // add the value to its correct position in the pivot table
        assignCell( outergroup, splitAccount, column, value, false, stockSplit );
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
    unsigned sourcemonth = (m_config_f.isColumnsAreDays())
      // use the user's locale to determine the week's start
      ? (m_beginDate.dayOfWeek() + 8 - KGlobal::locale()->weekStartDay()) % 7
      : m_beginDate.month();
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

      if (++sourcecolumn < m_numColumns) {
        if ((sourcemonth++ % columnpitch) == 0) {
          if (sourcecolumn != ++destcolumn)
            clearColumn (destcolumn);
        }
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

        (*it_row++)[column] = TCell();
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
        QString heading = KGlobal::locale()->calendar()->monthName(columnDate.month(), columnDate.year(), true) + " " + QString::number(columnDate.day());
        columnDate = columnDate.addDays(1);
        m_columnHeadings.append( heading);
      }
    }
    else
    {
      QDate day = m_beginDate;
      QDate prv = m_beginDate;

      // use the user's locale to determine the week's start
      unsigned dow = (day.dayOfWeek() +8 -KGlobal::locale()->weekStartDay())%7;

      while (day <= m_endDate)
      {
        if (((dow % columnpitch) == 0) || (day == m_endDate))
        {
          m_columnHeadings.append(QString("%1&nbsp;%2 - %3&nbsp;%4")
            .arg(KGlobal::locale()->calendar()->monthName(prv.month(), prv.year(), true))
            .arg(prv.day())
            .arg(KGlobal::locale()->calendar()->monthName(day.month(), day.year(), true))
            .arg(day.day()));
          prv = day.addDays(1);
        }
        day = day.addDays(1);
        dow++;
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
        QString heading = KGlobal::locale()->calendar()->monthName(1+segment*columnpitch, 2000, true);
        if ( columnpitch != 1 )
          heading += "-" + KGlobal::locale()->calendar()->monthName((1+segment)*columnpitch, 2000, true);
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

void PivotTable::createAccountRows(void)
{
  DEBUG_ENTER(__PRETTY_FUNCTION__);
  MyMoneyFile* file = MyMoneyFile::instance();

  QValueList<MyMoneyAccount> accounts;
  file->accountList(accounts);

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
      // place into the 'opening' column...
      assignCell( outergroup, account, 0, MyMoneyMoney() );
    }
    ++it_account;
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

  QValueList<MyMoneyAccount> accounts;
  file->accountList(accounts);

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

#if 1 // FIXME: openingbalance is now a transaction
      // remove the opening balance from the figure, if necessary
      QDate opendate = account.openingDate();
      if ( opendate >= from )
        value -= account.openingBalance();
#endif

      // place into the 'opening' column...
      assignCell( outergroup, account, 0, value );

#if 1 // FIXME: openingbalance is now a transaction
      if ( ( opendate >= from ) && ( opendate <= to ) )
      {
        // get the opening value
        MyMoneyMoney value = account.openingBalance();
        // place in the correct column
        unsigned column = columnValue(opendate) - columnValue(m_beginDate) + 1;
        assignCell( outergroup, account, column, value );
      }
#endif
    }
    else
    {
      DEBUG_OUTPUT(QString("DOES NOT INCLUDE account %1").arg(account.name()));
    }

    ++it_account;
  }
}

void PivotTable::calculateRunningSums( TInnerGroup::iterator& it_row)
{
  MyMoneyMoney runningsum = it_row.data()[0].calculateRunningSum(MyMoneyMoney(0,1));
  unsigned column = 1;
  while ( column < m_numColumns )
  {
    if ( it_row.data().count() <= column )
      throw new MYMONEYEXCEPTION(QString("Column %1 out of grid range (%2) in PivotTable::calculateRunningSums").arg(column).arg(it_row.data().count()));

    runningsum = it_row.data()[column].calculateRunningSum(runningsum);

    ++column;
  }
}

void PivotTable::calculateRunningSums( void )
{
  DEBUG_ENTER(__PRETTY_FUNCTION__);

  m_runningSumsCalculated = true;

  TGrid::iterator it_outergroup = m_grid.begin();
  while ( it_outergroup != m_grid.end() )
  {
    TOuterGroup::iterator it_innergroup = (*it_outergroup).begin();
    while ( it_innergroup != (*it_outergroup).end() )
    {
      TInnerGroup::iterator it_row = (*it_innergroup).begin();
      while ( it_row != (*it_innergroup).end() )
      {
#if 0
        MyMoneyMoney runningsum = it_row.data()[0];
        unsigned column = 1;
        while ( column < m_numColumns )
        {
          if ( it_row.data().count() <= column )
            throw new MYMONEYEXCEPTION(QString("Column %1 out of grid range (%2) in PivotTable::calculateRunningSums").arg(column).arg(it_row.data().count()));

          runningsum = ( it_row.data()[column] += runningsum );

          ++column;
        }
#endif
        calculateRunningSums( it_row );
        ++it_row;
      }
      ++it_innergroup;
    }
    ++it_outergroup;
  }
}

MyMoneyMoney PivotTable::cellBalance(const QString& outergroup, const ReportAccount& _row, unsigned _column, bool budget)
{
  if(m_runningSumsCalculated) {
    qDebug("You must not call PivotTable::cellBalance() after calling PivotTable::calculateRunningSums()");
    throw new MYMONEYEXCEPTION(QString("You must not call PivotTable::cellBalance() after calling PivotTable::calculateRunningSums()"));
  }

  // for budget reports, if this is the actual value, map it to the account which
  // holds its budget
  ReportAccount row = _row;
  if ( !budget && m_config_f.hasBudget() )
  {
    QCString newrow = m_budgetMap[row.id()];

    // if there was no mapping found, then the budget report is not interested
    // in this account.
    if ( newrow.isEmpty() )
      return MyMoneyMoney();

    row = newrow;
  }

  // ensure the row already exists (and its parental hierarchy)
  createRow( outergroup, row, true );

  // Determine the inner group from the top-most parent account
  QString innergroup( row.topParentName() );

  if ( m_numColumns <= _column )
    throw new MYMONEYEXCEPTION(QString("Column %1 out of m_numColumns range (%2) in PivotTable::cellBalance").arg(_column).arg(m_numColumns));
  if ( m_grid[outergroup][innergroup][row].count() <= _column )
    throw new MYMONEYEXCEPTION(QString("Column %1 out of grid range (%2) in PivotTable::cellBalance").arg(_column).arg(m_grid[outergroup][innergroup][row].count()));

  MyMoneyMoney balance;
  if ( budget )
    balance = m_grid[outergroup][innergroup][row].m_budget[0].cellBalance(MyMoneyMoney());
  else
    balance = m_grid[outergroup][innergroup][row][0].cellBalance(MyMoneyMoney());

  unsigned column = 1;
  while ( column < _column)
  {
    if ( m_grid[outergroup][innergroup][row].count() <= column )
      throw new MYMONEYEXCEPTION(QString("Column %1 out of grid range (%2) in PivotTable::cellBalance").arg(column).arg(m_grid[outergroup][innergroup][row].count()));

    balance = m_grid[outergroup][innergroup][row][column].cellBalance(balance);

    ++column;
  }

  return balance;
}


void PivotTable::calculateBudgetMapping( void )
{
  DEBUG_ENTER(__PRETTY_FUNCTION__);

  MyMoneyFile* file = MyMoneyFile::instance();

  // Only do this if there is at least one budget in the file
  if ( file->countBudgets() )
  {
    // Select a budget
    //
    // (Currently, we will choose the first budget in the list.  Ultimately,
    // we'll need to make this a configuration option for the user)
    QValueList<MyMoneyBudget> budgets = file->budgetList();
    const MyMoneyBudget& budget = budgets[0];

    // Dump the budget
    //kdDebug(2) << "Budget " << budget.name() << ": " << endl;

    if ( m_config_f.isIncludingBudgetActuals() )
    {
      //
      // Go through all accounts in the system to build the mapping
      //

      QValueList<MyMoneyAccount> accounts;
      file->accountList(accounts);
      QValueList<MyMoneyAccount>::const_iterator it_account = accounts.begin();
      while ( it_account != accounts.end() )
      {
        QCString id = (*it_account).id();
        QCString acid = id;

        // If the budget contains this account outright
        if ( budget.contains(id) )
        {
          // Add it to the mapping
          m_budgetMap[acid] = id;

          //kdDebug(2) << ReportAccount(acid).debugName() << " self-maps / type =" << budget.account(id).budgetlevel() << endl;
        }

        // Otherwise, search for a parent account which includes sub-accounts
        else
        {
          do
          {
            id = file->account(id).parentAccountId();
            if ( budget.contains(id) )
            {
              if ( budget.account(id).budgetSubaccounts() )
              {
                m_budgetMap[acid] = id;
                //kdDebug(2) << ReportAccount(acid).debugName() << " maps to " << ReportAccount(id).debugName() << endl;
                break;
              }
            }
          }
          while ( ! id.isEmpty() );
        }

        ++it_account;
      } // end while looping through the accounts in the file
    }

    if ( m_config_f.isIncludingBudgetActuals() )
    {
      //
      // Place the budget values into the budget grid
      //
      QValueList<MyMoneyBudget::AccountGroup> baccounts = budget.getaccounts();
      QValueList<MyMoneyBudget::AccountGroup>::const_iterator it_bacc = baccounts.begin();
      while ( it_bacc != baccounts.end() )
      {
        ReportAccount splitAccount = (*it_bacc).id();
        MyMoneyAccount::accountTypeE type = splitAccount.accountGroup();
        QString outergroup = accountTypeToString(type);

        // reverse sign to match common notation for cash flow direction, only for expense/income splits
        MyMoneyMoney reverse((splitAccount.accountType() == MyMoneyAccount::Expense) ? -1 : 1, 1);

        const QMap<QDate, MyMoneyBudget::PeriodGroup>& periods = (*it_bacc).getPeriods();
        MyMoneyMoney value = (*periods.begin()).amount() * reverse;
        unsigned column = 1;

        // based on the kind of budget it is, deal accordingly
        switch ( (*it_bacc).budgetLevel() )
        {
        case MyMoneyBudget::AccountGroup::eYearly:
          // divide the single yearly value by 12 and place it in each column

          value /= MyMoneyMoney(12,1);
        case MyMoneyBudget::AccountGroup::eNone:
        case MyMoneyBudget::AccountGroup::eMax:
        case MyMoneyBudget::AccountGroup::eMonthly:
          // place the single monthly value in each column of the report
          while ( column < m_numColumns )
          {

            assignCell( outergroup, splitAccount, column, value, true /*budget*/ );
            ++column;
          }
          break;

        case MyMoneyBudget::AccountGroup::eMonthByMonth:
          // place each value in the appropriate column
          QMap<QDate, MyMoneyBudget::PeriodGroup>::const_iterator it_period = periods.begin();
          while ( it_period != periods.end() )
          {
            value = (*it_period).amount() * reverse;
            column = (*it_period).startDate().month();
            if(column < m_numColumns) {
              assignCell( outergroup, splitAccount, column, value, true /*budget*/ );
            }

            ++it_period;
          }
          break;
        }

        ++it_bacc;
      }
    }
  } // end if there was a budget
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
          it_row.data()[column] = TCell( value );

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
          it_row.data()[column] = TCell( value );

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
  m_grid.m_total.insert( m_grid.m_total.end(), m_numColumns, TCell() );
  m_grid.m_total.m_budget.insert( m_grid.m_total.m_budget.end(), m_numColumns, TCell() );

  //
  // Outer groups
  //

  // iterate over outer groups
  TGrid::iterator it_outergroup = m_grid.begin();
  while ( it_outergroup != m_grid.end() )
  {
    (*it_outergroup).m_total.insert( (*it_outergroup).m_total.end(), m_numColumns, TCell() );
    (*it_outergroup).m_total.m_budget.insert( (*it_outergroup).m_total.m_budget.end(), m_numColumns, TCell() );

    //
    // Inner Groups
    //

    TOuterGroup::iterator it_innergroup = (*it_outergroup).begin();
    while ( it_innergroup != (*it_outergroup).end() )
    {
      (*it_innergroup).m_total.insert( (*it_innergroup).m_total.end(), m_numColumns, TCell() );
      (*it_innergroup).m_total.m_budget.insert( (*it_innergroup).m_total.m_budget.end(), m_numColumns, TCell() );

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

          MyMoneyMoney budget = it_row.data().m_budget[column];
          (*it_innergroup).m_total.m_budget[column] += budget;
          (*it_row).m_budget.m_total += budget;


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

        MyMoneyMoney budget = (*it_innergroup).m_total.m_budget[column];
        (*it_outergroup).m_total.m_budget[column] += budget;
        (*it_innergroup).m_total.m_budget.m_total += budget;

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

      MyMoneyMoney budget = (*it_outergroup).m_total.m_budget[column];
      (*it_outergroup).m_total.m_budget.m_total += budget;

      if ( invert_total )
        budget = -budget;

      m_grid.m_total.m_budget[column] += budget;

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

    MyMoneyMoney budget = m_grid.m_total.m_budget[totalcolumn];
    m_grid.m_total.m_budget.m_total += budget;

    ++totalcolumn;
  }

}

void PivotTable::assignCell( const QString& outergroup, const ReportAccount& _row, unsigned column, MyMoneyMoney value, bool budget, bool stockSplit )
{
  DEBUG_ENTER(__PRETTY_FUNCTION__);
  DEBUG_OUTPUT(QString("Parameters: %1,%2,%3,%4,%5").arg(outergroup).arg(_row.debugName()).arg(column).arg(DEBUG_SENSITIVE(value.toDouble())).arg(budget));

  // for budget reports, if this is the actual value, map it to the account which
  // holds its budget
  ReportAccount row = _row;
  if ( !budget && m_config_f.hasBudget() )
  {
    QCString newrow = m_budgetMap[row.id()];

    // if there was no mapping found, then the budget report is not interested
    // in this account.
    if ( newrow.isEmpty() )
      return;

    row = newrow;
  }

  // ensure the row already exists (and its parental hierarchy)
  createRow( outergroup, row, true );

  // Determine the inner group from the top-most parent account
  QString innergroup( row.topParentName() );

  if ( m_numColumns <= column )
    throw new MYMONEYEXCEPTION(QString("Column %1 out of m_numColumns range (%2) in PivotTable::assignCell").arg(column).arg(m_numColumns));
  if ( m_grid[outergroup][innergroup][row].count() <= column )
    throw new MYMONEYEXCEPTION(QString("Column %1 out of grid range (%2) in PivotTable::assignCell").arg(column).arg(m_grid[outergroup][innergroup][row].count()));

  if(!stockSplit) {
    // Determine whether the value should be inverted before being placed in the row
    if ( m_grid[outergroup].m_inverted )
      value = -value;

    // Add the value to the grid cell
    if ( budget )
      m_grid[outergroup][innergroup][row].m_budget[column] += value;
    else
      m_grid[outergroup][innergroup][row][column] += value;
  } else {
    m_grid[outergroup][innergroup][row][column] += TCell::stockSplit(value);
  }

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
        bool isUsed = it_row.data()[0].isUsed();
        while ( column < m_numColumns ) {
          isUsed |= it_row.data()[column].isUsed();
          rowdata += QString(",\"%1\"").arg(it_row.data()[column++].formatMoney());
        }

        if ( m_config_f.isShowingRowTotals() )
          rowdata += QString(",\"%1\"").arg((*it_row).m_total.formatMoney());

        //
        // Row Header
        //

        ReportAccount rowname = it_row.key();
        if(!rowname.isClosed() || isUsed) {
          innergroupdata += "\"" + QString().fill(' ',rowname.hierarchyDepth() - 1) + rowname.name();

          // if we don't convert the currencies to the base currency and the
          // current row contains a foreign currency, then we append the currency
          // to the name of the account
          if (!m_config_f.isConvertCurrency() && rowname.isForeignCurrency() )
            innergroupdata += QString(" (%1)").arg(rowname.currencyId());

          innergroupdata += "\"";

          if ( ! (*it_row).m_total.isZero() )
            innergroupdata += rowdata;

          innergroupdata += "\n";
        }
        ++it_row;
      }

      //
      // Inner Row Group Totals
      //

      bool finishrow = true;
      QString finalRow;
      bool isUsed = false;
      if ( m_config_f.isShowingSubAccounts() && ((*it_innergroup).size() > 1 ))
      {
        // Print the individual rows
        result += innergroupdata;

        if ( m_config_f.isShowingColumnTotals() )
        {
          // Start the TOTALS row
          finalRow = i18n("Total");
          isUsed = true;
        }
        else
        {
          ++rownum;
          finishrow = false;
        }
      }
      else
      {
        // Start the single INDIVIDUAL ACCOUNT row
        ReportAccount rowname = (*it_innergroup).begin().key();
        isUsed |= !rowname.isClosed();

        finalRow = "\"" + QString().fill(' ',rowname.hierarchyDepth() - 1) + rowname.name();
        if (!m_config_f.isConvertCurrency() && rowname.isForeignCurrency() )
          finalRow += QString(" (%1)").arg(rowname.currencyId());
        finalRow += "\"";
      }

      // Finish the row started above, unless told not to
      if ( finishrow )
      {
        unsigned column = 1;
        isUsed |= (*it_innergroup).m_total[0].isUsed();
        while ( column < m_numColumns )
        {
          isUsed |= (*it_innergroup).m_total[column].isUsed();
          finalRow += QString(",\"%1\"").arg((*it_innergroup).m_total[column++].formatMoney());
        }

        if (  m_config_f.isShowingRowTotals() )
          finalRow += QString(",\"%1\"").arg((*it_innergroup).m_total.m_total.formatMoney());

        finalRow += "\n";
      }

      if(isUsed)
      {
        result += finalRow;
        ++rownum;
      }
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
  result += QString("\n\n<table class=\"report\" cellspacing=\"0\">\n"
       "<thead><tr class=\"itemheader\">\n<th>%1</th>").arg(i18n("Account"));

  QString headerspan;
  if ( m_config_f.isIncludingBudgetActuals() )
    headerspan = " colspan=\"3\"";

  unsigned column = 1;
  while ( column < m_numColumns )
    result += QString("<th%1>%2</th>").arg(headerspan,QString(m_columnHeadings[column++]).replace(QRegExp(" "),"<br>"));

  if ( m_config_f.isShowingRowTotals() )
    result += QString("<th%1>%2</th>").arg(headerspan).arg(i18n("Total"));

  result += "</tr></thead>\n";

  //
  // "Budget/Actual" header
  //
  if ( m_config_f.isIncludingBudgetActuals() )
  {
    result += "<tr><td></td>";

    unsigned column = 1;
    while ( column < m_numColumns )
    {
      result += QString("<td>%1</td><td>%2</td><td>%3</td>").arg(i18n("Budget"),i18n("Actual"),i18n("Difference"));
      column++;
    }

    if ( m_config_f.isShowingRowTotals() )
      result += QString("<td>%1</td><td>%2</td><td>%3</td>").arg(i18n("Budget"),i18n("Actual"),i18n("Difference"));
    result += "</tr>";
  }

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
            bool isUsed = it_row.data()[0].isUsed();
            while ( column < m_numColumns )
            {
              if ( m_config_f.hasBudget() )
                rowdata += QString("<td>%1</td>")
                    .arg(coloredAmount(it_row.data().m_budget[column]));

              isUsed |= it_row.data()[column].isUsed();
              rowdata += QString("<td>%1</td>")
                  .arg(coloredAmount(it_row.data()[column]));

              if ( m_config_f.isIncludingBudgetActuals() ) {
                MyMoneyMoney diff = it_row.data().m_budget[column] - it_row.data()[column];
                rowdata += QString("<td>%1</td>")
                    .arg(coloredAmount(diff));
              }
              column++;
            }

            if ( m_config_f.isShowingRowTotals() )
            {
              if ( m_config_f.hasBudget() )
                rowdata += QString("<td>%1</td>")
                    .arg(coloredAmount((*it_row).m_budget.m_total));

              rowdata += QString("<td>%1</td>")
                  .arg(coloredAmount((*it_row).m_total));

              if ( m_config_f.isIncludingBudgetActuals() ) {
                MyMoneyMoney diff = (*it_row).m_budget.m_total - (*it_row).m_total;
                rowdata += QString("<td>%1</td>").arg(coloredAmount(diff));
              }
            }

            //
            // Row Header
            //

            ReportAccount rowname = it_row.key();

            // don't show closed accounts if they have not been used
            if(!rowname.isClosed() || isUsed) {
              innergroupdata += QString("<tr class=\"row-%1\"%2><td%3 class=\"left\" style=\"text-indent: %4.0em\">%5%6</td>")
                .arg(rownum & 0x01 ? "even" : "odd")
                .arg(rowname.isTopLevel() ? " id=\"topparent\"" : "")
                .arg("") //.arg((*it_row).m_total.isZero() ? colspan : "")  // colspan the distance if this row will be blank
                .arg(rowname.hierarchyDepth() - 1)
                .arg(rowname.name().replace(QRegExp(" "), "&nbsp;"))
                .arg((m_config_f.isConvertCurrency() || !rowname.isForeignCurrency() )?QString():QString(" (%1)").arg(rowname.currency()));

              // Don't print this row if it's going to be all zeros
              // TODO: Uncomment this, and deal with the case where the data
              // is zero, but the budget is non-zero
              //if ( !(*it_row).m_total.isZero() )
              innergroupdata += rowdata;

              innergroupdata += "</tr>\n";
            }

            ++it_row;
          }

          //
          // Inner Row Group Totals
          //

          bool finishrow = true;
          QString finalRow;
          bool isUsed = false;
          if ( m_config_f.isShowingSubAccounts() && ((*it_innergroup).size() > 1 ))
          {
            // Print the individual rows
            result += innergroupdata;

            if ( m_config_f.isShowingColumnTotals() )
            {
              // Start the TOTALS row
              finalRow = QString("<tr class=\"row-%1\" id=\"subtotal\"><td class=\"left\">&nbsp;&nbsp;%2</td>")
                .arg(rownum & 0x01 ? "even" : "odd")
                .arg(i18n("Total"));
              // don't suppress display of totals
              isUsed = true;
            }
            else
              finishrow = false;
              ++rownum;
          }
          else
          {
            // Start the single INDIVIDUAL ACCOUNT row
            // FIXME: There is a bit of a bug here with class=leftX.  There's only a finite number
            // of classes I can define in the .CSS file, and the user can theoretically nest deeper.
            // The right solution is to use style=Xem, and calculate X.  Let's see if anyone complains
            // first :)  Also applies to the row header case above.
            // FIXED: I found it in one of my reports and changed it to the proposed method.
            // This works for me (ipwizard)
            ReportAccount rowname = (*it_innergroup).begin().key();
            isUsed |= !rowname.isClosed();
            finalRow = QString("<tr class=\"row-%1\"%2><td class=\"left\" style=\"text-indent: %3.0em;\">%5%6</td>")
              .arg(rownum & 0x01 ? "even" : "odd")
              .arg( m_config_f.isShowingSubAccounts() ? "id=\"solo\"" : "" )
              .arg(rowname.hierarchyDepth() - 1)
              .arg(rowname.name().replace(QRegExp(" "), "&nbsp;"))
              .arg((m_config_f.isConvertCurrency() || !rowname.isForeignCurrency() )?QString():QString(" (%1)").arg(rowname.currency()));
          }

          // Finish the row started above, unless told not to
          if ( finishrow )
          {
            unsigned column = 1;
            isUsed |= (*it_innergroup).m_total[0].isUsed();
            while ( column < m_numColumns )
            {
              if ( m_config_f.hasBudget())
                finalRow += QString("<td>%1</td>")
                    .arg(coloredAmount((*it_innergroup).m_total.m_budget[column]));

              isUsed |= (*it_innergroup).m_total[column].isUsed();
              finalRow += QString("<td>%1</td>")
                  .arg(coloredAmount((*it_innergroup).m_total[column]));

              if ( m_config_f.isIncludingBudgetActuals() ) {
                MyMoneyMoney diff = (*it_innergroup).m_total.m_budget[column] - (*it_innergroup).m_total[column];
                finalRow += QString("<td>%1</td>").arg(coloredAmount(diff));
              }
              column++;
            }

            if (  m_config_f.isShowingRowTotals() )
            {
              if ( m_config_f.hasBudget() )
                finalRow += QString("<td>%1</td>")
                    .arg(coloredAmount((*it_innergroup).m_total.m_budget.m_total));

              finalRow += QString("<td>%1</td>")
                  .arg(coloredAmount((*it_innergroup).m_total.m_total));

              if ( m_config_f.isIncludingBudgetActuals() ) {
                MyMoneyMoney diff = (*it_innergroup).m_total.m_budget.m_total - (*it_innergroup).m_total.m_total;
                finalRow += QString("<td>%1</td>").arg(coloredAmount(diff));
              }
            }

            finalRow += "</tr>\n";
            if(isUsed) {
              result += finalRow;
              ++rownum;
            }
          }

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
        {
          if ( m_config_f.hasBudget() )
            result += QString("<td>%1</td>")
                .arg(coloredAmount((*it_outergroup).m_total.m_budget[column]));

          result += QString("<td>%1</td>")
              .arg(coloredAmount((*it_outergroup).m_total[column]));

          if ( m_config_f.isIncludingBudgetActuals() ) {
            MyMoneyMoney diff = (*it_outergroup).m_total.m_budget[column] - (*it_outergroup).m_total[column];
            result += QString("<td>%1</td>").arg(coloredAmount(diff));
          }
          column++;
        }

        if (  m_config_f.isShowingRowTotals() )
        {
          if ( m_config_f.hasBudget() )
            result += QString("<td>%1</td>")
                .arg(coloredAmount((*it_outergroup).m_total.m_budget.m_total));

          result += QString("<td>%1</td>")
              .arg(coloredAmount((*it_outergroup).m_total.m_total));

          if ( m_config_f.isIncludingBudgetActuals() ) {
            MyMoneyMoney diff = (*it_outergroup).m_total.m_budget.m_total - (*it_outergroup).m_total.m_total;
            result += QString("<td>%1</td>").arg(coloredAmount(diff));
          }
        }

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
    {
      if ( m_config_f.hasBudget() )
        result += QString("<td>%1</td>")
            .arg(coloredAmount(m_grid.m_total.m_budget[totalcolumn]));

      result += QString("<td>%1</td>")
          .arg(coloredAmount(m_grid.m_total[totalcolumn]));

      if ( m_config_f.isIncludingBudgetActuals() ) {
        MyMoneyMoney diff = m_grid.m_total.m_budget[totalcolumn] - m_grid.m_total[totalcolumn];
        result += QString("<td>%1</td>").arg(coloredAmount(diff));
      }
      totalcolumn++;
    }

    if (  m_config_f.isShowingRowTotals() )
    {
      if ( m_config_f.hasBudget())
        result += QString("<td>%1</td>")
            .arg(coloredAmount(m_grid.m_total.m_budget.m_total));

      result += QString("<td>%1</td>")
          .arg(coloredAmount(m_grid.m_total.m_total));

      if ( m_config_f.isIncludingBudgetActuals() ) {
        MyMoneyMoney diff = m_grid.m_total.m_budget.m_total - m_grid.m_total.m_total;
        result += QString("<td>%1</td>").arg(coloredAmount(diff));
      }
    }

    result += "</tr>\n";
  }

  result += QString("<tr class=\"spacer\"><td>&nbsp;</td></tr>\n");
  result += QString("<tr class=\"spacer\"><td>&nbsp;</td></tr>\n");
  result += "</table>\n";

  return result;
}

void PivotTable::dump( const QString& file, const QString& /* context */) const
{
  QFile g( file );
  g.open( IO_WriteOnly );
  QTextStream(&g) << renderHTML();
  g.close();
}

#ifdef HAVE_KDCHART
void PivotTable::drawChart( KReportChartView& _view ) const
{
#if 1 // make this "#if 1" if you want to play with the axis settings
  // not sure if 0 is X and 1 is Y.
  KDChartAxisParams xAxisParams, yAxisParams;
  KDChartAxisParams::deepCopy(xAxisParams, _view.params().axisParams(0));
  KDChartAxisParams::deepCopy(yAxisParams, _view.params().axisParams(1));

  // modify axis settings here
  xAxisParams.setAxisLabelsFontMinSize(12);
  xAxisParams.setAxisLabelsFontRelSize(20);
  yAxisParams.setAxisLabelsFontMinSize(12);
  yAxisParams.setAxisLabelsFontRelSize(20);

  _view.params().setAxisParams( 0, xAxisParams );
  _view.params().setAxisParams( 1, yAxisParams );
#endif
  _view.params().setLegendFontRelSize(20);
  _view.params().setLegendTitleFontRelSize(24);

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
    c = m_numColumns - 1;
  }
  else
  {
    c = 1;
    r = m_numColumns - 1;
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
              data.expand( ++rownum, m_numColumns-1 );
            else
              data.expand( m_numColumns-1, ++rownum );

            ++it_row;
          }
          ++it_innergroup;
        }
        ++it_outergroup;
      }

      if ( accountseries )
        data.expand( rownum-1, m_numColumns-1 );
      else
        data.expand( m_numColumns-1, rownum-1 );

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
            data.expand( ++rownum, m_numColumns-1 );
          else
            data.expand( m_numColumns-1, ++rownum );

          ++it_innergroup;
        }
        ++it_outergroup;
      }
      if ( accountseries )
        data.expand( rownum-1, m_numColumns-1 );
      else
        data.expand( m_numColumns-1, rownum-1 );

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
          data.expand( ++rownum, m_numColumns-1 );
        else
          data.expand( m_numColumns-1, ++rownum );

        ++it_outergroup;
      }
      if ( accountseries )
        data.expand( rownum-1, m_numColumns-1 );
      else
        data.expand( m_numColumns-1, rownum-1 );
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

#if 0
  // I have not been able to get this to work (ace)

  //
  // Set line to dashed for the future
  //

  if ( accountseries )
  {
    // the first column of report which represents a date in the future, or one past the
    // last column if all columns are in the present day. Only relevant when accountseries==true
    unsigned futurecolumn = columnValue(QDate::currentDate()) - columnValue(m_beginDate) + 1;

    // kdDebug(2) << "futurecolumn: " << futurecolumn << endl;
    // kdDebug(2) << "m_numColumns: " << m_numColumns << endl;

    // Properties for line charts whose values are in the future.
    KDChartPropertySet propSetFutureValue("future value", KDChartParams::KDCHART_PROPSET_NORMAL_DATA);
    propSetFutureValue.setLineStyle(KDChartPropertySet::OwnID, Qt::DotLine);
    const int idPropFutureValue = _view.params().registerProperties(propSetFutureValue);

    for(int col = futurecolumn; col < m_numColumns; ++col) {
      _view.setProperty(0, col, idPropFutureValue);
    }

  }
#endif
}
#else
void PivotTable::drawChart( KReportChartView& ) const { }
#endif

QString PivotTable::coloredAmount(const MyMoneyMoney& amount, const QString& currencySymbol, int prec) const
{
  QString result;
  if( amount.isNegative() )
    result += QString("<font color=\"rgb(%1,%2,%3)\">")
        .arg(KMyMoneyGlobalSettings::listNegativeValueColor().red())
        .arg(KMyMoneyGlobalSettings::listNegativeValueColor().green())
        .arg(KMyMoneyGlobalSettings::listNegativeValueColor().blue());
  result += amount.formatMoney(currencySymbol, prec);
  if( amount.isNegative() )
    result += QString("</font>");
  return result;
}


} // namespace
// vim:cin:si:ai:et:ts=2:sw=2:
