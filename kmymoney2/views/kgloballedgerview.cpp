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
#include "../widgets/kmymoneycombo.h"
#include "../kmymoneyutils.h"

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

  m_accountComboBox = new kMyMoneyCombo(false, this, "accountComboBox" );
  m_accountComboBox->setMinimumSize( QSize( 240, 0 ) );
  Layout2->addWidget( m_accountComboBox );
  QSpacerItem* spacer = new QSpacerItem( 20, 20,
                                         QSizePolicy::Expanding,
                                         QSizePolicy::Minimum );
  Layout2->addItem( spacer );
  m_formLayout->addLayout( Layout2 );

  m_accountStack = new QWidgetStack(this, "Stack");

  // Checkings account
  view = m_specificView[MyMoneyAccount::Checkings] = new KLedgerViewCheckings(this);
  m_accountStack->addWidget(view, MyMoneyAccount::Checkings);
  connect(view, SIGNAL(accountAndTransactionSelected(const QCString&, const QCString&)),
    this, SLOT(slotSelectAccountAndTransaction(const QCString&, const QCString&)));
  connect(view, SIGNAL(payeeSelected(const QCString&, const QCString&, const QCString&)),
    SIGNAL(payeeSelected(const QCString&, const QCString&, const QCString&)));
  connect(this, SIGNAL(cancelEdit()), view, SLOT(slotCancelEdit()));

  // Savings account
  view = m_specificView[MyMoneyAccount::Savings] = new KLedgerViewSavings(this);
  m_accountStack->addWidget(view, MyMoneyAccount::Savings);
  connect(view, SIGNAL(accountAndTransactionSelected(const QCString&, const QCString&)),
    this, SLOT(slotSelectAccountAndTransaction(const QCString&, const QCString&)));
  connect(view, SIGNAL(payeeSelected(const QCString&, const QCString&, const QCString&)),
    SIGNAL(payeeSelected(const QCString&, const QCString&, const QCString&)));
  connect(this, SIGNAL(cancelEdit()), view, SLOT(slotCancelEdit()));

  // Credit card account
  view = m_specificView[MyMoneyAccount::CreditCard] = new KLedgerViewCreditCard(this);
  m_accountStack->addWidget(view, MyMoneyAccount::CreditCard);
  connect(view, SIGNAL(accountAndTransactionSelected(const QCString&, const QCString&)),
    this, SLOT(slotSelectAccountAndTransaction(const QCString&, const QCString&)));
  connect(view, SIGNAL(payeeSelected(const QCString&, const QCString&, const QCString&)),
    SIGNAL(payeeSelected(const QCString&, const QCString&, const QCString&)));
  connect(this, SIGNAL(cancelEdit()), view, SLOT(slotCancelEdit()));

  // Cash account
  view = m_specificView[MyMoneyAccount::Cash] = new KLedgerViewCash(this);
  m_accountStack->addWidget(view, MyMoneyAccount::Cash);
  connect(view, SIGNAL(accountAndTransactionSelected(const QCString&, const QCString&)),
    this, SLOT(slotSelectAccountAndTransaction(const QCString&, const QCString&)));
  connect(view, SIGNAL(payeeSelected(const QCString&, const QCString&, const QCString&)),
    SIGNAL(payeeSelected(const QCString&, const QCString&, const QCString&)));
  connect(this, SIGNAL(cancelEdit()), view, SLOT(slotCancelEdit()));

  // Asset account
  view = m_specificView[MyMoneyAccount::Asset] = new KLedgerViewAsset(this);
  m_accountStack->addWidget(view, MyMoneyAccount::Asset);
  connect(view, SIGNAL(accountAndTransactionSelected(const QCString&, const QCString&)),
    this, SLOT(slotSelectAccountAndTransaction(const QCString&, const QCString&)));
  connect(view, SIGNAL(payeeSelected(const QCString&, const QCString&, const QCString&)),
    SIGNAL(payeeSelected(const QCString&, const QCString&, const QCString&)));
  connect(this, SIGNAL(cancelEdit()), view, SLOT(slotCancelEdit()));

  // Loan account
  view = m_specificView[MyMoneyAccount::Loan] = new KLedgerViewLoan(this);
  m_accountStack->addWidget(view, MyMoneyAccount::Loan);
  connect(view, SIGNAL(accountAndTransactionSelected(const QCString&, const QCString&)),
    this, SLOT(slotSelectAccountAndTransaction(const QCString&, const QCString&)));
  connect(view, SIGNAL(payeeSelected(const QCString&, const QCString&, const QCString&)),
    SIGNAL(payeeSelected(const QCString&, const QCString&, const QCString&)));
  connect(this, SIGNAL(cancelEdit()), view, SLOT(slotCancelEdit()));

  // AssetLoan account
  view = m_specificView[MyMoneyAccount::AssetLoan] = new KLedgerViewLoan(this);
  m_accountStack->addWidget(view, MyMoneyAccount::AssetLoan);
  connect(view, SIGNAL(accountAndTransactionSelected(const QCString&, const QCString&)),
    this, SLOT(slotSelectAccountAndTransaction(const QCString&, const QCString&)));
  connect(view, SIGNAL(payeeSelected(const QCString&, const QCString&, const QCString&)),
    SIGNAL(payeeSelected(const QCString&, const QCString&, const QCString&)));
  connect(this, SIGNAL(cancelEdit()), view, SLOT(slotCancelEdit()));

  // Liability account
  view = m_specificView[MyMoneyAccount::Liability] = new KLedgerViewLiability(this);
  m_accountStack->addWidget(view, MyMoneyAccount::Liability);
  connect(view, SIGNAL(accountAndTransactionSelected(const QCString&, const QCString&)),
    this, SLOT(slotSelectAccountAndTransaction(const QCString&, const QCString&)));
  connect(view, SIGNAL(payeeSelected(const QCString&, const QCString&, const QCString&)),
    SIGNAL(payeeSelected(const QCString&, const QCString&, const QCString&)));
  connect(this, SIGNAL(cancelEdit()), view, SLOT(slotCancelEdit()));

  //Investment View
  // view = m_specificView[MyMoneyAccount::Investment] = new KLedgerViewInvestments(this);
  // m_accountStack->addWidget(view, MyMoneyAccount::Investment);
  //connect(view, SIGNAL(accountAndTransactionSelected(const QCString&, const QCString&)),
  //  this, SLOT(slotSelectAccountAndTransaction(const QCString&, const QCString&)));
  //connect(view, SIGNAL(payeeSelected(const QCString&, const QCString&, const QCString&)),
  //  SIGNAL(payeeSelected(const QCString&, const QCString&, const QCString&)));
  //connect(this, SIGNAL(cancelEdit()), view, SLOT(slotCancelEdit()));


  m_formLayout->addWidget(m_accountStack);
  setMinimumHeight(m_accountComboBox->minimumHeight() + m_accountStack->sizeHint().height());

  m_accountId = QCString();

  MyMoneyFile::instance()->attach(MyMoneyFile::NotifyClassAccount, this);

  // setup connections
  connect(m_accountComboBox, SIGNAL(activated(const QString&)),
          this, SLOT(slotSelectAccount(const QString&)));
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

  m_accountComboBox->setEnabled(m_accountComboBox->count() > 0);
}

