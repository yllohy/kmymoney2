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

#include "../widgets/kmymoneyregistercheckings.h"
#include "../widgets/kmymoneytransactionform.h"

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

  qDebug("Creating KLedgerViewInvestments for accound id = %s, name=%s", m_account.id().data(),m_account.name().data());

}

KLedgerViewInvestments::~KLedgerViewInvestments()
{

}

void KLedgerViewInvestments::fillForm()
{

}

void KLedgerViewInvestments::fillSummary()
{

}

void KLedgerViewInvestments::showWidgets()
{
  createEditWidgets();
	
	kMyMoneyTransactionFormTable* table = m_form->table();

	if(table)
	{
		table->setCellWidget(2, 1, m_editMemo);
		table->setCellWidget(0, 4, m_editDate);
	}
}

void KLedgerViewInvestments::hideWidgets()
{
  m_editPayee = 0;
  m_editCategory = 0;
  m_editMemo = 0;
  m_editAmount = 0;
  m_editNr = 0;
  m_editDate = 0;
  m_editSplit = 0;
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
  m_editPayee = new kMyMoneyPayee(0, "editPayee");
  m_editCategory = new kMyMoneyCategory(0, "editCategory");
  m_editMemo = new kMyMoneyLineEdit(0, "editMemo", AlignLeft|AlignVCenter);
  m_editAmount = new kMyMoneyEdit(0, "editAmount");
  m_editDate = new kMyMoneyDateInput(0, "editDate");
  m_editNr = new kMyMoneyLineEdit(0, "editNr");
	
	m_editQuantity = new kMyMoneyLineEdit(0, "editQuanity", AlignLeft|AlignVCenter);
	
	
  // m_editFrom = new kMyMoneyCategory(0, "editFrom", static_cast<KMyMoneyUtils::categoryTypeE> (KMyMoneyUtils::asset | KMyMoneyUtils::liability));
  // m_editTo = new kMyMoneyCategory(0, "editTo", static_cast<KMyMoneyUtils::categoryTypeE> (KMyMoneyUtils::asset | KMyMoneyUtils::liability));
  m_editSplit = new KPushButton("Split", 0, "editSplit");
  m_editPayment = new kMyMoneyEdit(0, "editPayment");
  m_editDeposit = new kMyMoneyEdit(0, "editDeposit");
  m_editType = new kMyMoneyCombo(0, "editType");
  m_editType->setFocusPolicy(QWidget::StrongFocus);

}

void KLedgerViewInvestments::createForm(void)
{
  m_form = new kMyMoneyTransactionForm(this, NULL, 0, 4, 5);


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

  // connections
  connect(m_form->tabBar(), SIGNAL(selected(int)), this, SLOT(slotTypeSelected(int)));

  connect(m_form->editButton(), SIGNAL(clicked()), this, SLOT(slotStartEdit()));
  connect(m_form->cancelButton(), SIGNAL(clicked()), this, SLOT(slotCancelEdit()));
  connect(m_form->enterButton(), SIGNAL(clicked()), this, SLOT(slotEndEdit()));
  connect(m_form->newButton(), SIGNAL(clicked()), this, SLOT(slotNew()));

  m_form->enterButton()->setDefault(true);
	
	slotTypeSelected(KLedgerViewInvestments::AddShares);
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
