/***************************************************************************
                          kreconciledtableitem.h  -  description
                             -------------------
    begin                : Tue Feb 20 2001
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

#ifndef KRECONCILEDTABLEITEM_H
#define KRECONCILEDTABLEITEM_H

#include <qtable.h>

/**
  *@author Michael Edwardes
  */

class KReconciledTableItem : public QTableItem  {
public:
	KReconciledTableItem(QTable *t, EditType et, const QString &txt);
//  QWidget *createEditor() const;
//  void setContentFromEditor(QWidget *w);
  void setText(const QString &s);
  void paint(QPainter *p, const QColorGroup &cg, const QRect &cr, bool selected);
};

#endif
