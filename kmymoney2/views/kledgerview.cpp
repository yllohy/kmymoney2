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
#include <kpushbutton.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kledgerview.h"
#include "../widgets/kmymoneyregister.h"
#include "../widgets/kmymoneytransactionform.h"
#include "../widgets/kmymoneypayee.h"
#include "../widgets/kmymoneycategory.h"
#include "../widgets/kmymoneyedit.h"
#include "../widgets/kmymoneylineedit.h"
#include "../widgets/kmymoneydateinput.h"

KLedgerView::KLedgerView(QWidget *parent, const char *name )
  : QWidget(parent,name)
{
  m_editPayee = 0;
  m_register = 0;
  m_form = 0;
  m_transactionPtr = 0;
  m_timer = 0;
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
  slotCancelEdit();

  if(accountId != m_account.id()) {

    MyMoneyFile* file = MyMoneyFile::instance();

    file->detach(m_account.id(), this);

    try {
      m_account = file->account(accountId);
      file->attach(m_account.id(), this);
      // force initial load
      loadAccount();
      refreshView();
    } catch(MyMoneyException *e) {
      qDebug("Unexpected exception in KLedgerView::setCurrentAccount");
      delete e;
    }
  }
}

void KLedgerView::reloadAccount(const bool repaint)
{
  m_transactionList = MyMoneyFile::instance()->transactionList(m_account.id());

  // filter all unwanted transactions
  refreshView();

  if(repaint == true) {
    m_register->repaintContents();
    // if the very last transaction was deleted, we need to update
    // the index to the current transaction
    if(m_register->currentTransactionIndex() >= m_transactionList.count())
      m_register->setCurrentTransactionIndex(m_transactionList.count()-1);

    // don't forget to show the data in the form
    fillForm();
  }
}

void KLedgerView::loadAccount(void)
{
  reloadAccount(false);

  // select the last transaction
  m_register->setCurrentTransactionIndex(m_transactionList.count()-1);

  // make sure, full transaction is visible
  m_register->ensureTransactionVisible();
/*
  m_register->ensureCellVisible((m_transactionPtrVector.count() * m_register->rpt())-1, 0);
  m_register->ensureCellVisible((m_transactionPtrVector.count()-1) * m_register->rpt(), 0);
  m_register->repaintContents();
*/
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
  m_register->setTransactionCount(m_transactionPtrVector.count()+1);
  resizeEvent(NULL);
}

void KLedgerView::filterTransactions(void)
{
  int   i;

  QValueList<MyMoneyTransaction>::ConstIterator it_t;

  // setup the pointer vector
  m_transactionPtrVector.clear();
  m_transactionPtrVector.resize(m_transactionList.size());
  for(i = 0, it_t = m_transactionList.begin(); it_t != m_transactionList.end(); ++it_t) {
    // only show those transactions, that are posted after the configured start date
    if((*it_t).postDate() < m_dateStart)
      continue;

    // add more filters before this line ;-)

    // Wow, we made it through all the filters. Guess we have to show this one
    m_transactionPtrVector.insert(i, &(*it_t));
    ++i;
  }
  m_transactionPtrVector.resize(i);

  // sort the transactions
  m_transactionPtrVector.sort();

  // calculate the balance for each item
  MyMoneyMoney balance(0);
  m_balance.resize(i, balance);

  balance = MyMoneyFile::instance()->balance(accountId());
  // the trick is to go backwards ;-)
  while(--i >= 0) {
    m_balance[i] = balance;
    balance -= m_transactionPtrVector[i]->split(accountId()).value();
  }
}

void KLedgerView::update(const QCString& accountId)
{
  try {
    reloadAccount(true);
  } catch(MyMoneyException *e) {
    qDebug("Unexpected exception in KLedgerView::update");
    delete e;
  }
}

MyMoneyTransaction* const KLedgerView::transaction(const int idx) const
{
  if(idx >= 0 && idx < m_transactionPtrVector.count())
    return m_transactionPtrVector[idx];
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
    m_register->ensureTransactionVisible();
    m_register->repaintContents();
    fillForm();
    emit transactionSelected();
  }
}

void KLedgerView::selectTransaction(const QCString& transactionId)
{
  int i;

  for(i = 0; i < m_transactionPtrVector.size(); ++i) {
    if(m_transactionPtrVector[i]->id() == transactionId) {
      m_register->setCurrentTransactionIndex(i);
      m_register->ensureTransactionVisible();
      m_register->repaintContents();
      fillForm();
      emit transactionSelected();
      break;
    }
  }
}

