/***************************************************************************
                          kmethodtableitem.h  -  description
                             -------------------
    begin                : Tue Jan 23 2001
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

#ifndef KMETHODTABLEITEM_H
#define KMETHODTABLEITEM_H

#include <qtable.h>
#include <qcombobox.h>
#include "mymoney/mymoneytransaction.h"

/**
  *@author Michael Edwardes
  */

class KMethodTableItem : public QTableItem  {
  QComboBox *method_cb;
  MyMoneyTransaction::transactionMethod m_method;

public:
	KMethodTableItem(QTable *t, EditType et, const QString &txt);
  QWidget *createEditor() const;
  void setContentFromEditor(QWidget *w);
  void setText(const QString &s);
  MyMoneyTransaction::transactionMethod method(void);
  void paint(QPainter *p, const QColorGroup &cg, const QRect &cr, bool selected);
};

#endif
