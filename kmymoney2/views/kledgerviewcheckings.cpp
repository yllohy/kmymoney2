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
#include <qfocusdata.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kglobal.h>
#include <klocale.h>
#include <kpushbutton.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kledgerviewcheckings.h"
#include "../widgets/kmymoneytransactionform.h"
#include "../widgets/kmymoneyedit.h"
#include "../widgets/kmymoneydateinput.h"
#include "../widgets/kmymoneypayee.h"
#include "../widgets/kmymoneycategory.h"
#include "../widgets/kmymoneylineedit.h"
#include "../widgets/kmymoneyregistercheckings.h"

KLedgerViewCheckings::KLedgerViewCheckings(QWidget *parent, const char *name )
  : KLedgerView(parent,name)
{
  m_register = new kMyMoneyRegisterCheckings(this);
  m_register->setView(this);

  QGridLayout* formLayout = new QGridLayout( this, 1, 1, 11, 6, "Form1Layout");
  QVBoxLayout* buttonLayout = new QVBoxLayout( 0, 0, 6, "Layout2");
  QVBoxLayout* ledgerLayout = new QVBoxLayout( 0, 0, 6, "Layout3");

  m_detailsButton = new KPushButton(this, "detailsButton" );
  m_detailsButton->setText(i18n("Account Details"));
  buttonLayout->addWidget(m_detailsButton);

  m_reconcileButton = new KPushButton(this, "reconcileButton");
  m_reconcileButton->setText(i18n("Reconcile ..."));
  buttonLayout->addWidget(m_reconcileButton);

  QSpacerItem* spacer = new QSpacerItem( 20, 20,
                   QSizePolicy::Minimum, QSizePolicy::Expanding );
  buttonLayout->addItem( spacer );

  formLayout->addLayout( buttonLayout, 0, 1 );

  ledgerLayout->addWidget(m_register, 3);

  m_form = new kMyMoneyTransactionForm(this, 0, 0, 4, 5);
  m_tabCheck = new QTab(action2str(MyMoneySplit::ActionCheck, true));
  m_tabDeposit = new QTab(action2str(MyMoneySplit::ActionDeposit, true));
  m_tabTransfer = new QTab(action2str(MyMoneySplit::ActionTransfer, true));
  m_tabWithdrawal = new QTab(action2str(MyMoneySplit::ActionWithdrawal, true));
  m_tabAtm = new QTab(action2str(MyMoneySplit::ActionATM, true));

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

  // FIXME: make this a config setting and keep it in the rc file
  slotShowTransactionForm(m_transactionFormActive);

  // and the register has the focus
  m_register->setFocus();

  // connections
  connect(m_form->tabBar(), SIGNAL(selected(int)), this, SLOT(slotTypeSelected(int)));

  connect(m_register, SIGNAL(clicked(int, int, int, const QPoint&)), this, SLOT(slotRegisterClicked(int, int, int, const QPoint&)));
  connect(m_register, SIGNAL(doubleClicked(int, int, int, const QPoint&)), this, SLOT(slotRegisterDoubleClicked(int, int, int, const QPoint&)));

  connect(m_register, SIGNAL(signalEnter()), this, SLOT(slotStartEdit()));
  connect(m_register, SIGNAL(signalNextTransaction()), this, SLOT(slotNextTransaction()));
  connect(m_register, SIGNAL(signalPreviousTransaction()), this, SLOT(slotPreviousTransaction()));

  connect(m_form->editButton(), SIGNAL(clicked()), this, SLOT(slotStartEdit()));
  connect(m_form->cancelButton(), SIGNAL(clicked()), this, SLOT(slotCancelEdit()));
  connect(m_form->enterButton(), SIGNAL(clicked()), this, SLOT(slotEndEdit()));
  connect(m_form->newButton(), SIGNAL(clicked()), this, SLOT(slotNew()));
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
  // make sure, the QTabbar does not send out it's selected() signal
  // which would drive us crazy here. fillForm calls slotTypeSelected()
  // later on anyway.
  m_form->tabBar()->blockSignals(true);
  KLedgerView::show();
  m_form->tabBar()->blockSignals(false);

  fillForm();
  resizeEvent(NULL);
}

