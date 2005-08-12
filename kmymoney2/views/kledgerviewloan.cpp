/***************************************************************************
                          kledgerviewloan.cpp  -  description
                             -------------------
    begin                : Sat Sep 13 2003
    copyright            : (C) 2003 by Thomas Baumgart
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
#include <qwidgetstack.h>
#include <qcheckbox.h>
#include <qregexp.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kglobal.h>
#include <klocale.h>
#include <kpushbutton.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kledgerviewloan.h"
#include "kledgerviewcheckings.h"

#include "../widgets/kmymoneytransactionform.h"
#include "../widgets/kmymoneyedit.h"
#include "../widgets/kmymoneydateinput.h"
#include "../widgets/kmymoneypayee.h"
#include "../widgets/kmymoneycategory.h"
#include "../widgets/kmymoneylineedit.h"
#include "../widgets/kmymoneycombo.h"
#include "../widgets/kmymoneyregisterloan.h"
#include "../dialogs/kendingbalancedlg.h"
#include "../dialogs/ksplittransactiondlg.h"
#include "../dialogs/knewaccountdlg.h"
#include "../dialogs/keditloanwizard.h"
#include "../mymoney/mymoneyfile.h"

#define PAYEE_ROW         1
#define CATEGORY_ROW      2
#define MEMO_ROW          3
#define NR_ROW            0
#define DATE_ROW          1
#define AMORTIZATION_ROW  2
#define AMOUNT_ROW        3

#define PAYEE_TXT_COL           0
#define PAYEE_DATA_COL          (PAYEE_TXT_COL+1)
#define CATEGORY_TXT_COL        0
#define CATEGORY_DATA_COL       (CATEGORY_TXT_COL+1)
#define MEMO_TXT_COL            0
#define MEMO_DATA_COL           (MEMO_TXT_COL+1)
#define NR_TXT_COL              3
#define NR_DATA_COL             (NR_TXT_COL+1)
#define DATE_TXT_COL            3
#define DATE_DATA_COL           (DATE_TXT_COL+1)
#define AMORTIZATION_TXT_COL    3
#define AMORTIZATION_DATA_COL   (AMOUNT_TXT_COL+1)
#define AMOUNT_TXT_COL          3
#define AMOUNT_DATA_COL         (AMOUNT_TXT_COL+1)

KLedgerViewLoan::KLedgerViewLoan(QWidget *parent, const char *name ) :
  KLedgerView(parent, name)
{
  QGridLayout* formLayout = new QGridLayout( this, 1, 2, 11, 6, "FormLayout");
  QVBoxLayout* ledgerLayout = new QVBoxLayout( 6, "LedgerLayout");

  createRegister();
  ledgerLayout->addWidget(m_register, 3);

  createSummary();
  ledgerLayout->addWidget(m_summaryLine);

  createForm();
  ledgerLayout->addWidget(m_form);

  formLayout->addLayout( ledgerLayout, 0, 0);

  // create the context menu that is accessible via RMB
  createContextMenu();

  // create the context menu that is accessible via the MORE Button
  createMoreMenu();
  createAccountMenu();

  connect(m_contextMenu, SIGNAL(aboutToShow()), SLOT(slotConfigureContextMenu()));
  connect(m_moreMenu, SIGNAL(aboutToShow()), SLOT(slotConfigureMoreMenu()));

  // load the form with inital settings. Always consider transaction type Deposit
  m_form->tabBar()->blockSignals(true);
  fillFormStatics();
  m_form->tabBar()->blockSignals(false);

  // setup the form to be visible or not
  slotShowTransactionForm(m_transactionFormActive);

  // and the register has the focus
  m_register->setFocus();
}

KLedgerViewLoan::~KLedgerViewLoan()
{
}

void KLedgerViewLoan::refreshView(void)
{
  KLedgerView::refreshView();

  QDate date;
  if(!m_account.value("lastStatementDate").isEmpty())
    date = QDate::fromString(m_account.value("lastStatementDate"), Qt::ISODate);
// FIXME: move to fillSummary()
#if 0
  if(date.isValid())
    m_lastReconciledLabel->setText(i18n("Reconciled: %1").arg(KGlobal::locale()->formatDate(date, true)));
  else
    m_lastReconciledLabel->setText(QString());
#endif
}

void KLedgerViewLoan::enableWidgets(const bool enable)
{
  KLedgerView::enableWidgets(enable);
}

void KLedgerViewLoan::resizeEvent(QResizeEvent* /* ev */)
{
  m_register->setUpdatesEnabled(false);

  // resize the register
  int w = m_register->visibleWidth();

  m_register->adjustColumn(1);
  m_register->adjustColumn(3);
  m_register->adjustColumn(4);
  m_register->adjustColumn(5);

  int width = m_register->columnWidth(3);
  int width1 = m_register->columnWidth(4);
  int width2 = m_register->columnWidth(5);

  if(width < width1)
    width = width1;
  if(width < width2)
    width = width2;

  // Resize the date and money fields to either
  // a) the size required by the input widget if no transaction form is shown
  // b) the adjusted value for the input widget if the transaction form is visible
  if(!m_transactionFormActive) {
    kMyMoneyDateInput* datefield = new kMyMoneyDateInput();
    datefield->setFont(m_register->cellFont());
    m_register->setColumnWidth(0, datefield->minimumSizeHint().width());
    delete datefield;
    kMyMoneyEdit* valfield = new kMyMoneyEdit();
    valfield->setMinimumWidth(width);
    width = valfield->minimumSizeHint().width();
    delete valfield;
  } else {
    m_register->adjustColumn(0);
  }

  m_register->setColumnWidth(3, width);
  m_register->setColumnWidth(4, width);
  m_register->setColumnWidth(5, width);

  for(int i = 0; i < m_register->numCols(); ++i) {
    switch(i) {
      default:
        w -= m_register->columnWidth(i);
        break;
      case 2:     // skip the one, we want to set (payee)
        break;
    }
  }
  m_register->setColumnWidth(2, w);

  m_register->setUpdatesEnabled(true);
  m_register->repaintContents(false);

  // now resize the form
  kMyMoneyDateInput dateInput(0, "editDate");
  KPushButton splitButton(i18n("Split"), 0, "editSplit");

  kMyMoneyTransactionFormTable* table = static_cast<kMyMoneyTransactionFormTable *>(m_form->table());
  table->adjustColumn(0);
  table->setColumnWidth(2, splitButton.sizeHint().width());
  table->adjustColumn(3);
  table->adjustColumn(4, dateInput.minimumSizeHint().width()+10);

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

void KLedgerViewLoan::slotRegisterDoubleClicked(int /* row */,
                                                int /* col */,
                                                int /* button */,
                                                const QPoint & /* mousePos */)
{
  if(m_transactionPtr != 0)
    slotStartEdit();
}

void KLedgerViewLoan::createRegister(void)
{
  KLedgerView::createRegister(new kMyMoneyRegisterLoan(this, "Loans"));
}

void KLedgerViewLoan::createAccountMenu(void)
{
  // get the basic entries for all ledger views
  KLedgerView::createAccountMenu();

  m_form->accountButton()->setPopup(m_accountMenu);
}

void KLedgerViewLoan::createMoreMenu(void)
{
  // get the basic entries for all ledger views
  KLedgerView::createMoreMenu();

  // and now the specific entries for checkings/savings etc.
  KIconLoader *kiconloader = KGlobal::iconLoader();

  m_moreMenu->insertItem(i18n("Edit splits ..."), this, SLOT(slotOpenSplitDialog()),
      QKeySequence(), -1, 1);
  m_moreMenu->insertItem(kiconloader->loadIcon("goto", KIcon::Small),
                            i18n("Goto payee/receiver"), this, SLOT(slotPayeeSelected()),
                            QKeySequence(), -1, 2);

  m_form->moreButton()->setPopup(m_moreMenu);
}

void KLedgerViewLoan::createContextMenu(void)
{
  // get the basic entries for all ledger views
  KLedgerView::createContextMenu();

  // and now the specific entries for checkings/savings etc.
  KIconLoader *kiconloader = KGlobal::iconLoader();

  m_contextMenu->insertItem(i18n("Edit splits ..."), this, SLOT(slotOpenSplitDialog()),
      QKeySequence(), -1, 2);
  m_contextMenu->insertItem(kiconloader->loadIcon("goto", KIcon::Small),
                            i18n("Goto payee/receiver"), this, SLOT(slotPayeeSelected()),
                            QKeySequence(), -1, 3);
}

void KLedgerViewLoan::createForm(void)
{
  // determine the height of the objects in the table
  kMyMoneyDateInput dateInput(0, "editDate");
  KPushButton splitButton(i18n("Split"), 0, "editSplit");
  kMyMoneyCategory category(0, "category");

  // extract the maximal sizeHint height and subtract 8
  int h = QMAX(dateInput.sizeHint().height(), splitButton.sizeHint().height());
  h = QMAX(h, category.sizeHint().height())-4;

  m_form = new kMyMoneyTransactionForm(this, 0, 0, 4, 5, h);

  // never show horizontal scroll bars
  m_form->table()->setHScrollBarMode(QScrollView::AlwaysOff);

  // adjust size of form table
  m_form->table()->setMaximumHeight(m_form->table()->rowHeight(0)*m_form->table()->numRows());
  m_form->table()->setMinimumHeight(m_form->table()->rowHeight(0)*m_form->table()->numRows());

  // connections
  connect(m_form->editButton(), SIGNAL(clicked()), this, SLOT(slotStartEdit()));
  connect(m_form->cancelButton(), SIGNAL(clicked()), this, SLOT(slotCancelEdit()));
  connect(m_form->enterButton(), SIGNAL(clicked()), this, SLOT(slotEndEdit()));
  connect(m_form->newButton(), SIGNAL(clicked()), this, SLOT(slotNew()));

  // for now, we don't allow to enter new transactions here.
  m_form->newButton()->hide();
}

void KLedgerViewLoan::createSummary(void)
{
  m_summaryLine = new KLedgerViewCheckingsSummaryLine(this, 0);
}

void KLedgerViewLoan::fillSummary(void)
{
  MyMoneyMoney balance;
  MyMoneyFile* file = MyMoneyFile::instance();
  KLedgerViewCheckingsSummaryLine* summary = dynamic_cast<KLedgerViewCheckingsSummaryLine*>(m_summaryLine);

  if(summary) {
    summary->clear();
    if(!accountId().isEmpty()) {
      try {
        balance = file->balance(accountId());
        QString txt = balance.formatMoney();
        if(m_account.accountType() == MyMoneyAccount::Loan)
          summary->setBalance(i18n("You currently owe: ") + (-balance).formatMoney(file->currency(m_account.currencyId()).tradingSymbol()));
        else
          summary->setBalance(i18n("Current balance: ") + balance.formatMoney(file->currency(m_account.currencyId()).tradingSymbol()));

        QDate date;
        if(!m_account.value("lastStatementDate").isEmpty())
          date = QDate::fromString(m_account.value("lastStatementDate"), Qt::ISODate);
        if(date.isValid())
          summary->setReconciliationDate(i18n("Reconciled: %1").arg(KGlobal::locale()->formatDate(date, true)));

      } catch(MyMoneyException *e) {
          qDebug("Unexpected exception in KLedgerViewLoan::fillSummary");
      }
    }
  }
}

void KLedgerViewLoan::fillFormStatics(void)
{
  QTable* formTable = m_form->table();

  // clear complete table
  for(int r = 0; r < formTable->numRows(); ++r) {
    for(int c = 0; c < formTable->numCols(); ++c) {
      if(formTable->cellWidget(r, c) == 0)
        formTable->setText(r, c, " ");
    }
  }

  // common elements
  formTable->setText(NR_ROW, NR_TXT_COL, i18n("No."));

  formTable->setText(DATE_ROW, DATE_TXT_COL, i18n("Date"));

  formTable->setText(CATEGORY_ROW, CATEGORY_TXT_COL, i18n("Category"));
  formTable->setText(AMORTIZATION_ROW, AMORTIZATION_TXT_COL, i18n("Amortization"));

  formTable->setText(MEMO_ROW, MEMO_TXT_COL, i18n("Memo"));
  formTable->setText(AMOUNT_ROW, AMOUNT_TXT_COL, i18n("Amount"));

  switch(m_account.accountGroup()) {
    case MyMoneyAccount::Asset:
      formTable->setText(PAYEE_ROW, PAYEE_TXT_COL, i18n("From"));
      break;
    case MyMoneyAccount::Liability:
      formTable->setText(PAYEE_ROW, PAYEE_TXT_COL, i18n("Pay to"));
      break;
    default:
      if(m_account.id().isEmpty()) {
        formTable->setText(PAYEE_ROW, PAYEE_TXT_COL, i18n("Pay to"));
      } else {
        qFatal("KLedgerViewLoan::fillFormStatics(): Got a loan with an expense/income account. Strange!");
      }
      break;
  }
}

void KLedgerViewLoan::fillForm(void)
{
  QTable* formTable = m_form->table();
  m_transactionPtr = transaction(m_register->currentTransactionIndex());

  if(m_transactionPtr != 0) {
    // get my local copy of the selected transaction
    m_transaction = *m_transactionPtr;
    m_split = m_transaction.splitByAccount(accountId());

    MyMoneyMoney amortization = m_split.value();
    MyMoneyMoney amount;

    kMyMoneyTransactionFormTableItem* item;

    // setup the fields first
    fillFormStatics();

    // fill in common fields
    formTable->setText(MEMO_ROW, MEMO_DATA_COL, m_split.memo());

    // make sure, that the date is aligned to the right of the cell
    item = new kMyMoneyTransactionFormTableItem(formTable, QTableItem::Never, KGlobal::locale()->formatDate(m_transaction.postDate(), true));
    item->setAlignment(kMyMoneyTransactionFormTableItem::right);
    formTable->setItem(DATE_ROW, DATE_DATA_COL, item);

    // collect values
    QString payee;
    try {
      payee = MyMoneyFile::instance()->payee(m_split.payeeId()).name();
    } catch (MyMoneyException *e) {
      delete e;
      payee = " ";
    }

    // then fill in the data
    // receiver
    formTable->setText(PAYEE_ROW, PAYEE_DATA_COL, payee);

    MyMoneySplit s = m_transaction.splitByAccount(accountId(), false);
    MyMoneyAccount acc = MyMoneyFile::instance()->account(s.accountId());
    QString category = acc.name();

    QValueList<MyMoneySplit>::ConstIterator it;
    for(it = m_transaction.splits().begin(); it != m_transaction.splits().end(); ++it) {
      try {
        if((*it).action() == MyMoneySplit::ActionAmortization
        && (*it).id() != m_split.id()) {
          acc = MyMoneyFile::instance()->account((*it).accountId());
          s = m_transaction.splitByAccount(acc.id());
          amount = s.value();
          category = i18n("Loan payment");
        }
      } catch(MyMoneyException *e) {
        delete e;
      }
    }
    if(m_account.accountType() == MyMoneyAccount::AssetLoan) {
      amortization = -amortization;
    } else if(m_account.accountType() == MyMoneyAccount::Loan) {
      amount = -amount;
    } else {
      qWarning("Internal error: wrong account type in KLedgerViewLoan::fillForm");
    }
    // category
    formTable->setText(CATEGORY_ROW, CATEGORY_DATA_COL, category);

    // number
    item = new kMyMoneyTransactionFormTableItem(formTable, QTableItem::Never, m_split.number());
    item->setAlignment(kMyMoneyTransactionFormTableItem::right);
    formTable->setItem(NR_ROW, NR_DATA_COL, item);

    // amortization
    item = new kMyMoneyTransactionFormTableItem(formTable, QTableItem::Never, amortization.formatMoney());
    item->setAlignment(kMyMoneyTransactionFormTableItem::right);
    formTable->setItem(AMORTIZATION_ROW, AMORTIZATION_DATA_COL, item);

    // payment amount
    item = new kMyMoneyTransactionFormTableItem(formTable, QTableItem::Never, amount.formatMoney());
    item->setAlignment(kMyMoneyTransactionFormTableItem::right);
    formTable->setItem(AMOUNT_ROW, AMOUNT_DATA_COL, item);

    m_form->newButton()->setEnabled(true);
    m_form->editButton()->setEnabled(true);
    enableOkButton(false);
    enableCancelButton(false);
    enableMoreButton(true);
  } else {
    m_transaction = MyMoneyTransaction();
    m_split = MyMoneySplit();
    m_split.setAccountId(accountId());
    m_split.setAction(MyMoneySplit::ActionAmortization);

    m_transaction.addSplit(m_split);

    // transaction empty, clean out space
    for(int i = 0; i < formTable->numRows(); ++i) {
      formTable->setText(i, 1, "");
      formTable->setText(i, 4, "");
    }

    m_form->newButton()->setEnabled(true);
    m_form->editButton()->setEnabled(false);
    enableOkButton(false);
    enableCancelButton(false);
    enableMoreButton(false);
  }

  // make sure, fields can use all available space
  // by spanning items over multiple cells if necessary
  QTableItem* item;
  // payee
  item = formTable->item(PAYEE_ROW, PAYEE_DATA_COL);
  if(item)
    item->setSpan(PAYEE_DATA_COL, 2);
  // category
  item = formTable->item(CATEGORY_ROW, CATEGORY_DATA_COL);
  if(item)
    item->setSpan(CATEGORY_DATA_COL, 2);
  // memo
  item = formTable->item(MEMO_ROW, MEMO_DATA_COL);
  if(item)
    item->setSpan(MEMO_DATA_COL, 2);
}

void KLedgerViewLoan::createEditWidgets(void)
{
  if(!m_editPayee) {
    m_editPayee = new kMyMoneyPayee(0, "editPayee");
    connect(m_editPayee, SIGNAL(newPayee(const QString&)), this, SLOT(slotNewPayee(const QString&)));
    connect(m_editPayee, SIGNAL(payeeChanged(const QString&)), this, SLOT(slotPayeeChanged(const QString&)));
  }

  if(!m_editCategory) {
    m_editCategory = new kMyMoneyCategory(0, "editCategory");
    connect(m_editCategory, SIGNAL(categoryChanged(const QCString&)), this, SLOT(slotCategoryChanged(const QCString&)));
  }
  if(!m_editMemo) {
    m_editMemo = new kMyMoneyLineEdit(0, "editMemo", AlignLeft|AlignVCenter);
    connect(m_editMemo, SIGNAL(lineChanged(const QString&)), this, SLOT(slotMemoChanged(const QString&)));
  }

  if(!m_editAmount) {
    m_editAmount = new kMyMoneyEdit(0, "editAmount");
    connect(m_editAmount, SIGNAL(valueChanged(const QString&)), this, SLOT(slotAmountChanged(const QString&)));
  }

  if(!m_editDate) {
    m_editDate = new kMyMoneyDateInput(0, "editDate");
    m_editDate->setFocusPolicy(QWidget::StrongFocus);
    connect(m_editDate, SIGNAL(dateChanged(const QDate&)), this, SLOT(slotDateChanged(const QDate&)));
  }

  if(!m_editNr) {
    m_editNr = new kMyMoneyLineEdit(0, "editNr");
    connect(m_editNr, SIGNAL(lineChanged(const QString&)), this, SLOT(slotNrChanged(const QString&)));
  }

  if(!m_editSplit) {
    m_editSplit = new KPushButton(i18n("Split"), 0, "editSplit");
    connect(m_editSplit, SIGNAL(clicked()), this, SLOT(slotOpenSplitDialog()));
  }
}

void KLedgerViewLoan::reloadEditWidgets(const MyMoneyTransaction& t)
{
  MyMoneyMoney amount;
  QString payee;

  m_transaction = t;
  m_split = m_transaction.splitByAccount(accountId());
  amount = m_split.value();

  if(m_editCategory)
    disconnect(m_editCategory, SIGNAL(signalFocusIn()), this, SLOT(slotOpenSplitDialog()));

  try {
    if(!m_split.payeeId().isEmpty())
      payee = MyMoneyFile::instance()->payee(m_split.payeeId()).name();

    MyMoneySplit s;
    MyMoneyAccount acc;
    switch(t.splitCount()) {
      case 2:
        s = t.splitByAccount(accountId(), false);
        acc = MyMoneyFile::instance()->account(s.accountId());

        // if the second account is also an asset or liability account
        // then this is a loan payment transfer type transaction and handled
        // a little different
        switch(MyMoneyFile::instance()->accountGroup(acc.accountType())) {
          case MyMoneyAccount::Expense:
          case MyMoneyAccount::Income:
          case MyMoneyAccount::Equity:
            if(m_editCategory)
              m_editCategory->loadAccount(s.accountId());
            break;

          case MyMoneyAccount::Asset:
          case MyMoneyAccount::Liability:
            if(m_split.action() != MyMoneySplit::ActionAmortization) {
              m_split.setAction(MyMoneySplit::ActionAmortization);
              m_transaction.modifySplit(m_split);
            }
            if(s.action() != MyMoneySplit::ActionAmortization) {
              s.setAction(MyMoneySplit::ActionAmortization);
              m_transaction.modifySplit(s);
            }
/*
            if(m_split.value() > 0) {
              m_editFrom->loadText(MyMoneyFile::instance()->accountToCategory(s.accountId()));
              m_editTo->loadText(MyMoneyFile::instance()->accountToCategory(m_account.id()));
            } else {
              m_editTo->loadText(MyMoneyFile::instance()->accountToCategory(s.accountId()));
              m_editFrom->loadText(MyMoneyFile::instance()->accountToCategory(m_account.id()));
              amount = -amount;       // make it positive
            }
*/
            connect(m_editCategory, SIGNAL(signalFocusIn()), this, SLOT(slotOpenSplitDialog()));
            if(m_editCategory)
              m_editCategory->loadText(i18n("Loan payment"));
            break;

          default:
            qDebug("Unknown account group in KLedgerViewLoan::reloadEditWidgets()");
            break;
        }
        break;

      case 1:
        if(m_editCategory)
          m_editCategory->loadText("");
        break;

      default:
        connect(m_editCategory, SIGNAL(signalFocusIn()), this, SLOT(slotOpenSplitDialog()));
        if(m_editCategory)
          m_editCategory->loadText(i18n("Loan payment"));
    }
  } catch(MyMoneyException *e) {
    qDebug("Exception '%s' thrown in %s, line %ld caught in KLedgerViewLoan::reloadEditWidgets():%d",
      e->what().latin1(), e->file().latin1(), e->line(), __LINE__);
    delete e;
  }

  // for almost all transaction types we have to negate the value
  // exceptions are: deposits and transfers (which are always positive)
  if(transactionDirection(m_split) != Credit)
    amount = -amount;
  if(m_split.action() == MyMoneySplit::ActionAmortization && amount.isNegative()) {
    amount = -amount;
  }

  if(m_editPayee)
    m_editPayee->loadText(payee);
  if(m_editMemo)
    m_editMemo->loadText(m_split.memo());
  if(m_editAmount)
    m_editAmount->loadText(amount.formatMoney());
  if(m_editDate && m_transactionPtr)
    m_editDate->loadDate(m_transactionPtr->postDate());
  if(m_editNr)
    m_editNr->loadText(m_split.number());
}

void KLedgerViewLoan::loadEditWidgets(void)
{
  if(m_transactionPtr != 0) {
    reloadEditWidgets(*m_transactionPtr);
  } else {
    m_editDate->setDate(m_lastPostDate);
/*
    try {
      if(m_editNr != 0) {
        // if the CopyTypeToNr switch is set, we copy the m_action string
        // into the nr (No.) field of this transaction.
        KConfig *config = KGlobal::config();
        config->setGroup("General Options");
        if(config->readBoolEntry("CopyTypeToNr", false) == true) {
          m_split.setNumber(m_action);
          m_transaction.modifySplit(m_split);
          m_editNr->loadText(m_split.number());
        }
      }

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
      qDebug("Exception '%s' thrown in %s, line %ld caught in KLedgerViewLoan::showWidgets()",
        e->what().latin1(), e->file().latin1(), e->line());
      delete e;
    }
*/
  }
}

QWidget* KLedgerViewLoan::arrangeEditWidgetsInForm(void)
{
  // make sure, that the category has an associated button
  QTableItem* item;
  kMyMoneyTransactionFormTable* table = m_form->table();

  // category
  item = table->item(CATEGORY_ROW, CATEGORY_DATA_COL);
  item->setSpan(CATEGORY_DATA_COL, 1);

  // make sure, we're using the right palette
  QPalette palette = m_register->palette();
  m_editPayee->setPalette(palette);
  m_editCategory->setPalette(palette);
  m_editMemo->setPalette(palette);
  m_editAmount->setPalette(palette);
  m_editDate->setPalette(palette);
  m_editNr->setPalette(palette);

  // hide amount field during edit phase
  table->setText(AMOUNT_ROW, AMOUNT_TXT_COL, " ");
  table->setText(AMOUNT_ROW, AMOUNT_DATA_COL, " ");

  table->clearEditable();
  setFormCellWidget(MEMO_ROW, MEMO_DATA_COL, m_editMemo);
  setFormCellWidget(DATE_ROW, DATE_DATA_COL, m_editDate);
  setFormCellWidget(AMORTIZATION_ROW, AMORTIZATION_DATA_COL, m_editAmount);
  setFormCellWidget(NR_ROW, NR_DATA_COL, m_editNr);
  setFormCellWidget(CATEGORY_ROW, CATEGORY_DATA_COL, m_editCategory);
  setFormCellWidget(CATEGORY_ROW, CATEGORY_DATA_COL+1, m_editSplit);
  setFormCellWidget(PAYEE_ROW, PAYEE_DATA_COL, m_editPayee);

  table->setEditable(PAYEE_ROW, PAYEE_DATA_COL);
  table->setEditable(CATEGORY_ROW, CATEGORY_DATA_COL);
  table->setEditable(MEMO_ROW, MEMO_DATA_COL);
  table->setEditable(DATE_ROW, DATE_DATA_COL);
  table->setEditable(AMORTIZATION_ROW, AMORTIZATION_DATA_COL);
  table->setEditable(NR_ROW, NR_DATA_COL);

  // now setup the tab order
  m_tabOrderWidgets.clear();
  addToTabOrder(m_form->enterButton());
  addToTabOrder(m_form->cancelButton());
  addToTabOrder(m_form->moreButton());
  addToTabOrder(m_editPayee);
  addToTabOrder(m_editCategory);
  addToTabOrder(m_editSplit);
  addToTabOrder(m_editMemo);
  addToTabOrder(m_editNr);
  addToTabOrder(m_editDate);
  addToTabOrder(m_editAmount);

  return m_editPayee;
}

QWidget* KLedgerViewLoan::arrangeEditWidgetsInRegister(void)
{
  int   firstRow = m_register->currentTransactionIndex() * m_register->rpt();

  // place edit widgets in the register
  setRegisterCellWidget(firstRow, 0, m_editDate);
  setRegisterCellWidget(firstRow, 1, m_editNr);
  setRegisterCellWidget(firstRow, 2, m_editPayee);
  setRegisterCellWidget(firstRow+1, 2, m_editCategory);
  setRegisterCellWidget(firstRow+2, 2, m_editMemo);
  setRegisterCellWidget(firstRow, 4, m_editAmount);

  // place buttons
  setRegisterCellWidget(firstRow+2, 0, m_registerButtonFrame);

  // now setup the tab order
  m_tabOrderWidgets.clear();
  addToTabOrder(m_editDate);
  addToTabOrder(m_editNr);
  addToTabOrder(m_editPayee);
  addToTabOrder(m_editCategory);
  addToTabOrder(m_editMemo);
  addToTabOrder(m_editAmount);
  addToTabOrder(m_registerEnterButton);
  addToTabOrder(m_registerCancelButton);
  addToTabOrder(m_registerMoreButton);

  if(m_editSplit) {
    delete m_editSplit;
    m_editSplit = 0;
  }
  return m_editDate;
}

void KLedgerViewLoan::destroyWidgets(void)
{
  KLedgerView::destroyWidgets();

  m_form->table()->clearEditable();
  m_form->tabBar()->setEnabled(true);

  // make sure, category can use all available space (incl. split button)
  QTableItem* item;
  item = m_form->table()->item(CATEGORY_ROW, CATEGORY_DATA_COL);
  item->setSpan(CATEGORY_DATA_COL, 2);
}

bool KLedgerViewLoan::focusNextPrevChild(bool next)
{
  return KLedgerView::focusNextPrevChild(next);
}

void KLedgerViewLoan::slotRegisterClicked(int row, int col, int button, const QPoint &mousePos)
{
  if(!m_account.id().isEmpty()) {
    KLedgerView::slotRegisterClicked(row, col, button, mousePos);
  }
}

void KLedgerViewLoan::slotConfigureMoreMenu(void)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  int splitEditId = m_moreMenu->idAt(1);
  int gotoPayeeId = m_moreMenu->idAt(2);
  int markAsId = m_moreMenu->idAt(4);
  int moveToId = m_moreMenu->idAt(5);

  // we need to disconnect the slots, otherwise we cannot connect unconditionally
  m_moreMenu->disconnectItem(splitEditId, this, SLOT(slotGotoOtherSideOfTransfer()));
  m_moreMenu->disconnectItem(splitEditId, this, SLOT(slotOpenSplitDialog()));

  m_moreMenu->changeItem(splitEditId, i18n("Edit splits ..."));
  m_moreMenu->connectItem(splitEditId, this, SLOT(slotOpenSplitDialog()));
  if(m_transactionPtr != 0) {
    if(!m_split.payeeId().isEmpty() && !m_split.accountId().isEmpty() && !m_transaction.id().isEmpty()) {
      MyMoneyPayee payee = file->payee(m_split.payeeId());
      m_moreMenu->changeItem(gotoPayeeId, i18n("Goto '%1'").arg(payee.name()));
      m_moreMenu->setItemEnabled(gotoPayeeId, true);
    } else {
      m_moreMenu->changeItem(gotoPayeeId, i18n("Goto payee/receiver"));
      m_moreMenu->setItemEnabled(gotoPayeeId, false);
    }

    if(KMyMoneyUtils::transactionType(*m_transactionPtr) == KMyMoneyUtils::Transfer) {
      QString dest;
      try {
        MyMoneySplit split = m_transaction.splitByAccount(m_account.id(), false);
        MyMoneyAccount acc = file->account(split.accountId());
        dest = acc.name();
      } catch(MyMoneyException *e) {
        delete e;
        dest = "opposite account";
      }
      m_moreMenu->changeItem(splitEditId, i18n("Goto '%1'").arg(dest));
      m_moreMenu->connectItem(splitEditId, this, SLOT(slotGotoOtherSideOfTransfer()));
      m_moreMenu->disconnectItem(splitEditId, this, SLOT(slotOpenSplitDialog()));
    }
    m_moreMenu->setItemEnabled(splitEditId, true);
  } else {
    m_moreMenu->setItemEnabled(splitEditId, false);
  }

  m_moreMenu->setItemEnabled(markAsId, false);
  m_moreMenu->setItemEnabled(moveToId, false);
}

