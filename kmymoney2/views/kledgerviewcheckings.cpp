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

#include "kledgerviewcheckings.h"
#include "../widgets/kmymoneytransactionform.h"
#include "../widgets/kmymoneyedit.h"
#include "../widgets/kmymoneydateinput.h"
#include "../widgets/kmymoneypayee.h"
#include "../widgets/kmymoneycategory.h"
#include "../widgets/kmymoneylineedit.h"
#include "../widgets/kmymoneycombo.h"
#include "../widgets/kmymoneyregistercheckings.h"
#include "../dialogs/kendingbalancedlg.h"
#include "../dialogs/ksplittransactiondlg.h"
#include "../dialogs/knewaccountdlg.h"
#include "../mymoney/mymoneyfile.h"
#include "../views/kmymoneyview.h"

KLedgerViewCheckings::KLedgerViewCheckings(QWidget *parent, const char *name )
  : KLedgerView(parent,name)
{
  QGridLayout* formLayout = new QGridLayout( this, 1, 2, 11, 6, "FormLayout");
  QVBoxLayout* ledgerLayout = new QVBoxLayout( 6, "LedgerLayout");

  createInfoStack();
  formLayout->addWidget(m_infoStack, 0, 1 );

  createRegister();
  ledgerLayout->addWidget(m_register, 3);

  createSummary();
  ledgerLayout->addLayout(m_summaryLayout);

  createForm();
  // make sure, transfers are disabled if required
  m_tabTransfer->setEnabled(transfersPossible());
  ledgerLayout->addWidget(m_form);

  formLayout->addLayout( ledgerLayout, 0, 0);

  // create the context menu that is accessible via RMB 
  createContextMenu();

  // create the context menu that is accessible via the MORE Button
  createMoreMenu();

  connect(m_contextMenu, SIGNAL(aboutToShow()), SLOT(slotConfigureContextMenu()));
  connect(m_moreMenu, SIGNAL(aboutToShow()), SLOT(slotConfigureMoreMenu()));

  // load the form with inital settings. Always consider transaction type Deposit
  m_form->tabBar()->blockSignals(true);
  slotTypeSelected(1);
  m_form->tabBar()->blockSignals(false);

  // setup the form to be visible or not
  slotShowTransactionForm(m_transactionFormActive);

  // and the register has the focus
  m_register->setFocus();
}

KLedgerViewCheckings::~KLedgerViewCheckings()
{
  delete m_register;
}

void KLedgerViewCheckings::refreshView(void)
{
  if(m_inReconciliation)
    KLedgerView::refreshView(m_transactionCheckBox->isChecked());
  else
    KLedgerView::refreshView();

  QDate date;
  if(!m_account.value("lastStatementDate").isEmpty())
    date = QDate::fromString(m_account.value("lastStatementDate"), Qt::ISODate);
    
  if(date.isValid())  
    m_lastReconciledLabel->setText(i18n("Reconciled: %1").arg(KGlobal::locale()->formatDate(date, true)));
  else
    m_lastReconciledLabel->setText(QString());

  m_tabTransfer->setEnabled(transfersPossible());
}

void KLedgerViewCheckings::resizeEvent(QResizeEvent* /* ev */)
{
  
  // resize the register
  int w = m_register->visibleWidth();

  m_register->adjustColumn(0);
  m_register->adjustColumn(3);
  m_register->adjustColumn(4);
  m_register->adjustColumn(5);

  int width = m_register->columnWidth(3);
  if(width < m_register->columnWidth(4))
    width = m_register->columnWidth(4);
  if(width < m_register->columnWidth(5))
    width = m_register->columnWidth(5);

  m_register->setColumnWidth(4, width);
  m_register->setColumnWidth(5, width);
  m_register->setColumnWidth(6, width);
/*
  int m_debitWidth = 80;
  int m_creditWidth = 80;
  int m_balanceWidth = 100;
  m_register->setColumnWidth(0, 80);
*/

  // Resize the date field to the size required by the input widget
  kMyMoneyDateInput* datefield = new kMyMoneyDateInput();
  datefield->setFont(m_register->cellFont());
  m_register->setColumnWidth(1, datefield->minimumSizeHint().width());
  delete datefield;

  m_register->setColumnWidth(3, 20);
/*  
  m_register->setColumnWidth(4, m_debitWidth);
  m_register->setColumnWidth(5, m_creditWidth);
  m_register->setColumnWidth(6, m_balanceWidth);
*/

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
  table->setColumnWidth(3, 80);
  table->setColumnWidth(2, 40);
  table->setColumnWidth(4, 120);

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

void KLedgerViewCheckings::enableWidgets(const bool enable)
{
  m_detailsButton->setEnabled(enable);
  m_reconcileButton->setEnabled(enable);
  KLedgerView::enableWidgets(enable);
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

  m_action = transactionType(type);

  // specific elements (in the order of the tabs)
  switch(type) {
    case 0:   // Check
      formTable->setText(1, 0, i18n("Receiver"));
      formTable->setText(2, 0, i18n("Category"));
      formTable->setText(0, 3, i18n("Nr"));
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
      formTable->setText(0, 3, i18n("Nr"));
      formTable->setText(1, 0, i18n("Receiver"));
      formTable->setText(2, 0, i18n("Category"));
      break;
  }

  KConfig *config = KGlobal::config();
  config->setGroup("General Options");
  if(config->readBoolEntry("AlwaysShowNrField", false) == true)
    formTable->setText(0, 3, i18n("Nr"));

  if(!m_form->tabBar()->signalsBlocked())
    slotNew();
}

void KLedgerViewCheckings::slotRegisterDoubleClicked(int /* row */,
                                                     int /* col */,
                                                     int /* button */,
                                                     const QPoint & /* mousePos */)
{
  slotStartEdit();
}

void KLedgerViewCheckings::createRegister(void)
{
  m_register = new kMyMoneyRegisterCheckings(this, "Checkings");
  m_register->setParent(this);

  m_register->setAction(QCString(MyMoneySplit::ActionATM), i18n("ATM"));
  m_register->setAction(QCString(MyMoneySplit::ActionCheck), i18n("Cheque"));
  m_register->setAction(QCString(MyMoneySplit::ActionDeposit), i18n("Deposit"));
  m_register->setAction(QCString(MyMoneySplit::ActionWithdrawal), i18n("Withdrawal"));
  m_register->setAction(QCString(MyMoneySplit::ActionTransfer), i18n("Transfer"));
    
  connect(m_register, SIGNAL(clicked(int, int, int, const QPoint&)), this, SLOT(slotRegisterClicked(int, int, int, const QPoint&)));
  connect(m_register, SIGNAL(doubleClicked(int, int, int, const QPoint&)), this, SLOT(slotRegisterDoubleClicked(int, int, int, const QPoint&)));

  connect(m_register, SIGNAL(signalEnter()), this, SLOT(slotStartEdit()));
  connect(m_register, SIGNAL(signalNextTransaction()), this, SLOT(slotNextTransaction()));
  connect(m_register, SIGNAL(signalPreviousTransaction()), this, SLOT(slotPreviousTransaction()));
  connect(m_register, SIGNAL(signalDelete()), this, SLOT(slotDeleteTransaction()));
  connect(m_register, SIGNAL(signalSelectTransaction(const QCString&)), this, SLOT(selectTransaction(const QCString&)));
    
  connect(m_register->horizontalHeader(), SIGNAL(clicked(int)), this, SLOT(slotRegisterHeaderClicked(int)));
}

void KLedgerViewCheckings::createMoreMenu(void)
{
  // get the basic entries for all ledger views
  KLedgerView::createMoreMenu();

  // and now the specific entries for checkings/savings etc.
  KIconLoader *kiconloader = KGlobal::iconLoader();
  m_moreMenu->insertItem(i18n("Edit splits ..."), this, SLOT(slotStartEditSplit()),
      QKeySequence(), -1, 1);
  m_moreMenu->insertItem(kiconloader->loadIcon("goto", KIcon::Small),
                         i18n("Goto payee/receiver"), this, SLOT(slotPayeeSelected()),
                         QKeySequence(), -1, 2);
  m_moreMenu->insertItem(kiconloader->loadIcon("bookmark_add", KIcon::Small),
                         i18n("Create schedule..."), this, SLOT(slotCreateSchedule()),
                         QKeySequence(), -1, 3);

  m_form->moreButton()->setPopup(m_moreMenu);
}

void KLedgerViewCheckings::createContextMenu(void)
{
  // get the basic entries for all ledger views
  KLedgerView::createContextMenu();

  // and now the specific entries for checkings/savings etc.
  KIconLoader *kiconloader = KGlobal::iconLoader();
  
  m_contextMenu->insertItem(i18n("Edit splits ..."), this, SLOT(slotStartEditSplit()),
      QKeySequence(), -1, 2);
  m_contextMenu->insertItem(kiconloader->loadIcon("goto", KIcon::Small),
                            i18n("Goto payee/receiver"), this, SLOT(slotPayeeSelected()),
                            QKeySequence(), -1, 3);
  m_contextMenu->insertItem(kiconloader->loadIcon("bookmark_add", KIcon::Small),
                            i18n("Create schedule..."), this, SLOT(slotCreateSchedule()),
                            QKeySequence(), -1, 4);
}

void KLedgerViewCheckings::createForm(void)
{
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

  // never show horizontal scroll bars
  m_form->table()->setHScrollBarMode(QScrollView::AlwaysOff);

  // adjust size of form table
  m_form->table()->setMaximumHeight(m_form->table()->rowHeight(0)*m_form->table()->numRows());

  // connections
  connect(m_form->tabBar(), SIGNAL(selected(int)), this, SLOT(slotTypeSelected(int)));

  connect(m_form->editButton(), SIGNAL(clicked()), this, SLOT(slotStartEdit()));
  connect(m_form->cancelButton(), SIGNAL(clicked()), this, SLOT(slotCancelEdit()));
  connect(m_form->enterButton(), SIGNAL(clicked()), this, SLOT(slotEndEdit()));
  connect(m_form->newButton(), SIGNAL(clicked()), this, SLOT(slotNew()));

  m_form->enterButton()->setDefault(true);
}

void KLedgerViewCheckings::createSummary(void)
{
  m_summaryLayout = new QHBoxLayout(6, "SummaryLayout");

  QSpacerItem* spacer = new QSpacerItem( 20, 1, QSizePolicy::Expanding, QSizePolicy::Minimum );
  m_summaryLayout->addItem(spacer);

  m_summaryLine = new QLabel(this);

  m_summaryLayout->addWidget(m_summaryLine);
}

void KLedgerViewCheckings::createInfoStack(void)
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
                    i18n("Open the account dialog"),
                    i18n("Use this view and modify the account details."));
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
  
  connect(m_reconcileButton, SIGNAL(clicked()), this, SLOT(slotReconciliation()));
  connect(m_detailsButton, SIGNAL(clicked()), this, SLOT(slotAccountDetail()));

  QSpacerItem* spacer = new QSpacerItem( 20, 20,
                   QSizePolicy::Minimum, QSizePolicy::Expanding );
  buttonLayout->addItem( spacer );

  m_infoStack->addWidget(frame, KLedgerView::TransactionEdit);



  // Second page reconciliation info
  frame = new QFrame(m_infoStack, "ReconcileFrame");

  QVBoxLayout* reconcileLayout = new QVBoxLayout( frame, 0, 6, "ReconcileLayout");

  frame->setSizePolicy( QSizePolicy( QSizePolicy::MinimumExpanding,
                                     QSizePolicy::Minimum,
                                     0, 0,
                                     frame->sizePolicy().hasHeightForWidth() ) );

  QFrame* innerFrame = new QFrame(frame, "InnerFrame");

  innerFrame->setSizePolicy( QSizePolicy( QSizePolicy::MinimumExpanding,
                                     QSizePolicy::Minimum,
                                     0, 0,
                                     frame->sizePolicy().hasHeightForWidth() ) );

  innerFrame->setFrameShadow( QFrame::Plain );
  innerFrame->setFrameShape( QFrame::Panel );
  innerFrame->setMaximumWidth(125);
  innerFrame->setLineWidth(1);

  QVBoxLayout* innerLayout = new QVBoxLayout( innerFrame, 6, 0, "InnerLayout");

  QLabel* txt;
  txt = new QLabel(i18n("<center><b>Reconcile account</b></center><hr>\n"
                        "<b>1.</b> Click on column 'C' "
                        "to clear the transactions "
                        "appearing on your bank statement."), innerFrame);

  
  QFont defaultFont = KMyMoneyUtils::cellFont();
  txt->setFont(defaultFont);
  innerLayout->addWidget(txt);

  txt = new QLabel(i18n("<b>2.</b> Match the cleared "
                        "transactions with the amount "
                        "noted on your bank statement.<br>"), innerFrame);
  txt->setFont(defaultFont);
  innerLayout->addWidget(txt);


  QHBoxLayout* boxLayout = new QHBoxLayout(innerLayout, 0, "ClearedLayout");

  txt = new QLabel(i18n("Cleared:"), innerFrame);
  txt->setFont(defaultFont);
  boxLayout->addWidget(txt);

  spacer = new QSpacerItem( 1, 1,
                   QSizePolicy::Expanding, QSizePolicy::Minimum );
  boxLayout->addItem( spacer );

  m_clearedLabel = new QLabel("", innerFrame);
  m_clearedLabel->setFont(defaultFont);
  m_clearedLabel->setAlignment(Right);
  boxLayout->addWidget(m_clearedLabel);

  boxLayout = new QHBoxLayout(innerLayout, 0, "StatementLayout");

  txt = new QLabel(i18n("Statement:"), innerFrame);
  txt->setFont(defaultFont);
  boxLayout->addWidget(txt);

  spacer = new QSpacerItem( 1, 1,
                   QSizePolicy::Expanding, QSizePolicy::Minimum );
  boxLayout->addItem( spacer );

  m_statementLabel = new QLabel("", innerFrame);
  m_statementLabel->setFont(defaultFont);
  m_statementLabel->setAlignment(Right);
  boxLayout->addWidget(m_statementLabel);

  txt = new QLabel("<hr>", innerFrame);
  txt->setFont(defaultFont);
  txt->setAlignment(Top);
  txt->setMaximumHeight(10);
  innerLayout->addWidget(txt);


  boxLayout = new QHBoxLayout(innerLayout, 0, "DiffLayout");

  txt = new QLabel(i18n("Difference:"), innerFrame);
  txt->setFont(defaultFont);
  boxLayout->addWidget(txt);

  spacer = new QSpacerItem( 1, 1,
                   QSizePolicy::Expanding, QSizePolicy::Minimum );
  boxLayout->addItem( spacer );

  m_differenceLabel = new QLabel("", innerFrame);
  m_differenceLabel->setFont(defaultFont);
  m_differenceLabel->setAlignment(Right);
  boxLayout->addWidget(m_differenceLabel);


  txt = new QLabel(i18n("<p><b>3.</b> Hit the Finish "
                        "button when you're done."), innerFrame);
  txt->setFont(defaultFont);
  innerLayout->addWidget(txt);

  m_transactionCheckBox = new QCheckBox(i18n("Show\ntransactionform"), innerFrame, "showBox");
  m_transactionCheckBox->setFont(defaultFont);
  innerLayout->addWidget(m_transactionCheckBox);


  reconcileLayout->addWidget(innerFrame);


  KPushButton* m_finishButton = new KPushButton(frame, "FinishButton");
  m_finishButton->setText(i18n("&Finish"));
  reconcileLayout->addWidget(m_finishButton);

  connect(m_finishButton, SIGNAL(clicked()), this, SLOT(slotEndReconciliation()));

  KPushButton* m_postponeButton = new KPushButton(frame, "PostponeButton");
  m_postponeButton->setText(i18n("&Postpone"));
  reconcileLayout->addWidget(m_postponeButton);

  connect(m_postponeButton, SIGNAL(clicked()), this, SLOT(slotPostponeReconciliation()));

  spacer = new QSpacerItem( 1, 1,
                   QSizePolicy::Minimum, QSizePolicy::Expanding );
  reconcileLayout->addItem( spacer );

  m_infoStack->addWidget(frame, KLedgerView::Reconciliation);

  // Initially show the page with the buttons
  m_infoStack->raiseWidget(KLedgerView::TransactionEdit);
}

