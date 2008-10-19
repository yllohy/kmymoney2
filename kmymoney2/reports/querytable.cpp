/***************************************************************************
                          querytable.cpp
                         -------------------
    begin                : Fri Jul 23 2004
    copyright            : (C) 2004-2005 by Ace Jones
                           (C) 2007 Sascha Pfau
    email                :  acejones@users.sourceforge.net
                            MrPeacock@gmail.com
 ***************************************************************************/

/****************************************************************************
  Contains code from the func_xirr and related methods of financial.cpp
  - KOffice 1.6 by Sascha Pfau.  Sascha agreed to relicense those methods under
  GPLv2 or later.
*****************************************************************************/

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

double CashFlowList::calculateXIRR ( void ) const
{
  double resultRate = 0.00001;

  double resultZero = 0.00000;
  //if ( args.count() > 2 )
  //  resultRate = calc->conv()->asFloat ( args[2] ).asFloat();

// check pairs and count >= 2 and guess > -1.0
  //if ( args[0].count() != args[1].count() || args[1].count() < 2 || resultRate <= -1.0 )
  //  return Value::errorVALUE();

// define max epsilon
  static const double maxEpsilon = 1e-5;

// max number of iterations
  static const int maxIter = 50;

// Newton's method - try to find a res, with a accuracy of maxEpsilon
  double rateEpsilon, newRate, resultValue;
  int i = 0;
  bool contLoop;

  do
  {
    resultValue = xirrResult ( resultRate );

    double resultDerive = xirrResultDerive ( resultRate );

    //check what happens if xirrResultDerive is zero
    //Don't know if it is correct to dismiss the result
    if( resultDerive != 0 ) {
      newRate =  resultRate - resultValue / resultDerive;
    } else {

      newRate =  resultRate - resultValue;
    }

    rateEpsilon = fabs ( newRate - resultRate );

    resultRate = newRate;
    contLoop = ( rateEpsilon > maxEpsilon ) && ( fabs ( resultValue ) > maxEpsilon );
  }
  while ( contLoop && ( ++i < maxIter ) );

  if ( contLoop )
    return resultZero;

  return resultRate;
}

double CashFlowList::xirrResult ( double& rate ) const
{
  QDate date;

  double r = rate + 1.0;
  double res = 0.00000;//back().value().toDouble();

  QValueList<CashFlowListItem>::const_iterator list_it = begin();
  while( list_it != end() ) {
    double e_i = ( (* list_it).today().daysTo ( (* list_it).date() ) ) / 365.0;
    MyMoneyMoney val = (* list_it).value();

    res += val.toDouble() / pow ( r, e_i );
    ++list_it;
  }

  return res;
}


double CashFlowList::xirrResultDerive ( double& rate ) const
{
  QDate date;

  double r = rate + 1.0;
  double res = 0.00000;

  QValueList<CashFlowListItem>::const_iterator list_it = begin();
  while( list_it != end() ) {
    double e_i = ( (* list_it).today().daysTo ( (* list_it).date() ) ) / 365.0;
    MyMoneyMoney val = (* list_it).value();

    res -= e_i * val.toDouble() / pow ( r, e_i + 1.0 );
    ++list_it;
  }

  return res;
}

