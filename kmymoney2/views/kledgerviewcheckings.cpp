/***************************************************************************
                          kledgerviewcheckings.cpp  -  description
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

// ----------------------------------------------------------------------------
// QT Includes

#include <qlayout.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kglobal.h>
#include <klocale.h>
#include <kpushbutton.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kledgerviewcheckings.h"

KLedgerViewCheckings::KLedgerViewCheckings(QWidget *parent, const char *name )
  : KLedgerView(parent,name)
{
  m_register = new kMyMoneyRegisterCheckings(this);
  m_register->setView(this);

  QGridLayout* formLayout = new QGridLayout( this, 1, 1, 11, 6, "Form1Layout");
  QVBoxLayout* buttonLayout = new QVBoxLayout( 0, 0, 6, "Layout2");
  QVBoxLayout* ledgerLayout = new QVBoxLayout( 0, 0, 6, "Layout3");

  KPushButton* detailsButton = new KPushButton(this, "detailsButton" );
  detailsButton->setText(i18n("Account Details"));
  buttonLayout->addWidget(detailsButton);

  KPushButton* reconcileButton = new KPushButton(this, "reconcileButton");
  reconcileButton->setText(i18n("Reconcile ..."));
  buttonLayout->addWidget(reconcileButton);

  QSpacerItem* spacer = new QSpacerItem( 20, 20,
                   QSizePolicy::Minimum, QSizePolicy::Expanding );
  buttonLayout->addItem( spacer );

  formLayout->addLayout( buttonLayout, 0, 1 );

  ledgerLayout->addWidget(m_register, 3);

  m_form = new kMyMoneyTransactionForm(this, 0, 0, 4, 4);
  m_tabCheck = new QTab(action2str(MyMoneySplit::ActionCheck));
  m_tabDeposit = new QTab(action2str(MyMoneySplit::ActionDeposit));
  m_tabTransfer = new QTab(action2str(MyMoneySplit::ActionTransfer));
  m_tabWithdrawal = new QTab(action2str(MyMoneySplit::ActionWithdrawal));
  m_tabAtm = new QTab(action2str(MyMoneySplit::ActionATM));

  m_form->addTab(m_tabCheck);
  m_form->addTab(m_tabDeposit);
  m_form->addTab(m_tabTransfer);
  m_form->addTab(m_tabWithdrawal);
  m_form->addTab(m_tabAtm);

  ledgerLayout->addWidget(m_form);

  formLayout->addLayout( ledgerLayout, 0, 0);

  m_register->setNumCols(7);
  m_register->setCurrentCell(0, 1);
  m_register->horizontalHeader()->setLabel(0, i18n("Nr."));
	m_register->horizontalHeader()->setLabel(1, i18n("Date"));
	m_register->horizontalHeader()->setLabel(2, i18n("Payee"));
	m_register->horizontalHeader()->setLabel(3, i18n("C"));
	m_register->horizontalHeader()->setLabel(4, i18n("Payment"));
	m_register->horizontalHeader()->setLabel(5, i18n("Deposit"));
	m_register->horizontalHeader()->setLabel(6, i18n("Balance"));
 	m_register->setLeftMargin(0);
	m_register->verticalHeader()->hide();
  m_register->setColumnStretchable(0, false);
  m_register->setColumnStretchable(1, false);
	m_register->setColumnStretchable(2, false);
  m_register->setColumnStretchable(3, false);
	m_register->setColumnStretchable(4, false);
  m_register->setColumnStretchable(5, false);
	m_register->setColumnStretchable(6, false);
		
	m_register->horizontalHeader()->setResizeEnabled(false);
	m_register->horizontalHeader()->setMovingEnabled(false);

  // never show horizontal scroll bars
  m_register->setHScrollBarMode(QScrollView::AlwaysOff);
  m_form->table()->setHScrollBarMode(QScrollView::AlwaysOff);

  // adjust size of form table
  m_form->table()->setMaximumHeight(m_form->table()->rowHeight(0)*m_form->table()->numRows());

  // connections
  connect(m_form->tabBar(), SIGNAL(selected(int)), this, SLOT(slotTypeSelected(int)));
  connect(m_register, SIGNAL(clicked(int, int, int, const QPoint&)), this, SLOT(slotRegisterClicked(int, int, int, const QPoint&)));
}

KLedgerViewCheckings::~KLedgerViewCheckings()
{
  delete m_register;
}

void KLedgerViewCheckings::refreshView(void)
{
  KLedgerView::refreshView();
}

void KLedgerViewCheckings::resizeEvent(QResizeEvent* ev)
{
  // resize the register
  int w = m_register->visibleWidth();

  int m_debitWidth = 100;
  int m_creditWidth = 100;
  int m_balanceWidth = 100;

  m_register->setColumnWidth(0, 100);
  m_register->adjustColumn(1);
  m_register->setColumnWidth(3, 20);
  m_register->setColumnWidth(4, m_debitWidth);
  m_register->setColumnWidth(5, m_creditWidth);
  m_register->setColumnWidth(6, m_balanceWidth);

  for(int i = 0; i < m_register->numCols(); ++i) {
    switch(i) {
      default:
        w -= m_register->columnWidth(i);
        break;
      case 2:     // skip the one, we want to set
        break;
    }
  }
  m_register->setColumnWidth(2, w);

  // now resize the form
  QTable* table = m_form->table();
  table->setColumnWidth(0, 80);
  table->setColumnWidth(2, 80);
  table->setColumnWidth(3, m_balanceWidth);

  w = table->visibleWidth();
  for(int i = 0; i < table->numCols(); ++i) {
    switch(i) {
      default:
        w -= table->columnWidth(i);
        break;
      case 1:     // skip the one, we want to set
        break;
    }
  }
  table->setColumnWidth(1, w);
}

void KLedgerViewCheckings::show()
{
  KLedgerView::show();
  fillForm();
  resizeEvent(NULL);
}

void KLedgerViewCheckings::slotTypeSelected(int type)
{
  QTable* formTable = m_form->table();

  // clear complete table
  for(int r = 0; r < formTable->numRows(); ++r) {
    for(int c = 0; c < formTable->numCols(); ++c) {
      formTable->setText(r, c, " ");
    }
  }

  // common elements
  formTable->setText(3, 0, i18n("Memo"));

  formTable->setText(1, 2, i18n("Date"));
  formTable->setText(2, 2, i18n("Amount"));

  // specific elements (in the order of the tabs)
  switch(type) {
    case 0:   // Check
      formTable->setText(1, 0, i18n("Receiver"));
      formTable->setText(2, 0, i18n("Category"));

      formTable->setText(0, 2, i18n("Nr"));
      break;

    case 1:   // Deposit
      formTable->setText(1, 0, i18n("Payee"));
      formTable->setText(2, 0, i18n("Category"));
      break;

    case 2:   // Transfer
      formTable->setText(0, 0, i18n("From"));
      formTable->setText(1, 0, i18n("To"));
      formTable->setText(2, 0, i18n("Payee"));
      break;

    case 3:   // Withdrawal
      formTable->setText(1, 0, i18n("Receiver"));
      formTable->setText(2, 0, i18n("Category"));
      break;

    case 4:   // ATM
      formTable->setText(1, 0, i18n("Receiver"));
      formTable->setText(2, 0, i18n("Category"));
      break;
  }
}

void KLedgerViewCheckings::fillForm(void)
{
  QTable* formTable = m_form->table();
  MyMoneyTransaction* t = transaction(m_register->currentTransactionIndex());

  if(t != 0) {
    MyMoneySplit split = t->split(accountId());
    QDate date;
    kMyMoneyTransactionFormTableItem* item;

    // setup the fields first
    m_form->tabBar()->setCurrentTab(transactionType(split));

    // fill in common fields
    formTable->setText(3, 1, split.memo());

    date = t->postDate();
    if(split.reconcileFlag() == MyMoneySplit::Reconciled
    || split.reconcileFlag() == MyMoneySplit::Frozen)
      date = split.reconcileDate();

    // make sure, that the date is aligned to the right of the cell
    item = new kMyMoneyTransactionFormTableItem(formTable, QTableItem::Never, KGlobal::locale()->formatDate(date, true));
    item->setAlignment(kMyMoneyTransactionFormTableItem::right);
    formTable->setItem(1, 3, item);

    formTable->setText(2, 3, split.value().formatMoney());

    // collect values
    QString payee;
    try {
      payee = MyMoneyFile::instance()->payee(split.payeeId()).name();
    } catch (MyMoneyException *e) {
      delete e;
      payee = " ";
    }

    // then fill in the data depending on the transaction type
    switch(transactionType(split)) {
      case 0:   // Check
        // receiver
        formTable->setText(1, 1, payee);

        // category
        // formTable->setText(2, 0, "Category");

        // number
        item = new kMyMoneyTransactionFormTableItem(formTable, QTableItem::Never, split.number());
        item->setAlignment(kMyMoneyTransactionFormTableItem::right);
        formTable->setItem(0, 3, item);
        break;

      case 1:   // Deposit
        // payee
        formTable->setText(1, 1, payee);

        // category
        // formTable->setText(2, 0, "Category");
        break;

      case 2:   // Transfer
        // formTable->setText(0, 0, "From");
        // formTable->setText(1, 0, "To");

        // receiver
        formTable->setText(2, 1, payee);
        break;

      case 3:   // Withdrawal
        // receiver
        formTable->setText(1, 1, payee);

        // formTable->setText(2, 0, "Category");
        break;

      case 4:   // ATM
        // receiver
        formTable->setText(1, 1, payee);

        // formTable->setText(2, 0, "Category");
        break;
    }
  } else {
    // transaction empty, clean out space
    for(int i = 0; i < formTable->numRows(); ++i) {
      formTable->setText(i, 1, "");
      formTable->setText(i, 3, "");
    }
  }
}

int KLedgerViewCheckings::transactionType(const MyMoneySplit& split) const
{
  if(split.action() == MyMoneySplit::ActionCheck)
    return 0;
  if(split.action() == MyMoneySplit::ActionDeposit)
    return 1;
  if(split.action() == MyMoneySplit::ActionTransfer)
    return 2;
  if(split.action() == MyMoneySplit::ActionWithdrawal)
    return 3;
  if(split.action() == MyMoneySplit::ActionATM)
    return 4;
  return 0;
}
