/***************************************************************************
                          kledgerviewliability.cpp  -  description
                             -------------------
    begin                : Sun Sep 28 2003
    copyright            : (C) 2000-2003 by Michael Edwardes
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
#include <kpushbutton.h>

// ----------------------------------------------------------------------------
// Project Includes
#include "kledgerviewliability.h"
#include "../widgets/kmymoneytransactionform.h"
#include "../widgets/kmymoneyregister.h"
#include "../widgets/kmymoneyedit.h"
#include "../widgets/kmymoneydateinput.h"
#include "../widgets/kmymoneypayee.h"
#include "../widgets/kmymoneycategory.h"
#include "../widgets/kmymoneylineedit.h"
#include "../widgets/kmymoneycombo.h"
#include "../mymoney/mymoneyfile.h"

KLedgerViewLiability::KLedgerViewLiability(QWidget *parent, const char *name )
  : KLedgerViewCheckings(parent,name)
{
        m_register->horizontalHeader()->setLabel(4, i18n("Increase"));
        m_register->horizontalHeader()->setLabel(5, i18n("Decrease"));

  m_form->tabBar()->removeTab(m_tabCheck);
  m_form->tabBar()->removeTab(m_tabAtm);
  m_form->tabBar()->tabAt(0)->setText(i18n("Decrease"));
  m_form->tabBar()->tabAt(2)->setText(i18n("Increase"));

  m_register->setAction(QCString(MyMoneySplit::ActionDeposit), i18n("Decrease"));
  m_register->setAction(QCString(MyMoneySplit::ActionWithdrawal), i18n("Increase"));

  m_register->repaintContents(false);

  // setup action index
  m_actionIdx[0] =
  m_actionIdx[1] =
  m_actionIdx[3] = 0;
  m_actionIdx[2] = 1;
  m_actionIdx[3] = 2;
}

KLedgerViewLiability::~KLedgerViewLiability()
{
}

void KLedgerViewLiability::slotReconciliation(void)
{
  KLedgerViewCheckings::slotReconciliation();
}

void KLedgerViewLiability::createEditWidgets(void)
{
  if(!m_editPayee) {
    m_editPayee = new kMyMoneyPayee(0, "editPayee");
    connect(m_editPayee, SIGNAL(newPayee(const QString&)), this, SLOT(slotNewPayee(const QString&)));
    connect(m_editPayee, SIGNAL(payeeChanged(const QString&)), this, SLOT(slotPayeeChanged(const QString&)));
    connect(m_editPayee, SIGNAL(signalEnter()), this, SLOT(slotEndEdit()));
    connect(m_editPayee, SIGNAL(signalEsc()), this, SLOT(slotCancelEdit()));
  }

  if(!m_editCategory) {
    m_editCategory = new kMyMoneyCategory(0, "editCategory");
    connect(m_editCategory, SIGNAL(categoryChanged(const QCString&)), this, SLOT(slotCategoryChanged(const QCString&)));
    connect(m_editCategory, SIGNAL(signalEnter()), this, SLOT(slotEndEdit()));
    connect(m_editCategory, SIGNAL(signalEsc()), this, SLOT(slotCancelEdit()));
  }

  if(!m_editMemo) {
    m_editMemo = new kMyMoneyLineEdit(0, "editMemo", AlignLeft | AlignVCenter);
    connect(m_editMemo, SIGNAL(lineChanged(const QString&)), this, SLOT(slotMemoChanged(const QString&)));
    connect(m_editMemo, SIGNAL(signalEnter()), this, SLOT(slotEndEdit()));
    connect(m_editMemo, SIGNAL(signalEsc()), this, SLOT(slotCancelEdit()));
  }

  if(!m_editAmount) {
    m_editAmount = new kMyMoneyEdit(0, "editAmount");
    connect(m_editAmount, SIGNAL(valueChanged(const QString&)), this, SLOT(slotAmountChanged(const QString&)));
    connect(m_editAmount, SIGNAL(signalEnter()), this, SLOT(slotEndEdit()));
    connect(m_editAmount, SIGNAL(signalEsc()), this, SLOT(slotCancelEdit()));
  }

  if(!m_editDate) {
    m_editDate = new kMyMoneyDateInput(0, "editDate");
    connect(m_editDate, SIGNAL(dateChanged(const QDate&)), this, SLOT(slotDateChanged(const QDate&)));
    connect(m_editDate, SIGNAL(signalEnter()), this, SLOT(slotEndEdit()));
    connect(m_editDate, SIGNAL(signalEsc()), this, SLOT(slotCancelEdit()));
  }

  if(!m_editNr) {
    m_editNr = new kMyMoneyLineEdit(0, "editNr");
    connect(m_editNr, SIGNAL(lineChanged(const QString&)), this, SLOT(slotNrChanged(const QString&)));
    connect(m_editNr, SIGNAL(signalEnter()), this, SLOT(slotEndEdit()));
    connect(m_editNr, SIGNAL(signalEsc()), this, SLOT(slotCancelEdit()));
  }

  if(!m_editSplit) {
    m_editSplit = new KPushButton("Split", 0, "editSplit");
    connect(m_editSplit, SIGNAL(clicked()), this, SLOT(slotOpenSplitDialog()));
  }

  if(!m_editPayment) {
    m_editPayment = new kMyMoneyEdit(0, "editPayment");
    connect(m_editPayment, SIGNAL(valueChanged(const QString&)), this, SLOT(slotPaymentChanged(const QString&)));
    connect(m_editPayment, SIGNAL(signalEnter()), this, SLOT(slotEndEdit()));
    connect(m_editPayment, SIGNAL(signalEsc()), this, SLOT(slotCancelEdit()));
  }

  if(!m_editDeposit) {
    m_editDeposit = new kMyMoneyEdit(0, "editDeposit");
    connect(m_editDeposit, SIGNAL(valueChanged(const QString&)), this, SLOT(slotDepositChanged(const QString&)));
    connect(m_editDeposit, SIGNAL(signalEnter()), this, SLOT(slotEndEdit()));
    connect(m_editDeposit, SIGNAL(signalEsc()), this, SLOT(slotCancelEdit()));
  }

  if(!m_editType) {
    m_editType = new kMyMoneyCombo(0, "editType");
    m_editType->setFocusPolicy(QWidget::StrongFocus);
    connect(m_editType, SIGNAL(selectionChanged(int)), this, SLOT(slotActionSelected(int)));
    connect(m_editType, SIGNAL(signalEnter()), this, SLOT(slotEndEdit()));
    connect(m_editType, SIGNAL(signalEsc()), this, SLOT(slotCancelEdit()));
  }
}

void KLedgerViewLiability::fillSummary(void)
{
  MyMoneyMoney balance;
  MyMoneyFile* file = MyMoneyFile::instance();

  KLedgerViewCheckingsSummaryLine* summary = dynamic_cast<KLedgerViewCheckingsSummaryLine*>(m_summaryLine);
  if(summary) {
    summary->clear();

    if(!accountId().isEmpty()) {
      try {
        balance = file->balance(accountId());
        summary->setBalance(i18n("You currently owe: ") + (-balance).formatMoney(file->currency(m_account.currencyId()).tradingSymbol()));
  /* the fancy version. don't know, if we should use it
        if(balance < 0)
          summary->setText(i18n("You currently owe: ") + (-balance).formatMoney());
        else
          summary->setText(i18n("Current balance: ") + balance.formatMoney());
  */
        QDate date;
        if(!m_account.value("lastStatementDate").isEmpty())
          date = QDate::fromString(m_account.value("lastStatementDate"), Qt::ISODate);
        if(date.isValid())
          summary->setReconciliationDate(i18n("Reconciled: %1").arg(KGlobal::locale()->formatDate(date, true)));

      } catch(MyMoneyException *e) {
          qDebug("Unexpected exception in KLedgerViewCreditCard::fillSummary");
      }
    }
  }
}