void KLedgerViewCheckings::slotTypeSelected(int type)
{
  if(!m_form->tabBar()->signalsBlocked())
    slotCancelEdit();

  QTable* formTable = m_form->table();

  // clear complete table
  for(int r = 0; r < formTable->numRows(); ++r) {
    for(int c = 0; c < formTable->numCols(); ++c) {
      formTable->setText(r, c, " ");
    }
  }

  // common elements
  formTable->setText(3, 0, i18n("Memo"));

  formTable->setText(1, 3, i18n("Date"));
  formTable->setText(2, 3, i18n("Amount"));

  // specific elements (in the order of the tabs)
  switch(type) {
    case 0:   // Check
      m_action = MyMoneySplit::ActionCheck;
      formTable->setText(1, 0, i18n("Receiver"));
      formTable->setText(2, 0, i18n("Category"));

      formTable->setText(0, 3, i18n("Nr"));
      break;

    case 1:   // Deposit
      m_action = MyMoneySplit::ActionDeposit;
      formTable->setText(1, 0, i18n("Payee"));
      formTable->setText(2, 0, i18n("Category"));
      break;

    case 2:   // Transfer
      m_action = MyMoneySplit::ActionTransfer;
      formTable->setText(0, 0, i18n("From"));
      formTable->setText(1, 0, i18n("To"));
      formTable->setText(2, 0, i18n("Payee"));
      break;

    case 3:   // Withdrawal
      m_action = MyMoneySplit::ActionWithdrawal;
      formTable->setText(1, 0, i18n("Receiver"));
      formTable->setText(2, 0, i18n("Category"));
      break;

    case 4:   // ATM
      m_action = MyMoneySplit::ActionATM;
      formTable->setText(1, 0, i18n("Receiver"));
      formTable->setText(2, 0, i18n("Category"));
      break;
  }

  if(!m_form->tabBar()->signalsBlocked())
    slotNew();
}

void KLedgerViewCheckings::slotRegisterDoubleClicked(int row, int col, int button, const QPoint &mousePos)
{
  slotStartEdit();
}


