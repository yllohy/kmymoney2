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

#include <unistd.h>

// ----------------------------------------------------------------------------
// QT Includes

#include <qdatetime.h>
#include <qapplication.h>
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
#include "../dialogs/ieditscheduledialog.h"
#include "../dialogs/knewaccountdlg.h"

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
  MyMoneyMoney tmp;

  try {
    MyMoneySplit s1;
    MyMoneySplit s2;
    switch(m_idMode) {
      case AccountMode:
        s1 = t1->splitByAccount(m_id);
        s2 = t2->splitByAccount(m_id);
        break;
      case PayeeMode:
        s1 = t1->splitByPayee(m_id);
        s2 = t2->splitByPayee(m_id);
        break;
    }
    QString p1, p2;

    switch(m_sortType) {
      case SortValue:
        rc = 1;
        tmp = s2.value() - s1.value();
        if(tmp == 0) {
          // same value? Sort by date
          rc = t2->postDate().daysTo(t1->postDate());
          if(rc == 0) {
            // same date? Sort by id
            rc = compareItems(t1->id(), t2->id());
          }
        } else if(tmp < 0) {
          rc = -1;
        }
        break;

      case SortEntryDate:
        rc = t2->entryDate().daysTo(t1->entryDate());
        if(rc == 0) {
          // same date? Sort by value
          rc = 1;
          tmp = s2.value() - s1.value();
          if(tmp == 0) {
            // same value? sort by id
            rc = compareItems(t1->id(), t2->id());
          } else if(tmp < 0) {
            rc = -1;
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
              rc = 1;
              tmp = s2.value() - s1.value();
              if(tmp == 0) {
                // same value? sort by id
                rc = compareItems(t1->id(), t2->id());
              } else if(tmp < 0) {
                rc = -1;
              }
            }
          }
        }
        break;

      case SortReceiver:
        if(!s2.payeeId().isEmpty()) {
          p2 = MyMoneyFile::instance()->payee(s2.payeeId()).name();
        }
        if(!s1.payeeId().isEmpty()) {
          p1 = MyMoneyFile::instance()->payee(s1.payeeId()).name();
        }

        rc = compareItems(p1, p2);

        if(rc == 0) {
          // same payee? Sort by date
          rc = t2->postDate().daysTo(t1->postDate());
          if(rc == 0) {
            // same date? Sort by value
            rc = 1;
            tmp = s2.value() - s1.value();
            if(tmp == 0) {
              // same value? sort by id
              rc = compareItems(t1->id(), t2->id());
            } else if(tmp < 0) {
              rc = -1;
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
            rc = 1;
            tmp = s2.value() - s1.value();
            if(tmp == 0) {
              // same value? sort by id
              rc = compareItems(t1->id(), t2->id());
            } else if(tmp < 0) {
              rc = -1;
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
          rc = 1;
          tmp = s2.value() - s1.value();
          if(tmp == 0) {
            // same value? Sort by id
            rc = compareItems(t1->id(), t2->id());
          } else if(tmp < 0) {
            rc = -1;
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
  m_lastPostDate(QDate::currentDate()),
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
  MyMoneyFile::instance()->attach(MyMoneyFile::NotifyClassAccountHierarchy, this);
  
  connect(&m_blinkTimer, SIGNAL(timeout()), SLOT(slotBlinkTimeout()));
}

KLedgerView::~KLedgerView()
{
  if(m_infoStack != 0)
    delete m_infoStack;

  MyMoneyFile::instance()->detach(MyMoneyFile::NotifyClassAccountHierarchy, this);
  
  // the following observer could have been attached by slotSelectAccount(),
  // so we better get rid of him here
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

void KLedgerView::slotSelectAccount(const QCString& accountId)
{
  slotCancelEdit();
  
  MyMoneyFile* file = MyMoneyFile::instance();

  if(!accountId.isEmpty()) {
    if(accountId != m_account.id()) {

      if(!m_account.id().isEmpty())
        file->detach(m_account.id(), this);

      try {
        m_account = file->account(accountId);
        file->attach(m_account.id(), this);
        // force initial load
        m_transactionPtr = 0;
        // loadAccount();
        enableWidgets(true);
      } catch(MyMoneyException *e) {
        qDebug("Unexpected exception in KLedgerView::setCurrentAccount");
        delete e;
      }
    }
  } else {
    if(!m_account.id().isEmpty())
      file->detach(m_account.id(), this);
    m_transactionList.clear();
    m_transactionPtrVector.clear();
    m_transactionPtr = 0;
    m_account = MyMoneyAccount();
    m_register->setTransactionCount(1);
    fillForm();
    fillSummary();
    enableWidgets(false);
  }
  refreshView();
}

void KLedgerView::refreshView(void)
{
  // read in the configuration parameters for this view
  KConfig *config = KGlobal::config();
  config->setGroup("General Options");
  m_transactionFormActive = config->readBoolEntry("TransactionForm", true);
  refreshView(m_transactionFormActive);  
}

void KLedgerView::refreshView(const bool transactionFormVisible)
{
  // if we're currently editing a transaction, we don't refresh the view
  // this will screw us, if someone creates a category on the fly, as this
  // will come here when the notifications by the engine are send out.
  if(isEditMode())
    return;

  m_transactionFormActive = transactionFormVisible;
      
  // if a transaction is currently selected, keep the id
  QCString transactionId;
  if(m_transactionPtr != 0)
    transactionId = m_transactionPtr->id();
  m_transactionPtr = 0;
  m_transactionPtrVector.clear();
    
  // read in the configuration parameters for this view
  KConfig *config = KGlobal::config();
  config->setGroup("General Options");
  m_ledgerLens = config->readBoolEntry("LedgerLens", true);

  config->setGroup("List Options");
  QDateTime defaultDate;
  m_dateStart = config->readDateTimeEntry("StartDate", &defaultDate).date();

  m_register->readConfig();

  // in case someone changed the account info and we are called here
  // via the observer's update function, we just reload ourselves.
  MyMoneyFile* file = MyMoneyFile::instance();
  try {
    m_account = file->account(m_account.id());
    // setup the filter to select the transactions we want to display
    MyMoneyTransactionFilter filter(m_account.id());

    if(m_inReconciliation == true) {
      filter.addState(MyMoneyTransactionFilter::notReconciled);
      filter.addState(MyMoneyTransactionFilter::cleared);
    } else
      filter.setDateFilter(m_dateStart, QDate());

    // get the list of transactions
    QValueList<MyMoneyTransaction> list = file->transactionList(filter);
    QValueList<MyMoneyTransaction>::ConstIterator it;
    m_transactionList.clear();
    for(it = list.begin(); it != list.end(); ++it) {
      KMyMoneyTransaction k(*it);
      k.setSplitId((*it).splitByAccount(accountId()).id());
      m_transactionList.append(k);
    }
    
  } catch(MyMoneyException *e) {
    delete e;
    m_account = MyMoneyAccount();
    m_transactionList.clear();
  }

  updateView(transactionId);
}

void KLedgerView::updateView(const QCString& transactionId)
{
  //qDebug("KLedgerView::updateView()");

  // setup the transaction pointer array and the balance information
  setupPointerAndBalanceArrays();

  // set the number of transactions plus one for new entries
  //m_register->setTransactionCount(m_transactionList.count()+1);

  // show transaction form if selected
  slotShowTransactionForm(m_transactionFormActive);

  // select the last selected transaction or the one beyond the last one
  selectTransaction(transactionId);
}

void KLedgerView::setupPointerAndBalanceArrays(void)
{
  int   i;

  QValueList<KMyMoneyTransaction>::ConstIterator it_t;

  // setup the pointer vector
  m_transactionPtrVector.clear();
  m_transactionPtrVector.resize(m_transactionList.size());
  m_transactionPtrVector.setAccountId(m_account.id());

  for(i = 0, it_t = m_transactionList.begin(); it_t != m_transactionList.end(); ++it_t) {
    m_transactionPtrVector.insert(i, &(*it_t));
    ++i;
  }

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
      balance -= m_transactionPtrVector[i]->splitByAccount(accountId()).value();
      if(m_transactionPtrVector.sortType() == KTransactionPtrVector::SortPostDate) {
        if(m_transactionPtrVector[i]->postDate() > QDate::currentDate()) {
          m_register->setCurrentDateIndex(i+1);

        } else if(dateMarkPlaced == false) {
          m_register->setCurrentDateIndex(i+1);
          dateMarkPlaced = true;
        }
      }
    }
    // if the current date mark is not set, all transactions (if any) are in the
    // future and we can safely set the date mark to the very first slot
    if(dateMarkPlaced == false)
      m_register->setCurrentDateIndex(0);
      
  } catch(MyMoneyException *e) {
    if(!accountId().isEmpty())
      qDebug("Unexpected exception in KLedgerView::setupPointerAndBalanceArrays");
    delete e;
  }
}

void KLedgerView::enableWidgets(const bool enable)
{
  m_form->setEnabled(enable);
}

void KLedgerView::update(const QCString& /* accountId */)
{
  if(m_suspendUpdate == false) {
    try {
      refreshView();
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
    if(!m_account.id().isEmpty())
      update(QCString());
    
  } else
    m_suspendUpdate = suspend;
}

KMyMoneyTransaction* KLedgerView::transaction(const int idx) const
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

void KLedgerView::slotRegisterClicked(int row, int /* col */, int button, const QPoint& /* mousePos */)
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
  int   idx = -1;

  // qDebug("KLedgerView::selectTransaction(%s)", transactionId.data());
  if(!transactionId.isEmpty()) {
    for(unsigned i = 0; i < m_transactionPtrVector.count(); ++i) {
      if(m_transactionPtrVector[i]->id() == transactionId) {
        idx = i;
        break;
      }
    }
  } else {
    if(m_transactionPtrVector.count() > 0) {
      idx = m_transactionPtrVector.count()-1;
    }
  }

  if(idx != -1) {
    // qDebug("KLedgerView::selectTransaction index is %d", idx);
    m_transactionPtr = m_transactionPtrVector[idx];
    m_register->setCurrentTransactionIndex(idx);
    m_register->ensureTransactionVisible();
    m_register->repaintContents(false);
    fillForm();
    fillSummary();
    emit transactionSelected();
    rc = true;
  } else {
    fillForm();
    fillSummary();
  }
  return rc;
}

void KLedgerView::slotPayeeChanged(const QString& name)
{
  if(!m_editPayee)
    return;

  createSecondSplit();
  
  MyMoneySplit sp;
  if(m_transaction.splitCount() == 2) {
    sp = m_transaction.splitByAccount(m_account.id(), false);
  }

  if(!name.isEmpty()) {
    MyMoneyPayee payee;
    try {

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
    m_split.setPayeeId(QCString());
    // for tranfers, we always modify the other side as well
    if(m_split.action() == MyMoneySplit::ActionTransfer) {
      sp.setPayeeId(QCString());
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
      MyMoneySplit sp = m_transaction.splitByAccount(m_account.id(), false);
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
      MyMoneySplit split = m_transaction.splitByAccount(accountId(), false);
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
    m_editDeposit->loadText(QString());

    if(m_split.action() == MyMoneySplit::ActionDeposit)
      m_split.setAction(MyMoneySplit::ActionWithdrawal);

    m_transaction.modifySplit(m_split);
    if(m_transaction.splitCount() == 2) {
      MyMoneySplit split = m_transaction.splitByAccount(accountId(), false);
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
    m_editPayment->loadText(QString());

    if(m_split.action() != MyMoneySplit::ActionDeposit
    && m_split.action() != MyMoneySplit::ActionTransfer)
      m_split.setAction(MyMoneySplit::ActionDeposit);

    m_transaction.modifySplit(m_split);
    if(m_transaction.splitCount() == 2) {
      MyMoneySplit split = m_transaction.splitByAccount(accountId(), false);
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
  QString cat(category);
  
  // usually we call createSecondSplit() here in all the other xxxChanged()
  // functions, but we don't do it for slotCategoryChanged() because we will
  // remove it later on anyway.

  try {
    // First, we check if the category/account exists
    QCString id;
    switch(transactionType(m_split)) {
      case Transfer:
        id = MyMoneyFile::instance()->nameToAccount(category);
        if(id.isEmpty() && !category.isEmpty()) {
          KMessageBox::sorry(0, i18n("The account \"%1\" does not exist and direct creation of new account not yet implemented").arg(cat));
          return;
        }
        break;
       
      default:
        id = MyMoneyFile::instance()->categoryToAccount(category);
        if(id.isEmpty() && !category.isEmpty()) {
          // FIXME:
          /// Todo: Add account (hierarchy) upon new category
          if(KMessageBox::questionYesNo(0,
                i18n("The category \"%1\" currently does not exist. "
                     "Do you want to create it?").arg(category)) == KMessageBox::Yes) {
            MyMoneyAccount acc;
            int rc;
            acc.setName(category);

            KNewAccountDlg dlg(acc, false, true);
            rc = dlg.exec();
            if(rc == QDialog::Accepted) {
              try {
                MyMoneyAccount parentAccount;
                acc = dlg.account();
                parentAccount = dlg.parentAccount();
                MyMoneyFile::instance()->addAccount(acc, parentAccount);
                id = acc.id();
                cat = MyMoneyFile::instance()->accountToCategory(id);
              } catch(MyMoneyException *e) {
                KMessageBox::detailedSorry(0, i18n("Unable to add category"),
                    (e->what() + " " + i18n("thrown in") + " " + e->file()+ ":%1").arg(e->line()));
                delete e;
                rc = QDialog::Rejected;
              }
            }

            if(rc != QDialog::Accepted) {
              m_editCategory->resetText();
              m_editCategory->setFocus();
              return;
            }

          } else {
            m_editCategory->resetText();
            m_editCategory->setFocus();
            return;
          }
          break;
      }
    }

    if(cat.isEmpty()) {
      if(m_transaction.splitCount() == 2) {
        MyMoneySplit sp = m_transaction.splitByAccount(m_account.id(), false);
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
          split = m_transaction.splitByAccount(accountId(), false);
          split.setAccountId(id);
          // make sure the values match
          split.setValue(-m_split.value());
          m_transaction.modifySplit(split);
          break;
      }
    }
    m_editCategory->loadText(cat);

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
    if(id.isEmpty()) {
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
            split = m_transaction.splitByAccount(m_account.id(), false);
            m_transaction.removeSplit(split);
          }
          qDebug("split count is %d", m_transaction.splitCount());
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
      m_split = m_transaction.splitByAccount(m_account.id());
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
  m_transactionFormActive = visible;

  if(m_form != 0) {
    if(visible) {
      // make sure, that any pending edit activity is cancelled
      // when the transaction form is enabled. This could happen
      // during reconciliation.
      slotCancelEdit();
      
      // block signals here to avoid running into the NEW case
      // which is triggered by the signal tabBar()->selected(int)
      m_form->tabBar()->blockSignals(true);
      m_form->show();
      m_form->tabBar()->blockSignals(false);
    } else
      m_form->hide();
  }

  if(m_register != 0) {
    // bool lensSetting = m_register->ledgerLens();
    // unsigned int count = m_register->transactionCount();
    
    if(visible) {
      m_register->setLedgerLens(m_ledgerLens);
    } else {
      m_register->setLedgerLens(true);
    }

    // force update of row count because the lens setting might have changed
    m_register->setTransactionCount(m_transactionPtrVector.size()+1, false);

    // inform widget, if inline editing should be available or not
    // m_register->setInlineEditingMode(!visible);

    // make sure, full transaction is visible
    resizeEvent(NULL);

    // using a timeout is the only way, I got the 'ensureTransactionVisible'
    // working when coming from hidden form to visible form. I assume, this
    // has something to do with the delayed update of the display somehow.
    QTimer::singleShot(10, this, SLOT(timerDone()));
  }
}

void KLedgerView::timerDone(void)
{
  m_register->ensureTransactionVisible();
  m_register->repaintContents(false);
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
    rc = i18n("&Cheque");
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
  KMyMoneyUtils::newPayee(this, m_editPayee, payeeName);
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
  if(split.action() == MyMoneySplit::ActionAmortization) {
    if(m_account.accountType() == MyMoneyAccount::Loan)
      return Deposit;
    return Withdrawal;
  }
  qDebug("Unknown transaction type in KLedgerView::transactionType, Check assumed");
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

  if(m_editDate->getQDate().isValid())
    m_transaction.setPostDate(m_editDate->getQDate());

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
  fillForm();

  connect(m_register, SIGNAL(signalEnter()), this, SLOT(slotStartEdit()));
  m_register->setInlineEditingMode(false);
  m_register->setFocus();
}

void KLedgerView::slotEndEdit(void)
{
  // force focus change to update all data
  m_form->enterButton()->setFocus();

  // switch the context to enable refreshView() to work
  m_form->newButton()->setEnabled(true);
  m_form->enterButton()->setEnabled(false);
  m_form->cancelButton()->setEnabled(false);
  m_form->moreButton()->setEnabled(false);

  if(transaction(m_register->currentTransactionIndex()) != 0) {
    m_form->editButton()->setEnabled(true);
  }

  hideWidgets();

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

      QCString payeeId;
      for(it = list.begin(); it != list.end(); ++it) {
        if((*it).accountId().isEmpty()) {
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
            (*it).setPayeeId(QCString());
            m_transaction.modifySplit(*it);
            break;
        }
        if((*it).action() == MyMoneySplit::ActionTransfer
        && payeeId.isEmpty() && !(*it).payeeId().isEmpty())
          payeeId = (*it).payeeId();
      }

      if(m_transaction.splitCount() == 2 && !payeeId.isEmpty()) {
        for(it = list.begin(); it != list.end(); ++it) {
          if((*it).action() == MyMoneySplit::ActionTransfer
          && (*it).payeeId().isEmpty()) {
            (*it).setPayeeId(payeeId);
            m_transaction.modifySplit(*it);
          }
        }
      }

      // check if this is a transfer to/from loan account and
      // mark it as amortization in this case
      if(m_transaction.splitCount() == 2) {
        bool isAmortization = false;
        for(it = list.begin(); !isAmortization && it != list.end(); ++it) {
          if((*it).action() == MyMoneySplit::ActionTransfer) {
            MyMoneyAccount acc = file->account((*it).accountId());
            if(acc.accountType() == MyMoneyAccount::Loan
            || acc.accountType() == MyMoneyAccount::AssetLoan)
              isAmortization = true;
          }
        }

        if(isAmortization) {
          for(it = list.begin(); it != list.end(); ++it) {
            (*it).setAction(MyMoneySplit::ActionAmortization);
            m_transaction.modifySplit(*it);
          }          
        }
      }
            
      // differentiate between add and modify
      QCString id;
      if(m_transactionPtr == 0) {
        // in the add case, we don't have an ID yet. So let's get one
        // and use it down the line
        if(!m_transaction.postDate().isValid())
          m_transaction.setPostDate(QDate::currentDate());
        // remember date for next new transaction
        m_lastPostDate = m_transaction.postDate();

        // From here on, we need to use a local copy of the transaction
        // because m_transaction will be reassigned during the update
        // once the transaction has been entered into the engine. If this
        // happens, we have no idea about the id of the new transaction.
        MyMoneyTransaction t = m_transaction;
        file->addTransaction(t);
        id = t.id();
        
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

  connect(m_register, SIGNAL(signalEnter()), this, SLOT(slotStartEdit()));
  m_register->setInlineEditingMode(false);
  m_register->setFocus();
}

bool KLedgerView::focusNextPrevChild(bool next)
{
  bool  rc = false;

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
        unsigned int idx = m_register->currentTransactionIndex();
        
        MyMoneyFile::instance()->removeTransaction(m_transaction);
        
        if(m_transactionPtrVector.count() > 0) {
          if(idx >= m_transactionPtrVector.count())
            idx = m_transactionPtrVector.count()-1;
          m_transactionPtr = m_transactionPtrVector[idx];
          selectTransaction(m_transactionPtr->id());
        }
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
  MyMoneySplit split = m_transaction.splitByAccount(m_account.id(), false);

  emit accountAndTransactionSelected(split.accountId(), m_transaction.id());
}

void KLedgerView::hide(void)
{
  slotCancelEdit();
  QWidget::hide();
}

void KLedgerView::slotRegisterHeaderClicked(int /* col */)
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
  updateView(id);
}

const bool KLedgerView::isEditMode(void) const
{
  return m_editDate != 0 && m_editDate->isVisible();
}

void KLedgerView::show()
{
  // make sure, we have a transaction selected if at least one is available
  if(m_transactionPtr == 0 && m_transactionPtrVector.count() > 0) {
    m_register->setCurrentTransactionIndex(m_transactionPtrVector.count()-1);
  }
  
  // make sure, the QTabbar does not send out it's selected() signal
  // which would drive us crazy here. fillForm calls slotTypeSelected()
  // later on anyway.
  if(m_form)
    m_form->tabBar()->blockSignals(true);
  QWidget::show();
  if(m_form)
    m_form->tabBar()->blockSignals(false);

  fillForm();
  resizeEvent(NULL);
}

void KLedgerView::slotCreateSchedule(void)
{
  if (!m_transaction.id().isEmpty()) {
    MyMoneySchedule schedule;
    MyMoneySchedule::typeE scheduleType = MyMoneySchedule::TYPE_ANY;
    
    if(m_account.accountGroup() == MyMoneyAccount::Asset) {
      if(m_split.value() >= 0)
        scheduleType = MyMoneySchedule::TYPE_DEPOSIT;
      else
        scheduleType = MyMoneySchedule::TYPE_BILL;
    } else {
      if(m_split.value() >= 0)
        scheduleType = MyMoneySchedule::TYPE_BILL;
      else
        scheduleType = MyMoneySchedule::TYPE_DEPOSIT;
    }
    if(m_transaction.splitCount() == 2
    && m_split.action() == MyMoneySplit::ActionTransfer)
      scheduleType = MyMoneySchedule::TYPE_TRANSFER;

    // create a copy and reset all flags
    MyMoneyTransaction t;
    try {
      t = m_transaction;
      QValueList<MyMoneySplit> list = t.splits();
      QValueList<MyMoneySplit>::ConstIterator it_s;
      for(it_s = list.begin(); it_s != list.end(); ++it_s) {
        MyMoneySplit s = *it_s;
        s.setReconcileDate(QDate());
        s.setReconcileFlag(MyMoneySplit::NotReconciled);
        t.modifySplit(s);
      }
    } catch (MyMoneyException *e) {
      qDebug("Unable to reset flags in %s(%d):'%s'", __FILE__, __LINE__, e->what().latin1());
      delete e;
    }
    schedule.setTransaction(t);

    KEditScheduleDialog *m_keditscheddlg = new KEditScheduleDialog(
      m_transaction.splitByAccount(m_account.id()).action(),
      schedule, this);
      
    if (m_keditscheddlg->exec() == QDialog::Accepted) {
      schedule = m_keditscheddlg->schedule();
      try {
        MyMoneyFile::instance()->addSchedule(schedule);
        
      } catch (MyMoneyException *e) {
        KMessageBox::detailedError(this, i18n("Unable to add schedule: "), e->what());
        delete e;
      }
    }

    delete m_keditscheddlg;
  }
}

const bool KLedgerView::transfersPossible(void) const
{
  QValueList<MyMoneyAccount> list = MyMoneyFile::instance()->accountList();
  QValueList<MyMoneyAccount>::ConstIterator it_a;
  int cnt = 0;
  
  for(it_a = list.begin(); cnt < 2 && it_a != list.end(); ++it_a) {
    if((*it_a).accountGroup() == MyMoneyAccount::Asset
    || (*it_a).accountGroup() == MyMoneyAccount::Liability)
      ++cnt;
  }
  return (cnt >= 2) ? true : false;
}
