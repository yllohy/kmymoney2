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

#include "../mymoney/mymoneyaccount.h"
#include "../mymoney/mymoneyfile.h"
#include "../widgets/kmymoneyaccountcombo.h"
#include "../kapptest.h"

KGlobalLedgerView::KGlobalLedgerView(QWidget *parent, const char *name )
  : QWidget(parent,name)
{
  m_currentView = 0;
  KLedgerView* view;

  for(int i = 0; i < MyMoneyAccount::MaxAccountTypes; ++i)
    m_specificView[i] = 0;

  if ( !name )
    setName( "Account register" );

  setCaption( i18n( "Account register" ) );
  m_formLayout = new QVBoxLayout( this, 0, 6, "Form1Layout");

  QHBoxLayout* Layout2 = new QHBoxLayout( 0, 0, 6, "Layout2");

  m_accountComboBox = new kMyMoneyAccountCombo(this, KAppTest::widgetName(this, "kMyMoneyAccountCombo"));
  // m_accountComboBox->setMinimumSize( QSize( 240, 0 ) );
  Layout2->addWidget( m_accountComboBox );
  QSpacerItem* spacer = new QSpacerItem( 20, 20,
                                         QSizePolicy::Expanding,
                                         QSizePolicy::Minimum );
  Layout2->addItem( spacer );
  m_formLayout->addLayout( Layout2 );

  m_accountStack = new QWidgetStack(this, KAppTest::widgetName(this, "QWidgetStack"));

  // Checkings account
  view = m_specificView[MyMoneyAccount::Checkings] = new KLedgerViewCheckings(this, KAppTest::widgetName(this, "KLedgerViewCheckings"));
  m_accountStack->addWidget(view, MyMoneyAccount::Checkings);
  connect(view, SIGNAL(accountAndTransactionSelected(const QCString&, const QCString&)),
    this, SLOT(slotSelectAccountAndTransaction(const QCString&, const QCString&)));
  connect(view, SIGNAL(payeeSelected(const QCString&, const QCString&, const QCString&)),
    SIGNAL(payeeSelected(const QCString&, const QCString&, const QCString&)));
  connect(this, SIGNAL(cancelEdit()), view, SLOT(slotCancelEdit()));

  // Savings account
  view = m_specificView[MyMoneyAccount::Savings] = new KLedgerViewSavings(this, KAppTest::widgetName(this, "KLedgerViewSavings"));
  m_accountStack->addWidget(view, MyMoneyAccount::Savings);
  connect(view, SIGNAL(accountAndTransactionSelected(const QCString&, const QCString&)),
    this, SLOT(slotSelectAccountAndTransaction(const QCString&, const QCString&)));
  connect(view, SIGNAL(payeeSelected(const QCString&, const QCString&, const QCString&)),
    SIGNAL(payeeSelected(const QCString&, const QCString&, const QCString&)));
  connect(this, SIGNAL(cancelEdit()), view, SLOT(slotCancelEdit()));

  // Credit card account
  view = m_specificView[MyMoneyAccount::CreditCard] = new KLedgerViewCreditCard(this, KAppTest::widgetName(this, "KLedgerViewCreditCard"));
  m_accountStack->addWidget(view, MyMoneyAccount::CreditCard);
  connect(view, SIGNAL(accountAndTransactionSelected(const QCString&, const QCString&)),
    this, SLOT(slotSelectAccountAndTransaction(const QCString&, const QCString&)));
  connect(view, SIGNAL(payeeSelected(const QCString&, const QCString&, const QCString&)),
    SIGNAL(payeeSelected(const QCString&, const QCString&, const QCString&)));
  connect(this, SIGNAL(cancelEdit()), view, SLOT(slotCancelEdit()));

  // Cash account
  view = m_specificView[MyMoneyAccount::Cash] = new KLedgerViewCash(this, KAppTest::widgetName(this, "KLedgerViewCash"));
  m_accountStack->addWidget(view, MyMoneyAccount::Cash);
  connect(view, SIGNAL(accountAndTransactionSelected(const QCString&, const QCString&)),
    this, SLOT(slotSelectAccountAndTransaction(const QCString&, const QCString&)));
  connect(view, SIGNAL(payeeSelected(const QCString&, const QCString&, const QCString&)),
    SIGNAL(payeeSelected(const QCString&, const QCString&, const QCString&)));
  connect(this, SIGNAL(cancelEdit()), view, SLOT(slotCancelEdit()));

  // Asset account
  view = m_specificView[MyMoneyAccount::Asset] = new KLedgerViewAsset(this, KAppTest::widgetName(this, "KLedgerViewAsset"));
  m_accountStack->addWidget(view, MyMoneyAccount::Asset);
  connect(view, SIGNAL(accountAndTransactionSelected(const QCString&, const QCString&)),
    this, SLOT(slotSelectAccountAndTransaction(const QCString&, const QCString&)));
  connect(view, SIGNAL(payeeSelected(const QCString&, const QCString&, const QCString&)),
    SIGNAL(payeeSelected(const QCString&, const QCString&, const QCString&)));
  connect(this, SIGNAL(cancelEdit()), view, SLOT(slotCancelEdit()));

  // Loan account
  view = m_specificView[MyMoneyAccount::Loan] = m_specificView[MyMoneyAccount::AssetLoan] = new KLedgerViewLoan(this, KAppTest::widgetName(this, "KLedgerViewLoan"));
  m_accountStack->addWidget(view, MyMoneyAccount::Loan);
  connect(view, SIGNAL(accountAndTransactionSelected(const QCString&, const QCString&)),
    this, SLOT(slotSelectAccountAndTransaction(const QCString&, const QCString&)));
  connect(view, SIGNAL(payeeSelected(const QCString&, const QCString&, const QCString&)),
    SIGNAL(payeeSelected(const QCString&, const QCString&, const QCString&)));
  connect(this, SIGNAL(cancelEdit()), view, SLOT(slotCancelEdit()));

  // Liability account
  view = m_specificView[MyMoneyAccount::Liability] = new KLedgerViewLiability(this, KAppTest::widgetName(this, "KLedgerViewLiability"));
  m_accountStack->addWidget(view, MyMoneyAccount::Liability);
  connect(view, SIGNAL(accountAndTransactionSelected(const QCString&, const QCString&)),
    this, SLOT(slotSelectAccountAndTransaction(const QCString&, const QCString&)));
  connect(view, SIGNAL(payeeSelected(const QCString&, const QCString&, const QCString&)),
    SIGNAL(payeeSelected(const QCString&, const QCString&, const QCString&)));
  connect(this, SIGNAL(cancelEdit()), view, SLOT(slotCancelEdit()));

  m_formLayout->addWidget(m_accountStack);
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
  delete m_formLayout;
}

