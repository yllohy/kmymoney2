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
    typedef QValueList<MyMoneyMoney> TGridLine;
    typedef QMap<QString,TGridLine> TGrid;

    QMap<QString,TGrid> m_grid;

public:
    PivotTable( const MyMoneyFile* );
    QString renderHTML( void ) const;

protected:
    QString getType( const QCString&, QCString& ) const;

};

PivotTable::PivotTable( const MyMoneyFile* file )
{
  int currentyear = QDate::currentDate().year();
  MyMoneyTransactionFilter filter;
  filter.setDateFilter(QDate(currentyear,1,1),QDate(currentyear,12,31));
  filter.setReportAllSplits(false);
  QValueList<MyMoneyTransaction> transactions = file->transactionList(filter);
  QValueList<MyMoneyTransaction>::const_iterator it_transaction = transactions.begin();
  
  while ( it_transaction != transactions.end() )
  {
    int column = (*it_transaction).postDate().month() - 1;

    QValueList<MyMoneySplit> splits = (*it_transaction).splits();
    QValueList<MyMoneySplit>::const_iterator it_split = splits.begin();
    while ( it_split != splits.end() )
    {
      // determine the type of transaction, which will become the row group
      QCString accountid = (*it_split).accountId();
      QCString parentid;
      MyMoneyAccount account = file->account(accountid);
      
      // reverse sign to match common notation for cash flow direction
      MyMoneyMoney value = - (*it_split).value((*it_transaction).commodity(), account.currencyId());
      
      QString rowgroup = getType( accountid, parentid );
      // roll up subcategories into top level categories
      while ( rowgroup == "Subcategory" )
      {
        accountid = parentid;
        rowgroup = getType( accountid, parentid );
      }

      // read the account info of the top level account
      account = file->account(accountid);

      // the row itself is the account name
      QString row = account.name();

      // filter out undesirable row groups
      if ( rowgroup == "Income" || rowgroup == "Expense" )
      {
        if ( m_grid[rowgroup][row].isEmpty() )
          m_grid[rowgroup][row].insert( m_grid[rowgroup][row].end(), 12, 0 );

        m_grid[rowgroup][row][column] += value;

      }
      ++it_split;
    }
    ++it_transaction;
  }
}

QString PivotTable::getType( const QCString& accountid, QCString& parentid ) const
{
  QString result;
  MyMoneyFile* file = MyMoneyFile::instance();

  parentid = file->account(accountid).parentAccountId();

  if ( parentid == file->asset().id() )
    result = "Asset";
  else if ( parentid == file->liability().id() )
    result = "Liability";
  else if ( parentid == file->income().id() )
    result = "Income";
  else if ( parentid == file->expense().id() )
    result = "Expense";
  else
    result = "Subcategory";

  return result;
}

QString PivotTable::renderHTML( void ) const
{
  //
  // Table Header
  //

  QString result = QString("<table class=\"report\" cellspacing=\"0\">"
      "<tr class=\"itemheader\"><th>%1</th>").arg(i18n("Account"));

  // TODO: Remove the fixed # of columns and column meanings from the code.
  // This should be in the grid data.
  int month = 0;
  while ( ++month <= 12 )
    result += QString("<th>%1</th>").arg(QDate::shortMonthName(month));

  result += QString("<th>%1</th></tr>").arg(i18n("Total"));

  // calculate the column grand totals along the way
  QValueList<MyMoneyMoney> columngrandtotal;
  columngrandtotal.insert( columngrandtotal.end(), 12, MyMoneyMoney() );

  //
  // Row groups
  //

  // iterate over row groups
  QMap<QString,TGrid>::const_iterator it_rowgroup = m_grid.begin();
  while ( it_rowgroup != m_grid.end() )
  {
    //
    // Row Group Header
    //

    result += QString("<tr class=\"sectionheader\"><td class=\"left\" colspan=\"0\">%1</td></tr>").arg(it_rowgroup.key());

    QValueList<MyMoneyMoney> columntotal;
    columntotal.insert( columntotal.end(), 12, MyMoneyMoney() );

    //
    // Rows
    //

    TGrid::const_iterator it_row = (*it_rowgroup).begin();
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

      int column = 0;
      MyMoneyMoney rowtotal;
      while ( column < 12 )
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
      result += QString("<td>%1</td></tr>").arg(rowtotal.formatMoney());

      ++it_row;
    }

    //
    // Row Group Totals
    //

    result += QString("<tr class=\"sectionfooter\"><td class=\"left\">%1&nbsp;%1</td>").arg(i18n("Total")).arg(it_rowgroup.key());
    int column = 0;
    MyMoneyMoney rowgrouptotal;
    while ( column < 12 )
    {
      MyMoneyMoney value = columntotal[column];
      columngrandtotal[column] += value;
      rowgrouptotal += value;
      result += QString("<td>%1</td>").arg(value.formatMoney());

      ++column;
    }
    result += QString("<td>%1</td></tr>").arg(rowgrouptotal.formatMoney());

    ++it_rowgroup;
  }

  //
  // Report Totals
  //

  result += QString("<tr class=\"spacer\"><td></td></tr>");
  result += QString("<tr class=\"reportfooter\"><td class=\"left\">%1</td>").arg(i18n("Grand Total"));
  int totalcolumn = 0;
  MyMoneyMoney grandtotal;
  while ( totalcolumn < 12 )
  {
    MyMoneyMoney value = columngrandtotal[totalcolumn];
    grandtotal += value;
    result += QString("<td>%1</td>").arg(value.formatMoney());

    ++totalcolumn;
  }

  MyMoneyMoney value = grandtotal;
  result += QString("<td>%1</td></tr>").arg(value.formatMoney());

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

/* TODO:
  - Remove the fixed # of columns and column meanings from the code.
    This should be in the grid data.
*/

const QString KReportsView::createTable(const QString& links) const
{
  QString filename = KGlobal::dirs()->findResource("appdata", "html/kmymoney2.css");
  QString header = QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\">\n") +
    QString("<html><head><link rel=\"stylesheet\" type=\"text/css\" href=\"%1\"></head>\n").arg(filename);
  QString footer = "</body></html>\n";

  PivotTable pivot( MyMoneyFile::instance() );

  QString html;
  html += header;
  html += QString("<h2 class=\"report\">%1</h2>").arg(i18n("Monthly Income & Expenses"));
  html += QString("<center>") + i18n("All currencies converted to %1")
                                .arg(MyMoneyFile::instance()->baseCurrency().name())
       + QString("</center>");
  html += QString("<div class=\"gap\">&nbsp;</div>\n");

  html += pivot.renderHTML();
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

