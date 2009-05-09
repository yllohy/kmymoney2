/***************************************************************************
                          kgloballedgerview.cpp  -  description
                             -------------------
    begin                : Wed Jul 26 2006
    copyright            : (C) 2006 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <typeinfo>

// ----------------------------------------------------------------------------
// QT Includes

#include <qframe.h>
#include <qlayout.h>
#include <qtimer.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include "kdecompat.h"
#include <klocale.h>
#include <kcombobox.h>
#include <kpushbutton.h>
#include <kmessagebox.h>
#include <kiconloader.h>
#include <ktoolbar.h>
#include <ktoolbarbutton.h>
#include <kpassivepopup.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kgloballedgerview.h"

#include <kmymoney/mymoneyaccount.h>
#include <kmymoney/mymoneyfile.h>
#include <kmymoney/kmymoneyaccountcombo.h>
#include <kmymoney/kmymoneytitlelabel.h>
#include <kmymoney/register.h>
#include <kmymoney/transactioneditor.h>
#include <kmymoney/selectedtransaction.h>

#include <kmymoney/kmymoneyglobalsettings.h>

#include "../widgets/registersearchline.h"
#include "../dialogs/ksortoptiondlg.h"
#include "../kmymoney2.h"

#include "../widgets/scheduledtransaction.h"

class KGlobalLedgerView::Private
{
public:
  Private();

  MousePressFilter*    m_mousePressFilter;
  KMyMoneyRegister::RegisterSearchLineWidget* m_registerSearchLine;
  QPoint               m_startPoint;
  QString             m_reconciliationAccount;
  QDate                m_reconciliationDate;
  MyMoneyMoney         m_endingBalance;
  int                  m_precision;
  bool                 m_inLoading;
  bool                 m_recursion;
  bool                 m_showDetails;
  KMyMoneyRegister::Action m_action;
  QTimer               m_viewPosTimer;
};

MousePressFilter::MousePressFilter(QWidget* parent, const char* name) :
  QObject(parent, name),
  m_lastMousePressEvent(0),
  m_filterActive(true)
{
}

void MousePressFilter::addWidget(QWidget* w)
{
  m_parents.append(w);
}

void MousePressFilter::setFilterActive(bool state)
{
  m_filterActive = state;
}

bool MousePressFilter::isChildOf( QWidget* child, QWidget *parent )
{
  while(child) {
    if(child == parent)
      return true;
    // If one of the ancestors is a KPassivePopup then it's as
    // if it is a child of our own
    if(dynamic_cast<KPassivePopup*>(child))
      return true;
    child = child->parentWidget();
  }
  return false;
}

bool MousePressFilter::eventFilter(QObject* o, QEvent* e)
{
  if(m_filterActive) {
    if(e->type() == QEvent::MouseButtonPress && !m_lastMousePressEvent) {
      QValueList<QWidget*>::const_iterator it_w;
      for(it_w = m_parents.begin(); it_w != m_parents.end(); ++it_w) {
        if(isChildOf((QWidget*)o, (*it_w))) {
          m_lastMousePressEvent = e;
          break;
        }
      }
      if(it_w == m_parents.end()) {
        m_lastMousePressEvent = e;
        bool rc = false;
        emit mousePressedOnExternalWidget(rc);
      }
    }

    if(e->type() != QEvent::MouseButtonPress) {
      m_lastMousePressEvent = 0;
    }
  }
  return false;
}


KGlobalLedgerView::Private::Private() :
  m_mousePressFilter(0),
  m_registerSearchLine(0),
  m_inLoading(false),
  m_recursion(false),
  m_showDetails(false)
{
}

QDate KGlobalLedgerView::m_lastPostDate;

KGlobalLedgerView::KGlobalLedgerView(QWidget *parent, const char *name )
  : KMyMoneyViewBase(parent, name, i18n("Ledgers")),
  d(new Private),
  m_needReload(false),
  m_newAccountLoaded(true),
  m_inEditMode(false)
{
  d->m_mousePressFilter = new MousePressFilter((QWidget*)this);
  d->m_action = KMyMoneyRegister::ActionNone;;

  // create the toolbar frame at the top of the view
  m_toolbarFrame = new QFrame(this);
  QVBoxLayout* toolbarLayout = new QVBoxLayout(m_toolbarFrame, 0, 0);

  m_toolbar = new KToolBar(m_toolbarFrame, 0, true);
  toolbarLayout->addWidget(m_toolbar);
  m_toolbar->setIconText(KToolBar::IconTextRight);

  m_accountComboBox = new KMyMoneyAccountCombo(m_toolbar, "AccountCombo");
  m_toolbar->insertWidget(1, 100, m_accountComboBox);

#if 0
  // the account button at the right of the toolbar
  // I leave the code commented here for a while, so that I see
  // how I can add other  widgets at this point
  KIconLoader *il = KGlobal::iconLoader();
  m_toolbar->insertButton(il->loadIcon("document", KIcon::Small, KIcon::SizeSmall),
                        1,true,i18n("Account"));
  //m_toolbar->setMaximumSize(50,20);
  m_toolbar->alignItemRight(1);
#endif
  m_toolbar->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
  layout()->addWidget(m_toolbarFrame);

  // create the register frame
  m_registerFrame = new QFrame(this);
  QVBoxLayout* registerFrameLayout = new QVBoxLayout(m_registerFrame, 0, 0);
  layout()->addWidget(m_registerFrame);
  layout()->setStretchFactor(m_registerFrame, 2);
  m_register = new KMyMoneyRegister::Register(m_registerFrame);
  registerFrameLayout->addWidget(m_register);
  m_register->installEventFilter(this);
  connect(m_register, SIGNAL(openContextMenu()), this, SIGNAL(openContextMenu()));
  connect(m_register, SIGNAL(headerClicked()), this, SLOT(slotSortOptions()));
  connect(m_register, SIGNAL(reconcileStateColumnClicked(KMyMoneyRegister::Transaction*)), this, SLOT(slotToggleTransactionMark(KMyMoneyRegister::Transaction*)));
  connect(&d->m_viewPosTimer, SIGNAL(timeout()), this, SLOT(slotUpdateViewPos()));

  // insert search line widget

  d->m_registerSearchLine = new KMyMoneyRegister::RegisterSearchLineWidget(m_register, m_toolbar);

  // create the summary frame
  m_summaryFrame = new QFrame(this);
  QHBoxLayout* summaryFrameLayout = new QHBoxLayout(m_summaryFrame, 0, 0);
  m_leftSummaryLabel = new QLabel(m_summaryFrame);
  m_centerSummaryLabel = new QLabel(m_summaryFrame);
  m_rightSummaryLabel = new QLabel(m_summaryFrame);
  summaryFrameLayout->addWidget(m_leftSummaryLabel);
  QSpacerItem* spacer = new QSpacerItem( 20, 1, QSizePolicy::Expanding, QSizePolicy::Minimum );
  summaryFrameLayout->addItem(spacer);
  summaryFrameLayout->addWidget(m_centerSummaryLabel);
  spacer = new QSpacerItem( 20, 1, QSizePolicy::Expanding, QSizePolicy::Minimum );
  summaryFrameLayout->addItem(spacer);
  summaryFrameLayout->addWidget(m_rightSummaryLabel);
  layout()->addWidget(m_summaryFrame);

  // create the button frame
  m_buttonFrame = new QFrame(this);
  QVBoxLayout* buttonLayout = new QVBoxLayout(m_buttonFrame, 0, 0);
  layout()->addWidget(m_buttonFrame);
  m_buttonbar = new KToolBar(m_buttonFrame, 0, true);
  m_buttonbar->setIconText(KToolBar::IconTextRight);
  buttonLayout->addWidget(m_buttonbar);

  kmymoney2->action("transaction_new")->plug(m_buttonbar);
  kmymoney2->action("transaction_delete")->plug(m_buttonbar);
  kmymoney2->action("transaction_edit")->plug(m_buttonbar);
  kmymoney2->action("transaction_enter")->plug(m_buttonbar);
  kmymoney2->action("transaction_cancel")->plug(m_buttonbar);
  kmymoney2->action("transaction_accept")->plug(m_buttonbar);
  kmymoney2->action("transaction_match")->plug(m_buttonbar);

  // create the transaction form frame
  m_formFrame = new QFrame(this);
  QVBoxLayout* frameLayout = new QVBoxLayout(m_formFrame, 5, 0);
  m_form = new KMyMoneyTransactionForm::TransactionForm(m_formFrame);
  frameLayout->addWidget(m_form->tabBar(m_formFrame));
  frameLayout->addWidget(m_form);
  m_formFrame->setFrameShape( QFrame::Panel );
  m_formFrame->setFrameShadow( QFrame::Raised );
  layout()->addWidget(m_formFrame);

  connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotLoadView()));
  connect(m_register, SIGNAL(focusChanged(KMyMoneyRegister::Transaction*)), m_form, SLOT(slotSetTransaction(KMyMoneyRegister::Transaction*)));
  connect(m_register, SIGNAL(focusChanged()), kmymoney2, SLOT(slotUpdateActions()));
  connect(m_accountComboBox, SIGNAL(accountSelected(const QString&)), this, SLOT(slotSelectAccount(const QString&)));
  connect(m_register, SIGNAL(selectionChanged(const KMyMoneyRegister::SelectedTransactions&)), this, SIGNAL(transactionsSelected(const KMyMoneyRegister::SelectedTransactions&)));
  connect(m_register, SIGNAL(editTransaction()), this, SIGNAL(startEdit()));
  connect(m_register, SIGNAL(emptyItemSelected()), this, SLOT(slotNewTransaction()));
  connect(m_register, SIGNAL(aboutToSelectItem(KMyMoneyRegister::RegisterItem*, bool&)), this, SLOT(slotAboutToSelectItem(KMyMoneyRegister::RegisterItem*, bool&)));
  connect(d->m_mousePressFilter, SIGNAL(mousePressedOnExternalWidget(bool&)), this, SIGNAL(cancelOrEndEdit(bool&)));

  connect(m_form, SIGNAL(newTransaction(KMyMoneyRegister::Action)), this, SLOT(slotNewTransaction(KMyMoneyRegister::Action)));

  // setup mouse press filter
  d->m_mousePressFilter->addWidget(m_formFrame);
  d->m_mousePressFilter->addWidget(m_buttonFrame);
  d->m_mousePressFilter->addWidget(m_summaryFrame);
  d->m_mousePressFilter->addWidget(m_registerFrame);
}

KGlobalLedgerView::~KGlobalLedgerView()
{
  delete d;
}

void KGlobalLedgerView::slotAboutToSelectItem(KMyMoneyRegister::RegisterItem* item, bool& okToSelect)
{
  Q_UNUSED(item);
  emit cancelOrEndEdit(okToSelect);
}

void KGlobalLedgerView::slotLoadView(void)
{
  m_needReload = true;
  if(isVisible()) {
    if(!m_inEditMode) {
      loadView();
      m_needReload = false;
      // force a new account if the current one is empty
      m_newAccountLoaded = m_account.id().isEmpty();
    }
  }
}

void KGlobalLedgerView::clear(void)
{
  // clear current register contents
  m_register->clear();

  // setup header font
  QFont font = KMyMoneyGlobalSettings::listHeaderFont();
  QFontMetrics fm( font );
  int height = fm.lineSpacing()+6;
  m_register->horizontalHeader()->setMinimumHeight(height);
  m_register->horizontalHeader()->setMaximumHeight(height);
  m_register->horizontalHeader()->setFont(font);

  // setup cell font
  font = KMyMoneyGlobalSettings::listCellFont();
  m_register->setFont(font);

  // clear the form
  m_form->clear();

  // the selected transactions list
  m_transactionList.clear();

  // and the selected account in the combo box
  m_accountComboBox->setSelected(QString());

  // fraction defaults to two digits
  d->m_precision = 2;
}

void KGlobalLedgerView::loadView(void)
{
  MYMONEYTRACER(tracer);

  // setup form visibility
  m_formFrame->setShown(KMyMoneyGlobalSettings::transactionForm());

  // no account selected
  emit accountSelected(MyMoneyAccount());
  // no transaction selected
  KMyMoneyRegister::SelectedTransactions list;
  emit transactionsSelected(list);

  QMap<QString, bool> isSelected;
  QString focusItemId;
  QString anchorItemId;

  if(!d->m_inLoading)
    d->m_startPoint = QPoint(-1, -1);

  if(!m_newAccountLoaded) {
    // remember the current selected transactions
    KMyMoneyRegister::RegisterItem* item = m_register->firstItem();
    for(; item; item = item->nextItem()) {
      if(item->isSelected()) {
        isSelected[item->id()] = true;
      }
    }
    // remember the item that has the focus
    if(m_register->focusItem())
      focusItemId = m_register->focusItem()->id();
    // and the one that has the selection anchor
    if(m_register->anchorItem())
      anchorItemId = m_register->anchorItem()->id();

    // remember the upper left corner of the viewport
    if(!d->m_inLoading && d->m_showDetails == KMyMoneyGlobalSettings::showRegisterDetailed())
      d->m_startPoint = QPoint(m_register->contentsX(), m_register->contentsY());
  } else {
    if(d->m_viewPosTimer.isActive())
      d->m_viewPosTimer.stop();
    d->m_startPoint = QPoint(-1, -1);
    d->m_inLoading = false;
    d->m_registerSearchLine->searchLine()->reset();
  }

  // clear the current contents ...
  clear();

  // ... load the combobox widget and select current account ...
  loadAccounts();

  // ... setup the register columns ...
  m_register->setupRegister(m_account);

  // ... setup the form ...
  m_form->setupForm(m_account);

  if(m_account.id().isEmpty()) {
    // if we don't have an account we bail out
    setEnabled(false);
    return;
  }
  setEnabled(true);

  m_register->setUpdatesEnabled(false);

  // ... and recreate it
  KMyMoneyRegister::RegisterItem* focusItem = 0;
  KMyMoneyRegister::RegisterItem* anchorItem = 0;
  QMap<QString, MyMoneyMoney> actBalance, clearedBalance, futureBalance;
  QMap<QString, MyMoneyMoney>::iterator it_b;
  try {
    // setup the filter to select the transactions we want to display
    // and update the sort order
    QString sortOrder;
    QString key;
    QDate reconciliationDate = d->m_reconciliationDate;

    MyMoneyTransactionFilter filter(m_account.id());
    // if it's an investment account, we also take care of
    // the sub-accounts (stock accounts)
    if(m_account.accountType() == MyMoneyAccount::Investment)
      filter.addAccount(m_account.accountList());

    if(isReconciliationAccount()) {
      key = "kmm-sort-reconcile";
      sortOrder = KMyMoneyGlobalSettings::sortReconcileView();
      filter.addState(MyMoneyTransactionFilter::notReconciled);
      filter.addState(MyMoneyTransactionFilter::cleared);
    } else {
      filter.setDateFilter(KMyMoneyGlobalSettings::startDate().date(), QDate());
      key = "kmm-sort-std";
      sortOrder = KMyMoneyGlobalSettings::sortNormalView();
      if (KMyMoneyGlobalSettings::hideReconciledTransactions()) {
        filter.addState(MyMoneyTransactionFilter::notReconciled);
        filter.addState(MyMoneyTransactionFilter::cleared);
      }
    }
    filter.setReportAllSplits(true);

    // check if we have an account override of the sort order
    if(!m_account.value(key).isEmpty())
      sortOrder = m_account.value(key);

    // setup sort order
    m_register->setSortOrder(sortOrder);

    // retrieve the list from the engine
    MyMoneyFile::instance()->transactionList(m_transactionList, filter);

    kmymoney2->slotStatusProgressBar(0, m_transactionList.count());

    // create the elements for the register
    QValueList<QPair<MyMoneyTransaction, MyMoneySplit> >::const_iterator it;
    QMap<QString, int>uniqueMap;
    int i = 0;
    for(it = m_transactionList.begin(); it != m_transactionList.end(); ++it) {
      uniqueMap[(*it).first.id()]++;
      KMyMoneyRegister::Transaction* t = KMyMoneyRegister::Register::transactionFactory(m_register, (*it).first, (*it).second, uniqueMap[(*it).first.id()]);
      actBalance[t->split().accountId()] = MyMoneyMoney(0,1);
      kmymoney2->slotStatusProgressBar(++i, 0);
      // if we're in reconciliation and the state is cleared, we
      // force the item to show in dimmed intensity to get a visual focus
      // on those items, that we need to work on
      if(isReconciliationAccount() && (*it).second.reconcileFlag() == MyMoneySplit::Cleared) {
        t->setReducedIntensity(true);
      }
    }

    // create dummy entries for the scheduled transactions if sorted by postdate
    int period = KMyMoneyGlobalSettings::schedulePreview();
    if(m_register->primarySortKey() == KMyMoneyRegister::PostDateSort) {
      // show scheduled transactions which have a scheduled postdate
      // within the next 'period' days. In reconciliation mode, the
      // period starts on the statement date.
      QDate endDate = QDate::currentDate().addDays(period);
      if(isReconciliationAccount())
        endDate = reconciliationDate.addDays(period);
      QValueList<MyMoneySchedule> scheduleList = MyMoneyFile::instance()->scheduleList(m_account.id());
      while(scheduleList.count() > 0){
        MyMoneySchedule& s = scheduleList.first();
        for(;;) {
          if(s.isFinished() || s.nextDueDate() > endDate) {
            break;
          }

          MyMoneyTransaction t(s.id(), KMyMoneyUtils::scheduledTransaction(s));
          t.setPostDate(s.nextDueDate());
          // if the transaction is scheduled and overdue, it can't
          // certainly be posted in the past. So we take todays date
          // as the alternative
          if(s.isOverdue())
            t.setPostDate(QDate::currentDate());
          const QValueList<MyMoneySplit>& splits = t.splits();
          QValueList<MyMoneySplit>::const_iterator it_s;
          for(it_s = splits.begin(); it_s != splits.end(); ++it_s) {
            if((*it_s).accountId() == m_account.id()) {
              new KMyMoneyRegister::StdTransactionScheduled(m_register, t, *it_s, uniqueMap[t.id()]);
            }
          }
          // keep track of this payment locally (not in the engine)
          s.setLastPayment(t.postDate());

          // if this is a one time schedule, we can bail out here as we're done
          if(s.occurence() == MyMoneySchedule::OCCUR_ONCE)
            break;

          // for all others, we check if the next payment date is still 'in range'
          s.setNextDueDate(s.nextPayment(s.nextDueDate()));
        }
        scheduleList.pop_front();
      }
    }

    // add the group markers
    m_register->addGroupMarkers();

    // sort the transactions according to the sort setting
    m_register->sortItems();

    // remove trailing and adjacent markers
    m_register->removeUnwantedGroupMarkers();

    // add special markers for reconciliation now so that they do not get
    // removed by m_register->removeUnwantedGroupMarkers(). Needs resorting
    // of items but that's ok.

    KMyMoneyRegister::StatementGroupMarker* statement = 0;
    KMyMoneyRegister::StatementGroupMarker* dStatement = 0;
    KMyMoneyRegister::StatementGroupMarker* pStatement = 0;

    if(isReconciliationAccount()) {
      switch(m_register->primarySortKey()) {
        case KMyMoneyRegister::PostDateSort:
          statement = new KMyMoneyRegister::StatementGroupMarker(m_register, KMyMoneyRegister::Deposit, reconciliationDate, i18n("Statement Details"));
          m_register->sortItems();
          break;
        case KMyMoneyRegister::TypeSort:
          dStatement = new KMyMoneyRegister::StatementGroupMarker(m_register, KMyMoneyRegister::Deposit, reconciliationDate, i18n("Statement Deposit Details"));
          pStatement = new KMyMoneyRegister::StatementGroupMarker(m_register, KMyMoneyRegister::Payment, reconciliationDate, i18n("Statement Payment Details"));
          m_register->sortItems();
          break;
        default:
          break;
      }
    }

    // we need at least the balance for the account we currently show
    actBalance[m_account.id()] = MyMoneyMoney();

    if(m_account.accountType() == MyMoneyAccount::Investment) {
      QValueList<QString>::const_iterator it_a;
      for(it_a = m_account.accountList().begin(); it_a != m_account.accountList().end(); ++it_a) {
            actBalance[*it_a] = MyMoneyMoney();
          }
    }

    // determine balances (actual, cleared). We do this by getting the actual
    // balance of all entered transactions from the engine and walk the list
    // of transactions backward. Also re-select a transaction if it was
    // selected before and setup the focus item.

    MyMoneyMoney factor(1,1);
    if(m_account.accountGroup() == MyMoneyAccount::Liability
    || m_account.accountGroup() == MyMoneyAccount::Equity)
      factor = -factor;

    QMap<QString, int> deposits;
    QMap<QString, int> payments;
    QMap<QString, MyMoneyMoney> depositAmount;
    QMap<QString, MyMoneyMoney> paymentAmount;
    for(it_b = actBalance.begin(); it_b != actBalance.end(); ++it_b) {
      MyMoneyMoney balance = MyMoneyFile::instance()->balance(it_b.key());
      balance = balance * factor;
      clearedBalance[it_b.key()] =
      futureBalance[it_b.key()] =
      (*it_b) = balance;
      deposits[it_b.key()] = payments[it_b.key()] = 0;
      depositAmount[it_b.key()] = MyMoneyMoney();
      paymentAmount[it_b.key()] = MyMoneyMoney();
    }

    tracer.printf("total balance of %s = %s", m_account.name().data(), actBalance[m_account.id()].formatMoney("", 2).data());
    tracer.printf("future balance of %s = %s", m_account.name().data(), futureBalance[m_account.id()].formatMoney("", 2).data());
    tracer.printf("cleared balance of %s = %s", m_account.name().data(), clearedBalance[m_account.id()].formatMoney("", 2).data());

    KMyMoneyRegister::RegisterItem* p = m_register->lastItem();
    focusItem = 0;

    // take care of possibly trailing scheduled transactions (bump up the future balance)
    while(p) {
      if(p->isSelectable()) {
        KMyMoneyRegister::Transaction* t = dynamic_cast<KMyMoneyRegister::Transaction*>(p);
        if(t && t->isScheduled()) {
          MyMoneyMoney balance = futureBalance[t->split().accountId()];
          const MyMoneySplit& split = t->split();
          // if this split is a stock split, we can't just add the amount of shares
          if(t->transaction().isStockSplit()) {
            balance = balance * split.shares();
          } else {
            balance += split.shares() * factor;
          }
          futureBalance[split.accountId()] = balance;
        } else if(t && !focusItem)
          focusItem = p;
      }
      p = p->prevItem();
    }

    p = m_register->lastItem();
    while(p) {
      KMyMoneyRegister::Transaction* t = dynamic_cast<KMyMoneyRegister::Transaction*>(p);
      if(t) {
        if(isSelected.contains(t->id()))
          t->setSelected(true);

        if(t->id() == focusItemId)
          focusItem = t;
        if(t->id() == anchorItemId)
          anchorItem = t;

        MyMoneyMoney balance = futureBalance[t->split().accountId()];
        t->setBalance(balance.formatMoney("", d->m_precision));
        const MyMoneySplit& split = t->split();

        // if this split is a stock split, we can't just add the amount of shares
        if(t->transaction().isStockSplit()) {
          balance /= split.shares();
        } else {
          balance -= split.shares() * factor;
        }

        if(!t->isScheduled()) {
          if(split.reconcileFlag() == MyMoneySplit::NotReconciled) {
            tracer.printf("Reducing cleared balance by %s because %s/%s(%s) is not reconciled", (split.shares() * factor).formatMoney("", 2).data(), t->transaction().id().data(), split.id().data(), t->transaction().postDate().toString(Qt::ISODate).data());
            clearedBalance[split.accountId()] -= split.shares() * factor;
          }
          if(isReconciliationAccount() && t->transaction().postDate() > reconciliationDate && split.reconcileFlag() == MyMoneySplit::Cleared) {
            tracer.printf("Reducing cleared balance by %s because we are in reconciliation, %s/%s(%s)'s date is after or on reconciliation date (%s) and is cleared", (split.shares() * factor).formatMoney("", 2).data(), t->transaction().id().data(), split.id().data(), t->transaction().postDate().toString(Qt::ISODate).data(), reconciliationDate.toString(Qt::ISODate).data());

            clearedBalance[split.accountId()] -= split.shares() * factor;
          }
          if(isReconciliationAccount() && t->transaction().postDate() <= reconciliationDate && split.reconcileFlag() == MyMoneySplit::Cleared) {
            if(split.shares().isNegative()) {
              payments[split.accountId()]++;
              paymentAmount[split.accountId()] += split.shares();
            } else {
              deposits[split.accountId()]++;
              depositAmount[split.accountId()] += split.shares();
            }
          }

          if(t->transaction().postDate() > QDate::currentDate()) {
            tracer.printf("Reducing actual balance by %s because %s/%s(%s) is in the future", (split.shares() * factor).formatMoney("", 2).data(), t->transaction().id().data(), split.id().data(), t->transaction().postDate().toString(Qt::ISODate).data());
            actBalance[split.accountId()] -= split.shares() * factor;
          }
        }
        futureBalance[split.accountId()] = balance;
      }
      p = p->prevItem();
    }

    tracer.printf("total balance of %s = %s", m_account.name().data(), actBalance[m_account.id()].formatMoney("", 2).data());
    tracer.printf("future balance of %s = %s", m_account.name().data(), futureBalance[m_account.id()].formatMoney("", 2).data());
    tracer.printf("cleared balance of %s = %s", m_account.name().data(), clearedBalance[m_account.id()].formatMoney("", 2).data());

    // update statement information
    if(statement) {
      statement->setText(i18n("%1 deposits (%3), %2 payments (%4)").
          arg(deposits[m_account.id()]).
          arg(payments[m_account.id()]).
          arg(depositAmount[m_account.id()].abs().formatMoney(m_account.fraction())).
          arg(paymentAmount[m_account.id()].abs().formatMoney(m_account.fraction())) );
    }
    if(pStatement) {
      pStatement->setText(i18n("%1 payments (%2)").arg(payments[m_account.id()]).
          arg(paymentAmount[m_account.id()].abs().formatMoney(m_account.fraction())) );
    }
    if(dStatement) {
      dStatement->setText(i18n("%1 deposits (%2)").arg(deposits[m_account.id()]).
          arg(depositAmount[m_account.id()].abs().formatMoney(m_account.fraction())) );
    }

    // add a last empty entry for new transactions
    // leave some information about the current account
    MyMoneySplit split;
    split.setReconcileFlag(MyMoneySplit::NotReconciled);
    // make sure to use the value specified in the option during reconciliation
    if(isReconciliationAccount())
      split.setReconcileFlag(static_cast<MyMoneySplit::reconcileFlagE>(KMyMoneyGlobalSettings::defaultReconciliationState()));
    KMyMoneyRegister::Register::transactionFactory(m_register, MyMoneyTransaction(), split, 0);

    m_register->updateRegister(true);

    if(focusItem) {
      // in case we have some selected items we just set the focus item
      // in other cases, we make the focusitem also the selected item
      if(isSelected.count() > 1) {
        m_register->setFocusItem(focusItem);
        m_register->setAnchorItem(anchorItem);
      } else
        m_register->selectItem(focusItem, true);
    } else {
      // just use the empty line at the end if nothing else exists in the ledger
      p = m_register->lastItem();
      m_register->setFocusItem(p);
      m_register->selectItem(p);
      focusItem = p;
    }

    updateSummaryLine(actBalance, clearedBalance);
    kmymoney2->slotStatusProgressBar(-1, -1);

  } catch(MyMoneyException *e) {
    delete e;
    m_account = MyMoneyAccount();
    clear();
  }

  // (re-)position viewport
  if(m_newAccountLoaded) {
    if(focusItem) {
      d->m_startPoint = QPoint(-1, -1);
    } else {
      d->m_startPoint = QPoint(0, 0);
    }
  }
  if(!d->m_inLoading) {
    d->m_viewPosTimer.start(30, true);
    d->m_inLoading = true;
  }

  d->m_showDetails = KMyMoneyGlobalSettings::showRegisterDetailed();

  // and tell everyone what's selected
  emit accountSelected(m_account);
}

void KGlobalLedgerView::updateSummaryLine(const QMap<QString, MyMoneyMoney>& actBalance, const QMap<QString, MyMoneyMoney>& clearedBalance)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  m_leftSummaryLabel->show();
  m_centerSummaryLabel->show();
  m_rightSummaryLabel->show();

  if(isReconciliationAccount()) {
    if(m_account.accountType() != MyMoneyAccount::Investment) {
      m_leftSummaryLabel->setText(i18n("Statement: %1").arg(d->m_endingBalance.formatMoney("", d->m_precision)));
      m_centerSummaryLabel->setText(i18n("Cleared: %1").arg(clearedBalance[m_account.id()].formatMoney("", d->m_precision)));
      m_rightSummaryLabel->setText(i18n("Difference: %1").arg((clearedBalance[m_account.id()] - d->m_endingBalance).formatMoney("", d->m_precision)));
    }
  } else {
    // update summary line in normal mode
    QDate reconcileDate = m_account.lastReconciliationDate();

    if(reconcileDate.isValid()) {
      m_leftSummaryLabel->setText(i18n("Last reconciled: %1").arg(KGlobal::locale()->formatDate(reconcileDate, true)));
    } else {
      m_leftSummaryLabel->setText(i18n("Never reconciled"));
    }

    if(m_account.accountType() != MyMoneyAccount::Investment) {
      m_centerSummaryLabel->setText(i18n("Cleared: %1").arg(clearedBalance[m_account.id()].formatMoney("", d->m_precision)));
      m_rightSummaryLabel->setText(i18n("Balance: %1").arg(actBalance[m_account.id()].formatMoney("", d->m_precision)));
    } else {
      m_centerSummaryLabel->hide();
      MyMoneyMoney balance;
      MyMoneySecurity base = file->baseCurrency();
      QMap<QString, MyMoneyMoney>::const_iterator it_b;
      bool approx = false;
      for(it_b = actBalance.begin(); it_b != actBalance.end(); ++it_b) {
        MyMoneyAccount stock = file->account(it_b.key());
        QString currencyId = stock.currencyId();
        MyMoneySecurity sec = file->security(currencyId);
        MyMoneyPrice priceInfo;
        MyMoneyMoney rate(1,1);

        if(stock.isInvest()) {
          currencyId = sec.tradingCurrency();
          priceInfo = file->price(sec.id(), currencyId);
          approx |= !priceInfo.isValid();
          rate = priceInfo.rate(sec.tradingCurrency());
        }

        if(currencyId != base.id()) {
          priceInfo = file->price(sec.tradingCurrency(), base.id());
          approx |= !priceInfo.isValid();
          rate = (rate * priceInfo.rate(base.id())).convert(MyMoneyMoney::precToDenom(KMyMoneyGlobalSettings::pricePrecision()));
        }
        balance += ((*it_b) * rate).convert(base.smallestAccountFraction());
      }
      m_rightSummaryLabel->setText(i18n("Investment value: %1%2").arg(approx ? "~" : "").arg(balance.formatMoney(base.tradingSymbol(), d->m_precision)));
    }
  }
}

void KGlobalLedgerView::slotUpdateViewPos(void)
{
  m_register->setUpdatesEnabled(true);

  if(d->m_startPoint == QPoint(-1, -1)) {
    m_register->ensureItemVisible(m_register->focusItem());
    m_register->updateContents();
  } else {
    m_register->setContentsPos(d->m_startPoint.x(), d->m_startPoint.y());
    m_register->repaintContents();
  }
  d->m_inLoading = false;
}


void KGlobalLedgerView::resizeEvent(QResizeEvent* ev)
{
  m_register->resize(KMyMoneyRegister::DetailColumn);
  m_form->resize(KMyMoneyTransactionForm::ValueColumn1);
  KMyMoneyViewBase::resizeEvent(ev);
}

void KGlobalLedgerView::loadAccounts(void)
{
  MyMoneyFile* file = MyMoneyFile::instance();

  // check if the current account still exists and make it the
  // current account
  if(!m_account.id().isEmpty()) {
    try {
      m_account = file->account(m_account.id());
    } catch(MyMoneyException *e) {
      delete e;
      m_account = MyMoneyAccount();
    }
  }

  m_accountComboBox->loadList((KMyMoneyUtils::categoryTypeE)(KMyMoneyUtils::asset | KMyMoneyUtils::liability));

  if(m_account.id().isEmpty()) {
    QStringList list = m_accountComboBox->accountList();
    if(list.count()) {
      QStringList::Iterator it;
      for(it = list.begin(); it != list.end(); ++it) {
        MyMoneyAccount a = file->account(*it);
        if(!a.isInvest()) {
          if(a.value("PreferredAccount") == "Yes") {
            m_account = a;
            break;
          } else if(m_account.id().isEmpty()) {
            m_account = a;
          }
        }
      }
    }
  }

  if(!m_account.id().isEmpty()) {
    m_accountComboBox->setSelected(m_account);
    try {
      d->m_precision = MyMoneyMoney::denomToPrec(m_account.fraction());
    } catch(MyMoneyException *e) {
      qDebug("Security %s for account %s not found", m_account.currencyId().data(), m_account.name().data());
      delete e;
      d->m_precision = 2;
    }
  }
}

void KGlobalLedgerView::selectTransaction(const QString& id)
{
  if(!id.isEmpty()) {
    KMyMoneyRegister::RegisterItem* p = m_register->lastItem();
    while(p) {
      KMyMoneyRegister::Transaction* t = dynamic_cast<KMyMoneyRegister::Transaction*>(p);
      if(t) {
        if(t->transaction().id() == id) {
          m_register->selectItem(t);
          m_register->ensureItemVisible(t);
          break;
        }
      }
      p = p->prevItem();
    }
  }
}

void KGlobalLedgerView::slotSelectAllTransactions(void)
{
  KMyMoneyRegister::RegisterItem* p = m_register->firstItem();
  while(p) {
    KMyMoneyRegister::Transaction* t = dynamic_cast<KMyMoneyRegister::Transaction*>(p);
    if(t) {
      if(t->isVisible() && t->isSelectable() && !t->isScheduled() && !t->id().isEmpty()) {
        t->setSelected(true);
      }
    }
    p = p->nextItem();
  }
  m_register->repaintItems();

  // inform everyone else about the selected items
  KMyMoneyRegister::SelectedTransactions list(m_register);
  emit transactionsSelected(list);
}

void KGlobalLedgerView::slotSetReconcileAccount(const MyMoneyAccount& acc, const QDate& reconciliationDate, const MyMoneyMoney& endingBalance)
{
  if(d->m_reconciliationAccount != acc.id()) {
    // make sure the account is selected
    if(!acc.id().isEmpty())
      slotSelectAccount(acc.id());

    d->m_reconciliationAccount = acc.id();
    d->m_reconciliationDate = reconciliationDate;
    d->m_endingBalance = endingBalance;
    if(acc.accountGroup() == MyMoneyAccount::Liability)
      d->m_endingBalance = -endingBalance;

    m_newAccountLoaded = true;
    if(acc.id().isEmpty()) {
      kmymoney2->action("account_reconcile_postpone")->unplug(m_buttonbar);
      kmymoney2->action("account_reconcile_finish")->unplug(m_buttonbar);
    } else {
      kmymoney2->action("account_reconcile_postpone")->plug(m_buttonbar);
      kmymoney2->action("account_reconcile_finish")->plug(m_buttonbar);
      // when we start reconciliation, we need to reload the view
      // because no data has been changed. When postponing or finishing
      // reconciliation, the data change in the engine takes care of updateing
      // the view.
      slotLoadView();
    }
  }
}

bool KGlobalLedgerView::isReconciliationAccount(void) const
{
  return m_account.id() == d->m_reconciliationAccount;
}

bool KGlobalLedgerView::slotSelectAccount(const MyMoneyObject& obj)
{
  if(typeid(obj) != typeid(MyMoneyAccount))
    return false;

  if(d->m_recursion)
    return false;

  d->m_recursion = true;
  const MyMoneyAccount& acc = dynamic_cast<const MyMoneyAccount&>(obj);
  bool rc = slotSelectAccount(acc.id());
  d->m_recursion = false;
  return rc;
}

bool KGlobalLedgerView::slotSelectAccount(const QString& id, const QString& transactionId)
{
  bool    rc = true;

  if(!id.isEmpty()) {
    if(m_account.id() != id) {
      try {
        m_account = MyMoneyFile::instance()->account(id);
        // if a stock account is selected, we show the
        // the corresponding parent (investment) account
        if(m_account.isInvest()) {
          m_account = MyMoneyFile::instance()->account(m_account.parentAccountId());
        }
        m_newAccountLoaded = true;
        slotLoadView();
      } catch(MyMoneyException* e) {
        qDebug("Unable to retrieve account %s", id.data());
        delete e;
        rc = false;
      }
    } else {
      // we need to refresh m_account.m_accountList, a child could have been deleted
      m_account = MyMoneyFile::instance()->account(id);
      emit accountSelected(m_account);
    }
    selectTransaction(transactionId);
  }
  return rc;
}

void KGlobalLedgerView::slotNewTransaction(KMyMoneyRegister::Action id)
{
  if(!m_inEditMode) {
    d->m_action = id;
    emit newTransaction();
  }
}

void KGlobalLedgerView::slotNewTransaction(void)
{
  slotNewTransaction(KMyMoneyRegister::ActionNone);
}

void KGlobalLedgerView::setupDefaultAction(void)
{
  switch(m_account.accountType()) {
    // TODO if we want a different action for different account types
    //      we can add cases here
    default:
      d->m_action = KMyMoneyRegister::ActionWithdrawal;
      break;
  }
}

bool KGlobalLedgerView::selectEmptyTransaction(void)
{
  bool rc = false;

  if(!m_inEditMode) {
    // in case we don't know the type of transaction to be created,
    // have at least one selected transaction and the id of
    // this transaction is not empty, we take it as template for the
    // transaction to be created
    KMyMoneyRegister::SelectedTransactions list(m_register);
    if((d->m_action == KMyMoneyRegister::ActionNone) && (list.count() > 0) && (!list[0].transaction().id().isEmpty())) {
      // the new transaction to be created will have the same type
      // as the one that currently has the focus
      KMyMoneyRegister::Transaction* t = dynamic_cast<KMyMoneyRegister::Transaction*>(m_register->focusItem());
      if(t)
        d->m_action = t->actionType();
      m_register->clearSelection();
    }

    // if we still don't have an idea which type of transaction
    // to create, we use the default.
    if(d->m_action == KMyMoneyRegister::ActionNone) {
      setupDefaultAction();
    }

    m_register->selectItem(m_register->lastItem());
    m_register->updateRegister();
    rc = true;
  }
  return rc;
}

TransactionEditor* KGlobalLedgerView::startEdit(const KMyMoneyRegister::SelectedTransactions& list)
{
  // we use the warnlevel to keep track, if we have to warn the
  // user that some or all splits have been reconciled or if the
  // user cannot modify the transaction if at least one split
  // has the status frozen. The following value are used:
  //
  // 0 - no sweat, user can modify
  // 1 - user should be warned that at least one split has been reconciled
  //     already
  // 2 - user will be informed, that this transaction cannot be changed anymore

  int warnLevel = list.warnLevel();
  Q_ASSERT(warnLevel<2);  // otherwise the edit action should not be enabled

  switch(warnLevel) {
    case 0:
      break;

    case 1:
      if(KMessageBox::warningContinueCancel(0,
        i18n(
          "At least one split of the selected transactions has been reconciled. "
          "Do you wish to continue to edit the transactions anyway?"
        ),
        i18n("Transaction already reconciled"), KStdGuiItem::cont(),
        "EditReconciledTransaction") == KMessageBox::Cancel) {
        warnLevel = 2;
      }
      break;

    case 2:
      KMessageBox::sorry(0,
            i18n("At least one split of the selected transactions has been frozen. "
                 "Editing the transactions is therefore prohibited."),
            i18n("Transaction already frozen"));
      break;

    case 3:
      KMessageBox::sorry(0,
            i18n("At least one split of the selected transaction references an account that has been closed. "
                 "Editing the transactions is therefore prohibited."),
            i18n("Account closed"));
      break;
  }

  if(warnLevel > 1)
    return 0;


  TransactionEditor* editor = 0;
  KMyMoneyRegister::Transaction* item = dynamic_cast<KMyMoneyRegister::Transaction*>(m_register->focusItem());

  if(item) {
    // in case the current focus item is not selected, we move the focus to the first selected transaction
    if(!item->isSelected()) {
      KMyMoneyRegister::RegisterItem* p;
      for(p = m_register->firstItem(); p; p = p->nextItem()) {
        KMyMoneyRegister::Transaction* t = dynamic_cast<KMyMoneyRegister::Transaction*>(p);
        if(t && t->isSelected()) {
          m_register->setFocusItem(t);
          item = t;
          break;
        }
      }
    }

    // decide, if we edit in the register or in the form
    TransactionEditorContainer* parent;
    if(m_formFrame->isVisible())
      parent = m_form;
    else {
      parent = m_register;
    }

    editor = item->createEditor(parent, list, m_lastPostDate);

    // check that we use the same transaction commodity in all selected transactions
    // if not, we need to update this in the editor's list. The user can also bail out
    // of this operation which means that we have to stop editing here.
    if(editor) {
      if(!editor->fixTransactionCommodity(m_account)) {
        // if the user wants to quit, we need to destroy the editor
        // and bail out
        delete editor;
        editor = 0;
      }
    }

    if(editor) {
      if(parent == m_register) {
        // make sure, the height of the table is correct
        m_register->updateRegister(KMyMoneyGlobalSettings::ledgerLens() | !KMyMoneyGlobalSettings::transactionForm());
      }

      m_inEditMode = true;
      connect(editor, SIGNAL(transactionDataSufficient(bool)), kmymoney2->action("transaction_enter"), SLOT(setEnabled(bool)));
      connect(editor, SIGNAL(returnPressed()), kmymoney2->action("transaction_enter"), SLOT(activate()));
      connect(editor, SIGNAL(escapePressed()), kmymoney2->action("transaction_cancel"), SLOT(activate()));

      connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), editor, SLOT(slotReloadEditWidgets()));
      connect(editor, SIGNAL(finishEdit(const KMyMoneyRegister::SelectedTransactions&)), this, SLOT(slotLeaveEditMode(const KMyMoneyRegister::SelectedTransactions&)));

      connect(editor, SIGNAL(objectCreation(bool)), d->m_mousePressFilter, SLOT(setFilterDeactive(bool)));
      connect(editor, SIGNAL(createPayee(const QString&, QString&)), kmymoney2, SLOT(slotPayeeNew(const QString&, QString&)));
      connect(editor, SIGNAL(createCategory(MyMoneyAccount&, const MyMoneyAccount&)), kmymoney2, SLOT(slotCategoryNew(MyMoneyAccount&, const MyMoneyAccount&)));
      connect(editor, SIGNAL(createSecurity(MyMoneyAccount&, const MyMoneyAccount&)), kmymoney2, SLOT(slotInvestmentNew(MyMoneyAccount&, const MyMoneyAccount&)));
      connect(editor, SIGNAL(assignNumber(void)), kmymoney2, SLOT(slotTransactionAssignNumber()));
      connect(editor, SIGNAL(lastPostDateUsed(const QDate&)), this, SLOT(slotKeepPostDate(const QDate&)));

      // create the widgets, place them in the parent and load them with data
      // setup tab order
      m_tabOrderWidgets.clear();
      editor->setup(m_tabOrderWidgets, m_account, d->m_action);

      Q_ASSERT(!m_tabOrderWidgets.isEmpty());

      // install event filter in all taborder widgets
      for(QWidget* w = m_tabOrderWidgets.first(); w; w = m_tabOrderWidgets.next()) {
        w->installEventFilter(this);
      }

      // Install a filter that checks if a mouse press happened outside
      // of one of our own widgets.
      qApp->installEventFilter(d->m_mousePressFilter);

      // Check if the editor has some preference on where to set the focus
      // If not, set the focus to the first widget in the tab order
      QWidget* focusWidget = editor->firstWidget();
      if(!focusWidget)
        focusWidget = m_tabOrderWidgets.first();

      // for some reason, this only works reliably if delayed a bit
      QTimer::singleShot(10, focusWidget, SLOT(setFocus()));

      // preset to 'I have no idea which type to create' for the next round.
      d->m_action = KMyMoneyRegister::ActionNone;
    }
  }
  return editor;
}

void KGlobalLedgerView::slotLeaveEditMode(const KMyMoneyRegister::SelectedTransactions& list)
{
  m_inEditMode = false;
  qApp->removeEventFilter(d->m_mousePressFilter);

  // a possible focusOut event may have removed the focus, so we
  // install it back again.
  m_register->focusItem()->setFocus(true);

  // if we come back from editing a new item, we make sure that
  // we always select the very last known transaction entry no
  // matter if the transaction has been created or not.

  if(list.count() && list[0].transaction().id().isEmpty()) {
    // block signals to prevent some infinite loops that might occur here.
    m_register->blockSignals(true);
    m_register->clearSelection();
    KMyMoneyRegister::RegisterItem* p = m_register->lastItem();
    if(p && p->prevItem())
      p = p->prevItem();
    m_register->selectItem(p);
    m_register->updateRegister(true);
    m_register->blockSignals(false);
    // we need to update the form manually as sending signals was blocked
    KMyMoneyRegister::Transaction* t = dynamic_cast<KMyMoneyRegister::Transaction*>(p);
    if(t)
      m_form->slotSetTransaction(t);
  }

  if(m_needReload)
    slotLoadView();

  m_register->setFocus();
}

bool KGlobalLedgerView::focusNextPrevChild(bool next)
{
  bool  rc = false;

  // qDebug("KGlobalLedgerView::focusNextPrevChild(editmode=%s)", m_inEditMode ? "true" : "false");
  if(m_inEditMode) {
    QWidget *w = 0;
    QWidget *currentWidget;

    w = qApp->focusWidget();
    while(w && m_tabOrderWidgets.find(w) == -1) {
      // qDebug("'%s' not in list, use parent", w->className());
      w = w->parentWidget();
    }
    // if(w) qDebug("tab order is at '%s'", w->className());
    currentWidget = m_tabOrderWidgets.current();
    w = next ? m_tabOrderWidgets.next() : m_tabOrderWidgets.prev();

    do {
      if(!w) {
        w = next ? m_tabOrderWidgets.first() : m_tabOrderWidgets.last();
      }

      if(w != currentWidget
      && ((w->focusPolicy() & TabFocus) == TabFocus)
      && w->isVisible() && w->isEnabled()) {
        // qDebug("Selecting '%s' as focus", w->className());
        w->setFocus();
        rc = true;
        break;
      }
      w = next ? m_tabOrderWidgets.next() : m_tabOrderWidgets.prev();
    } while(w != currentWidget);

  } else
    rc = KMyMoneyViewBase::focusNextPrevChild(next);

  return rc;
}


void KGlobalLedgerView::show(void)
{
  if(m_needReload) {
    if(!m_inEditMode) {
      loadView();
      m_needReload = false;
      m_newAccountLoaded = false;
    }

  } else {
    emit accountSelected(m_account);
    KMyMoneyRegister::SelectedTransactions list(m_register);
    emit transactionsSelected(list);
  }

  // don't forget base class implementation
  KMyMoneyViewBase::show();
}

bool KGlobalLedgerView::eventFilter(QObject* o, QEvent* e)
{
  bool rc = false;

  if(e->type() == QEvent::KeyPress) {
    QKeyEvent *k = static_cast<QKeyEvent*>(e);
    if(m_inEditMode) {
      // qDebug("object = %s, key = %d", o->className(), k->key());
      if(o == m_register) {
        // we hide all key press events from the register
        // while editing a transaction
        rc = true;
      }
    } else {
      // qDebug("object = %s, key = %d", o->className(), k->key());
      if(o == m_register) {
        if((k->state() & Qt::KeyButtonMask) == 0) {
          switch(k->key()) {
            case Qt::Key_Return:
            case Qt::Key_Enter:
              kmymoney2->action("transaction_edit")->activate();
              rc = true;
              break;
          }
        }
      }
    }
  }

  if(!rc)
    rc = KMyMoneyViewBase::eventFilter(o, e);

  return rc;
}

void KGlobalLedgerView::slotSortOptions(void)
{
  KSortOptionDlg* dlg = new KSortOptionDlg(this);

  QString key;
  QString sortOrder, def;
  if(isReconciliationAccount()) {
    key = "kmm-sort-reconcile";
    def = KMyMoneyGlobalSettings::sortReconcileView();
  } else {
    key = "kmm-sort-std";
    def = KMyMoneyGlobalSettings::sortNormalView();
  }

  // check if we have an account override of the sort order
  if(!m_account.value(key).isEmpty())
    sortOrder = m_account.value(key);

  QString oldOrder = sortOrder;

  dlg->setSortOption(sortOrder, def);

  if(dlg->exec() == QDialog::Accepted) {
    sortOrder = dlg->sortOption();
    if(sortOrder != oldOrder) {
      if(sortOrder.isEmpty()) {
        m_account.deletePair(key);
      } else {
        m_account.setValue(key, sortOrder);
      }
      MyMoneyFileTransaction ft;
      try {
        MyMoneyFile::instance()->modifyAccount(m_account);
        ft.commit();
      } catch(MyMoneyException* e) {
        qDebug("Unable to update sort order for account '%s': %s", m_account.name().latin1(), e->what().latin1());
        delete e;
      }
    }
  }
  delete dlg;
}

void KGlobalLedgerView::slotToggleTransactionMark(KMyMoneyRegister::Transaction* /* t */)
{
  if(!m_inEditMode) {
    emit toggleReconciliationFlag();
  }
}

