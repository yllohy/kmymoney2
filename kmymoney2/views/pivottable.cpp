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

// (Ace) The reports are starting to proliferate helper classes, and clutter the
// views class tree.  So I created a separate namespace for them.

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
    default:
      returnString = i18n("Unknown");
  }

  return returnString;
}

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

PivotTable::PivotTable( const ReportConfiguration& config ):
  m_config( config )
{
  DEBUG_ENTER("PivotTable::PivotTable");

  // TODO: Merge the reportconfiguration class into the pivottable class
  
  //
  // Initialize member variables
  //

  // strip out the 'days' component of the begin and end dates.
  // we're only concerned about the month & year.
  m_beginDate =  QDate( config.getStart().year(), config.getStart().month(), 1 );
  QDate e = config.getEnd().addDays(1);
  m_endDate = QDate( e.year(), e.month(), 1 );

  m_displayRowTotals = !config.getRunningSum();
  m_showSubCategories = config.getShowSubAccounts();
  m_name = config.getName();

  //
  // Initialize locals
  //

  MyMoneyFile* file = MyMoneyFile::instance();

  QValueList<MyMoneyAccount::accountTypeE> accounttypes;
  unsigned int configaccounts = config.getRowFilter();
  if ( configaccounts == ReportConfiguration::eAssetLiability )
  {
    accounttypes.append(MyMoneyAccount::Asset);
    accounttypes.append(MyMoneyAccount::Liability);
  }
  if ( configaccounts == ReportConfiguration::eExpenseIncome )
  {
    accounttypes.append(MyMoneyAccount::Expense);
    accounttypes.append(MyMoneyAccount::Income);
  }

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

  if ( config.getRunningSum() )
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
    
    // Include this transaction if ANY of the asset/liability
    // accounts are included in the configuration filter.
    QValueList<MyMoneySplit> splits = (*it_transaction).splits();
    QValueList<MyMoneySplit>::const_iterator it_split = splits.begin();
    bool include = false;
    while ( it_split != splits.end() )
    {
      MyMoneyAccount splitAccount = file->account((*it_split).accountId());
      if ( ( ( splitAccount.accountGroup() == MyMoneyAccount::Asset) ||
             ( splitAccount.accountGroup() == MyMoneyAccount::Liability) )
           && m_config.includesAccount(splitAccount.id()) )
      {
        include = true;
        break;
      }
      ++it_split;
    }

    if ( include )
    {
      it_split = splits.begin();
      while ( it_split != splits.end() )
      {
        MyMoneyAccount splitAccount = file->account((*it_split).accountId());
        // only reverse sign for expense/income splits
        MyMoneyMoney reverse((splitAccount.accountGroup() == MyMoneyAccount::Income) |
                             (splitAccount.accountGroup() == MyMoneyAccount::Expense) ? -1 : 1, 1);

        // Include this split only if the accounts is included in the configuration filter.
        // only include this item if its account group is included in this report
        if ( accounttypes.contains(splitAccount.accountGroup()) && m_config.includesAccount(splitAccount.id()) )
        {
          // reverse sign to match common notation for cash flow direction
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
    }
    ++it_transaction;
  }

  //
  // Calculate the running sums
  // (for running sum reports only)
  //

  if ( config.getRunningSum() )
    calculateRunningSums();

  //
  // Convert all values to the base currency
  //

  convertToBaseCurrency();

  //
  // Calculate row and column totals
  //

  calculateTotals();
}

void PivotTable::calculateOpeningBalances( const QValueList<MyMoneyAccount::accountTypeE>& accounttypes )
{
  DEBUG_ENTER("PivotTable::calculateOpeningBalances");

  MyMoneyFile* file = MyMoneyFile::instance();

  const QValueList<MyMoneyAccount>& accounts = file->accountList();
  QValueList<MyMoneyAccount>::const_iterator it_account = accounts.begin();

  while ( it_account != accounts.end() )
  {
    // only include this item if its account group is included in this report
    // and if the report includes this account
    if ( accounttypes.contains((*it_account).accountGroup()) && m_config.includesAccount((*it_account).id()))
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

        int column = 0;
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

      int column = 0;
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

    int column = 0;
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

  int totalcolumn = 0;
  while ( totalcolumn < m_numColumns )
  {
    MyMoneyMoney value = m_grid.m_total[totalcolumn];
    m_grid.m_total.m_total += value;

    ++totalcolumn;
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

  // Add the value to the grid cell
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

    m_grid[outergroup][innergroup].m_total.insert( m_grid[outergroup][innergroup].m_total.end(), m_numColumns, 0 );
    m_grid[outergroup].m_total.insert( m_grid[outergroup].m_total.end(), m_numColumns, 0 );
    m_grid.m_total.insert( m_grid.m_total.end(), m_numColumns, 0 );

    if ( recursive && !row.isTopLevel() )
        createRow( outergroup, row.getParent(), recursive );
  }
}

QString PivotTable::renderHTML( void ) const
{
  DEBUG_ENTER("PivotTable::renderHTML");

  QString colspan = QString(" colspan=\"%1\"").arg(m_numColumns + 1 + (m_displayRowTotals ? 1 : 0) );

  //
  // Report Title
  //

  QString result = QString("<h2 class=\"report\">%1</h2>\n").arg(m_name);
  result += QString("<div class=\"subtitle\">");
  result += i18n("All currencies converted to %1").arg(MyMoneyFile::instance()->baseCurrency().name());
  result += QString("</div>\n");
  result += QString("<div class=\"gap\">&nbsp;</div>\n");

  //
  // Table Header
  //
  
  result += QString("\n\n<table class=\"report\" cellspacing=\"0\">"
      "<tr class=\"itemheader\">\n<th>%1</th>").arg(i18n("Account"));

  int column = 1;
  while ( column < m_numColumns )
    result += QString("<th>%1</th>").arg(m_columnHeadings[column++]);

  if ( m_displayRowTotals )
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
    int rownum = 0;
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
        int column = 1;
        while ( column < m_numColumns )
          rowdata += QString("<td>%1</td>").arg(it_row.data()[column++].formatMoney());

        if ( m_displayRowTotals )
          rowdata += QString("<td>%1</td>").arg((*it_row).m_total.formatMoney());

        //
        // Row Header
        //

        AccountDescriptor rowname = it_row.key();

        innergroupdata += QString("<tr class=\"row-%1\"%2><td class=\"left\"%3>%4</td>")
          .arg(rownum & 0x01 ? "even" : "odd")
          .arg(rowname.isTopLevel() ? " id=\"topparent\"" : "")
          .arg((*it_row).m_total == 0 ? colspan : "")
          .arg(rowname.htmlTabbedName());

        if ( (*it_row).m_total != 0 )
          innergroupdata += rowdata;

        innergroupdata += "</tr>\n";

        ++it_row;
      }

      //
      // Inner Row Group Totals
      //

      if ( m_showSubCategories && ((*it_innergroup).size() > 1 ))
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
          .arg(m_showSubCategories ? "id=\"solo\"" : "" )
          .arg(rowname.htmlTabbedName());
      }

      int column = 1;
      while ( column < m_numColumns )
        result += QString("<td>%1</td>").arg((*it_innergroup).m_total[column++].formatMoney());

      if ( m_displayRowTotals )
        result += QString("<td>%1</td>").arg((*it_innergroup).m_total.m_total.formatMoney());

      result += "</tr>\n";

      ++rownum;
      ++it_innergroup;
    }

    //
    // Outer Row Group Totals
    //

    result += QString("<tr class=\"sectionfooter\"><td class=\"left\">%1&nbsp;%2</td>").arg(i18n("Total")).arg(it_outergroup.key());
    int column = 1;
    while ( column < m_numColumns )
      result += QString("<td>%1</td>").arg((*it_outergroup).m_total[column++].formatMoney());

    if ( m_displayRowTotals )
      result += QString("<td>%1</td>").arg((*it_outergroup).m_total.m_total.formatMoney());

    result += "</tr>\n";

    ++it_outergroup;
  }

  //
  // Report Totals
  //

  result += QString("<tr class=\"spacer\"><td>&nbsp;</td></tr>\n");
  result += QString("<tr class=\"reportfooter\"><td class=\"left\">%1</td>").arg(i18n("Grand Total"));
  int totalcolumn = 1;
  while ( totalcolumn < m_numColumns )
    result += QString("<td>%1</td>").arg(m_grid.m_total[totalcolumn++].formatMoney());

  if ( m_displayRowTotals )
    result += QString("<td>%1</td>").arg(m_grid.m_total.m_total.formatMoney());

  result += "</tr>\n";

  result += QString("<tr class=\"spacer\"><td>&nbsp;</td></tr>\n");
  result += QString("<tr class=\"spacer\"><td>&nbsp;</td></tr>\n");
  result += "</table>\n";

  return result;
}

}
