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

/**
  * Holds a 'pivot table' of information about the transaction database.
  * Based on pivot tables in MS Excel, and implemented as 'Data Pilot' in
  * OpenOffice.Org Calc.
  *
  * Currently only creates and holds one view, but can be extended to hold
  * different varieties and take different options in the future.
  *
  * Constructs the following pivot table, where type = income or expense, and
  * year = current year
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
    typedef QValueList<MyMoneyMoney> TGridRow;
    typedef QMap<QString,TGridRow> TRowGroup;
    typedef QMap<QString,TRowGroup> TGrid;

    TGrid m_grid;
    QStringList m_columnHeadings;
    bool m_displayRowTotals;
    int m_numColumns;

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
    PivotTable( const MyMoneyFile* file,const QDate& begindate, const QDate& enddate, const QValueList<MyMoneyAccount::accountTypeE>& accounttypes, bool runningsum );

  /**
    * Render the report to an HTML stream.
    *
    * @return QString HTML string representing the report
    */
    QString renderHTML( void ) const;

protected:
  /**
    * Returns the top level account
    *
    * For example, if 'Hotel' is a subcategory of the 'Travel' expense
    * account, this method will return the accountid for 'Travel'.
    *
    * @param accountid ID for the account in question
    * @return QCString ID for the top level account
    */
    QCString topLevelAccount( const QCString& accountid ) const;

  /**
    * Record the opening balances of all qualifying accounts into the grid.
    *
    * For accounts opened before the report period, places the balance into the '0' column.
    * For those opened during the report period, places the balance into the appropriate column
    * for the month when it was opened.
    *
    * @param file Pointer to the current MyMoneyFile
    * @param begindate First date of the range to include transactions (only month and year are used, day is assumed to be '1').
    * @param enddate One date past the range to include transactions (only month and year are used, day is assumed to be '1').
    * @param accounttypes Which account types to include.  Valid values are major account types: MyMoneyAccount::Asset, Liability, Income, Expense.
    */
    void calculateOpeningBalances( const MyMoneyFile* file, const QDate& begindate, const QDate& enddate, const QValueList<MyMoneyAccount::accountTypeE>& accounttypes );

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
    * Convert an amount in a possible different currency to the base value
    * using the price for a given date. If the currency of the account
    * is already the base currency, then the amount is returned unchanged.
    *
    * @param file pointer to MyMoneyFile object
    * @param amount original amount in possible foreign currency
    * @param acc the account for which the conversion should be performed
    * @param date the date for which the conversion should be performed
    *
    * @return MyMoneyMoney object containing the converted value
    */
  const MyMoneyMoney baseValue(const MyMoneyFile* file, const MyMoneyMoney& amount, const MyMoneyAccount& acc, const QDate& date) const;

};