void KGlobalLedgerView::slotKeepPostDate(const QDate& date)
{
  m_lastPostDate = date;
}

bool KGlobalLedgerView::canCreateTransactions(QString& tooltip) const
{
  bool rc = true;
  if(m_account.id().isEmpty()) {
    tooltip = i18n("Cannot create transactions when no account is selected.");
    rc = false;
  }
  if(m_account.accountGroup() == MyMoneyAccount::Income
  || m_account.accountGroup() == MyMoneyAccount::Expense) {
    tooltip = i18n("Cannot create transactions in the context of a category.");
    rc = false;
  }
  if(m_account.isClosed()) {
    tooltip = i18n("Cannot create transactions in a closed account.");
    rc = false;
  }
  return rc;
}

bool KGlobalLedgerView::canProcessTransactions(const KMyMoneyRegister::SelectedTransactions& list, QString& tooltip) const
{
  if(m_register->focusItem() == 0)
    return false;

  if(!m_register->focusItem()->isSelected()) {
    tooltip = i18n("Cannot process transaction with focus if it is not selected.");
    return false;
  }
  return list.count() > 0;
}

bool KGlobalLedgerView::canModifyTransactions(const KMyMoneyRegister::SelectedTransactions& list, QString& tooltip) const
{
  return canProcessTransactions(list,tooltip) && list.canModify();
}

