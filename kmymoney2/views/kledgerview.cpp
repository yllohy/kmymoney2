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
#include <qapp.h>
#include <qwidgetstack.h>
#include <qcursor.h>
#include <qtimer.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kmessagebox.h>
#include <kpushbutton.h>
#include <kconfig.h>
#include <kpopupmenu.h>
#include <kiconloader.h>
#include <kcmenumngr.h>

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
#include "../widgets/kmymoneycombo.h"
#include "../mymoney/mymoneyfile.h"

int KTransactionPtrVector::compareItems(const QCString& s1, const QCString& s2) const
{
  if(s1 == s2)
    return 0;
  if(s1 < s2)
    return -1;
  return 1;
}

int KTransactionPtrVector::compareItems(const QString& s1, const QString& s2) const
{
  if(s1 == s2)
    return 0;
  if(s1 < s2)
    return -1;
  return 1;
}

int KTransactionPtrVector::compareItems(KTransactionPtrVector::Item d1, KTransactionPtrVector::Item d2)
{
  int   rc = 0;
  MyMoneyTransaction* t1 = static_cast<MyMoneyTransaction*>(d1);
  MyMoneyTransaction* t2 = static_cast<MyMoneyTransaction*>(d2);

  try {
    MyMoneySplit s1;
    MyMoneySplit s2;
    switch(m_idMode) {
      case AccountMode:
        s1 = t1->split(m_id);
        s2 = t2->split(m_id);
        break;
      case PayeeMode:
        s1 = t1->splitByPayee(m_id);
        s2 = t2->splitByPayee(m_id);
        break;
    }
    QString p1, p2;

    switch(m_sortType) {
      case SortValue:
        rc = static_cast<int> ((s2.value() - s1.value()).value());
        if(rc == 0) {
          // same value? Sort by date
          rc = t2->postDate().daysTo(t1->postDate());
          if(rc == 0) {
            // same date? Sort by value
            rc = static_cast<int> ((s2.value() - s1.value()).value());
            if(rc == 0) {
              // same value? sort by id
              rc = compareItems(t1->id(), t2->id());
            }
          }
        }
        break;

      case SortEntryDate:
        rc = t2->entryDate().daysTo(t1->entryDate());
        if(rc == 0) {
          // same date? Sort by value
          rc = static_cast<int> ((s2.value() - s1.value()).value());
          if(rc == 0) {
            // same value? sort by id
            rc = compareItems(t1->id(), t2->id());
          }
        }
        break;

      case SortTypeNr:
        rc = compareItems(s1.action(), s2.action());

        if(rc == 0) {
          // same action? Sort by nr
          rc = compareItems(s1.number(), s2.number());
          if(rc == 0) {
            // same number? Sort by date
            rc = t2->postDate().daysTo(t1->postDate());
            if(rc == 0) {
              // same date? Sort by value
              rc = static_cast<int> ((s2.value() - s1.value()).value());
              if(rc == 0) {
                // same value? sort by id
                rc = compareItems(t1->id(), t2->id());
              }
            }
          }
        }
        break;

      case SortReceiver:
        if(s2.payeeId() != "") {
          p2 = MyMoneyFile::instance()->payee(s2.payeeId()).name();
        }
        if(s1.payeeId() != "") {
          p1 = MyMoneyFile::instance()->payee(s1.payeeId()).name();
        }

        rc = compareItems(p1, p2);

        if(rc == 0) {
          // same payee? Sort by date
          rc = t2->postDate().daysTo(t1->postDate());
          if(rc == 0) {
            // same date? Sort by value
            rc = static_cast<int> ((s2.value() - s1.value()).value());
            if(rc == 0) {
              // same value? sort by id
              rc = compareItems(t1->id(), t2->id());
            }
          }
        }
        break;

      case SortNr:
        rc = compareItems(s1.number(), s2.number());
        if(rc == 0) {
          // same number? Sort by date
          rc = t2->postDate().daysTo(t1->postDate());
          if(rc == 0) {
            // same date? Sort by value
            rc = static_cast<int> ((s2.value() - s1.value()).value());
            if(rc == 0) {
              // same value? sort by id
              rc = compareItems(t1->id(), t2->id());
            }
          }
        }
        break;

      case SortPostDate:
      // tricky fall through here!
      default:
        // sort by post date
        rc = t2->postDate().daysTo(t1->postDate());
        if(rc == 0) {
          // on same day, larger amounts show up first
          rc = static_cast<int> ((s2.value() - s1.value()).value());
          if(rc == 0) {
            // same value? Sort by id
            rc = compareItems(t1->id(), t2->id());
          }
        }
        break;
    }
  } catch (MyMoneyException *e) {
    delete e;
  }
  return rc;
}

void KTransactionPtrVector::setSortType(const TransactionSortE type)
{
  m_sortType = type;
  sort();
}

void KTransactionPtrVector::setAccountId(const QCString& id)
{
  m_id = id;
  m_idMode = AccountMode;
}

void KTransactionPtrVector::setPayeeId(const QCString& id)
{
  m_id = id;
  m_idMode = PayeeMode;
}

KLedgerView::KLedgerView(QWidget *parent, const char *name )
  : QWidget(parent,name),
  m_contextMenu(0),
  m_blinkTimer(parent),
  m_suspendUpdate(false)
{
  KConfig *config = KGlobal::config();
  config->setGroup("General Options");
  m_ledgerLens = config->readBoolEntry("LedgerLens", true);
  m_transactionFormActive = config->readBoolEntry("TransactionForm", true);

  m_register = 0;
  m_form = 0;
  m_transactionPtr = 0;
  m_timer = 0;

  m_editPayee = 0;
  m_editCategory = 0;
  m_editMemo = 0;
  m_editAmount = 0;
  m_editNr = 0;
  m_editDate = 0;
  m_editFrom = 0;
  m_editTo = 0;
  m_editType = 0;

  m_infoStack = 0;
  m_inReconciliation = false;

  m_blinkTimer.start(500);       // setup blink frequency to one hertz
  m_blinkState = false;
  connect(&m_blinkTimer, SIGNAL(timeout()), SLOT(slotBlinkTimeout()));
}

