/***************************************************************************
                          listtable.cpp
                         -------------------
    begin                : Sat 28 jun 2008
    copyright            : (C) 2004-2005 by Ace Jones
                               2008 by Alvaro Soliverez
    email                :  acejones@users.sourceforge.net
                            asoliverez@gmail.com
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
#include <qvaluelist.h>
#include <qfile.h>
#include <qtextstream.h>

// ----------------------------------------------------------------------------
// KDE Includes
// This is just needed for i18n().  Once I figure out how to handle i18n
// without using this macro directly, I'll be freed of KDE dependency.

#include <klocale.h>
#include <kdebug.h>

// ----------------------------------------------------------------------------
// Project Includes
#include "../mymoney/mymoneyfile.h"
#include "../mymoney/mymoneyreport.h"
#include "../mymoney/mymoneyexception.h"
#include "../kmymoneyutils.h"
#include "../kmymoneyglobalsettings.h"
#include "reportdebug.h"
#include "listtable.h"

namespace reports {

  QStringList ListTable::TableRow::m_sortCriteria;

  // ****************************************************************************
  //
  // Group Iterator
  //
  // ****************************************************************************

  class GroupIterator
  {
    public:
      GroupIterator ( const QString& _group, const QString& _subtotal, unsigned _depth ) : m_depth ( _depth ), m_groupField ( _group ), m_subtotalField ( _subtotal ) {}
      GroupIterator ( void ) {}
      void update ( const ListTable::TableRow& _row )
      {
        m_previousGroup = m_currentGroup;
        m_currentGroup = _row[m_groupField];
        if ( isSubtotal() )
        {
          m_previousSubtotal = m_currentSubtotal;
          m_currentSubtotal = MyMoneyMoney();
        }
        m_currentSubtotal += _row[m_subtotalField];
      }

      bool isNewHeader ( void ) const { return ( m_currentGroup != m_previousGroup ); }
      bool isSubtotal ( void ) const { return ( m_currentGroup != m_previousGroup ) && ( !m_previousGroup.isEmpty() ); }
      const MyMoneyMoney& subtotal ( void ) const { return m_previousSubtotal; }
      const MyMoneyMoney& currenttotal ( void ) const { return m_currentSubtotal; }
      unsigned depth ( void ) const { return m_depth; }
      const QString& name ( void ) const { return m_currentGroup; }
      const QString& oldName ( void ) const { return m_previousGroup; }
      const QString& groupField ( void ) const { return m_groupField; }
      const QString& subtotalField ( void ) const { return m_subtotalField; }
      // ***DV*** HACK make the currentGroup test different but look the same
      void force ( void ) { m_currentGroup += " "; }
    private:
      MyMoneyMoney m_currentSubtotal;
      MyMoneyMoney m_previousSubtotal;
      unsigned m_depth;
      QString m_currentGroup;
      QString m_previousGroup;
      QString m_groupField;
      QString m_subtotalField;
  };

// ****************************************************************************
//
// ListTable implementation
//
// ****************************************************************************

  bool ListTable::TableRow::operator< ( const TableRow& _compare ) const
  {
    bool result = false;

    QStringList::const_iterator it_criterion = m_sortCriteria.begin();
    while ( it_criterion != m_sortCriteria.end() )
    {
      if ( this->operator[] ( *it_criterion ) < _compare[ *it_criterion ] )
      {
        result = true;
        break;
      }
      else if ( this->operator[] ( *it_criterion ) > _compare[ *it_criterion ] )
        break;

      ++it_criterion;
    }
    return result;
  }

// needed for KDE < 3.2 implementation of qHeapSort
  bool ListTable::TableRow::operator<= ( const TableRow& _compare ) const
  {
    return ( ! ( _compare < *this ) );
  }

  bool ListTable::TableRow::operator== ( const TableRow& _compare ) const
  {
    return ( ! ( *this < _compare ) && ! ( _compare < *this ) );
  }

  bool ListTable::TableRow::operator> ( const TableRow& _compare ) const
  {
    return ( _compare < *this );
  }

  /**
     * TODO
     *
     * - Collapse 2- & 3- groups when they are identical
     * - Way more test cases (especially splits & transfers)
     * - Option to collapse splits
     * - Option to exclude transfers
     *
   */

  ListTable::ListTable ( const MyMoneyReport& _report ) : m_config ( _report )
  {
  }

  void ListTable::render ( QString& result, QString& csv ) const
  {
    MyMoneyMoney grandtotal;
    MyMoneyFile* file = MyMoneyFile::instance();

    result = "";
    csv = "";
    result += QString ( "<h2 class=\"report\">%1</h2>\n" ).arg ( m_config.name() );
    csv += "\"Report: " + m_config.name() + "\"\n";
      //actual dates of the report
    result += QString("<div class=\"subtitle\">");
    if(!m_config.fromDate().isNull()) {
      result += i18n("Report date range", "%1 through %2").arg(KGlobal::locale()->formatDate(m_config.fromDate(), true)).arg(KGlobal::locale()->formatDate(m_config.toDate(), true));
      result += QString("</div>\n");
      result += QString("<div class=\"gap\">&nbsp;</div>\n");

      csv += i18n("Report date range", "%1 through %2").arg(KGlobal::locale()->formatDate(m_config.fromDate(), true)).arg(KGlobal::locale()->formatDate(m_config.toDate(), true));
      csv += QString("\n");
    }


    result += QString ( "<div class=\"subtitle\">" );
    if ( m_config.isConvertCurrency() )
    {
      result += i18n ( "All currencies converted to %1" ).arg ( file->baseCurrency().name() );
      csv += i18n ( "All currencies converted to %1\n" ).arg ( file->baseCurrency().name() );
    }
    else
    {
      result += i18n ( "All values shown in %1 unless otherwise noted" ).arg ( file->baseCurrency().name() );
      csv += i18n ( "All values shown in %1 unless otherwise noted\n" ).arg ( file->baseCurrency().name() );
    }
    result += QString ( "</div>\n" );
    result += QString ( "<div class=\"gap\">&nbsp;</div>\n" );

    // retrieve the configuration parameters from the report definition.
    // the things that we care about for query reports are:
    // how to group the rows, what columns to display, and what field
    // to subtotal on
    QStringList groups = QStringList::split ( ",", m_group );
    QStringList columns = QStringList::split ( ",", m_columns );
    columns += m_subtotal;
    QStringList postcolumns = QStringList::split ( ",", m_postcolumns );
    columns += postcolumns;

    //
    // Table header
    //
    QMap<QString, QString> i18nHeaders;
    i18nHeaders["postdate"] = i18n ( "Date" );
    i18nHeaders["value"] = i18n ( "Amount" );
    i18nHeaders["number"] = i18n ( "Num" );
    i18nHeaders["payee"] = i18n ( "Payee" );
    i18nHeaders["category"] = i18n ( "Category" );
    i18nHeaders["account"] = i18n ( "Account" );
    i18nHeaders["memo"] = i18n ( "Memo" );
    i18nHeaders["topcategory"] = i18n ( "Top Category" );
    i18nHeaders["categorytype"] = i18n ( "Category Type" );
    i18nHeaders["month"] = i18n ( "Month" );
    i18nHeaders["week"] = i18n ( "Week" );
    i18nHeaders["reconcileflag"] = i18n ( "Reconciled" );
    i18nHeaders["action"] = i18n ( "Action" );
    i18nHeaders["shares"] = i18n ( "Shares" );
    i18nHeaders["price"] = i18n ( "Price" );
    i18nHeaders["latestprice"] = i18n ( "Price" );
    i18nHeaders["netinvvalue"] = i18n ( "Net Value" );
    i18nHeaders["buys"] = i18n ( "Buys" );
    i18nHeaders["sells"] = i18n ( "Sells" );
    i18nHeaders["reinvestincome"] = i18n ( "Dividends Reinvested" );
    i18nHeaders["cashincome"] = i18n ( "Dividends Paid Out" );
    i18nHeaders["startingbal"] = i18n ( "Starting Balance" );
    i18nHeaders["endingbal"] = i18n ( "Ending Balance" );
    i18nHeaders["return"] = i18n ( "Annualized Return" );
    i18nHeaders["returninvestment"] = i18n ( "Return On Investment" );
    i18nHeaders["fees"] = i18n ( "Fees" );
    i18nHeaders["interest"] = i18n ( "Interest" );
    i18nHeaders["payment"] = i18n ( "Payment" );
    i18nHeaders["balance"] = i18n ( "Balance" );
    i18nHeaders["type"] = i18n ( "Type" );
    i18nHeaders["name"] = i18n ( "Name" );
    i18nHeaders["nextduedate"] = i18n ( "Next Due Date" );
    i18nHeaders["occurence"] = i18n ( "Occurence" );
    i18nHeaders["paymenttype"] = i18n ( "Payment Method" );
    i18nHeaders["institution"] = i18n ( "Institution" );
    i18nHeaders["description"] = i18n ( "Description" );
    i18nHeaders["openingdate"] = i18n ( "Opening Date" );
    i18nHeaders["currencyname"] = i18n ( "Currency" );
    i18nHeaders["balancewarning"] = i18n ( "Balance Early Warning" );
    i18nHeaders["maxbalancelimit"] = i18n ( "Balance Max Limit" );
    i18nHeaders["creditwarning"] = i18n ( "Credit Early Warning" );
    i18nHeaders["maxcreditlimit"] = i18n ( "Credit Max Limit" );
    i18nHeaders["tax"] = i18n ( "Tax" );
    i18nHeaders["favorite"] = i18n ( "Preferred" );
    i18nHeaders["loanamount"] = i18n ( "Loan Amount" );
    i18nHeaders["interestrate"] = i18n ( "Interest Rate" );
    i18nHeaders["nextinterestchange"] = i18n ( "Next Interest Change" );
    i18nHeaders["periodicpayment"] = i18n ( "Periodic Payment" );
    i18nHeaders["finalpayment"] = i18n ( "Final Payment" );
    i18nHeaders["currentbalance"] = i18n ( "Current Balance" );

    // the list of columns which represent money, so we can display them correctly
    QStringList moneyColumns = QStringList::split ( ",", "value,shares,price,latestprice,netinvvalue,buys,sells,cashincome,reinvestincome,startingbal,fees,interest,payment,balance,balancewarning,maxbalancelimit,creditwarning,maxcreditlimit,loanamount,periodicpayment,finalpayment,currentbalance" );

    // the list of columns which represent shares, which is like money except the
    // transaction currency will not be displayed
    QStringList sharesColumns = QStringList::split ( ",", "shares" );

    // the list of columns which represent a percentage, so we can display them correctly
    QStringList percentColumns = QStringList::split ( ",", "return,returninvestment,interestrate" );

    // the list of columns which represent dates, so we can display them correctly
    QStringList dateColumns = QStringList::split ( ",", "postdate,entrydate,nextduedate,openingdate,nextinterestchange" );

    result += "<table class=\"report\">\n<thead><tr class=\"itemheader\">";

    QStringList::const_iterator it_column = columns.begin();
    while ( it_column != columns.end() )
    {
      QString i18nName = i18nHeaders[*it_column];
      if ( i18nName.isEmpty() )
        i18nName = *it_column;
      result += "<th>" + i18nName + "</th>";
      csv += i18nName + ",";
      ++it_column;
    }

    result += "</tr></thead>\n";
    csv = csv.left ( csv.length() - 1 );
    csv += "\n";

    //
    // Set up group iterators
    //
    // There is one active iterator for each level of grouping.
    // As we step through the rows
    // we update the group iterators each time based on the row data.  If
    // the group iterator changes and it had a previous value, we print a
    // subtotal.  Whether or not it had a previous value, we print a group
    // header.  The group iterator keeps track of a subtotal also.

    int depth = 1;
    QValueList<GroupIterator> groupIteratorList;
    QStringList::const_iterator it_grouplevel = groups.begin();
    while ( it_grouplevel != groups.end() )
    {
      groupIteratorList += GroupIterator ( ( *it_grouplevel ), m_subtotal, depth++ );
      ++it_grouplevel;
    }

    //
    // Rows
    //

    bool row_odd = true;

    // ***DV***
    MyMoneyMoney startingBalance;
    for ( QValueList<TableRow>::const_iterator it_row = m_rows.begin();
          it_row != m_rows.end();
          ++it_row ) {

      // the standard fraction is the fraction of an non-cash account in the base currency
      // this could be overridden using the "fraction" element of a row for each row.
      // Currently (2008-02-21) this override is not used at all (ipwizard)
      int fraction = file->baseCurrency().smallestAccountFraction();
      if ( ( *it_row ).find ( "fraction" ) != ( *it_row ).end() )
        fraction = ( *it_row ) ["fraction"].toInt();

      //
      // Process Groups
      //

      // ***DV*** HACK to force a subtotal and header, since this render doesn't
      // always detect a group change for different accounts with the same name
      // (as occurs with the same stock purchased from different investment accts)
      if ( it_row != m_rows.begin() )
        if ( ( ( * it_row ) ["rank"] == "-2" ) && ( ( * it_row ) ["id"] == "A" ) )
          ( groupIteratorList.last() ).force();

      // There's a subtle bug here.  If an earlier group gets a new group,
      // then we need to force all the downstream groups to get one too.

      // Update the group iterators with the current row value
      QValueList<GroupIterator>::iterator it_group = groupIteratorList.begin();
      while ( it_group != groupIteratorList.end() )
      {
        ( *it_group ).update ( *it_row );
        ++it_group;
      }

      // Do subtotals backwards
      if ( m_config.isConvertCurrency() )
      {
        it_group = groupIteratorList.fromLast();
        while ( it_group != groupIteratorList.end() )
        {
          if ( ( *it_group ).isSubtotal() )
          {
            if ( ( *it_group ).depth() == 1 )
              grandtotal += ( *it_group ).subtotal();
              grandtotal = grandtotal.convert(fraction);

            QString subtotal_html = ( *it_group ).subtotal().formatMoney ( fraction );
            QString subtotal_csv = ( *it_group ).subtotal().formatMoney ( fraction, false );

            // ***DV*** HACK fix the side-effiect from .force() method above
            QString oldName = QString ( ( *it_group ).oldName() ).stripWhiteSpace();

            result +=
              "<tr class=\"sectionfooter\">"
              "<td class=\"left" + QString::number ( ( ( *it_group ).depth() - 1 ) ) + "\" "
              "colspan=\"" +
              QString::number ( columns.count() - 1 - postcolumns.count() ) + "\">" +
              i18n ( "Total" ) + " " + oldName + "</td>"
              "<td>" + subtotal_html + "</td></tr>\n";

            csv +=
              "\"" + i18n ( "Total" ) + " " + oldName + "\",\"" + subtotal_csv + "\"\n";
          }
          --it_group;
        }
      }

      // And headers forwards
      it_group = groupIteratorList.begin();
      while ( it_group != groupIteratorList.end() )
      {
        if ( ( *it_group ).isNewHeader() )
        {
          row_odd = true;
          result += "<tr class=\"sectionheader\">"
                    "<td class=\"left" + QString::number ( ( ( *it_group ).depth() - 1 ) ) + "\" "
                    "colspan=\"" + QString::number ( columns.count() ) + "\">" +
                    ( *it_group ).name() + "</td></tr>\n";
          csv += "\"" + ( *it_group ).name() + "\"\n";
        }
        ++it_group;
      }

      //
      // Columns
      //

      // skip the opening and closing balance row,
      // if the balance column is not shown
      if ( ( columns.contains ( "balance" ) == 0 ) && ( ( *it_row ) ["rank"] == "-2" ) )
        continue;

      bool need_label = true;

      // ***DV***
      if ( ( * it_row ) ["rank"] == "0" ) row_odd = ! row_odd;

      if ( ( * it_row ) ["rank"] == "-2" )
        result += QString ( "<tr class=\"item%1\">" ).arg ( ( * it_row ) ["id"] );
      else
        if ( ( * it_row ) ["rank"] == "1" )
          result += QString ( "<tr class=\"%1\">" ).arg ( row_odd ? "item1" : "item0" );
        else
          result += QString ( "<tr class=\"%1\">" ).arg ( row_odd ? "row-odd " : "row-even" );

      QStringList::const_iterator it_column = columns.begin();
      while ( it_column != columns.end() )
      {
        QString data = ( *it_row ) [*it_column];

        // ***DV***
        if ( ( * it_row ) ["rank"] == "1" ) {
          if ( * it_column == "value" )
            data = ( * it_row ) ["split"];
          else if ( *it_column == "postdate"
                    || *it_column == "number"
                    || *it_column == "payee"
                    || *it_column == "action"
                    || *it_column == "shares"
                    || *it_column == "price"
                    || *it_column == "nextduedate"
                    || *it_column == "balance"
                    || *it_column == "account"
                    || *it_column == "name" )
            data = "";
        }

        // ***DV***
        if ( ( * it_row ) ["rank"] == "-2" ) {
          if ( *it_column == "balance" ) {
            data = ( * it_row ) ["balance"];
            if ( ( * it_row ) ["id"] == "A" )     // opening balance?
              startingBalance = MyMoneyMoney ( data );
          }

          if ( need_label ) {
            if ( ( * it_column == "payee" ) ||
                 ( * it_column == "category" ) ||
                 ( * it_column == "memo" ) ) {
              if ( ( * it_row ) ["shares"] != "" ) {
                data = ( ( * it_row ) ["id"] == "A" )
                       ? i18n ( "Initial Market Value" )
                       : i18n ( "Ending Market Value" );
              } else {
                data = ( ( * it_row ) ["id"] == "A" )
                       ? i18n ( "Opening Balance" )
                       : i18n ( "Closing Balance" );
              }
              need_label = false;
            }
          }
        }

        // The 'balance' column is calculated at render-time
        // but not printed on split lines
        else if ( *it_column == "balance" && ( * it_row ) ["rank"] == "0" )
        {
          // Take the balance off the deepest group iterator
          data = ( groupIteratorList.back().currenttotal() + startingBalance ).toString();
        }

        // Figure out how to render the value in this column, depending on
        // what its properties are.
        //
        // TODO: This and the i18n headings are handled
        // as a set of parallel vectors.  Would be much better to make a single
        // vector of a properties class.
        if ( sharesColumns.contains ( *it_column ) )
        {
          if ( data.isEmpty() ) {
            result += QString ( "<td></td>" );
            csv += "\"\",";
          }
          else {
            result += QString ( "<td>%1</td>" ).arg ( MyMoneyMoney ( data ).formatMoney ( "", 3 ) );
            csv += "\"" + MyMoneyMoney ( data ).formatMoney ( "", 3, false ) + "\",";
          }
        }
        else if ( moneyColumns.contains ( *it_column ) )
        {
          if ( data.isEmpty() ) {
            result += QString ( "<td%1></td>" )
                      .arg ( ( *it_column == "value" ) ? " class=\"value\"" : "" );
            csv += "\"\",";
          } else if ( MyMoneyMoney( data ) == MyMoneyMoney::autoCalc ) {
            result += QString ( "<td%1>%2</td>" )
                .arg ( ( *it_column == "value" ) ? " class=\"value\"" : "" )
                .arg (i18n("Calculated"));
            csv += "\""+ i18n("Calculated") +"\",";
          } else {
            result += QString ( "<td%1>%2&nbsp;%3</td>" )
                      .arg ( ( *it_column == "value" ) ? " class=\"value\"" : "" )
                      .arg ( ( *it_row ) ["currency"] )
                      .arg ( MyMoneyMoney ( data ).formatMoney ( fraction ) );
            csv += "\"" + ( *it_row ) ["currency"] + " " + MyMoneyMoney ( data ).formatMoney ( fraction, false ) + "\",";
          }
        }
        else if ( percentColumns.contains ( *it_column ) )
        {
          data = ( MyMoneyMoney ( data ) * MyMoneyMoney ( 100, 1 ) ).formatMoney ( fraction );
          result += QString ( "<td>%1%</td>" ).arg ( data );
          csv += data + "%,";
        }
        else if ( dateColumns.contains ( *it_column ) )
        {
          // do this before we possibly change data
          csv += "\"" + data + "\",";

          // if we have a locale() then use its date formatter
          if ( KGlobal::locale() && ! data.isEmpty() ) {
            QDate qd = QDate::fromString ( data, Qt::ISODate );
            data = KGlobal::locale()->formatDate ( qd, true );
          }
          result += QString ( "<td class=\"left\">%1</td>" ).arg ( data );
        }
        else
        {
          result += QString ( "<td class=\"left\">%1</td>" ).arg ( data );
          csv += "\"" + data + "\",";
        }
        ++it_column;
      }

      result += "</tr>\n";
      csv = csv.left ( csv.length() - 1 ); // remove final comma
      csv += "\n";
    }

    //
    // Final group totals
    //

    // Do subtotals backwards
    if ( m_config.isConvertCurrency() )
    {
      int fraction = file->baseCurrency().smallestAccountFraction();
      QValueList<GroupIterator>::iterator it_group = groupIteratorList.fromLast();
      while ( it_group != groupIteratorList.end() )
      {
        ( *it_group ).update ( TableRow() );

        if ( ( *it_group ).depth() == 1 ) {
          grandtotal += ( *it_group ).subtotal();
          grandtotal = grandtotal.convert(fraction);
        }


        QString subtotal_html = ( *it_group ).subtotal().formatMoney ( fraction );
        QString subtotal_csv = ( *it_group ).subtotal().formatMoney ( fraction, false );

        result += "<tr class=\"sectionfooter\">"
                  "<td class=\"left" + QString::number ( ( *it_group ).depth() - 1 ) + "\" "
                  "colspan=\"" + QString::number ( columns.count() - 1 - postcolumns.count() ) + "\">" +
                  i18n ( "Total" ) + " " + ( *it_group ).oldName() + "</td>"
                  "<td>" + subtotal_html + "</td></tr>\n";
        csv += "\"" + i18n ( "Total" ) + " " + ( *it_group ).oldName() + "\",\"" + subtotal_csv + "\"\n";
        --it_group;
      }

      //
      // Grand total
      //

      QString grandtotal_html = grandtotal.formatMoney ( fraction );
      QString grandtotal_csv = grandtotal.formatMoney ( fraction, false );

      result += "<tr class=\"sectionfooter\">"
                "<td class=\"left0\" "
                "colspan=\"" + QString::number ( columns.count() - 1 - postcolumns.count() ) + "\">" +
                i18n ( "Grand Total" ) + "</td>"
                "<td>" + grandtotal_html + "</td></tr>\n";
      csv += "\"" + i18n ( "Grand Total" ) + "\",\"" + grandtotal_csv + "\"\n";
    }
    result += "</table>\n";
  }

  QString ListTable::renderHTML ( void ) const
  {
    QString html, csv;
    render ( html, csv );
    return html;
  }

  QString ListTable::renderCSV ( void ) const
  {
    QString html, csv;
    render ( html, csv );
    return csv;
  }

  void ListTable::dump ( const QString& file, const QString& context ) const
  {
    QFile g ( file );
    g.open ( IO_WriteOnly );

    if ( ! context.isEmpty() )
      QTextStream ( &g ) << context.arg ( renderHTML() );
    else
      QTextStream ( &g ) << renderHTML();
    g.close();
  }

  void ListTable::includeInvestmentSubAccounts()
  {
  // if we're not in expert mode, we need to make sure
  // that all stock accounts for the selected investment
  // account are also selected
    QStringList accountList;
    if(m_config.accounts(accountList)) {
      if(!KMyMoneyGlobalSettings::expertMode()) {
        QStringList::const_iterator it_a, it_b;
        for(it_a = accountList.begin(); it_a != accountList.end(); ++it_a) {
          MyMoneyAccount acc = MyMoneyFile::instance()->account(*it_a);
          if(acc.accountType() == MyMoneyAccount::Investment) {
            for(it_b = acc.accountList().begin(); it_b != acc.accountList().end(); ++it_b) {
              if(!accountList.contains(*it_b)) {
                m_config.addAccount(*it_b);
              }
            }
          }
        }
      }
    }
  }

}
