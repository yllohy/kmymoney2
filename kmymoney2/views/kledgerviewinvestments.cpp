/***************************************************************************
                          kledgerviewinvestments.cpp  -  description
                             -------------------
    begin                : Sun Mar 7 2004
    copyright            : (C) 2000-2004 by Michael Edwardes
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

#include <qwidgetstack.h>
/*
#include <qpushbutton.h>
#include <qtabwidget.h>
#include <qlistview.h>
#include <qlayout.h>
#include <qwidget.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qfile.h>
#include <qtextstream.h>
*/

// ----------------------------------------------------------------------------
// KDE Includes

#include <kpushbutton.h>
#include <kiconloader.h>
#include <kguiitem.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kledgerview.h"
#include "kledgerviewcheckings.h"
#include "kledgerviewinvestments.h"

#include "../mymoney/mymoneyfile.h"

#include "../dialogs/knewaccountdlg.h"
#include "../dialogs/knewequityentrydlg.h"

#include "../widgets/kmymoneyregistercheckings.h"
#include "../widgets/kmymoneytransactionform.h"


#define SYMBOL_ROW        0
#define QUANTITY_ROW      1
#define MEMO_ROW          2
#define PRICE_ROW         3
#define DATE_ROW          0
#define AMOUNT_ROW        1
#define FEES_ROW          2

#define SYMBOL_TXT_COL    0
#define SYMBOL_DATA_COL   (SYMBOL_TXT_COL+1)
#define QUANTITY_TXT_COL  0
#define QUANTITY_DATA_COL (QUANTITY_TXT_COL+1)
#define MEMO_TXT_COL      0
#define MEMO_DATA_COL     (MEMO_TXT_COL+1)
#define PRICE_TXT_COL     0
#define PRICE_DATA_COL    (PRICE_TXT_COL+1)
#define DATE_TXT_COL      3
#define DATE_DATA_COL     (DATE_TXT_COL+1)
#define AMOUNT_TXT_COL    3
#define AMOUNT_DATA_COL   (AMOUNT_TXT_COL+1)
#define FEES_TXT_COL      3
#define FEES_DATA_COL     (FEES_TXT_COL+1)

KLedgerViewInvestments::KLedgerViewInvestments(QWidget *parent, const char *name) : KLedgerView(parent, name)
{
  QGridLayout* formLayout = new QGridLayout( this, 1, 2, 11, 6, "InvestmentFormLayout");
  QVBoxLayout* ledgerLayout = new QVBoxLayout(this, 5, 6, "InvestmentLedgerLayout");

  createInfoStack();
  formLayout->addWidget(m_infoStack, 0, 1 );

  createRegister();
  ledgerLayout->addWidget(m_register, 3);

  createSummary();
  ledgerLayout->addLayout(m_summaryLayout);

  createForm();
  ledgerLayout->addWidget(m_form);

  formLayout->addLayout( ledgerLayout, 0, 0);

  m_editPPS = 0;
  m_editShares = 0;
  m_editSymbolName = 0;
  m_editTotalAmount = 0;
  m_editFees = 0;
}

KLedgerViewInvestments::~KLedgerViewInvestments()
{

}

void KLedgerViewInvestments::fillForm()
{
  fillFormStatics();
}

void KLedgerViewInvestments::fillFormStatics(void)
{
  QTable* formTable = m_form->table();

  // clear complete table, but leave the cell widgets while editing
  for(int r = 0; r < formTable->numRows(); ++r) {
    for(int c = 0; c < formTable->numCols(); ++c) {
      if(formTable->cellWidget(r, c) == 0)
        formTable->setText(r, c, " ");
    }
  }

  // common elements
  formTable->setText(SYMBOL_ROW, SYMBOL_TXT_COL, i18n("Symbol Name"));
  formTable->setText(QUANTITY_ROW, QUANTITY_TXT_COL, i18n("Shares"));
  formTable->setText(MEMO_ROW, MEMO_TXT_COL, i18n("Memo"));
  formTable->setText(PRICE_ROW, PRICE_TXT_COL, i18n("Price Per Share"));
  formTable->setText(DATE_ROW, DATE_TXT_COL, i18n("Date"));
  formTable->setText(AMOUNT_ROW, AMOUNT_TXT_COL, i18n("Total Amount"));
  formTable->setText(FEES_ROW, FEES_TXT_COL, i18n("Commission"));

  switch(transactionType(m_transaction, m_split)) {
    case Transfer:
#if 0
      switch( transactionDirection(m_split) ){
        case Credit:
        case UnknownDirection:
          // formTable->setText(PAYEE_ROW, PAYEE_TXT_COL, i18n("Payer"));
          //formTable->setText(CATEGORY_ROW, CATEGORY_TXT_COL, i18n("From"));
          break;
        case Debit:
          formTable->setText(PAYEE_ROW, PAYEE_TXT_COL, i18n("Receiver"));
          formTable->setText(CATEGORY_ROW, CATEGORY_TXT_COL, i18n("To"));
          break;
      }
#endif
      break;

    default:
#if 0
      formTable->setText(CATEGORY_ROW, CATEGORY_TXT_COL, i18n("Category"));
      switch( transactionDirection(m_split) ){
        case Credit:
        case UnknownDirection:
          formTable->setText(PAYEE_ROW, PAYEE_TXT_COL, i18n("Payer"));
          break;
        case Debit:
          formTable->setText(PAYEE_ROW, PAYEE_TXT_COL, i18n("Receiver"));
          break;
      }
#endif
      break;
  }

#if 0
  if(showNrField(m_transaction, m_split))
    formTable->setText(NR_ROW, NR_TXT_COL, i18n("Nr"));
#endif
}

void KLedgerViewInvestments::fillSummary()
{

}

void KLedgerViewInvestments::showWidgets()
{
#if 0
  // the code found in KLedgerViewCheckings
  QWidget* focusWidget;

  createEditWidgets();
  loadEditWidgets();

  if(m_transactionFormActive) {
    focusWidget = arrangeEditWidgetsInForm();
  } else {
    focusWidget = arrangeEditWidgetsInRegister();
  }

  // make sure, size of all form columns are correct
  resizeEvent(0);

  m_tabOrderWidgets.find(focusWidget);
  focusWidget->setFocus();
#endif




  createEditWidgets();

  kMyMoneyTransactionFormTable* table = m_form->table();

  if(table)
  {
    table->setCellWidget(MEMO_ROW, MEMO_DATA_COL, m_editMemo);
    table->setCellWidget(DATE_ROW, DATE_DATA_COL, m_editDate);
    table->setCellWidget(PRICE_ROW, PRICE_DATA_COL, m_editPPS);
    table->setCellWidget(SYMBOL_ROW, SYMBOL_DATA_COL, m_editSymbolName);
    table->setCellWidget(QUANTITY_ROW, QUANTITY_DATA_COL, m_editShares);
    table->setCellWidget(AMOUNT_ROW, AMOUNT_DATA_COL, m_editTotalAmount);
    table->setCellWidget(FEES_ROW, FEES_DATA_COL, m_editFees);
    
    table->setEditable(MEMO_ROW, MEMO_DATA_COL);
    table->setEditable(DATE_ROW, DATE_DATA_COL);
    table->setEditable(PRICE_ROW, PRICE_DATA_COL);
    table->setEditable(SYMBOL_ROW, SYMBOL_DATA_COL);
    table->setEditable(QUANTITY_ROW, QUANTITY_DATA_COL);
    table->setEditable(AMOUNT_ROW, AMOUNT_DATA_COL, false);
    table->setEditable(FEES_ROW, FEES_DATA_COL);
  }
  
  
  
  // now setup the tab order
  m_tabOrderWidgets.clear();
  m_tabOrderWidgets.append(m_form->enterButton());
  m_tabOrderWidgets.append(m_form->cancelButton());
  m_tabOrderWidgets.append(m_form->moreButton());
  m_tabOrderWidgets.append(m_editSymbolName);
  m_tabOrderWidgets.append(m_editDate);
  m_tabOrderWidgets.append(m_editShares);
  m_tabOrderWidgets.append(m_editTotalAmount);
  m_tabOrderWidgets.append(m_editMemo);
  m_tabOrderWidgets.append(m_editFees);
  m_tabOrderWidgets.append(m_editPPS);
}

void KLedgerViewInvestments::hideWidgets()
{
  for(int i=0; i < m_form->table()->numRows(); ++i) {
    m_form->table()->clearCellWidget(i, 1);
    m_form->table()->clearCellWidget(i, 2);
    m_form->table()->clearCellWidget(i, 4);
  }

  int   firstRow = m_register->currentTransactionIndex() * m_register->rpt();
  for(int i = 0; i < m_register->maxRpt(); ++i) {
    for(int j = 0; j < m_register->numCols(); ++j) {
      m_register->clearCellWidget(firstRow+i, j);
    }
  }

  m_editPayee = 0;
  m_editCategory = 0;
  m_editMemo = 0;
  m_editAmount = 0;
  m_editNr = 0;
  m_editDate = 0;
  m_editSplit = 0;
  m_editType = 0;
  m_editPayment = 0;
  m_editDeposit = 0;

  m_editPPS = 0;
  m_editShares = 0;
  m_editSymbolName = 0;
  m_editTotalAmount = 0;
  m_editFees = 0;

  m_form->table()->clearEditable();
  m_form->tabBar()->setEnabled(true);
}

void KLedgerViewInvestments::reloadEditWidgets(const MyMoneyTransaction& t)
{

}

void KLedgerViewInvestments::slotReconciliation(void)
{

}

void KLedgerViewInvestments::slotNew()
{
  KLedgerView::slotNew();


}

void KLedgerViewInvestments::createEditWidgets()
{
  if(!m_editPayee) {
    m_editPayee = new kMyMoneyPayee(0, "editPayee");
  }
  if(!m_editMemo) {
    m_editMemo = new kMyMoneyLineEdit(0, "editMemo", AlignLeft|AlignVCenter);
  }
  if(!m_editAmount) {
    m_editAmount = new kMyMoneyEdit(0, "editAmount");
  }
  if(!m_editDate) {
    m_editDate = new kMyMoneyDateInput(0, "editDate");
  }
  if(!m_editShares) {
    m_editShares = new kMyMoneyEdit(0, "editShares");
  }
  if(!m_editPPS) {
    m_editPPS = new kMyMoneyEdit(0, "editPPS");
  }
  if(!m_editSymbolName) {
    m_editSymbolName = new kMyMoneyLineEdit(0, "editSymbolName", AlignLeft|AlignVCenter);
  }
  if(!m_editTotalAmount) {
    m_editTotalAmount = new kMyMoneyEdit(0, "editTotalAmount");
  }
  if(!m_editFees) {
    m_editFees = new kMyMoneyEdit(0, "editFees");
  }

  if(!m_editType) {
    m_editType = new kMyMoneyCombo(0, "editType");
    m_editType->setFocusPolicy(QWidget::StrongFocus);
  }
}

void KLedgerViewInvestments::createForm(void)
{
  // determine the height of the objects in the table
  kMyMoneyDateInput dateInput(0, "editDate");
  KPushButton splitButton(i18n("Split"), 0, "editSplit");
  kMyMoneyCategory category(0, "category");

  // extract the maximal sizeHint height and subtract 8
  int h = QMAX(dateInput.sizeHint().height(), splitButton.sizeHint().height());
  h = QMAX(h, category.sizeHint().height())-8;

  m_form = new kMyMoneyTransactionForm(this, NULL, 0, 4, 5, h);

  m_tabAddShares = new QTab(action2str(MyMoneySplit::ActionAddShares, true));
  m_tabRemoveShares = new QTab(action2str(MyMoneySplit::ActionRemoveShares, true));
  //m_tabTransfer = new QTab(action2str(MyMoneySplit::ActionTransfer, true));
  //m_tabWithdrawal = new QTab(action2str(MyMoneySplit::ActionWithdrawal, true));
  //m_tabAtm = new QTab(action2str(MyMoneySplit::ActionATM, true));

  m_form->addTab(m_tabAddShares);
  m_form->addTab(m_tabRemoveShares);
  //m_form->addTab(m_tabTransfer);
  //m_form->addTab(m_tabWithdrawal);
  //m_form->addTab(m_tabAtm);


  // never show horizontal scroll bars
  m_form->table()->setHScrollBarMode(QScrollView::AlwaysOff);

  // adjust size of form table
  m_form->table()->setMaximumHeight(m_form->table()->rowHeight(0)*m_form->table()->numRows());
  m_form->table()->setMinimumHeight(m_form->table()->rowHeight(0)*m_form->table()->numRows());

  // connections
  connect(m_form->tabBar(), SIGNAL(selected(int)), this, SLOT(slotTypeSelected(int)));

  connect(m_form->editButton(), SIGNAL(clicked()), this, SLOT(slotStartEdit()));
  connect(m_form->cancelButton(), SIGNAL(clicked()), this, SLOT(slotCancelEdit()));
  connect(m_form->enterButton(), SIGNAL(clicked()), this, SLOT(slotEndEdit()));
  connect(m_form->newButton(), SIGNAL(clicked()), this, SLOT(slotNew()));

  m_form->enterButton()->setDefault(true);

  // slotTypeSelected(KLedgerViewInvestments::AddShares);
}

void KLedgerViewInvestments::createInfoStack(void)
{
  // create the widget stack first
  KLedgerView::createInfoStack();

  // First page buttons inside a frame with layout
  QFrame* frame = new QFrame(m_infoStack, "ButtonFrame");

  frame->setSizePolicy( QSizePolicy( QSizePolicy::MinimumExpanding,
                                     QSizePolicy::Minimum,
                                     0, 0,
                                     frame->sizePolicy().hasHeightForWidth() ) );

  QVBoxLayout* buttonLayout = new QVBoxLayout( frame, 0, 6, "ButtonLayout");

  KIconLoader* il = KGlobal::iconLoader();

  m_detailsButton = new KPushButton(frame, "detailsButton" );
  KGuiItem detailsButtenItem( i18n("&Account Details"),
                    QIconSet(il->loadIcon("viewmag", KIcon::Small, KIcon::SizeSmall)),
                    i18n("Modify the loan details for this loan"),
                    i18n("Use this to start a wizard that allows changing the details for this loan."));
  m_detailsButton->setGuiItem(detailsButtenItem);
  buttonLayout->addWidget(m_detailsButton);

  m_reconcileButton = new KPushButton(frame, "reconcileButton");
  KGuiItem reconcileButtenItem( i18n("&Reconcile ..."),
                    QIconSet(il->loadIcon("reconcile", KIcon::Small, KIcon::SizeSmall)),
                    i18n("Start the account reconciliation"),
                    i18n("Use this to reconcile your account against the bank statement."));
  m_reconcileButton->setGuiItem(reconcileButtenItem);
  buttonLayout->addWidget(m_reconcileButton);

  m_lastReconciledLabel = new QLabel("", frame);
  buttonLayout->addWidget(m_lastReconciledLabel);

/*
  // FIXME: This should not be required anymore as this
  //        this type of stuff is handled in the KEditLoanWizard
  m_interestButton = new KPushButton(frame, "interestButton");
  KGuiItem interestButtonItem( i18n("&Modify interest..."),
                    QIconSet(il->loadIcon("edit", KIcon::Small, KIcon::SizeSmall)),
                    i18n("Modify the interest rate for this loan"),
                    i18n("Use this to start a wizard that allows changing the interest rate."));
  m_interestButton->setGuiItem(interestButtonItem);
  buttonLayout->addWidget(m_interestButton);

  m_loanDetailsButton = new KPushButton(frame, "loanDetailsButton");
  KGuiItem loanDetailsButtonItem( i18n("&Modify loan details..."),
                    QIconSet(il->loadIcon("configure", KIcon::Small, KIcon::SizeSmall)),
                    i18n("Modify the loan details for this loan"),
                    i18n("Use this to start a wizard that allows changing the details for this loan."));
  m_loanDetailsButton->setGuiItem(loanDetailsButtonItem);
  buttonLayout->addWidget(m_loanDetailsButton);

  connect(m_reconcileButton, SIGNAL(clicked()), this, SLOT(slotReconciliation()));
  connect(m_detailsButton, SIGNAL(clicked()), this, SLOT(slotLoanAccountDetail()));
  // connect(m_loanDetailsButton, SIGNAL(clicked()), this, SLOT(slotLoanAccountDetail()));

  // FIXME: add functionality to modify interest rate etc.
  // For now just show the proposed functionality
  m_interestButton->setEnabled(false);
  m_loanDetailsButton->setEnabled(false);
*/
  connect(m_detailsButton, SIGNAL(clicked()), this, SLOT(slotAccountDetail()));
  connect(m_reconcileButton, SIGNAL(clicked()), this, SLOT(slotReconciliation()));

  QSpacerItem* spacer = new QSpacerItem( 20, 20,
                   QSizePolicy::Minimum, QSizePolicy::Expanding );
  buttonLayout->addItem( spacer );

  m_infoStack->addWidget(frame, KLedgerView::TransactionEdit);

  // Initially show the page with the buttons
  m_infoStack->raiseWidget(KLedgerView::TransactionEdit);
}

void KLedgerViewInvestments::slotTypeSelected(int type)
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
  formTable->setText(0, 0, i18n("Symbol Name"));
  formTable->setText(1, 0, i18n("Quantity"));
  formTable->setText(2, 0, i18n("Memo"));
  formTable->setText(3, 0, i18n("Price Per Share"));
  formTable->setText(0, 3, i18n("Date"));
  formTable->setText(1, 3, i18n("Amount"));
  formTable->setText(2, 3, i18n("Fees"));

  m_action = transactionType(type);

  // specific elements (in the order of the tabs)
  switch(type) {
    case KLedgerViewInvestments::AddShares:
      //formTable->setText(1, 0, i18n("Symbol Name"));
      //formTable->setText(2, 0, i18n("Category"));
      //formTable->setText(0, 3, i18n("Nr"));
      break;

    case KLedgerViewInvestments::RemoveShares:
      formTable->setText(1, 0, i18n("Payee"));
      formTable->setText(2, 0, i18n("Category"));
      break;

    /*case 2:   // Transfer
      formTable->setText(0, 0, i18n("From"));
      formTable->setText(1, 0, i18n("To"));
      formTable->setText(2, 0, i18n("Payee"));
      break;

    case 3:   // Withdrawal
      formTable->setText(1, 0, i18n("Receiver"));
      formTable->setText(2, 0, i18n("Category"));
      break;

    case 4:   // ATM
      formTable->setText(0, 3, i18n("Nr"));
      formTable->setText(1, 0, i18n("Receiver"));
      formTable->setText(2, 0, i18n("Category"));
      break;     */
  }

  KConfig *config = KGlobal::config();
  config->setGroup("General Options");
  if(config->readBoolEntry("AlwaysShowNrField", false) == true)
    formTable->setText(0, 3, i18n("Nr"));

  if(!m_form->tabBar()->signalsBlocked())
    slotNew();
}

void KLedgerViewInvestments::slotRegisterDoubleClicked(int /* row */,
                                                int /* col */,
                                                int /* button */,
                                                const QPoint & /* mousePos */)
{
  if(m_transactionPtr != 0)
    slotStartEdit();
}

void KLedgerViewInvestments::createRegister(void)
{
  m_register = new kMyMoneyRegisterCheckings(this, "Investments");
  m_register->setParent(this);

  m_register->setAction(QCString(MyMoneySplit::ActionAddShares), i18n("Add Shares"));
  m_register->setAction(QCString(MyMoneySplit::ActionRemoveShares), i18n("Remove Shares"));
  //m_register->setAction(QCString(MyMoneySplit::ActionDeposit), i18n("Deposit"));
  //m_register->setAction(QCString(MyMoneySplit::ActionWithdrawal), i18n("Withdrawal"));
  //m_register->setAction(QCString(MyMoneySplit::ActionTransfer), i18n("Transfer"));

  connect(m_register, SIGNAL(clicked(int, int, int, const QPoint&)), this, SLOT(slotRegisterClicked(int, int, int, const QPoint&)));
  connect(m_register, SIGNAL(doubleClicked(int, int, int, const QPoint&)), this, SLOT(slotRegisterDoubleClicked(int, int, int, const QPoint&)));

  connect(m_register, SIGNAL(signalEnter()), this, SLOT(slotStartEdit()));
  connect(m_register, SIGNAL(signalNextTransaction()), this, SLOT(slotNextTransaction()));
  connect(m_register, SIGNAL(signalPreviousTransaction()), this, SLOT(slotPreviousTransaction()));
  connect(m_register, SIGNAL(signalDelete()), this, SLOT(slotDeleteTransaction()));
  connect(m_register, SIGNAL(signalSelectTransaction(const QCString&)), this, SLOT(selectTransaction(const QCString&)));

  connect(m_register->horizontalHeader(), SIGNAL(clicked(int)), this, SLOT(slotRegisterHeaderClicked(int)));
}

