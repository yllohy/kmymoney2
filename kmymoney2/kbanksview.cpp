/***************************************************************************
                          kbanksview.cpp
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes
    email                : Michael.Edwardes@students.dmu.ac.uk
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
#include "kmymoneysettings.h"
#include <qheader.h>
#include <kglobal.h>
#include <klocale.h>
#include <qtooltip.h>

KBanksView::KBanksView(QWidget *parent, const char *name)
 : KBankViewDecl(parent,name)
{
  descriptionLabel->setText(i18n("Double click on a bank to view it's accounts.\n"
    "You can create a new bank by using the Menu Bar (Bank | Add new...),\n"
    "right clicking on the list box or double clicking in an empty row"));

	bankListView->setRootIsDecorated(true);
	bankListView->setAllColumnsShowFocus(true);
	bankListView->addColumn(i18n("Banks"));
	bankListView->setMultiSelection(false);

  KMyMoneySettings *p_settings = KMyMoneySettings::singleton();
  if (p_settings)
    bankListView->header()->setFont(p_settings->lists_headerFont());

  connect(bankListView, SIGNAL(executed(QListViewItem*, const QPoint&, int)),
    this, SLOT(slotListDoubleClick(QListViewItem*, const QPoint&, int)));
  connect(bankListView, SIGNAL(rightButtonClicked(QListViewItem* , const QPoint&, int)),
    this, SLOT(slotListRightMouse(QListViewItem*, const QPoint&, int)));
  m_bSelectedBank=false;
  m_bSelectedAccount=false;
}

KBanksView::~KBanksView()
{
}

void KBanksView::slotListRightMouse(QListViewItem* item, const QPoint& point, int col)
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

void KBanksView::slotListDoubleClick(QListViewItem* item, const QPoint& point, int col)
{
  if (item!=0 && col!=-1) {
    KBankListItem *bankItem = (KBankListItem*)item;
    if (!bankItem->isBank()) {
      m_bSelectedBank = true;
      m_selectedBank = bankItem->bank();
      m_bSelectedAccount=true;
      m_selectedAccount = bankItem->account();
      emit accountDoubleClick(bankItem->account());
    }
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

void KBanksView::refresh(MyMoneyFile file)
{
  KMyMoneySettings *p_settings = KMyMoneySettings::singleton();
  if (p_settings)
    bankListView->header()->setFont(p_settings->lists_headerFont());

  clear();
  MyMoneyBank *bank;

  for ( bank=file.bankFirst(); bank; bank=file.bankNext() ) {
    KBankListItem *item0 = new KBankListItem(bankListView, *bank);
    MyMoneyAccount *account;
    for (account=bank->accountFirst(); account; account=bank->accountNext()) {
      KBankListItem *itemAccount = new KBankListItem(item0, *bank, *account);
    }
  }
}

void KBanksView::clear(void)
{
  bankListView->clear();
  m_bSelectedBank = m_bSelectedAccount = false;
}
