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
#include <kglobal.h>
#include <kconfig.h>
#include <qpalette.h>
#include "kmymoneytable.h"

kMyMoneyTable::kMyMoneyTable(QWidget *parent, const char *name )
 : QTable(parent,name)
{
	setFocusPolicy(QWidget::NoFocus);
	m_nLastRow = 0;
}

kMyMoneyTable::~kMyMoneyTable()
{
}

void kMyMoneyTable::paintEmptyArea(QPainter *p, int cx, int cy, int cw, int ch)
{
  KConfig *config = KGlobal::config();
  config->setGroup("List Options");
  QColor defaultColor = Qt::white;
	
  QPalette pal = palette();
  pal.setColor(QColorGroup::Base, config->readColorEntry("listColor", &defaultColor));
  setPalette(pal);

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

void kMyMoneyTable::paintCell(QPainter *p, int row, int col, const QRect& r, bool selected)
{
  KConfig *config = KGlobal::config();
  config->setGroup("List Options");

  QFont defaultFont = QFont("helvetica", 12);
  QColor defaultColor = Qt::white;
  QColor defaultBGColor = Qt::gray;
	
  const int NO_ROWS = (config->readEntry("RowCount", "2").toInt());
  const bool showgrid = config->readBoolEntry("ShowGrid", true);

  QColorGroup g = colorGroup();
  // Paint two lines at a time the same colour
      if (row< (NO_ROWS-1))
        g.setColor(QColorGroup::Base, config->readColorEntry("listBGColor", &defaultBGColor));
      else if ((row/NO_ROWS)%2)
        g.setColor(QColorGroup::Base, config->readColorEntry("listColor", &defaultColor));
      else
        g.setColor(QColorGroup::Base, config->readColorEntry("listBGColor", &defaultBGColor));

  p->setFont(config->readFontEntry("listCellFont", &defaultFont));
	
  QString firsttext = text(row, col);
//  QString secondtext = m_qstringSecondItem;
//  QRect myrect = r;
//  myrect.setX(0);
//  myrect.setY(0);
/* works but can't detect second text
  if (col==2 && (row%2)) {
		QRect arect = cellGeometry(row,col);
		QBrush backgroundBrush(g.base());
		p->setPen(g.foreground());
		p->fillRect(myrect,backgroundBrush);
		int newwidth = arect.width() / 2;
		//myrect.setWidth(newwidth);
		p->drawText(myrect,Qt::AlignLeft,firsttext);
		myrect.setX(newwidth);
		myrect.setWidth(newwidth);
		p->fillRect(myrect,backgroundBrush);
		p->drawLine(newwidth, 0,newwidth,myrect.height());
		myrect.setX(newwidth + 1);
		p->setPen(g.foreground());
		p->drawText(myrect,Qt::AlignLeft,secondtext);
  }
  else {
*/
  QRect rr = r;
  QRect rr2 = r;
  rr.setX(1);
  rr.setY(1);
  rr2.setX(0);
  rr2.setY(0);
  QBrush backgroundBrush(g.base());
  p->setPen(g.foreground());
  p->fillRect(rr,backgroundBrush);
  switch (col) {
    case 0:
    case 1:
    case 2:
      if (showgrid) {
        p->drawLine(rr.x()-1, 0, rr.x()-1, rr.height()-1);
        p->drawLine(rr.x()-1, rr.y()-1, rr.width()-1, 0);
        p->drawText(rr, Qt::AlignLeft,firsttext);
      } else
        p->drawText(rr2, Qt::AlignLeft,firsttext);
      break;
    case 3:
      if (showgrid) {
        p->drawLine(rr.x()-1, 0, rr.x()-1, rr.height()-1);
        p->drawLine(rr.x()-1, rr.y()-1, rr.width()-1, 0);
        p->drawText(rr, Qt::AlignLeft,firsttext);
      } else
        p->drawText(rr2, Qt::AlignLeft,firsttext);
      // why doesn't AlignCenter work ??
      break;
    case 4:
    case 5:
    case 6:
      if (showgrid) {
        p->drawLine(rr.x()-1, 0, rr.x()-1, rr.height()-1);
        p->drawLine(rr.x()-1, rr.y()-1, rr.width()-1, 0);
        p->drawLine(rr.x()-1+rr.width()-1, 0, rr.x()-1+rr.width()-1, rr.height()-1);
        p->drawText(rr, Qt::AlignLeft,firsttext);
      } else
        p->drawText(rr2, Qt::AlignLeft,firsttext);
      // why doesn't AlignRight work ??
      break;
  }
//  }
}
