/***************************************************************************
                          pivottable.cpp  -  description
                             -------------------
    begin                : Mon May 17 2004
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
    default:
      returnString = i18n("Unknown");
  }

  return returnString;
}

QString Tester::m_sTabs;
bool Tester::m_sEnabled = DEBUG_ENABLED_BY_DEFAULT;

Tester::Tester( const QString& _name ): m_methodName( _name ), m_enabled( m_sEnabled )
{
  if (m_enabled)
  {
    qDebug( "%s%s(): ENTER", m_sTabs.latin1(), m_methodName.latin1() );
    m_sTabs.append("  ");

  }
}

Tester::~Tester()
{
  if ( m_enabled )
  {
    m_sTabs.remove(0,2);
    qDebug( "%s%s(): EXIT", m_sTabs.latin1(), m_methodName.latin1() );
  }
}

void Tester::output( const QString& _text )
{
  if ( m_enabled )
    qDebug( "%s%s(): %s", m_sTabs.latin1(), m_methodName.latin1(), _text.latin1() );
}

PivotTable::AccountDescriptor::AccountDescriptor( void )
{
  DEBUG_ENTER("AccountDescriptor::AccountDescriptor( void )");
  m_file = MyMoneyFile::instance();
}

PivotTable::AccountDescriptor::AccountDescriptor( const AccountDescriptor& copy ):
  m_account( copy.m_account ), m_names( copy.m_names )
{
  // NOTE: I implemented the copy constructor solely for debugging reasons

  DEBUG_ENTER("AccountDescriptor::AccountDescriptor( const AccountDescriptor&  )");
  m_file = MyMoneyFile::instance();
}

PivotTable::AccountDescriptor::AccountDescriptor( const QCString& accountid ): m_account( accountid )
{
  DEBUG_ENTER("AccountDescriptor::AccountDescriptor( account )");
  m_file = MyMoneyFile::instance();
  calculateAccountHierarchy();
}

void PivotTable::AccountDescriptor::calculateAccountHierarchy( void )
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

MyMoneyMoney PivotTable::AccountDescriptor::currencyPrice(const QDate& date) const
{
  DEBUG_ENTER("AccountDescriptor::currencyPrice");

  MyMoneyMoney value(1.0);

  MyMoneyAccount account = m_file->account(m_account);

  if(account.currencyId() != m_file->baseCurrency().id()) {
    QString name;
    if(account.accountType() == MyMoneyAccount::Stock) {
      MyMoneyEquity equity = m_file->equity(account.currencyId());
      name = equity.name();
      value = equity.price(date);
    } else {
      MyMoneyCurrency currency = m_file->currency(account.currencyId());
      name = currency.name();
      value = currency.price(date);
    }

    DEBUG_OUTPUT(QString("Converting %1 to %2, price on %3 is %4").arg(name).arg(m_file->baseCurrency().name()).arg(date.toString()).arg(value.toDouble()));
  }

  return value;
}

bool PivotTable::AccountDescriptor::operator<(const AccountDescriptor& second) const
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

QString PivotTable::AccountDescriptor::htmlTabbedName( bool showcurrency, bool htmlformat ) const
{
  QString result = m_names.back();
  unsigned tabs = m_names.size() - 1;
  while ( tabs-- )
    result.prepend("  ");

  if ( showcurrency )
  {
    MyMoneyAccount account = m_file->account(m_account);
    if ( account.currencyId() != m_file->baseCurrency().id() )
      if(account.accountType() == MyMoneyAccount::Stock)
        result.append(QString(" (%1)").arg(m_file->equity(account.currencyId()).tradingSymbol()));
      else
        result.append(QString(" (%1)").arg(m_file->currency(account.currencyId()).tradingSymbol()));
  }
  if ( htmlformat )
    result.replace(QRegExp(" "),QString("&nbsp;"));

  return result;
}

QString PivotTable::AccountDescriptor::debugName( void ) const
{
  return m_names.join("|");
}

bool PivotTable::AccountDescriptor::isTopLevel( void ) const
{
  return ( m_names.size() == 1 );
}

PivotTable::AccountDescriptor PivotTable::AccountDescriptor::getParent( void ) const
{
  return AccountDescriptor( m_file->account(m_account).parentAccountId() );
}

QString PivotTable::AccountDescriptor::getTopLevel( void ) const
{
  return m_names.first();
}

PivotTable::PivotTable( const MyMoneyReport& _config_f ):
  m_config_f( _config_f )
{
  DEBUG_ENTER("PivotTable::PivotTable()");

  //
  // Initialize locals
  //

  MyMoneyFile* file = MyMoneyFile::instance();

  //
  // Initialize member variables
  //

  if ( m_config_f.rowType() == MyMoneyReport::eAssetLiability )
  {
    m_accounttypes.append(MyMoneyAccount::Asset);
    m_accounttypes.append(MyMoneyAccount::Liability);
  }
  if ( m_config_f.rowType() == MyMoneyReport::eExpenseIncome )
  {
    m_accounttypes.append(MyMoneyAccount::Expense);
    m_accounttypes.append(MyMoneyAccount::Income);
  }

  m_beginDate = m_config_f.fromDate();
  m_endDate = m_config_f.toDate();


  // if either begin or end date are invalid we have one of the following
  // possible date filters:
  //
  // a) begin date not set - first transaction until given end date
  // b) end date not set   - from given date until last transaction
  // c) both not set       - first transaction until last transaction
  //
  // If there is no transaction in the engine at all, we use the current
  // year as the filter criteria.

  if ( !m_beginDate.isValid() || !m_endDate.isValid()) {
    QValueList<MyMoneyTransaction> list = file->transactionList(m_config_f);
    QDate tmpBegin, tmpEnd;
    
    if(!list.isEmpty()) {
      qHeapSort(list);
      tmpBegin = list.front().postDate();
      tmpEnd = list.back().postDate();
    } else {
      tmpBegin = QDate(QDate::currentDate().year(),1,1); // the first date in the file
      tmpEnd = QDate(QDate::currentDate().year(),12,31);// the last date in the file
    }
    if(!m_beginDate.isValid())
      m_beginDate = tmpBegin;
    if(!m_endDate.isValid())
      m_endDate = tmpEnd;
  }
  if ( m_beginDate > m_endDate )
    m_beginDate = m_endDate;

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

  DEBUG_OUTPUT(QString("Matched %1 transactions").arg(transactions.count()));
  while ( it_transaction != transactions.end() )
  {
    QDate postdate = (*it_transaction).postDate();
    unsigned column = postdate.year() * 12 + postdate.month() - colofs;

    QValueList<MyMoneySplit> splits = (*it_transaction).splits();
    QValueList<MyMoneySplit>::const_iterator it_split = splits.begin();
    while ( it_split != splits.end() )
    {
      MyMoneyAccount splitAccount = file->account((*it_split).accountId());

      // Include this split only if the accounts is included in the configuration filter
      // AND if its account group is included in this report type
      if ( includesAccount( splitAccount ) )
      {
        // reverse sign to match common notation for cash flow direction, only for expense/income splits
        MyMoneyMoney reverse((splitAccount.accountGroup() == MyMoneyAccount::Income) |
                          (splitAccount.accountGroup() == MyMoneyAccount::Expense) ? -1 : 1, 1);
        MyMoneyMoney value = (*it_split).value((*it_transaction).commodity(), splitAccount.currencyId())*reverse;

        // the outer group is the account class (major account type)
        QString outergroup = accountTypeToString(splitAccount.accountGroup());

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

  if ( m_config_f.isRunningSum() )
    calculateRunningSums();

  //
  // Convert all values to the base currency
  //

  if ( m_config_f.isConvertCurrency() )
    convertToBaseCurrency();

  //
  // Collapse columns to match column type
  //

  if ( m_config_f.columnType() != MyMoneyReport::eMonths )
    collapseColumns();

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
        // TODO: Clean up this rather inefficient kludge. We really should jump by an entire
        // destcolumn at a time on RS reports, and calculate the proper sourcecolumn to use,
        // allowing us to clear and accumulate only ONCE per destcolumn
        if ( m_config_f.isRunningSum() )
          clearColumn(destcolumn);
          
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
        (*it_row++)[column] = 0;

      ++it_innergroup;
    }
    ++it_outergroup;
  }
}

void PivotTable::calculateColumnHeadings(void)  
{  
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
  DEBUG_ENTER("PivotTable::calculateOpeningBalances");

  MyMoneyFile* file = MyMoneyFile::instance();

  const QValueList<MyMoneyAccount>& accounts = file->accountList();

  QValueList<MyMoneyAccount>::const_iterator it_account = accounts.begin();

  while ( it_account != accounts.end() )
  {
    // only include this item if its account group is included in this report
    // and if the report includes this account
    if ( includesAccount( *it_account ) )
    {
      // extract the balance of the account for the given begin date
      MyMoneyMoney value = file->balance((*it_account).id(), m_beginDate.addDays(-1));

      // place into the 'opening' column...
      QDate opendate = (*it_account).openingDate();
      unsigned column = 0;
      
      // unless the account was opened after the start of the report period
      if ( opendate >= m_beginDate )
      {
        if ( opendate <= m_endDate ) {
          // in which case it should be placed in the correct column
          column = opendate.year() * 12 + opendate.month() - m_beginDate.year() * 12 - m_beginDate.month() + 1;
        } else
          // or disregarded if the account was opened after the end of the
          // report period
          value = 0;
      }

      // the row group is the account class (major account type)
      QString outergroup = accountTypeToString((*it_account).accountGroup());

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
        unsigned column = 1;
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
        unsigned column = 1;
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
    MyMoneyMoney value = m_grid.m_total[totalcolumn];
    m_grid.m_total.m_total += value;

    ++totalcolumn;
  }

}

void PivotTable::assignCell( const QString& outergroup, const PivotTable::AccountDescriptor& row, unsigned column, MyMoneyMoney value )
{
  DEBUG_ENTER("PivotTable::assignCell");
  DEBUG_OUTPUT(QString("Parameters: %1,%2,%3,%4").arg(outergroup).arg(row.debugName()).arg(column).arg(value.toDouble()));

  // ensure the row already exists (and its parental hierarchy)
  createRow( outergroup, row, true );

  // Determine the inner group from the top-most parent account
  QString innergroup( row.getTopLevel() );

  // Add the value to the grid cell
  m_grid[outergroup][innergroup][row][column] += value;

}

void PivotTable::createRow( const QString& outergroup, const PivotTable::AccountDescriptor& row, bool recursive )
{
  DEBUG_ENTER("PivotTable::createRow");

  // Determine the inner group from the top-most parent account
  QString innergroup( row.getTopLevel() );

  // fill the row list with blanks if it doesn't already exist.
  if ( m_grid[outergroup][innergroup][row].isEmpty() )
  {
    DEBUG_OUTPUT(QString("m_grid[%1][%2][%3].insert(%4)").arg(outergroup).arg(innergroup).arg(row.debugName()).arg(m_numColumns));

    m_grid[outergroup][innergroup][row].insert( m_grid[outergroup][innergroup][row].end(), m_numColumns, 0 );

    m_grid[outergroup][innergroup].m_total.insert( m_grid[outergroup][innergroup].m_total.end(), m_numColumns, 0 );
    m_grid[outergroup].m_total.insert( m_grid[outergroup].m_total.end(), m_numColumns, 0 );
    m_grid.m_total.insert( m_grid.m_total.end(), m_numColumns, 0 );

    if ( recursive && !row.isTopLevel() )
        createRow( outergroup, row.getParent(), recursive );
  }
}

bool PivotTable::isCategory(const MyMoneyAccount& account)
{
  return ( (account.accountGroup() == MyMoneyAccount::Income) || (account.accountGroup() == MyMoneyAccount::Expense) );
  
}

bool PivotTable::includesAccount( const MyMoneyAccount& account ) const
{
  bool result = false;

  if
  (
    m_accounttypes.contains(account.accountGroup())
    &&
    (
      ( isCategory(account) && m_config_f.includesCategory(account.id()) )
      ||
      ( !isCategory(account) && m_config_f.includesAccount(account.id()) )
    )
  )
    result = true;

  return result;
}

QString PivotTable::renderCSV( void ) const
{
  DEBUG_ENTER("PivotTable::renderHTML");

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
          rowdata += QString(",%1").arg(it_row.data()[column++].formatMoney());

        if ( m_config_f.isShowingRowTotals() )
          rowdata += QString(",%1").arg((*it_row).m_total.formatMoney());

        //
        // Row Header
        //

        AccountDescriptor rowname = it_row.key();

        innergroupdata += "\"" + rowname.htmlTabbedName(!m_config_f.isConvertCurrency(),false) + "\"";

        if ( (*it_row).m_total != 0 )
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
        AccountDescriptor rowname = (*it_innergroup).begin().key();
        result += "\"" + rowname.htmlTabbedName(!m_config_f.isConvertCurrency(),false) + "\"";
      }

      // Finish the row started above, unless told not to
      if ( finishrow )
      {
        unsigned column = 1;
        while ( column < m_numColumns )
          result += QString(",%1").arg((*it_innergroup).m_total[column++].formatMoney());
  
        if (  m_config_f.isShowingRowTotals() )
          result += QString(",%1").arg((*it_innergroup).m_total.m_total.formatMoney());
  
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
        result += QString(",%1").arg((*it_outergroup).m_total[column++].formatMoney());
  
      if (  m_config_f.isShowingRowTotals() )
        result += QString(",%1").arg((*it_outergroup).m_total.m_total.formatMoney());
  
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
      result += QString(",%1").arg(m_grid.m_total[totalcolumn++].formatMoney());
  
    if (  m_config_f.isShowingRowTotals() )
      result += QString(",%1").arg(m_grid.m_total.m_total.formatMoney());
  
    result += "\n";
  }
  
  return result;
  
}

QString PivotTable::renderHTML( void ) const
{
  DEBUG_ENTER("PivotTable::renderHTML");

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
    result += QString("<th>%1</th>").arg(QString(m_columnHeadings[column++]).replace(" ","<br>"));

  if ( m_config_f.isShowingRowTotals() )
    result += QString("<th>%1</th>").arg(i18n("Total"));

  result += "</tr>\n";

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

        AccountDescriptor rowname = it_row.key();

        innergroupdata += QString("<tr class=\"row-%1\"%2><td class=\"left\"%3>%4</td>")
          .arg(rownum & 0x01 ? "even" : "odd")
          .arg(rowname.isTopLevel() ? " id=\"topparent\"" : "")
          .arg((*it_row).m_total == 0 ? colspan : "")
          .arg(rowname.htmlTabbedName(!m_config_f.isConvertCurrency()));

        if ( (*it_row).m_total != 0 )
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
        AccountDescriptor rowname = (*it_innergroup).begin().key();
        result += QString("<tr class=\"row-%1\"%2><td class=\"left\">%3</td>")
          .arg(rownum & 0x01 ? "even" : "odd")
          .arg( m_config_f.isShowingSubAccounts() ? "id=\"solo\"" : "" )
          .arg(rowname.htmlTabbedName(!m_config_f.isConvertCurrency()));
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
    }

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
  }

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

}
