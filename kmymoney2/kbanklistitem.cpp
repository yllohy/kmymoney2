/***************************************************************************
                          kbanklistitem.cpp
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
#include <kglobal.h>
#include <klocale.h>
#include <kstddirs.h>
#include <qpixmap.h>

#include "kbanklistitem.h"
#include "kmymoneysettings.h"

KBankListItem::KBankListItem(QListView *parent, MyMoneyBank bank )
 : QListViewItem(parent)
{
  m_bank = bank;

  setText(0, m_bank.name());
  setPixmap(0, QPixmap(KGlobal::dirs()->findResource("appdata", "icons/hicolor/22x22/actions/bank.png")));
  setText(1, "Bank");  // dynamic in future, depending on institution type
  MyMoneyMoney balance;
  MyMoneyAccount *account;
  for (account=bank.accountFirst(); account; account=bank.accountNext())
    balance += account->balance();
  setText(2, KGlobal::locale()->formatMoney(balance.amount()));
  m_isBank=true;
}

KBankListItem::KBankListItem(KBankListItem *parent, MyMoneyBank bank, MyMoneyAccount account)
  : QListViewItem(parent)
{
  m_bank = bank;
  m_account = account;

  setText(0, m_account.name());
  setPixmap(0, QPixmap(KGlobal::dirs()->findResource("appdata", "pics/account-folder.xpm")));
  setText(1, "Current"); // dynamic in future...
  setText(2, KGlobal::locale()->formatMoney(m_account.balance().amount()));

  m_isBank=false;
}

KBankListItem::~KBankListItem()
{
}

MyMoneyBank KBankListItem::bank(void)
{
  return m_bank;
}

MyMoneyAccount KBankListItem::account(void)
{
  return m_account;
}

bool KBankListItem::isBank(void)
{
  return m_isBank;
}

void KBankListItem::paintCell(QPainter *p, const QColorGroup & cg, int column, int width, int align)
{
  KMyMoneySettings *p_settings = KMyMoneySettings::singleton();
  if (p_settings)
    p->setFont(p_settings->lists_cellFont());
  if (column==2)
    QListViewItem::paintCell(p, cg, column, width, 2);
  else
    QListViewItem::paintCell(p, cg, column, width, align);
}
