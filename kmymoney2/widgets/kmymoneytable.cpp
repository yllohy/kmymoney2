/***************************************************************************
                          kmymoneytable.cpp  -  description
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
#include <qpalette.h>
#include "kmymoneytable.h"
#include "../kmymoneysettings.h"

kMyMoneyTable::kMyMoneyTable(QWidget *parent, const char *name )
 : QTable(parent,name)
{
	setFocusPolicy(QWidget::NoFocus);
}

kMyMoneyTable::~kMyMoneyTable()
{
}

void kMyMoneyTable::paintEmptyArea(QPainter *p, int cx, int cy, int cw, int ch)
{
  KMyMoneySettings *p_settings = KMyMoneySettings::singleton();
  if (p_settings) {
    QPalette pal = palette();
    pal.setColor(QColorGroup::Base, p_settings->lists_color());
    setPalette(pal);
  }

  QTable::paintEmptyArea(p, cx, cy, cw, ch);
}

QWidget* kMyMoneyTable::beginEdit(int row, int col, bool replace)
{
  m_orig = text(row, col);
  return QTable::beginEdit(row, col, replace);

}

void kMyMoneyTable::insertWidget(int row,int col, QWidget* w)
{
  if(col == 0)
    m_col0 = w;
  if(col == 1)
    m_col1 = w;
  if(col == 2)
    m_col2 = w;
  if(col == 3)
    m_col3 = w;
  if(col == 4)
    m_col4 = w;
  if(col == 5)
    m_col5 = w;
  if(col == 6)
    m_col6 = w;
  if(col == 7)
    m_col7 = w;
  if(col == 8)
    m_col8 = w;
  if(col == 9)
    m_col9 = w;
  if(col == 10)
    m_col10 = w;

}

void kMyMoneyTable::columnWidthChanged(int col)
{

	

}


QWidget* kMyMoneyTable::cellWidget(int row, int col)const
{
  if(col == 0)
    return m_col0;
  if(col == 1)
    return m_col1;
  if(col == 2)
    return m_col2;
  if(col == 3)
    return m_col3;
  if(col == 4)
    return m_col4;
  if(col == 5)
    return m_col5;
  if(col == 6)
    return m_col6;
  if(col == 7)
    return m_col7;
  if(col == 8)
    return m_col8;
  if(col == 9)
    return m_col9;
  if(col == 10)
    return m_col10;

}

void kMyMoneyTable::clearCellWidget(int row, int col)
{

}

/** No descriptions */
void kMyMoneyTable::setCurrentCell(int row, int col){

	clicked(row,col,m_button,m_point);
}
/** No descriptions */
bool kMyMoneyTable::eventFilter(QObject *o, QEvent *e){

	if(e->type() == QEvent::MouseButtonPress)
	{
     	QMouseEvent *m = (QMouseEvent *) e ;
		m_point = m->pos();
		m_button = m->button();
	}
	return QTable::eventFilter(o,e);
}