double CashFlowList::IRR( void ) const
{
  double result = 0.0;

  // set 'today', which is the most recent of all dates in the list
  CashFlowListItem::setToday( mostRecent().date() );

  result = calculateXIRR();
  return result;

  /*
  double lobound = 0.0; // this is as low as we support
  double hibound = 1.00;
  double precision = 0.00001; // how precise do we want the answer?

  // first, see if one of the bounds themselves is the final answer
  MyMoneyMoney hinpv = NPV( hibound );
  MyMoneyMoney lonpv = NPV( lobound );

  if ( lonpv.isZero() )
  {
    result = lobound;
    return result;
  }
  else if ( hinpv.isZero() )
  {
    result = hibound;
    return result;
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
      return result;

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
  return result;
  */
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
// QueryTable implementation
//
// ****************************************************************************

/**
  * TODO
  *
  * - Collapse 2- & 3- groups when they are identical
  * - Way more test cases (especially splits & transfers)
  * - Option to collapse splits
  * - Option to exclude transfers
  *
  */

QueryTable::QueryTable(const MyMoneyReport& _report): ListTable(_report)
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
  case MyMoneyReport::eAccountReconcile:
    m_group = "account,reconcileflag";
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
    m_columns += ",startingbal,buys,sells,reinvestincome,cashincome,return,returninvestment";
  if ( qc & MyMoneyReport::eQCloan )
  {
    m_columns += ",payment,interest,fees";
    m_postcolumns = "balance";
  }
  if ( qc & MyMoneyReport::eQCbalance)
    m_postcolumns = "balance";

  TableRow::setSortCriteria(sort);
  qHeapSort(m_rows);
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
  case MyMoneyReport::ePayee:
    use_summary = false;
    use_transfers = false;
    hide_details = (m_config.detailLevel() == MyMoneyReport::eDetailNone);
    break;
  default:
    use_summary = true;
    use_transfers = true;
    hide_details = (m_config.detailLevel() == MyMoneyReport::eDetailNone);
    break;
  }

  // support for opening and closing balances
  QMap<QString, MyMoneyAccount> accts;

  //get all transactions for this report
  QValueList<MyMoneyTransaction> transactions = file->transactionList(report);
  for (QValueList<MyMoneyTransaction>::const_iterator it_transaction = transactions.begin(); it_transaction != transactions.end(); ++it_transaction) {

    TableRow qA, qS;
    QDate pd;

    qA["id"] = qS["id"] = (* it_transaction).id();
    qA["entrydate"] = qS["entrydate"] = (* it_transaction).entryDate().toString(Qt::ISODate);
    qA["postdate"] = qS["postdate"] = (* it_transaction).postDate().toString(Qt::ISODate);
    qA["commodity"] = qS["commodity"] = (* it_transaction).commodity();

    pd = (* it_transaction).postDate();
    qA["month"] = qS["month"] = i18n("Month of %1").arg(QDate(pd.year(),pd.month(),1).toString(Qt::ISODate));
    qA["week"] = qS["week"] = i18n("Week of %1").arg(pd.addDays(1-pd.dayOfWeek()).toString(Qt::ISODate));

    qA["currency"] = qS["currency"] = "";

    if((* it_transaction).commodity() != file->baseCurrency().id()) {
      if (!report.isConvertCurrency()) {
        qA["currency"] = qS["currency"] = (*it_transaction).commodity();
      }
    }

    // to handle splits, we decide on which account to base the split
    // (a reference point or point of view so to speak). here we take the
    // first account that is a stock account or loan account (or the first account
    // that is not an income or expense account if there is no stock or loan account)
    // to be the account (qA) that will have the sub-item "split" entries. we add
    // one transaction entry (qS) for each subsequent entry in the split.

    const QValueList<MyMoneySplit>& splits = (*it_transaction).splits();
    QValueList<MyMoneySplit>::const_iterator myBegin, it_split;
    //S_end = splits.end();

    for (it_split = splits.begin(), myBegin = splits.end(); it_split != splits.end(); ++it_split) {
      ReportAccount splitAcc = (* it_split).accountId();
      // always put split with a "stock" account if it exists
      if (splitAcc.isInvest())
        break;

      // prefer to put splits with a "loan" account if it exists
      if(splitAcc.isLoan())
        myBegin = it_split;

      if((myBegin == splits.end()) && ! splitAcc.isIncomeExpense()) {
        myBegin = it_split;
      }
    }

    // select our "reference" split
    if (it_split == splits.end()) {
      it_split = myBegin;
    } else {
      myBegin = it_split;
    }

    // if the split is still unknown, use the first one. I have seen this
    // happen with a transaction that has only a single split referencing an income or expense
    // account and has an amount and value of 0. Such a transaction will fall through
    // the above logic and leave 'is' pointing to S_end which causes the remainder
    // of this to end in an infinite loop.
    if(it_split == splits.end()) {
      it_split = splits.begin();
    }

    // for "loan" reports, the loan transaction gets special treatment.
    // the splits of a loan transaction are placed on one line in the
    // reference (loan) account (qA). however, we process the matching
    // split entries (qS) normally.

    bool loan_special_case = false;
    if(m_config.queryColumns() & MyMoneyReport::eQCloan) {
      ReportAccount splitAcc = (*it_split).accountId();
      loan_special_case = splitAcc.isLoan();
    }

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
    bool transaction_text = false; //indicates whether a text should be considered as a match for the transaction or for a split only
    QString a_fullname = "";
    QString a_memo = "";
    unsigned int pass = 1;

    do {
      MyMoneyMoney xr;
      ReportAccount splitAcc = (* it_split).accountId();

      //get fraction for account
      int fraction = splitAcc.fraction();

      //use base currency fraction if not initialized
      if(fraction == -1)
        fraction = file->baseCurrency().smallestAccountFraction();

      QCString institution = splitAcc.institutionId();
      QCString payee = (*it_split).payeeId();

      if ( m_config.isConvertCurrency() ) {
        xr = (splitAcc.deepCurrencyPrice((*it_transaction).postDate()) * splitAcc.baseCurrencyPrice((*it_transaction).postDate())).reduce();
      } else {
        xr = splitAcc.deepCurrencyPrice((*it_transaction).postDate()).reduce();
      }

      //there is a bug where the price sometimes returns 1
      //get the price from the split in that case
      /*if(m_config.isConvertCurrency() && xr == MyMoneyMoney(1,1)) {
        xr = (*it_split).price();
      }*/

      if (splitAcc.isInvest()) {

        // use the institution of the parent for stock accounts
        institution = splitAcc.parent().institutionId();
        MyMoneyMoney shares = (*it_split).shares();

        qA["action"] = (*it_split).action();
        qA["shares"] = shares.isZero() ? "" : (*it_split).shares().toString();
        qA["price"] = shares.isZero() ? "" : xr.toString();

        if (((*it_split).action() == MyMoneySplit::ActionBuyShares) && (*it_split).shares().isNegative())
          qA["action"] = "Sell";

        qA["investaccount"] = splitAcc.parent().name();
      }

      if (it_split == myBegin) {

        include_me = m_config.includes(splitAcc);
        a_fullname = splitAcc.fullName();
        a_memo = (*it_split).memo();

        transaction_text = m_config.match(&(*it_split));

        qA["price"] = xr.toString();
        qA["account"] = splitAcc.name();
        qA["accountid"] = splitAcc.id();
        qA["topaccount"] = splitAcc.topParentName();

        qA["institution"] = institution.isEmpty()
          ? i18n("No Institution")
          : file->institution(institution).name();

        qA["payee"] = payee.isEmpty()
          ? i18n("[Empty Payee]")
          : file->payee(payee).name().simplifyWhiteSpace();

        qA["reconciledate"] = (*it_split).reconcileDate().toString(Qt::ISODate);
        qA["reconcileflag"] = KMyMoneyUtils::reconcileStateToString((*it_split).reconcileFlag(), true );
        qA["number"] = (*it_split).number();

        qA["memo"] = a_memo;

        qS["reconciledate"] = qA["reconciledate"];
        qS["reconcileflag"] = qA["reconcileflag"];
        qS["number"] = qA["number"];

        qS["topcategory"] = splitAcc.topParentName();
        qS["categorytype"] = i18n("Transfer");

        // only include the configured accounts
        if (include_me) {

          if (loan_special_case) {

            // put the principal amount in the "value" column and convert to lowest fraction
            qA["value"] = ((-(*it_split).shares()) * xr).convert(fraction).toString();

            qA["rank"] = "0";
            qA["split"] = "";

          }
          else {

            if ((splits.count() > 2) && use_summary) {

              // add the "summarized" split transaction
              // this is the sub-total of the split detail
              // convert to lowest fraction
              qA["value"] = ((*it_split).shares() * xr).convert(fraction).toString();
              qA["rank"] = "0";
              qA["category"] = i18n("[Split Transaction]");
              qA["topcategory"] = i18n("Split");
              qA["categorytype"] = i18n("Split");

              m_rows += qA;
            }
          }

          // track accts that will need opening and closing balances
          accts.insert (splitAcc.id(), splitAcc);
        }

      } else {

        if (include_me) {

          if (loan_special_case) {
            MyMoneyMoney value = ((-(* it_split).shares()) * xr).convert(fraction);

            if ((*it_split).action() == MyMoneySplit::ActionAmortization) {
              // put the payment in the "payment" column and convert to lowest fraction
              qA["payment"] = value.toString();
            }
            else if ((*it_split).action() == MyMoneySplit::ActionInterest) {
              // put the interest in the "interest" column and convert to lowest fraction
              qA["interest"] = value.toString();
            }
            else if (splits.count() > 2) {
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
              qA["fees"] = (n0 + value).toString();
            }
            // we don't add qA here for a loan transaction. we'll add one
            // qA afer all of the split components have been processed.
            // (see below)

          }

          //--- special case to hide split transaction details
          else if (hide_details && (splits.count() > 2)) {
            // essentially, don't add any qA entries
          }

          //--- default case includes all transaction details
          else {

            if ((splits.count() > 2) && use_summary) {
              qA["value"] = "";

              //convert to lowest fraction
              qA["split"] = ((-(*it_split).shares()) * xr).convert(fraction).toString();
              qA["rank"] = "1";
            }
            else {
              qA["split"] = "";

              //multiply by currency and convert to lowest fraction
              qA["value"] = ((-(*it_split).shares()) * xr).convert(fraction).toString();
              qA["rank"] = "0";
            }

            qA ["memo"] = (*it_split).memo();

            if (! splitAcc.isIncomeExpense()) {
              qA["category"] = ((*it_split).shares().isNegative()) ? 
                  i18n("Transfer from %1").arg(splitAcc.fullName())
                : i18n("Transfer to %1").arg(splitAcc.fullName());
              qA["topcategory"] = splitAcc.topParentName();
              qA["categorytype"] = i18n("Transfer");
            }
            else {
              qA ["category"] = splitAcc.fullName();
              qA ["topcategory"] = splitAcc.topParentName();
              qA ["categorytype"] = KMyMoneyUtils::accountTypeToString(splitAcc.accountGroup());
            }

            if (use_transfers || (splitAcc.isIncomeExpense() && m_config.includes(splitAcc)))
            {
              if(qA["rank"] == "1"
                 && !transaction_text ) {
                if( m_config.match( &(*it_split) )  ) {
                  m_rows += qA;
                }
              } else {
                m_rows += qA;
              }
            }
          }
        }

        if (m_config.includes(splitAcc) && use_transfers) {
          if (! splitAcc.isIncomeExpense()) {

            //multiply by currency and convert to lowest fraction
            qS["value"] = ((*it_split).shares() * xr).convert(fraction).toString();

            qS["rank"] = "0";

            qS["account"] = splitAcc.name();
            qS["accountid"] = splitAcc.id();
            qS["topaccount"] = splitAcc.topParentName();

            qS["category"] = ((*it_split).shares().isNegative())
              ? i18n("Transfer to %1").arg(a_fullname)
              : i18n("Transfer from %1").arg(a_fullname);

            qS["institution"] = institution.isEmpty()
              ? i18n("No Institution")
              : file->institution(institution).name();

            qS["memo"] = (*it_split).memo().isEmpty()
              ? a_memo
              : (*it_split).memo();

            qS["payee"] = payee.isEmpty()
              ? qA["payee"]
              : file->payee(payee).name().simplifyWhiteSpace();

            //check the specific split against the filter for text and amount
            //TODO this should be done at the engine, but I have no clear idea how -- asoliverez
            if(m_config.match( &(*it_split) )
              || transaction_text )
              m_rows += qS;

            // track accts that will need opening and closing balances
            accts.insert (splitAcc.id(), splitAcc);
          }
        }
      }

      ++it_split;

      // look for wrap-around
      if (it_split == splits.end())
        it_split = splits.begin();

      // but terminate if this transaction has only a single split
      if(splits.count() < 2)
        break;

      //check if there have been more passes than there are splits
      //this is to prevent infinite loops in cases of data inconsistency -- asoliverez
      ++pass;
      if( pass > splits.count() )
        break;

    } while (it_split != myBegin);

    if (loan_special_case) {
      m_rows += qA;
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

  QDate startDate, endDate;

  report.validDateRange(startDate, endDate);
  QString strStartDate = startDate.toString(Qt::ISODate);
  QString strEndDate = endDate.toString(Qt::ISODate);
  startDate = startDate.addDays(-1);

  QMap<QString, MyMoneyAccount>::const_iterator it_account, accts_end;
  for (it_account = accts.begin(); it_account != accts.end(); ++it_account) {

    TableRow qA;

    ReportAccount account = (* it_account);

    //get fraction for account
    int fraction = account.fraction();

    //use base currency fraction if not initialized
    if(fraction == -1)
      fraction = file->baseCurrency().smallestAccountFraction();
    QCString institution = account.institutionId();

    // use the institution of the parent for stock accounts
    if (account.isInvest())
      institution = account.parent().institutionId();

    MyMoneyMoney startBalance, endBalance, startPrice, endPrice;
    MyMoneyMoney startShares, endShares;

    //get price and convert currency if necessary
    if ( m_config.isConvertCurrency() ) {
      startPrice = (account.deepCurrencyPrice(startDate) * account.baseCurrencyPrice(startDate)).reduce();
      endPrice = (account.deepCurrencyPrice(endDate) * account.baseCurrencyPrice(endDate)).reduce();
    } else {
      startPrice = account.deepCurrencyPrice(startDate).reduce();
      endPrice = account.deepCurrencyPrice(endDate).reduce();
    }
    startShares = file->balance(account.id(),startDate);
    endShares = file->balance(account.id(),endDate);

    //get starting and ending balances
    startBalance = startShares * startPrice;
    endBalance = endShares * endPrice;

    //starting balance
    // don't show currency if we're converting or if it's not foreign
    qA["currency"] = (m_config.isConvertCurrency() || ! account.isForeignCurrency()) ? "" : account.currency().id();

    qA["accountid"] = account.id();
    qA["account"] = account.name();
    qA["topaccount"] = account.topParentName();
    qA["institution"] = institution.isEmpty() ? i18n("No Institution") : file->institution(institution).name();
    qA["rank"] = "-2";

    qA["price"] = startPrice.toString();
    if (account.isInvest()) {
      qA["shares"] = startShares.toString();
    }

    qA["postdate"] = strStartDate;
    qA["balance"] = startBalance.convert(fraction).toString();
    qA["value"] = QString();
    qA["id"] = "A";
    m_rows += qA;

    //ending balance
    qA["price"] = endPrice.toString();

    if (account.isInvest()) {
      qA["shares"] = endShares.toString();
    }

    qA["postdate"] = strEndDate;
    qA["balance"] = endBalance.toString();
    qA["id"] = "Z";
    m_rows += qA;
  }
}

void QueryTable::constructPerformanceRow( const ReportAccount& account, TableRow& result ) const
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

  //calculate starting balance
  if ( m_config.isConvertCurrency() ) {
    price = account.deepCurrencyPrice(startingDate) * account.baseCurrencyPrice(startingDate);
  } else {
    price = account.deepCurrencyPrice(startingDate);
  }

  //work around if there is no price for the starting balance
  if(!(file->balance(account.id(),startingDate)).isZero()
     && account.deepCurrencyPrice(startingDate) == MyMoneyMoney(1, 1))
  {
    MyMoneyTransactionFilter filter;
    //get the transactions for the time before the report
    filter.setDateFilter(QDate(), startingDate);
    filter.addAccount(account.id());
    filter.setReportAllSplits(true);

    QValueList<MyMoneyTransaction> startTransactions = file->transactionList(filter);
    if(startTransactions.size() > 0)
    {
      //get the last transaction
      MyMoneyTransaction startTrans = startTransactions.back();
      MyMoneySplit s = startTrans.splitByAccount(account.id());
      //get the price from the split of that account
      price = s.price();
      if ( m_config.isConvertCurrency() )
        price = price * account.baseCurrencyPrice(startingDate);
    }
  }if ( m_config.isConvertCurrency() ) {
    price = account.deepCurrencyPrice(endingDate) * account.baseCurrencyPrice(endingDate);
  } else {
    price = account.deepCurrencyPrice(endingDate);
  }


  MyMoneyMoney startingBal = file->balance(account.id(),startingDate) * price;

  //convert to lowest fraction
  startingBal = startingBal.convert(account.fraction());

  //calculate ending balance
  if ( m_config.isConvertCurrency() ) {
    price = account.deepCurrencyPrice(endingDate) * account.baseCurrencyPrice(endingDate);
  } else {
    price = account.deepCurrencyPrice(endingDate);
  }
  MyMoneyMoney endingBal = file->balance((account).id(),endingDate) * price;

  //convert to lowest fraction
  endingBal = endingBal.convert(account.fraction());

  //add start balance to calculate return on investment
  MyMoneyMoney returnInvestment = startingBal;
  MyMoneyMoney paidDividend;
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

    //get price for the day of the transaction if we have to calculate base currency
    //we are using the value of the split which is in deep currency
    if ( m_config.isConvertCurrency() ) {
      price = account.baseCurrencyPrice(endingDate); //we only need base currency because the value is in deep currency
    } else {
      price = MyMoneyMoney(1,1);
    }

    MyMoneyMoney value = s.value() * price;

    const QCString& action = s.action();
    if ( action == MyMoneySplit::ActionBuyShares )
    {
      if ( s.value().isPositive() ) {
        buys += CashFlowListItem( (*it_transaction).postDate(), -value );
      } else {
        sells += CashFlowListItem( (*it_transaction).postDate(), -value );
      }
      returnInvestment += value;
        //convert to lowest fraction
      returnInvestment = returnInvestment.convert(account.fraction());
    } else if ( action == MyMoneySplit::ActionReinvestDividend ) {
      reinvestincome += CashFlowListItem( (*it_transaction).postDate(), value );
    } else if ( action == MyMoneySplit::ActionDividend || action == MyMoneySplit::ActionYield ) {
      // find the split with the category, which has the actual amount of the dividend
      QValueList<MyMoneySplit> splits = (*it_transaction).splits();
      QValueList<MyMoneySplit>::const_iterator it_split = splits.begin();
      bool found = false;
      while( it_split != splits.end() ) {
        ReportAccount acc = (*it_split).accountId();
        if ( acc.isIncomeExpense() ) {
          found = true;
          break;
        }
        ++it_split;
      }

      if ( found ) {
        cashincome += CashFlowListItem( (*it_transaction).postDate(), -(*it_split).value() * price);
        paidDividend += ((-(*it_split).value()) * price).convert(account.fraction());
      }
    } else {
      //if the split does not match any action above, add it as buy or sell depending on sign

      //if value is zero, get the price for that date
      if( s.value().isZero() ) {
        if ( m_config.isConvertCurrency() ) {
          price = account.deepCurrencyPrice((*it_transaction).postDate()) * account.baseCurrencyPrice((*it_transaction).postDate());
        } else {
          price = account.deepCurrencyPrice((*it_transaction).postDate());
        }
        value = s.shares() * price;
        if ( s.shares().isPositive() ) {
          buys += CashFlowListItem( (*it_transaction).postDate(), -value );
        } else {
          sells += CashFlowListItem( (*it_transaction).postDate(), -value );
        }
        returnInvestment += value;
      } else {
        value = s.value() * price;
        if ( s.value().isPositive() ) {
          buys += CashFlowListItem( (*it_transaction).postDate(), -value );
        } else {
          sells += CashFlowListItem( (*it_transaction).postDate(), -value );
        }
        returnInvestment += value;
      }
    }
    ++it_transaction;
  }

  // Note that reinvested dividends are not included , because these do not
  // represent a cash flow event.
  CashFlowList all;
  all += buys;
  all += sells;
  all += cashincome;
  all += CashFlowListItem(startingDate, -startingBal);
  all += CashFlowListItem(endingDate, endingBal);

  //check if no activity on that term
  if(!returnInvestment.isZero() && !endingBal.isZero()) {
    returnInvestment = ((endingBal + paidDividend) - returnInvestment)/returnInvestment;
    returnInvestment = returnInvestment.convert(10000);
  } else {
    returnInvestment = MyMoneyMoney(0,1);
  }

  try
  {
    MyMoneyMoney annualReturn = MyMoneyMoney(all.IRR(),10000);
    result["return"] = annualReturn.toString();
    result["returninvestment"] = returnInvestment.toString();
  }
  catch (QString e)
  {
    kdDebug(2) << e << endl;
  }

  result["buys"] = (-(buys.total())).toString();
  result["sells"] = (-(sells.total())).toString();
  result["cashincome"] = (cashincome.total()).toString();
  result["reinvestincome"] = (reinvestincome.total()).toString();
  result["startingbal"] = (startingBal).toString();
  result["endingbal"] = (endingBal).toString();
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

    //get fraction for account
    int fraction = account.fraction();

      //use base currency fraction if not initialized
    if(fraction == -1)
      fraction = MyMoneyFile::instance()->baseCurrency().smallestAccountFraction();

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
        qaccountrow["currency"] = account.currency().id();
      }

      qaccountrow["account"] = account.name();
      qaccountrow["accountid"] = account.id();
      qaccountrow["topaccount"] = account.topParentName();

      MyMoneyMoney shares = file->balance(account.id(),m_config.toDate());
      qaccountrow["shares"] = shares.toString();

      MyMoneyMoney netprice = account.deepCurrencyPrice(m_config.toDate()).reduce() * displayprice;
      qaccountrow["price"] = ( netprice.reduce() ).convert(fraction).toString();
      qaccountrow["value"] = ( netprice.reduce() * shares.reduce() ).convert(fraction).toString();

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
        constructPerformanceRow( account, qaccountrow );
      }
      else
        qaccountrow["equitytype"] = QString();

      // don't add the account if it is closed. In fact, the business logic
      // should prevent that an account can be closed with a balance not equal
      // to zero, but we never know.
      if(!(shares.isZero() && account.isClosed()))
        m_rows += qaccountrow;
    }

    ++it_account;
  }
}

}
// vim:cin:si:ai:et:ts=2:sw=2:
