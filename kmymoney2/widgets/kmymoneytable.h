/***************************************************************************
                          kmymoneytable.h  -  description
                             -------------------
    begin                : Wed Feb 21 2001
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
  QWidget* m_col0;
  QWidget* m_col1;
  QWidget* m_col2;
  QWidget* m_col3;
  QWidget* m_col4;
  QWidget* m_col5;
  QWidget* m_col6;
  QWidget* m_col7;
  QWidget* m_col8;
  QWidget* m_col9;
  QWidget* m_col10;

protected:
  void paintEmptyArea(QPainter *p, int cx, int cy, int cw, int ch);
  QWidget* beginEdit(int row, int col, bool replace);

public:
	kMyMoneyTable(QWidget *parent=0, const char *name=0);
	~kMyMoneyTable();

  void insertWidget(int row, int col, QWidget* w);
  QWidget* cellWidget(int row,int col) const;
	void clearCellWidget(int row, int col);


	QString cellEditedOriginalText(void) { return m_orig; }

protected slots:
	virtual void columnWidthChanged(int col);

};

#endif
