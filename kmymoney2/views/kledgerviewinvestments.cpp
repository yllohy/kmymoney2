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
  QGridLayout* formLayout = new QGridLayout( this, 1, 2, 11, 6, "FormLayout");
  QVBoxLayout* ledgerLayout = new QVBoxLayout( 6, "LedgerLayout");

  createInfoStack();
  formLayout->addWidget(m_infoStack, 0, 1 );

  createRegister();
  ledgerLayout->addWidget(m_register, 3);

  createSummary();
  ledgerLayout->addLayout(m_summaryLayout);

  createForm();
  ledgerLayout->addWidget(m_form);

  formLayout->addLayout( ledgerLayout, 0, 0);

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

}

void KLedgerViewInvestments::hideWidgets()
{

}

void KLedgerViewInvestments::reloadEditWidgets(const MyMoneyTransaction& t)
{

}

void KLedgerViewInvestments::slotReconciliation(void)
{

}

void KLedgerViewInvestments::createForm(void)
{
/*
  mainGrid = new QGridLayout( this, 1, 1, 5, 5 );

  m_InvestmentTabs = new QTabWidget(this);

  m_SummaryTab = new QWidget(m_InvestmentTabs);
  m_TransactionTab = new QWidget(m_InvestmentTabs);

  

  textBrowser = new KTextBrowser(m_SummaryTab);
  
  // QLabel *liFirstName = new QLabel( "First &Name", m_SummaryTab );
  

  QGridLayout *grid2 = new QGridLayout( m_TransactionTab, 2, 1, 5, 5 );

  m_form = new kMyMoneyTransactionForm(this, NULL, 15, 4, 5);
  //m_form->hide();
  m_register = new kMyMoneyRegisterCheckings(this, "Checkings");
  m_register->setParent(this);
  //m_register->hide();
  
  grid2->addWidget(m_form, 1, 0);
  grid2->addWidget(m_register, 0, 0);

  //QPoint point(0,0);
  //m_form->reparent(m_TransactionTab, QPoint(300,0), true);
  
  //m_register->reparent(m_TransactionTab, QPoint(0, 100), true);
  //m_form->show();
  //m_register->show();
  
  m_InvestmentTabs->addTab( m_SummaryTab, "&Account Summary" );
  m_InvestmentTabs->addTab( m_TransactionTab, "Account &Transactions" );

  mainGrid->addWidget( m_InvestmentTabs, 0, 0 );


  //this->addWidget(m_InvestmentTabs);
    
  //m_SummaryTab = new QTab("Tab1");
  //m_TransactionTab = new QTab("Tab2");
  //m_InvestmentTabs->addTab(m_SummaryTab);
  //m_InvestmentTabs->addTab(m_TransactionTab);
  
  

              
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
*/
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
/*
  connect(m_register, SIGNAL(clicked(int, int, int, const QPoint&)), this, SLOT(slotRegisterClicked(int, int, int, const QPoint&)));
  connect(m_register, SIGNAL(doubleClicked(int, int, int, const QPoint&)), this, SLOT(slotRegisterDoubleClicked(int, int, int, const QPoint&)));

  connect(m_register, SIGNAL(signalEnter()), this, SLOT(slotStartEdit()));

  connect(m_register, SIGNAL(signalNextTransaction()), this, SLOT(slotNextTransaction()));
  connect(m_register, SIGNAL(signalPreviousTransaction()), this, SLOT(slotPreviousTransaction()));
  connect(m_register, SIGNAL(signalDelete()), this, SLOT(slotDeleteTransaction()));
  connect(m_register, SIGNAL(signalSelectTransaction(const QCString&)), this, SLOT(selectTransaction(const QCString&)));

  connect(m_register->horizontalHeader(), SIGNAL(clicked(int)), this, SLOT(slotRegisterHeaderClicked(int)));
*/
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

