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
  connect(m_register, SIGNAL(doubleClicked(int, int, int, const QPoint&)), this, SLOT(slotRegisterDoubleClicked(int, int, int, const QPoint&)));

  connect(this, SIGNAL(transactionSelected()), this, SLOT(slotCancelEdit()));

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
    m_split = m_transactionPtr->split(accountId());
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
    item = new kMyMoneyTransactionFormTableItem(formTable, QTableItem::Never, KGlobal::locale()->formatDate(m_transactionPtr->postDate(), true));
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
      if(m_transactionPtr->splitCount() == 2) {
        MyMoneySplit s = m_transactionPtr->split(accountId(), false);
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
    m_split = MyMoneySplit();

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

void KLedgerViewCheckings::slotNew(void)
{
  // select the very last line (empty one), and load it into the form
  m_register->setCurrentTransactionIndex(m_transactionList.count());
  m_register->repaintContents();
  fillForm();

  m_form->enterButton()->setEnabled(true);
  m_form->cancelButton()->setEnabled(true);
  m_form->moreButton()->setEnabled(true);
  m_form->editButton()->setEnabled(false);
  m_form->newButton()->setEnabled(false);

  showWidgets();
}

void KLedgerViewCheckings::slotStartEdit(void)
{
  m_form->newButton()->setEnabled(true);
  m_form->enterButton()->setEnabled(true);
  m_form->cancelButton()->setEnabled(true);
  m_form->moreButton()->setEnabled(true);
  m_form->editButton()->setEnabled(false);

  showWidgets();
}

void KLedgerViewCheckings::slotCancelEdit(void)
{
  m_form->newButton()->setEnabled(true);
  m_form->enterButton()->setEnabled(false);
  m_form->cancelButton()->setEnabled(false);
  m_form->moreButton()->setEnabled(false);

  if(transaction(m_register->currentTransactionIndex()) != 0) {
    m_form->editButton()->setEnabled(true);
  }

  hideWidgets();
}

void KLedgerViewCheckings::slotEndEdit(void)
{
  MyMoneyTransaction t;

  // so, we now have to save something here.
  // if an existing transaction has been changed, we take it as the base
  if(m_transactionPtr != 0) {
    t = *m_transactionPtr;
  }

  // perform any checks on the data, before we start storeing the transaction

      // FIXME: add those checks, currently I don't know what could be done

  // checks are done, now we store the data and switch the context
  // first the transaction data
  t.setMemo(m_editMemo->text());
  t.setPostDate(m_editDate->getQDate());

  // now the splits. for now, we don't worry about splitted
  // transactions. so there always will be two splits!

  try {
    MyMoneySplit accSplit;
    MyMoneySplit catSplit;

    // locate the payee in the list
    QValueList<MyMoneyPayee> payeeList = MyMoneyFile::instance()->payeeList();
    QValueList<MyMoneyPayee>::ConstIterator it_p;
    for(it_p = payeeList.begin(); it_p != payeeList.end(); ++it_p) {
      if((*it_p).name() == m_editPayee->text())
        break;
    }
    if(it_p == payeeList.end())
      throw new MYMONEYEXCEPTION("Unable to find payee in list");

    accSplit = t.split(accountId());
    catSplit = t.split(accountId(), false);

    accSplit.setAccountId(accountId());
    accSplit.setMemo(m_editMemo->text());
    if(m_editNr != 0)
      accSplit.setNumber(m_editNr->text());
    accSplit.setPayeeId((*it_p).id());
    // accSplit.setValue();
    accSplit.setShares(accSplit.value());
    // accSplit.setAction();

  } catch(MyMoneyException *e) {
    qDebug("Exception '%s' thrown in %s, line %ld caught in KLedgerViewCheckings::showWidgets()",
      e->what().latin1(), e->file().latin1(), e->line());
    delete e;
  }

  m_form->newButton()->setEnabled(true);
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
  QPalette palette = m_register->palette();
  QWidget* focusWidget;

  focusWidget = m_editPayee = new kMyMoneyPayee();
  m_editCategory = new kMyMoneyCategory();
  m_editMemo = new kMyMoneyLineEdit(0);
  m_editAmount = new kMyMoneyEdit(0);
  m_editDate = new kMyMoneyDateInput();
  m_editNr = new kMyMoneyLineEdit(0);

  connect(m_editPayee, SIGNAL(newPayee(const QString&)), this, SLOT(slotNewPayee(const QString&)));

  // make sure, we're using the right palette
  m_editPayee->setPalette(palette);
  m_editCategory->setPalette(palette);
  m_editMemo->setPalette(palette);
  m_editAmount->setPalette(palette);
  m_editDate->setPalette(palette);
  m_editNr->setPalette(palette);

  int transType;

  if(m_transactionPtr != 0) {
    QString category, payee;
    try {
      if(m_split.payeeId() != "")
        payee = MyMoneyFile::instance()->payee(m_split.payeeId()).name();

      if(m_transactionPtr->splitCount() == 2) {
        MyMoneySplit s = m_transactionPtr->split(accountId(), false);
        category = MyMoneyFile::instance()->accountToCategory(s.accountId());
      } else {
        category = i18n("Splitted transaction");
      }

    } catch(MyMoneyException *e) {
      qDebug("Exception '%s' thrown in %s, line %ld caught in KLedgerViewCheckings::showWidgets()",
        e->what().latin1(), e->file().latin1(), e->line());
      delete e;
    }

    MyMoneyMoney amount = m_split.value();
    // for all other transaction types than deposit, we
    // have to negate the value
    if(transactionType(m_split) != 2)
      amount = -amount;

    m_editPayee->setText(payee);
    m_editCategory->setText(category);
    m_editMemo->setText(m_split.memo());
    m_editAmount->setText(amount.formatMoney());
    m_editDate->setDate(m_transactionPtr->postDate());
    m_editNr->setText(m_split.number());

    transType = transactionType(m_split);

  } else {
    m_editDate->setDate(QDate::currentDate());
    transType = m_form->tabBar()->currentTab();
  }

  if(m_form->isVisible()) {
    m_form->table()->clearEditable();
    m_form->table()->setCellWidget(3, 1, m_editMemo);
    m_form->table()->setCellWidget(1, 3, m_editDate);
    m_form->table()->setCellWidget(2, 3, m_editAmount);

    m_form->table()->setEditable(1, 1);
    m_form->table()->setEditable(2, 1);
    m_form->table()->setEditable(3, 1);
    m_form->table()->setEditable(1, 3);
    m_form->table()->setEditable(2, 3);

    // depending on the transaction type, we figure out the
    // location of the fields in the form. A row info of -1 means,
    // that the field is not used in this type.
    int payeeRow = 1,
        categoryRow = 2,
        nrRow = 0;
    switch(transType) {
      case 0:
      case 4:
        m_form->table()->setEditable(0, 3);
        break;

      case 1:
      case 3:
        nrRow = -1;
        break;

      case 2:
        payeeRow = 2;
        categoryRow = -1;
        nrRow = -1;

        m_form->table()->setEditable(0, 1);
        break;
    }

    m_form->table()->setCellWidget(payeeRow, 1, m_editPayee);

    if(categoryRow != -1) {
      m_form->table()->setCellWidget(categoryRow, 1, m_editCategory);
    } else {
      delete m_editCategory;
      m_editCategory = 0;
    }

    if(nrRow != -1) {
      m_form->table()->setCellWidget(0, 3, m_editNr);
    } else {
      delete m_editNr;
      m_editNr = 0;
    }

    focusWidget->setFocus();
  } else {
    /// todo: in register editing of transactions in KLedgerViewCheckings
  }
}

void KLedgerViewCheckings::hideWidgets(void)
{
  for(int i=0; i < m_form->table()->numRows(); ++i) {
    m_form->table()->clearCellWidget(i, 1);
    m_form->table()->clearCellWidget(i, 3);
  }
  m_form->table()->clearEditable();
}
