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
#include <kconfig.h>
#include <klocale.h>
#include <kstddirs.h>
#include <qpixmap.h>
#include <qcolor.h>

#include "kbanklistitem.h"

#if QT_VERSION > 300
#include <qpainter.h>
#endif

KBankListItem::KBankListItem(QListView *parent, MyMoneyBank bank )
 : QListViewItem(parent)
{
  m_bank = bank;

  setText(0, m_bank.name());
  setPixmap(0, QPixmap(KGlobal::dirs()->findResource("appdata", "icons/hicolor/22x22/actions/bank.png")));
  setText(1, i18n("Bank"));  // dynamic in future, depending on institution type
  MyMoneyMoney balance;
  MyMoneyAccount *account;
  for (account=bank.accountFirst(); account; account=bank.accountNext())
    balance += account->balance();
  setText(2, KGlobal::locale()->formatMoney(balance.amount(), "",
                                            KGlobal::locale()->fracDigits()));
  m_isBank=true;
}

KBankListItem::KBankListItem(KBankListItem *parent, MyMoneyBank bank, MyMoneyAccount account)
  : QListViewItem(parent)
{
  m_bank = bank;
  m_account = account;

  setText(0, m_account.name());
  setPixmap(0, QPixmap(KGlobal::dirs()->findResource("appdata", "icons/hicolor/22x22/actions/account.png")));
  //setText(1, i18n("Current")); // dynamic in future...
  setText(1, m_account.getTypeName());
	setText(2, KGlobal::locale()->formatMoney(m_account.balance().amount(), "",
                                            KGlobal::locale()->fracDigits()));

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
	KConfig *config = KGlobal::config();
  config->setGroup("List Options");
  QFont defaultFont = QFont("helvetica", 12);
  p->setFont(config->readFontEntry("listCellFont", &defaultFont));

  if (column==2) {
    QFont font = p->font();
    if (isBank())
      font.setBold(true);
    else {
      if (m_account.balance().amount()<0)
        font.setItalic(true);
    }
    p->setFont(font);
    QListViewItem::paintCell(p, cg, column, width, 2);
  }
  else
    QListViewItem::paintCell(p, cg, column, width, align);
}
