/***************************************************************************
                          ksplittransactiondlg.cpp  -  description
                             -------------------
    begin                : Thu Jan 10 2002
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

#include <kglobal.h>
#include <kconfig.h>
#include <klocale.h>
#include <kiconloader.h>

#include <qpushbutton.h>
#include <qlabel.h>
#include <qtable.h>
#include <qtimer.h>

#include "ksplittransactiondlg.h"
#include "../widgets/kmymoneysplittable.h"

KSplitTransactionDlg::KSplitTransactionDlg(QWidget* parent,  const char* name,
                                           MyMoneyMoney* amount, const bool amountValid) :
  kSplitTransactionDlgDecl(parent, name, true),
  m_amountTransaction(amount),
  m_amountValid(amountValid),
  m_numSplits(0),
  m_numExtraLines(0)
{
  transactionsTable->setNumRows(1);
  transactionsTable->setNumCols(3);
  transactionsTable->horizontalHeader()->setLabel(0, i18n("Category"));
	transactionsTable->horizontalHeader()->setLabel(1, i18n("Memo"));
	transactionsTable->horizontalHeader()->setLabel(2, i18n("Amount"));
	transactionsTable->setSelectionMode(QTable::NoSelection);
 	transactionsTable->setLeftMargin(0);
	transactionsTable->verticalHeader()->hide();
  transactionsTable->setColumnStretchable(0, false);
  transactionsTable->setColumnStretchable(1, false);
	transactionsTable->setColumnStretchable(2, false);
	transactionsTable->horizontalHeader()->setResizeEnabled(false);
	transactionsTable->horizontalHeader()->setMovingEnabled(false);

  // set up an initial width for the amount column
  initAmountWidth();

  // create required input widgets
  createInputWidgets();

  // initialize the sum display
  updateSums();

  // connect signals with slots
	connect(cancelBtn, SIGNAL(clicked()), this, SLOT(slotCancelClicked()));
	connect(finishBtn, SIGNAL(clicked()), this, SLOT(slotFinishClicked()));
  connect(clearAllBtn, SIGNAL(clicked()), this, SLOT(slotClearAllClicked()));

  // Trick: it seems, that the initial sizing of the dialog does
  // not work correctly. At least, the columns do not get displayed
  // correct. Reason: the return value of transactionsTable->visibleWidth()
  // is incorrect. If the widget is visible, resizing works correctly.
  // So, we let the dialog show up and resize it then. It's not really
  // clean, but the only way I got the damned thing working.
  QTimer::singleShot( 10, this, SLOT(initSize()) );
}

void KSplitTransactionDlg::createInputWidgets(void)
{
  m_amount = new kMyMoneyEdit(0);
  m_category = new kMyMoneyCombo(false, 0);
  m_memo = new kMyMoneyLineEdit(0);
}

KSplitTransactionDlg::~KSplitTransactionDlg()
{
}

void KSplitTransactionDlg::initSize(void)
{
  QDialog::resize(width(), height()+1);
}

void KSplitTransactionDlg::resizeEvent(QResizeEvent* ev)
{
  int w = transactionsTable->visibleWidth() - m_amountWidth;

  // resize the columns
  transactionsTable->setColumnWidth(0, w/2);
  transactionsTable->setColumnWidth(1, w/2);
  transactionsTable->setColumnWidth(2, m_amountWidth);

  // get current size of transactions table
  int rowHeight = transactionsTable->cellGeometry(0, 0).height();
  int tableHeight = transactionsTable->height();

  // see if we need some extra lines to fill the current size with the grid
  m_numExtraLines = (tableHeight / rowHeight) - m_numSplits;
  if(m_numExtraLines < 0)
    m_numExtraLines = 0;

  transactionsTable->setNumRows(m_numSplits + m_numExtraLines);
}

void KSplitTransactionDlg::initAmountWidth(void)
{
  m_amountWidth = 80;
}

void KSplitTransactionDlg::slotFinishClicked()
{
  accept();
}

void KSplitTransactionDlg::slotCancelClicked()
{
  reject();
}

void KSplitTransactionDlg::slotClearAllClicked()
{
}

void KSplitTransactionDlg::updateSums(void)
{
  MyMoneyMoney diff;

  if(m_amountTransaction == NULL) {
    qDebug("pointer to transaction amount is NULL in KSplitTransactionDlg::updateSums");
    return;
  }

  // if there is an amount specified in the transaction, we need to calculate the
  // difference, otherwise we display the difference as 0 and display the same sum.
  if(m_amountValid) {
    diff = *m_amountTransaction - m_amountSplits;
  } else {
    *m_amountTransaction = m_amountSplits;
    diff = 0.0;
  }

  splitSum->setText("<b>" + KGlobal::locale()->formatMoney(m_amountSplits.amount(), "") + " ");
  splitUnassigned->setText("<b>" + KGlobal::locale()->formatMoney(diff.amount(), "") + " ");
  transactionAmount->setText("<b>" + KGlobal::locale()->formatMoney(m_amountTransaction->amount(), "") + " ");
}

