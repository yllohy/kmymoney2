/***************************************************************************
                          knumbertableitem.h  -  description
                             -------------------
    begin                : Fri Jan 26 2001
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

#ifndef KNUMBERTABLEITEM_H
#define KNUMBERTABLEITEM_H

#include <qtable.h>
#include <qlineedit.h>

/**
  *@author Michael Edwardes
  */

class KNumberTableItem : public QTableItem  {
  QLineEdit *number_edit;

public:
	KNumberTableItem(QTable *t, EditType et, const QString &txt);
//  QWidget *createEditor() const;
//  void setContentFromEditor(QWidget *w);
  void setText(const QString& s);
  void paint(QPainter *p, const QColorGroup &cg, const QRect &cr, bool selected);
};

#endif
