/***************************************************************************
                          kmergetransactionsdlg.cpp
                             -------------------
    begin                : Sun Aug 20 2006
    copyright            : (C) 2006 by Ace Jones
    email                : <acejones@users.sf.net>
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

#include <qpushbutton.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kapplication.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "../mymoney/mymoneyfile.h"
#include "../mymoney/mymoneytransaction.h"
#include "kmergetransactionsdlg.h"

KMergeTransactionsDlg::KMergeTransactionsDlg(QCString _accountid): m_displayaccountid(_accountid)
{
  m_register->setParent(this);

  connect(m_helpButton, SIGNAL(clicked()), this, SLOT(slotHelp()));
}

void KMergeTransactionsDlg::addTransaction(const QCString& id)
{
  KMyMoneyTransaction ktx(MyMoneyFile::instance()->transaction(id));
  ktx.setSplitId(ktx.splitByAccount(m_displayaccountid).id());
  m_transactionList += ktx;
  m_register->setTransactionCount(m_transactionList.count());
}

void KMergeTransactionsDlg::slotHelp(void)
{
  kapp->invokeHelp("details.ledgers.match");
}

void KMergeTransactionsDlg::show(void)
{
  KMergeTransactionsDlgDecl::show();
  resizeRegister();
}

void KMergeTransactionsDlg::resizeEvent(QResizeEvent* ev)
{
  // don't forget the resizer
  KMergeTransactionsDlgDecl::resizeEvent(ev);

  // resize the register
  resizeRegister();
}

void KMergeTransactionsDlg::resizeRegister(void)
{
  int w = m_register->visibleWidth();

  int m_debitWidth = 80;
  int m_creditWidth = 80;

  m_register->adjustColumn(0);
  m_register->adjustColumn(1);
  m_register->adjustColumn(2);

  m_register->setColumnWidth(4, m_debitWidth);
  m_register->setColumnWidth(5, m_creditWidth);

  for(int i = 0; i < m_register->numCols(); ++i) {
    switch(i) {
      default:
        w -= m_register->columnWidth(i);
        break;
      case 3:     // skip the one, we want to set
        break;
    }
  }
  m_register->setColumnWidth(3, w);
}

#include "kmergetransactionsdlg.moc"
// vim:cin:si:ai:et:ts=2:sw=2:
