/***************************************************************************
                          ktransactiontableitem.h  -  description
                             -------------------
    begin                : Wed Apr 18 2001
    copyright            : (C) 2001 by Michael Edwardes
    email                : mte@users.sourceforge.net
                             Javier Campos Morales <javi_c@ctv.es>
                             Felix Rodriguez <frodriguez@mail.wesleyan.edu>
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

#include <qwidget.h>
#include <qtable.h>

/**
  *@author Michael Edwardes
  */

class KTransactionTableItem : public QTableItem  {
//   Q_OBJECT
public:
	KTransactionTableItem(QTable *t, EditType et, const QString &txt);
  void paint(QPainter *p, const QColorGroup &cg, const QRect &cr, bool selected);
};

#endif