void KLedgerViewLoan::slotConfigureContextMenu(void)
{
  int splitEditId = m_contextMenu->idAt(2);
  int gotoPayeeId = m_contextMenu->idAt(3);
  int markAsId = m_contextMenu->idAt(5);
  int moveToId = m_contextMenu->idAt(6);
  int deleteId = m_contextMenu->idAt(8);
  MyMoneyFile* file = MyMoneyFile::instance();

  // we need to disconnect the slots, otherwise we cannot connect unconditionally
  m_contextMenu->disconnectItem(splitEditId, this, SLOT(slotGotoOtherSideOfTransfer()));
  m_contextMenu->disconnectItem(splitEditId, this, SLOT(slotOpenSplitDialog()));

  m_contextMenu->changeItem(splitEditId, i18n("Edit splits ..."));
  m_contextMenu->connectItem(splitEditId, this, SLOT(slotOpenSplitDialog()));
  if(m_transactionPtr != 0) {
    if(!m_split.payeeId().isEmpty() && !m_split.accountId().isEmpty() && !m_transaction.id().isEmpty()) {
      MyMoneyPayee payee = file->payee(m_split.payeeId());
      QString name = payee.name();
      name.replace(QRegExp("&(?!&)"), "&&");
      m_contextMenu->changeItem(gotoPayeeId, i18n("Goto '%1'").arg(name));
      m_contextMenu->setItemEnabled(gotoPayeeId, true);
    } else {
      m_contextMenu->changeItem(gotoPayeeId, i18n("Goto payee/receiver"));
      m_contextMenu->setItemEnabled(gotoPayeeId, false);
    }
    if(KMyMoneyUtils::transactionType(*m_transactionPtr) == KMyMoneyUtils::Transfer) {
      QString dest;
      try {
        MyMoneySplit split = m_transaction.splitByAccount(m_account.id(), false);
        MyMoneyAccount acc = MyMoneyFile::instance()->account(split.accountId());
        dest = acc.name();
      } catch(MyMoneyException *e) {
        delete e;
        dest = i18n("opposite account");
      }
      m_contextMenu->changeItem(splitEditId, i18n("Goto '%1'").arg(dest));
      m_contextMenu->connectItem(splitEditId, this, SLOT(slotGotoOtherSideOfTransfer()));
      m_contextMenu->disconnectItem(splitEditId, this, SLOT(slotOpenSplitDialog()));
    }
    m_contextMenu->setItemEnabled(splitEditId, true);
    m_contextMenu->setItemEnabled(deleteId, true);
  } else {
    m_contextMenu->setItemEnabled(splitEditId, false);
    m_contextMenu->setItemEnabled(deleteId, false);
  }

  m_contextMenu->setItemEnabled(markAsId, false);
  m_contextMenu->setItemEnabled(moveToId, false);
}

