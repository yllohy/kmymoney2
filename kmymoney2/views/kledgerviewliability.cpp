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

  m_register->repaintContents(false);
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
  m_editPayee = new kMyMoneyPayee(0, "editPayee");
  m_editCategory = new kMyMoneyCategory(0, "editCategory");
  m_editMemo = new kMyMoneyLineEdit(0, "editMemo", AlignLeft | AlignVCenter);
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
  connect(m_editDate, SIGNAL(dateChanged(const QDate&)), this, SLOT(slotDateChanged(const QDate&)));
  connect(m_editFrom, SIGNAL(categoryChanged(const QString&)), this, SLOT(slotFromChanged(const QString&)));
  connect(m_editTo, SIGNAL(categoryChanged(const QString&)), this, SLOT(slotToChanged(const QString&)));
  connect(m_editPayment, SIGNAL(valueChanged(const QString&)), this, SLOT(slotPaymentChanged(const QString&)));
  connect(m_editDeposit, SIGNAL(valueChanged(const QString&)), this, SLOT(slotDepositChanged(const QString&)));
  connect(m_editType, SIGNAL(selectionChanged(int)), this, SLOT(slotTypeChanged(int)));

  connect(m_editPayee, SIGNAL(signalEnter()), this, SLOT(slotEndEdit()));
  connect(m_editMemo, SIGNAL(signalEnter()), this, SLOT(slotEndEdit()));
  connect(m_editCategory, SIGNAL(signalEnter()), this, SLOT(slotEndEdit()));
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
  connect(m_editDate, SIGNAL(signalEsc()), this, SLOT(slotCancelEdit()));
  connect(m_editFrom, SIGNAL(signalEsc()), this, SLOT(slotCancelEdit()));
  connect(m_editTo, SIGNAL(signalEsc()), this, SLOT(slotCancelEdit()));
  connect(m_editAmount, SIGNAL(signalEsc()), this, SLOT(slotCancelEdit()));
  connect(m_editPayment, SIGNAL(signalEsc()), this, SLOT(slotCancelEdit()));
  connect(m_editDeposit, SIGNAL(signalEsc()), this, SLOT(slotCancelEdit()));
  connect(m_editType, SIGNAL(signalEsc()), this, SLOT(slotCancelEdit()));
}

void KLedgerViewLiability::fillSummary(void)
{
  MyMoneyMoney balance;
  QLabel *summary = static_cast<QLabel *> (m_summaryLine);

  if(!accountId().isEmpty()) {
    try {
      balance = MyMoneyFile::instance()->balance(accountId());
      summary->setText(i18n("You currently owe: ") + (-balance).formatMoney());
/* the fancy version. don't know, if we should use it
      if(balance < 0)
        summary->setText(i18n("You currently owe: ") + (-balance).formatMoney());
      else
        summary->setText(i18n("Current balance: ") + balance.formatMoney());
*/

    } catch(MyMoneyException *e) {
        qDebug("Unexpected exception in KLedgerViewCreditCard::fillSummary");
    }
  } else
    summary->setText("");
}
