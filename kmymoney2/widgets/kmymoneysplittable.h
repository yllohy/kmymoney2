/***************************************************************************
                          kmymoneysplittable.h  -  description
                             -------------------
    begin                : Thu Jan 10 2002
    copyright            : (C) 2000-2002 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KMYMONEYSPLITTABLE_H
#define KMYMONEYSPLITTABLE_H

#include <qwidget.h>
#include <qtable.h>

/**
  *@author Thomas Baumgart
  */

class kMyMoneySplitTable : public QTable  {
   Q_OBJECT
public: 
	kMyMoneySplitTable(QWidget *parent=0, const char *name=0);
	~kMyMoneySplitTable();

  void paintCell(QPainter *p, int row, int col, const QRect& r, bool /*selected*/);
  void paintFocus(QPainter *p, const QRect &cr);

  QWidget* createEditor(int row, int col, bool initFromCell) const;
	void clearCellWidget(int row, int col);
  void insertWidget(int row, int col, QWidget* w);
  QWidget* cellWidget(int row,int col) const;

  void setCurrentRow(int row);
  int currentRow(void) { return m_currentRow; }
  void setMaxRows(int row);

protected:
  void contentsMousePressEvent( QMouseEvent* e );
  void contentsMouseReleaseEvent( QMouseEvent* e );

public slots:
  /** No descriptions */
  virtual void setCurrentCell(int row, int col);

private:
  // array to be used to access the input widgets
  QWidget* m_colWidget[3];

  // point where mouse event happened
  QPoint m_mousePoint;

  // button of mouse that caused the event
  int m_mouseButton;

  // the currently selected row (will be printed as selected)
  int m_currentRow;

  // the number of rows filled with data
  int m_maxRows;

protected slots:
	virtual void columnWidthChanged(int col);
  bool eventFilter(QObject *o, QEvent *e);
};

#endif