void KLedgerViewInvestments::createSummary(void)
{
  m_summaryLayout = new QHBoxLayout(6, "SummaryLayout");

  QSpacerItem* spacer = new QSpacerItem( 20, 1, QSizePolicy::Expanding, QSizePolicy::Minimum );
  m_summaryLayout->addItem(spacer);

  m_summaryLine = new QLabel(this);

  m_summaryLayout->addWidget(m_summaryLine);
}

void KLedgerViewInvestments::slotAccountDetail(void)
{
  KNewAccountDlg dlg(m_account, true, false, this, "hi", i18n("Edit an Account"));

  if (dlg.exec()) {
    try {
      MyMoneyFile::instance()->modifyAccount(dlg.account());
    } catch (MyMoneyException *e) {
      qDebug("Unexpected exception in KLedgerViewCheckings::slotAccountDetail");
      delete e;
    }
  }
}

int KLedgerViewInvestments::transactionType(const MyMoneyTransaction& t, const MyMoneySplit& split) const
{
  qDebug("logic of KLedgerViewInvestments::transactionType should go to KLedgerView::transactionType");
  if(split.action() == MyMoneySplit::ActionAddShares)
    return KLedgerViewInvestments::AddShares;
  else if(split.action() == MyMoneySplit::ActionRemoveShares)
    return KLedgerViewInvestments::RemoveShares;
  else
    return KLedgerView::transactionType(t);

  //qDebug("Unknown transaction type in KLedgerView::transactionType, Check assumed");
  //return Check;
}

const QCString KLedgerViewInvestments::transactionType(int type) const
{
  switch(type) {
    default:
      qWarning("Unknown transaction type used in KLedgerView::transactionType(int)");
      // Tricky fall through here!

    case KLedgerViewInvestments::AddShares: // Check
      return MyMoneySplit::ActionAddShares;

    case KLedgerViewInvestments::RemoveShares: // Deposit
      return MyMoneySplit::ActionRemoveShares;
  }

  return KLedgerView::transactionType(type);
}

void KLedgerViewInvestments::updateTabBar(const MyMoneyTransaction& /* t */, const MyMoneySplit& /* s */)
{
}

void KLedgerViewInvestments::resizeEvent(QResizeEvent* /* ev */)
{
  // resize the register
  int w = m_register->visibleWidth();

  // check which space we need
  m_register->adjustColumn(0);
  m_register->adjustColumn(4);
  m_register->adjustColumn(5);
  m_register->adjustColumn(6);

  // make amount columns all the same size
  int width = m_register->columnWidth(4);
  if(width < m_register->columnWidth(5))
    width = m_register->columnWidth(5);
  if(width < m_register->columnWidth(6))
    width = m_register->columnWidth(6);

  m_register->setColumnWidth(4, width);
  m_register->setColumnWidth(5, width);
  m_register->setColumnWidth(6, width);

  // Resize the date field to either
  // a) the size required by the input widget if no transaction form is shown
  // b) the adjusted value for the date if the transaction form is visible
  if(!m_transactionFormActive) {
    kMyMoneyDateInput* datefield = new kMyMoneyDateInput();
    datefield->setFont(m_register->cellFont());
    m_register->setColumnWidth(1, datefield->minimumSizeHint().width());
    delete datefield;
  } else {
    m_register->adjustColumn(1);
  }

  m_register->setColumnWidth(3, 20);

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

void KLedgerViewInvestments::slotStartEdit()
{

}

void KLedgerViewInvestments::slotEndEdit()
{
  QString name = m_editSymbolName->text();
  qDebug("Symbol name is %s", name.data());
  
  KNewEquityEntryDlg *pDlg = new KNewEquityEntryDlg(this, name.data());
  pDlg->setSymbolName(name);
  if(pDlg->exec())
  {
  
  }
}