void KLedgerView::slotPayeeChanged(const QString& name)
{
  if(name != "") {
    MyMoneyPayee payee;
    try {
      payee = MyMoneyFile::instance()->payeeByName(name);
      m_split.setPayeeId(payee.id());

    } catch(MyMoneyException *e) {
      KMessageBox::detailedSorry(0, i18n("Unable to setup payee/receiver"),
        (e->what() + " " + i18n("thrown in") + " " + e->file()+ ":%1").arg(e->line()));
      delete e;
      m_editPayee->resetText();
    }

  } else {
    // clear the field
    m_split.setPayeeId("");
  }

  try {
    m_transaction.modifySplit(m_split);
    m_editPayee->loadText(name);

  } catch(MyMoneyException *e) {
    KMessageBox::detailedSorry(0, i18n("Unable to modify split"),
        (e->what() + " " + i18n("thrown in") + " " + e->file()+ ":%1").arg(e->line()));
    delete e;
    m_editPayee->resetText();
  }

}

void KLedgerView::slotMemoChanged(const QString& memo)
{
  m_split.setMemo(memo);

  try {
    m_transaction.modifySplit(m_split);
    m_editMemo->loadText(memo);
  } catch(MyMoneyException *e) {
    KMessageBox::detailedSorry(0, i18n("Unable to modify split"),
        (e->what() + " " + i18n("thrown in") + " " + e->file()+ ":%1").arg(e->line()));
    delete e;
    m_editMemo->resetText();
  }
}

void KLedgerView::slotAmountChanged(const QString& value)
{
  MyMoneyTransaction t;
  MyMoneySplit s;

  t = m_transaction;
  s = m_split;

  try {
    m_editAmount->loadText(value);
    m_split.setValue(MyMoneyMoney(value.toDouble()));
    switch(transactionType(m_split)) {
      case Deposit:
        break;
      default:
        // make it negative in case of !deposit
        m_split.setValue(-m_split.value());
        break;
    }
    m_transaction.modifySplit(m_split);
    if(m_transaction.splitCount() == 2) {
      MyMoneySplit split = m_transaction.split(accountId(), false);
      split.setValue(-m_split.value());
      m_transaction.modifySplit(split);
    }
  } catch(MyMoneyException *e) {
    KMessageBox::detailedSorry(0, i18n("Unable to modify amount"),
        (e->what() + " " + i18n("thrown in") + " " + e->file()+ ":%1").arg(e->line()));
    delete e;
    m_editAmount->resetText();
    m_transaction = t;
    m_split = s;
  }
}

void KLedgerView::slotCategoryChanged(const QString& category)
{
  MyMoneyTransaction t = m_transaction;
  MyMoneySplit s = m_split;

  try {
    // First, we check if the category exists
    QCString id = MyMoneyFile::instance()->categoryToAccount(category);
    if(id == "") {
      // FIXME:
      /// Todo: Add account (hierarchy) upon new category
      KMessageBox::sorry(0, i18n("Direct creation of new account not yet implemented"));
      m_editCategory->resetText();
      m_editCategory->setFocus();
      return;
    }

    // We have to distinguish between the following cases
    //
    // a) transaction contains just a single split
    // b) transaction contains exactly two splits
    // c) transaction contains more than two splits
    //
    // Here's how we react
    //
    // a) add a split with the account set to the new category
    // b) modify the split and modify the account to the new category
    // c) ask the user that all data about the splits are lost. Upon
    //    positive response remove all splits except the one that
    //    references accountId() then continue with a)

    QValueList<MyMoneySplit> list = m_transaction.splits();
    QValueList<MyMoneySplit>::Iterator it;
    MyMoneySplit split;

    switch(m_transaction.splitCount()) {
      default:
        if(KMessageBox::warningContinueCancel(0,
            i18n("If you press continue, information about all other splits will be lost"),
            i18n("Splitted Transaction")) == KMessageBox::Cancel) {
          m_editCategory->resetText();
          m_editCategory->setFocus();
        }
        for(it = list.begin(); it != list.end(); ++it) {
          if((*it) == m_split)
            continue;
          m_transaction.removeSplit((*it));
        }
        // tricky fall through here
      case 1:
        split.setAccountId(id);
        split.setValue(-m_split.value());
        m_transaction.addSplit(split);
        break;

      case 2:
        // find the 'other' split
        split = m_transaction.split(accountId(), false);
        split.setAccountId(id);
        m_transaction.modifySplit(split);
        break;
    }

    m_editCategory->loadText(category);

  } catch(MyMoneyException *e) {
    KMessageBox::detailedSorry(0, i18n("Unable to modify category"),
        (e->what() + " " + i18n("thrown in") + " " + e->file()+ ":%1").arg(e->line()));
    delete e;
    m_editCategory->resetText();

    m_transaction = t;
    m_split = s;
  }
}

void KLedgerView::slotFromChanged(const QString& from)
{
  MyMoneyTransaction t = m_transaction;
  MyMoneySplit s = m_split;

  try {
    // First, we check if the account exists
    QCString id = MyMoneyFile::instance()->nameToAccount(from);
    if(id == "") {
      // FIXME:
      /// Todo: Add account (hierarchy) upon new category
      KMessageBox::sorry(0, i18n("Direct creation of new account not yet implemented"));
      m_editFrom->resetText();
      m_editFrom->setFocus();
      return;
    }

    // For transfers we assume, that the first split contains the
    // from-account and the second split contains the to-account.
    // Therefor, the following cases can exist:
    //
    // a) one split available
    // b) two splits available
    //
    // In both cases, we just modify the first split.

    MyMoneySplit split = m_transaction.splits()[0];

    split.setAccountId(id);
    m_transaction.modifySplit(split);

    m_editFrom->loadText(from);

  } catch(MyMoneyException *e) {
    KMessageBox::detailedSorry(0, i18n("Unable to modify account"),
        (e->what() + " " + i18n("thrown in") + " " + e->file()+ ":%1").arg(e->line()));
    delete e;
    m_editFrom->resetText();

    m_transaction = t;
    m_split = s;
  }

}

void KLedgerView::slotToChanged(const QString& to)
{
  MyMoneyTransaction t = m_transaction;
  MyMoneySplit s = m_split;

  try {
    // First, we check if the account exists
    QCString id = MyMoneyFile::instance()->nameToAccount(to);
    if(id == "") {
      // FIXME:
      /// Todo: Add account (hierarchy) upon new category
      KMessageBox::sorry(0, i18n("Direct creation of new account not yet implemented"));
      m_editTo->resetText();
      m_editTo->setFocus();
      return;
    }

    // For transfers we assume, that the first split contains the
    // from-account and the second split contains the to-account.
    // Therefor, the following cases can exist:
    //
    // a) one split available
    // b) two splits available
    //
    // In the first case, we add a new split, in the second, we
    // modifiy the existing split.

    MyMoneySplit split;
    if(m_transaction.splitCount() == 1) {
      split.setAction(MyMoneySplit::ActionTransfer);
      m_transaction.addSplit(split);
    }

    split = m_transaction.splits()[1];

    split.setAccountId(id);
    m_transaction.modifySplit(split);

    m_editTo->loadText(to);

  } catch(MyMoneyException *e) {
    KMessageBox::detailedSorry(0, i18n("Unable to modify account"),
        (e->what() + " " + i18n("thrown in") + " " + e->file()+ ":%1").arg(e->line()));
    delete e;
    m_editTo->resetText();

    m_transaction = t;
    m_split = s;
  }
}


void KLedgerView::slotNrChanged(const QString& nr)
{
  MyMoneyTransaction t = m_transaction;
  MyMoneySplit s = m_split;
  try {
    qDebug("slotNrChanged from %s to %s", m_split.number().latin1(), nr.latin1());
    m_split.setNumber(nr);
    m_transaction.modifySplit(m_split);
    m_editNr->loadText(nr);

  } catch(MyMoneyException *e) {
    KMessageBox::detailedSorry(0, i18n("Unable to modify number"),
        (e->what() + " " + i18n("thrown in") + " " + e->file()+ ":%1").arg(e->line()));
    delete e;
    m_editNr->resetText();

    m_transaction = t;
    m_split = s;
  }
}

void KLedgerView::slotDateChanged(const QDate& date)
{
  MyMoneyTransaction t = m_transaction;
  MyMoneySplit s = m_split;
  try {
    m_transaction.setPostDate(date);
    m_editDate->loadDate(date);

  } catch(MyMoneyException *e) {
    KMessageBox::detailedSorry(0, i18n("Unable to modify number"),
        (e->what() + " " + i18n("thrown in") + " " + e->file()+ ":%1").arg(e->line()));
    delete e;
    m_editDate->resetDate();

    m_transaction = t;
    m_split = s;
  }

}

void KLedgerView::slotShowTransactionForm(bool visible)
{
  if(m_form != 0) {
    if(visible) {
      // block signals here to avoid running into the NEW case
      // which is triggered by the signal tabBar()->selected(int)
      m_form->tabBar()->blockSignals(true);
      m_form->show();
      m_form->tabBar()->blockSignals(false);
    } else
      m_form->hide();
  }

  if(m_register != 0) {
    m_register->setInlineEditingAvailable(!visible);
    // make sure, full transaction is visible
    resizeEvent(NULL);

    // using a timeout is the only way, I got the 'ensureTransactionVisible'
    // working when coming from hidden form to visible form. I assume, this
    // has something to do with the delayed update of the display somehow.
    if(m_timer == 0) {
    	m_timer = new QTimer();
      connect( m_timer, SIGNAL(timeout()), this, SLOT(timerDone()) );
      m_timer->start(50, true);
    }
  }
}

