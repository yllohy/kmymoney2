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

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kledgerview.h"
#include "kledgerviewcheckings.h"
#include "kledgerviewinvestments.h"

#include "../widgets/kmymoneyregistercheckings.h"
#include "../widgets/kmymoneytransactionform.h"

KLedgerViewInvestments::KLedgerViewInvestments(QWidget *parent, const char *name) : KLedgerView(parent, name)
{
  createForm();
  createRegister();
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

void KLedgerViewInvestments::slotReconciliation()
{

}

void KLedgerViewInvestments::createForm()
{
  m_form = new kMyMoneyTransactionForm(this, 0, 0, 4, 5);

  m_SummaryTab = new QTab("Tab1");  
  m_TransactionTab = new QTab("Tab2");               
  /*m_tabCheck = new QTab(action2str(MyMoneySplit::ActionCheck, true));
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

  m_form->enterButton()->setDefault(true);*/
}

void KLedgerViewInvestments::createRegister(void)
{
  m_register = new kMyMoneyRegisterCheckings(this, "Checkings");
  m_register->setParent(this);

  /*m_register->setAction(QCString(MyMoneySplit::ActionATM), i18n("ATM"));
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
*/}

