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
#include "../mymoney/mymoneyaccount.h"
#include "../mymoney/mymoneyfile.h"

KGlobalLedgerView::KGlobalLedgerView(QWidget *parent, const char *name )
  : QWidget(parent,name)
{
  m_currentView = 0;

  for(int i = 0; i < MyMoneyAccount::MaxAccountTypes; ++i)
    m_specificView[i] = 0;

  if ( !name )
    setName( "Account register" );

  setCaption( i18n( "Account register" ) );
  Form1Layout = new QVBoxLayout( this, 11, 6, "Form1Layout");

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
  m_specificView[MyMoneyAccount::Checkings] = new KLedgerViewCheckings(this);
  m_accountStack->addWidget(m_specificView[MyMoneyAccount::Checkings],
                                 MyMoneyAccount::Checkings);

  // Savings account
  KPushButton* savingsAccount = new KPushButton("Savings account", m_accountStack);
  m_accountStack->addWidget(savingsAccount, MyMoneyAccount::Savings);

  // Credit card account
  KPushButton* creditCardAccount = new KPushButton("Credit Card account", m_accountStack);
  m_accountStack->addWidget(creditCardAccount, MyMoneyAccount::CreditCard);

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
}

KGlobalLedgerView::~KGlobalLedgerView()
{
}

void KGlobalLedgerView::show()
{
  refresh();
  // only show selection box if filled with at least one account
  accountComboBox->setEnabled(accountComboBox->count() > 0);

  emit signalViewActivated();
  QWidget::show();
}


void KGlobalLedgerView::refresh(void)
{
  loadAccounts();
}

void KGlobalLedgerView::refreshView(void)
{
  // FIXME: this should actually call all views on the stack not
  //        only the current selected view
  if(m_currentView != 0)
    m_currentView->refreshView();
}

void KGlobalLedgerView::selectAccount(const QCString& accountId)
{
  MyMoneyAccount acc = MyMoneyFile::instance()->account(accountId);
  m_accountStack->raiseWidget(acc.accountType());
  if(m_specificView[acc.accountType()] != 0) {
    m_currentView = m_specificView[acc.accountType()];
    m_currentView->setCurrentAccount(accountId);
    m_accountId = accountId;
    accountComboBox->setCurrentText(acc.name());
  } else {
    QString msg = "Specific ledger view for account type " +
      QString::number(acc.accountType()) + " not yet implemented";
    KMessageBox::sorry(0, msg, "Implementation problem");
  }
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