void KLedgerViewCheckings::fillSummary(void)
{
  MyMoneyMoney balance;
  QLabel *summary = static_cast<QLabel *> (m_summaryLine);

  if(accountId().length() > 0) {
    try {
      balance = MyMoneyFile::instance()->balance(accountId());
      QString txt = balance.formatMoney();
      if(balance < 0)
        txt = "<font color=\"red\"><b>" + txt + "</b></font>";
      txt = "<nobr>"+ i18n("Current balance: ") + txt + "</nobr>";

      summary->setText(txt);
    } catch(MyMoneyException *e) {
        qDebug("Unexpected exception in KLedgerViewCheckings::fillSummary");
    }
  } else
    summary->setText("");
}

void KLedgerViewCheckings::fillForm(void)
{
  QTable* formTable = m_form->table();
  m_transactionPtr = transaction(m_register->currentTransactionIndex());

  if(m_transactionPtr != 0) {
    // get my local copy of the selected transaction
    m_transaction = *m_transactionPtr;
    m_split = m_transaction.splitByAccount(accountId());

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
      MyMoneySplit s;
      switch(m_transaction.splitCount()) {
        case 2:
          if(m_transaction.isLoanPayment())
            category = i18n("Loan Payment");
          else {
            s = m_transaction.splitByAccount(accountId(), false);
            category = MyMoneyFile::instance()->accountToCategory(s.accountId());
          }
          break;
          
        case 1:
          category = " ";
          break;
          
        default:
          if(m_transaction.isLoanPayment())
            category = i18n("Loan Payment");
          else
            category = i18n("Splitted transaction");
          break;
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
    item = new kMyMoneyTransactionFormTableItem(formTable, QTableItem::Never, amount.formatMoney());
    item->setAlignment(kMyMoneyTransactionFormTableItem::right);
    formTable->setItem(2, 4, item);

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

void KLedgerViewCheckings::createEditWidgets(void)
{
  m_editPayee = new kMyMoneyPayee(0, "editPayee");
  m_editCategory = new kMyMoneyCategory(0, "editCategory");
  m_editMemo = new kMyMoneyLineEdit(0, "editMemo", AlignLeft|AlignVCenter);
  m_editAmount = new kMyMoneyEdit(0, "editAmount");
  m_editDate = new kMyMoneyDateInput(0, "editDate");
  m_editNr = new kMyMoneyLineEdit(0, "editNr");
  m_editFrom = new kMyMoneyCategory(0, "editFrom", static_cast<KMyMoneyUtils::categoryTypeE> (KMyMoneyUtils::asset | KMyMoneyUtils::liability));
  m_editTo = new kMyMoneyCategory(0, "editTo", static_cast<KMyMoneyUtils::categoryTypeE> (KMyMoneyUtils::asset | KMyMoneyUtils::liability));
  m_editSplit = new KPushButton("Split", 0, "editSplit");
  m_editPayment = new kMyMoneyEdit(0, "editPayment");
  m_editDeposit = new kMyMoneyEdit(0, "editDeposit");
  m_editType = new kMyMoneyCombo(0, "editType");
  m_editType->setFocusPolicy(QWidget::StrongFocus);

  connect(m_editSplit, SIGNAL(clicked()), this, SLOT(slotOpenSplitDialog()));

  connect(m_editPayee, SIGNAL(newPayee(const QString&)), this, SLOT(slotNewPayee(const QString&)));
  connect(m_editPayee, SIGNAL(payeeChanged(const QString&)), this, SLOT(slotPayeeChanged(const QString&)));
  connect(m_editMemo, SIGNAL(lineChanged(const QString&)), this, SLOT(slotMemoChanged(const QString&)));
  connect(m_editCategory, SIGNAL(categoryChanged(const QString&)), this, SLOT(slotCategoryChanged(const QString&)));
  connect(m_editAmount, SIGNAL(valueChanged(const QString&)), this, SLOT(slotAmountChanged(const QString&)));
  connect(m_editNr, SIGNAL(lineChanged(const QString&)), this, SLOT(slotNrChanged(const QString&)));
  connect(m_editDate, SIGNAL(dateChanged(const QDate&)), this, SLOT(slotDateChanged(const QDate&)));
  connect(m_editFrom, SIGNAL(categoryChanged(const QString&)), this, SLOT(slotFromChanged(const QString&)));
  connect(m_editTo, SIGNAL(categoryChanged(const QString&)), this, SLOT(slotToChanged(const QString&)));
  connect(m_editPayment, SIGNAL(valueChanged(const QString&)), this, SLOT(slotPaymentChanged(const QString&)));
  connect(m_editDeposit, SIGNAL(valueChanged(const QString&)), this, SLOT(slotDepositChanged(const QString&)));
  connect(m_editType, SIGNAL(selectionChanged(int)), this, SLOT(slotTypeChanged(int)));

  connect(m_editPayee, SIGNAL(signalEnter()), this, SLOT(slotEndEdit()));
  connect(m_editMemo, SIGNAL(signalEnter()), this, SLOT(slotEndEdit()));
  connect(m_editCategory, SIGNAL(signalEnter()), this, SLOT(slotEndEdit()));
  connect(m_editNr, SIGNAL(signalEnter()), this, SLOT(slotEndEdit()));
  connect(m_editDate, SIGNAL(signalEnter()), this, SLOT(slotEndEdit()));
  connect(m_editFrom, SIGNAL(signalEnter()), this, SLOT(slotEndEdit()));
  connect(m_editTo, SIGNAL(signalEnter()), this, SLOT(slotEndEdit()));
  connect(m_editAmount, SIGNAL(signalEnter()), this, SLOT(slotEndEdit()));
  connect(m_editPayment, SIGNAL(signalEnter()), this, SLOT(slotEndEdit()));
  connect(m_editDeposit, SIGNAL(signalEnter()), this, SLOT(slotEndEdit()));
  connect(m_editType, SIGNAL(signalEnter()), this, SLOT(slotEndEdit()));

  connect(m_editPayee, SIGNAL(signalEsc()), this, SLOT(slotCancelEdit()));
  connect(m_editMemo, SIGNAL(signalEsc()), this, SLOT(slotCancelEdit()));
  connect(m_editCategory, SIGNAL(signalEsc()), this, SLOT(slotCancelEdit()));
  connect(m_editNr, SIGNAL(signalEsc()), this, SLOT(slotCancelEdit()));
  connect(m_editDate, SIGNAL(signalEsc()), this, SLOT(slotCancelEdit()));
  connect(m_editFrom, SIGNAL(signalEsc()), this, SLOT(slotCancelEdit()));
  connect(m_editTo, SIGNAL(signalEsc()), this, SLOT(slotCancelEdit()));
  connect(m_editAmount, SIGNAL(signalEsc()), this, SLOT(slotCancelEdit()));
  connect(m_editPayment, SIGNAL(signalEsc()), this, SLOT(slotCancelEdit()));
  connect(m_editDeposit, SIGNAL(signalEsc()), this, SLOT(slotCancelEdit()));
  connect(m_editType, SIGNAL(signalEsc()), this, SLOT(slotCancelEdit()));
}

void KLedgerViewCheckings::reloadEditWidgets(const MyMoneyTransaction& t)
{
  MyMoneyMoney amount;
  QString category, payee;

  m_transaction = t;
  m_split = m_transaction.splitByAccount(accountId());
  amount = m_split.value();

  if(m_editCategory != 0)
    disconnect(m_editCategory, SIGNAL(signalFocusIn()), this, SLOT(slotOpenSplitDialog()));

  if(m_editType)
    m_editType->loadCurrentItem(m_form->tabBar()->indexOf(transactionType(m_split)));

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
        // then this is a transfer type transaction and handled a little different
        switch(MyMoneyFile::instance()->accountGroup(acc.accountType())) {
          case MyMoneyAccount::Expense:
          case MyMoneyAccount::Income:
            category = MyMoneyFile::instance()->accountToCategory(s.accountId());
            m_editCategory->loadList(static_cast<KMyMoneyUtils::categoryTypeE> (KMyMoneyUtils::income | KMyMoneyUtils::expense));
            break;

          case MyMoneyAccount::Asset:
          case MyMoneyAccount::Liability:
            if(!m_transaction.isLoanPayment()) {
              if(m_editCategory)
                m_editCategory->loadList(static_cast<KMyMoneyUtils::categoryTypeE> (KMyMoneyUtils::asset | KMyMoneyUtils::liability));
              if(m_split.action() != MyMoneySplit::ActionTransfer) {
                m_split.setAction(MyMoneySplit::ActionTransfer);
                m_transaction.modifySplit(m_split);
              }
              if(s.action() != MyMoneySplit::ActionTransfer) {
                s.setAction(MyMoneySplit::ActionTransfer);
                m_transaction.modifySplit(s);
              }
              if(m_split.value() > 0) {
                category = MyMoneyFile::instance()->accountToCategory(m_account.id());
                if(m_editFrom)
                  m_editFrom->loadText(MyMoneyFile::instance()->accountToCategory(s.accountId()));
                if(m_editTo)
                  m_editTo->loadText(category);
                  
              } else {
                category = MyMoneyFile::instance()->accountToCategory(s.accountId());
                if(m_editTo)
                  m_editTo->loadText(category);
                if(m_editFrom)
                  m_editFrom->loadText(MyMoneyFile::instance()->accountToCategory(m_account.id()));
                amount = -amount;       // make it positive
              }
              
            } else {
              // Make sure that we do not allow the user to modify the category
              // directly, but only through the split edit dialog
              connect(m_editCategory, SIGNAL(signalFocusIn()), this, SLOT(slotOpenSplitDialog()));
              category = i18n("Loan payment");
            }
            break;

          default:
            qDebug("Unknown account group in KLedgerCheckingsView::showWidgets()");
            break;
        }
        break;

      case 1:
        category = QString();
        break;

      default:
        // Make sure that we do not allow the user to modify the category
        // directly, but only through the split edit dialog
        connect(m_editCategory, SIGNAL(signalFocusIn()), this, SLOT(slotOpenSplitDialog()));
        if(t.isLoanPayment())
          category = i18n("Loan payment");
        else
          category = i18n("Splitted transaction");
    }
  } catch(MyMoneyException *e) {
    qDebug("Exception '%s' thrown in %s, line %ld caught in KLedgerViewCheckings::reloadEditWidgets():%d",
      e->what().latin1(), e->file().latin1(), e->line(), __LINE__-2);
    delete e;
  }

  // for almost all transaction types we have to negate the value
  // exceptions are: deposits and transfers (which are always positive)
  if(transactionType(m_split) != Deposit)
    amount = -amount;
  if(m_split.action() == MyMoneySplit::ActionTransfer && amount < 0) {
    amount = -amount;
  }

  if(m_editPayee != 0)
    m_editPayee->loadText(payee);
  if(m_editCategory != 0)
    m_editCategory->loadText(category);
  if(m_editMemo != 0)
    m_editMemo->loadText(m_split.memo());
  if(m_editAmount != 0)
    m_editAmount->loadText(amount.formatMoney());
  if(m_editDate != 0 && m_transactionPtr)
    m_editDate->loadDate(m_transactionPtr->postDate());
  if(m_editNr != 0)
    m_editNr->loadText(m_split.number());

  if(m_editPayment != 0 && m_editDeposit != 0) {
    if(m_split.value() < 0) {
      m_editPayment->loadText((-m_split.value()).formatMoney());
      m_editDeposit->loadText(QString());
    } else {
      m_editPayment->loadText(QString());
      m_editDeposit->loadText((m_split.value()).formatMoney());
    }
  }
}

void KLedgerViewCheckings::loadEditWidgets(int& transType)
{
  // need to load the combo box with the required values?
  if(m_editType) {
    if(m_editType->count() == 0) {
      // copy them from the tab's in the transaction form
      for(int i = 0; i < m_form->tabBar()->count(); ++i) {
        QString txt = m_form->tabBar()->tabAt(i)->text();
        txt = txt.replace(QRegExp("&"), "");
        m_editType->insertItem(txt);
      }
    }
  }
  if(m_transactionPtr != 0) {
    reloadEditWidgets(*m_transactionPtr);
    transType = transactionType(m_split);
  } else {
    m_editDate->setDate(m_lastPostDate);
    transType = m_form->tabBar()->currentTab();

    try {
      if(m_editNr != 0) {
        // if the CopyTypeToNr switch is set, we copy the m_action string
        // into the nr field of this transaction.
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
      qDebug("Exception '%s' thrown in %s, line %ld caught in KLedgerViewCheckings::showWidgets()",
        e->what().latin1(), e->file().latin1(), e->line());
      delete e;
    }
  }
}

void KLedgerViewCheckings::arrangeEditWidgetsInForm(QWidget*& focusWidget, const int transType)
{
  // span items over multiple cells if necessary
  QTableItem* item;
  // from account
  item = m_form->table()->item(0,1);
  item->setSpan(1, 2);
  // to account, payee
  item = m_form->table()->item(1,1);
  item->setSpan(1, 2);
  // category
  item = m_form->table()->item(2,1);
  item->setSpan(1, 1);
  // memo
  item = m_form->table()->item(3,1);
  item->setSpan(1, 2);

  // make sure, we're using the right palette
  QPalette palette = m_register->palette();
  m_editPayee->setPalette(palette);
  m_editCategory->setPalette(palette);
  m_editMemo->setPalette(palette);
  m_editAmount->setPalette(palette);
  m_editDate->setPalette(palette);
  if(m_editNr != 0)
    m_editNr->setPalette(palette);
  m_editFrom->setPalette(palette);
  m_editTo->setPalette(palette);

  // delete widgets that are used for the register edit mode only
  delete m_editPayment; m_editPayment = 0;
  delete m_editDeposit; m_editDeposit = 0;
  delete m_editType; m_editType = 0;

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

  KConfig *config = KGlobal::config();
  config->setGroup("General Options");

  switch(transType) {
    case Check: // Check
    case ATM: // ATM
      m_form->table()->setEditable(0, 4);
      fromRow = toRow = -1;
      if(m_editNr == 0)
        nrRow = -1;
      break;

    case Deposit: // Deposit
    case Withdrawal: // Withdrawal
      fromRow = toRow = -1;
      if(config->readBoolEntry("AlwaysShowNrField", false) == false)
        nrRow = -1;
      break;

    case Transfer: // Transfer
      item = m_form->table()->item(2,1);
      item->setSpan(1, 2);

      payeeRow = 2;
      categoryRow = -1;
      if(config->readBoolEntry("AlwaysShowNrField", false) == false)
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

  if(m_editNr != 0)
    m_tabOrderWidgets.append(m_editNr);
  m_tabOrderWidgets.append(m_editDate->focusWidget());
  m_tabOrderWidgets.append(m_editAmount);
}

void KLedgerViewCheckings::arrangeEditWidgetsInRegister(QWidget*& focusWidget, const int /* transType */)
{
  delete m_editAmount; m_editAmount = 0;

  int   firstRow = m_register->currentTransactionIndex() * m_register->rpt();
  if(m_editNr != 0)
    m_register->setCellWidget(firstRow, 0, m_editNr);
  m_register->setCellWidget(firstRow, 1, m_editDate);
  m_register->setCellWidget(firstRow+1, 1, m_editType);
  m_register->setCellWidget(firstRow, 2, m_editPayee);
  m_register->setCellWidget(firstRow+1, 2, m_editCategory);
  m_register->setCellWidget(firstRow+2, 2, m_editMemo);
  m_register->setCellWidget(firstRow, 4, m_editPayment);
  m_register->setCellWidget(firstRow, 5, m_editDeposit);

  if(m_editNr != 0) {
    m_tabOrderWidgets.append(m_editNr);
    focusWidget = m_editNr;
  } else
    focusWidget = m_editDate;

  m_tabOrderWidgets.append(m_editDate->focusWidget());
  m_tabOrderWidgets.append(m_editType);
  m_tabOrderWidgets.append(m_editPayee);
  m_tabOrderWidgets.append(m_editCategory);
  m_tabOrderWidgets.append(m_editMemo);
  m_tabOrderWidgets.append(m_editPayment);
  m_tabOrderWidgets.append(m_editDeposit);

  if(m_editFrom) {
    delete m_editFrom;
    m_editFrom = 0;
  }
  if(m_editTo) {
    delete m_editTo;
    m_editTo = 0;
  }
  if(m_editSplit) {
    delete m_editSplit;
    m_editSplit = 0;
  }
}

void KLedgerViewCheckings::showWidgets(void)
{
  QWidget* focusWidget;
  int transType;

  // clear the tab order
  m_tabOrderWidgets.clear();

  createEditWidgets();
  loadEditWidgets(transType);

  focusWidget = m_editPayee;

  if(m_transactionFormActive) {
    arrangeEditWidgetsInForm(focusWidget, transType);
  } else {
    arrangeEditWidgetsInRegister(focusWidget, transType);
  }

  m_tabOrderWidgets.find(focusWidget);
  focusWidget->setFocus();
}

void KLedgerViewCheckings::hideWidgets(void)
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

void KLedgerViewCheckings::slotReconciliation(void)
{
  slotCancelEdit();

/*
  MyMoneyMoney lastReconciledBalance(m_account.value("lastReconciledBalance"));
  if(lastReconciledBalance == 0)
    lastReconciledBalance = MyMoneyMoney(m_account.value("lastStatementBalance"));

  MyMoneyMoney statementBalance(m_account.value("statementBalance"));
  
  QDate statementDate;
  if(!m_account.value("statementDate").isEmpty())
    statementDate = QDate::fromString(m_account.value("statementDate"), Qt::ISODate);
    
  if(!statementDate.isValid())
    statementDate = QDate::currentDate();
*/
  KEndingBalanceDlg dlg(m_account);

  if(dlg.exec()) {
    QCString transactionId;

    // check if the user requests us to create interest
    // or charge transactions.
    MyMoneyTransaction ti = dlg.interestTransaction();
    MyMoneyTransaction tc = dlg.chargeTransaction();
    if(ti != MyMoneyTransaction()) {
      try {
        MyMoneyFile::instance()->addTransaction(ti);
      } catch(MyMoneyException *e) {
        qWarning("interest transaction not stored: '%s'", e->what().data());
        delete e;
      }
    }
    if(tc != MyMoneyTransaction()) {
      try {
        MyMoneyFile::instance()->addTransaction(tc);
      } catch(MyMoneyException *e) {
        qWarning("charge transaction not stored: '%s'", e->what().data());
        delete e;
      }
    }
    
    m_infoStack->raiseWidget(KLedgerView::Reconciliation);
    m_inReconciliation = true;
    m_summaryLine->hide();
    m_transactionFormActive = false;

    m_prevBalance = dlg.previousBalance();
    m_endingBalance = dlg.endingBalance();
    m_endingDate = dlg.statementDate();

    refreshView();
    fillReconcileData();

    // allow SPACE to be used to toggle the clear state
    connect(m_register, SIGNAL(signalSpace()), this, SLOT(slotToggleClearFlag()));
    m_transactionCheckBox->setChecked(false);
    slotShowTransactionForm(false);
    connect(m_transactionCheckBox, SIGNAL(toggled(bool)), this, SLOT(slotShowTransactionForm(bool)));
  }
}

void KLedgerViewCheckings::fillReconcileData(void)
{
  MyMoneyMoney cleared(m_prevBalance);

  for(unsigned int i = 0; i < m_transactionPtrVector.size(); ++i) {
    MyMoneyTransaction* t = m_transactionPtrVector[i];
    MyMoneySplit sp = t->splitByAccount(m_account.id());
    if(sp.reconcileFlag() == MyMoneySplit::Cleared)
      cleared += sp.value();
  }

  m_clearedLabel->setText(cleared.formatMoney());
  m_statementLabel->setText(m_endingBalance.formatMoney());
  m_differenceLabel->setText((cleared - m_endingBalance).formatMoney());
}

void KLedgerViewCheckings::slotRegisterClicked(int row, int col, int button, const QPoint &mousePos)
{
  if(!m_account.id().isEmpty()) {
    KLedgerView::slotRegisterClicked(row, col, button, mousePos);

    if(m_inReconciliation == true
    && col == 3) {    // reconcileFlag column
      slotToggleClearFlag();
    }
  }
}

void KLedgerViewCheckings::slotConfigureMoreMenu(void)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  int splitEditId = m_moreMenu->idAt(1);
  int gotoPayeeId = m_moreMenu->idAt(2);
  m_moreMenu->disconnectItem(splitEditId, this, SLOT(slotStartEditSplit()));
  m_moreMenu->disconnectItem(splitEditId, this, SLOT(slotGotoOtherSideOfTransfer()));

  m_moreMenu->changeItem(splitEditId, i18n("Edit splits ..."));
  if(m_transactionPtr != 0) {
    if(!m_split.payeeId().isEmpty() && !m_split.accountId().isEmpty() && !m_transaction.id().isEmpty()) {
      MyMoneyPayee payee = file->payee(m_split.payeeId());
      m_moreMenu->changeItem(gotoPayeeId, i18n("Goto '%1'").arg(payee.name()));
      m_moreMenu->setItemEnabled(gotoPayeeId, true);
    } else {
      m_moreMenu->changeItem(gotoPayeeId, i18n("Goto payee/receiver"));
      m_moreMenu->setItemEnabled(gotoPayeeId, false);
    }

    if(transactionType(m_split) != Transfer) {
      m_moreMenu->connectItem(splitEditId, this, SLOT(slotStartEditSplit()));
    } else {
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
    }
    m_moreMenu->setItemEnabled(splitEditId, true);
  } else {
    m_moreMenu->setItemEnabled(splitEditId, false);
  }
}

void KLedgerViewCheckings::slotConfigureContextMenu(void)
{
  int splitEditId = m_contextMenu->idAt(2);
  int gotoPayeeId = m_contextMenu->idAt(3);
  int deleteId = m_contextMenu->idAt(9);
  MyMoneyFile* file = MyMoneyFile::instance();

  m_contextMenu->disconnectItem(splitEditId, this, SLOT(slotStartEditSplit()));
  m_contextMenu->disconnectItem(splitEditId, this, SLOT(slotGotoOtherSideOfTransfer()));

  m_contextMenu->changeItem(splitEditId, i18n("Edit splits ..."));
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
    if(transactionType(m_split) != Transfer) {
      m_contextMenu->connectItem(splitEditId, this, SLOT(slotStartEditSplit()));
    } else {
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
    }
    m_contextMenu->setItemEnabled(splitEditId, true);
    m_contextMenu->setItemEnabled(deleteId, true);
  } else {
    m_contextMenu->setItemEnabled(splitEditId, false);
    m_contextMenu->setItemEnabled(deleteId, false);
  }
}

void KLedgerViewCheckings::slotToggleClearFlag(void)
{
  if(m_transactionPtr != 0) {
    MyMoneySplit sp = m_transactionPtr->splitByAccount(m_account.id());
    switch(sp.reconcileFlag()) {
      case MyMoneySplit::NotReconciled:
        sp.setReconcileFlag(MyMoneySplit::Cleared);
        break;
      default:
        sp.setReconcileFlag(MyMoneySplit::NotReconciled);
        break;
    }
    try {
      m_transactionPtr->modifySplit(sp);
      MyMoneyFile::instance()->modifyTransaction(*m_transactionPtr);

    } catch(MyMoneyException *e) {
      qDebug("Unexpected exception when setting reconcile flag");
      delete e;
    }

    // update the display is partially done through the observer
    // on the account in KLedgerView::update(). We add what's missing.
    fillReconcileData();
  }
}

void KLedgerViewCheckings::slotPostponeReconciliation(void)
{
  if(m_inReconciliation == true) {
    m_account.setValue("lastReconciledBalance", m_prevBalance.toString());
    m_account.setValue("statementBalance", m_endingBalance.toString());
    m_account.setValue("statementDate", m_endingDate.toString(Qt::ISODate));

    try {
      MyMoneyFile::instance()->modifyAccount(m_account);
    } catch(MyMoneyException *e) {
      qDebug("Unexpected exception when setting last reconcile info into account");
      delete e;
    }

    endReconciliation();
  }
}

void KLedgerViewCheckings::endReconciliation(void)
{
  QCString transactionId;
  KConfig *config = KGlobal::config();
  config->setGroup("General Options");

  m_infoStack->raiseWidget(KLedgerView::TransactionEdit);
  m_inReconciliation = false;
  m_summaryLine->show();
  m_transactionFormActive = config->readBoolEntry("TransactionForm", true);

  refreshView();
  
  disconnect(m_register, SIGNAL(signalSpace()), this, SLOT(slotToggleClearFlag()));
  disconnect(m_transactionCheckBox, SIGNAL(toggled(bool)), this, SLOT(slotShowTransactionForm(bool)));
}

void KLedgerViewCheckings::slotEndReconciliation(void)
{
  if(m_inReconciliation == true) {
    m_account.setValue("lastStatementBalance", m_endingBalance.toString());
    m_account.setValue("lastStatementDate", m_endingDate.toString(Qt::ISODate));

    m_account.deletePair("lastReconciledBalance");
    m_account.deletePair("statementBalance");
    m_account.deletePair("statementDate");

    MyMoneyFile::instance()->suspendNotify(true);
    try {
      MyMoneyFile::instance()->modifyAccount(m_account);
      MyMoneyTransactionFilter filter(m_account.id());
      QValueList<MyMoneyTransaction> list = MyMoneyFile::instance()->transactionList(filter);
      QValueList<MyMoneyTransaction>::Iterator it;

      for(it = list.begin(); it != list.end(); ++it) {
        MyMoneySplit sp = (*it).splitByAccount(m_account.id());
        if(sp.reconcileFlag() == MyMoneySplit::Cleared) {
          sp.setReconcileFlag(MyMoneySplit::Reconciled);
          sp.setReconcileDate(m_endingDate);
          (*it).modifySplit(sp);
          MyMoneyFile::instance()->modifyTransaction(*it);
        }
      }
    } catch(MyMoneyException *e) {
      qDebug("Unexpected exception when setting cleared to reconcile");
      delete e;
    }
    MyMoneyFile::instance()->suspendNotify(false);
    endReconciliation();
  }
}

void KLedgerViewCheckings::slotOpenSplitDialog(void)
{
  bool isDeposit = false;
  bool isValidAmount = false;

  if(m_transactionFormActive) {
    isDeposit = transactionType(m_split) == Deposit;
    isValidAmount = m_editAmount->text().length() != 0;
  } else {
    if(m_editPayment->text().length() != 0) {
      isDeposit = false;
      isValidAmount = true;
    }
    if(m_editDeposit->text().length() != 0) {
      isDeposit = true;
      isValidAmount = true;
    }
  }
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

void KLedgerViewCheckings::slotStartEditSplit(void)
{
  slotStartEdit();
  slotOpenSplitDialog();
}

void KLedgerViewCheckings::slotAccountDetail(void)
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

void KLedgerViewCheckings::slotPayeeSelected(void)
{
  slotCancelEdit();
  if(!m_split.payeeId().isEmpty() && !m_split.accountId().isEmpty() && !m_transaction.id().isEmpty())
    emit payeeSelected(m_split.payeeId(), m_split.accountId(), m_transaction.id());
}
