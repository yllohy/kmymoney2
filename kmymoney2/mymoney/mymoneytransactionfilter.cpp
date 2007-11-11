/***************************************************************************
                          mymoneytransactionfilter.cpp  -  description
                             -------------------
    begin                : Fri Aug 22 2003
    copyright            : (C) 2003 by Thomas Baumgart
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
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

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "../mymoney/storage/imymoneystorage.h"

#include "mymoneytransactionfilter.h"

MyMoneyTransactionFilter::MyMoneyTransactionFilter()
{
  m_filterSet.allFilter = 0;
  m_reportAllSplits = true;
  m_considerCategory = true;
}

MyMoneyTransactionFilter::MyMoneyTransactionFilter(const QCString& id)
{
  m_filterSet.allFilter = 0;
  m_reportAllSplits = false;
  m_considerCategory = false;

  addAccount(id);
  // addCategory(id);
}

MyMoneyTransactionFilter::~MyMoneyTransactionFilter()
{
}
void MyMoneyTransactionFilter::clear(void)
{
  m_filterSet.allFilter = 0;
  m_accounts.clear();
  m_categories.clear();
  m_payees.clear();
  m_types.clear();
  m_states.clear();
  m_validity.clear();
  m_matchingSplits.clear();
  m_fromDate = QDate();
  m_toDate = QDate();
}

void MyMoneyTransactionFilter::clearAccountFilter(void)
{
  m_filterSet.singleFilter.accountFilter = 0;
  m_accounts.clear();
}

void MyMoneyTransactionFilter::setTextFilter(const QRegExp& text)
{
  m_filterSet.singleFilter.textFilter = 1;
  m_text = text;
}

void MyMoneyTransactionFilter::addAccount(const QCStringList& ids)
{
  QCStringList::ConstIterator it;

  m_filterSet.singleFilter.accountFilter = 1;
  for(it = ids.begin(); it != ids.end(); ++it)
    addAccount(*it);
}

void MyMoneyTransactionFilter::addAccount(const QCString& id)
{
  if(!m_accounts.isEmpty() && !id.isEmpty()) {
    if(m_accounts.find(id) != 0)
      return;
  }
  if(m_accounts.count() >= m_accounts.size()*2) {
    m_accounts.resize(457);
  }
  m_filterSet.singleFilter.accountFilter = 1;
  if(!id.isEmpty())
    m_accounts.insert(id, "");
}

void MyMoneyTransactionFilter::addCategory(const QCStringList& ids)
{
  QCStringList::ConstIterator it;

  m_filterSet.singleFilter.categoryFilter = 1;
  for(it = ids.begin(); it != ids.end(); ++it)
    addCategory(*it);
}

void MyMoneyTransactionFilter::addCategory(const QCString& id)
{
  if(!m_categories.isEmpty() && !id.isEmpty()) {
    if(m_categories.find(id) != 0)
      return;
  }
  if(m_categories.count() >= m_categories.size()*2) {
    m_categories.resize(457);
  }
  m_filterSet.singleFilter.categoryFilter = 1;
  if(!id.isEmpty())
    m_categories.insert(id, "");
}

void MyMoneyTransactionFilter::setDateFilter(const QDate& from, const QDate& to)
{
  m_filterSet.singleFilter.dateFilter = from.isValid() | to.isValid();
  m_fromDate = from;
  m_toDate = to;
}

void MyMoneyTransactionFilter::setAmountFilter(const MyMoneyMoney& from, const MyMoneyMoney& to)
{
  m_filterSet.singleFilter.amountFilter = 1;
  m_fromAmount = from.abs();
  m_toAmount = to.abs();

  // make sure that the user does not try to fool us  ;-)
  if(from > to) {
    MyMoneyMoney tmp = m_fromAmount;
    m_fromAmount = m_toAmount;
    m_toAmount = tmp;
  }
}

void MyMoneyTransactionFilter::addPayee(const QCString& id)
{
  if(!m_payees.isEmpty() && !id.isEmpty()) {
    if(m_payees.find(id) != 0)
      return;
  }
  if(m_payees.count() >= m_payees.size()*2) {
    m_payees.resize(457);
  }
  m_filterSet.singleFilter.payeeFilter = 1;
  if(!id.isEmpty())
    m_payees.insert(id, "");
}

void MyMoneyTransactionFilter::addType(const int type)
{
  if(!m_types.isEmpty()) {
    if(m_types.find(type) != 0)
      return;
  }
  // we don't have more than 4 or 5 types, so we don't worry about
  // the size of the QIntDict object.
  m_filterSet.singleFilter.typeFilter = 1;
  m_types.insert(type, "");
}

void MyMoneyTransactionFilter::addState(const int state)
{
  if(!m_states.isEmpty()) {
    if(m_states.find(state) != 0)
      return;
  }
  // we don't have more than 4 or 5 states, so we don't worry about
  // the size of the QIntDict object.
  m_filterSet.singleFilter.stateFilter = 1;
  m_states.insert(state, "");
}

void MyMoneyTransactionFilter::addValidity(const int type)
{
  if(!m_validity.isEmpty()) {
    if(m_validity.find(type) != 0)
      return;
  }
  // we don't have more than 4 or 5 states, so we don't worry about
  // the size of the QIntDict object.
  m_filterSet.singleFilter.validityFilter = 1;
  m_validity.insert(type, "");
}

void MyMoneyTransactionFilter::setNumberFilter(const QString& from, const QString& to)
{
  m_filterSet.singleFilter.nrFilter = 1;
  m_fromNr = from;
  m_toNr = to;
}

void MyMoneyTransactionFilter::setReportAllSplits(const bool report)
{
  m_reportAllSplits = report;
}

void MyMoneyTransactionFilter::setConsiderCategory(const bool check)
{
  m_considerCategory = check;
}

const QValueList<MyMoneySplit>& MyMoneyTransactionFilter::matchingSplits(void) const
{
  return m_matchingSplits;
}

bool MyMoneyTransactionFilter::match(const MyMoneySplit * const sp, const IMyMoneyStorage* const storage) const
{
  bool rc = true;

  // check if the text is contained in one of the fields
  // memo, value, number, payee, account, date
  if(m_filterSet.singleFilter.textFilter) {
    bool textMatch = false;
    const MyMoneyAccount& acc = storage->account(sp->accountId());
    if(sp->memo().contains(m_text)
    || sp->value().formatMoney().contains(m_text)
    || sp->number().contains(m_text))
      textMatch = true;

    if(!textMatch && acc.name().contains(m_text))
      textMatch = true;

    if(!textMatch && !sp->payeeId().isEmpty()) {
      const MyMoneyPayee& payee = storage->payee(sp->payeeId());
      if(payee.name().contains(m_text))
        textMatch = true;
    }

    if(!textMatch)
      rc = false;
  }

  if(rc && m_filterSet.singleFilter.amountFilter) {
    if(((sp->value().abs() < m_fromAmount) || sp->value().abs() > m_toAmount)
    && ((sp->shares().abs() < m_fromAmount) || sp->shares().abs() > m_toAmount))
      rc = false;
  }

  return rc;
}

const bool MyMoneyTransactionFilter::match(const MyMoneyTransaction& transaction, const IMyMoneyStorage* const storage)
{
  QValueList<MyMoneySplit>::ConstIterator it;

  m_matchingSplits.clear();

  // qDebug("T: %s", transaction.id().data());
  // if no filter is set, we can savely return a match
  // if we should report all splits, then we collect them
  if(!m_filterSet.allFilter) {
    if(m_reportAllSplits) {
      m_matchingSplits = transaction.splits();
    }
    return true;
  }

  // perform checks on the MyMoneyTransaction object first

  // check the date range
  if(m_filterSet.singleFilter.dateFilter) {
    if(m_fromDate != QDate()) {
      if(transaction.postDate() < m_fromDate)
        return false;
    }

    if(m_toDate != QDate()) {
      if(transaction.postDate() > m_toDate)
        return false;
    }
  }

  // construct a local list of pointers to all splits and
  // remove the ones that do not match account and/or categories.

  QPtrList<MyMoneySplit> matchingSplits;
  for(it = transaction.splits().begin(); it != transaction.splits().end(); ++it) {
    matchingSplits.append(&(*it));
  }

  bool categoryMatched = !m_filterSet.singleFilter.categoryFilter;
  bool accountMatched = !m_filterSet.singleFilter.accountFilter;
  bool isTransfer = true;

  // check the transaction's validity
  if(m_filterSet.singleFilter.validityFilter) {
    if(m_validity.count() > 0) {
      if(!m_validity.find(validTransaction(transaction)))
        return false;
    }
  }

  MyMoneySplit* sp;

  if(m_filterSet.singleFilter.accountFilter == 1
  || m_filterSet.singleFilter.categoryFilter == 1) {
    for(sp = matchingSplits.first(); sp != 0; ) {
      MyMoneySplit* removeSplit = sp;
      const MyMoneyAccount& acc = storage->account(sp->accountId());
      if(m_considerCategory) {
        switch(acc.accountGroup()) {
          case MyMoneyAccount::Income:
          case MyMoneyAccount::Expense:
            isTransfer = false;
            // check if the split references one of the categories in the list
            if(m_filterSet.singleFilter.categoryFilter) {
              if(m_categories.count() > 0) {
                if(m_categories.find(sp->accountId())) {
                  categoryMatched = true;
                  removeSplit = 0;
                }
              } else {
                // we're looking for transactions with 'no' categories
                return false;
              }
            }
            break;

          default:
            // check if the split references one of the accounts in the list
            if(m_filterSet.singleFilter.accountFilter) {
              if(m_accounts.count() > 0) {
                if(m_accounts.find(sp->accountId())) {
                  accountMatched = true;
                  removeSplit = 0;
                }
              }
            } else
              removeSplit = 0;

            break;
        }

      } else {
        if(m_filterSet.singleFilter.accountFilter) {
          if(m_accounts.count() > 0) {
            if(m_accounts.find(sp->accountId())) {
              accountMatched = true;
              removeSplit = 0;
            }
          }
        } else
          removeSplit = 0;
      }

      sp = matchingSplits.next();
      if(removeSplit) {
        // qDebug(" S: %s", (*it).id().data());
        matchingSplits.remove(removeSplit);
      }
    }
  }

  // check if we're looking for transactions without assigned category
  if(!categoryMatched && transaction.splitCount() == 1 && m_categories.count() == 0) {
    categoryMatched = true;
  }

  // if there's no category filter and the category did not
  // match, then we still want to see this transaction if it's
  // a transfer
  if(!categoryMatched && !m_filterSet.singleFilter.categoryFilter)
    categoryMatched = isTransfer;

  if(matchingSplits.count() == 0
  || !(accountMatched && categoryMatched))
    return false;

  FilterSet filterSet = m_filterSet;
  filterSet.singleFilter.dateFilter =
  filterSet.singleFilter.accountFilter =
  filterSet.singleFilter.categoryFilter = 0;

  // check if we still have something to do
  if(filterSet.allFilter != 0) {
    for(sp = matchingSplits.first(); sp != 0;) {
      MyMoneySplit* removeSplit = match(sp, storage) ? 0 : sp;

      const MyMoneyAccount& acc = storage->account(sp->accountId());

      // Determine if this account is a category or an account
      bool isCategory = false;
      switch(acc.accountGroup()) {
        case MyMoneyAccount::Income:
        case MyMoneyAccount::Expense:
          isCategory = true;
        default:
          break;
      }

#if 0
      // TODO code moved to match() and should be removed here
      const MyMoneyAccount& acc = storage->account(sp->accountId());

      // Determine if this account is a category or an account
      bool isCategory = false;
      switch(acc.accountGroup()) {
        case MyMoneyAccount::Income:
        case MyMoneyAccount::Expense:
          isCategory = true;
        default:
          break;
      }

      // check if the text is contained in one of the fields
      // memo, value, number, payee, account, date
      if(m_filterSet.singleFilter.textFilter) {
        bool textMatch = false;
        if(sp->memo().contains(m_text)
        || sp->value().formatMoney().contains(m_text)
        || sp->number().contains(m_text))
          textMatch = true;

        if(!textMatch && acc.name().contains(m_text))
          textMatch = true;

        if(!textMatch && !sp->payeeId().isEmpty()) {
          const MyMoneyPayee& payee = storage->payee(sp->payeeId());
          if(payee.name().contains(m_text))
            textMatch = true;
        }

        if(!textMatch)
          removeSplit = sp;
      }

      if(!removeSplit && m_filterSet.singleFilter.amountFilter) {
        if(sp->value().abs() < m_fromAmount)
          removeSplit = sp;
        if(sp->value().abs() > m_toAmount)
          removeSplit = sp;
      }
#endif

      if(!isCategory && !removeSplit) {
        // check the payee list
        if(!removeSplit && m_filterSet.singleFilter.payeeFilter) {
          if(m_payees.count() > 0) {
            if(sp->payeeId().isEmpty() || !m_payees.find(sp->payeeId()))
              removeSplit = sp;
          } else if(!sp->payeeId().isEmpty())
              removeSplit = sp;
        }

        // check the type list
        if(!removeSplit && m_filterSet.singleFilter.typeFilter) {
          if(m_types.count() > 0) {
            if(!m_types.find(splitType(storage, transaction, *sp)))
              removeSplit = sp;
          }
        }

        // check the state list
        if(!removeSplit && m_filterSet.singleFilter.stateFilter) {
          if(m_states.count() > 0) {
            if(!m_states.find(splitState(*sp)))
              removeSplit = sp;
          }
        }

        if(!removeSplit && m_filterSet.singleFilter.nrFilter) {
          if(!m_fromNr.isEmpty()) {
            if(sp->number() < m_fromNr)
              removeSplit = sp;
          }
          if(!m_toNr.isEmpty()) {
            if(sp->number() > m_toNr)
              removeSplit = sp;
          }
        }
      } else if(m_filterSet.singleFilter.payeeFilter
      || m_filterSet.singleFilter.typeFilter
      || m_filterSet.singleFilter.stateFilter
      || m_filterSet.singleFilter.nrFilter)
        removeSplit = sp;

      sp = matchingSplits.next();
      if(removeSplit) {
        // qDebug(" S: %s", (*it).id().data());
        matchingSplits.remove(removeSplit);
      }
    }
  }

  if(m_reportAllSplits == false && matchingSplits.count() != 0) {
    m_matchingSplits.append(transaction.splits()[0]);
  } else {
    for(sp = matchingSplits.first(); sp != 0; sp = matchingSplits.next()) {
      m_matchingSplits.append(*sp);
    }
  }
  // all filters passed, I guess we have a match
  // qDebug("  C: %d", m_matchingSplits.count());
  return matchingSplits.count() != 0;
}

const int MyMoneyTransactionFilter::splitState(const MyMoneySplit& split) const
{
  int rc = notReconciled;

  switch(split.reconcileFlag()) {
    default:
    case MyMoneySplit::NotReconciled:
      break;;

    case MyMoneySplit::Cleared:
      rc = cleared;
      break;

    case MyMoneySplit::Reconciled:
      rc = reconciled;
      break;

    case MyMoneySplit::Frozen:
      rc = frozen;
      break;
  }
  return rc;
}

const int MyMoneyTransactionFilter::splitType(const IMyMoneyStorage* const storage, const MyMoneyTransaction& t, const MyMoneySplit& split) const
{
  MyMoneyAccount a, b;
  a = storage->account(split.accountId());
  if((a.accountGroup() == MyMoneyAccount::Income
  || a.accountGroup() == MyMoneyAccount::Expense))
    return allTypes;

  if(t.splitCount() == 2) {
    QCString ida, idb;
    ida = t.splits()[0].accountId();
    idb = t.splits()[1].accountId();

    a = storage->account(ida);
    b = storage->account(idb);
    if((a.accountGroup() != MyMoneyAccount::Expense
    && a.accountGroup() != MyMoneyAccount::Income)
    && (b.accountGroup() != MyMoneyAccount::Expense
    && b.accountGroup() != MyMoneyAccount::Income))
      return transfers;
  }

  if(split.value().isPositive())
    return deposits;

  return payments;
}

const MyMoneyTransactionFilter::validityOptionE MyMoneyTransactionFilter::validTransaction(const MyMoneyTransaction& t) const
{
  QValueList<MyMoneySplit>::ConstIterator it_s;
  MyMoneyMoney val;

  for(it_s = t.splits().begin(); it_s != t.splits().end(); ++it_s) {
    val += (*it_s).value();
  }
  return (val == MyMoneyMoney(0,1)) ? valid : invalid;
}

const bool MyMoneyTransactionFilter::includesCategory( const QCString& cat ) const
{
  return (! m_filterSet.singleFilter.categoryFilter) || m_categories.find( cat );
}

const bool MyMoneyTransactionFilter::includesAccount( const QCString& acc ) const
{
  return (! m_filterSet.singleFilter.accountFilter) || m_accounts.find( acc );
}

const bool MyMoneyTransactionFilter::includesPayee( const QCString& pye ) const
{
  return (! m_filterSet.singleFilter.payeeFilter) || m_payees.find( pye );
}

const bool MyMoneyTransactionFilter::dateFilter( QDate& from, QDate& to ) const
{
  from = m_fromDate;
  to = m_toDate;
  return m_filterSet.singleFilter.dateFilter==1;
}

const bool MyMoneyTransactionFilter::amountFilter( MyMoneyMoney& from, MyMoneyMoney& to ) const
{
  from = m_fromAmount;
  to = m_toAmount;
  return m_filterSet.singleFilter.amountFilter==1;
}

const bool MyMoneyTransactionFilter::numberFilter( QString& from, QString& to ) const
{
  from = m_fromNr;
  to = m_toNr;
  return m_filterSet.singleFilter.nrFilter==1;
}

const bool MyMoneyTransactionFilter::payees(QCStringList& list) const
{
  bool result = m_filterSet.singleFilter.payeeFilter;

  if ( result )
  {
    QAsciiDictIterator<char> it_payee( m_payees );
    while ( it_payee.current() )
    {
      list += it_payee.currentKey();
      ++it_payee;
    }
  }
  return result;
}

const bool MyMoneyTransactionFilter::accounts(QCStringList& list) const
{
  bool result = m_filterSet.singleFilter.accountFilter;

  if ( result )
  {
    QAsciiDictIterator<char> it_account( m_accounts );
    while ( it_account.current() )
    {
      QCString account = it_account.currentKey();
      list += account;
      ++it_account;
    }
  }
  return result;
}

const bool MyMoneyTransactionFilter::categories(QCStringList& list) const
{
  bool result = m_filterSet.singleFilter.categoryFilter;

  if ( result )
  {
    QAsciiDictIterator<char> it_category( m_categories );
    while ( it_category.current() )
    {
      list += it_category.currentKey();
      ++it_category;
    }
  }
  return result;
}

const bool MyMoneyTransactionFilter::types(QValueList<int>& list) const
{
  bool result = m_filterSet.singleFilter.typeFilter;

  if ( result )
  {
    QIntDictIterator<char> it_type( m_types );
    while ( it_type.current() )
    {
      list += it_type.currentKey();
      ++it_type;
    }
  }
  return result;
}

const bool MyMoneyTransactionFilter::states(QValueList<int>& list) const
{
  bool result = m_filterSet.singleFilter.stateFilter;

  if ( result )
  {
    QIntDictIterator<char> it_state( m_states );
    while ( it_state.current() )
    {
      list += it_state.currentKey();
      ++it_state;
    }
  }
  return result;
}

const bool MyMoneyTransactionFilter::firstType(int&i) const
{
  bool result = m_filterSet.singleFilter.typeFilter;

  if ( result )
  {
    QIntDictIterator<char> it_type( m_types );
    if ( it_type.current() )
      i = it_type.currentKey();
  }
  return result;
}

const bool MyMoneyTransactionFilter::firstState(int&i) const
{
  bool result = m_filterSet.singleFilter.stateFilter;

  if ( result )
  {
    QIntDictIterator<char> it_state( m_states );
    if ( it_state.current() )
      i = it_state.currentKey();
  }
  return result;
}

const bool MyMoneyTransactionFilter::textFilter(QRegExp& exp) const
{
  exp = m_text;
  return m_filterSet.singleFilter.textFilter == 1;
}

void MyMoneyTransactionFilter::setDateFilter(dateOptionE range)
{
  QDate from, to;
  if ( translateDateRange(range,from,to) )
    setDateFilter(from,to);
}

static int fiscalYearStartMonth = 1;

void MyMoneyTransactionFilter::setStartMonthFiscalYear(int firstMonth)
{
  fiscalYearStartMonth = firstMonth;
}

bool MyMoneyTransactionFilter::translateDateRange(dateOptionE id, QDate& start, QDate& end)
{
  bool rc = true;
  int yr, mon, day;
  yr = QDate::currentDate().year();
  mon = QDate::currentDate().month();
  day = QDate::currentDate().day();
  QDate tmp;

  switch(id) {
    case MyMoneyTransactionFilter::allDates:
      start = QDate();
      end = QDate();
      break;
    case MyMoneyTransactionFilter::untilToday:
      start = QDate();
      end =  QDate::currentDate();
      break;
    case MyMoneyTransactionFilter::currentMonth:
      start = QDate(yr,mon,1);
      end = QDate(yr,mon,1).addMonths(1).addDays(-1);
      break;
    case MyMoneyTransactionFilter::currentYear:
      start = QDate(yr,1,1);
      end = QDate(yr,12,31);
      break;
    case MyMoneyTransactionFilter::monthToDate:
      start = QDate(yr,mon,1);
      end = QDate::currentDate();
      break;
    case MyMoneyTransactionFilter::yearToDate:
      start = QDate(yr,1,1);
      end = QDate::currentDate();
      break;
    case MyMoneyTransactionFilter::yearToMonth:
      start = QDate(yr,1,1);
      end = QDate(yr,mon,1).addDays(-1);
      break;
    case MyMoneyTransactionFilter::lastMonth:
      start = QDate(yr,mon,1).addMonths(-1);
      end = QDate(yr,mon,1).addDays(-1);
      break;
    case MyMoneyTransactionFilter::lastYear:
      start = QDate(yr,1,1).addYears(-1);
      end = QDate(yr,12,31).addYears(-1);
      break;
    case MyMoneyTransactionFilter::last7Days:
      start = QDate::currentDate().addDays(-7);
      end = QDate::currentDate();
      break;
    case MyMoneyTransactionFilter::last30Days:
      start = QDate::currentDate().addDays(-30);
      end = QDate::currentDate();
      break;
    case MyMoneyTransactionFilter::last3Months:
      start = QDate::currentDate().addMonths(-3);
      end = QDate::currentDate();
      break;
    case MyMoneyTransactionFilter::last6Months:
      start = QDate::currentDate().addMonths(-6);
      end = QDate::currentDate();
      break;
    case MyMoneyTransactionFilter::last11Months:
      start = QDate(yr,mon,1).addMonths(-12);
      end = QDate(yr,mon,1).addDays(-1);
      break;
    case MyMoneyTransactionFilter::last12Months:
      start = QDate::currentDate().addMonths(-12);
      end = QDate::currentDate();
      break;
    case MyMoneyTransactionFilter::next7Days:
      start = QDate::currentDate();
      end = QDate::currentDate().addDays(7);
      break;
    case MyMoneyTransactionFilter::next30Days:
      start = QDate::currentDate();
      end = QDate::currentDate().addDays(30);
      break;
    case MyMoneyTransactionFilter::next3Months:
      start = QDate::currentDate();
      end = QDate::currentDate().addMonths(3);
      break;
    case MyMoneyTransactionFilter::next6Months:
      start = QDate::currentDate();
      end = QDate::currentDate().addMonths(6);
      break;
    case MyMoneyTransactionFilter::next12Months:
      start = QDate::currentDate();
      end = QDate::currentDate().addMonths(12);
      break;
    case MyMoneyTransactionFilter::userDefined:
      start = QDate();
      end = QDate();
      break;
    case MyMoneyTransactionFilter::last3ToNext3Months:
      start = QDate::currentDate().addMonths(-3);
      end = QDate::currentDate().addMonths(3);
      break;
    case MyMoneyTransactionFilter::currentQuarter:
      start = QDate(yr, mon - ((mon-1) % 3), 1);
      end = start.addMonths(3).addDays(-1);
      break;
    case MyMoneyTransactionFilter::lastQuarter:
      start = QDate(yr, mon - ((mon-1) % 3), 1).addMonths(-3);
      end = start.addMonths(3).addDays(-1);
      break;
    case MyMoneyTransactionFilter::nextQuarter:
      start = QDate(yr, mon - ((mon-1) % 3), 1).addMonths(3);
      end = start.addMonths(3).addDays(-1);
      break;
    case MyMoneyTransactionFilter::currentFiscalYear:
      start = QDate(QDate::currentDate().year(), fiscalYearStartMonth, 1);
      if(QDate::currentDate() < start)
        start = start.addYears(-1);
      end = start.addYears(1).addDays(-1);
      break;
    case MyMoneyTransactionFilter::lastFiscalYear:
      start = QDate(QDate::currentDate().year(), fiscalYearStartMonth, 1);
      if(QDate::currentDate() < start)
       start = start.addYears(-1);
      start = start.addYears(-1);
      end = start.addYears(1).addDays(-1);
      break;
    default:
      qFatal("Unknown date identifier %d in MyMoneyTransactionFilter::translateDateRange()", id);
      rc = false;
      break;
  }
  return rc;
}

void MyMoneyTransactionFilter::removeReference(const QCString& id)
{
  if(m_accounts.find(id)) {
    qDebug(QString("Remove account '%1' from report").arg(id));
    m_accounts.remove(id);
  } else if(m_categories.find(id)) {
    qDebug(QString("Remove category '%1' from report").arg(id));
    m_categories.remove(id);
  } else if(m_payees.find(id)) {
    m_payees.remove(id);
  }
}


// vim:cin:si:ai:et:ts=2:sw=2:
