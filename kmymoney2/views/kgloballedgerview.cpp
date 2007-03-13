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

// ----------------------------------------------------------------------------
// QT Includes

#include <qcstring.h>
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

// ----------------------------------------------------------------------------
// Project Includes

#include "kgloballedgerview.h"

#include <kmymoney/mymoneyaccount.h>
#include <kmymoney/mymoneyfile.h>
#include <kmymoney/kmymoneyaccountcombo.h>
#include <kmymoney/kmymoneytitlelabel.h>
#include <kmymoney/register.h>
#include <kmymoney/mymoneyobjectcontainer.h>
#include <kmymoney/transactioneditor.h>
#include <kmymoney/selectedtransaction.h>

#include <kmymoney/kmymoneyglobalsettings.h>

#include "../widgets/registersearchline.h"
#include "../dialogs/ksortoptiondlg.h"
#include "../kmymoney2.h"

class KGlobalLedgerViewPrivate
{
public:
  KGlobalLedgerViewPrivate();

  MousePressFilter*    m_mousePressFilter;
  KMyMoneyRegister::RegisterSearchLineWidget* m_registerSearchLine;
  QPoint               m_startPoint;
  QCString             m_reconciliationAccount;
  MyMoneyMoney         m_endingBalance;
  int                  m_precision;
  bool                 m_inLoading;
  bool                 m_recursion;
  KMyMoneyRegister::Action m_action;
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
        emit mousePressedOnExternalWidget();
      }
    }

    if(e->type() != QEvent::MouseButtonPress) {
      m_lastMousePressEvent = 0;
    }
  }
  return false;
}


KGlobalLedgerViewPrivate::KGlobalLedgerViewPrivate() :
  m_mousePressFilter(0),
  m_registerSearchLine(0),
  m_inLoading(false),
  m_recursion(false)
{
}

QDate KGlobalLedgerView::m_lastPostDate;

KGlobalLedgerView::KGlobalLedgerView(QWidget *parent, const char *name )
  : KMyMoneyViewBase(parent, name, i18n("Ledgers")),
  d(new KGlobalLedgerViewPrivate),
  m_objects(0),
  m_needReload(false),
  m_newAccountLoaded(true),
  m_inEditMode(false)
{
  d->m_mousePressFilter = new MousePressFilter((QWidget*)this);
  setupDefaultAction();

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
  connect(m_register, SIGNAL(reconcileStateColumnClicked(KMyMoneyRegister::Transaction*)), this, SLOT(slotToggleMarkTransactionCleared(KMyMoneyRegister::Transaction*)));

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
  connect(m_accountComboBox, SIGNAL(accountSelected(const QCString&)), this, SLOT(slotSelectAccount(const QCString&)));
  connect(m_register, SIGNAL(selectionChanged(const QValueList<KMyMoneyRegister::SelectedTransaction>&)), this, SIGNAL(transactionsSelected(const QValueList<KMyMoneyRegister::SelectedTransaction>&)));
  connect(m_register, SIGNAL(editTransaction()), this, SIGNAL(startEdit()));
  connect(m_register, SIGNAL(emptyItemSelected()), this, SLOT(slotNewTransaction()));
  connect(m_register, SIGNAL(aboutToSelectItem(KMyMoneyRegister::RegisterItem*)), this, SIGNAL(cancelOrEndEdit()));
  connect(d->m_mousePressFilter, SIGNAL(mousePressedOnExternalWidget()), this, SIGNAL(cancelOrEndEdit()));

  connect(m_form, SIGNAL(newTransaction(KMyMoneyRegister::Action)), this, SLOT(slotNewTransaction(KMyMoneyRegister::Action)));

  // create object container for this view
  m_objects = new MyMoneyObjectContainer();
  connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), m_objects, SLOT(clear()));

  // setup mouse press filter
  d->m_mousePressFilter->addWidget(m_formFrame);
  d->m_mousePressFilter->addWidget(m_buttonFrame);
  d->m_mousePressFilter->addWidget(m_summaryFrame);
  d->m_mousePressFilter->addWidget(m_registerFrame);


#if 0
  m_currentView = 0;
  KLedgerView* view;

  for(int i = 0; i < MyMoneyAccount::MaxAccountTypes; ++i)
    m_specificView[i] = 0;

  if ( !name )
    setName( "Account register" );

  setCaption( i18n( "Account register" ) );

  KIconLoader *il = KGlobal::iconLoader();

  m_toolbar = new KToolBar(this, "LedgerToolBar", true);
  m_toolbar->setIconText(KToolBar::IconTextRight);

  m_accountComboBox = new KMyMoneyAccountCombo(m_toolbar, "AccountCombo");
  m_toolbar->insertWidget(1,100,m_accountComboBox);

  m_toolbar->insertButton(il->loadIcon("document", KIcon::Small, KIcon::SizeSmall),
                        1,true,i18n("Account"));
  //m_toolbar->setMaximumSize(50,20);
  m_toolbar->alignItemRight(1);
  m_toolbar->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Fixed);
  // KToolBarButton* m_buttonAccount = m_toolbar->getButton(1);

  m_viewLayout->addWidget(m_toolbar);

  m_accountStack = new QWidgetStack(this, "AccountStack");

  // Checkings account
  view = m_specificView[MyMoneyAccount::Checkings] = new KLedgerViewCheckings(this, "CheckingsView");
  m_accountStack->addWidget(view, MyMoneyAccount::Checkings);

  // connect(this, SIGNAL(cancelEdit()), view, SLOT(slotCancelEdit()));

  // Savings account
  view = m_specificView[MyMoneyAccount::Savings] = new KLedgerViewSavings(this, "SavingsView");
  m_accountStack->addWidget(view, MyMoneyAccount::Savings);

  // Credit card account
  view = m_specificView[MyMoneyAccount::CreditCard] = new KLedgerViewCreditCard(this, "CreditCardView");
  m_accountStack->addWidget(view, MyMoneyAccount::CreditCard);

  // Cash account
  view = m_specificView[MyMoneyAccount::Cash] = new KLedgerViewCash(this, "CashView");
  m_accountStack->addWidget(view, MyMoneyAccount::Cash);

  // Asset account
  view = m_specificView[MyMoneyAccount::Asset] = new KLedgerViewAsset(this, "AssetView");
  m_accountStack->addWidget(view, MyMoneyAccount::Asset);

  // Loan account
  view = m_specificView[MyMoneyAccount::Loan] = m_specificView[MyMoneyAccount::AssetLoan] = new KLedgerViewLoan(this, "LoanView");
  m_accountStack->addWidget(view, MyMoneyAccount::Loan);

  // Liability account
  view = m_specificView[MyMoneyAccount::Liability] = new KLedgerViewLiability(this, "LiabilityView");
  m_accountStack->addWidget(view, MyMoneyAccount::Liability);

#if 0
  // FIXME: I removed the below code to enable switching to the investment view
  // when an investment account is selected via the combo box.  People reported,
  // that the comboboxes on the globalledgerview and the investmentview are not
  // in sync.  Not providing an investment ledger view within the global ledger view
  // solves this problem.

  // Investment account
  view = m_specificView[MyMoneyAccount::Investment] = new KLedgerViewInvestments(this, "InvestmentsView");
  m_accountStack->addWidget(view, MyMoneyAccount::Investment);
  connect(view, SIGNAL(accountAndTransactionSelected(const QCString&, const QCString&)),
    this, SLOT(slotSelectAccount(const QCString&, const QCString&)));
  connect(view, SIGNAL(payeeSelected(const QCString&, const QCString&, const QCString&)),
    SIGNAL(payeeSelected(const QCString&, const QCString&, const QCString&)));
  // connect(this, SIGNAL(cancelEdit()), view, SLOT(slotCancelEdit()));
#endif

  // connect signals that are identical for all views
  for(int i=0; i < MyMoneyAccount::MaxAccountTypes; ++i) {
    setupConnections(m_specificView[i]);
  }

  m_viewLayout->addWidget(m_accountStack);
  setMinimumHeight(m_accountComboBox->minimumHeight() + m_accountStack->sizeHint().height());

  m_accountId = QCString();

  MyMoneyFile::instance()->attach(MyMoneyFile::NotifyClassAccount, this);

  // setup connections
  connect(m_accountComboBox, SIGNAL(accountSelected(const QCString&)),
          this, SLOT(slotSelectAccount(const QCString&)));
#endif
  // setup connections
}

KGlobalLedgerView::~KGlobalLedgerView()
{
  delete m_objects;
  delete d;
}

