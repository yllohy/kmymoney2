/***************************************************************************
                          kbanksview.cpp
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
#include "kbanksview.h"
#include "kbanklistitem.h"
#include <qheader.h>
#include <kglobal.h>
#include <klocale.h>
#include <qtooltip.h>
#include <klistview.h>
#include <kconfig.h>

KBanksView::KBanksView(QWidget *parent, const char *name)
 : KBankViewDecl(parent,name)
{
	bankListView->setRootIsDecorated(true);
	bankListView->setAllColumnsShowFocus(true);
	bankListView->addColumn(i18n("Institutions/Accounts"));
	bankListView->addColumn(i18n("Type"));
	bankListView->addColumn(i18n("Balance"));
	bankListView->setMultiSelection(false);
	bankListView->header()->setResizeEnabled(false);
	bankListView->setColumnWidthMode(0, QListView::Manual);

	KConfig *config = KGlobal::config();
  QFont defaultFont = QFont("helvetica", 12);
  bankListView->header()->setFont(config->readFontEntry("listHeaderFont", &defaultFont));

  connect(bankListView, SIGNAL(selectionChanged(QListViewItem*)),
    this, SLOT(slotSelectionChanged(QListViewItem*)));
  connect(bankListView, SIGNAL(rightButtonClicked(QListViewItem* , const QPoint&, int)),
    this, SLOT(slotListRightMouse(QListViewItem*, const QPoint&, int)));
  connect(bankListView, SIGNAL(doubleClicked(QListViewItem*, const QPoint&, int)),
    this, SLOT(slotListDoubleClicked(QListViewItem*, const QPoint&, int)));

  m_bSelectedBank=false;
  m_bSelectedAccount=false;

  // never show a horizontal scroll bar
  bankListView->setHScrollBarMode(QScrollView::AlwaysOff);
}

KBanksView::~KBanksView()
{
}

void KBanksView::slotListDoubleClicked(QListViewItem* pItem, const QPoint& pos, int c)
{
  if(pItem)
  {
    KBankListItem *bankItem = (KBankListItem*)pItem;
    if(bankItem->isBank())
    {
      m_bSelectedBank=true;
      m_selectedBank = bankItem->bank();
      m_bSelectedAccount=false;
    }
    else
    {
      m_bSelectedBank = true;
      m_selectedBank = bankItem->bank();
      m_bSelectedAccount=true;
      m_selectedAccount = bankItem->account();

      emit accountDoubleClick();
    }
  }
}

void KBanksView::slotListRightMouse(QListViewItem* item, const QPoint& , int col)
{
  MyMoneyBank returnBank;
  if (item==0 || col==-1) {
    m_bSelectedBank = m_bSelectedAccount = false;
    emit bankRightMouseClick(returnBank, false);
    return;
  }

  KBankListItem *bankItem = (KBankListItem*)item;
  if (bankItem->isBank()) {
    m_bSelectedBank=true;
    m_selectedBank = bankItem->bank();
    m_bSelectedAccount=false;
    emit bankRightMouseClick(bankItem->bank(), true);
  } else {
    m_bSelectedBank = true;
    m_selectedBank = bankItem->bank();
    m_bSelectedAccount=true;
    m_selectedAccount = bankItem->account();

    emit accountRightMouseClick(bankItem->account(), true);
  }
}

MyMoneyBank KBanksView::currentBank(bool& success)
{
  success=m_bSelectedBank;
  return m_selectedBank;
}

MyMoneyAccount KBanksView::currentAccount(bool& success)
{
  success=m_bSelectedAccount;
  return m_selectedAccount;
}

void KBanksView::refresh(MyMoneyFile file, MyMoneyAccount *selectAccount, MyMoneyBank *selectBank)
{
	KConfig *config = KGlobal::config();
  QFont defaultFont = QFont("helvetica", 12);
  bankListView->header()->setFont(config->readFontEntry("listHeaderFont", &defaultFont));

  clear();
  MyMoneyMoney totalProfit;
  MyMoneyBank *bank;

  if (selectAccount != NULL)
  {
    m_selectedAccount = *selectAccount;
  }
  else if (selectBank != NULL)
  {
    m_selectedBank = *selectBank;
  }


  KBankListItem *item;

  for ( bank=file.bankFirst(); bank; bank=file.bankNext() ) {
    KBankListItem *item0 = new KBankListItem(bankListView, *bank);
    // if this bank is identical to the selected bank, update flag
    if(item0->bank() == m_selectedBank)
    {
      m_bSelectedBank = true;
      item = item0;
    }

    MyMoneyAccount *account;
    for (account=bank->accountFirst(); account; account=bank->accountNext()) {
      KBankListItem *itemAccount = new KBankListItem(item0, *bank, *account);
      // if this account is identical to the selected account, update flag
      if(*account == m_selectedAccount)
      {
        m_bSelectedAccount = true;
        item = itemAccount;
      }
      totalProfit += account->balance();
    }
    bankListView->setOpen(item0, true);
  }

  QString s(i18n("Total Profits: "));
  s += KGlobal::locale()->formatMoney(totalProfit.amount(), "",
                                      KGlobal::locale()->fracDigits());

  totalProfitsLabel->setFont(config->readFontEntry("listCellFont", &defaultFont));
  totalProfitsLabel->setText(s);

  if (m_bSelectedBank || m_bSelectedAccount)
    bankListView->setSelected(item, true);
}

void KBanksView::clear(void)
{
  bankListView->clear();
  m_bSelectedBank = m_bSelectedAccount = false;
}

void KBanksView::resizeEvent(QResizeEvent* e)
{
	bankListView->setColumnWidth(0,400);
	bankListView->setColumnWidth(1,150);
	int totalWidth=bankListView->width();
	bankListView->setColumnWidth(2, totalWidth-550-5);

	// call base class resizeEvent()
	KBankViewDecl::resizeEvent(e);
}

void KBanksView::slotSelectionChanged(QListViewItem *item)
{
  KBankListItem *bankItem = (KBankListItem*)item;
  if (bankItem->isBank()) {
    m_bSelectedBank = true;
    m_selectedBank = bankItem->bank();
    emit bankSelected();
  } else {
    m_bSelectedBank = true;
    m_selectedBank = bankItem->bank();
    m_bSelectedAccount = true;
    m_selectedAccount = bankItem->account();
    emit accountSelected();
  }
}

void KBanksView::show()
{
  emit signalViewActivated();
  QWidget::show();
}
