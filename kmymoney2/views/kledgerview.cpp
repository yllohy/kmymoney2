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
#include "kdecompat.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <qdatetime.h>
#include <qapplication.h>
#include <qwidgetstack.h>
#include <qcursor.h>
#include <qtimer.h>
#include <qlayout.h>
#include <qpalette.h>
#include <qapplication.h>
#include <qwidgetlist.h>

#if QT_IS_VERSION(3,3,0)
#include <qeventloop.h>
#endif

// ----------------------------------------------------------------------------
// KDE Includes

#include <kmessagebox.h>
#include <kpushbutton.h>
#include <kconfig.h>
#include <kpopupmenu.h>
#include <kiconloader.h>
#include <kcmenumngr.h>
#include <kcompletionbox.h>

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
#include "../dialogs/kcurrencycalculator.h"

/* -------------------------------------------------------------------------------*/
/*                               KTransactionPtrVector                            */
/* -------------------------------------------------------------------------------*/

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
        if(tmp.isZero()) {
          // same value? Sort by date
          rc = t2->postDate().daysTo(t1->postDate());
          if(rc == 0) {
            // same date? Sort by id
            rc = compareItems(t1->id(), t2->id());
          }
        } else if(tmp.isNegative()) {
          rc = -1;
        }
        break;

      case SortEntryDate:
        rc = t2->entryDate().daysTo(t1->entryDate());
        if(rc == 0) {
          // same date? Sort by value
          rc = 1;
          tmp = s2.value() - s1.value();
          if(tmp.isZero()) {
            // same value? sort by id
            rc = compareItems(t1->id(), t2->id());
          } else if(tmp.isNegative()) {
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
              if(tmp.isZero()) {
                // same value? sort by id
                rc = compareItems(t1->id(), t2->id());
              } else if(tmp.isNegative()) {
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
            if(tmp.isZero()) {
              // same value? sort by id
              rc = compareItems(t1->id(), t2->id());
            } else if(tmp.isNegative()) {
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
            if(tmp.isZero()) {
              // same value? sort by id
              rc = compareItems(t1->id(), t2->id());
            } else if(tmp.isNegative()) {
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
          if(tmp.isZero()) {
            // same value? Sort by id
            rc = compareItems(t1->id(), t2->id());
          } else if(tmp.isNegative()) {
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


/* --------------------------------------------------------------*/
/*                           KLedgerView                         */
/* --------------------------------------------------------------*/

QDate KLedgerView::m_lastPostDate = QDate();

KLedgerView::KLedgerView(QWidget *parent, const char *name )
  : QWidget(parent,name),
  m_editMode(false),
  m_contextMenu(0),
  m_suspendUpdate(false)
{
  KConfig *config = KGlobal::config();
  config->setGroup("General Options");
  m_ledgerLens = config->readBoolEntry("LedgerLens", true);
  m_transactionFormActive = config->readBoolEntry("TransactionForm", true);

  if(!m_lastPostDate.isValid())
    m_lastPostDate = QDate::currentDate();

  m_register = 0;
  m_form = 0;
  m_transactionPtr = 0;

  m_inReconciliation = false;

  MyMoneyFile::instance()->attach(MyMoneyFile::NotifyClassAccountHierarchy, this);
}

KLedgerView::~KLedgerView()
{
  MyMoneyFile::instance()->detach(MyMoneyFile::NotifyClassAccountHierarchy, this);

  // the following observer could have been attached by slotSelectAccount(),
  // so we better get rid of him here
  MyMoneyFile::instance()->detach(m_account.id(), this);
}

void KLedgerView::createRegister(kMyMoneyRegister* r)
{
  Q_CHECK_PTR(r);

  m_register = r;
  m_register->setParent(this);
  m_register->installEventFilter(this);

  connect(m_register, SIGNAL(clicked(int, int, int, const QPoint&)), this, SLOT(slotRegisterClicked(int, int, int, const QPoint&)));
  connect(m_register, SIGNAL(doubleClicked(int, int, int, const QPoint&)), this, SLOT(slotRegisterDoubleClicked(int, int, int, const QPoint&)));

  connect(m_register, SIGNAL(signalNextTransaction()), this, SLOT(slotNextTransaction()));
  connect(m_register, SIGNAL(signalPreviousTransaction()), this, SLOT(slotPreviousTransaction()));
  connect(m_register, SIGNAL(signalDelete()), this, SLOT(slotDeleteTransaction()));
  connect(m_register, SIGNAL(signalSelectTransaction(const QCString&)), this, SLOT(selectTransaction(const QCString&)));

  connect(m_register->horizontalHeader(), SIGNAL(clicked(int)), this, SLOT(slotRegisterHeaderClicked(int)));
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

  if(!accountId().isEmpty()) {
    balance = MyMoneyFile::instance()->balance(accountId());
    // the trick is to go backwards ;-)

    while(--i >= 0) {
      m_balance[i] = balance;
      try {
        MyMoneySplit split = m_transactionPtrVector[i]->splitByAccount(accountId());
        balance -= split.value(m_transactionPtrVector[i]->commodity(), m_account.currencyId());
      } catch(MyMoneyException *e) {
        // for KLedgerViewInvestments this will fail, because there's no split with accountId()
        // in the transaction, only with the stock child
        if(!accountId().isEmpty() && !inherits("KLedgerViewInvestments"))
          qDebug("Unexpected exception in KLedgerView::setupPointerAndBalanceArrays: %s", e->what().latin1());
        delete e;
      }
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

    if(m_register->differentTransaction(row) == true) {
      cancelOrEndEdit();

      m_register->setCurrentTransactionRow(row);
      m_register->ensureTransactionVisible();
      m_register->repaintContents(false);

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
        cancelOrEndEdit();

        Q_CHECK_PTR(m_contextMenu);
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

      // we have a new payee assigned to this transaction.
      // in case there is no category assigned, no value entered and no
      // memo available, we search for the last transaction of this payee
      // in the account.
      if(m_transaction.splitCount() == 2) {
        KConfig* kconfig = KGlobal::config();
        kconfig->setGroup("General Options");
        if(sp.accountId().isEmpty()
        && m_split.memo().isEmpty()
        && m_split.value().isZero()
        && kconfig->readBoolEntry("AutoFillTransaction", false) == true) {
          MyMoneyTransactionFilter filter(m_account.id());
          filter.addPayee(payee.id());
          QValueList<MyMoneyTransaction> list = MyMoneyFile::instance()->transactionList(filter);
          if(!list.empty()) {
            // ok, we found a previous transaction. now we clear out
            // what we have collected so far and add those splits from
            // the previous transaction
            MyMoneyTransaction t = list.last();
            m_transaction.removeSplits();
            QValueList<MyMoneySplit>::ConstIterator it;
            for(it = t.splits().begin(); it != t.splits().end(); ++it) {
              MyMoneySplit s(*it);
              s.setReconcileFlag(MyMoneySplit::NotReconciled);
              s.setId(QCString());
              m_transaction.addSplit(s);
            }
            if(m_transaction.splitCount() == 2) {
              sp = m_transaction.splitByAccount(m_account.id(), false);
            }

            // update the UI
            reloadEditWidgets(m_transaction);

            m_editPayee->setFocus();
          }
        }
      }

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
  amountChanged(value, transactionDirection(m_split));
}

void KLedgerView::amountChanged(const QString& value, int dir)
{
  MyMoneyTransaction t;
  MyMoneySplit s;

  t = m_transaction;
  s = m_split;

  try {
    createSecondSplit();

    MyMoneyMoney val = MyMoneyMoney(value);
    // if someone enters a negative number, we have to make sure that
    // the action is corrected. For transfers, we don't have to do anything
    if(val.isNegative()) {
      switch(transactionDirection(s)) {
        case Credit:
        case UnknownDirection:
          dir = Debit;
          break;
        case Debit:
          dir = Credit;
          break;
      }
    } else if(dir == UnknownDirection) {
      dir = Credit;
    }

    val = val.abs();
    if(dir == Debit)
      val = -val;

    // if we edit a transaction, the previous price is of interest as
    // we need it to recalculate shares/values.
    MyMoneyMoney price = m_split.shares() / m_split.value();

    MyMoneySplit split = m_transaction.splitByAccount(accountId(), false);
    bool checkVat = false;
    if(!split.accountId().isEmpty()) {
      MyMoneyAccount vacc = MyMoneyFile::instance()->account(split.accountId());
      checkVat = split.value().isZero() && !vacc.value("VatAccount").isEmpty();
    }
    if(m_account.value("NoVat") == "Yes")
      checkVat = false;

    // set either shares or value depending on the currencies
    m_split.setValue(val, m_transaction.commodity(), m_account.currencyId());

    // if we use a different currency, then re-calculate the 'value' in
    // the transactions commodity based on the previous price but only
    // if we had a previous price.
    if(m_transaction.commodity() != m_account.currencyId() && !price.isZero())
      m_split.setValue(val / price);

    if(m_editAmount)
      m_editAmount->loadText(val.abs().formatMoney(""));

    m_transaction.modifySplit(m_split);

    // let's take care of the other half of the transaction
    if(m_transaction.splitCount() == 2) {
      // initialize in case we don't find it later on
      MyMoneyAccount acc;
      acc.setCurrencyId(m_transaction.commodity());

      // if the user enters the amount first w/o the category field
      // being filled, we cannot determine the 'other side' of the
      // transaction correctly. In this case, we assume a price of 0
      // slotEndEdit() must handle it then
      if(!split.accountId().isEmpty()) {
        acc = MyMoneyFile::instance()->account(split.accountId());
        // also keep track of a possible previous price
        price = (!split.shares().isZero()) ? split.value() / split.shares() : MyMoneyMoney(0);
      } else
        price = MyMoneyMoney(0);

      // in any case, we need to set the value to the negative value of
      // the first split to balance the transaction.
      split.setValue(-m_split.value());

      // if we use a different currency, then re-calculate the shares
      // in the accounts commodity/currency based on the previous price
      if(m_transaction.commodity() != acc.currencyId() && !price.isZero()) {
        split.setShares(split.value() / price);
      }

      m_transaction.modifySplit(split);
      if(m_transaction.commodity() == m_account.currencyId() && checkVat) {
        vatCheck(m_transaction, split);
      }

    }
    reloadEditWidgets(m_transaction);

  } catch(MyMoneyException *e) {
    KMessageBox::detailedSorry(0, i18n("Unable to modify amount"),
        (e->what() + " " + i18n("thrown in") + " " + e->file()+ ":%1").arg(e->line()));
    delete e;
    if(m_editAmount)
      m_editAmount->resetText();
    m_transaction = t;
    m_split = s;
  }
}

void KLedgerView::slotPaymentChanged(const QString& value)
{
  if(!m_editPayment)
    return;

  MyMoneyMoney val(value);

  m_split.setValue(0);
  if(val.isNegative()) {
    slotDepositChanged((-val).formatMoney());
    return;
  }
  amountChanged((-val).formatMoney());
  m_editPayment->loadText(value);
  m_editDeposit->loadText(QString());
  updateTabBar(m_transaction, m_split);
}

void KLedgerView::slotDepositChanged(const QString& value)
{
  if(!m_editDeposit)
    return;

  MyMoneyMoney val(value);

  if(val.isNegative()) {
    slotPaymentChanged((-val).formatMoney());
    return;
  }
  amountChanged(val.formatMoney());
  m_editDeposit->loadText(value);
  m_editPayment->loadText(QString());
  updateTabBar(m_transaction, m_split);
}

void KLedgerView::vatCheck(MyMoneyTransaction& t, MyMoneySplit& split)
{
  bool needFocusOutEvent = false;
  try {
    MyMoneyAccount acc = MyMoneyFile::instance()->account(split.accountId());
    // verify that vat account exists
    MyMoneyAccount vatAcc = MyMoneyFile::instance()->account(acc.value("VatAccount").latin1());
    MyMoneyMoney vatRate;
    MyMoneyMoney gv, nv;    // gross value, net value
    vatRate.fromString(vatAcc.value("VatRate"));
    if(!vatRate.isZero()) {
      MyMoneySplit vatSplit;
      try {
        vatSplit = t.splitByAccount(vatAcc.id(), true);
      } catch(MyMoneyException *e) {
        // if the split does not yet exist, we end up here and create it
        vatSplit.setAccountId(vatAcc.id());
        t.addSplit(vatSplit);
        // if we switch from 'non-split' to 'split' transaction, we
        // need to get out of the category field.
        needFocusOutEvent = true;
        delete e;
      }

      qDebug("vat amount is '%s'", acc.value("VatAmount").latin1());
      if(acc.value("VatAmount") != QString("Net")) {
        // split value is the gross value
        gv = split.value();
        nv = gv / (MyMoneyMoney(1,1) + vatRate);
        split.setValue(nv);
        t.modifySplit(split);

      } else {
        // split value is the net value
        nv = split.value();
        gv = nv * (MyMoneyMoney(1,1) + vatRate);
        MyMoneySplit accSplit = t.splitByAccount(accountId());
        accSplit.setValue(accSplit.value() - (gv - nv));
        t.modifySplit(accSplit);
      }

      vatSplit.setValue(vatSplit.value() + (gv - nv));
      t.modifySplit(vatSplit);
    }
  } catch(MyMoneyException *e) {
    delete e;
  }
  if(needFocusOutEvent && qApp->focusWidget() == m_editCategory) {
    m_editMemo->setFocus();
  }
}

void KLedgerView::slotCategoryChanged(const QCString& categoryId)
{
  MyMoneyTransaction t;
  MyMoneySplit s;

  t = m_transaction;
  s = m_split;

  try {
    createSecondSplit();
    MyMoneySplit split = m_transaction.splitByAccount(accountId(), false);
    if(!categoryId.isEmpty()) {
      // verify that account exists
      MyMoneyAccount acc = MyMoneyFile::instance()->account(categoryId);
      // do we have to check for vat?
      bool checkVat = split.accountId().isEmpty() && !(split.value().isZero()) && !acc.value("VatAccount").isEmpty();

      split.setAccountId(categoryId);
      m_transaction.modifySplit(split);
      if(checkVat) {
        vatCheck(m_transaction, split);
        reloadEditWidgets(m_transaction);
      }
    } else {
      m_transaction.removeSplit(split);
      createSecondSplit();
    }
    fillFormStatics();

  } catch(MyMoneyException *e) {
    KMessageBox::detailedSorry(0, i18n("Unable to modify category"),
        (e->what() + " " + i18n("thrown in") + " " + e->file()+ ":%1").arg(e->line()));
    delete e;
    m_transaction = t;
    m_split = s;
  }
}

void KLedgerView::slotCategoryChanged(const QString& /* category */)
{
  qDebug("usage of KLedgerView::slotCategoryChanged(const QString& category) is deprecated");
}

void KLedgerView::createSecondSplit(void)
{
  MyMoneySplit split;
  if(m_transaction.splitCount() == 1) {
    split.setAction(m_split.action());
    split.setPayeeId(m_split.payeeId());
    split.setMemo(m_split.memo());
    split.setValue(-m_split.value());
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

void KLedgerView::actionChanged(const QCString& action)
{

  MyMoneyTransaction t = m_transaction;
  MyMoneySplit s = m_split;

  try {
    m_split.setAction(action);
    m_transaction.modifySplit(m_split);
    fillFormStatics();
    reloadEditWidgets(m_transaction);

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
    if(visible) {
      m_register->setLedgerLens(m_ledgerLens);
    } else {
      m_register->setLedgerLens(true);
    }

    // force update of row count because the lens setting might have changed
    m_register->setTransactionCount(m_transactionPtrVector.size()+1, false);

    // inform widget, if inline editing should be available or not
    m_register->setInlineEditingMode(!visible);

    // using a timeout is the only way, I got the 'ensureTransactionVisible'
    // working when coming from hidden form to visible form. I assume, this
    // has something to do with the delayed update of the display somehow.
    resize(width()-1, height());
    QTimer::singleShot(10, this, SLOT(timerDone()));
  }
}

void KLedgerView::timerDone(void)
{
  m_register->ensureTransactionVisible();
  m_register->repaintContents(false);

  // the resize operation does the trick to adjust
  // all widgets in the view to the size they should
  // have and show up correctly. Don't ask me, why
  // this is, but it cured the problem (ipwizard).
  resize(width()+1, height());
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
#if 0
  else if(action == MyMoneySplit::ActionAddShares)
    rc = i18n("&Add Shares");
  else if(action == MyMoneySplit::ActionRemoveShares)
    rc = i18n("&Remove Shares");
#endif

  if(rc.isEmpty()) {
    qDebug("Unsupported action string %s, set to check", static_cast<const char *>(action));
    rc = i18n("&Check");
  }

  if(showHotkey == false) {
    rc = rc.replace(QRegExp("&"), "");
  }
  return rc;
}

void KLedgerView::slotNewPayee(const QString& payeeName)
{
  KMyMoneyUtils::newPayee(this, m_editPayee, payeeName);
}

int KLedgerView::transactionDirection(const MyMoneySplit& split)
{
  return (split.value().isZero()) ? UnknownDirection: ((split.value().isPositive()) ? Credit : Debit);
}

const MyMoneySplit KLedgerView::stockSplit(const MyMoneyTransaction& t)
{
  QValueList<MyMoneySplit>::ConstIterator it_s;
  for(it_s = t.splits().begin(); it_s != t.splits().end(); ++it_s) {
    if(!(*it_s).accountId().isEmpty()) {
      MyMoneyAccount acc = MyMoneyFile::instance()->account((*it_s).accountId());
      if(acc.accountType() == MyMoneyAccount::Stock) {
        return *it_s;
      }
    }
  }
  return MyMoneySplit();
}

int KLedgerView::transactionType(const MyMoneyTransaction& t)
{
  if(!stockSplit(t).id().isEmpty())
    return InvestmentTransaction;

  if(t.splitCount() < 2) {
    return Unknown;
  } else if(t.splitCount() > 2) {
    // FIXME check for loan transaction here
    return SplitTransaction;
  }
  QCString ida, idb;
  ida = t.splits()[0].accountId();
  idb = t.splits()[1].accountId();
  if(ida.isEmpty() || idb.isEmpty())
    return Normal;

  MyMoneyAccount a, b;
  a = MyMoneyFile::instance()->account(ida);
  b = MyMoneyFile::instance()->account(idb);
  if((a.accountGroup() == MyMoneyAccount::Asset
   || a.accountGroup() == MyMoneyAccount::Liability)
  && (b.accountGroup() == MyMoneyAccount::Asset
   || b.accountGroup() == MyMoneyAccount::Liability))
    return Transfer;
  return Normal;
}

const QCString KLedgerView::transactionType(int type) const
{
  qDebug("Usage of KLedgerView::transactionType(int type) is deprecated!");
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

void KLedgerView::showWidgets(void)
{
  QWidget* focusWidget;

  createEditWidgets();
  if(!m_transactionFormActive) {
    createRegisterButtons();
  }
  loadEditWidgets();

  if(m_transactionFormActive) {
    focusWidget = arrangeEditWidgetsInForm();
  } else {
    focusWidget = arrangeEditWidgetsInRegister();
  }

  // setup the keyboard filter for all widgets
  for(QWidget* w = m_tabOrderWidgets.first(); w; w = m_tabOrderWidgets.next()) {
    w->installEventFilter(this);
  }

  // make sure, size of all form columns are correct
  resizeEvent(0);

  m_tabOrderWidgets.find(focusWidget);
  focusWidget->setFocus();
  m_editMode = true;
}

void KLedgerView::destroyWidgets(void)
{
  for(int i=0; i < m_form->table()->numRows(); ++i) {
    m_form->table()->clearCellWidget(i, 1);
    m_form->table()->clearCellWidget(i, 2);
    m_form->table()->clearCellWidget(i, 4);
  }

  int   firstRow = m_register->currentTransactionIndex() * m_register->rpt();
  for(int i = 0; i < m_register->maxRpt(); ++i) {
    for(int j = 0; j < m_register->numCols(); ++j) {
      m_register->clearCellWidget(firstRow+i, j);
    }
  }
  m_editMode = false;

#if QT_IS_VERSION(3,3,0)
  // make sure, that the widgets will be gone (really deleted) before we continue
  QApplication::eventLoop()->processEvents(QEventLoop::ExcludeUserInput, 100);
#else
  qApp->processEvents(10);
#endif
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

  m_form->editButton()->setEnabled(false);
  m_form->newButton()->setEnabled(false);
  enableOkButton(true);
  enableCancelButton(true);
  enableMoreButton(true);

  showWidgets();

  if(m_editDate->getQDate().isValid())
    m_transaction.setPostDate(m_editDate->getQDate());

  if(!m_transactionFormActive)
    m_register->setInlineEditingMode(true);
}

void KLedgerView::slotStartEdit(void)
{
  // make sure, the view supports the type of transaction
  if(transactionType(m_transaction) == InvestmentTransaction
  && !inherits("KLedgerViewInvestments")) {
    if(KMessageBox::questionYesNo(0, i18n("An investment transaction can only be modified in the transaction view. Do you want to change to the investment view?")) == KMessageBox::Yes) {

      emit accountAndTransactionSelected(stockSplit(m_transaction).accountId(), m_transaction.id());
    }
    return;
  }

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
        i18n("Transaction already reconciled"), KStdGuiItem::cont(),
        "EditReconciledTransaction") == KMessageBox::Cancel) {

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
  m_form->editButton()->setEnabled(false);
  enableOkButton(true);
  enableCancelButton(true);
  enableMoreButton(true);

  showWidgets();

  if(!m_transactionFormActive)
    m_register->setInlineEditingMode(true);
}

void KLedgerView::cancelOrEndEdit(void)
{
  KConfig* kconfig = KGlobal::config();
  kconfig->setGroup("General Options");
  if(kconfig->readBoolEntry("FocusChangeIsEnter", false) == true)
    slotEndEdit();
  else
    slotCancelEdit();
}

void KLedgerView::slotCancelEdit(void)
{
  // force focusOut processing of the widgets
  m_register->setFocus();

  m_form->newButton()->setEnabled(true);
  enableOkButton(false);
  enableCancelButton(false);
  enableMoreButton(false);

  if(transaction(m_register->currentTransactionIndex()) != 0) {
    m_form->editButton()->setEnabled(true);
  }

  if(m_editDate && m_editDate->isVisible()) {
    destroyWidgets();
    fillForm();
  }

  m_register->setInlineEditingMode(false);
  m_register->setFocus();
}

void KLedgerView::slotEndEdit(void)
{
  // force focus change to update all data
  m_form->enterButton()->setFocus();

  MyMoneyFile* file = MyMoneyFile::instance();

  try {

    // make sure, the post date is valid
    if(!m_transaction.postDate().isValid())
      m_transaction.setPostDate(QDate::currentDate());
    // remember date for next new transaction
    m_lastPostDate = m_transaction.postDate();

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
    // Price information for other currencies will be collected

    QMap<QCString, MyMoneyMoney> priceInfo;
    for(it = list.begin(); it != list.end(); ++it) {
      if((*it).accountId().isEmpty()) {
        m_transaction.removeSplit(*it);
        continue;
      }
      MyMoneyAccount acc = file->account((*it).accountId());
      MyMoneySecurity currency(file->currency(m_account.currencyId()));
      int fract = currency.smallestAccountFraction();
      if(acc.accountType() == MyMoneyAccount::Cash)
        fract = currency.smallestCashFraction();

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

      if(m_transaction.commodity() != acc.currencyId()) {
        // different currencies, try to find recent price info
        QMap<QCString, MyMoneyMoney>::Iterator it_p;
        QCString key = m_transaction.commodity() + "-" + acc.currencyId();
        it_p = priceInfo.find(key);

        // if it's not found, then collect it from the user first
        MyMoneyMoney price;
        if(it_p == priceInfo.end()) {
          MyMoneySecurity fromCurrency, toCurrency;
          MyMoneyMoney fromValue, toValue;
          if(m_transaction.commodity() != m_account.currencyId()) {
            toCurrency = file->currency(m_transaction.commodity());
            fromCurrency = file->currency(acc.currencyId());
            toValue = (*it).value();
            fromValue = (*it).shares();
          } else {
            fromCurrency = file->currency(m_transaction.commodity());
            toCurrency = file->currency(acc.currencyId());
            fromValue = (*it).value();
            toValue = (*it).shares();
          }

          KCurrencyCalculator calc(fromCurrency,
                                   toCurrency,
                                   fromValue,
                                   toValue,
                                   m_transaction.postDate(),
                                   fract,
                                   this, "currencyCalculator");
          if(calc.exec() == QDialog::Rejected) {
            return;
          }
          price = calc.price();
          priceInfo[key] = price;
        } else {
          price = (*it_p);
        }

        // update shares if the transaction commodity is the currency
        // of the current selected account
        if(m_transaction.commodity() == m_account.currencyId()) {
          (*it).setShares(((*it).value() * price).convert(fract));
        }

        // now update the split
        m_transaction.modifySplit(*it);
      } else {
        if((*it).shares() != (*it).value()) {
          (*it).setShares((*it).value());
          m_transaction.modifySplit(*it);
        }
      }
    }

    if(transactionType(m_transaction) == Transfer && !m_split.payeeId().isEmpty()) {
      for(it = list.begin(); it != list.end(); ++it) {
        if((*it).id() == m_split.id())
          continue;

        if((*it).payeeId().isEmpty()) {
          (*it).setPayeeId(m_split.payeeId());
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
  } catch(MyMoneyException *e) {
    KMessageBox::detailedSorry(0, i18n("Unable to add/modify transaction"),
      (e->what() + " " + i18n("thrown in") + " " + e->file()+ ":%1").arg(e->line()));
    delete e;
    return;
  }


  // switch the context to enable refreshView() to work
  m_form->newButton()->setEnabled(true);
  enableOkButton(false);
  enableCancelButton(false);
  enableMoreButton(false);

  if(transaction(m_register->currentTransactionIndex()) != 0) {
    m_form->editButton()->setEnabled(true);
  }

  destroyWidgets();

  MyMoneyTransaction t;

  // so, we now have to save something here.
  // if an existing transaction has been changed, we take it as the base
  if(m_transactionPtr != 0) {
    t = *m_transactionPtr;
  }

  if(!(t == m_transaction)) {
    try {
      // differentiate between add and modify
      QCString id;
      if(m_transactionPtr == 0) {
        // in the add case, we don't have an ID yet. So let's get one
        // and use it down the line

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

  m_register->setInlineEditingMode(false);
  m_register->setFocus();
}

void KLedgerView::clearTabOrder(void)
{
  m_tabOrderWidgets.clear();
}

void KLedgerView::addToTabOrder(QWidget* w)
{
  if(w) {
    while(w->focusProxy())
      w = w->focusProxy();
    m_tabOrderWidgets.append(w);
  }
}

bool KLedgerView::focusNextPrevChild(bool next)
{
  bool  rc = false;

  if(m_editDate) {
    QWidget *w = 0;
    QWidget *currentWidget;

    m_tabOrderWidgets.find(qApp->focusWidget());
    currentWidget = m_tabOrderWidgets.current();
    w = next ? m_tabOrderWidgets.next() : m_tabOrderWidgets.prev();

    do {
      if(!w) {
        w = next ? m_tabOrderWidgets.first() : m_tabOrderWidgets.last();
      }

      if(w != currentWidget
      && ((w->focusPolicy() & TabFocus) == TabFocus)
      && w->isVisible() && w->isEnabled()) {
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

void KLedgerView::enableWidgets(QPtrList<QWidget> list, const bool enabled)
{
  QWidget* it;
  for(it = list.first(); it; it = list.next())
    it->setEnabled(enabled);
}

void KLedgerView::enableOkButton(const bool enabled)
{
  QPtrList<QWidget> list;
  if(m_registerEnterButton)
    list.append(m_registerEnterButton);
  if(m_form && m_form->enterButton())
    list.append(m_form->enterButton());
  enableWidgets(list, enabled);
}

void KLedgerView::enableCancelButton(const bool enabled)
{
  QPtrList<QWidget> list;
  if(m_registerCancelButton)
    list.append(m_registerCancelButton);
  if(m_form && m_form->cancelButton())
    list.append(m_form->cancelButton());
  enableWidgets(list, enabled);
}

void KLedgerView::enableMoreButton(const bool enabled)
{
  QPtrList<QWidget> list;
  if(m_registerMoreButton)
    list.append(m_registerMoreButton);
  if(m_form && m_form->moreButton())
    list.append(m_form->moreButton());
  enableWidgets(list, enabled);
}

void KLedgerView::createRegisterButtons(void)
{
  if(!m_registerButtonFrame) {
    KIconLoader *il = KGlobal::iconLoader();
    m_registerButtonFrame = new QFrame(this, "buttonFrame");
    QPalette palette = m_registerButtonFrame->palette();
    palette.setColor(QColorGroup::Background, m_registerButtonFrame->colorGroup().highlight() );
    m_registerButtonFrame->setPalette(palette);

    QHBoxLayout* l = new QHBoxLayout(m_registerButtonFrame);
    m_registerEnterButton = new KPushButton(il->loadIcon("button_ok", KIcon::Small, KIcon::SizeSmall), QString(), m_registerButtonFrame, "EnterButton");
    m_registerCancelButton = new KPushButton(il->loadIcon("button_cancel", KIcon::Small, KIcon::SizeSmall), QString(), m_registerButtonFrame, "CancelButton");
    m_registerMoreButton = new KPushButton(il->loadIcon("configure", KIcon::Small, KIcon::SizeSmall), QString(), m_registerButtonFrame, "MoreButton");
    if(m_form->moreButton()->popup()) {
      m_registerMoreButton->setPopup(m_form->moreButton()->popup());
    }
    l->addWidget(m_registerEnterButton);
    l->addWidget(m_registerCancelButton);
    l->addWidget(m_registerMoreButton);
    l->addStretch(2);

    connect(m_registerEnterButton, SIGNAL(clicked()), this, SLOT(slotEndEdit()));
    connect(m_registerCancelButton, SIGNAL(clicked()), this, SLOT(slotCancelEdit()));

  }
}

void KLedgerView::createAccountMenu(void)
{
  Q_CHECK_PTR(m_register);

  KIconLoader *kiconloader = KGlobal::iconLoader();

  m_accountMenu = new KPopupMenu(this);
  m_accountMenu->insertTitle(i18n("Account Options"));
  m_accountMenu->insertItem(kiconloader->loadIcon("viewmag", KIcon::Small), i18n("Account Details ..."), this, SLOT(slotAccountDetail()));
  m_accountMenu->insertItem(kiconloader->loadIcon("reconcile", KIcon::Small), i18n("Reconcile ..."), this, SLOT(slotReconciliation()));
}

void KLedgerView::createContextMenu(void)
{
  Q_CHECK_PTR(m_register);

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
  if(m_transactionPtr != 0 || isEditMode()) {
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
  Q_CHECK_PTR(m_register);

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
  m_register->setFocus();
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
  // return m_editDate && m_editDate->isVisible();
  return m_editMode;
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
  // the following timer event forces an update of the view once
  // it is shown. This corrects the sizes of all widgets. If we
  // don't do this, some of the data/widgets might not be visible.
  QTimer::singleShot(10,this, SLOT(timerDone()));
}

void KLedgerView::slotCreateSchedule(void)
{
  if (!m_transaction.id().isEmpty()) {
    MyMoneySchedule schedule;
    MyMoneySchedule::typeE scheduleType = MyMoneySchedule::TYPE_ANY;

    if(m_account.accountGroup() == MyMoneyAccount::Asset) {
      if(!m_split.value().isNegative())
        scheduleType = MyMoneySchedule::TYPE_DEPOSIT;
      else
        scheduleType = MyMoneySchedule::TYPE_BILL;
    } else {
      if(!m_split.value().isNegative())
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

void KLedgerView::setRegisterCellWidget(const int r, const int c, QWidget *w)
{
  setCellWidget(m_register, r, c, w);
}

void KLedgerView::setFormCellWidget(const int r, const int c, QWidget* w)
{
  setCellWidget(m_form->table(), r, c, w);
}

void KLedgerView::setCellWidget(QTable* table, const int row, const int col, QWidget* w)
{
  Q_CHECK_PTR(table != 0);

  if(table->cellWidget(row, col) != w)
    table->setCellWidget(row, col, w);
}

bool KLedgerView::eventFilter(QObject* o, QEvent* e)
{
  bool rc = false;

  if(o->isWidgetType()) {
    if(e->type() == QEvent::KeyPress) {
      const QWidget* w = dynamic_cast<const QWidget*>(o);
      QKeyEvent *k = static_cast<QKeyEvent *> (e);
      if(m_tabOrderWidgets.findRef(w) != -1) {
        rc = true;
        bool terminate = true;
        switch(k->key()) {
          default:
            rc = false;
            break;

          case Qt::Key_Return:
          case Qt::Key_Enter:
            // we cannot call the slot directly, as it destroys the caller of
            // this method :-(  So we let the event handler take care of calling
            // the respective slot using a timeout. For a KLineEdit derived object
            // it could be, that at this point the user selected a value from
            // a completion list. In this case, we close the completion list and
            // do not end editing of the transaction.
            if(o->inherits("KLineEdit")) {
              KLineEdit* le = dynamic_cast<KLineEdit*> (o);
              KCompletionBox* box = le->completionBox(false);
              if(box && box->isVisible()) {
                terminate = false;
                le->completionBox(false)->hide();
              }
            }
            if(terminate)
              QTimer::singleShot(0, this, SLOT(slotEndEdit()));
            break;

          case Qt::Key_Escape:
            // we cannot call the slot directly, as it destroys the caller of
            // this method :-(  So we let the event handler take care of calling
            // the respective slot using a timeout.
            QTimer::singleShot(0, this, SLOT(slotCancelEdit()));
            break;
        }
      } else if(w == m_register && !isEditMode()) {
        switch(k->key()) {
          default:
            break;

          case Qt::Key_Return:
          case Qt::Key_Enter:
            rc = true;
            slotStartEdit();
            break;
        }
      }
    }
  }
  return rc;
}

