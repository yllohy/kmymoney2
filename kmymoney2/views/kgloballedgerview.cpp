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

// ----------------------------------------------------------------------------
// KDE Includes

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

  // Savings account
  view = m_specificView[MyMoneyAccount::Savings] = new KLedgerViewSavings(this);
  m_accountStack->addWidget(view, MyMoneyAccount::Savings);
  connect(view, SIGNAL(accountAndTransactionSelected(const QCString&, const QCString&)),
    this, SLOT(slotSelectAccountAndTransaction(const QCString&, const QCString&)));
  connect(view, SIGNAL(payeeSelected(const QCString&, const QCString&, const QCString&)),
    SIGNAL(payeeSelected(const QCString&, const QCString&, const QCString&)));

  // Credit card account
  view = m_specificView[MyMoneyAccount::CreditCard] = new KLedgerViewCreditCard(this);
  m_accountStack->addWidget(view, MyMoneyAccount::CreditCard);
  connect(view, SIGNAL(accountAndTransactionSelected(const QCString&, const QCString&)),
    this, SLOT(slotSelectAccountAndTransaction(const QCString&, const QCString&)));
  connect(view, SIGNAL(payeeSelected(const QCString&, const QCString&, const QCString&)),
    SIGNAL(payeeSelected(const QCString&, const QCString&, const QCString&)));

  // Cash account
  view = m_specificView[MyMoneyAccount::Cash] = new KLedgerViewCash(this);
  m_accountStack->addWidget(view, MyMoneyAccount::Cash);
  connect(view, SIGNAL(accountAndTransactionSelected(const QCString&, const QCString&)),
    this, SLOT(slotSelectAccountAndTransaction(const QCString&, const QCString&)));
  connect(view, SIGNAL(payeeSelected(const QCString&, const QCString&, const QCString&)),
    SIGNAL(payeeSelected(const QCString&, const QCString&, const QCString&)));

  // Asset account
  view = m_specificView[MyMoneyAccount::Asset] = new KLedgerViewAsset(this);
  m_accountStack->addWidget(view, MyMoneyAccount::Asset);
  connect(view, SIGNAL(accountAndTransactionSelected(const QCString&, const QCString&)),
    this, SLOT(slotSelectAccountAndTransaction(const QCString&, const QCString&)));
  connect(view, SIGNAL(payeeSelected(const QCString&, const QCString&, const QCString&)),
    SIGNAL(payeeSelected(const QCString&, const QCString&, const QCString&)));


  m_formLayout->addWidget(m_accountStack);

  // read the configuration
  m_accountId = "";

  // setup connections
  connect(m_accountComboBox, SIGNAL(activated(const QString&)),
          this, SLOT(slotAccountSelected(const QString&)));
}

void KGlobalLedgerView::loadAccounts(void)
{
  MyMoneyFile* file = MyMoneyFile::instance();

  m_accountComboBox->clear();

  MyMoneyAccount acc;
  QCStringList::ConstIterator it_s;

  acc = file->asset();
  for(it_s = acc.accountList().begin(); it_s != acc.accountList().end(); ++it_s) {
    m_accountComboBox->insertItem(file->account(*it_s).name());
    if(m_accountId == "") {
      m_accountId = *it_s;
    }
  }

  acc = file->liability();
  for(it_s = acc.accountList().begin(); it_s != acc.accountList().end(); ++it_s) {
    m_accountComboBox->insertItem(file->account(*it_s).name());
    if(m_accountId == "") {
      m_accountId = *it_s;
    }
  }
}

KGlobalLedgerView::~KGlobalLedgerView()
{
  delete m_formLayout;
}

void KGlobalLedgerView::show()
{
  QWidget::show();
  
  if(m_accountId == "")
    QTimer::singleShot(0, this, SLOT(reloadView()));
    
  // only show selection box if filled with at least one account
  m_accountComboBox->setEnabled(m_accountComboBox->count() > 0);
  
  emit signalViewActivated();
}