KLedgerView::~KLedgerView()
{
  if(m_infoStack != 0)
    delete m_infoStack;

  MyMoneyFile::instance()->detach(m_account.id(), this);
}

void KLedgerView::slotBlinkTimeout(void)
{
  m_blinkState = !m_blinkState;
  if(m_register) {
    m_register->slotSetErrorColor(m_blinkState);
    m_register->repaintContents(false);
  }
}

void KLedgerView::setCurrentAccount(const QCString& accountId, const bool force)
{
  slotCancelEdit();

  if(!accountId.isEmpty()) {
    if(accountId != m_account.id() || force == true) {

      MyMoneyFile* file = MyMoneyFile::instance();

      file->detach(m_account.id(), this);

      try {
        m_account = file->account(accountId);
        file->attach(m_account.id(), this);
        // force initial load
        loadAccount();
        enableWidgets(true);
        // refreshView();
      } catch(MyMoneyException *e) {
        qDebug("Unexpected exception in KLedgerView::setCurrentAccount");
        delete e;
      }
    }
  } else {
    enableWidgets(false);
  }
}

void KLedgerView::reloadAccount(const bool repaint)
{
  // in case someone changed the account info and we are called here
  // via the observer's update function, we just reload ourselves.
  MyMoneyFile* file = MyMoneyFile::instance();
  m_account = file->account(m_account.id());

  // get a current transaction list for the account
  m_transactionList = file->transactionList(m_account.id());

  // filter all unwanted transactions
  updateView();

  if(repaint == true) {
    m_register->repaintContents(false);
    // if the very last transaction was deleted, we need to update
    // the index to the current transaction
    bool selectFlag = false;
    if(static_cast<unsigned>(m_register->currentTransactionIndex()) >= m_transactionList.count()) {
      m_register->setCurrentTransactionIndex(m_transactionList.count()-1);
      selectFlag = true;
    }

    // don't forget to show the data in the form
    fillForm();

    // and the summary line
    fillSummary();

    if(selectFlag == true)
      emit transactionSelected();
  }

}

void KLedgerView::loadAccount(void)
{
  reloadAccount(false);

  // select the last transaction
  m_register->setCurrentTransactionIndex(m_transactionList.count()-1);

  // make sure, full transaction is visible
  m_register->ensureTransactionVisible();

  // fill in the form with the currently selected transaction
  fillForm();

  // fill in the current summary as well
  fillSummary();

  // Let others know, that we selected a transaction
  emit transactionSelected();
}

void KLedgerView::refreshView(void)
{
  KConfig *config = KGlobal::config();
  config->setGroup("General Options");
  m_ledgerLens = config->readBoolEntry("LedgerLens", true);
  m_transactionFormActive = config->readBoolEntry("TransactionForm", true);
  m_register->readConfig();

  updateView();
}

void KLedgerView::updateView(void)
{
  KConfig *config = KGlobal::config();
  config->setGroup("List Options");

  QDateTime defaultDate;
  m_dateStart = config->readDateTimeEntry("StartDate", &defaultDate).date();

  filterTransactions();

  slotShowTransactionForm(m_transactionFormActive);

  m_register->setTransactionCount(m_transactionPtrVector.count()+1);
  resizeEvent(NULL);
}

void KLedgerView::enableWidgets(const bool enable)
{
  m_form->setEnabled(enable);
}

void KLedgerView::filterTransactions(void)
{
  int   i;

  QValueList<MyMoneyTransaction>::ConstIterator it_t;

  // setup the pointer vector
  m_transactionPtrVector.clear();
  m_transactionPtrVector.resize(m_transactionList.size());
  m_transactionPtrVector.setAccountId(m_account.id());

  for(i = 0, it_t = m_transactionList.begin(); it_t != m_transactionList.end(); ++it_t) {

    // if in reconciliation mode, don't show old stuff and don't
    // use any of the other filters
    if(m_inReconciliation == true) {
      MyMoneySplit s = (*it_t).split(m_account.id());
      if(s.reconcileFlag() == MyMoneySplit::Reconciled
      || s.reconcileFlag() == MyMoneySplit::Frozen) {
        continue;
      }
    } else {

      // only show those transactions, that are posted after the configured start date
      if((*it_t).postDate() < m_dateStart)
        continue;

      // add more filters before this line ;-)
    }

    // Wow, we made it through all the filters. Guess we have to show this one
    m_transactionPtrVector.insert(i, &(*it_t));
    ++i;
  }
  m_transactionPtrVector.resize(i);

  // sort the transactions
  m_transactionPtrVector.sort();

  // calculate the balance for each item. At the same time
  // we figure out the row where the current date mark should
  // be shown if it's sorted by post date.

  MyMoneyMoney balance(0);
  m_balance.resize(i, balance);

  bool dateMarkPlaced = false;
  m_register->setCurrentDateIndex();    // turn off date mark

  try {
    balance = MyMoneyFile::instance()->balance(accountId());
    // the trick is to go backwards ;-)
    while(--i >= 0) {
      m_balance[i] = balance;
      balance -= m_transactionPtrVector[i]->split(accountId()).value();
      if(m_transactionPtrVector.sortType() == KTransactionPtrVector::SortPostDate) {
        if(m_transactionPtrVector[i]->postDate() > QDate::currentDate()) {
          m_register->setCurrentDateIndex(i+1);

        } else if(dateMarkPlaced == false) {
          m_register->setCurrentDateIndex(i+1);
          dateMarkPlaced = true;
        }
      }
    }
  } catch(MyMoneyException *e) {
    if(accountId() != "")
      qDebug("Unexpected exception in KLedgerView::filterTransactions");
    delete e;
  }
}