void KLedgerView::timerDone(void)
{
  m_register->ensureTransactionVisible();
  m_register->repaintContents();
  delete m_timer;
  m_timer = 0;
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
        (e->what() + " " + i18n("thrown in") + " " + e->file()+ ":%1").arg(e->line()));
      delete e;

      m_editPayee->resetText();
    }
  } else
    m_editPayee->resetText();
}

int KLedgerView::transactionType(const MyMoneySplit& split) const
{
  if(split.action() == MyMoneySplit::ActionCheck)
    return Check;
  if(split.action() == MyMoneySplit::ActionDeposit)
    return Deposit;
  if(split.action() == MyMoneySplit::ActionTransfer)
    return Transfer;
  if(split.action() == MyMoneySplit::ActionWithdrawal)
    return Withdrawal;
  if(split.action() == MyMoneySplit::ActionATM)
    return ATM;
  return Check;
}

void KLedgerView::hideEvent(QHideEvent *ev)
{
  slotCancelEdit();
}

void KLedgerView::slotNew(void)
{
  // select the very last line (empty one), and load it into the form
  m_register->setCurrentTransactionIndex(m_transactionList.count());
  m_register->ensureTransactionVisible();
  m_register->repaintContents();
  fillForm();

  m_form->enterButton()->setEnabled(true);
  m_form->cancelButton()->setEnabled(true);
  m_form->moreButton()->setEnabled(true);
  m_form->editButton()->setEnabled(false);
  m_form->newButton()->setEnabled(false);

  showWidgets();
}

void KLedgerView::slotStartEdit(void)
{
  m_register->ensureTransactionVisible();

  m_form->newButton()->setEnabled(false);
  m_form->enterButton()->setEnabled(true);
  m_form->cancelButton()->setEnabled(true);
  m_form->moreButton()->setEnabled(true);
  m_form->editButton()->setEnabled(false);

  showWidgets();
}

void KLedgerView::slotCancelEdit(void)
{
  m_form->newButton()->setEnabled(true);
  m_form->enterButton()->setEnabled(false);
  m_form->cancelButton()->setEnabled(false);
  m_form->moreButton()->setEnabled(false);

  if(transaction(m_register->currentTransactionIndex()) != 0) {
    m_form->editButton()->setEnabled(true);
  }

  hideWidgets();
}

void KLedgerView::slotEndEdit(void)
{
  // force focus change to update all data
  m_form->enterButton()->setFocus();

  MyMoneyTransaction t;

  // so, we now have to save something here.
  // if an existing transaction has been changed, we take it as the base
  if(m_transactionPtr != 0) {
    t = *m_transactionPtr;
  }

  if(!(t == m_transaction)) {
    // If there are any differences, we need to update the storage
    // But first we check for the following things:
    //
    // a) transaction must have 2 or more than 2 splits
    // b) the sum of all split amounts must be zero

    if(m_transaction.splitCount() < 2) {
      ;
    }
    if(m_transaction.splitSum() != 0) {
      ;
    }
    try {
      // differentiate between add and modify
      QCString id;
      if(m_transactionPtr == 0) {
        // in the add case, we don't have an ID yet. So let's get one
        // and use it down the line
        m_transaction.setPostDate(QDate::currentDate());
        MyMoneyFile::instance()->addTransaction(m_transaction);
        id = m_transaction.id();
      } else {
        // in the modify case, we have to keep the id. The call to
        // modifyTransaction might change m_transaction due to some
        // callbacks.
        id = m_transaction.id();
        MyMoneyFile::instance()->modifyTransaction(m_transaction);
      }

      // make sure the transaction stays selected. It's position might
      // have changed within the register (e.g. date changed)
      selectTransaction(id);

    } catch(MyMoneyException *e) {
      KMessageBox::detailedSorry(0, i18n("Unable to add/modify transaction"),
        (e->what() + " " + i18n("thrown in") + " " + e->file()+ ":%1").arg(e->line()));
      delete e;
    }
  }

  // now switch the context
  m_form->newButton()->setEnabled(true);
  m_form->enterButton()->setEnabled(false);
  m_form->cancelButton()->setEnabled(false);
  m_form->moreButton()->setEnabled(false);

  if(transaction(m_register->currentTransactionIndex()) != 0) {
    m_form->editButton()->setEnabled(true);
  }

  hideWidgets();
}