void KGlobalLedgerView::hide()
{
  for(int i = 0; i < MyMoneyAccount::MaxAccountTypes; ++i) {
    if(m_specificView[i] != 0)
      m_specificView[i]->hide();
  }
}

void KGlobalLedgerView::reloadView(void)
{
  loadAccounts();
  selectAccount(m_accountId, "", false, true);
  
  // only show selection box if filled with at least one account
  m_accountComboBox->setEnabled(m_accountComboBox->count() > 0);
}

void KGlobalLedgerView::refreshView(void)
{
  if(m_accountId == "")
    QTimer::singleShot(0, this, SLOT(reloadView()));

  for(int i = 0; i < MyMoneyAccount::MaxAccountTypes; ++i) {
    if(m_specificView[i] != 0)
      m_specificView[i]->refreshView();
  }
}

void KGlobalLedgerView::suspendUpdate(const bool suspend)
{
  for(int i = 0; i < MyMoneyAccount::MaxAccountTypes; ++i) {
    if(m_specificView[i] != 0)
      m_specificView[i]->suspendUpdate(suspend);
  }
}

void KGlobalLedgerView::slotSelectAccountAndTransaction(const QCString& accountId, const QCString& transactionId)
{
  selectAccount(accountId, transactionId);
}

void KGlobalLedgerView::selectAccount(const QCString& accountId, const QCString& transaction, const bool reconciliation, const bool forceLoad)
{
  slotCancelEdit();
  if(accountId != "") {
    MyMoneyAccount acc = MyMoneyFile::instance()->account(accountId);
    if(m_specificView[acc.accountType()] != 0) {
      m_accountStack->raiseWidget(acc.accountType());
      m_currentView = m_specificView[acc.accountType()];
      m_currentView->setCurrentAccount(accountId, forceLoad);
      m_accountId = accountId;
      m_accountComboBox->setCurrentItem(acc.name());
      m_currentView->selectTransaction(transaction);
    } else {
      QString msg = "Specific ledger view for account type '" +
        KMyMoneyUtils::accountTypeToString(acc.accountType()) + "' not yet implemented";
      KMessageBox::sorry(0, msg, "Implementation problem");
    }
  } else {
    m_accountId = "";
    m_accountStack->raiseWidget(MyMoneyAccount::Checkings);
    if(m_specificView[MyMoneyAccount::Checkings] != 0) {
      m_currentView = m_specificView[MyMoneyAccount::Checkings];
      m_currentView->setCurrentAccount(accountId, forceLoad);
    }
  }
  if(reconciliation == true && m_currentView)
    m_currentView->slotReconciliation();
}

void KGlobalLedgerView::slotAccountSelected(const QString& account)
{
  slotAccountSelected(account, false);
}

void KGlobalLedgerView::slotAccountSelected(const QString& account, const bool reconciliation)
{
  MyMoneyAccount acc;
  QCStringList::ConstIterator it_s;

  loadAccounts();
  acc = MyMoneyFile::instance()->asset();
  for(it_s = acc.accountList().begin(); it_s != acc.accountList().end(); ++it_s) {
    if(account == MyMoneyFile::instance()->account(*it_s).name())
      break;
  }

  if(it_s == acc.accountList().end()) {
    acc = MyMoneyFile::instance()->liability();
    for(it_s = acc.accountList().begin(); it_s != acc.accountList().end(); ++it_s) {
      if(account == MyMoneyFile::instance()->account(*it_s).name())
        break;
    }
  }

  if(it_s != acc.accountList().end()) {
    selectAccount(*it_s, "", reconciliation);
  } else
    selectAccount("", "", false);
}

void KGlobalLedgerView::slotShowTransactionForm(bool show)
{
  for(int i = 0; i < MyMoneyAccount::MaxAccountTypes; ++i) {
    if(m_specificView[i] != 0)
      m_specificView[i]->slotShowTransactionForm(show);
  }
}

void KGlobalLedgerView::slotCancelEdit(void)
{
  for(int i = 0; i < MyMoneyAccount::MaxAccountTypes; ++i) {
    if(m_specificView[i] != 0)
      m_specificView[i]->slotCancelEdit();
  }
}