void KLedgerView::update(const QCString& accountId)
{
  if(m_suspendUpdate == false) {
    try {
      reloadAccount(true);
    } catch(MyMoneyException *e) {
      qDebug("Unexpected exception in KLedgerView::update for '%s'", m_account.id().data());
      delete e;
    }
  }
}

void KLedgerView::suspendUpdate(const bool suspend)
{
  // force a refresh, if update was off
  if(m_suspendUpdate == true
  && suspend == false) {
    m_suspendUpdate = false;
    if(m_account.id() != "")
      update("");
    
  } else
    m_suspendUpdate = suspend;
}

MyMoneyTransaction* const KLedgerView::transaction(const int idx) const
{
  if(idx >= 0 && static_cast<unsigned> (idx) < m_transactionPtrVector.count())
    return m_transactionPtrVector[idx];
  return 0;
}

const MyMoneyMoney KLedgerView::balance(const int idx) const
{
  MyMoneyMoney bal(0);

  if(idx >= 0 && static_cast<unsigned> (idx) < m_balance.size())
    bal = m_balance[idx];

  if(MyMoneyFile::instance()->accountGroup(m_account.accountType()) == MyMoneyAccount::Liability)
    bal = -bal; 
  return bal;
}

void KLedgerView::slotRegisterClicked(int row, int col, int button, const QPoint &mousePos)
{
  if(!(m_account.id().isEmpty())) {
    // only redraw the register and form, when a different
    // transaction has been selected with this click.
    if(m_register->setCurrentTransactionRow(row) == true) {
      m_register->ensureTransactionVisible();
      m_register->repaintContents(false);

      slotCancelEdit();

      // if the very last entry has been selected, it means, that
      // a new transaction should be created.
      if(static_cast<unsigned> (m_register->currentTransactionIndex()) == m_transactionList.count()) {
        slotNew();
      } else {
        fillForm();
        fillSummary();
      }

      emit transactionSelected();

    }

    if(button == Qt::RightButton) {
      if(static_cast<unsigned> (m_register->currentTransactionIndex()) != m_transactionList.count()) {
        slotCancelEdit();
        m_contextMenu->exec(QCursor::pos());
      }
    }
  }
}

void KLedgerView::slotNextTransaction(void)
{
  // up and down movement is not allowed when editing inline
  if(!m_transactionFormActive && isEditMode())
    return;

  if(static_cast<unsigned> (m_register->currentTransactionIndex() + 1) <= m_transactionPtrVector.count()) {
    slotCancelEdit();
    m_register->setCurrentTransactionIndex(m_register->currentTransactionIndex()+1);
    m_register->ensureTransactionVisible();
    m_register->repaintContents(false);
    fillForm();
    fillSummary();
    emit transactionSelected();
  }
}

void KLedgerView::slotPreviousTransaction(void)
{
  // up and down movement is not allowed when editing inline
  if(!m_transactionFormActive && isEditMode())
    return;

  if(m_register->currentTransactionIndex() > 0) {
    slotCancelEdit();
    m_register->setCurrentTransactionIndex(m_register->currentTransactionIndex()-1);
    m_register->ensureTransactionVisible();
    m_register->repaintContents(false);
    fillForm();
    fillSummary();
    emit transactionSelected();
  }
}

bool KLedgerView::selectTransaction(const QCString& transactionId)
{
  bool  rc = false;

  for(unsigned i = 0; i < m_transactionPtrVector.size(); ++i) {
    if(m_transactionPtrVector[i]->id() == transactionId) {
      m_register->setCurrentTransactionIndex(i);
      m_register->ensureTransactionVisible();
      m_register->repaintContents(false);
      fillForm();
      fillSummary();
      emit transactionSelected();
      rc = true;
      break;
    }
  }
  return rc;
}