bool KGlobalLedgerView::canDuplicateTransactions(const KMyMoneyRegister::SelectedTransactions& list, QString& tooltip) const
{
  return canProcessTransactions(list,tooltip) && list.canDuplicate();
}

bool KGlobalLedgerView::canEditTransactions(const KMyMoneyRegister::SelectedTransactions& list, QString& tooltip) const
{
  // check if we can edit the list of transactions. We can edit, if
  //
  //   a) no mix of standard and investment transactions exist
  //   b) if a split transaction is selected, this is the only selection
  //   c) none of the splits is frozen
  //   d) the transaction having the current focus is selected

  // check for d)
  if(!canProcessTransactions(list, tooltip))
    return false;
  // check for c)
  if (list.warnLevel() == 2) {
    tooltip = i18n("Cannot edit transactions with frozen splits.");
    return false;
  }


  bool rc = true;
  int investmentTransactions = 0;
  int normalTransactions = 0;

  if(m_account.accountGroup() == MyMoneyAccount::Income
  || m_account.accountGroup() == MyMoneyAccount::Expense) {
    tooltip = i18n("Cannot edit transactions in the context of a category.");
    rc = false;
  }

  KMyMoneyRegister::SelectedTransactions::const_iterator it_t;
  for(it_t = list.begin(); rc && it_t != list.end(); ++it_t) {
    if((*it_t).transaction().id().isEmpty()) {
      tooltip = QString();
      rc = false;
      continue;
    }

    if(KMyMoneyUtils::transactionType((*it_t).transaction()) == KMyMoneyUtils::InvestmentTransaction)
      ++investmentTransactions;
    else
      ++normalTransactions;

    // check for a)
    if(investmentTransactions != 0 && normalTransactions != 0) {
      tooltip = i18n("Cannot edit investment transactions and non-investment transactions together.");
      rc = false;
      break;
    }

    // check for b) but only for normalTransactions
    if((*it_t).transaction().splitCount() > 2 && normalTransactions != 0) {
      if(list.count() > 1) {
        tooltip = i18n("Cannot edit multiple split transactions at once.");
        rc = false;
        break;
      }
    }
  }

  // now check that we have the correct account type for investment transactions
  if(rc == true && investmentTransactions != 0) {
    if(m_account.accountType() != MyMoneyAccount::Investment) {
      tooltip = i18n("Cannot edit investment transactions in the context of this account.");
      rc = false;
    }
  }
  return rc;
}

#include "kgloballedgerview.moc"
