/***************************************************************************
                          kdatetableitem.h  -  description
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

#ifndef KDATETABLEITEM_H
#define KDATETABLEITEM_H

#include <qtable.h>
#include <qdatetime.h>
#include "widgets/kmymoneydateinput.h"

/**
  *@author Michael Edwardes
  */

class KDateTableItem : public QTableItem  {
  kMyMoneyDateInput *date_edit;
  QDate m_date;

public: 
	KDateTableItem(QTable *t, EditType et, const QString &txt);
//  QWidget *createEditor() const;
//  void setContentFromEditor(QWidget *w);
  void setText(const QString &s);
  QDate date(void);
  void paint(QPainter *p, const QColorGroup &cg, const QRect &cr, bool selected);
};

#endif