void KLedgerView::slotPayeeChanged(const QString& name)
{
  if(!m_editPayee)
    return;

  MyMoneySplit sp;
  if(m_transaction.splitCount() == 2) {
    sp = m_transaction.split(m_account.id(), false);
  }

  if(name != "") {
    MyMoneyPayee payee;
    try {
      createSecondSplit();

      payee = MyMoneyFile::instance()->payeeByName(name);
      m_split.setPayeeId(payee.id());
      // for tranfers, we always modify the other side as well
      if(m_split.action() == MyMoneySplit::ActionTransfer) {
        sp.setPayeeId(payee.id());
      }

    } catch(MyMoneyException *e) {
      KMessageBox::detailedSorry(0, i18n("Unable to setup payee/receiver"),
        (e->what() + " " + i18n("thrown in") + " " + e->file()+ ":%1").arg(e->line()));
      delete e;
      m_editPayee->resetText();
    }

  } else {
    // clear the field
    m_split.setPayeeId("");
    // for tranfers, we always modify the other side as well
    if(m_split.action() == MyMoneySplit::ActionTransfer) {
      sp.setPayeeId("");
    }
  }

  try {
    m_transaction.modifySplit(m_split);
    if(m_split.action() == MyMoneySplit::ActionTransfer
    && m_transaction.splitCount() == 2)
      m_transaction.modifySplit(sp);

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
  if(!m_editMemo)
    return;

  m_split.setMemo(memo);

  try {
    createSecondSplit();
    m_transaction.modifySplit(m_split);
    m_editMemo->loadText(memo);
    // for tranfers, we always modify the other side as well
    if(m_transaction.splitCount() == 2
    && m_split.action() == MyMoneySplit::ActionTransfer) {
      MyMoneySplit sp = m_transaction.split(m_account.id(), false);
      sp.setMemo(memo);
      m_transaction.modifySplit(sp);
    }
  } catch(MyMoneyException *e) {
    KMessageBox::detailedSorry(0, i18n("Unable to modify split"),
        (e->what() + " " + i18n("thrown in") + " " + e->file()+ ":%1").arg(e->line()));
    delete e;
    m_editMemo->resetText();
  }
}

void KLedgerView::slotAmountChanged(const QString& value)
{
  if(!m_editAmount)
    return;

  MyMoneyTransaction t;
  MyMoneySplit s;

  t = m_transaction;
  s = m_split;

  try {
    createSecondSplit();

    MyMoneyMoney val = MyMoneyMoney(value);
    // if someone enters a negative number, we have to make sure that
    // the action is corrected. For transfers, we don't have to do anything
    // The accounts will be 'exchanged' in reloadEditWidgets() and fillForm()
    if(MyMoneyMoney(value) < 0) {
      QCString accountId;
      MyMoneySplit split;
      switch(transactionType(m_split)) {
        case Transfer:
          break;
        case Deposit:
          m_split.setAction(MyMoneySplit::ActionWithdrawal);
          val = -val;
          break;
        default:
          m_split.setAction(MyMoneySplit::ActionDeposit);
          val = -val;
          break;
      }
    }

    switch(transactionType(m_split)) {
      case Deposit:
        break;
      case Transfer:
        // if it's the deposit part of a transfer, we don't invert sign
        if(m_split.value() > 0)
          break;
        // tricky fall through here in case we take something out of the
        // current account :-(

      default:
        // make it negative in case of !deposit
        val = -val;
        break;
    }

    m_split.setValue(val);

    m_editAmount->loadText(value);
    m_transaction.modifySplit(m_split);
    if(m_transaction.splitCount() == 2) {
      MyMoneySplit split = m_transaction.split(accountId(), false);
      split.setValue(-m_split.value());
      m_transaction.modifySplit(split);
    }
    reloadEditWidgets(m_transaction);

  } catch(MyMoneyException *e) {
    KMessageBox::detailedSorry(0, i18n("Unable to modify amount"),
        (e->what() + " " + i18n("thrown in") + " " + e->file()+ ":%1").arg(e->line()));
    delete e;
    m_editAmount->resetText();
    m_transaction = t;
    m_split = s;
  }
}

void KLedgerView::slotPaymentChanged(const QString& value)
{
  if(!m_editPayment)
    return;

  MyMoneyTransaction t;
  MyMoneySplit s;
  MyMoneyMoney val(value);

  t = m_transaction;
  s = m_split;

  if(val < 0) {
    val = -val;
    slotDepositChanged(val.formatMoney());
    return;
  }

  try {
    createSecondSplit();

    m_split.setValue(-MyMoneyMoney(value));
    m_editPayment->loadText(value);
    m_editDeposit->loadText("");

    if(m_split.action() == MyMoneySplit::ActionDeposit)
      m_split.setAction(MyMoneySplit::ActionWithdrawal);

    m_transaction.modifySplit(m_split);
    if(m_transaction.splitCount() == 2) {
      MyMoneySplit split = m_transaction.split(accountId(), false);
      split.setValue(-m_split.value());
      m_transaction.modifySplit(split);
    }
    reloadEditWidgets(m_transaction);

  } catch(MyMoneyException *e) {
    KMessageBox::detailedSorry(0, i18n("Unable to modify amount"),
        (e->what() + " " + i18n("thrown in") + " " + e->file()+ ":%1").arg(e->line()));
    delete e;
    m_editAmount->resetText();
    m_transaction = t;
    m_split = s;
  }
}

void KLedgerView::slotDepositChanged(const QString& value)
{
  if(!m_editPayment)
    return;

  MyMoneyTransaction t;
  MyMoneySplit s;
  MyMoneyMoney val(value);

  if(val < 0) {
    val = -val;
    slotPaymentChanged(val.formatMoney());
    return;
  }

  t = m_transaction;
  s = m_split;

  try {
    createSecondSplit();

    m_split.setValue(MyMoneyMoney(value));
    m_editDeposit->loadText(value);
    m_editPayment->loadText("");

    if(m_split.action() != MyMoneySplit::ActionDeposit
    && m_split.action() != MyMoneySplit::ActionTransfer)
      m_split.setAction(MyMoneySplit::ActionDeposit);

    m_transaction.modifySplit(m_split);
    if(m_transaction.splitCount() == 2) {
      MyMoneySplit split = m_transaction.split(accountId(), false);
      split.setValue(-m_split.value());
      m_transaction.modifySplit(split);
    }
    reloadEditWidgets(m_transaction);

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
  if(!m_editCategory)
    return;

  MyMoneyTransaction t = m_transaction;
  MyMoneySplit s = m_split;

  // usually we call createSecondSplit() here in all the other xxxChanged()
  // functions, but we don't do it for slotCategoryChanged() because we will
  // remove it later on anyway.

  try {
    // First, we check if the category exists
    QCString id = MyMoneyFile::instance()->categoryToAccount(category);
    if(id == "" && category != "") {
      // FIXME:
      /// Todo: Add account (hierarchy) upon new category
      KMessageBox::sorry(0, i18n("Direct creation of new account not yet implemented"));
      m_editCategory->resetText();
      m_editCategory->setFocus();
      return;
    }

    if(category == "") {
      if(m_transaction.splitCount() == 2) {
        MyMoneySplit sp = m_transaction.split(m_account.id(), false);
        m_transaction.removeSplit(sp);
      }
    } else {

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
        // The more I think about it, this case cannot happen, as
        // the widget does not get active. The split dialog is
        // opened instead. So I think, we could remove the logic
        // for default
        default:
          if(KMessageBox::warningContinueCancel(0,
              i18n("If you press continue, information about all other splits will be lost"),
              i18n("Splitted Transaction")) == KMessageBox::Cancel) {
            m_editCategory->resetText();
            m_editCategory->setFocus();
            return;
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
  fromToChanged(true, from);
}

void KLedgerView::slotToChanged(const QString& to)
{
  fromToChanged(false, to);
}

void KLedgerView::fromToChanged(const bool fromChanged, const QString& accountName)
{
  if(!m_editFrom)
    return;

  MyMoneyTransaction t = m_transaction;
  MyMoneySplit s = m_split;

  try {
    createSecondSplit();

    // First, we check if the account exists
    QCString id = MyMoneyFile::instance()->nameToAccount(accountName);
    if(id == "") {
      // FIXME:
      /// Todo: Add account (hierarchy) upon new category
      KMessageBox::sorry(0, i18n("Direct creation of new account not yet implemented"));
      m_editFrom->resetText();
      m_editFrom->setFocus();
      return;
    }

    // see, which split is the from and which is the to part
    // keep the indeces and work with indeces in the following
    MyMoneySplit split;
    int fromIdx, toIdx;
    
    if(m_transaction.splits()[0].value() < 0) {
      fromIdx = 0;
      toIdx = 1;
    } else {
      fromIdx = 1;
      toIdx = 0;
    }

    if(fromChanged == true)
      split = m_transaction.splits()[fromIdx];
    else
      split = m_transaction.splits()[toIdx];
    split.setAccountId(id);
    m_transaction.modifySplit(split);

    // if the 'from account' is not this account, then the 'to account'
    // must be the current account. We force it to be this way ;-)
    // The same applies if the 'to account' is not this account, then the
    // 'from account' will be forced to be this one.
    if(id != m_account.id()) {
      switch(m_transaction.splitCount()) {
        case 2:
          if(fromChanged == true)
            split = m_transaction.splits()[toIdx];
          else
            split = m_transaction.splits()[fromIdx];
          split.setAccountId(m_account.id());
          m_transaction.modifySplit(split);
          break;
        default:
          qWarning("Transfer transaction with more than two splits detected!!!");
          break;
      }
    }
    reloadEditWidgets(m_transaction);

  } catch(MyMoneyException *e) {
    KMessageBox::detailedSorry(0, i18n("Unable to modify transaction"),
        (e->what() + " " + i18n("thrown in") + " " + e->file()+ ":%1").arg(e->line()));
    delete e;
    reloadEditWidgets(t);
    m_transaction = t;
    m_split = s;
  }
}

void KLedgerView::createSecondSplit(void)
{
  MyMoneySplit split;
  if(m_transaction.splitCount() == 1) {
    split.setAction(m_split.action());
    split.setPayeeId(m_split.payeeId());
    split.setMemo(m_split.memo());
    split.setValue(-split.value());
    m_transaction.addSplit(split);
  }
}

void KLedgerView::slotNrChanged(const QString& nr)
{
  if(!m_editNr)
    return;

  MyMoneyTransaction t = m_transaction;
  MyMoneySplit s = m_split;

  try {
    createSecondSplit();

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
  if(!isEditMode())
    return;

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

void KLedgerView::slotTypeChanged(int sel)
{
  if(!m_editType)
    return;

  QCString action;
  QTab *tab;
  tab = m_form->tabBar()->tabAt(sel);
  if(tab)
    slotTypeChanged(transactionType(tab->identifier()));
}

void KLedgerView::slotTypeChanged(const QCString& action)
{
  if(!m_editType)
    return;

  MyMoneyTransaction t = m_transaction;
  MyMoneySplit s = m_split;

  try {
    if((action == MyMoneySplit::ActionTransfer && m_split.action() != MyMoneySplit::ActionTransfer)
    || (action != MyMoneySplit::ActionTransfer && m_split.action() == MyMoneySplit::ActionTransfer)) {
      if(KMessageBox::warningContinueCancel(0,
          i18n("Changing the transaction type in the selected direction will delete all information about categories and accounts. "
                "If you press continue, this information will be lost!"),
          i18n("Transaction Type Change")) == KMessageBox::Continue) {
        // we have to remove all other splits as they are not valid anymore
        MyMoneySplit split;
        try {
          for(;;) {
            split = m_transaction.split(m_account.id(), false);
            m_transaction.removeSplit(split);
          }
        } catch(MyMoneyException *e) {
          delete e;
        }

      } else {
        m_editType->resetCurrentItem();
        m_editType->setFocus();
        return;
      }
    }

    if((action == MyMoneySplit::ActionDeposit && m_split.action() != MyMoneySplit::ActionDeposit)
    || (action != MyMoneySplit::ActionDeposit && m_split.action() == MyMoneySplit::ActionDeposit)) {
      // If we change from deposit to withdrawal and vice versa, we change the
      // sign of all splits
      QValueList<MyMoneySplit> list = m_transaction.splits();
      QValueList<MyMoneySplit>::Iterator it;

      for(it = list.begin(); it != list.end(); ++it) {
        (*it).setValue(-(*it).value());
        m_transaction.modifySplit(*it);
      }
      // don't forget to update our copy of the specific split
      m_split = m_transaction.split(m_account.id());
    }

    m_split.setAction(action);
    m_transaction.modifySplit(m_split);
    reloadEditWidgets(m_transaction);

    // now we refresh the list of accounts available with this setting
    if(m_transactionFormActive) {

    } else {
      if(m_split.action() == MyMoneySplit::ActionTransfer) {
        m_editCategory->loadList(static_cast<KMyMoneyUtils::categoryTypeE> (KMyMoneyUtils::asset | KMyMoneyUtils::liability));
      } else {
        m_editCategory->loadList(static_cast<KMyMoneyUtils::categoryTypeE> (KMyMoneyUtils::income | KMyMoneyUtils::expense));
      }
    }

  } catch(MyMoneyException *e) {
    KMessageBox::detailedSorry(0, i18n("Unable to modify type"),
        (e->what() + " " + i18n("thrown in") + " " + e->file()+ ":%1").arg(e->line()));
    delete e;
    if(m_editType)
      m_editType->resetCurrentItem();

    m_transaction = t;
    m_split = s;
  }
}

void KLedgerView::slotShowTransactionForm(bool visible)
{
/*
  // if the setting is different, don't forget to update
  // the configuration
  if(m_transactionFormActive != visible) {
    KConfig *config = KGlobal::config();
    config->setGroup("General Options");
    config->writeEntry("TransactionForm", visible);
  }
*/
  m_transactionFormActive = visible;

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
    if(visible) {
      m_register->setLedgerLens(m_ledgerLens);
    } else {
      m_register->setLedgerLens(true);
    }

    // force update of row count because the lens setting might have changed
    m_register->setTransactionCount(m_transactionPtrVector.size()+1);

    // inform widget, if inline editing should be available or not
    // m_register->setInlineEditingMode(!visible);

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
  m_register->repaintContents(false);
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

const QString KLedgerView::action2str(const QCString &action, const bool showHotkey) const
{
  QString rc;

  if(action == MyMoneySplit::ActionCheck)
    rc = i18n("&Check");
  else if(action == MyMoneySplit::ActionDeposit)
    rc = i18n("&Deposit");
  else if(action == MyMoneySplit::ActionTransfer)
    rc = i18n("&Transfer");
  else if(action == MyMoneySplit::ActionWithdrawal)
    rc = i18n("&Withdrawal");
  else if(action == MyMoneySplit::ActionATM)
    rc = i18n("AT&M");

  if(rc.isEmpty()) {
    qDebug("Unsupported action string %s, set to check", static_cast<const char *>(action));
    rc = i18n("&Check");
  }

  if(showHotkey == false) {
  }
  return rc;
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

const QCString KLedgerView::transactionType(int type) const
{
  switch(type) {
    default:
      qWarning("Unknown transaction type used in KLedgerView::transactionType(int)");
      // Tricky fall through here!

    case 0: // Check
      return MyMoneySplit::ActionCheck;

    case 1: // Deposit
      return MyMoneySplit::ActionDeposit;

    case 2: // Transfer
      return MyMoneySplit::ActionTransfer;

    case 3: // Withdrawal
      return MyMoneySplit::ActionWithdrawal;

    case 4: // ATM
      return MyMoneySplit::ActionATM;
  }
}

void KLedgerView::slotNew(void)
{
  // this is not available when we have no account
  if(m_account.id().isEmpty())
    return;

  // select the very last line (empty one), and load it into the form
  m_register->setCurrentTransactionIndex(m_transactionList.count());
  m_register->ensureTransactionVisible();
  m_register->repaintContents(false);
  fillForm();
  fillSummary();

  m_form->enterButton()->setEnabled(true);
  m_form->cancelButton()->setEnabled(true);
  m_form->moreButton()->setEnabled(true);
  m_form->editButton()->setEnabled(false);
  m_form->newButton()->setEnabled(false);

  showWidgets();

  disconnect(m_register, SIGNAL(signalEnter()), this, SLOT(slotStartEdit()));
  if(!m_transactionFormActive)
    m_register->setInlineEditingMode(true);
}

void KLedgerView::slotStartEdit(void)
{
  // this is not available when we have no account
  if(m_account.id().isEmpty())
    return;
    
  // we use the warnlevel to keep track, if we have to warn the
  // user that some or all splits have been reconciled or if the
  // user cannot modify the transaction if at least one split
  // has the status frozen. The following value are used:
  //
  // 0 - no sweat, user can modify
  // 1 - user should be warned that at least one split has been reconciled
  //     already
  // 2 - user will be informed, that this transaction cannot be changed anymore

  int warnLevel = 0;

  QValueList<MyMoneySplit>::ConstIterator it;

  for(it = m_transaction.splits().begin(); it != m_transaction.splits().end(); ++it) {
    if((*it).reconcileFlag() == MyMoneySplit::Frozen)
      warnLevel = 2;
    if((*it).reconcileFlag() == MyMoneySplit::Reconciled && warnLevel < 1)
      warnLevel = 1;
  }

  switch(warnLevel) {
    case 0:
      break;

    case 1:
      if(KMessageBox::warningContinueCancel(0,
        i18n(
          "At least one split of this transaction has been reconciled. "
          "Do you wish to continue to edit the transaction anyway?"
        ),
        i18n("Transaction already reconciled")) == KMessageBox::Cancel) {

        warnLevel = 2;
      }
      break;

    case 2:
      KMessageBox::sorry(0,
            i18n("At least one split of this transaction has been frozen. "
                 "Editing this transaction is therefore prohibited."),
            i18n("Transaction already frozen"));
      break;
  }

  if(warnLevel == 2)
    return;

  m_register->ensureTransactionVisible();

  m_form->newButton()->setEnabled(false);
  m_form->enterButton()->setEnabled(true);
  m_form->cancelButton()->setEnabled(true);
  m_form->moreButton()->setEnabled(true);
  m_form->editButton()->setEnabled(false);

  showWidgets();

  disconnect(m_register, SIGNAL(signalEnter()), this, SLOT(slotStartEdit()));
  if(!m_transactionFormActive)
    m_register->setInlineEditingMode(true);
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

  connect(m_register, SIGNAL(signalEnter()), this, SLOT(slotStartEdit()));
  m_register->setInlineEditingMode(false);
  m_register->setFocus();
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
    try {
      MyMoneyFile* file = MyMoneyFile::instance();

      // If there are any differences, we need to update the storage.
      // All splits with no account id will be removed here. These splits
      // are refused by the engine, so it's better to discard them before.
      // Then we check for the following things and warn the user if we
      // find a mismatch:
      //
      // a) transaction should have 2 or more splits
      // b) the sum of all split amounts should be zero

      QValueList<MyMoneySplit> list = m_transaction.splits();
      QValueList<MyMoneySplit>::Iterator it;

      // Fix the payeeId. For non-asset and non-liability accounts,
      // the payeeId will be cleared. If a transfer has one split
      // with an empty payeeId the other one will be copied.
      //
      // Splits not referencing any account will be removed.

      QCString payeeId = "";
      for(it = list.begin(); it != list.end(); ++it) {
        if((*it).accountId() == "") {
          m_transaction.removeSplit(*it);
          continue;
        }
        MyMoneyAccount acc = file->account((*it).accountId());
        MyMoneyAccount::accountTypeE accType = file->accountGroup(acc.accountType());
        switch(accType) {
          case MyMoneyAccount::Asset:
          case MyMoneyAccount::Liability:
            break;

          default:
            (*it).setPayeeId("");
            m_transaction.modifySplit(*it);
            break;
        }
        if((*it).action() == MyMoneySplit::ActionTransfer
        && payeeId == "" && (*it).payeeId() != "")
          payeeId = (*it).payeeId();
      }

      if(m_transaction.splitCount() == 2 && payeeId != "") {
        for(it = list.begin(); it != list.end(); ++it) {
          if((*it).action() == MyMoneySplit::ActionTransfer
          && (*it).payeeId() == "") {
            (*it).setPayeeId(payeeId);
            m_transaction.modifySplit(*it);
          }
        }
      }


      if(m_transaction.splitCount() < 2) {
        qDebug("Transaction has less than 2 splits");
      }
      if(m_transaction.splitSum() != 0) {
        qDebug("Splits of transaction do not sum up to 0");
      }
  
      // differentiate between add and modify
      QCString id;
      if(m_transactionPtr == 0) {
        // in the add case, we don't have an ID yet. So let's get one
        // and use it down the line
        if(!m_transaction.postDate().isValid())
          m_transaction.setPostDate(QDate::currentDate());
        file->addTransaction(m_transaction);
        id = m_transaction.id();
      } else {
        // in the modify case, we have to keep the id. The call to
        // modifyTransaction might change m_transaction due to some
        // callbacks.
        id = m_transaction.id();
        file->modifyTransaction(m_transaction);
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

  connect(m_register, SIGNAL(signalEnter()), this, SLOT(slotStartEdit()));
  m_register->setInlineEditingMode(false);
  m_register->setFocus();
}

bool KLedgerView::focusNextPrevChild(bool next)
{
  bool  rc;

  if(m_editDate != 0) {
    QWidget *w = 0;
    QWidget *currentWidget;

    m_tabOrderWidgets.find(qApp->focusWidget());
    currentWidget = m_tabOrderWidgets.current();
    w = next ? m_tabOrderWidgets.next() : m_tabOrderWidgets.prev();

    do {
      if(!w)
        w = next ? m_tabOrderWidgets.first() : m_tabOrderWidgets.last();

      if(w != currentWidget
      && ((w->focusPolicy() & TabFocus) == TabFocus)
      && !w->focusProxy() && w->isVisible() && w->isEnabled()) {
        w->setFocus();
        rc = true;
        break;
      }
      w = next ? m_tabOrderWidgets.next() : m_tabOrderWidgets.prev();
    } while(w != currentWidget);

  } else
    rc = QWidget::focusNextPrevChild(next);

  return rc;
}

void KLedgerView::createInfoStack(void)
{
  if(m_infoStack != 0)
    delete m_infoStack;

  m_infoStack = new QWidgetStack(this, "InfoStack");
}

void KLedgerView::createContextMenu(void)
{
  if(m_register == 0)
    qFatal("KLedgerView::createContextMenu called before register was created!");

  KIconLoader *kiconloader = KGlobal::iconLoader();

  KPopupMenu* submenu = new KPopupMenu(this);
  submenu->insertItem(i18n("Not cleared"), this, SLOT(slotMarkNotReconciled()));
  submenu->insertItem(i18n("Cleared"), this, SLOT(slotMarkCleared()));
  submenu->insertItem(i18n("Reconciled"), this, SLOT(slotMarkReconciled()));

  m_contextMenu = new KPopupMenu(this);
  m_contextMenu->insertTitle(i18n("Transaction Options"));
  m_contextMenu->insertItem(kiconloader->loadIcon("edit", KIcon::Small), i18n("Edit ..."), this, SLOT(slotStartEdit()));
  m_contextMenu->insertSeparator();
  m_contextMenu->insertItem(i18n("Mark as ..."), submenu);
  m_contextMenu->insertItem(i18n("Move to account ..."), this, SLOT(slotMoveToAccount()));
  m_contextMenu->insertSeparator();

  m_contextMenu->insertItem(kiconloader->loadIcon("delete", KIcon::Small),
                        i18n("Delete transaction ..."),
                        this, SLOT(slotDeleteTransaction()));

  m_sortMenu = new KPopupMenu(this);

  m_sortMenu->insertTitle(i18n("Select sort order"));
  m_sortMenu->insertItem(i18n("Post date"), KTransactionPtrVector::SortPostDate);
  m_sortMenu->insertItem(i18n("Entry date"), KTransactionPtrVector::SortEntryDate);
  m_sortMenu->insertSeparator();
  m_sortMenu->insertItem(i18n("Type, number"), KTransactionPtrVector::SortTypeNr);
  m_sortMenu->insertItem(i18n("Number"), KTransactionPtrVector::SortNr);
  m_sortMenu->insertItem(i18n("Receiver"), KTransactionPtrVector::SortReceiver);
  m_sortMenu->insertItem(i18n("Value"), KTransactionPtrVector::SortValue);

  m_sortMenu->setItemChecked(m_transactionPtrVector.sortType(), true);

  KContextMenuManager::insert(m_register->horizontalHeader(), m_sortMenu);

  connect(m_sortMenu, SIGNAL(activated(int)), this, SLOT(slotSortOrderChanged(int)));
}

void KLedgerView::createMoreMenu(void)
{
  if(m_register == 0)
    qFatal("KLedgerView::createMoreMenu called before register was created!");

  KIconLoader *kiconloader = KGlobal::iconLoader();

  KPopupMenu* submenu = new KPopupMenu(this);
  submenu->insertItem(i18n("Not cleared"), this, SLOT(slotMarkNotReconciled()));
  submenu->insertItem(i18n("Cleared"), this, SLOT(slotMarkCleared()));
  submenu->insertItem(i18n("Reconciled"), this, SLOT(slotMarkReconciled()));

  m_moreMenu = new KPopupMenu(this);
  m_moreMenu->insertTitle(i18n("Transaction Options"));
  m_moreMenu->insertSeparator();
  m_moreMenu->insertItem(i18n("Mark as ..."), submenu);
  m_moreMenu->insertItem(i18n("Move to account ..."), this, SLOT(slotMoveToAccount()));
  m_moreMenu->insertSeparator();

  m_moreMenu->insertItem(kiconloader->loadIcon("delete", KIcon::Small),
                        i18n("Delete transaction ..."),
                        this, SLOT(slotDeleteTransaction()));
}

void KLedgerView::markSplit(MyMoneySplit::reconcileFlagE flag)
{
  if(m_transactionPtr != 0) {
    try {
      m_split.setReconcileFlag(flag);
      if(flag == MyMoneySplit::Reconciled)
        m_split.setReconcileDate(QDate::currentDate());

      m_transaction.modifySplit(m_split);
      if(!isEditMode())
        MyMoneyFile::instance()->modifyTransaction(m_transaction);
    } catch (MyMoneyException *e) {
      KMessageBox::detailedSorry(0, i18n("Unable to mark split"),
        (e->what() + " " + i18n("thrown in") + " " + e->file()+ ":%1").arg(e->line()));
      delete e;
    }
  }
}

void KLedgerView::slotMarkNotReconciled(void)
{
  markSplit(MyMoneySplit::NotReconciled);
}

void KLedgerView::slotMarkCleared(void)
{
  markSplit(MyMoneySplit::Cleared);
}

void KLedgerView::slotMarkReconciled(void)
{
  markSplit(MyMoneySplit::Reconciled);
}

void KLedgerView::slotMoveToAccount(void)
{
  KMessageBox::information(0,i18n("Moving a split to a different account is not yet implemented"), "Function not implemented");
}

void KLedgerView::slotDeleteTransaction(void)
{
  int answer;
  if(m_transactionPtr != 0) {
    answer = KMessageBox::warningContinueCancel (NULL,
       i18n("You are about to delete the selected transaction. "
            "Do you really want to continue?"),
       i18n("Delete transaction"),
       i18n("Continue")
       );
    if(answer == KMessageBox::Continue) {
      slotCancelEdit();
      try {
        MyMoneyFile::instance()->removeTransaction(m_transaction);
      } catch(MyMoneyException *e) {
        KMessageBox::detailedSorry(0, i18n("Unable to remove transaction"),
          (e->what() + " " + i18n("thrown in") + " " + e->file()+ ":%1").arg(e->line()));
        delete e;
      }
    }
  }
}

void KLedgerView::slotGotoOtherSideOfTransfer(void)
{
  MyMoneySplit split = m_transaction.split(m_account.id(), false);

  emit accountAndTransactionSelected(split.accountId(), m_transaction.id());
}

void KLedgerView::hide(void)
{
  slotCancelEdit();
  QWidget::hide();
}

void KLedgerView::slotRegisterHeaderClicked(int col)
{
}

void KLedgerView::slotSortOrderChanged(int order)
{
  slotCancelEdit();

  QCString id = m_transaction.id();

  m_sortMenu->setItemChecked(m_transactionPtrVector.sortType(), false);
  m_transactionPtrVector.setSortType(static_cast<KTransactionPtrVector::TransactionSortE> (order));
  m_sortMenu->setItemChecked(m_transactionPtrVector.sortType(), true);

  // make sure the transaction stays selected. It's position might
  // have changed within the register while re-sorting
  selectTransaction(id);
  updateView();
}

const bool KLedgerView::isEditMode(void) const
{
  return m_editDate != 0 && m_editDate->isVisible();
}
