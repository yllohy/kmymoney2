/***************************************************************************
                          ktransactiontableitem.h  -  description
                             -------------------
    begin                : Tue Jan 23 2001
    copyright            : (C) 2001 by Michael Edwardes
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

#ifndef KTRANSACTIONTABLEITEM_H
#define KTRANSACTIONTABLEITEM_H

#include <qtable.h>
#include <qrect.h>
#include <qcombobox.h>
#include "mymoney/mymoneyfile.h"
#include "widgets/kmymoneydateinput.h"

/**
  *@author Michael Edwardes
  */

class KTransactionTableItem : public QTableItem  {
public:
  enum types { NoType, MethodType, DateType, NumberType, MemoType, CategoryType };

private:
  types m_type;
  MyMoneyFile *m_file;

  QComboBox *method_cb;
  kMyMoneyDateInput *date_edit;

public:
	KTransactionTableItem(QTable *t, EditType et, const QString &txt, types type, MyMoneyFile *file);
	KTransactionTableItem(QTable *t, EditType et, const QString &txt);
//	void paint(QPainter *p, const QColorGroup &cg, const QRect &cr, bool selected);
  QWidget *createEditor() const;
  void setContentFromEditor(QWidget *w);
  void setText(const QString &s);
};

#endif
