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


#include "mymoneytransactionfilter.h"

MyMoneyTransactionFilter::MyMoneyTransactionFilter()
{
  m_filterSet.allFilter = 0;
}

MyMoneyTransactionFilter::MyMoneyTransactionFilter(const QCString& id)
{
  m_filterSet.allFilter = 0;
  addAccount(id);
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
}

void MyMoneyTransactionFilter::setTextFilter(const QRegExp& text)
{
  m_filterSet.singleFilter.textFilter = 1;
  m_text = text;
}

void MyMoneyTransactionFilter::addAccount(const QCString& id)
{
  if(!m_accounts.isEmpty()) {
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

void MyMoneyTransactionFilter::addCategory(const QCString& id)
{
  if(!m_categories.isEmpty()) {
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
  m_filterSet.singleFilter.dateFilter = 1;
  m_fromDate = from;
  m_toDate = to;
}

void MyMoneyTransactionFilter::setAmountFilter(const MyMoneyMoney& from, const MyMoneyMoney& to)
{
  m_filterSet.singleFilter.amountFilter = 1;
  m_fromAmount = from;
  m_toAmount = to;
}

void MyMoneyTransactionFilter::addPayee(const QCString& id)
{
  if(!m_payees.isEmpty()) {
    if(m_payees.find(id) != 0)
      return;
  }
  if(m_payees.count() >= m_payees.size()*2) {
    m_payees.resize(457);
  }
  m_filterSet.singleFilter.payeeFilter = 1;
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

void MyMoneyTransactionFilter::setNumberFilter(const QString& from, const QString& to)
{
  m_filterSet.singleFilter.nrFilter = 1;
  m_fromNr = from;
  m_toNr = to;
}

const bool MyMoneyTransactionFilter::match(const MyMoneyTransaction& transaction)
{
  QValueList<MyMoneySplit>::ConstIterator it;
  m_matchingSplit = QCString();
  
  // if no filter is set, we can savely return a match
  if(!m_filterSet.allFilter)
    return true;

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

  if(m_filterSet.singleFilter.accountFilter
  || m_filterSet.singleFilter.textFilter
  || m_filterSet.singleFilter.payeeFilter
  || m_filterSet.singleFilter.typeFilter
  || m_filterSet.singleFilter.stateFilter
  || m_filterSet.singleFilter.nrFilter
  || m_filterSet.singleFilter.amountFilter) {
    // now scan all the splits for matches
    for(it = transaction.splits().begin(); it != transaction.splits().end(); ++it) {

      // check if the split references one of the accounts in the list
      if(m_filterSet.singleFilter.accountFilter) {
        if(m_accounts.count() > 0) {
          if(!m_accounts.find((*it).accountId()))
            continue;
        }
      }

      // check if the text is contained in one of the fields
      if(m_filterSet.singleFilter.textFilter) {

      }

      // check the payee list
      if(m_filterSet.singleFilter.payeeFilter) {
        if(m_payees.count() > 0) {
          if(!m_payees.find((*it).payeeId()))
            continue;
        }
      }

      // check the type list
      if(m_filterSet.singleFilter.typeFilter) {
        if(m_types.count() > 0) {
          if(!m_types.find(splitType(*it)))
            continue;
        }
      }

      // check the state list
      if(m_filterSet.singleFilter.stateFilter) {
        if(m_states.count() > 0) {
          if(!m_states.find(splitState(*it)))
            continue;
        }
      }

      if(m_filterSet.singleFilter.nrFilter) {
        if(!m_fromNr.isEmpty()) {
          if((*it).number() < m_fromNr)
            continue;
        }
        if(!m_toNr.isEmpty()) {
          if((*it).number() > m_toNr)
            continue;
        }
      }

      if(m_filterSet.singleFilter.amountFilter) {
        if((*it).value() < m_fromAmount)
          continue;
        if((*it).value() > m_toAmount)
          continue;
      }
      // all filters passed, this one seems to match
      break;
    }
    if(it == transaction.splits().end())
      return false;
      
    m_matchingSplit = (*it).id();
  }
      
  // all filters passed, I guess we have a match
  return true;
}

const int MyMoneyTransactionFilter::splitState(const MyMoneySplit& split) const
{
  switch(split.reconcileFlag()) {
    case MyMoneySplit::NotReconciled:
      return notReconciled;
      
    case MyMoneySplit::Cleared:
      return cleared;
      
    case MyMoneySplit::Reconciled:
      return reconciled;
      
    case MyMoneySplit::Frozen:
      return frozen;
  }
  return notReconciled;
}

const int MyMoneyTransactionFilter::splitType(const MyMoneySplit& split) const
{
  if(split.action().isEmpty())
    return allTypes;
    
  if(split.action() == MyMoneySplit::ActionDeposit)
    return deposits;
  if(split.action() == MyMoneySplit::ActionTransfer)
    return transfers;
    
  return payments;  
}
