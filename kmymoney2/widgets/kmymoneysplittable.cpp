/***************************************************************************
                          kmymoneysplittable.cpp  -  description
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

#include <kconfig.h>
#include <kglobal.h>

#include "kmymoneysplittable.h"

kMyMoneySplitTable::kMyMoneySplitTable(QWidget *parent, const char *name )
 : QTable(parent,name)
{
  setVScrollBarMode(QScrollView::AlwaysOn);
  // never show a horizontal scroll bar
  setHScrollBarMode(QScrollView::AlwaysOff);
}

kMyMoneySplitTable::~kMyMoneySplitTable(){
}

void kMyMoneySplitTable::paintCell(QPainter *p, int row, int col, const QRect& r, bool /*selected*/)
{
  KConfig *config = KGlobal::config();
  config->setGroup("List Options");

  QFont defaultFont = QFont("helvetica", 12);
  QColor defaultColor = Qt::white;
  QColor defaultBGColor = Qt::gray;
  QColor defaultGridColor = Qt::black;
	
  const bool bShowGrid = config->readBoolEntry("ShowGrid", true);

  QColorGroup g = colorGroup();
  if (row%2)
    g.setColor(QColorGroup::Base, config->readColorEntry("listColor", &defaultColor));
  else
    g.setColor(QColorGroup::Base, config->readColorEntry("listBGColor", &defaultBGColor));

  defaultGridColor = config->readColorEntry("listGridColor", &defaultGridColor);

  p->setFont(config->readFontEntry("listCellFont", &defaultFont));

#if 0
  bool bLastTransaction = (row >= numRows()-2);
  if (bLastTransaction && config->readBoolEntry("TextPrompt", true)) {
    QFont qfontItalic = p->font();
//    qfontItalic.setPointSize(qfontItalic.pointSize()-2);
    qfontItalic.setWeight(QFont::Light);
    qfontItalic.setItalic(true);
    p->setFont(qfontItalic);
  }
#endif
	
  QString firsttext = text(row, col);
  QString qstringCategory;
  QString qstringMemo;

  int intPos = firsttext.find("|");
  if(intPos > -1)
  {
    qstringCategory = firsttext.left(intPos);
    qstringMemo = firsttext.mid(intPos + 1);
  }

  QRect rr = r;
  QRect rr2 = r;
  rr.setX(0);
  rr.setY(0);
  rr.setWidth(columnWidth(col));
  rr.setHeight(rowHeight(row));

  rr2.setX(2);
  rr2.setY(0);
  rr2.setWidth(columnWidth(col)-4);
  rr2.setHeight(rowHeight(row));

  QBrush backgroundBrush(g.base());
  p->setPen(g.foreground());
  p->fillRect(rr,backgroundBrush);

  if (bShowGrid) {
    p->setPen(defaultGridColor);
    p->drawLine(rr.x(), 0, rr.x(), rr.height()-1);    // left frame
    p->drawLine(rr.x(), rr.y(), rr.width(), 0);       // bottom frame
    p->setPen(g.foreground());
  }

  switch (col) {
    case 0:     // category
    case 1:     // memo
      p->drawText(rr2, Qt::AlignLeft | Qt::AlignVCenter, text(row, col));
      break;

    case 2:     // amount
      p->drawText(rr2, Qt::AlignRight | Qt::AlignVCenter,firsttext);
      break;
  }
}

/** Override the QTable member function to avoid display of focus */
void kMyMoneySplitTable::paintFocus(QPainter *p, const QRect &cr)
{
}

void kMyMoneySplitTable::columnWidthChanged(int col)
{
  for (int i=0; i<numRows(); i++)
    updateCell(i, col);
}

QWidget* kMyMoneySplitTable::createEditor(int row, int col, bool replace) const
{
  switch(col) {
    case 0:   // category
    case 1:   // memo
    case 2:   // amount
      return QTable::createEditor(row, col, replace);
      break;
    default:
      qDebug("Unknown widget for KMyMoneySplitTable::createEditor in row %d and col %d", row, col);
      break;
  }
  return NULL;
}

void kMyMoneySplitTable::clearCellWidget(int /*row*/, int /*col*/)
{
}

QWidget* kMyMoneySplitTable::cellWidget(int row, int col)const
{
  if(col >= 0 && col <= 2)
    return m_colWidget[col];

  qDebug("Unknown widget for KMyMoneySplitTable::cellWidget in row %d and col %d", row, col);
  return (QWidget*)0;
}

void kMyMoneySplitTable::insertWidget(int row, int col, QWidget* w)
{
  if(col >= 0 && col <= 2)
    m_colWidget[col] = w;
  else
    qDebug("Unknown widget for KMyMoneySplitTable::insertWidget in row %d and col %d", row, col);
}

bool kMyMoneySplitTable::eventFilter(QObject *o, QEvent *e)
{
  if(e->type() == QEvent::MouseButtonPress) {
    QMouseEvent *m = (QMouseEvent *) e ;
    m_mousePoint = m->pos();
    m_mouseButton = m->button();
  }
  return QTable::eventFilter(o,e);
}

void kMyMoneySplitTable::setCurrentCell(int row, int col)
{
	clicked(row,col,m_mouseButton,m_mousePoint);
}