PivotTable::PivotTable( const MyMoneyFile* file, const QDate& _begindate, const QDate& _enddate, const QValueList<MyMoneyAccount::accountTypeE>& accounttypes, bool runningsum )
{
  // strip out the 'days' component of the begin and end dates.
  // we're only concerned about the month & year.
  QDate begindate( _begindate.year(), _begindate.month(), 1 );
  QDate enddate( _enddate.year(), _enddate.month(), 1 );

  m_displayRowTotals = !runningsum;

  //
  // Determine column headings
  //

  // one column for the opening balance
  m_columnHeadings.append( "Opening" );

  // one for each month in the date range 
  QDate columndate = begindate;
  while ( columndate < enddate )
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
    calculateOpeningBalances( file, begindate, enddate, accounttypes );

  //
  // Populate all transactions into the row/column pivot grid
  //

  MyMoneyTransactionFilter filter;
  filter.setReportAllSplits(false);
  filter.setDateFilter(begindate, enddate);
  QValueList<MyMoneyTransaction> transactions = file->transactionList(filter);
  QValueList<MyMoneyTransaction>::const_iterator it_transaction = transactions.begin();
  int colofs = begindate.year() * 12 + begindate.month() - 1;
  
  while ( it_transaction != transactions.end() )
  {
    int column;

    QDate postdate = (*it_transaction).postDate();

    column = postdate.year() * 12 + postdate.month() - colofs;

    QValueList<MyMoneySplit> splits = (*it_transaction).splits();
    QValueList<MyMoneySplit>::const_iterator it_split = splits.begin();
    while ( it_split != splits.end() )
    {
      MyMoneyAccount splitAccount = file->account((*it_split).accountId());
      // only reverse sign for expense/income splits
      MyMoneyMoney reverse((splitAccount.accountGroup() == MyMoneyAccount::Income) |
                           (splitAccount.accountGroup() == MyMoneyAccount::Expense) ? -1 : 1, 1);

      // TODO: Make this a protected method

      // only include this item if its account group is included in this report
      if ( accounttypes.contains(splitAccount.accountGroup()) )
      {
        // reverse sign to match common notation for cash flow direction
        MyMoneyMoney value = (*it_split).value((*it_transaction).commodity(), splitAccount.currencyId())*reverse;

        // always use the last of the current month as the date to calculate the value
        QDate valuedate = QDate(postdate.year(), postdate.month(), 1);
        valuedate.addMonths(1);
        valuedate.addDays(-1);

        // extract the topmost parent (excluding the standard accounts)
        //
        // Ideally I would like this method to live in MyMoneyAccount (ace jones)
        // maybe in MyMoneyFile, as MyMoneyAccount does not know anything about
        // the special standard accounts (asset, ...) (thomas baumgart)
        MyMoneyAccount account = file->account(topLevelAccount(splitAccount.id()));

        // the row group is the account group (major account type)
        QString rowgroup = KMyMoneyUtils::accountTypeToString(account.accountGroup());

        // the row is the account name
        QString row = account.name();

        // fill the row list with blanks if it doesn't already exist.
        if ( m_grid[rowgroup][row].isEmpty() )
          m_grid[rowgroup][row].insert( m_grid[rowgroup][row].end(), m_numColumns, 0 );

        // add the value to its correct position in the pivot table
        m_grid[rowgroup][row][column] += baseValue(file, value, splitAccount, valuedate);

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
  
}

QCString PivotTable::topLevelAccount( const QCString& accountid ) const
{
  MyMoneyFile* file = MyMoneyFile::instance();
  QCString resultid = accountid;
  QCString parentid = file->account(resultid).parentAccountId();

  while (!file->isStandardAccount(parentid))
  {
    // take on the identity of our parent
    resultid = parentid;
    
    // and try again
    parentid = file->account(resultid).parentAccountId();
  }
  
  return resultid;
}

void PivotTable::calculateOpeningBalances( const MyMoneyFile* file, const QDate& begindate, const QDate& enddate, const QValueList<MyMoneyAccount::accountTypeE>& accounttypes )
{
  QValueList<MyMoneyAccount> accounts = file->accountList();
  QValueList<MyMoneyAccount>::const_iterator it_account = accounts.begin();

  while ( it_account != accounts.end() )
  {
    // only include this item if its account group is included in this report
    if ( accounttypes.contains((*it_account).accountGroup()) )
    {
      // extract the balance of the account for the given begin date
      MyMoneyMoney value = file->balance((*it_account).id(), begindate.addDays(-1));
      
      // place into the 'opening' column...
      int column = 0;

      // unless the account was opened after the start of the report period
      QDate opendate = (*it_account).openingDate();
      QDate valuedate = begindate.addDays(-1);
      if ( opendate >= begindate )
      {
        if ( opendate < enddate ) {
          // in which case it should be placed in the correct column
          column = opendate.year() * 12 + opendate.month() - begindate.year() * 12 - begindate.month() + 1;
          // make the value date the last of the month
          valuedate = opendate;

        } else
          // or disregarded if the account was opened after the end of the
          // report period
          value = 0;
      }

      // always use the last of the current month as the date to calculate the value
      valuedate = QDate(valuedate.year(), valuedate.month(), 1);
      valuedate.addMonths(1);
      valuedate.addDays(-1);

      // extract the topmost parent (excluding the standard accounts)
      //
      // Ideally I would like this method to live in MyMoneyAccount (ace jones)
      // maybe in MyMoneyFile, as MyMoneyAccount does not know anything about
      // the special standard accounts (asset, ...) (thomas baumgart)
      MyMoneyAccount account = file->account(topLevelAccount((*it_account).id()));

      // the row group is the account group (major account type)
      QString rowgroup = KMyMoneyUtils::accountTypeToString(account.accountGroup());

      // the row is the account name
      QString row = account.name();

      // fill the row list with blanks if it doesn't already exist.
      if ( m_grid[rowgroup][row].isEmpty() )
        m_grid[rowgroup][row].insert( m_grid[rowgroup][row].end(), m_numColumns, 0 );

      // add the value to its correct position in the pivot table
      m_grid[rowgroup][row][column] += baseValue(file, value, *it_account, valuedate);

    }
    ++it_account;
  }
}

const MyMoneyMoney PivotTable::baseValue(const MyMoneyFile* file, const MyMoneyMoney& amount, const MyMoneyAccount& acc, const QDate& date) const
{
  MyMoneyMoney value(amount);
  
  if(acc.currencyId() != file->baseCurrency().id()) {
    MyMoneyCurrency currency = file->currency(acc.currencyId());
    value = currency.price(date);
  }
  
  return value;
}

void PivotTable::calculateRunningSums( void )
{
  TGrid::iterator it_rowgroup = m_grid.begin();
  while ( it_rowgroup != m_grid.end() )
  {
    TRowGroup::iterator it_row = (*it_rowgroup).begin();
    while ( it_row != (*it_rowgroup).end() )
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
    ++it_rowgroup;
  }
}

QString PivotTable::renderHTML( void ) const
{
  //
  // Table Header
  //

  QString result = QString("<table class=\"report\" cellspacing=\"0\">"
      "<tr class=\"itemheader\"><th>%1</th>").arg(i18n("Account"));
  
  int column = 1;
  while ( column < m_numColumns )
  {
    result += QString("<th>%1</th>").arg(m_columnHeadings[column]);
    ++column;
  }

  if ( m_displayRowTotals )
    result += QString("<th>%1</th>").arg(i18n("Total"));

  result += "</tr>";
    
  // calculate the column grand totals along the way
  QValueList<MyMoneyMoney> columngrandtotal;
  columngrandtotal.insert( columngrandtotal.end(), m_numColumns, MyMoneyMoney() );

  //
  // Row groups
  //

  // iterate over row groups
  TGrid::const_iterator it_rowgroup = m_grid.begin();
  while ( it_rowgroup != m_grid.end() )
  {
    //
    // Row Group Header
    //

    result += QString("<tr class=\"sectionheader\"><td class=\"left\" colspan=\"0\">%1</td></tr>").arg(it_rowgroup.key());

    QValueList<MyMoneyMoney> columntotal;
    columntotal.insert( columntotal.end(), m_numColumns, MyMoneyMoney() );

    //
    // Rows
    //

    TRowGroup::const_iterator it_row = (*it_rowgroup).begin();
    int rownum = 0;
    while ( it_row != (*it_rowgroup).end() )
    {
      //
      // Row Header
      //

      QString rowname = it_row.key();
      rowname.replace(QRegExp(" "),QString("&nbsp;"));
      result += QString("<tr class=\"row-%1\"><td class=\"left\">%2</td>")
        .arg(rownum++ & 0x01 ? "even" : "odd")
        .arg(rowname);

      //
      // Columns
      //

      int column = 1;
      MyMoneyMoney rowtotal;
      while ( column < m_numColumns )
      {
        MyMoneyMoney value = it_row.data()[column];
        columntotal[column] += value;
        rowtotal += value;
        result += QString("<td>%1</td>").arg(value.formatMoney());

        ++column;
      }

      //
      // Row Total
      //
      if ( m_displayRowTotals )
        result += QString("<td>%1</td>").arg(rowtotal.formatMoney());

      result += "</tr>";

      ++it_row;
    }

    //
    // Row Group Totals
    //

    result += QString("<tr class=\"sectionfooter\"><td class=\"left\">%1&nbsp;%1</td>").arg(i18n("Total")).arg(it_rowgroup.key());
    int column = 1;
    MyMoneyMoney rowgrouptotal;
    while ( column < m_numColumns )
    {
      MyMoneyMoney value = columntotal[column];
      columngrandtotal[column] += value;
      rowgrouptotal += value;
      result += QString("<td>%1</td>").arg(value.formatMoney());

      ++column;
    }

    if ( m_displayRowTotals )
      result += QString("<td>%1</td>").arg(rowgrouptotal.formatMoney());

    result += "</tr>";

    ++it_rowgroup;
  }

  //
  // Report Totals
  //

  result += QString("<tr class=\"spacer\"><td></td></tr>");
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

  result += "</tr>";
    
  result += QString("<tr class=\"spacer\"><td></td></tr>");
  result += QString("<tr class=\"spacer\"><td></td></tr>");
  result += "</table>";

  return result;
}

KReportsView::KReportsView(QWidget *parent, const char *name )
 : QWidget(parent,name)
{
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
  QString filename = KGlobal::dirs()->findResource("appdata", "html/kmymoney2.css");
  QString header = QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\">\n") +
    QString("<html><head><link rel=\"stylesheet\" type=\"text/css\" href=\"%1\"></head><body>\n").arg(filename);
  QString footer = "</body></html>\n";

  int currentyear = QDate::currentDate().year();
  QValueList<MyMoneyAccount::accountTypeE> accounttypes;

  QString html;
  html += header;
  html += QString("<h2 class=\"report\">%1</h2>").arg(i18n("Monthly Income & Expenses"));
  html += QString("<div class=\"subtitle\">") + i18n("All currencies converted to %1")
                                .arg(MyMoneyFile::instance()->baseCurrency().name())
       + QString("</div>");
  html += QString("<div class=\"gap\">&nbsp;</div>\n");

  accounttypes.clear();
  accounttypes.append(MyMoneyAccount::Expense);
  accounttypes.append(MyMoneyAccount::Income);  
  PivotTable expensereport( MyMoneyFile::instance(), QDate(currentyear,1,1),QDate(currentyear+1,1,1),accounttypes, false );
  html += expensereport.renderHTML();

  html += QString("<h2 class=\"report\">%1</h2>").arg(i18n("Net Worth Over Time"));
  html += QString("<div class=\"subtitle\">") + i18n("All currencies converted to %1")
                                .arg(MyMoneyFile::instance()->baseCurrency().name())
       + QString("</div>");
  html += QString("<div class=\"gap\">&nbsp;</div>\n");

  accounttypes.clear();
  accounttypes.append(MyMoneyAccount::Asset);
  accounttypes.append(MyMoneyAccount::Liability);
  PivotTable networthreport( MyMoneyFile::instance(), QDate(currentyear,1,1),QDate(currentyear+1,1,1),accounttypes, true );
  html += networthreport.renderHTML();
    
  html += links;
  html += footer;

  return html;
}

void KReportsView::slotRefreshView(void)
{
  QString links;
  links += linkfull(VIEW_REPORTS, QString("?command=copy"),i18n("Copy to Clipboard"));
  
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

const QString KReportsView::linkfull(const QString& view, const QString& query, const QString& label)
{
  return QString("<a href=\"/") + view + query + "\">" + label + "</a>";
}

void KReportsView::slotOpenURL(const KURL &url, const KParts::URLArgs& /* args */)
{
  QString view = url.fileName(false);
  QCString id = url.queryItem("id").data();
  QCString command = url.queryItem("command").data();
  
  if(view == VIEW_REPORTS) {

      if ( command.isEmpty() )
        slotRefreshView();
      else if ( command == "print" )
        slotPrintView();
      else if ( command == "copy" )
        slotCopyView();
      else
        qDebug("Unknown command '%s' in KReportsView::slotOpenURL()", static_cast<const char*>(command));

  } else {
    qDebug("Unknown view '%s' in KReportsView::slotOpenURL()", view.latin1());
  }
}

