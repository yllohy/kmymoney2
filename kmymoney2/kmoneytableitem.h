/***************************************************************************
                          kmoneytableitem.h  -  description
                             -------------------
    begin                : Mon Feb 12 2001
    copyright            : (C) 2001 by Michael Edwardes
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

#ifndef KMONEYTABLEITEM_H
#define KMONEYTABLEITEM_H

#include <qtable.h>
#include "widgets/kmymoneyedit.h"
#include "mymoney/mymoneymoney.h"

/**
  *@author Michael Edwardes
  */

class KMoneyTableItem : public QTableItem  {
  kMyMoneyEdit *money_edit;
  MyMoneyMoney m_money;

public:
	KMoneyTableItem(QTable *t, EditType et, const QString &txt);
//  QWidget *createEditor() const;
//  void setContentFromEditor(QWidget *w);
  void setText(const QString &s);
  MyMoneyMoney money(void) { return m_money; }
  void paint(QPainter *p, const QColorGroup &cg, const QRect &cr, bool selected);
};

#endif
