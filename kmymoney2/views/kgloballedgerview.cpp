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
  Form1Layout = new QVBoxLayout( this, 0, 6, "Form1Layout");

  Layout2 = new QHBoxLayout( 0, 0, 6, "Layout2");

  accountComboBox = new KComboBox( FALSE, this, "accountComboBox" );
  accountComboBox->setMinimumSize( QSize( 240, 0 ) );
  Layout2->addWidget( accountComboBox );
  QSpacerItem* spacer = new QSpacerItem( 20, 20,
                                         QSizePolicy::Expanding,
                                         QSizePolicy::Minimum );
  Layout2->addItem( spacer );
  Form1Layout->addLayout( Layout2 );

  m_accountStack = new QWidgetStack(this, "Stack");
  // Checkings account
  view = m_specificView[MyMoneyAccount::Checkings] = new KLedgerViewCheckings(this);
  m_accountStack->addWidget(view, MyMoneyAccount::Checkings);
  connect(view, SIGNAL(accountAndTransactionSelected(const QCString&, const QCString&)),
    this, SLOT(slotSelectAccountAndTransaction(const QCString&, const QCString&)));

  // Savings account
  view = m_specificView[MyMoneyAccount::Savings] = new KLedgerViewSavings(this);
  m_accountStack->addWidget(view, MyMoneyAccount::Savings);
  connect(view, SIGNAL(accountAndTransactionSelected(const QCString&, const QCString&)),
    this, SLOT(slotSelectAccountAndTransaction(const QCString&, const QCString&)));

  // Credit card account
  view = m_specificView[MyMoneyAccount::CreditCard] = new KLedgerViewCreditCard(this);
  m_accountStack->addWidget(view, MyMoneyAccount::CreditCard);
  connect(view, SIGNAL(accountAndTransactionSelected(const QCString&, const QCString&)),
    this, SLOT(slotSelectAccountAndTransaction(const QCString&, const QCString&)));

  // Cash account
  view = m_specificView[MyMoneyAccount::Cash] = new KLedgerViewCash(this);
  m_accountStack->addWidget(view, MyMoneyAccount::Cash);
  connect(view, SIGNAL(accountAndTransactionSelected(const QCString&, const QCString&)),
    this, SLOT(slotSelectAccountAndTransaction(const QCString&, const QCString&)));

  // Asset account
  view = m_specificView[MyMoneyAccount::Asset] = new KLedgerViewAsset(this);
  m_accountStack->addWidget(view, MyMoneyAccount::Asset);
  connect(view, SIGNAL(accountAndTransactionSelected(const QCString&, const QCString&)),
    this, SLOT(slotSelectAccountAndTransaction(const QCString&, const QCString&)));


  Form1Layout->addWidget(m_accountStack);

  // read the configuration
  m_accountId = "";

  // setup connections
  connect(accountComboBox, SIGNAL(activated(const QString&)),
          this, SLOT(slotAccountSelected(const QString&)));
}

void KGlobalLedgerView::loadAccounts(void)
{
  MyMoneyFile* file = MyMoneyFile::instance();

  accountComboBox->clear();

  MyMoneyAccount acc;
  QCStringList::ConstIterator it_s;

  acc = file->asset();
  for(it_s = acc.accountList().begin(); it_s != acc.accountList().end(); ++it_s) {
    accountComboBox->insertItem(file->account(*it_s).name());
    if(m_accountId == "") {
      selectAccount(*it_s);
      accountComboBox->setCurrentText(file->account(*it_s).name());
    }
  }

  acc = file->liability();
  for(it_s = acc.accountList().begin(); it_s != acc.accountList().end(); ++it_s) {
    accountComboBox->insertItem(file->account(*it_s).name());
  }

  if(m_accountId == "") {       // no accounts available?
    selectAccount("");
  }
}

KGlobalLedgerView::~KGlobalLedgerView()
{
}

void KGlobalLedgerView::show()
{
  loadAccounts();
  // only show selection box if filled with at least one account
  accountComboBox->setEnabled(accountComboBox->count() > 0);

  emit signalViewActivated();
  QWidget::show();
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
  m_accountId = "";
  for(int i = 0; i < MyMoneyAccount::MaxAccountTypes; ++i) {
    if(m_specificView[i] != 0)
      m_specificView[i]->setCurrentAccount("");
  }
  loadAccounts();
  accountComboBox->setEnabled(accountComboBox->count() > 0);
  refreshView();
}

void KGlobalLedgerView::refreshView(void)
{
  for(int i = 0; i < MyMoneyAccount::MaxAccountTypes; ++i) {
    if(m_specificView[i] != 0)
      m_specificView[i]->refreshView();
  }
}

void KGlobalLedgerView::slotSelectAccountAndTransaction(const QCString& accountId, const QCString& transactionId)
{
  selectAccount(accountId);
  m_currentView->selectTransaction(transactionId);
}

void KGlobalLedgerView::selectAccount(const QCString& accountId, const bool reconciliation)
{
  slotCancelEdit();
  if(accountId != "") {
    MyMoneyAccount acc = MyMoneyFile::instance()->account(accountId);
    if(m_specificView[acc.accountType()] != 0) {
      m_currentView = m_specificView[acc.accountType()];
      m_currentView->setCurrentAccount(accountId);
      m_accountStack->raiseWidget(acc.accountType());
      m_accountId = accountId;
      accountComboBox->setCurrentText(acc.name());
    } else {
      QString msg = "Specific ledger view for account type " +
        QString::number(acc.accountType()) + " not yet implemented";
      KMessageBox::sorry(0, msg, "Implementation problem");
    }
  } else {
    m_accountStack->raiseWidget(MyMoneyAccount::Checkings);
    if(m_specificView[MyMoneyAccount::Checkings] != 0) {
      m_currentView = m_specificView[MyMoneyAccount::Checkings];
    }
  }
  if(reconciliation == true && m_currentView)
    m_currentView->slotReconciliation();
}

void KGlobalLedgerView::slotAccountSelected(const QString& account)
{
  MyMoneyAccount acc;
  QCStringList::ConstIterator it_s;

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
    selectAccount(*it_s);
  }
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