void KLedgerViewLoan::slotOpenSplitDialog(void)
{
  // if we're not in edit mode, then start editing
  if(!isEditMode())
    slotStartEdit();

  // but the user may have decided not to edit, so we bail out
  if(!isEditMode())
    return;

  bool isDeposit = false;
  bool isValidAmount = false;

  if(!m_split.value().isNegative()) {
    isDeposit = true;
  }
#if 0
    isDeposit = transactionType(m_transaction) == Credit;
#endif
  isValidAmount = m_editAmount->text().length() != 0;

  KSplitTransactionDlg* dlg = new KSplitTransactionDlg(m_transaction,
                                                       m_account,
                                                       isValidAmount,
                                                       isDeposit,
                                                       0,
                                                       this);

  if(dlg->exec()) {
    reloadEditWidgets(dlg->transaction());
  }

  delete dlg;

  m_editMemo->setFocus();
}

void KLedgerViewLoan::slotAccountDetail(void)
{
  slotLoanAccountDetail();
/*
  KNewAccountDlg dlg(m_account, true, false, this, "hi", i18n("Edit an Account"));

  if (dlg.exec()) {
    try {
      MyMoneyFile::instance()->modifyAccount(dlg.account());
    } catch (MyMoneyException *e) {
      qDebug("Unexpected exception in KLedgerViewLoan::slotAccountDetail");
      delete e;
    }
  }
*/
}

