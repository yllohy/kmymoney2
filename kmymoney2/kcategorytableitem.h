/***************************************************************************
                          kcategorytableitem.h  -  description
                             -------------------
    begin                : Wed Jan 31 2001
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

#ifndef KCATEGORYTABLEITEM_H
#define KCATEGORYTABLEITEM_H

#include <qtable.h>
#include <qcombobox.h>
#include "mymoney/mymoneyfile.h"
#include "mymoney/mymoneytransaction.h"
#include "mymoney/mymoneycategory.h"

/**
  *@author Michael Edwardes
  */

class KCategoryTableItem : public QTableItem  {
  QComboBox *category_cb;
  QStringList categoryList;
  MyMoneyCategory m_category;

public:
	KCategoryTableItem(QTable *t, EditType et, const QString& txt, MyMoneyFile *file);
//  QWidget *createEditor() const;
//  void setContentFromEditor(QWidget *w);
  void setText(const QString &s);
  MyMoneyCategory category(void) { return m_category; }
  void paint(QPainter *p, const QColorGroup &cg, const QRect &cr, bool selected);
};

#endif
