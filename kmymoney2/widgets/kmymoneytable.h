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
  QWidget* m_col11;
  QWidget* m_col12;
  QWidget* m_col13;
  int m_button;
  QPoint m_point;
  QString m_qstringSecondItem;
  int m_nLastRow;
  int m_rowOffset;
  int m_currentDateRow;

protected:
  void paintCell(QPainter *p, int row, int col, const QRect& r, bool selected);
  void paintEmptyArea(QPainter *p, int cx, int cy, int cw, int ch);
  QWidget* beginEdit(int row, int col, bool replace);

public:
	kMyMoneyTable(QWidget *parent=0, const char *name=0);
	~kMyMoneyTable();

  void insertWidget(int row, int col, QWidget* w);
  QWidget* cellWidget(int row,int col) const;
	void clearCellWidget(int row, int col);
  QWidget* createEditor(int row, int col, bool initFromCell) const;

  /** Override the QTable member function to avoid display of focus */
  void paintFocus(QPainter *p, const QRect &cr);

  void setRowOffset(int row);
  void setCurrentDateRow(int row);

	QString cellEditedOriginalText(void) { return m_orig; }
	
	void setItem2(const QString& text) { m_qstringSecondItem=text; }

  QSize sizeHint(void) const;

public slots:
  /** No descriptions */
  virtual void setCurrentCell(int row, int col);

protected slots:
	virtual void columnWidthChanged(int col);
  /** No descriptions */
  bool eventFilter(QObject *o, QEvent *e);

};

#endif