void KLedgerViewLoan::slotPayeeSelected(void)
{
  slotCancelEdit();
  if(!m_split.payeeId().isEmpty() && !m_split.accountId().isEmpty() && !m_transaction.id().isEmpty())
    emit payeeSelected(m_split.payeeId(), m_split.accountId(), m_transaction.id());
}

void KLedgerViewLoan::slotLoanAccountDetail(void)
{
  KEditLoanWizard* wizard = new KEditLoanWizard(m_account);
  if(wizard->exec() == QDialog::Accepted) {
    MyMoneyFile* file = MyMoneyFile::instance();
    MyMoneySchedule sch = file->schedule(m_account.value("schedule").latin1());
    if(!(m_account == wizard->account())
    || !(sch == wizard->schedule())) {
      try {
        file->modifyAccount(wizard->account());
        sch = wizard->schedule();
        try {
          file->schedule(sch.id());
          file->modifySchedule(sch);
        } catch (MyMoneyException *e) {
          try {
            file->addSchedule(sch);
          } catch (MyMoneyException *f) {
            qDebug("Cannot add schedule: '%s'", f->what().data());
            delete f;
          }
          delete e;
        }
      } catch(MyMoneyException *e) {
        qDebug("Unable to modify account %s: '%s'", m_account.name().data(),
            e->what().data());
        delete e;
      }
    }
  }
  delete wizard;
}