void KLedgerViewCheckings::fillForm(void)
{
  QTable* formTable = m_form->table();
  m_transactionPtr = transaction(m_register->currentTransactionIndex());

  if(m_transactionPtr != 0) {
    // get my local copy of the selected transaction
    m_transaction = *m_transactionPtr;
    m_split = m_transaction.split(accountId());

    MyMoneyMoney amount = m_split.value();

    kMyMoneyTransactionFormTableItem* item;

    // setup the fields first
    m_form->tabBar()->blockSignals(true);
    m_form->tabBar()->setCurrentTab(transactionType(m_split));
    slotTypeSelected(transactionType(m_split));
    m_form->tabBar()->blockSignals(false);

    // fill in common fields
    formTable->setText(3, 1, m_split.memo());

    // make sure, that the date is aligned to the right of the cell
    item = new kMyMoneyTransactionFormTableItem(formTable, QTableItem::Never, KGlobal::locale()->formatDate(m_transaction.postDate(), true));
    item->setAlignment(kMyMoneyTransactionFormTableItem::right);
    formTable->setItem(1, 4, item);

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
      if(m_transaction.splitCount() == 2) {
        MyMoneySplit s = m_transaction.split(accountId(), false);
        category = MyMoneyFile::instance()->accountToCategory(s.accountId());
      } else {
        category = i18n("Splitted transaction");
      }
    } catch (MyMoneyException *e) {
      delete e;
      category = " ";
    }

    QString from;
    try {
      from = MyMoneyFile::instance()->accountToCategory(m_account.id());
    } catch (MyMoneyException *e) {
      delete e;
      from = " ";
    }

    // then fill in the data depending on the transaction type
    switch(transactionType(m_split)) {
      case Check:
        // receiver
        formTable->setText(1, 1, payee);

        // category
        formTable->setText(2, 1, category);

        // number
        item = new kMyMoneyTransactionFormTableItem(formTable, QTableItem::Never, m_split.number());
        item->setAlignment(kMyMoneyTransactionFormTableItem::right);
        formTable->setItem(0, 4, item);

        amount = -amount;
        break;

      case Deposit:
        // payee
        formTable->setText(1, 1, payee);

        // category
        formTable->setText(2, 1, category);
        break;

      case Transfer:
        // if it's the deposit part, we have to exchange from and to first
        if(amount >= 0) {
          QString tmp = category;
          category = from;
          from = tmp;
        } else
          amount = -amount;

        // from
        formTable->setText(0, 1, from);

        // to
        formTable->setText(1, 1, category);

        // receiver
        formTable->setText(2, 1, payee);
        break;

      case Withdrawal:
        // receiver
        formTable->setText(1, 1, payee);

        // category
        formTable->setText(2, 1, category);

        amount = -amount;
        break;

      case ATM:
        // receiver
        formTable->setText(1, 1, payee);

        // category
        formTable->setText(2, 1, category);

        amount = -amount;
        break;
    }
    formTable->setText(2, 4, amount.formatMoney());


    m_form->newButton()->setEnabled(true);
    m_form->editButton()->setEnabled(true);
    m_form->enterButton()->setEnabled(false);
    m_form->cancelButton()->setEnabled(false);
    m_form->moreButton()->setEnabled(false);
  } else {
    m_transaction = MyMoneyTransaction();
    m_split = MyMoneySplit();
    m_split.setAccountId(accountId());
    m_split.setAction(m_action);
    m_transaction.addSplit(m_split);

    // transaction empty, clean out space
    for(int i = 0; i < formTable->numRows(); ++i) {
      formTable->setText(i, 1, "");
      formTable->setText(i, 4, "");
    }

    m_form->newButton()->setEnabled(true);
    m_form->editButton()->setEnabled(false);
    m_form->enterButton()->setEnabled(false);
    m_form->cancelButton()->setEnabled(false);
    m_form->moreButton()->setEnabled(false);
  }
}

