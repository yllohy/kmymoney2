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

#include <kmessagebox.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kledgerview.h"

KLedgerView::KLedgerView(QWidget *parent, const char *name )
  : QWidget(parent,name)
{
  m_editPayee = 0;
  m_register = 0;
  m_form = 0;
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
  // filter all unwanted transactions
  refreshView();

  // add another, empty transaction that will be the new transaction

  // when opened, show the last transaction
  m_register->setCurrentTransactionIndex(m_transactionList.count()-1);

  // make sure, full transaction is visible
  m_register->ensureCellVisible(m_transactionPtr.count() * m_register->rpt(), 0);
  m_register->ensureCellVisible((m_transactionPtr.count()-1) * m_register->rpt(), 0);
  m_register->repaintContents();

  // fill in the form with the currently selected transaction
  fillForm();
}

void KLedgerView::refreshView(void)
{
  KConfig *config = KGlobal::config();
  config->setGroup("List Options");

  QDateTime defaultDate;

  m_dateStart = config->readDateTimeEntry("StartDate", &defaultDate).date();

  filterTransactions();

  m_register->readConfig();
  m_register->setTransactionCount(m_transactionPtr.count()+1);
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

void KLedgerView::slotRegisterClicked(int row, int col, int button, const QPoint &mousePos)
{
  // only redraw the register and form, when a different
  // transaction has been selected with this click.
  if(m_register->setCurrentTransactionIndex(row / m_register->rpt()) == true) {
    m_register->repaintContents();
    fillForm();
  }
}

void KLedgerView::slotShowTransactionForm(bool visible)
{
  if(m_form != 0) {
    if(visible)
      m_form->show();
    else
      m_form->hide();
  }

  if(m_register != 0) {
    m_register->setInlineEditingAvailable(!visible);
  }
}

const QCString KLedgerView::str2action(const QString &action) const
{
  if(action == i18n("Check"))
    return MyMoneySplit::ActionCheck;
  if(action == i18n("Deposit"))
    return MyMoneySplit::ActionDeposit;
  if(action == i18n("Transfer"))
    return MyMoneySplit::ActionTransfer;
  if(action == i18n("Withdrawal"))
    return MyMoneySplit::ActionWithdrawal;
  if(action == i18n("ATM"))
    return MyMoneySplit::ActionATM;

  qDebug("Unsupported action string %s, set to check", action.latin1());
  return MyMoneySplit::ActionCheck;
}

const QString KLedgerView::action2str(const QCString &action) const
{
  if(action == MyMoneySplit::ActionCheck)
    return i18n("Check");
  if(action == MyMoneySplit::ActionDeposit)
    return i18n("Deposit");
  if(action == MyMoneySplit::ActionTransfer)
    return i18n("Transfer");
  if(action == MyMoneySplit::ActionWithdrawal)
    return i18n("Withdrawal");
  if(action == MyMoneySplit::ActionATM)
    return i18n("ATM");

  qDebug("Unsupported action string %s, set to check", static_cast<const char *>(action));
  return i18n("Check");
}

void KLedgerView::slotNewPayee(const QString& payeeName)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  MyMoneyPayee payee;

  // Ask the user if that is what he intended to do?
  QString msg = i18n("Do you want to add '%1' as payee/receiver ?").arg(payeeName);

  if(KMessageBox::questionYesNo(this, msg, i18n("New payee/receiver")) == KMessageBox::Yes) {
    // for now, we just add the payee to the pool. In the future,
    // we could open a dialog and ask for all the other attributes
    // of the payee.
    payee.setName(payeeName);

    try {
      file->addPayee(payee);
      m_editPayee->loadList();

    } catch(MyMoneyException *e) {
      KMessageBox::detailedSorry(0, i18n("Unable to add payee/receiver"),
        e->what());
      delete e;

      m_editPayee->resetText();
    }
  } else
    m_editPayee->resetText();
}