void KLedgerViewLoan::slotReconciliation(void)
{
  slotCancelEdit();

  KEndingBalanceLoanDlg dlg(m_account);

  if(dlg.exec()) {
    MyMoneyTransaction t = dlg.adjustmentTransaction();

    if(t != MyMoneyTransaction()) {
      try {
        MyMoneyFile::instance()->addTransaction(t);
      } catch(MyMoneyException *e) {
        qWarning("adjustment transaction not stored: '%s'", e->what().data());
        delete e;
      }
    }

    // suppress any modifications on the gui
    MyMoneyFile::instance()->suspendNotify(true);

    // now go through all transactions and mark them reconciled for
    // this account.
    MyMoneyTransactionFilter filter(m_account.id());
    filter.setDateFilter(dlg.startDate(), dlg.endDate());

    QValueList<MyMoneyTransaction> list = MyMoneyFile::instance()->transactionList(filter);
    QValueList<MyMoneyTransaction>::Iterator it_t;

    for(it_t = list.begin(); it_t != list.end(); ++it_t) {
      MyMoneySplit sp = (*it_t).splitByAccount(m_account.id());
      sp.setReconcileFlag(MyMoneySplit::Reconciled);
      sp.setReconcileDate(QDate::currentDate());

      try {
        (*it_t).modifySplit(sp);
        MyMoneyFile::instance()->modifyTransaction(*it_t);
      } catch(MyMoneyException *e) {
        qDebug("Unable to reconcile split: %s", e->what().data());
        delete e;
      }
    }

    // remember until when this account is reconciled
    m_account.setValue("lastStatementDate", dlg.endDate().toString(Qt::ISODate));
    try {
      MyMoneyFile::instance()->modifyAccount(m_account);
    } catch(MyMoneyException *e) {
      qDebug("Unable to update setting for 'lastStatementDate': %s", e->what().data());
      delete e;
    }

    // re-enable modifications on the gui and force an immediate update
    MyMoneyFile::instance()->suspendNotify(false);
  }
}