void KLedgerViewCheckings::showWidgets(void)
{
  QPalette palette = m_register->palette();
  QWidget* focusWidget;
  MyMoneyMoney amount;

  QTableItem* item;

  item = m_form->table()->item(0,1);
  item->setSpan(1, 2);
  item = m_form->table()->item(1,1);
  item->setSpan(1, 2);
  item = m_form->table()->item(2,1);
  item->setSpan(1, 1);
  item = m_form->table()->item(3,1);
  item->setSpan(1, 2);

  // clear the tab order
  m_tabOrderWidgets.clear();

  focusWidget = m_editPayee = new kMyMoneyPayee(0, "editPayee");
  m_editCategory = new kMyMoneyCategory(0, "editCategory");
  m_editMemo = new kMyMoneyLineEdit(0, "editMemo");
  m_editAmount = new kMyMoneyEdit(0, "editAmount");
  m_editDate = new kMyMoneyDateInput(0, "editDate");
  m_editNr = new kMyMoneyLineEdit(0, "editNr");
  m_editFrom = new kMyMoneyCategory(0, "editFrom", static_cast<kMyMoneyCategory::categoryTypeE> (kMyMoneyCategory::asset | kMyMoneyCategory::liability));
  m_editTo = new kMyMoneyCategory(0, "editTo", static_cast<kMyMoneyCategory::categoryTypeE> (kMyMoneyCategory::asset | kMyMoneyCategory::liability));
  m_editSplit = new KPushButton("Split", 0, "editSplit");

  // for now, the split button is not usable
  m_editSplit->setEnabled(false);

  connect(m_editPayee, SIGNAL(newPayee(const QString&)), this, SLOT(slotNewPayee(const QString&)));
  connect(m_editPayee, SIGNAL(payeeChanged(const QString&)), this, SLOT(slotPayeeChanged(const QString&)));
  connect(m_editMemo, SIGNAL(lineChanged(const QString&)), this, SLOT(slotMemoChanged(const QString&)));
  connect(m_editCategory, SIGNAL(categoryChanged(const QString&)), this, SLOT(slotCategoryChanged(const QString&)));
  connect(m_editAmount, SIGNAL(valueChanged(const QString&)), this, SLOT(slotAmountChanged(const QString&)));
  connect(m_editNr, SIGNAL(lineChanged(const QString&)), this, SLOT(slotNrChanged(const QString&)));
  connect(m_editDate, SIGNAL(dateChanged(const QDate&)), this, SLOT(slotDateChanged(const QDate&)));
  connect(m_editFrom, SIGNAL(categoryChanged(const QString&)), this, SLOT(slotFromChanged(const QString&)));
  connect(m_editTo, SIGNAL(categoryChanged(const QString&)), this, SLOT(slotToChanged(const QString&)));

  connect(m_editPayee, SIGNAL(signalEnter()), this, SLOT(slotEndEdit()));
  connect(m_editMemo, SIGNAL(signalEnter()), this, SLOT(slotEndEdit()));
  connect(m_editCategory, SIGNAL(signalEnter()), this, SLOT(slotEndEdit()));
  connect(m_editNr, SIGNAL(signalEnter()), this, SLOT(slotEndEdit()));
  connect(m_editDate, SIGNAL(signalEnter()), this, SLOT(slotEndEdit()));
  connect(m_editFrom, SIGNAL(signalEnter()), this, SLOT(slotEndEdit()));
  connect(m_editTo, SIGNAL(signalEnter()), this, SLOT(slotEndEdit()));
  connect(m_editAmount, SIGNAL(signalEnter()), this, SLOT(slotEndEdit()));

  connect(m_editPayee, SIGNAL(signalEsc()), this, SLOT(slotCancelEdit()));
  connect(m_editMemo, SIGNAL(signalEsc()), this, SLOT(slotCancelEdit()));
  connect(m_editCategory, SIGNAL(signalEsc()), this, SLOT(slotCancelEdit()));
  connect(m_editNr, SIGNAL(signalEsc()), this, SLOT(slotCancelEdit()));
  connect(m_editDate, SIGNAL(signalEsc()), this, SLOT(slotCancelEdit()));
  connect(m_editFrom, SIGNAL(signalEsc()), this, SLOT(slotCancelEdit()));
  connect(m_editTo, SIGNAL(signalEsc()), this, SLOT(slotCancelEdit()));
  connect(m_editAmount, SIGNAL(signalEsc()), this, SLOT(slotCancelEdit()));

  // make sure, we're using the right palette
  m_editPayee->setPalette(palette);
  m_editCategory->setPalette(palette);
  m_editMemo->setPalette(palette);
  m_editAmount->setPalette(palette);
  m_editDate->setPalette(palette);
  m_editNr->setPalette(palette);
  m_editFrom->setPalette(palette);
  m_editTo->setPalette(palette);

  int transType;

  if(m_transactionPtr != 0) {
    QString category, payee;

    // get my local copy of the selected transaction
    m_transaction = *m_transactionPtr;
    m_split = m_transaction.split(accountId());
    amount = m_split.value();

    try {
      if(m_split.payeeId() != "")
        payee = MyMoneyFile::instance()->payee(m_split.payeeId()).name();

      if(m_transactionPtr->splitCount() == 2) {
        MyMoneySplit s = m_transactionPtr->split(accountId(), false);
        MyMoneyAccount acc = MyMoneyFile::instance()->account(s.accountId());

        // if the second account is also an asset or liability account
        // then this is a transfer type transaction and handled a little different
        switch(MyMoneyFile::instance()->accountGroup(acc.accountType())) {
          case MyMoneyAccount::Expense:
          case MyMoneyAccount::Income:
            category = MyMoneyFile::instance()->accountToCategory(s.accountId());
            break;

          case MyMoneyAccount::Asset:
          case MyMoneyAccount::Liability:
            if(m_split.action() != MyMoneySplit::ActionTransfer) {
              m_split.setAction(MyMoneySplit::ActionTransfer);
              m_transaction.modifySplit(m_split);
            }
            if(s.action() != MyMoneySplit::ActionTransfer) {
              s.setAction(MyMoneySplit::ActionTransfer);
              m_transaction.modifySplit(s);
            }
            if(m_split.value() > 0) {
              m_editFrom->loadText(MyMoneyFile::instance()->accountToCategory(s.accountId()));
              m_editTo->loadText(MyMoneyFile::instance()->accountToCategory(m_account.id()));
            } else {
              m_editTo->loadText(MyMoneyFile::instance()->accountToCategory(s.accountId()));
              m_editFrom->loadText(MyMoneyFile::instance()->accountToCategory(m_account.id()));
              amount = -amount;       // make it positive
            }
            break;

          default:
            qDebug("Unknown account group in KLedgerCheckingsView::showWidgets()");
            break;
        }
      } else {
        category = i18n("Splitted transaction");
      }


    } catch(MyMoneyException *e) {
      qDebug("Exception '%s' thrown in %s, line %ld caught in KLedgerViewCheckings::showWidgets():%d",
        e->what().latin1(), e->file().latin1(), e->line(), __LINE__);
      delete e;
    }

    // for almost all transaction types we have to negate the value
    // exceptions are: deposits and transfers (which are always positive)
    if(transactionType(m_split) != Deposit)
      amount = -amount;
    if(m_split.action() == MyMoneySplit::ActionTransfer && amount < 0) {
      amount = -amount;
    }

    m_editPayee->loadText(payee);
    m_editCategory->loadText(category);
    m_editMemo->loadText(m_split.memo());
    m_editAmount->loadText(amount.formatMoney());
    m_editDate->loadDate(m_transactionPtr->postDate());
    m_editNr->loadText(m_split.number());

    transType = transactionType(m_split);

  } else {
    m_editDate->setDate(QDate::currentDate());
    transType = m_form->tabBar()->currentTab();

    try {
      // setup a new transaction with the defaults
      switch(transType) {
        case Transfer: // Transfer
          m_split.setAction(MyMoneySplit::ActionTransfer);
          m_split.setAccountId(m_account.id());
          m_transaction.modifySplit(m_split);
          m_editFrom->loadText(m_account.name());
          break;
      }
    } catch(MyMoneyException *e) {
      qDebug("Exception '%s' thrown in %s, line %ld caught in KLedgerViewCheckings::showWidgets()",
        e->what().latin1(), e->file().latin1(), e->line());
      delete e;
    }
  }

  if(m_form->isVisible()) {
    m_form->table()->clearEditable();
    m_form->table()->setCellWidget(3, 1, m_editMemo);
    m_form->table()->setCellWidget(1, 4, m_editDate);
    m_form->table()->setCellWidget(2, 4, m_editAmount);

    m_form->table()->setEditable(1, 1);
    m_form->table()->setEditable(2, 1);
    m_form->table()->setEditable(3, 1);
    m_form->table()->setEditable(1, 4);
    m_form->table()->setEditable(2, 4);

    // depending on the transaction type, we figure out the
    // location of the fields in the form. A row info of -1 means,
    // that the field is not used in this type.
    int payeeRow = 1,
        categoryRow = 2,
        fromRow = 0,
        toRow = 1,
        nrRow = 0;
    switch(transType) {
      case Check: // Check
      case ATM: // ATM
        m_form->table()->setEditable(0, 4);
        fromRow = toRow = -1;
        break;

      case Deposit: // Deposit
      case Withdrawal: // Withdrawal
        fromRow = toRow = nrRow = -1;
        break;

      case Transfer: // Transfer
        item = m_form->table()->item(2,1);
        item->setSpan(1, 2);

        payeeRow = 2;
        categoryRow = -1;
        nrRow = -1;

        focusWidget = m_editTo;
        // m_form->table()->setEditable(0, 1);
        break;
    }

    m_form->table()->setCellWidget(payeeRow, 1, m_editPayee);

    if(fromRow != -1) {
      m_form->table()->setCellWidget(fromRow, 1, m_editFrom);
    } else {
      delete m_editFrom;
      m_editFrom = 0;
    }

    if(toRow != -1) {
      m_form->table()->setCellWidget(toRow, 1, m_editTo);
    } else {
      delete m_editTo;
      m_editTo = 0;
    }

    if(categoryRow != -1) {
      m_form->table()->setCellWidget(categoryRow, 1, m_editCategory);
      m_form->table()->setCellWidget(categoryRow, 2, m_editSplit);
    } else {
      delete m_editCategory;
      delete m_editSplit;
      m_editCategory = 0;
      m_editSplit = 0;
    }

    if(nrRow != -1) {
      m_form->table()->setCellWidget(0, 4, m_editNr);
    } else {
      delete m_editNr;
      m_editNr = 0;
    }

    // now setup the tab order

    m_tabOrderWidgets.append(m_form->enterButton());
    m_tabOrderWidgets.append(m_form->cancelButton());
    m_tabOrderWidgets.append(m_form->moreButton());

    switch(transType) {
      case Check: // Check
      case ATM: // ATM
        m_tabOrderWidgets.append(m_editPayee);
        m_tabOrderWidgets.append(m_editCategory);
        m_tabOrderWidgets.append(m_editSplit);
        m_tabOrderWidgets.append(m_editMemo);
        m_tabOrderWidgets.append(m_editNr);
        break;

      case Deposit: // Deposit
      case Withdrawal: // Withdrawal
        m_tabOrderWidgets.append(m_editPayee);
        m_tabOrderWidgets.append(m_editCategory);
        m_tabOrderWidgets.append(m_editSplit);
        m_tabOrderWidgets.append(m_editMemo);
        break;

      case Transfer: // Transfer
        m_tabOrderWidgets.append(m_editFrom);
        m_tabOrderWidgets.append(m_editTo);
        m_tabOrderWidgets.append(m_editPayee);
        m_tabOrderWidgets.append(m_editMemo);
        break;
    }

    m_tabOrderWidgets.append(m_editDate->focusWidget());
    m_tabOrderWidgets.append(m_editAmount);
    m_tabOrderWidgets.find(focusWidget);
    focusWidget->setFocus();

  } else {
    /// @todo FIXME: in register editing of transactions in KLedgerViewCheckings
  }
}

void KLedgerViewCheckings::hideWidgets(void)
{
  for(int i=0; i < m_form->table()->numRows(); ++i) {
    m_form->table()->clearCellWidget(i, 1);
    m_form->table()->clearCellWidget(i, 2);
    m_form->table()->clearCellWidget(i, 4);
  }

  m_editPayee = 0;
  m_editCategory = 0;
  m_editMemo = 0;
  m_editAmount = 0;
  m_editNr = 0;
  m_editDate = 0;
  m_editFrom = 0;
  m_editTo = 0;
  m_editSplit = 0;

  m_form->table()->clearEditable();
  m_form->tabBar()->setEnabled(true);
}

bool KLedgerViewCheckings::focusNextPrevChild(bool next)
{
  return KLedgerView::focusNextPrevChild(next);
}

