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
#include "../widgets/kmymoneycombo.h"


KLedgerViewSavings::KLedgerViewSavings(QWidget *parent, const char *name )
  : KLedgerViewCheckings(parent,name)
{
  m_form->tabBar()->removeTab(m_tabCheck);
  m_form->tabBar()->removeTab(m_tabAtm);
  m_tabCheck = m_tabAtm = 0;
  KConfig *config = KGlobal::config();
  config->setGroup("General Options");
  if(config->readBoolEntry("AlwaysShowNrField", false) == false)
    m_register->hideColumn(0);
  m_register->repaintContents(false);

  // setup action index
  m_actionIdx[0] =
  m_actionIdx[1] =
  m_actionIdx[3] = 0;
  m_actionIdx[2] = 1;
  m_actionIdx[3] = 2;
}

KLedgerViewSavings::~KLedgerViewSavings()
{
}

void KLedgerViewSavings::resizeEvent(QResizeEvent* /* ev */)
{
  // resize the register
  int w = m_register->visibleWidth();

  KConfig *config = KGlobal::config();
  config->setGroup("General Options");
  if(config->readBoolEntry("AlwaysShowNrField", false) == false) {
    m_register->hideColumn(0);
    m_register->setColumnWidth(0, 0);             // we don't have a Nr. here
  } else {
    m_register->showColumn(0);
    m_register->adjustColumn(0);
  }

  // check which space we need
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

void KLedgerViewSavings::createEditWidgets(void)
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
    m_editMemo = new kMyMoneyLineEdit(0, "editMemo", AlignLeft | AlignVCenter);
    connect(m_editMemo, SIGNAL(lineChanged(const QString&)), this, SLOT(slotMemoChanged(const QString&)));
  }

  if(!m_editAmount) {
    m_editAmount = new kMyMoneyEdit(0, "editAmount");
    connect(m_editAmount, SIGNAL(valueChanged(const QString&)), this, SLOT(slotAmountChanged(const QString&)));
  }

  if(!m_editDate) {
    m_editDate = new kMyMoneyDateInput(0, "editDate");
    connect(m_editDate, SIGNAL(dateChanged(const QDate&)), this, SLOT(slotDateChanged(const QDate&)));
  }

  if(!m_editNr) {
    m_editNr = new kMyMoneyLineEdit(0, "editNr");
    connect(m_editNr, SIGNAL(lineChanged(const QString&)), this, SLOT(slotNrChanged(const QString&)));
  }

  if(!m_editSplit) {
    m_editSplit = new KPushButton("Split", 0, "editSplit");
    connect(m_editSplit, SIGNAL(clicked()), this, SLOT(slotOpenSplitDialog()));
  }

  if(!m_editPayment) {
    m_editPayment = new kMyMoneyEdit(0, "editPayment");
    connect(m_editPayment, SIGNAL(valueChanged(const QString&)), this, SLOT(slotPaymentChanged(const QString&)));
  }

  if(!m_editDeposit) {
    m_editDeposit = new kMyMoneyEdit(0, "editDeposit");
    connect(m_editDeposit, SIGNAL(valueChanged(const QString&)), this, SLOT(slotDepositChanged(const QString&)));
  }

  if(!m_editType) {
    m_editType = new kMyMoneyCombo(0, "editType");
    m_editType->setFocusPolicy(QWidget::StrongFocus);
    connect(m_editType, SIGNAL(selectionChanged(int)), this, SLOT(slotActionSelected(int)));
  }
}

void KLedgerViewSavings::slotReconciliation(void)
{
  KLedgerViewCheckings::slotReconciliation();
}

bool KLedgerViewSavings::eventFilter( QObject *o, QEvent *e )
{
  return KLedgerViewCheckings::eventFilter(o, e);
}