int KLedgerViewLoan::currentActionTab(void) const
{
  return -1;
}

void KLedgerViewLoan::updateTabBar(const MyMoneyTransaction& /* t */, const MyMoneySplit& /* s */, const bool /* enableAll */)
{
}

bool KLedgerViewLoan::eventFilter( QObject *o, QEvent *e )
{
  return KLedgerView::eventFilter(o, e);
}

// Make sure, that these definitions are only used within this file
// this does not seem to be necessary, but when building RPMs the
// build option 'final' is used and all CPP files are concatenated.
// So it could well be, that in another CPP file these definitions
// are also used.
#undef PAYEE_ROW
#undef CATEGORY_ROW
#undef MEMO_ROW
#undef NR_ROW
#undef DATE_ROW
#undef AMORTIZATION_ROW
#undef AMOUNT_ROW

#undef PAYEE_TXT_COL
#undef PAYEE_DATA_COL
#undef CATEGORY_TXT_COL
#undef CATEGORY_DATA_COL
#undef MEMO_TXT_COL
#undef MEMO_DATA_COL
#undef NR_TXT_COL
#undef NR_DATA_COL
#undef DATE_TXT_COL
#undef DATE_DATA_COL
#undef AMORTIZATION_TXT_COL
#undef AMORTIZATION_DATA_COL
#undef AMOUNT_TXT_COL
#undef AMOUNT_DATA_COL

#include "kledgerviewloan.moc"
