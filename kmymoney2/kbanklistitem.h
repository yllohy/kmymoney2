/***************************************************************************
                          kbanklistitem.h
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

#ifndef KBANKLISTITEM_H
#define KBANKLISTITEM_H

#include <qwidget.h>
#include <qlistview.h>
#include "./mymoney/mymoneybank.h"
#include "./mymoney/mymoneyaccount.h"

/**
  *@author Michael Edwardes
  */

class KBankListItem : public QListViewItem  {
  MyMoneyBank m_bank;
  MyMoneyAccount m_account;
  bool m_isBank;
public:
  KBankListItem(QListView *parent, MyMoneyBank bank );
  KBankListItem(KBankListItem *parent, MyMoneyBank bank, MyMoneyAccount account);
	~KBankListItem();
	MyMoneyBank bank(void);
	MyMoneyAccount account(void);
	bool isBank(void);
  void paintCell(QPainter *p, const QColorGroup & cg, int column, int width, int align);
};

#endif
