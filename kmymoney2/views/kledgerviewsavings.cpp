/***************************************************************************
                          kledgerviewsavings.cpp  -  description
                             -------------------
    begin                : Wed Nov 6 2002
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

// ----------------------------------------------------------------------------
// KDE Includes

#include <kpushbutton.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kledgerviewsavings.h"
#include "../widgets/kmymoneytransactionform.h"
#include "../widgets/kmymoneyregister.h"
#include "../widgets/kmymoneyedit.h"
#include "../widgets/kmymoneydateinput.h"
#include "../widgets/kmymoneypayee.h"
#include "../widgets/kmymoneycategory.h"
#include "../widgets/kmymoneylineedit.h"


KLedgerViewSavings::KLedgerViewSavings(QWidget *parent, const char *name )
  : KLedgerViewCheckings(parent,name)
{
  m_form->tabBar()->removeTab(m_tabCheck);
  m_form->tabBar()->removeTab(m_tabAtm);
  m_register->hideColumn(0);
  m_register->repaintContents();
}

KLedgerViewSavings::~KLedgerViewSavings()
{
}

void KLedgerViewSavings::resizeEvent(QResizeEvent* ev)
{
  // resize the register
  int w = m_register->visibleWidth();

  int m_debitWidth = 80;
  int m_creditWidth = 80;
  int m_balanceWidth = 100;

  m_register->setColumnWidth(0, 0);             // we don't have a Nr. here
  m_register->setColumnWidth(1, 100);
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

void KLedgerViewSavings::createEditWidgets(void)
{
  m_editPayee = new kMyMoneyPayee(0, "editPayee");
  m_editCategory = new kMyMoneyCategory(0, "editCategory");
  m_editMemo = new kMyMoneyLineEdit(0, "editMemo");
  m_editAmount = new kMyMoneyEdit(0, "editAmount");
  m_editDate = new kMyMoneyDateInput(0, "editDate");
  m_editNr = 0;
  m_editFrom = new kMyMoneyCategory(0, "editFrom", static_cast<kMyMoneyCategory::categoryTypeE> (kMyMoneyCategory::asset | kMyMoneyCategory::liability));
  m_editTo = new kMyMoneyCategory(0, "editTo", static_cast<kMyMoneyCategory::categoryTypeE> (kMyMoneyCategory::asset | kMyMoneyCategory::liability));
  m_editSplit = new KPushButton("Split", 0, "editSplit");
  m_editPayment = new kMyMoneyEdit(0, "editPayment");
  m_editDeposit = new kMyMoneyEdit(0, "editDeposit");

  // for now, the split button is not usable
  m_editSplit->setEnabled(false);

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

  connect(m_editPayee, SIGNAL(signalEnter()), this, SLOT(slotEndEdit()));
  connect(m_editMemo, SIGNAL(signalEnter()), this, SLOT(slotEndEdit()));
  connect(m_editCategory, SIGNAL(signalEnter()), this, SLOT(slotEndEdit()));
  connect(m_editDate, SIGNAL(signalEnter()), this, SLOT(slotEndEdit()));
  connect(m_editFrom, SIGNAL(signalEnter()), this, SLOT(slotEndEdit()));
  connect(m_editTo, SIGNAL(signalEnter()), this, SLOT(slotEndEdit()));
  connect(m_editAmount, SIGNAL(signalEnter()), this, SLOT(slotEndEdit()));
  connect(m_editPayment, SIGNAL(signalEnter()), this, SLOT(slotEndEdit()));
  connect(m_editDeposit, SIGNAL(signalEnter()), this, SLOT(slotEndEdit()));

  connect(m_editPayee, SIGNAL(signalEsc()), this, SLOT(slotCancelEdit()));
  connect(m_editMemo, SIGNAL(signalEsc()), this, SLOT(slotCancelEdit()));
  connect(m_editCategory, SIGNAL(signalEsc()), this, SLOT(slotCancelEdit()));
  connect(m_editDate, SIGNAL(signalEsc()), this, SLOT(slotCancelEdit()));
  connect(m_editFrom, SIGNAL(signalEsc()), this, SLOT(slotCancelEdit()));
  connect(m_editTo, SIGNAL(signalEsc()), this, SLOT(slotCancelEdit()));
  connect(m_editAmount, SIGNAL(signalEsc()), this, SLOT(slotCancelEdit()));
  connect(m_editPayment, SIGNAL(signalEsc()), this, SLOT(slotCancelEdit()));
  connect(m_editDeposit, SIGNAL(signalEsc()), this, SLOT(slotCancelEdit()));
}
