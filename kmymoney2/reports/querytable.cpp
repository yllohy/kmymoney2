/***************************************************************************
                          querytable.cpp
                         -------------------
    begin                : Fri Jul 23 2004
    copyright            : (C) 2004-2005 by Ace Jones
    email                :  acejones@users.sourceforge.net
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
#include "../mymoney/mymoneytransaction.h"
#include "../mymoney/mymoneyreport.h"
#include "../mymoney/mymoneyexception.h"
#include "../kmymoneyutils.h"
#include "reportaccount.h"
#include "reportdebug.h"
#include "querytable.h"

namespace reports {

// this should be in mymoneysplit.h
static const QStringList kReconcileText = QStringList::split(",","notreconciled,cleared,reconciled,frozen,none");
static const QStringList kReconcileTextChar = QStringList::split(",","N,C,R,F,none");

QStringList QueryTable::TableRow::m_sortCriteria;

// ****************************************************************************
//
// CashFlowListItem implementation
//
//   Cash flow analysis tools for investment reports
//
// ****************************************************************************

QDate CashFlowListItem::m_sToday = QDate::currentDate();

MyMoneyMoney CashFlowListItem::NPV( double _rate ) const
{
  double T = static_cast<double>(m_sToday.daysTo(m_date)) / 365.0;
  MyMoneyMoney result = m_value.toDouble() / pow(1+_rate,T);

  //kdDebug(2) << "CashFlowListItem::NPV( " << _rate << " ) == " << result << endl;

  return result;
}

// ****************************************************************************
//
// CashFlowList implementation
//
//   Cash flow analysis tools for investment reports
//
// ****************************************************************************

const CashFlowListItem& CashFlowList::mostRecent(void) const
{
  CashFlowList dupe( *this );
  qHeapSort( dupe );

  //kdDebug(2) << " CashFlowList::mostRecent() == " << dupe.back().date().toString(Qt::ISODate) << endl;

  return dupe.back();
}

MyMoneyMoney CashFlowList::NPV( double _rate ) const
{
  MyMoneyMoney result = 0.0;

  const_iterator it_cash = begin();
  while ( it_cash != end() )
  {
    result += (*it_cash).NPV( _rate );
    ++it_cash;
  }

  //kdDebug(2) << "CashFlowList::NPV( " << _rate << " ) == " << result << endl << "------------------------" << endl;

  return result;
}

double CashFlowList::IRR( void ) const
{
  double result = 0.0;

  // set 'today', which is the most recent of all dates in the list
  CashFlowListItem::setToday( mostRecent().date() );

  double lobound = 0.0; // this is as low as we support
  double hibound = 1.00;
  double precision = 0.00001; // how precise do we want the answer?

  // first, see if one of the bounds themselves is the final answer
  MyMoneyMoney hinpv = NPV( hibound );
  MyMoneyMoney lonpv = NPV( lobound );

  if ( lonpv.isZero() )
  {
    result = lobound;
    goto done;
  }
  else if ( hinpv.isZero() )
  {
    result = hibound;
    goto done;
  }

  // our next goal is to ensure that NPV==0 lies somewhere in between lobound & hibound.
  // this means that NPV(lobound) & NPV(hibound) must be opposite signs.
  // move the bounds out until this initial condition is satisfied

  while ( ( hinpv.isPositive() ) == ( lonpv.isPositive() ) )
  {
    hibound *= 10;
    if ( lobound == 0.0 )
      lobound = -0.000999;
    else
      lobound *= 10;

    if ( lobound < -1.0 )
      throw QString("IRR is < -100%, not supported");

    hinpv = NPV( hibound );
    lonpv = NPV( lobound );
  }

  // now that we have a suitable low and high bound to start with,
  // figure out which half of the range 0 falls in, and then use
  // that range.  Continue until the range is suitably small,
  // and return the midpoint of it.

  while ( (hibound-lobound) > precision )
  {
    double ratio = lonpv.toDouble()/(hinpv.toDouble()-lonpv.toDouble());
    if ( ratio < 0.0 )
      ratio = -ratio;
    result = lobound + (hibound-lobound)*ratio;

    MyMoneyMoney npv = NPV( result );

    // if we've found zero, we're done!
    if ( npv.isZero() )
      goto done;

    // if lobound & result have the opposite sign, then use the range from lobound to result
    if ( lonpv.isPositive() != npv.isPositive() )
    {
      hibound = result;
      hinpv = npv;
    }

    // else hibound & result have the opposite sign, so use the range from result to hibound
    else
    {
      lobound = result;
      lonpv = npv;
    }
  }

done:
  return result;
}

MyMoneyMoney CashFlowList::total(void) const
{
  MyMoneyMoney result;

  const_iterator it_cash = begin();
  while ( it_cash != end() )
  {
    result += (*it_cash).value();
    ++it_cash;
  }

  return result;
}

void CashFlowList::dumpDebug(void) const
{
  const_iterator it_item = begin();
  while ( it_item != end() )
  {
    kdDebug(2) << (*it_item).date().toString(Qt::ISODate) << " " << (*it_item).value().toString() << endl;
    ++it_item;
  }
}

// ****************************************************************************
//
// Group Iterator
//
// ****************************************************************************

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
  const MyMoneyMoney& currenttotal(void) const { return m_currentSubtotal; }
  const unsigned depth(void) const { return m_depth; }
  const QString& name(void) const { return m_currentGroup; }
  const QString& oldName(void) const { return m_previousGroup; }
  const QString& groupField(void) const { return m_groupField; }
  const QString& subtotalField(void) const { return m_subtotalField; }
  // ***DV*** HACK make the currentGroup test different but look the same
  void force (void) { m_currentGroup += " "; }
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
// QueryTable implementation
//
// ****************************************************************************

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
  // seperated into its own method to allow debugging (setting breakpoints
  // directly in ctors somehow does not work for me (ipwizard))
  // TODO: remove the init() method and move the code back to the ctor
  init();
}

void QueryTable::init(void)
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

    case MyMoneyReport::eAccount:
      constructTransactionTable();
      m_columns="accountid,postdate";
      break;

    case MyMoneyReport::ePayee:
    case MyMoneyReport::eMonth:
    case MyMoneyReport::eWeek:
      constructTransactionTable();
      m_columns="postdate,account";
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
    m_group = "institution,topaccount";
    break;
  default:
    throw new MYMONEYEXCEPTION("QueryTable::QueryTable(): unhandled row type");
  }

  QString sort = m_group + "," + m_columns + ",id,rank";

  switch (m_config.rowType()) {
    case MyMoneyReport::eAccountByTopAccount:
    case MyMoneyReport::eEquityType:
    case MyMoneyReport::eAccountType:
    case MyMoneyReport::eInstitution:
      m_columns="account";
      break;

    default:
      m_columns="postdate";
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
  if ( qc & MyMoneyReport::eQCperformance )
    m_columns += ",startingbal,buys,sells,reinvestincome,cashincome,return";
  if ( qc & MyMoneyReport::eQCloan )
  {
    m_columns += ",payment,interest,fees";
    m_postcolumns = "balance";
  }
  if ( qc & MyMoneyReport::eQCbalance)
    m_postcolumns = "balance";

  TableRow::setSortCriteria(sort);
  qHeapSort(m_transactions);
}

void QueryTable::constructTransactionTable(void)
{
  MyMoneyFile* file = MyMoneyFile::instance();

  MyMoneyReport report(m_config);
  report.setReportAllSplits(false);
  report.setConsiderCategory(true);

  bool use_transfers;
  bool use_summary;
  bool hide_details;

  switch (m_config.rowType()) {
  case MyMoneyReport::eCategory:
  case MyMoneyReport::eTopCategory:
    use_summary = false;
    use_transfers = false;
    hide_details = false;
    break;

  default:
    use_summary = true;
    use_transfers = true;
    hide_details = (m_config.detailLevel() == MyMoneyReport::eDetailNone);
    break;
  }

  // support for opening and closing balances
  QMap<QString, MyMoneyAccount> accts;

  QValueList<MyMoneyTransaction> T = file->transactionList(report);
  QValueList<MyMoneyTransaction>::const_iterator it, T_end;
  T_end = T.end();

  for (it = T.begin(); it != T_end; ++it) {

    TableRow qA, qS;
    QDate pd;

    qA["id"] = qS["id"] = (* it).id();
    qA["entrydate"] = qS["entrydate"] = (* it).entryDate().toString(Qt::ISODate);
    qA["postdate"] = qS["postdate"] = (* it).postDate().toString(Qt::ISODate);
    qA["commodity"] = qS["commodity"] = (* it).commodity();

    pd = (* it).postDate();
    qA["month"] = qS["month"] = i18n("Month of %1").arg(QDate(pd.year(),pd.month(),1).toString(Qt::ISODate));
    qA["week"] = qS["week"] = i18n("Week of %1").arg(pd.addDays(1-pd.dayOfWeek()).toString(Qt::ISODate));

    MyMoneyMoney xr(1.0);
    qA["currency"] = qS["currency"] = "";

    if((* it).commodity() != file->baseCurrency().id()) {
      if (report.isConvertCurrency()) {
        MyMoneySecurity c = file->currency((* it).commodity());
        xr = file->price(c.id(), file->baseCurrency().id(), (* it).postDate()).rate(file->baseCurrency().id());
      }
      else {
        qA["currency"] = qS["currency"] = (* it).commodity();
      }
    }

    // to handle splits, we decide on which account to base the split
    // (a reference point or point of view so to speak). here we take the
    // first account that is a stock account or loan account (or the first account
    // that is not an income or expense account if there is no stock or loan account)
    // to be the account (qA) that will have the sub-item "split" entries. we add
    // one transaction entry (qS) for each subsequent entry in the split.

    const QValueList<MyMoneySplit>& S = (* it).splits();
    QValueList<MyMoneySplit>::const_iterator myBegin, is, S_end;
    S_end = S.end();

    myBegin = S_end;
    for (is = S.begin(); is != S_end; ++is) {
      ReportAccount a = (* is).accountId();
      // always put split with a "stock" account if it exists
      if (a.isInvest())
        break;

      // prefer to put splits with a "loan" account if it exists
      if(a.isLoan())
        myBegin = is;

      if((myBegin == S_end) && ! a.isIncomeExpense()) {
        myBegin = is;
      }
    }

    // select our "reference" split
    if (is == S_end) is = myBegin; else myBegin = is;


    ReportAccount a = (* is).accountId();

    // for "loan" reports, the loan transaction gets special treatment.
    // the splits of a loan transaction are placed on one line in the
    // reference (loan) account (qA). however, we process the matching
    // split entries (qS) normally.

    bool loan_special_case = ((m_config.queryColumns() & MyMoneyReport::eQCloan) && a.isLoan());

#if 0
    // a stock dividend or yield transaction is also a special case.
    // [dv: the original comment follows]
    // handle cash dividends. these little fellas require very special handling.
    // the stock account will produce a row with zero value & zero shares. Then
    // there will be 2 split rows, a category and a transfer account. We are
    // only concerned with the transfer account, and we will NOT show the income
    // account. (This may have to be changed later if we feel we need it.)

    // [dv: this special case just doesn't make sense to me -- it seems to
    // violate the "zero sum" transaction concept. for now, then, the stock
    // dividend / yield special case goes unimplemented.]

    bool stock_special_case =
      (a.isInvest() &&
       ((* is).action() == MyMoneySplit::ActionDividend ||
        (* is).action() == MyMoneySplit::ActionYield));
#endif

    bool include_me = true;
    QString a_fullname = "";
    QString a_memo = "";

    do {

      ReportAccount a = (* is).accountId();
      QCString i = a.institutionId();
      QCString p = (* is).payeeId();

      if (a.isInvest()) {

        // use the institution of the parent for stock accounts
        i = a.parent().institutionId();

        MyMoneyMoney sh = (* is).shares();



        qA["shares"] = sh.isZero() ? "" : sh.toString();
        qA["price"] = sh.isZero() ? "" : ((* is).value()*xr/sh).toString();

        qA["action"] = (* is).action();

        if (((*is).action() == MyMoneySplit::ActionBuyShares) && sh.isNegative())
          qA["action"] = "Sell";

        qA["investaccount"] = a.parent().name();
      }

      if (is == myBegin) {

        include_me = m_config.includes(a);
        a_fullname = a.fullName();
        a_memo = (* is).memo();

        if (m_config.isConvertCurrency() && a.isForeignCurrency()) {
          xr = a.baseCurrencyPrice((* it).postDate()).reduce();
          qA["price"] = a.baseCurrencyPrice((* it).postDate()).reduce().toString();
        }

        qA["account"] = a.name();
        qA["accountid"] = a.id();
        qA["topaccount"] = a.topParentName();

        qA["institution"] = i.isEmpty()
          ? i18n("No Institution")
          : file->institution(i).name();

        qA["payee"] = p.isEmpty()
          ? i18n("[Empty Payee]")
          : file->payee(p).name().simplifyWhiteSpace();

        qA["reconciledate"] = (* is).reconcileDate().toString(Qt::ISODate);
        qA["reconcileflag"] = kReconcileTextChar[(* is).reconcileFlag()];
        qA["number"] = (* is).number();
        // qA["action"] = (* is).action();
        qA["memo"] = a_memo;

        qS["reconciledate"] = qA["reconciledate"];
        qS["reconcileflag"] = qA["reconcileflag"];
        qS["number"] = qA["number"];
        // qS["action"] = qA["action"];

        qS["topcategory"] = a.topParentName();
        qS["categorytype"] = i18n("Transfer");

        // only include the configured accounts
        if (include_me) {

          if (loan_special_case) {

            // put the principal amount in the "value" column
            qA["value"] = ((-(* is).value()) * xr).toString();
            qA["rank"] = "0";
            qA["split"] = "";

          }
          else {

            if ((S.count() > 2) && use_summary) {

              // add the "summarized" split transaction
              // this is the sub-total of the split detail
              qA["value"] = ((* is).value() * xr).toString();
              qA["rank"] = "0";
              qA["category"] = i18n("[Split Transaction]");
              qA["topcategory"] = i18n("Split");
              qA["categorytype"] = i18n("Split");

              m_transactions += qA;
            }
          }

          // track accts that will need opening and closing balances
          accts.insert (a.id(), a);
        }

      }
      else {

        if (include_me) {

          if (loan_special_case) {

            if ((*is).action() == MyMoneySplit::ActionAmortization) {
              // put the payment in the "payment" column
              qA["payment"] = ((-(* is).value()) * xr).toString();
            }
            else if ((*is).action() == MyMoneySplit::ActionInterest) {
              // put the interest in the "interest" column
              qA["interest"] = ((-(* is).value()) * xr).toString();
            }
            else if (S.count() > 2) {
              // [dv: This comment carried from the original code. I am
              // not exactly clear on what it means or why we do this.]
              // Put the initial pay-in nowhere (that is, ignore it). This
              // is dangerous, though. The only way I can tell the initial
              // pay-in apart from fees is if there are only 2 splits in
              // the transaction.  I wish there was a better way.
            }
            else {
              // accumulate everything else in the "fees" column
              MyMoneyMoney n0 = MyMoneyMoney(qA["fees"]);
              MyMoneyMoney n1 = ((-(* is).value()) * xr).toString();
              qA["fees"] = (n0 + n1).toString();
            }
            // we don't add qA here for a loan transaction. we'll add one
            // qA afer all of the split components have been processed.
            // (see below)

          }

          //--- special case to hide split transaction details
          else if (hide_details && (S.count() > 2)) {
            // essentially, don't add any qA entries
          }

          //--- default case includes all transaction details
          else {

            if ((S.count() > 2) && use_summary) {
              qA["value"] = "";
              qA["split"] = ((-(* is).value()) * xr).toString();
              qA["rank"] = "1";
            }
            else {
              qA["split"] = "";
              qA["value"] = ((-(* is).value()) * xr).toString();
              qA["rank"] = "0";
            }

            qA ["memo"] = (* is).memo();

            if (! a.isIncomeExpense()) {
              qA["category"] = ((* is).value().isNegative())
                ? i18n("Transfer from %1").arg(a.fullName())
                : i18n("Transfer to %1").arg(a.fullName());
              qA["topcategory"] = a.topParentName();
              qA["categorytype"] = i18n("Transfer");
            }
            else {
              qA ["category"] = a.fullName();
              qA ["topcategory"] = a.topParentName();
              qA ["categorytype"] = accountTypeToString(a.accountGroup());
            }

            if (use_transfers || (a.isIncomeExpense() && m_config.includesCategory(a.id())))
              m_transactions += qA;
          }
        }

        if (m_config.includes(a) && use_transfers) {
          if (! a.isIncomeExpense()) {

            qS["value"] = ((* is).value() * xr).toString();
            qS["rank"] = "0";

            qS["account"] = a.name();
            qS["accountid"] = a.id();
            qS["topaccount"] = a.topParentName();

            qS["category"] = ((* is).value().isNegative())
              ? i18n("Transfer to %1").arg(a_fullname)
              : i18n("Transfer from %1").arg(a_fullname);

            qS["institution"] = i.isEmpty()
              ? i18n("No Institution")
              : file->institution(i).name();

            qS["memo"] = (* is).memo().isEmpty()
              ? a_memo
              : (* is).memo();

            qS["payee"] = p.isEmpty()
              ? qA["payee"]
              : file->payee(p).name().simplifyWhiteSpace();

            m_transactions += qS;

            // track accts that will need opening and closing balances
            accts.insert (a.id(), a);
          }
        }
      }

      ++is;
      // look for wrap-around
      if (is == S_end) is = S.begin();

    } while (is != myBegin);

    if (loan_special_case) {
      m_transactions += qA;
    }
  }

  // now run through our accts list and add opening and closing balances

  switch (m_config.rowType()) {
    case MyMoneyReport::eAccount:
    case MyMoneyReport::eTopAccount:
      break;

    // case MyMoneyReport::eCategory:
    // case MyMoneyReport::eTopCategory:
    // case MyMoneyReport::ePayee:
    // case MyMoneyReport::eMonth:
    // case MyMoneyReport::eWeek:
    default:
      return;
  }

  QDate date0;
  QDate date1;

  report.validDateRange(date0, date1);
  QString date0s = date0.toString(Qt::ISODate);
  QString date1s = date1.toString(Qt::ISODate);
  date0 = date0.addDays(-1);

  QMap<QString, MyMoneyAccount>::const_iterator ia, accts_end;
  accts_end = accts.end();

  for (ia = accts.begin(); ia != accts_end; ++ia) {

    TableRow qA;

    ReportAccount a = (* ia);

    QCString i = a.institutionId();

    // use the institution of the parent for stock accounts
    if (a.isInvest())
      i = a.parent().institutionId();

    MyMoneyMoney b0,b1, s0,s1, p0,p1;

    if (a.isInvest()) {
      MyMoneySecurity s = file->security(a.currencyId());
      s0 = file->balance(a.id(),date0);
      s1 = file->balance(a.id(),date1);
      p0 = file->price(s.id(), QCString(), date0).rate(QCString());
      p1 = file->price(s.id(), QCString(), date1).rate(QCString());
      b0 = s0 * p0;
      b1 = s1 * p1;
    }
    else {
      b0 = file->balance(a.id(),date0);
      b1 = file->balance(a.id(),date1);
    }

    MyMoneyMoney xr(1.0);

    // adjust exchange rate
    if (m_config.isConvertCurrency() && a.isForeignCurrency())
      xr = a.baseCurrencyPrice(m_config.fromDate()).reduce();

    //starting balance
    // don't show currency if we're converting or if it's not foreign
    qA["currency"] = (m_config.isConvertCurrency() || ! a.isForeignCurrency()) ? "" : a.currency();

    qA["accountid"] = a.id();
    qA["account"] = a.name();
    qA["topaccount"] = a.topParentName();
    qA["institution"] = i.isEmpty() ? i18n("No Institution") : file->institution(i).name();
    qA["rank"] = "-2";

    if (a.isInvest()) {
      qA["price"] = (p0 * xr).toString();
      qA["shares"] = s0.toString();
    }

    if (m_config.isConvertCurrency() && a.isForeignCurrency())
      qA["price"] = xr.toString();

    qA["postdate"] = date0s;
    qA["balance"] = (b0 * xr).toString();
    qA["value"] = QString();
    qA["id"] = "A";
    m_transactions += qA;

    //ending balance
    if (m_config.isConvertCurrency() && a.isForeignCurrency()) {
      xr = a.baseCurrencyPrice(m_config.toDate()).reduce();
      qA["price"] = xr.toString();
    }

    if (a.isInvest()) {
      qA["price"] = (p1 * xr).toString();
      qA["shares"] = s1.toString();
    }

    qA["postdate"] = date1s;
    qA["balance"] = (b1 * xr).toString();
    qA["id"] = "Z";
    m_transactions += qA;
  }
}

void QueryTable::constructPerformanceRow( const ReportAccount& account, TableRow& result, const MyMoneyMoney& displayprice ) const
{
  MyMoneyFile* file = MyMoneyFile::instance();
  MyMoneySecurity security = file->security(account.currencyId());

  result["equitytype"] = KMyMoneyUtils::securityTypeToString(security.securityType());

  //
  // Calculate performance
  //

  // The following columns are created:
  //    Account, Value on <Opening>, Buys, Sells, Income, Value on <Closing>, Return%

  MyMoneyReport report = m_config;
  QDate startingDate;
  QDate endingDate;
  MyMoneyMoney price;
  report.validDateRange( startingDate, endingDate );
  startingDate = startingDate.addDays(-1);

  if ( m_config.isConvertCurrency() ) {
    price = file->price(security.id(), QCString(), startingDate).rate(QCString()) * account.baseCurrencyPrice(startingDate);
  } else {
    price = file->price(security.id(), QCString(), startingDate).rate(QCString());
  }
  MyMoneyMoney startingBal = file->balance(account.id(),startingDate) * price;
  if ( m_config.isConvertCurrency() ) {
    price = file->price(security.id(), QCString(), endingDate).rate(QCString()) * account.baseCurrencyPrice(endingDate);
  } else {
    price = file->price(security.id(), QCString(), endingDate).rate(QCString());
  }
  MyMoneyMoney endingBal = file->balance((account).id(),endingDate) * price;
  CashFlowList buys;
  CashFlowList sells;
  CashFlowList reinvestincome;
  CashFlowList cashincome;

  report.setReportAllSplits(false);
  report.setConsiderCategory(true);
  report.clearAccountFilter();
  report.addAccount(account.id());
  QValueList<MyMoneyTransaction> transactions = file->transactionList( report );
  QValueList<MyMoneyTransaction>::const_iterator it_transaction = transactions.begin();
  while ( it_transaction != transactions.end() )
  {
    // s is the split for the stock account
    MyMoneySplit s = (*it_transaction).splitByAccount(account.id());

    const QCString& action = s.action();
    if ( action == MyMoneySplit::ActionBuyShares )
    {
      if ( s.value().isPositive() )
        buys += CashFlowListItem( (*it_transaction).postDate(), -s.value() );
      else
        sells += CashFlowListItem( (*it_transaction).postDate(), -s.value() );
    }
    else if ( action == MyMoneySplit::ActionReinvestDividend )
    {
      reinvestincome += CashFlowListItem( (*it_transaction).postDate(), s.value() );
    }
    else if ( action == MyMoneySplit::ActionDividend || action == MyMoneySplit::ActionYield )
    {
      // find the split with the category, which has the actual amount of the dividend
      QValueList<MyMoneySplit> splits = (*it_transaction).splits();
      QValueList<MyMoneySplit>::const_iterator it_split = splits.begin();
      bool found = false;
      while( it_split != splits.end() )
      {
        ReportAccount acc = (*it_split).accountId();
        if ( acc.isIncomeExpense() )
        {
          found = true;
          break;
        }
        ++it_split;
      }

      if ( found )
        cashincome += CashFlowListItem( (*it_transaction).postDate(), -(*it_split).value() );
    }
    ++it_transaction;
  }

  // Note that reinvested dividends are not included , because these do not
  // represent a cash flow event.
  CashFlowList all;
  all += buys;
  all += sells;
  all += cashincome;
  all += CashFlowListItem(startingDate,-startingBal);
  all += CashFlowListItem(endingDate,endingBal);

  try
  {
    result["return"] = MyMoneyMoney(all.IRR(),10000).toString();
  }
  catch (QString e)
  {
    kdDebug(2) << e << endl;
  }

  result["buys"] = (-(buys.total())*displayprice).toString();
  result["sells"] = (-(sells.total())*displayprice).toString();
  result["cashincome"] = (cashincome.total()*displayprice).toString();
  result["reinvestincome"] = (reinvestincome.total()*displayprice).toString();
  result["startingbal"] = (startingBal*displayprice).toString();
  result["endingbal"] = (endingBal*displayprice).toString();
}

void QueryTable::constructAccountTable(void)
{
  MyMoneyFile* file = MyMoneyFile::instance();

  QValueList<MyMoneyAccount> accounts;
  file->accountList(accounts);
  QValueList<MyMoneyAccount>::const_iterator it_account = accounts.begin();
  while ( it_account != accounts.end() )
  {
    ReportAccount account = *it_account;

    // Note, "Investment" accounts are never included in account rows because
    // they don't contain anything by themselves.  In reports, they are only
    // useful as a "topaccount" aggregator of stock accounts
    if ( account.isAssetLiability() && m_config.includes(account) && account.accountType() != MyMoneyAccount::Investment )
    {
      TableRow qaccountrow;

      // help for sort and render functions
      qaccountrow["rank"] = "0";

      //
      // Handle currency conversion
      //

      MyMoneyMoney displayprice(1.0);
      if ( m_config.isConvertCurrency() )
      {
        // display currency is base currency, so set the price
        if ( account.isForeignCurrency() )
          displayprice = account.baseCurrencyPrice(m_config.toDate()).reduce();
      }
      else
      {
        // display currency is the account's deep currency.  display this fact in the report
        qaccountrow["currency"] = account.currency();
      }

      qaccountrow["account"] = account.name();
      qaccountrow["accountid"] = account.id();
      qaccountrow["topaccount"] = account.topParentName();

      MyMoneyMoney shares = file->balance(account.id(),m_config.toDate());
      qaccountrow["shares"] = shares.toString();

      MyMoneyMoney netprice = account.deepCurrencyPrice(m_config.toDate()).reduce() * displayprice;
      qaccountrow["price"] = ( netprice.reduce() ).toString();
      qaccountrow["value"] = ( netprice.reduce() * shares.reduce() ).toString();

      QCString iid = (*it_account).institutionId();

      // If an account does not have an institution, get it from the top-parent.
      if ( iid.isEmpty() && ! account.isTopLevel() )
      {
        ReportAccount topaccount = account.topParent();
        iid = topaccount.institutionId();
      }

      if ( iid.isEmpty() )
        qaccountrow["institution"] = i18n("None");
      else
        qaccountrow["institution"] = file->institution(iid).name();

      qaccountrow["type"] = KMyMoneyUtils::accountTypeToString((*it_account).accountType());

      // TODO: Only do this if the report we're making really needs performance.  Otherwise
      // it's an expensive calculation done for no reason
      if ( account.isInvest() )
      {
        constructPerformanceRow( account, qaccountrow, displayprice );
      }
      else
        qaccountrow["equitytype"] = QString();

      // don't add the account if it is closed. In fact, the business logic
      // should prevent that an account can be closed with a balance not equal
      // to zero, but we never know.
      if(!(shares.isZero() && account.isClosed()))
        m_transactions += qaccountrow;
    }

    ++it_account;
  }
}
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
  QStringList postcolumns = QStringList::split(",",m_postcolumns);
  columns += postcolumns;

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
  i18nHeaders["buys"] = i18n("Buys");
  i18nHeaders["sells"] = i18n("Sells");
  i18nHeaders["reinvestincome"] = i18n("Dividends Reinvested");
  i18nHeaders["cashincome"] = i18n("Dividends Paid Out");
  i18nHeaders["startingbal"] = i18n("Starting Balance");
  i18nHeaders["endingbal"] = i18n("Ending Balance");
  i18nHeaders["return"] = i18n("Annualized Return");
  i18nHeaders["fees"] = i18n("Fees");
  i18nHeaders["interest"] = i18n("Interest");
  i18nHeaders["payment"] = i18n("Payment");
  i18nHeaders["balance"] = i18n("Balance");

  // the list of columns which represent money, so we can display them correctly
  QStringList moneyColumns = QStringList::split(",","value,shares,price,latestprice,netinvvalue,buys,sells,cashincome,reinvestincome,startingbal,fees,interest,payment,balance");

  // the list of columns which represent shares, which is like money except the
  // transaction currency will not be displayed
  QStringList sharesColumns = QStringList::split(",","shares");

  // the list of columns which represent a percentage, so we can display them correctly
  QStringList percentColumns = QStringList::split(",","return");

  // the list of columns which represent dates, so we can display them correctly
  QStringList dateColumns = QStringList::split(",", "postdate,entrydate");

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

  // ***DV***
  MyMoneyMoney startingBalance;
  for (QValueList<TableRow>::const_iterator it_row = m_transactions.begin();
       it_row != m_transactions.end();
       ++it_row) {

    //
    // Process Groups
    //

    // ***DV*** HACK to force a subtotal and header, since this render doesn't
    // always detect a group change for different accounts with the same name
    // (as occurs with the same stock purchased from different investment accts)
    if (it_row != m_transactions.begin())
      if (((* it_row)["rank"] == "-2") && ((* it_row)["id"] == "A"))
        (groupIteratorList.last()).force();

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
          QString subtotal_csv = (*it_group).subtotal().formatMoney("", 2, true);

          // ***DV*** HACK fix the side-effiect from .force() method above
          QString oldName = QString((*it_group).oldName()).stripWhiteSpace();

          result +=
            "<tr class=\"sectionfooter\">"
            "<td class=\"left"+ QString::number(((*it_group).depth()-1)) + "\" "
            "colspan=\"" +
            QString::number(columns.count()-1-postcolumns.count()) + "\">"+
            i18n("Total")+" " + oldName + "</td>"
            "<td>" + subtotal_html + "</td></tr>\n";

          csv +=
            "\""+ i18n("Total")+ " "+ oldName + "\",\""+ subtotal_csv + "\"\n";
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

    // skip the opening and closing balance row,
    // if the balance column is not shown
    if ((columns.contains("balance") == 0) && ((*it_row)["rank"] == "-2"))
      continue;

    bool need_label = true;

    // ***DV***
    if ((* it_row)["rank"] == "0") row_odd = ! row_odd;

    if ((* it_row)["rank"] == "-2")
      result += QString("<tr class=\"item%1\">").arg((* it_row)["id"]);
    else
    if ((* it_row)["rank"] == "1")
      result += QString("<tr class=\"%1\">").arg(row_odd?"item1":"item0");
    else
      result += QString("<tr class=\"%1\">").arg(row_odd?"row-odd ":"row-even");

    QStringList::const_iterator it_column = columns.begin();
    while ( it_column != columns.end() )
    {
      QString data = (*it_row)[*it_column];

      // ***DV***
      if ((* it_row)["rank"] == "1") {
        if (* it_column == "value")
          data = (* it_row)["split"];
        else if(*it_column == "postdate"
        || *it_column == "number"
        || *it_column == "payee"
        || *it_column == "action"
        || *it_column == "shares"
        || *it_column == "price"
        || *it_column == "balance"
        || *it_column == "account")
          data = "";
      }

      // ***DV***
      if ((* it_row)["rank"] == "-2") {
        if (*it_column == "balance") {
          data = (* it_row)["balance"];
          if((* it_row)["id"] == "A")           // opening balance?
            startingBalance = MyMoneyMoney(data);
        }

        if (need_label) {
          if ((* it_column == "payee") ||
              (* it_column == "category") ||
              (* it_column == "memo")) {
            if ((* it_row)["shares"] != "") {
              data = ((* it_row)["id"] == "A")
                ? i18n("Initial Market Value")
                : i18n("Ending Market Value");
            } else {
              data = ((* it_row)["id"] == "A")
                ? i18n("Opening Balance")
                : i18n("Closing Balance");
            }
            need_label = false;
          }
        }
      }

      // The 'balance' column is calculated at render-time
      // but not printed on split lines
      else if ( *it_column == "balance" && (* it_row)["rank"] == "0")
      {
        // Take the balance off the deepest group iterator
        data = (groupIteratorList.back().currenttotal() + startingBalance).toString();
      }

      // Figure out how to render the value in this column, depending on
      // what its properties are.
      //
      // TODO: This and the i18n headings are handled
      // as a set of parallel vectors.  Would be much better to make a single
      // vector of a properties class.
      if ( sharesColumns.contains(*it_column) )
      {
        if (data.isEmpty()) {
          result += QString("<td></td>");
          csv += "\"\",";
        }
        else {
          result += QString("<td>%1</td>").arg(MyMoneyMoney(data).formatMoney("",3));
          csv += "\"" + MyMoneyMoney(data).formatMoney("", 3, true) + "\",";
        }
      }
      else if ( moneyColumns.contains(*it_column) )
      {
        if (data.isEmpty()) {
          result += QString("<td%1></td>")
            .arg((*it_column == "value") ? " class=\"value\"" : "");
          csv += "\"\",";
        }
        else {
          result += QString("<td%1>%2&nbsp;%3</td>")
            .arg((*it_column == "value") ? " class=\"value\"" : "")
            .arg((*it_row)["currency"])
            .arg(MyMoneyMoney(data).formatMoney());
          csv += "\"" + (*it_row)["currency"] + " " + MyMoneyMoney(data).formatMoney("", 2, true) + "\",";
        }
      }
      else if ( percentColumns.contains(*it_column) )
      {
        data = (MyMoneyMoney(data) * MyMoneyMoney(100,1)).formatMoney();
        result += QString("<td>%1%</td>").arg(data);
        csv += data + "%,";
      }
      else if ( dateColumns.contains(*it_column) )
      {
        // do this before we possibly change data
        csv += "\""+ data + "\",";

        // if we have a locale() then use its date formatter
        if (KGlobal::locale() && ! data.isEmpty()) {
          QDate qd = QDate::fromString(data, Qt::ISODate);
          data = KGlobal::locale()->formatDate(qd, true);
        }
        result += QString("<td class=\"left\">%1</td>").arg(data);
      }
      else
      {
        result += QString("<td class=\"left\">%1</td>").arg(data);
        csv += "\""+ data + "\",";
      }
      ++it_column;
    }

    result += "</tr>\n";
    csv = csv.left( csv.length() - 1 ); // remove final comma
    csv += "\n";
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
        "colspan=\"" + QString::number(columns.count()-1-postcolumns.count()) + "\">"+
        i18n("Total")+" " + (*it_group).oldName() + "</td>"
        "<td>" + subtotal_html + "</td></tr>\n";
      csv += "\"" + i18n("Total") + " " + (*it_group).oldName() + "\",\"" + subtotal_csv + "\"\n";
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
      "colspan=\"" + QString::number(columns.count()-1-postcolumns.count()) + "\">"+
      i18n("Grand Total") + "</td>"
      "<td>" + grandtotal_html + "</td></tr>\n";
    csv += "\"" + i18n("Grand Total") + "\",\"" + grandtotal_csv + "\"\n";
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