void KGlobalLedgerView::slotReloadView(void)
{
  // qDebug("KGlobalLedgerView::slotReloadView()");

  // make sure to determine the current account from scratch
  m_accountId = QCString();

  slotRefreshView();
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

const bool KGlobalLedgerView::slotSelectAccount(const QCString& id)
{
  return slotSelectAccount(id, false);
}

const bool KGlobalLedgerView::slotSelectAccount(const QCString& id, const bool reconciliation)
{
  bool    rc = false;

  // cancel any pending edit operation in the ledger views
  emit cancelEdit();

  if(!id.isEmpty()) {
    // if the account id differs, then we have to do something
    MyMoneyAccount acc = MyMoneyFile::instance()->account(id);
    if(m_accountId != id) {
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
        m_accountComboBox->setSelected(acc);

        // keep this as the current account
        m_accountId = id;

        rc = true;
      } else {
        // keep the current selection ...
        acc = MyMoneyFile::instance()->account(m_accountId);
        m_accountComboBox->setSelected(acc);
        // ... and let's see, if someone else can handle this request
        emit accountSelected(id);
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
    }

  } else {
    if(m_specificView[MyMoneyAccount::Checkings] != 0) {
      m_accountStack->raiseWidget(MyMoneyAccount::Checkings);
      m_currentView = m_specificView[MyMoneyAccount::Checkings];
      m_currentView->slotSelectAccount(id);
      m_accountComboBox->setText("");

      // keep this as the current account
      m_accountId = QCString();

      if(reconciliation == true && m_currentView)
        m_currentView->slotReconciliation();

    } else {
      qFatal("Houston: we have a serious problem in KGlobalLedgerView");
    }
  }
  return rc;
}


void KGlobalLedgerView::slotSelectAccountAndTransaction(const QCString& accountId,
                                                const QCString& transactionId)
{
  // if the selection of the account succeeded then select the desired transaction
  if(slotSelectAccount(accountId)) {
    m_currentView->selectTransaction(transactionId);
  }
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
  emit cancelEdit();
}

void KGlobalLedgerView::show()
{
  // only show selection box if filled with at least one account
  m_accountComboBox->setEnabled(m_accountComboBox->count() > 0);

  QWidget::show();
  emit signalViewActivated();
}

void KGlobalLedgerView::hide()
{
  for(int i = 0; i < MyMoneyAccount::MaxAccountTypes; ++i) {
    if(m_specificView[i] != 0)
      m_specificView[i]->hide();
  }
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