void KGlobalLedgerView::slotLoadView(void)
{
  if(!d->m_inLoading) {
    m_needReload = true;
    if(isVisible()) {
      if(!m_inEditMode) {
        d->m_inLoading = true;
        loadView();
        m_needReload = false;
        // force a new account if the current one is empty
        m_newAccountLoaded = m_account.id().isEmpty();
      }
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

  // the object cache
  m_objects->clear();

  // and the selected account in the combo box
  m_accountComboBox->setSelected(QCString());

  // fraction defaults to two digits
  d->m_precision = 2;
}

void KGlobalLedgerView::loadView(void)
{
  // setup form visibility
  m_formFrame->setShown(KMyMoneyGlobalSettings::transactionForm());

  // no account selected
  emit accountSelected(MyMoneyAccount());
  // no transaction selected
  QValueList<KMyMoneyRegister::SelectedTransaction> list;
  emit transactionsSelected(list);
  // no match transaction selected
  m_matchTransaction = MyMoneyTransaction();

  QMap<QCString, bool> isSelected;
  QCString focusItemId;

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

    // remember if a transaction has a match mark
    KMyMoneyRegister::Transaction* t = dynamic_cast<KMyMoneyRegister::Transaction*>(item);
    if(t && t->hasMatchMark())
      m_matchTransaction = t->transaction();

    // remember the upper left corner of the viewport
    d->m_startPoint = QPoint(m_register->contentsX(), m_register->contentsY());
  } else
    d->m_registerSearchLine->searchLine()->reset();

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
    d->m_inLoading = false;
    setEnabled(false);
    return;
  }
  setEnabled(true);

  m_register->setUpdatesEnabled(false);

  // ... and recreate it
  KMyMoneyRegister::RegisterItem* focusItem = 0;
  MyMoneyMoney actBalance, clearedBalance, futureBalance;
  try {
    // setup the filter to select the transactions we want to display
    // and update the sort order
    QString sortOrder;
    QCString key;

    MyMoneyTransactionFilter filter(m_account.id());
    // if it's an investment account, we also take care of
    // the sub-accounts (stock accounts)
    if(m_account.accountType() == MyMoneyAccount::Investment)
      filter.addAccount(m_account.accountList());

    if(isReconciliationAccount()) {
      key = "kmm-sort-reconcile";
      sortOrder = KMyMoneySettings::sortReconcileView();
      filter.addState(MyMoneyTransactionFilter::notReconciled);
      filter.addState(MyMoneyTransactionFilter::cleared);
    } else {
      filter.setDateFilter(KMyMoneySettings::startDate().date(), QDate());
      key = "kmm-sort-std";
      sortOrder = KMyMoneySettings::sortNormalView();
      if (KMyMoneySettings::hideReconciledTransactions()) {
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

    // create the elements for the register
    QValueList<QPair<MyMoneyTransaction, MyMoneySplit> >::const_iterator it;
    QMap<QCString, int>uniqueMap;
    for(it = m_transactionList.begin(); it != m_transactionList.end(); ++it) {
      uniqueMap[(*it).first.id()]++;
      KMyMoneyRegister::Register::transactionFactory(m_register, m_objects, (*it).first, (*it).second, uniqueMap[(*it).first.id()]);
    }

    // add the group markers
    addGroupMarkers();

    // sort the transactions according to the sort setting
    m_register->sortItems();

    // remove all trailing group markers
    KMyMoneyRegister::RegisterItem* p = m_register->lastItem();
    while(p) {
      KMyMoneyRegister::Transaction* t = dynamic_cast<KMyMoneyRegister::Transaction*>(p);
      if(t)
        break;
      KMyMoneyRegister::RegisterItem* q = p;
      p = p->prevItem();
      delete q;
    }

    // remove all adjacent group markers
    bool lastWasGroupMarker = false;
    p = m_register->lastItem();
    while(p) {
      KMyMoneyRegister::GroupMarker* m = dynamic_cast<KMyMoneyRegister::GroupMarker*>(p);
      p = p->prevItem();
      if(m) {
        if(lastWasGroupMarker) {
          delete m;
        }
        lastWasGroupMarker = true;
      } else
        lastWasGroupMarker = false;
    }


    // determine balances (actual, cleared). We do this by getting the actual
    // balance of all entered transactions from the engine and walk the list
    // of transactions backward. Also re-select a transaction if it was
    // selected before and setup the focus item.
    MyMoneyMoney balance = MyMoneyFile::instance()->balance(m_account.id());
    MyMoneyMoney factor(1,1);
    if(m_account.accountGroup() == MyMoneyAccount::Liability)
      factor = -factor;

    balance = balance * factor;
    actBalance = clearedBalance = futureBalance = balance;
    p = m_register->lastItem();
    focusItem = p;
    while(p) {
      KMyMoneyRegister::Transaction* t = dynamic_cast<KMyMoneyRegister::Transaction*>(p);
      if(t) {
        if(isSelected.contains(t->id()))
          t->setSelected(true);

        if(t->transaction().id() == m_matchTransaction.id())
          t->setMatchMark(true);

        if(t->id() == focusItemId)
          focusItem = t;

        t->setBalance(balance.formatMoney("", d->m_precision));
        const MyMoneySplit& split = t->split();
        balance -= split.shares() * factor;
        if(split.reconcileFlag() == MyMoneySplit::NotReconciled) {
          clearedBalance -= split.shares() * factor;
        }
        if(t->transaction().postDate() > QDate::currentDate()) {
          actBalance -= split.shares() * factor;
        }
      }
      p = p->prevItem();
    }

    // add a last empty entry for new transactions
    // leave some information about the current account
    MyMoneySplit split;
    split.setReconcileFlag(MyMoneySplit::Unknown);
    KMyMoneyRegister::Register::transactionFactory(m_register, m_objects, MyMoneyTransaction(), split, 0);

    m_register->updateRegister(true);

    if(focusItem) {
      m_register->selectItem(focusItem, true);
    } else {
      // make sure to skip the empty entry at the end if anything else exists
      // otherwise point to that emtpy line
      p = m_register->lastItem();
      m_register->setFocusItem(p);
      m_register->selectItem(p);
      focusItem = p;
    }

    updateSummaryLine(actBalance, clearedBalance);

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
  QTimer::singleShot(0, this, SLOT(slotUpdateViewPos()));

  // and tell everyone what's selected
  emit accountSelected(m_account);
  emit matchTransactionSelected(m_matchTransaction);
}

void KGlobalLedgerView::slotStartMatchTransaction(const MyMoneyTransaction& tr)
{
  KMyMoneyRegister::RegisterItem* p = m_register->firstItem();

  emit matchTransactionSelected(MyMoneyTransaction());

  while(p) {
    KMyMoneyRegister::Transaction* t = dynamic_cast<KMyMoneyRegister::Transaction*>(p);
    if(t) {
      bool matches = (t->transaction().id() == tr.id()) && (!tr.id().isEmpty());
      t->setMatchMark(matches);
      if(matches) {
        m_matchTransaction = t->transaction();
        emit matchTransactionSelected(m_matchTransaction);
      }
    }
    p = p->nextItem();
  }
}

void KGlobalLedgerView::slotCancelMatchTransaction(void)
{
  slotStartMatchTransaction(MyMoneyTransaction());
}

void KGlobalLedgerView::updateSummaryLine(const MyMoneyMoney& actBalance, const MyMoneyMoney& clearedBalance)
{
  if(isReconciliationAccount()) {
    m_leftSummaryLabel->setText(i18n("Statement: %1").arg(d->m_endingBalance.formatMoney("", d->m_precision)));
    m_centerSummaryLabel->setText(i18n("Cleared: %1").arg(clearedBalance.formatMoney("", d->m_precision)));
    m_rightSummaryLabel->setText(i18n("Difference: %1").arg((clearedBalance - d->m_endingBalance).formatMoney("", d->m_precision)));
  } else {
    // update summary line in normal mode
    QDate reconcileDate = m_account.lastReconciliationDate();
    // For some historic reason, the last reconciliation statement is
    // stored in the KVP area of the account under the key 'lastStatementDate'.
    // As long as we don't change that, we continue to read it from there
    // if the lastReconciliationDate() method returns an invalid date.
    if(!reconcileDate.isValid()) {
      if(!m_account.value("lastStatementDate").isEmpty())
        reconcileDate = QDate::fromString(m_account.value("lastStatementDate"), Qt::ISODate);
    }

    if(reconcileDate.isValid()) {
      m_leftSummaryLabel->setText(i18n("Last reconciled: %1").arg(KGlobal::locale()->formatDate(reconcileDate, true)));
    } else {
      m_leftSummaryLabel->setText(i18n("Never reconciled"));
    }

    m_centerSummaryLabel->setText(i18n("Cleared: %1").arg(clearedBalance.formatMoney("", d->m_precision)));
    m_rightSummaryLabel->setText(i18n("Balance: %1").arg(actBalance.formatMoney("", d->m_precision)));
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

void KGlobalLedgerView::addGroupMarkers(void)
{
  QMap<QString, int> list;
  QMap<QString, int>::const_iterator it;
  KMyMoneyRegister::RegisterItem* p = m_register->firstItem();
  KMyMoneyRegister::Transaction* t;
  QString name;
  QDate today;
  QDate yesterday, thisWeek, lastWeek;
  QDate thisMonth, lastMonth;
  QDate thisYear;
  int weekStartOfs;

  switch(m_register->primarySortKey()) {
    case KMyMoneyRegister::PostDateSort:
    case KMyMoneyRegister::EntryDateSort:
      today = QDate::currentDate();
      thisMonth.setYMD(today.year(), today.month(), 1);
      lastMonth = thisMonth.addMonths(-1);
      yesterday = today.addDays(-1);
      // a = QDate::dayOfWeek()      todays weekday (1 = Monday, 7 = Sunday)
      // b = KLocale::weekStartDay() first day of week (1 = Monday, 7 = Sunday)
      weekStartOfs = today.dayOfWeek() - KGlobal::locale()->weekStartDay();
      if(weekStartOfs < 0) {
        weekStartOfs = 7 + weekStartOfs;
      }
      thisWeek = today.addDays(-weekStartOfs);
      lastWeek = thisWeek.addDays(-7);
      thisYear.setYMD(today.year(), 1, 1);
      if(KMyMoneySettings::showFancyMarker()) {
        new KMyMoneyRegister::FancyDateGroupMarker(m_register, thisYear, i18n("This year"));
        new KMyMoneyRegister::FancyDateGroupMarker(m_register, lastMonth, i18n("Last month"));
        new KMyMoneyRegister::FancyDateGroupMarker(m_register, thisMonth, i18n("This month"));
        new KMyMoneyRegister::FancyDateGroupMarker(m_register, lastWeek, i18n("Last week"));
        new KMyMoneyRegister::FancyDateGroupMarker(m_register, thisWeek, i18n("This week"));
        new KMyMoneyRegister::FancyDateGroupMarker(m_register, yesterday, i18n("Yesterday"));
        new KMyMoneyRegister::FancyDateGroupMarker(m_register, today, i18n("Today"));
        new KMyMoneyRegister::FancyDateGroupMarker(m_register, today.addDays(1), i18n("Future transactions"));
      } else {
        new KMyMoneyRegister::SimpleDateGroupMarker(m_register, today.addDays(1), i18n("Future transactions"));
      }
      break;

    case KMyMoneyRegister::TypeSort:
      if(KMyMoneySettings::showFancyMarker()) {
        new KMyMoneyRegister::TypeGroupMarker(m_register, KMyMoneyRegister::Deposit, m_account.accountType());
        new KMyMoneyRegister::TypeGroupMarker(m_register, KMyMoneyRegister::Payment, m_account.accountType());
      }
      break;

    case KMyMoneyRegister::ReconcileStateSort:
      if(KMyMoneySettings::showFancyMarker()) {
        new KMyMoneyRegister::ReconcileGroupMarker(m_register, MyMoneySplit::NotReconciled);
        new KMyMoneyRegister::ReconcileGroupMarker(m_register, MyMoneySplit::Cleared);
        new KMyMoneyRegister::ReconcileGroupMarker(m_register, MyMoneySplit::Reconciled);
        new KMyMoneyRegister::ReconcileGroupMarker(m_register, MyMoneySplit::Frozen);
      }
      break;

    case KMyMoneyRegister::PayeeSort:
      if(KMyMoneySettings::showFancyMarker()) {
        while(p) {
          t = dynamic_cast<KMyMoneyRegister::Transaction*>(p);
          if(t) {
            list[t->sortPayee()] = 1;
          }
          p = p->nextItem();
        }
        for(it = list.begin(); it != list.end(); ++it) {
          name = it.key();
          if(name.isEmpty()) {
            name = i18n("Unknown payee", "Unknown");
          }
          new KMyMoneyRegister::PayeeGroupMarker(m_register, name);
        }
      }
      break;

    case KMyMoneyRegister::CategorySort:
      if(KMyMoneySettings::showFancyMarker()) {
        while(p) {
          t = dynamic_cast<KMyMoneyRegister::Transaction*>(p);
          if(t) {
            list[t->sortCategory()] = 1;
          }
          p = p->nextItem();
        }
        for(it = list.begin(); it != list.end(); ++it) {
          name = it.key();
          if(name.isEmpty()) {
            name = i18n("Unknown category", "Unknown");
          }
          new KMyMoneyRegister::CategoryGroupMarker(m_register, name);
        }
      }
      break;

    case KMyMoneyRegister::SecuritySort:
      if(KMyMoneySettings::showFancyMarker()) {
        while(p) {
          t = dynamic_cast<KMyMoneyRegister::InvestTransaction*>(p);
          if(t) {
            list[t->sortSecurity()] = 1;
          }
          p = p->nextItem();
        }
        for(it = list.begin(); it != list.end(); ++it) {
          name = it.key();
          if(name.isEmpty()) {
            name = i18n("Unknown security", "Unknown");
          }
          new KMyMoneyRegister::CategoryGroupMarker(m_register, name);
        }
      }
      break;

    default: // no markers supported
      break;
  }
}

void KGlobalLedgerView::removeGroupMarkers(void)
{
  qDebug("KGlobalLedgerView::removeGroupMarkers(void) not yet implemented");
}

void KGlobalLedgerView::resizeEvent(QResizeEvent* ev)
{
  m_register->resize(KMyMoneyRegister::DetailColumn);
  m_form->resize(KMyMoneyTransactionForm::ValueColumn1);
  KMyMoneyViewBase::resizeEvent(ev);
}

void KGlobalLedgerView::setupConnections(KLedgerView*  /*view*/)
{
#if 0
  if(view) {
    connect(view, SIGNAL(accountAndTransactionSelected(const QCString&, const QCString&)),
      this, SLOT(slotSelectAccount(const QCString&, const QCString&)));
    connect(view, SIGNAL(payeeSelected(const QCString&, const QCString&, const QCString&)),
      SIGNAL(payeeSelected(const QCString&, const QCString&, const QCString&)));
    connect(view, SIGNAL(newCategory(MyMoneyAccount&)), kmymoney2, SLOT(slotCategoryNew(MyMoneyAccount&)));
    connect(view, SIGNAL(reportGenerated(const MyMoneyReport&)),
      SIGNAL(reportGenerated(const MyMoneyReport&)));
  }
#endif
}

void KGlobalLedgerView::loadInvestmentView(void)
{
#if 0
  KLedgerView* view;
  // Investment account
  view = m_specificView[MyMoneyAccount::Investment] = new KLedgerViewInvestments(this, "InvestmentsView");
  m_accountStack->addWidget(view, MyMoneyAccount::Investment);
  setupConnections(view);
#endif
}

#if 0
void KGlobalLedgerView::slotReloadView(void)
{
  ::timetrace("Start KGlobalLedgerView::slotReloadView");
  // qDebug("KGlobalLedgerView::slotReloadView()");

  // make sure to determine the current account from scratch
  m_accountId = QCString();

  slotRefreshView();
  ::timetrace("Done KGlobalLedgerView::slotReloadView");
}

void KGlobalLedgerView::slotRefreshView(void)
{
  QCString id = m_accountId;

  // qDebug("KGlobalLedgerView::slotRefreshView()");

  // load the combobox from scratch and determine the current account
  loadAccounts();

  // if the current account differs from the previous selection
  // then select the correct ledgerview first and force loading
  // the newly selected account
  if(m_accountId != id) {
    id = m_accountId;
    m_accountId = QCString();
    slotSelectAccount(id);
  } else if(m_accountId.isEmpty()) {
    m_accountId = QCString();
    slotSelectAccount(m_accountId);
  } else if(m_currentView != 0) {
    m_currentView->refreshView();
  } else
    qFatal("Houston: we have a problem in KGlobalLedgerView::slotRefreshView()");

  // Enable rest of view only, if we have at least one account out of this group
  QValueList<MyMoneyAccount::accountTypeE> typeList;
  typeList << MyMoneyAccount::Checkings;
  typeList << MyMoneyAccount::Savings;
  typeList << MyMoneyAccount::Cash;
  typeList << MyMoneyAccount::CreditCard;
  typeList << MyMoneyAccount::Loan;
  typeList << MyMoneyAccount::Asset;
  typeList << MyMoneyAccount::Liability;
  typeList << MyMoneyAccount::Currency;
  typeList << MyMoneyAccount::AssetLoan;
  m_accountStack->setEnabled(m_accountComboBox->accountList(typeList).count() > 0);

  m_accountComboBox->setEnabled(m_accountComboBox->count() > 0);
}
#endif

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
    QCStringList list = m_accountComboBox->accountList();
    if(list.count()) {
      QCStringList::Iterator it;
      for(it = list.begin(); it != list.end(); ++it) {
        MyMoneyAccount a = file->account(*it);
        if(a.accountType() != MyMoneyAccount::Stock) {
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
      MyMoneySecurity sec = MyMoneyFile::instance()->security(m_account.currencyId());
      d->m_precision = MyMoneyMoney::denomToPrec(m_account.fraction(sec));
    } catch(MyMoneyException *e) {
      qDebug("Security %s for account %s not found", m_account.currencyId().data(), m_account.name().data());
      delete e;
      d->m_precision = 2;
    }
  }
}

void KGlobalLedgerView::selectTransaction(const QCString& id)
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

void KGlobalLedgerView::slotSetReconcileAccount(const MyMoneyAccount& acc, const MyMoneyMoney& endingBalance)
{
  if(d->m_reconciliationAccount != acc.id()) {
    // make sure the account is selected
    if(!acc.id().isEmpty())
      slotSelectAccount(acc.id());

    d->m_reconciliationAccount = acc.id();
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

bool KGlobalLedgerView::slotSelectAccount(const QCString& id, const QCString& transactionId)
{
  bool    rc = true;

  if(!id.isEmpty()) {
    if(m_account.id() != id) {
      try {
        m_account = MyMoneyFile::instance()->account(id);
        // if a stock account is selected, we show the
        // the corresponding parent (investment) account
        if(m_account.accountType() == MyMoneyAccount::Stock) {
          m_account = MyMoneyFile::instance()->account(m_account.parentAccountId());
        }
        m_newAccountLoaded = true;
        d->m_inLoading = false;    // force load no matter what stage a previous load is
        slotLoadView();
      } catch(MyMoneyException* e) {
        qDebug("Unable to retrieve account %s", id.data());
        delete e;
        rc = false;
      }
    } else {
      emit accountSelected(m_account);
    }
    selectTransaction(transactionId);
  }
  return rc;
}

void KGlobalLedgerView::suspendUpdate(const bool /*suspend*/)
{
#if 0
  for(int i = 0; i < MyMoneyAccount::MaxAccountTypes; ++i) {
    if(m_specificView[i] != 0)
      m_specificView[i]->suspendUpdate(suspend);
  }
#endif
}


void KGlobalLedgerView::slotCancelEdit(void)
{
#if 0
  // cancel any pending edit operation in the ledger views
  if(m_accountStack->isVisible()) {
    KLedgerView* view = dynamic_cast<KLedgerView*>(m_accountStack->visibleWidget());
    Q_CHECK_PTR(view);
    view->slotCancelEdit();
  }
#endif
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
  if(!m_inEditMode) {
    setupDefaultAction();
    emit newTransaction();
  }
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
    QValueList<KMyMoneyRegister::SelectedTransaction> list;
    m_register->selectedTransactions(list);
    if(list.count() > 0 && !list[0].transaction().id().isEmpty()) {
      m_register->clearSelection();
      m_register->selectItem(m_register->lastItem());
      m_register->updateRegister();
    }
    rc = true;
  }
  return rc;
}

TransactionEditor* KGlobalLedgerView::startEdit(const QValueList<KMyMoneyRegister::SelectedTransaction>& list)
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

  int warnLevel = 0;

  QValueList<KMyMoneyRegister::SelectedTransaction>::const_iterator it_t;
  for(it_t = list.begin(); warnLevel < 2 && it_t != list.end(); ++it_t) {
    const QValueList<MyMoneySplit>& splits = (*it_t).transaction().splits();
    QValueList<MyMoneySplit>::const_iterator it_s;
    for(it_s = splits.begin(); warnLevel < 2 && it_s != splits.end(); ++it_s) {
      if((*it_s).reconcileFlag() == MyMoneySplit::Frozen)
        warnLevel = 2;
      if((*it_s).reconcileFlag() == MyMoneySplit::Reconciled && warnLevel < 1)
        warnLevel = 1;

    }
  }

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
  }

  if(warnLevel == 2)
    return 0;


  TransactionEditor* editor = 0;
  KMyMoneyRegister::Transaction* item = dynamic_cast<KMyMoneyRegister::Transaction*>(m_register->focusItem());
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

  if(item) {
    // decide, if we edit in the register or in the form
    TransactionEditorContainer* parent;
    if(m_formFrame->isVisible())
      parent = m_form;
    else {
      parent = m_register;
    }

    // TODO create the right editor depending on the account type we look at
    editor = item->createEditor(parent, m_objects, list, m_lastPostDate);

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
        m_register->updateRegister(KMyMoneySettings::ledgerLens() | !KMyMoneySettings::transactionForm());
      }

      m_inEditMode = true;
      connect(editor, SIGNAL(transactionDataSufficient(bool)), kmymoney2->action("transaction_enter"), SLOT(setEnabled(bool)));
      connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), editor, SLOT(slotReloadEditWidgets()));
      connect(editor, SIGNAL(finishEdit(const QValueList<KMyMoneyRegister::SelectedTransaction >&)), this, SLOT(slotLeaveEditMode(const QValueList<KMyMoneyRegister::SelectedTransaction >&)));

      connect(editor, SIGNAL(objectCreation(bool)), d->m_mousePressFilter, SLOT(setFilterDeactive(bool)));
      connect(editor, SIGNAL(createPayee(const QString&, QCString&)), kmymoney2, SLOT(slotPayeeNew(const QString&, QCString&)));
      connect(editor, SIGNAL(createCategory(MyMoneyAccount&, const MyMoneyAccount&)), kmymoney2, SLOT(slotCategoryNew(MyMoneyAccount&, const MyMoneyAccount&)));
      connect(editor, SIGNAL(createSecurity(MyMoneyAccount&, const MyMoneyAccount&)), kmymoney2, SLOT(slotInvestmentNew(MyMoneyAccount&, const MyMoneyAccount&)));

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

      // make sure to have the default action preset for next round
      setupDefaultAction();
    }
  }
  return editor;
}

