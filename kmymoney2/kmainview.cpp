/***************************************************************************
                          kmainview.cpp
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes
    email                : mte@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kmainview.h"

KMainView::KMainView(QWidget *parent, const char *name )
 : QWidget(parent,name)
{
	this->resize(690, 450);
	this->setMinimumSize(690, 450);
  banksView = new KBanksView(this, "banksView");
  transactionView = new KTransactionView(this, "transactionsView");

  connect(banksView, SIGNAL(bankRightMouseClick(const MyMoneyBank, bool)), this, SLOT(slotBRightMouseClick(const MyMoneyBank, bool)));
  connect(banksView, SIGNAL(accountRightMouseClick(const MyMoneyAccount, bool)), this, SLOT(slotARightMouseClick(const MyMoneyAccount, bool)));
  connect(banksView, SIGNAL(accountDoubleClick(const MyMoneyAccount)), this, SLOT(slotADoubleClick(const MyMoneyAccount)));

  connect(transactionView, SIGNAL(transactionListChanged()), this, SLOT(slotTransactionListChanged()));

  banksView->hide();
  transactionView->hide();
  m_showing=None;
}

KMainView::~KMainView()
{
}

void KMainView::slotBRightMouseClick(const MyMoneyBank bank, bool inList)
{
  emit bankRightMouseClick(bank, inList);
}

void KMainView::slotARightMouseClick(const MyMoneyAccount account, bool inList)
{
  emit accountRightMouseClick(account, inList);
}

void KMainView::slotADoubleClick(const MyMoneyAccount account)
{
  emit accountDoubleClick(account);
}

void KMainView::slotTransactionListChanged()
{
  emit transactionListChanged();
}

void KMainView::clear(void)
{
  banksView->clear();
  banksView->hide();

  transactionView->clear();
//  transactionView->clearTabbedInputBox();
  transactionView->hide();

  m_showing=None;
}

void KMainView::refreshBankView(MyMoneyFile file)
{
  banksView->refresh(file);
}

void KMainView::refreshTransactionView(void)
{
  transactionView->refresh();
}

MyMoneyBank KMainView::currentBank(bool& success)
{
  return banksView->currentBank(success);
}

MyMoneyAccount KMainView::currentAccount(bool& success)
{
  return banksView->currentAccount(success);
}

void KMainView::viewBankList(void)
{
  banksView->show();
  transactionView->hide();
  m_showing = BankList;
}

void KMainView::viewTransactionList(void)
{
  banksView->hide();
  transactionView->show();
  m_showing = TransactionList;
}

void KMainView::resizeEvent(QResizeEvent *e)
{
  QSize size = e->size();
  resize(size);
  banksView->resize(size);
  transactionView->resize(size);
}

void KMainView::showInputBox(bool val)
{
  transactionView->showInputBox(val);
}

void KMainView::initTransactionView(MyMoneyFile *file, const MyMoneyBank bank, const MyMoneyAccount account)
{
  transactionView->init(file, bank, account);
}
