/***************************************************************************
                          kmymoneytable.h  -  description
                             -------------------
    begin                : Wed Feb 21 2001
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

#ifndef KMYMONEYTABLE_H
#define KMYMONEYTABLE_H

#include <qwidget.h>
#include <qtable.h>

/**
  *@author Michael Edwardes
  */

class kMyMoneyTable : public QTable  {
   Q_OBJECT
private:
  QString m_orig;

protected:
  void paintEmptyArea(QPainter *p, int cx, int cy, int cw, int ch);
  QWidget* beginEdit(int row, int col, bool replace);

public:
	kMyMoneyTable(QWidget *parent=0, const char *name=0);
	~kMyMoneyTable();
	QString cellEditedOriginalText(void) { return m_orig; }
};

#endif