void KGlobalLedgerView::slotLeaveEditMode(const QValueList<KMyMoneyRegister::SelectedTransaction>& list)
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
      d->m_inLoading = true;
      loadView();
      m_needReload = false;
      m_newAccountLoaded = false;
    }

  } else {
    emit accountSelected(m_account);
    QValueList<KMyMoneyRegister::SelectedTransaction> list;
    m_register->selectedTransactions(list);
    emit transactionsSelected(list);
    emit matchTransactionSelected(m_matchTransaction);
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
      if(m_tabOrderWidgets.findRef(dynamic_cast<QWidget*>(o)) != -1) {
        if((k->state() & Qt::KeyButtonMask) == 0) {
          switch(k->key()) {
            case Qt::Key_Escape:
              kmymoney2->action("transaction_cancel")->activate();
              rc = true;
              break;

            case Qt::Key_Return:
            case Qt::Key_Enter:
              kmymoney2->action("transaction_enter")->activate();
              rc = true;
              break;
          }
        }
      } else if(o == m_register) {
        // we hide all key press events from the register
        // while editing a transaction
        rc = true;
      }
    } else {
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

  QCString key;
  QString sortOrder, def;
  if(isReconciliationAccount()) {
    key = "kmm-sort-reconcile";
    def = KMyMoneySettings::sortReconcileView();
  } else {
    key = "kmm-sort-std";
    def = KMyMoneySettings::sortNormalView();
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
      try {
        MyMoneyFile::instance()->modifyAccount(m_account);
      } catch(MyMoneyException* e) {
        qDebug("Unable to update sort order for account '%s': %s", m_account.name().latin1(), e->what().latin1());
        delete e;
      }
    }
  }
  delete dlg;
}

