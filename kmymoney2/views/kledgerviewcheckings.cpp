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
#include "../widgets/kmymoneypayee.h"
#include "../widgets/kmymoneycategory.h"
#include "../widgets/kmymoneylineedit.h"

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

  // for now, show the form upon program start. We should
  // make this a config setting and keep it in the rc file
  slotShowTransactionForm(true);

  // and the register has the focus
  m_register->setFocus();

  // connections
  connect(m_form->tabBar(), SIGNAL(selected(int)), this, SLOT(slotTypeSelected(int)));
  connect(m_register, SIGNAL(clicked(int, int, int, const QPoint&)), this, SLOT(slotRegisterClicked(int, int, int, const QPoint&)));

  connect(m_form->editButton(), SIGNAL(clicked()), this, SLOT(slotStartEdit()));
  connect(m_form->cancelButton(), SIGNAL(clicked()), this, SLOT(slotCancelEdit()));
  connect(m_form->enterButton(), SIGNAL(clicked()), this, SLOT(slotEndEdit()));
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
  m_transaction = transaction(m_register->currentTransactionIndex());

  if(m_transaction != 0) {
    m_split = m_transaction->split(accountId());
    MyMoneyMoney amount = m_split.value();

    kMyMoneyTransactionFormTableItem* item;

    // setup the fields first
    m_form->tabBar()->setCurrentTab(transactionType(m_split));

    // fill in common fields
    formTable->setText(3, 1, m_split.memo());

    // make sure, that the date is aligned to the right of the cell
    item = new kMyMoneyTransactionFormTableItem(formTable, QTableItem::Never, KGlobal::locale()->formatDate(m_transaction->postDate(), true));
    item->setAlignment(kMyMoneyTransactionFormTableItem::right);
    formTable->setItem(1, 3, item);

    // collect values
    QString payee;
    try {
      payee = MyMoneyFile::instance()->payee(m_split.payeeId()).name();
    } catch (MyMoneyException *e) {
      delete e;
      payee = " ";
    }

    QString category;
    try {
      if(m_transaction->splitCount() == 2) {
        MyMoneySplit s = m_transaction->split(accountId(), false);
        category = MyMoneyFile::instance()->accountToCategory(s.accountId());
      } else {
        category = i18n("Splitted transaction");
      }
    } catch (MyMoneyException *e) {
      delete e;
      category = " ";
    }

    // then fill in the data depending on the transaction type
    switch(transactionType(m_split)) {
      case 0:   // Check
        // receiver
        formTable->setText(1, 1, payee);

        // category
        formTable->setText(2, 1, category);

        // number
        item = new kMyMoneyTransactionFormTableItem(formTable, QTableItem::Never, m_split.number());
        item->setAlignment(kMyMoneyTransactionFormTableItem::right);
        formTable->setItem(0, 3, item);

        amount = -amount;
        break;

      case 1:   // Deposit
        // payee
        formTable->setText(1, 1, payee);

        // category
        formTable->setText(2, 1, category);
        break;

      case 2:   // Transfer
        // formTable->setText(0, 0, "From");
        // formTable->setText(1, 0, "To");

        // receiver
        formTable->setText(2, 1, payee);

        amount = -amount;
        break;

      case 3:   // Withdrawal
        // receiver
        formTable->setText(1, 1, payee);

        // category
        formTable->setText(2, 1, category);

        amount = -amount;
        break;

      case 4:   // ATM
        // receiver
        formTable->setText(1, 1, payee);

        // category
        formTable->setText(2, 1, category);

        amount = -amount;
        break;
    }
    formTable->setText(2, 3, amount.formatMoney());


    m_form->newButton()->setEnabled(true);
    m_form->editButton()->setEnabled(true);
    m_form->enterButton()->setEnabled(false);
    m_form->cancelButton()->setEnabled(false);
    m_form->moreButton()->setEnabled(false);
  } else {
    // transaction empty, clean out space
    for(int i = 0; i < formTable->numRows(); ++i) {
      formTable->setText(i, 1, "");
      formTable->setText(i, 3, "");
    }

    m_form->newButton()->setEnabled(true);
    m_form->editButton()->setEnabled(false);
    m_form->enterButton()->setEnabled(false);
    m_form->cancelButton()->setEnabled(false);
    m_form->moreButton()->setEnabled(false);
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

void KLedgerViewCheckings::slotStartEdit(void)
{
  m_form->enterButton()->setEnabled(true);
  m_form->cancelButton()->setEnabled(true);
  m_form->moreButton()->setEnabled(true);
  m_form->editButton()->setEnabled(false);

  showWidgets();
}

void KLedgerViewCheckings::slotCancelEdit(void)
{
  m_form->enterButton()->setEnabled(false);
  m_form->cancelButton()->setEnabled(false);
  m_form->moreButton()->setEnabled(false);

  if(transaction(m_register->currentTransactionIndex()) != 0) {
    m_form->editButton()->setEnabled(true);
  }

  hideWidgets();
  fillForm();
}

void KLedgerViewCheckings::slotEndEdit(void)
{
  m_form->enterButton()->setEnabled(false);
  m_form->cancelButton()->setEnabled(false);
  m_form->moreButton()->setEnabled(false);

  if(transaction(m_register->currentTransactionIndex()) != 0) {
    m_form->editButton()->setEnabled(true);
  }

  hideWidgets();
}

void KLedgerViewCheckings::showWidgets(void)
{
  kMyMoneyPayee* test = new kMyMoneyPayee();

  QPalette palette = m_register->palette();

  m_editPayee = new kMyMoneyPayee();
  m_editCategory = new kMyMoneyCategory();
  m_editMemo = new kMyMoneyLineEdit(0);
  m_editAmount = new kMyMoneyEdit(0);
  m_editDate = new kMyMoneyDateInput();

  connect(m_editPayee, SIGNAL(newPayee(const QString&)), this, SLOT(slotNewPayee(const QString&)));

  m_editPayee->setPalette(palette);
  m_editPayee->setText(m_form->table()->text(1, 1));

  m_editCategory->setPalette(palette);
  m_editCategory->setText(m_form->table()->text(2, 1));

  m_editMemo->setPalette(palette);
  m_editMemo->setText(m_form->table()->text(3, 1));

  m_editAmount->setPalette(palette);
  m_editAmount->setText(m_form->table()->text(2, 3));

  m_editDate->setPalette(palette);
  m_editDate->setDate(m_transaction->postDate());

  if(m_form->isVisible()) {
    m_form->table()->setCellWidget(1, 1, m_editPayee);
    m_form->table()->setCellWidget(2, 1, m_editCategory);
    m_form->table()->setCellWidget(3, 1, m_editMemo);
    m_form->table()->setCellWidget(2, 3, m_editAmount);
    m_form->table()->setCellWidget(1, 3, m_editDate);
  } else {
  }
}

void KLedgerViewCheckings::hideWidgets(void)
{
  m_form->table()->clearCellWidget(1, 1);
  m_form->table()->clearCellWidget(2, 1);
  m_form->table()->clearCellWidget(3, 1);
  m_form->table()->clearCellWidget(1, 3);
  m_form->table()->clearCellWidget(2, 3);
}
