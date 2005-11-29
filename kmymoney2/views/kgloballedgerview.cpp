/***************************************************************************
                          kgloballedgerview.cpp  -  description
                             -------------------
    begin                : Sat Jul 13 2002
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

#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qcstring.h>
#include <qtimer.h>
#include <qlistbox.h>

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
#include "kledgerviewcheckings.h"
#include "kledgerviewsavings.h"
#include "kledgerviewcreditcard.h"
#include "kledgerviewasset.h"
#include "kledgerviewcash.h"
#include "kledgerviewloan.h"
#include "kledgerviewliability.h"
#include "kledgerviewinvestments.h"

#include <kmymoney/mymoneyexception.h>
#include <kmymoney/mymoneyaccount.h>
#include <kmymoney/mymoneyfile.h>
#include "../widgets/kmymoneyaccountcombo.h"
#include "../widgets/kmymoneytitlelabel.h"
#include "../kmymoney2.h"

KGlobalLedgerView::KGlobalLedgerView(QWidget *parent, const char *name )
  : KMyMoneyViewBase(parent, name, i18n("Ledgers"))
{
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

  m_accountComboBox = new kMyMoneyAccountCombo(m_toolbar, "AccountCombo");
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
  connect(view, SIGNAL(accountAndTransactionSelected(const QCString&, const QCString&)),
    this, SLOT(slotSelectAccount(const QCString&, const QCString&)));
  connect(view, SIGNAL(payeeSelected(const QCString&, const QCString&, const QCString&)),
    SIGNAL(payeeSelected(const QCString&, const QCString&, const QCString&)));
  // connect(this, SIGNAL(cancelEdit()), view, SLOT(slotCancelEdit()));

  // Savings account
  view = m_specificView[MyMoneyAccount::Savings] = new KLedgerViewSavings(this, "SavingsView");
  m_accountStack->addWidget(view, MyMoneyAccount::Savings);
  connect(view, SIGNAL(accountAndTransactionSelected(const QCString&, const QCString&)),
    this, SLOT(slotSelectAccount(const QCString&, const QCString&)));
  connect(view, SIGNAL(payeeSelected(const QCString&, const QCString&, const QCString&)),
    SIGNAL(payeeSelected(const QCString&, const QCString&, const QCString&)));
  // connect(this, SIGNAL(cancelEdit()), view, SLOT(slotCancelEdit()));

  // Credit card account
  view = m_specificView[MyMoneyAccount::CreditCard] = new KLedgerViewCreditCard(this, "CreditCardView");
  m_accountStack->addWidget(view, MyMoneyAccount::CreditCard);
  connect(view, SIGNAL(accountAndTransactionSelected(const QCString&, const QCString&)),
    this, SLOT(slotSelectAccount(const QCString&, const QCString&)));
  connect(view, SIGNAL(payeeSelected(const QCString&, const QCString&, const QCString&)),
    SIGNAL(payeeSelected(const QCString&, const QCString&, const QCString&)));
  // connect(this, SIGNAL(cancelEdit()), view, SLOT(slotCancelEdit()));

  // Cash account
  view = m_specificView[MyMoneyAccount::Cash] = new KLedgerViewCash(this, "CashView");
  m_accountStack->addWidget(view, MyMoneyAccount::Cash);
  connect(view, SIGNAL(accountAndTransactionSelected(const QCString&, const QCString&)),
    this, SLOT(slotSelectAccount(const QCString&, const QCString&)));
  connect(view, SIGNAL(payeeSelected(const QCString&, const QCString&, const QCString&)),
    SIGNAL(payeeSelected(const QCString&, const QCString&, const QCString&)));
  // connect(this, SIGNAL(cancelEdit()), view, SLOT(slotCancelEdit()));

  // Asset account
  view = m_specificView[MyMoneyAccount::Asset] = new KLedgerViewAsset(this, "AssetView");
  m_accountStack->addWidget(view, MyMoneyAccount::Asset);
  connect(view, SIGNAL(accountAndTransactionSelected(const QCString&, const QCString&)),
    this, SLOT(slotSelectAccount(const QCString&, const QCString&)));
  connect(view, SIGNAL(payeeSelected(const QCString&, const QCString&, const QCString&)),
    SIGNAL(payeeSelected(const QCString&, const QCString&, const QCString&)));
  // connect(this, SIGNAL(cancelEdit()), view, SLOT(slotCancelEdit()));

  // Loan account
  view = m_specificView[MyMoneyAccount::Loan] = m_specificView[MyMoneyAccount::AssetLoan] = new KLedgerViewLoan(this, "LoanView");
  m_accountStack->addWidget(view, MyMoneyAccount::Loan);
  connect(view, SIGNAL(accountAndTransactionSelected(const QCString&, const QCString&)),
    this, SLOT(slotSelectAccount(const QCString&, const QCString&)));
  connect(view, SIGNAL(payeeSelected(const QCString&, const QCString&, const QCString&)),
    SIGNAL(payeeSelected(const QCString&, const QCString&, const QCString&)));
  // connect(this, SIGNAL(cancelEdit()), view, SLOT(slotCancelEdit()));

  // Liability account
  view = m_specificView[MyMoneyAccount::Liability] = new KLedgerViewLiability(this, "LiabilityView");
  m_accountStack->addWidget(view, MyMoneyAccount::Liability);
  connect(view, SIGNAL(accountAndTransactionSelected(const QCString&, const QCString&)),
    this, SLOT(slotSelectAccount(const QCString&, const QCString&)));
  connect(view, SIGNAL(payeeSelected(const QCString&, const QCString&, const QCString&)),
    SIGNAL(payeeSelected(const QCString&, const QCString&, const QCString&)));
  // connect(this, SIGNAL(cancelEdit()), view, SLOT(slotCancelEdit()));

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

  m_viewLayout->addWidget(m_accountStack);
  setMinimumHeight(m_accountComboBox->minimumHeight() + m_accountStack->sizeHint().height());

  m_accountId = QCString();

  MyMoneyFile::instance()->attach(MyMoneyFile::NotifyClassAccount, this);

  // setup connections
  connect(m_accountComboBox, SIGNAL(accountSelected(const QCString&)),
          this, SLOT(slotSelectAccount(const QCString&)));
}

KGlobalLedgerView::~KGlobalLedgerView()
{
  MyMoneyFile::instance()->detach(MyMoneyFile::NotifyClassAccount, this);
  delete m_viewLayout;
}

void KGlobalLedgerView::loadInvestmentView(void)
{
  KLedgerView* view;
  // Investment account
  view = m_specificView[MyMoneyAccount::Investment] = new KLedgerViewInvestments(this, "InvestmentsView");
  m_accountStack->addWidget(view, MyMoneyAccount::Investment);
  connect(view, SIGNAL(accountAndTransactionSelected(const QCString&, const QCString&)),
    this, SLOT(slotSelectAccount(const QCString&, const QCString&)));
  connect(view, SIGNAL(payeeSelected(const QCString&, const QCString&, const QCString&)),
    SIGNAL(payeeSelected(const QCString&, const QCString&, const QCString&)));
  // connect(this, SIGNAL(cancelEdit()), view, SLOT(slotCancelEdit()));
}

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

void KGlobalLedgerView::loadAccounts(void)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  MyMoneyAccount acc;

  // check if the current account still exists and make it the
  // current account
  if(!m_accountId.isEmpty()) {
    try {
      acc = file->account(m_accountId);
      m_accountId = acc.id();
    } catch(MyMoneyException *e) {
      delete e;
      m_accountId = QCString();
      acc = MyMoneyAccount();
    }
  }

  m_accountComboBox->loadList((KMyMoneyUtils::categoryTypeE)(KMyMoneyUtils::asset | KMyMoneyUtils::liability));

  if(acc.id().isEmpty()) {
    QCStringList list = m_accountComboBox->accountList();
    if(list.count()) {
      QCStringList::Iterator it;
      for(it = list.begin(); it != list.end(); ++it) {
        MyMoneyAccount a = file->account(*it);
        if(a.accountType() != MyMoneyAccount::Investment) {
          if(a.value("PreferredAccount") == "Yes") {
            acc = a;
            break;
          } else if(acc.id().isEmpty()) {
            acc = a;
          }
        }
      }
    }
  }

  slotSelectAccount(acc.id());
}

void KGlobalLedgerView::reconcileAccount(void)
{
  if(m_currentView)
    m_currentView->slotReconciliation();
}

const bool KGlobalLedgerView::slotSelectAccount(const QCString& id, const QCString& transactionId, const bool reconciliation)
{
  bool    rc = false;

  if(!id.isEmpty()) {
    // if the account id differs, then we have to do something
    MyMoneyAccount acc = MyMoneyFile::instance()->account(id);
    if(isVisible())
      kmymoney2->selectAccount(acc);
    if(m_accountId != id) {
      // cancel any pending edit operation in the ledger views
      // when switching to a different account
      slotCancelEdit();

      if(m_specificView[acc.accountType()] != 0) {
        // loan and asset-loan share a view. So there's
        // only one widget on the widget stack. Make sure
        // we pick the right one
        int viewType = acc.accountType();
        if(viewType == MyMoneyAccount::AssetLoan)
          viewType = MyMoneyAccount::Loan;
        m_accountStack->raiseWidget(viewType);
        m_currentView = m_specificView[acc.accountType()];
        m_currentView->slotSelectAccount(id);
        if(!transactionId.isEmpty())
          m_currentView->selectTransaction(transactionId);
        m_accountComboBox->setSelected(acc);

        // keep this as the current account
        m_accountId = id;

        rc = true;
      } else {
        // keep the current selection ...
        if(!m_accountId.isEmpty()) {
          acc = MyMoneyFile::instance()->account(m_accountId);
          m_accountComboBox->setSelected(acc);
        } else
          m_accountComboBox->setSelected(QCString());
        // ... and let's see, if someone else can handle this request
        emit accountSelected(id, transactionId);
      }
    } else {
#if KDE_VERSION < 310
      // in KDE 3.1 and above, QWidgetStack::show() takes care of this
      m_accountStack->raiseWidget(acc.accountType());
#endif
      rc = true;

      // keep this as the current account
      m_accountId = id;

      if(reconciliation == true && m_currentView)
        m_currentView->slotReconciliation();
      if(!transactionId.isEmpty()) {
        // cancel any pending edit operation in the ledger views
        // when switching to a specific transaction
        slotCancelEdit();
        m_currentView->selectTransaction(transactionId);
      }
    }

  } else {
    if(m_specificView[MyMoneyAccount::Checkings] != 0) {
      // cancel any pending edit operation in the ledger views
      // when switching to a non existing account
      slotCancelEdit();

      m_accountStack->raiseWidget(MyMoneyAccount::Checkings);
      m_currentView = m_specificView[MyMoneyAccount::Checkings];
      m_currentView->slotSelectAccount(id);
      m_accountComboBox->setSelected(QCString());

      // keep this as the current account
      m_accountId = QCString();

      if(reconciliation == true && m_currentView)
        m_currentView->slotReconciliation();

    } else {
      qFatal("Houston: we have a serious problem in KGlobalLedgerView");
    }
    // no account selected
    if(isVisible())
      kmymoney2->selectAccount();
  }
  
  // Now that the ledger view has changed, we have to update the "account" button
  // to point to the correct ledger's account menu.
  KLedgerView* ledgerview = dynamic_cast<KLedgerView*>(m_accountStack->visibleWidget());
  if ( ledgerview )
    m_toolbar->getButton(1)->setPopup(ledgerview->accountMenu());
    
  return rc;
}

void KGlobalLedgerView::suspendUpdate(const bool suspend)
{
  for(int i = 0; i < MyMoneyAccount::MaxAccountTypes; ++i) {
    if(m_specificView[i] != 0)
      m_specificView[i]->suspendUpdate(suspend);
  }
}


void KGlobalLedgerView::slotCancelEdit(void)
{
  // cancel any pending edit operation in the ledger views
  if(m_accountStack->isVisible()) {
    KLedgerView* view = dynamic_cast<KLedgerView*>(m_accountStack->visibleWidget());
    Q_CHECK_PTR(view);
    view->slotCancelEdit();
  }
}

void KGlobalLedgerView::show()
{
  // only show selection box if filled with at least one account
  m_accountComboBox->setEnabled(m_accountComboBox->count() > 0);

  // if we have a selected account, notify the application about it
  if(!m_accountId.isEmpty()) {
    try {
      MyMoneyAccount acc = MyMoneyFile::instance()->account(m_accountId);
      kmymoney2->selectAccount(acc);
    } catch(MyMoneyException* e) {
      delete e;
    }
  }

  KMyMoneyViewBase::show();
  emit signalViewActivated();
}

void KGlobalLedgerView::update(const QCString& /* id */)
{
  if(m_accountStack->isEnabled()) {
    QCString lastUsed = m_accountId;
    loadAccounts();
    if(m_accountId != lastUsed) {
      m_accountId = lastUsed;
      slotRefreshView();
    }
  } else {
    slotRefreshView();
  }
}

#include "kgloballedgerview.moc"