void KGlobalLedgerView::loadAccounts(void)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  QString currentName;

  // qDebug("KGlobalLedgerView::loadAccounts()");
  m_accountComboBox->clear();

  MyMoneyAccount acc, subAcc;

  // check if the current account still exists and make it the
  // current account
  if(!m_accountId.isEmpty()) {
    try {
      acc = file->account(m_accountId);
      currentName = acc.name();
    } catch(MyMoneyException *e) {
      delete e;
      m_accountId = QCString();
    }
  }

  // load all asset and liability accounts into the combobox
  QCStringList::ConstIterator it_s;
  acc = file->asset();
  for(it_s = acc.accountList().begin(); it_s != acc.accountList().end(); ++it_s) {
    subAcc = file->account(*it_s);
    if(subAcc.accountType() != MyMoneyAccount::Investment) {
      m_accountComboBox->insertItem(subAcc.name());
      if(m_accountId.isEmpty()) {
        m_accountId = *it_s;
        currentName = subAcc.name();
      }
    }
  }

  acc = file->liability();
  for(it_s = acc.accountList().begin(); it_s != acc.accountList().end(); ++it_s) {
    subAcc = file->account(*it_s);
    m_accountComboBox->insertItem(subAcc.name());
    if(m_accountId.isEmpty()) {
      m_accountId = *it_s;
      currentName = subAcc.name();
    }
  }

  // sort list by name of accounts
  m_accountComboBox->listBox()->sort();
  if(!currentName.isEmpty())
    m_accountComboBox->setCurrentItem(currentName);
}

const bool KGlobalLedgerView::slotSelectAccount(const QString& accountName)
{
  // qDebug("KGlobalLedgerView::slotSelectAccount(const QString& accountName)");

  QCString id = MyMoneyFile::instance()->nameToAccount(accountName);
  bool     rc = false;
  if(!id.isEmpty()) {
    rc = slotSelectAccount(id);
  }
  return rc;
}

const bool KGlobalLedgerView::slotSelectAccount(const QCString& id, const bool reconciliation)
{
  bool    rc = false;

  // qDebug("KGlobalLedgerView::slotSelectAccount(const QCString& id, const bool reconciliation)");

  // cancel any pending edit operation in the ledger views
  emit cancelEdit();

  if(!id.isEmpty()) {
    // if the account id differs, then we have to do something
    MyMoneyAccount acc = MyMoneyFile::instance()->account(id);
    if(m_accountId != id) {
      if(m_specificView[acc.accountType()] != 0) {
        m_accountStack->raiseWidget(acc.accountType());
        m_currentView = m_specificView[acc.accountType()];
        m_currentView->slotSelectAccount(id);
        m_accountComboBox->setCurrentItem(acc.name());
        rc = true;

      } else {
        QString msg = "Specific ledger view for account type '" +
          KMyMoneyUtils::accountTypeToString(acc.accountType()) + "' not yet implemented";
        KMessageBox::sorry(0, msg, "Implementation problem");
      }
    } else {
#if KDE_VERSION < 310
      // in KDE 3.1 and above, QWidgetStack::show() takes care of this
      m_accountStack->raiseWidget(acc.accountType());
#endif
      rc = true;
    }

  } else {
    if(m_specificView[MyMoneyAccount::Checkings] != 0) {
      m_accountStack->raiseWidget(MyMoneyAccount::Checkings);
      m_currentView = m_specificView[MyMoneyAccount::Checkings];
      m_currentView->slotSelectAccount(id);

    } else {
      qFatal("Houston: we have a serious problem in KGlobalLedgerView");
    }
  }

  // keep this as the current account
  m_accountId = id;

  if(reconciliation == true && m_currentView)
    m_currentView->slotReconciliation();

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
  QCString lastUsed = m_accountId;
  loadAccounts();
  if(m_accountId != lastUsed) {
    m_accountId = lastUsed;
    slotRefreshView();
  }
}
