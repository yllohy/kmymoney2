/***************************************************************************
                          querytable.cpp
                         -------------------
    begin                : Fri Jul 23 2004
    copyright            : (C) 2004 by Ace Jones
    email                : ace.j@hotpop.com
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

// ----------------------------------------------------------------------------
// Project Includes
#include "../mymoney/mymoneyfile.h"
#include "../mymoney/mymoneytransaction.h"
#include "../mymoney/mymoneyreport.h"
#include "../mymoney/mymoneyexception.h"
#include "../kmymoneyutils.h"
#include "pivottable.h"
#include "querytable.h"

namespace reports {

// this should be in mymoneysplit.h
static const QStringList kReconcileText = QStringList::split(",","notreconciled,cleared,reconciled,frozen,none");
static const QStringList kReconcileTextChar = QStringList::split(",","N,C,R,F,none");

QStringList QueryTable::TableRow::m_sortCriteria;

//
// Cash Flow analysis tools for investment reports
//

QDate CashFlowListItem::m_sToday = QDate::currentDate();

MyMoneyMoney CashFlowListItem::NPV( double _rate ) const
{
  double T = static_cast<double>(m_sToday.daysTo(m_date)) / 365.0;
  MyMoneyMoney result = m_value.toDouble() / pow(1+_rate,T);
  
  return result;
}

const CashFlowListItem& CashFlowList::mostRecent(void) const
{
  CashFlowList dupe( *this );
  qHeapSort( dupe );
  
  return dupe.back();
}

MyMoneyMoney CashFlowList::NPV( double _rate )
{
  MyMoneyMoney result = 0.0;
  
  const_iterator it_cash = begin();
  while ( it_cash != end() )
  {
    result += (*it_cash).NPV( _rate );
    ++it_cash;
  }
  
  return result;
}

double CashFlowList::IRR( void )
{
  double result = 0.0;
  
  // set 'today', which is the most recent of all dates in the list
  CashFlowListItem::setToday( mostRecent().date() );
  
  double lobound = -0.999; // this is as low as we support
  double hibound = 0.01;
  double precision = 0.00001; // how precise do we want the answer?
  
  // is IRR so low we can't calculate it?
  if ( NPV( lobound ) > 0 )
  {
    throw QString("IRR is < -100%, not supported");
  }
  
  // shift the range upward as much as needed
  while ( NPV( hibound ) < 0 )
  {
    lobound = hibound;
    hibound *= 10;
  }
  
  while ( (hibound-lobound) > precision )
  {
    result = (lobound + hibound) / 2;
    MyMoneyMoney npv = NPV( result );
    if ( npv >= 0 )
      hibound = result;
    if ( npv <= 0 )
      lobound = result;
  }
  
  return result;
}

//
// QueryTable implementation
//

bool QueryTable::TableRow::operator<( const TableRow& _compare ) const
{
  bool result = false;

  QStringList::const_iterator it_criterion = m_sortCriteria.begin();
  while ( it_criterion != m_sortCriteria.end() )
  {
    if ( this->operator[]( *it_criterion ) < _compare[ *it_criterion ] )
    {
      result = true;
      break;
    }
    else if ( this->operator[]( *it_criterion ) > _compare[ *it_criterion ] )
      break;

    ++it_criterion;
  }
  return result;
}

// needed for KDE < 3.2 implementation of qHeapSort
bool QueryTable::TableRow::operator<=( const TableRow& _compare ) const
{
  return ( ! ( _compare < *this ) );
}
bool QueryTable::TableRow::operator==( const TableRow& _compare ) const
{
  return ( !( *this < _compare ) && !( _compare < *this ) );
}
bool QueryTable::TableRow::operator>( const TableRow& _compare ) const
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

// stealing this function from pivottable.cpp.  As noted in the comments of that file,
// we do need a better solution, but it'll do 'for now'.
const QString accountTypeToString(const MyMoneyAccount::accountTypeE accountType);

QueryTable::QueryTable(const MyMoneyReport& _report): m_config(_report)
{
  switch ( m_config.rowType() )
  {
    case MyMoneyReport::eAccountByTopAccount: 
    case MyMoneyReport::eEquityType:
    case MyMoneyReport::eAccountType:
    case MyMoneyReport::eInstitution:
      constructAccountTable();
      m_columns="account";
      break;
    default:
      constructTransactionTable();
      m_columns="postdate";
  }

  // Sort the data to match the report definition
  m_subtotal="value";

  switch ( m_config.rowType() )
  {
  case MyMoneyReport::eCategory:
    m_group = "categorytype,topcategory,category";
    break;
  case MyMoneyReport::eTopCategory:
    m_group = "categorytype,topcategory";
    break;
  case MyMoneyReport::eTopAccount:
    m_group = "topaccount,account";
    break;
  case MyMoneyReport::eAccount:
    m_group = "account";
    break;
  case MyMoneyReport::ePayee:
    m_group = "payee";
    break;
  case MyMoneyReport::eMonth:
    m_group = "month";
    break;
  case MyMoneyReport::eWeek:
    m_group = "week";
    break;
  case MyMoneyReport::eAccountByTopAccount: 
    m_group = "topaccount";
    break;
  case MyMoneyReport::eEquityType:
    m_group = "equitytype";
    break;
  case MyMoneyReport::eAccountType:
    m_group = "type";
    break;
  case MyMoneyReport::eInstitution:
    m_group = "institution";
    break;
  default:
    throw new MYMONEYEXCEPTION("QueryTable::QueryTable(): unhandled row type");
  }
  
  unsigned qc = m_config.queryColumns();

  if ( qc & MyMoneyReport::eQCnumber )
    m_columns += ",number";
  if ( qc & MyMoneyReport::eQCpayee )
    m_columns += ",payee";
  if ( qc & MyMoneyReport::eQCcategory )
    m_columns += ",category";
  if ( qc & MyMoneyReport::eQCaccount )
    m_columns += ",account";
  if ( qc & MyMoneyReport::eQCreconciled )
    m_columns += ",reconcileflag";
  if ( qc & MyMoneyReport::eQCmemo )
    m_columns += ",memo";
  if ( qc & MyMoneyReport::eQCaction )
    m_columns += ",action";
  if ( qc & MyMoneyReport::eQCshares )
    m_columns += ",shares";
  if ( qc & MyMoneyReport::eQCprice )
    m_columns += ",price";

  QString sort;
  if ( ! m_group.isEmpty() )
    sort += m_group + ",";
  sort += m_columns;
  if ( ! m_subtotal.isEmpty() )
    sort += "," + m_subtotal;

  TableRow::setSortCriteria(sort);
  qHeapSort(m_transactions);

}

void QueryTable::constructTransactionTable(void)
{
  //
  // Translate the transaction & split list
  // into a database-style table, which we can then query later SQL-style.
  //

  MyMoneyFile* file = MyMoneyFile::instance();

  MyMoneyReport report(m_config);
  report.setReportAllSplits(false);
  report.setConsiderCategory(true);
  QValueList<MyMoneyTransaction> transactions = file->transactionList( report );
  QValueList<MyMoneyTransaction>::const_iterator it_transaction = transactions.begin();
  while ( it_transaction != transactions.end() )
  {
    TableRow qtransactionrow;

    qtransactionrow["id"] = (*it_transaction).id();
    qtransactionrow["entrydate"] = (*it_transaction).entryDate().toString(Qt::ISODate);
    qtransactionrow["commodity"] = (*it_transaction).commodity();

    QDate postdate = (*it_transaction).postDate();
    qtransactionrow["postdate"] = postdate.toString(Qt::ISODate);

    QDate month = QDate( postdate.year(), postdate.month(), 1);
    qtransactionrow["month"] = i18n("Month of %1").arg(month.toString(Qt::ISODate));

    QDate week = postdate.addDays( 1 - postdate.dayOfWeek() );
    qtransactionrow["week"] = i18n("Week of %1").arg(week.toString(Qt::ISODate));

    // There is a subtle difference between the way this report deals with
    // foreign currencies and the way pivot tables do.  In this transaction
    // report, if you choose not to convert to base currency, all values
    // are reported in the TRANSACTION currency, because it's a transaction-
    // based report.  With pivots, values are reported in the ACCOUNT's
    // currency, because it's an account-based report.

    MyMoneyMoney currencyfactor(1.0);
    if ( report.isConvertCurrency() )
    {
      if((*it_transaction).commodity() != file->baseCurrency().id())
      {
        // I have no idea how to deal with stock accounts quite yet (acejones)
        MyMoneyCurrency currency = file->currency((*it_transaction).commodity());
        currencyfactor = currency.price(postdate);

      }
    }
    else
      if((*it_transaction).commodity() != file->baseCurrency().id())
        qtransactionrow["currency"] = (*it_transaction).commodity();

    // A table row ALWAYS has one asset/liability account.  A transaction
    // will generate one table row for each A/L account.
    //
    // Likewise, a table row ALWAYS has one E/I category.  In the case of complex
    // multi-split transactions, another table, the 'splits table' will be used to
    // hold the extra info in case the user wants to see it.  (Someday when this is
    // written)
    //
    // Splits table handling differs depending on what the user has asked for.
    // If the user wants transactions BY CATEGORY, then multiple table row
    // are generated, one for each category.
    //
    // For now, implementing the simple case...which is one row per-A/L account
    // per E/I category.

    QValueList<MyMoneySplit> splits = (*it_transaction).splits();
    QValueList<MyMoneySplit>::const_iterator it_split = splits.begin();
    while ( it_split != splits.end() )
    {
      QCString splitaccountid = (*it_split).accountId();
      MyMoneyAccount splitaccount = file->account(splitaccountid);
      // Loop through the splits once for every asset/liability account.
      // Create one table row for each such account
      // But skip the account if it is not present in the filter.
      // Also skip the account if we only want investments & it's not an investmet account
      if (
          ( splitaccount.accountGroup() == MyMoneyAccount::Asset || splitaccount.accountGroup() == MyMoneyAccount::Liability ) 
          && 
          m_config.includesAccount( splitaccountid )
          &&
          (!m_config.isInvestmentsOnly() || splitaccount.accountType() == MyMoneyAccount::Stock) 
         )
      {
        TableRow qaccountrow = qtransactionrow;

        // These items are relevant for each A/L split
        QCString payeeid = (*it_split).payeeId();
        if ( payeeid.isEmpty() )
          qaccountrow["payee"] = "[Empty Payee]";
        else
          qaccountrow["payee"] = file->payee(payeeid).name().simplifyWhiteSpace();

        qaccountrow["reconciledate"] = (*it_split).reconcileDate().toString(Qt::ISODate);
        qaccountrow["reconcileflag"] = kReconcileTextChar[(*it_split).reconcileFlag()];
        qaccountrow["number"] = (*it_split).number();
        qaccountrow["action"] = (*it_split).action();
        qaccountrow["memo"] = (*it_split).memo();
        
        // handle investments
        if ( file->account((*it_split).accountId()).accountType() == MyMoneyAccount::Stock )
        {
          MyMoneyMoney shares = (*it_split).shares();
          qaccountrow["shares"] = shares.toString();
          qaccountrow["price"] = (((*it_split).value())*currencyfactor / shares).toString();
          if ( (*it_split).action() == MyMoneySplit::ActionBuyShares && shares < 0 )
            // note this is not localized because neither is MyMoneySplit::action()
            qaccountrow["action"] = "Sell";
          qaccountrow["investaccount"] = file->account(splitaccount.parentAccountId()).name();
        }
        
        QValueList<MyMoneySplit>::const_iterator it_split2 = splits.begin();
        while ( it_split2 != splits.end() )
        {
          // Only process this split if it is not the A/L account we're working with anyway
          if ( (*it_split2).accountId() != (*it_split).accountId() )
          {
            // Create one query line for each target account/category
            // (This is the 'expand categories' behaviour.
            // 'collapse categories' would entail cramming them all into one
            // line and calling it "Split Categories").

            TableRow qsplitrow = qaccountrow;
            qsplitrow["account"] = splitaccount.name();
            qsplitrow["accountid"] = splitaccount.id();

            // Find the top-most parent account
            QCString topaccountid = (*it_split).accountId();
            QCString parentid = file->account(topaccountid).parentAccountId();
            while (! file->isStandardAccount(parentid) )
            {
              topaccountid = parentid;
              parentid = file->account(topaccountid).parentAccountId();
            }
            qsplitrow["topaccount"] = file->account(topaccountid).name();

            // retrieve the value in the transaction's currency, and convert
            // to the base currency if needed
            //qsplitrow["value"] = ((-(*it_split2).value())*currencyfactor).formatMoney();
            qsplitrow["value"] = ((-(*it_split2).value())*currencyfactor).toString();
            qsplitrow["id"] = (*it_split2).id();
            
            // handle sub-categories.  the 'category' field contains the
            // fully-qualified category hierarchy, e.g. "Computers: Hardware: CPUs"
            // the 'topparent' field contains just the top-most parent, in this
            // example "Computers"

            PivotTable::AccountDescriptor acd = (*it_split2).accountId();

            // This final screening level includes the check to exclude non-tax categories
            // in a tax report.  This is something of a hack, although it functions fine.
            // Better would be to handle this at the category selector level with a "Tax
            // Categories" button.  But then we'd need to persist that selection in the
            // filter so that newly changed tax categories get updated, similiar to the
            // 'date lock'.  Until we do that, the current way will work.
            //
            // TODO: Do this!

            // if this is a transfer
            if (
              ( file->account((*it_split2).accountId()).accountGroup() == MyMoneyAccount::Asset || file->account((*it_split2).accountId()).accountGroup() == MyMoneyAccount::Liability  )
              &&
              ! m_config.isTax()
            )
            {
              // skip this split if we have removed the target of the transfer
              // *** Or not *** Lately, I've been thinking that the A/L filter should
              // only remove transactions at the *it_split level.
              //if ( m_config.includesAccount( (*it_split2).accountId() ) )
              {
                QString fromto = ((*it_split2).value()<0)?"from":"to";
                qsplitrow["category"] = i18n("Transfer %1 %2").arg(fromto).arg(acd.fullName());
                qsplitrow["topcategory"] = acd.getTopLevel();
                qsplitrow["categorytype"] = i18n("Transfer");
                m_transactions += qsplitrow;
              }
            }
            else
            {
              if (
                m_config.includesCategory( (*it_split2).accountId() )
                &&
                (
                  ! m_config.isTax() || (file->account((*it_split2).accountId()).value("Tax") == "Yes")
                )
              )
              {
                qsplitrow["category"] = acd.fullName();
                qsplitrow["topcategory"] = acd.getTopLevel();
                qsplitrow["categorytype"] = accountTypeToString(file->account((*it_split2).accountId()).accountGroup());
                m_transactions += qsplitrow;
              }
            }
          }
          ++it_split2;
        }
      }
      ++it_split;
    }
    ++it_transaction;
  }
}

void QueryTable::constructAccountTable(void)
{
  MyMoneyFile* file = MyMoneyFile::instance();

  QValueList<MyMoneyAccount> accounts = file->accountList();
  QValueList<MyMoneyAccount>::const_iterator it_account = accounts.begin();
  while ( it_account != accounts.end() )
  {
    if (
        ( (*it_account).accountGroup() == MyMoneyAccount::Asset || (*it_account).accountGroup() == MyMoneyAccount::Liability ) 
        && 
        m_config.includesAccount( (*it_account).id() )
        &&
        (!m_config.isInvestmentsOnly() || (*it_account).accountType() == MyMoneyAccount::Stock) 
        )
    {
      TableRow qaccountrow;
  
      qaccountrow["account"] = (*it_account).name();
      qaccountrow["accountid"] = (*it_account).id();
  
      // Find the top-most parent account
      QCString topaccountid = (*it_account).id();
      QCString parentid = file->account(topaccountid).parentAccountId();
      while (! file->isStandardAccount(parentid) )
      {
        topaccountid = parentid;
        parentid = file->account(topaccountid).parentAccountId();
      }
      qaccountrow["topaccount"] = file->account(topaccountid).name();
  
      MyMoneyMoney shares = file->balance((*it_account).id(),m_config.toDate());
      qaccountrow["shares"] = shares.toString();
      
      MyMoneyEquity equity = MyMoneyFile::instance()->equity((*it_account).currencyId());
      MyMoneyMoney price = equity.price( m_config.toDate() );
      qaccountrow["price"] = price.toString();
      
      qaccountrow["value"] = ( price * shares ).toString();

      QCString iid = (*it_account).institutionId();
      if ( iid.isEmpty() )
        qaccountrow["institution"] = "None";
      else
        qaccountrow["institution"] = file->institution(iid).name();
      
      qaccountrow["type"] = KMyMoneyUtils::accountTypeToString((*it_account).accountType());
      
      if ( (*it_account).accountType() == MyMoneyAccount::Stock )
        qaccountrow["equitytype"] = KMyMoneyUtils::equityTypeToString(equity.equityType());        
      else
        qaccountrow["equitytype"] = QString();
            
      m_transactions += qaccountrow;
    }
           
    ++it_account;
  }
}

class GroupIterator
  {
  public:
    GroupIterator(const QString& _group, const QString& _subtotal, unsigned _depth): m_depth(_depth), m_groupField(_group), m_subtotalField(_subtotal) {}
    GroupIterator(void) {}
    void update(const QueryTable::TableRow& _row)
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
    
    bool isNewHeader(void) const { return (m_currentGroup != m_previousGroup); }
    bool isSubtotal(void) const { return (m_currentGroup != m_previousGroup) && (!m_previousGroup.isEmpty()); }
    const MyMoneyMoney& subtotal(void) const { return m_previousSubtotal; }
    const unsigned depth(void) const { return m_depth; }
    const QString& name(void) const { return m_currentGroup; }
    const QString& oldName(void) const { return m_previousGroup; }
    const QString& groupField(void) const { return m_groupField; }
    const QString& subtotalField(void) const { return m_subtotalField; }
  private:
    MyMoneyMoney m_currentSubtotal;
    MyMoneyMoney m_previousSubtotal;
    unsigned m_depth;
    QString m_currentGroup;
    QString m_previousGroup;
    QString m_groupField;
    QString m_subtotalField;
  };

void QueryTable::render( QString& result, QString& csv ) const
{
//   MyMoneyMoney::signPosition savesignpos = MyMoneyMoney::negativeMonetarySignPosition();
  unsigned char savethsep = MyMoneyMoney::thousandSeparator();
  
  MyMoneyMoney grandtotal;
  
  result="";
  csv="";
  result += QString("<h2 class=\"report\">%1</h2>\n").arg(m_config.name());
  csv += "\"Report: " + m_config.name() + "\"\n";
  result += QString("<div class=\"subtitle\">");
  if ( m_config.isConvertCurrency() )
  {
    result += i18n("All currencies converted to %1").arg(MyMoneyFile::instance()->baseCurrency().name());
    csv += i18n("All currencies converted to %1\n").arg(MyMoneyFile::instance()->baseCurrency().name());
  }
  else
  {
    result += i18n("All values shown in %1 unless otherwise noted").arg(MyMoneyFile::instance()->baseCurrency().name());
    csv += i18n("All values shown in %1 unless otherwise noted\n").arg(MyMoneyFile::instance()->baseCurrency().name());
  }
  result += QString("</div>\n");
  result += QString("<div class=\"gap\">&nbsp;</div>\n");

  // retrieve the configuration parameters from the report definition.
  // the things that we care about for query reports are:
  // how to group the rows, what columns to display, and what field
  // to subtotal on
  QStringList groups = QStringList::split(",",m_group);
  QStringList columns = QStringList::split(",",m_columns);
  columns += m_subtotal;

  //
  // Table header
  //

  QMap<QString,QString> i18nHeaders;
  i18nHeaders["postdate"] = i18n("Date");
  i18nHeaders["value"] = i18n("Amount");
  i18nHeaders["number"] = i18n("Num");
  i18nHeaders["payee"] = i18n("Payee");
  i18nHeaders["category"] = i18n("Category");
  i18nHeaders["account"] = i18n("Account");
  i18nHeaders["memo"] = i18n("Memo");
  i18nHeaders["topcategory"] = i18n("Top Category");
  i18nHeaders["categorytype"] = i18n("Category Type");
  i18nHeaders["month"] = i18n("Month");
  i18nHeaders["week"] = i18n("Week");
  i18nHeaders["reconcileflag"] = i18n("R");
  i18nHeaders["action"] = i18n("Action");
  i18nHeaders["shares"] = i18n("Shares");
  i18nHeaders["price"] = i18n("Price");
  i18nHeaders["latestprice"] = i18n("Price");
  i18nHeaders["netinvvalue"] = i18n("Net Value");
  
  // the list of columns which represent money, so we can display them correctly
  QStringList moneyColumns = QStringList::split(",","value,shares,price,latestprice,netinvvalue");

  result += "<table class=\"report\">\n<tr class=\"itemheader\">";

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

  result += "</tr>\n";
  csv = csv.left( csv.length() - 1 );
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
    groupIteratorList += GroupIterator((*it_grouplevel),m_subtotal,depth++);
    ++it_grouplevel;
  }

  //
  // Rows
  //

  bool row_odd = true;
  QValueList<TableRow>::const_iterator it_row = m_transactions.begin();
  while ( it_row != m_transactions.end() )
  {
    //
    // Process Groups
    //

    // There's a subtle bug here.  If an earlier group gets a new group,
    // then we need to force all the downstream groups to get one too.

    // Update the group iterators with the current row value
    QValueList<GroupIterator>::iterator it_group = groupIteratorList.begin();
    while ( it_group != groupIteratorList.end() )
    {
      (*it_group).update(*it_row);
      ++it_group;
    }
    // Do subtotals backwards
    if ( m_config.isConvertCurrency() )
    {
      it_group = groupIteratorList.fromLast();
      while ( it_group != groupIteratorList.end() )
      {
        if ((*it_group).isSubtotal())
        {
          if ( (*it_group).depth() == 1 )
            grandtotal += (*it_group).subtotal();
          
          QString subtotal_html = (*it_group).subtotal().formatMoney();
          MyMoneyMoney::setThousandSeparator('\0');
          QString subtotal_csv = (*it_group).subtotal().formatMoney();
          MyMoneyMoney::setThousandSeparator(savethsep);

          result += "<tr class=\"sectionfooter\">"
            "<td class=\"left"+ QString::number(((*it_group).depth()-1)) + "\" "
            "colspan=\"" + QString::number(columns.count()-1) + "\">"+
            i18n("Total")+" " + (*it_group).oldName() + "</td>"
            "<td>" + subtotal_html + "</td></tr>\n";
          csv += "\"" + i18n("Total") + " " + (*it_group).oldName() + "\"," + subtotal_csv + "\n";
        }
        --it_group;
      }
    }
    // And headers forwards
    it_group = groupIteratorList.begin();
    while ( it_group != groupIteratorList.end() )
    {
      if ((*it_group).isNewHeader())
      {
        row_odd = true;
        result += "<tr class=\"sectionheader\">"
          "<td class=\"left"+ QString::number(((*it_group).depth()-1)) + "\" "
          "colspan=\"" + QString::number(columns.count()) + "\">" +
          (*it_group).name() + "</td></tr>\n";
        csv += "\"" + (*it_group).name() + "\"\n";
      }
      ++it_group;
    }
    //
    // Columns
    //

    result += QString("<tr class=\"%1\">").arg(row_odd?"row-odd ":"row-even");
    QStringList::const_iterator it_column = columns.begin();
    while ( it_column != columns.end() )
    {
      QString data = (*it_row)[*it_column];
      
      if ( moneyColumns.contains(*it_column) )
      {
        result += QString("<td>%1&nbsp;%2</td>")
          .arg((*it_row)["currency"])
          .arg(MyMoneyMoney(data).formatMoney());

        MyMoneyMoney::setThousandSeparator('\0');

        csv += (*it_row)["currency"] + " " + MyMoneyMoney(data).formatMoney() + ",";

        MyMoneyMoney::setThousandSeparator(savethsep);

      }
      else
      {
        result += QString("<td class=\"left\">%1</td>")
          .arg(data);
        csv += "\""+ data + "\",";
      }
      ++it_column;
    }

    result += "</tr>\n";
    csv = csv.left( csv.length() - 1 );
    csv += "\n";
    row_odd = ! row_odd;
    ++it_row;
  }
  
  //
  // Final group totals
  //

  // Do subtotals backwards
  if ( m_config.isConvertCurrency() )
  {
    QValueList<GroupIterator>::iterator it_group = groupIteratorList.fromLast();
    while ( it_group != groupIteratorList.end() )
    {
      (*it_group).update(TableRow());
      
      if ( (*it_group).depth() == 1 )
        grandtotal += (*it_group).subtotal();

      QString subtotal_html = (*it_group).subtotal().formatMoney();
      MyMoneyMoney::setThousandSeparator('\0');
      QString subtotal_csv = (*it_group).subtotal().formatMoney();
      MyMoneyMoney::setThousandSeparator(savethsep);

      result += "<tr class=\"sectionfooter\">"
        "<td class=\"left"+ QString::number((*it_group).depth()-1) + "\" "
        "colspan=\"" + QString::number(columns.count()-1) + "\">"+
        i18n("Total")+" " + (*it_group).oldName() + "</td>"
        "<td>" + subtotal_html + "</td></tr>\n";
      csv += "\"" + i18n("Total") + " " + (*it_group).oldName() + "\"," + subtotal_csv + "\n";
      --it_group;
    }

    //
    // Grand total
    //
    
    QString grandtotal_html = grandtotal.formatMoney();
    MyMoneyMoney::setThousandSeparator('\0');
    QString grandtotal_csv = grandtotal.formatMoney();
    MyMoneyMoney::setThousandSeparator(savethsep);

    result += "<tr class=\"sectionfooter\">"
      "<td class=\"left0\" "
      "colspan=\"" + QString::number(columns.count()-1) + "\">"+
      i18n("Grand Total") + "</td>"
      "<td>" + grandtotal_html + "</td></tr>\n";
    csv += "\"" + i18n("Grand Total") + "\"," + grandtotal_csv + "\n";
  }
  result += "</table>\n";
}

QString QueryTable::renderHTML( void ) const
{
  QString html, csv;
  render( html,csv );
  return html;
}

QString QueryTable::renderCSV( void ) const
{
  QString html, csv;
  render( html,csv );
  return csv;
}

void QueryTable::dump( const QString& file, const QString& context ) const
{
  QFile g( file );
  g.open( IO_WriteOnly );

  if ( ! context.isEmpty() )
    QTextStream(&g) << context.arg(renderHTML());
  else
    QTextStream(&g) << renderHTML();
  g.close();
}

}
// vim:cin:si:ai:et:ts=2:sw=2:
