/***************************************************************************
                          kledgerview.cpp  -  description
                             -------------------
    begin                : Sun Jul 14 2002
    copyright            : (C) 2000-2002 by Michael Edwardes
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

#include <qdatetime.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kledgerview.h"

KLedgerView::KLedgerView(QWidget *parent, const char *name )
  : QWidget(parent,name)
{
}

KLedgerView::~KLedgerView()
{
  MyMoneyFile::instance()->detach(m_account.id(), this);
}

int KTransactionPtrVector::compareItems(KTransactionPtrVector::Item d1, KTransactionPtrVector::Item d2)
{
  MyMoneyTransaction* t1 = static_cast<MyMoneyTransaction*>(d1);
  MyMoneyTransaction* t2 = static_cast<MyMoneyTransaction*>(d2);
  int   rc;

  switch(m_sortType) {
    case SortEntryDate:
      rc = t2->entryDate().daysTo(t1->entryDate());
      break;

    case SortPostDate:
    // tricky fall through here!
    default:
      rc = t2->postDate().daysTo(t1->postDate());
      break;
  }
  return rc;
}

void KLedgerView::setCurrentAccount(const QCString& accountId)
{
  if(accountId != m_account.id()) {
    MyMoneyFile* file = MyMoneyFile::instance();

    file->detach(m_account.id(), this);

    try {
      m_account = file->account(accountId);
      file->attach(m_account.id(), this);
      update(accountId);
    } catch(MyMoneyException *e) {
      qDebug("Unexpected exception in KLedgerView::setCurrentAccount");
      delete e;
    }
  } else
    refreshView();
}

void KLedgerView::loadAccount(void)
{
  m_transactionList = MyMoneyFile::instance()->transactionList(m_account.id());
  refreshView();
}

void KLedgerView::refreshView(void)
{
  KConfig *config = KGlobal::config();
  config->setGroup("List Options");

  QDateTime defaultDate;

  m_dateStart = config->readDateTimeEntry("StartDate", &defaultDate).date();

  filterTransactions();

  m_register->readConfig();
  m_register->setTransactionCount(m_transactionPtr.count());
  m_register->repaintContents();
  resizeEvent(NULL);
}

void KLedgerView::filterTransactions(void)
{
  int   i;

  QValueList<MyMoneyTransaction>::ConstIterator it_t;

  // setup the pointer vector
  m_transactionPtr.clear();
  m_transactionPtr.resize(m_transactionList.size());
  for(i = 0, it_t = m_transactionList.begin(); it_t != m_transactionList.end(); ++it_t) {
    // only show those transactions, that are posted after the configured start date
    if((*it_t).postDate() < m_dateStart)
      continue;

    // add more filters before this line ;-)

    // Wow, we made it through all the filters. Guess we have to show this one
    m_transactionPtr.insert(i, &(*it_t));
    ++i;
  }
  m_transactionPtr.resize(i);

  // sort the transactions
  m_transactionPtr.sort();

  // calculate the balance for each item
  MyMoneyMoney balance(0);
  m_balance.resize(i, balance);

  balance = MyMoneyFile::instance()->balance(accountId());
  // the trick is to go backwards ;-)
  while(--i >= 0) {
    m_balance[i] = balance;
    balance -= m_transactionPtr[i]->split(accountId()).value();
  }
}

void KLedgerView::update(const QCString& accountId)
{
  try {
    loadAccount();
  } catch(MyMoneyException *e) {
    qDebug("Unexpected exception in KLedgerView::update");
    delete e;
  }
}

MyMoneyTransaction* const KLedgerView::transaction(const int idx) const
{
  if(idx >= 0 && idx < m_transactionPtr.count())
    return m_transactionPtr[idx];
  return 0;
}

const MyMoneyMoney& KLedgerView::balance(const int idx) const
{
  static MyMoneyMoney null(0);

  if(idx >= 0 && idx < m_balance.size())
    return m_balance[idx];
  return null;
}