void KGlobalLedgerView::slotToggleMarkTransactionCleared(KMyMoneyRegister::Transaction* t)
{
  if(isReconciliationAccount()) {
    switch(t->split().reconcileFlag()) {
      case MyMoneySplit::NotReconciled:
        emit markTransactionCleared();
        break;
      case MyMoneySplit::Cleared:
        emit markTransactionNotReconciled();
        break;
      default:
        break;
    }
  }
}

bool KGlobalLedgerView::canCreateTransactions(QString& tooltip) const
{
  bool rc = true;
  if(m_account.accountGroup() == MyMoneyAccount::Income
  || m_account.accountGroup() == MyMoneyAccount::Expense) {
    tooltip = i18n("Cannot create transactions in the context of a category.");
    rc = false;
  }
  return rc;
}

bool KGlobalLedgerView::canModifyTransactions(const QValueList<KMyMoneyRegister::SelectedTransaction>& list, QString& tooltip) const
{
  if(m_register->focusItem() == 0)
    return false;

  if(!m_register->focusItem()->isSelected()) {
    tooltip = i18n("Cannot modify transaction with focus if it is not selected.");
    return false;
  }
  return list.count() > 0;
}

bool KGlobalLedgerView::canEditTransactions(const QValueList<KMyMoneyRegister::SelectedTransaction>& list, QString& tooltip) const
{
  // check if we can edit the list of transactions. We can edit, if
  //
  //   a) no mix of standard and investment transactions exist
  //   b) if a split transaction is selected, this is the only selection
  //   c) none of the splits is frozen
  //   d) the transaction having the current focus is selected

  // check for d)
  if(!canModifyTransactions(list, tooltip))
    return false;

  bool rc = true;
  int investmentTransactions = 0;
  int normalTransactions = 0;

  if(m_account.accountGroup() == MyMoneyAccount::Income
  || m_account.accountGroup() == MyMoneyAccount::Expense) {
    tooltip = i18n("Cannot edit transactions in the context of a category.");
    rc = false;
  }

  QValueList<KMyMoneyRegister::SelectedTransaction>::const_iterator it_t;
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

    // check for c)
    const QValueList<MyMoneySplit>& splits = (*it_t).transaction().splits();
    QValueList<MyMoneySplit>::const_iterator it_s;
    for(it_s = splits.begin(); rc && it_s != splits.end(); ++it_s) {
      if((*it_s).reconcileFlag() == MyMoneySplit::Frozen) {
        tooltip = i18n("Cannot edit transactions with frozen splits.");
        rc = false;
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
