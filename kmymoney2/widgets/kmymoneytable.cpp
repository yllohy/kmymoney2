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

#if QT_VERSION > 300
#include <qpainter.h>
#endif

kMyMoneyTable::kMyMoneyTable(QWidget *parent, const char *name )
 : QTable(parent,name)
{
  KConfig *config = KGlobal::config();
  config->setGroup("List Options");
  QColor defaultColor = Qt::white;
	
  QPalette pal = palette();
  pal.setColor(QColorGroup::Base, config->readColorEntry("listColor", &defaultColor));
  setPalette(pal);

	setFocusPolicy(QWidget::NoFocus);
  setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred));

	m_nLastRow = 0;
  m_rowOffset = 0;
  m_currentDateRow = 0;

  // never show a horizontal scroll bar
  setHScrollBarMode(QScrollView::AlwaysOff);
}

kMyMoneyTable::~kMyMoneyTable()
{
}

void kMyMoneyTable::paintEmptyArea(QPainter *p, int cx, int cy, int cw, int ch)
{
  // calling the base class version of paintEmptyArea here causes
  // some problems in the middle of a large table. I have no idea
  // why, but not calling the base class function solves the problem.
  // ipwizard@users.sourceforge.net   12/11/2001

  // QTable::paintEmptyArea(p, cx, cy, cw, ch);
}

QWidget* kMyMoneyTable::beginEdit(int row, int col, bool replace)
{
  m_orig = text(row, col);
  return QTable::beginEdit(row, col, replace);

}

QWidget* kMyMoneyTable::createEditor(int row, int col, bool replace) const
{
  switch(((row - m_rowOffset) % 2) * numCols() + col) {
    case 0:   // date
    case 1:   // type
    case 2:   // payee
    case 4:   // payment
    case 5:   // deposit
    case 8:   // number
    case 9:   // category
      return QTable::createEditor(row, col, replace);
      break;
    case 3:   // reconcile-flag
    case 6:   // balance
    case 7:   // empty
    case 10:  // empty split?
    case 11:  // enter
    case 12:  // cancel
    case 13:  // delete
      break;
    default:
      qDebug("Unknown widget for KMyMoneyTable::createEditor in row %d and col %d", row, col);
      break;
  }
  return NULL;
}

void kMyMoneyTable::insertWidget(int row, int col, QWidget* w)
{
  switch(((row - m_rowOffset) % 2) * numCols() + col) {
    case 0:   // date
      m_col0 = w;
      break;
    case 1:   // type
      m_col1 = w;
      break;
    case 2:   // payee
      m_col2 = w;
      break;
    case 3:   // reconcile-flag
      m_col3 = w;
      break;
    case 4:   // payment
      m_col4 = w;
      break;
    case 5:   // deposit
      m_col5 = w;
      break;
    case 6:   // balance
      m_col6 = w;
      break;
    case 7:   // empty
      m_col7 = w;
      break;
    case 8:   // number
      m_col8 = w;
      break;
    case 9:   // category
      m_col9 = w;
      break;
    case 10:  // empty
      m_col10 = w;
      break;
    case 11:  // enter
      m_col11 = w;
      break;
    case 12:  // cancel
      m_col12 = w;
      break;
    case 13:  // delete
      m_col13 = w;
      break;
    default:
      qDebug("Unknown widget for KMyMoneyTable::insertWidget in row %d and col %d", row, col);
      break;
  }
}

void kMyMoneyTable::columnWidthChanged(int col)
{
  for (int i=0; i<numRows(); i++)
    updateCell(i, col);
}

QWidget* kMyMoneyTable::cellWidget(int row, int col)const
{
  if(row >= 0 && col >= 0) {
    switch(((row - m_rowOffset) % 2) * numCols() + col) {
      case 0:
        return m_col0;
        break;
      case 1:
        return m_col1;
        break;
      case 2:
        return m_col2;
        break;
      case 3:
        return m_col3;
        break;
      case 4:
        return m_col4;
        break;
      case 5:
        return m_col5;
        break;
      case 6:
        return m_col6;
        break;
      case 7:
        return m_col7;
        break;
      case 8:
        return m_col8;
        break;
      case 9:
        return m_col9;
        break;
      case 10:
        return m_col10;
        break;
      case 11:
        return m_col11;
        break;
      case 12:
        return m_col12;
        break;
      case 13:
        return m_col13;
        break;
      default:
        qDebug("Unknown widget for KMyMoneyTable::cellWidget in row %d and col %d", row, col);
        break;
    }
  }
  return (QWidget*)0;
}

void kMyMoneyTable::clearCellWidget(int /*row*/, int /*col*/)
{
}

/** No descriptions */
void kMyMoneyTable::setCurrentCell(int row, int col){

	clicked(row,col,m_button,m_point);
}
/** No descriptions */
bool kMyMoneyTable::eventFilter(QObject *o, QEvent *e)
{
  if(e->type() == QEvent::MouseButtonPress) {
    QMouseEvent *m = (QMouseEvent *) e ;
    m_point = m->pos();
    m_button = m->button();
  }
  return QTable::eventFilter(o,e);
}

void kMyMoneyTable::paintCell(QPainter *p, int row, int col, const QRect& r, bool /*selected*/)
{
  KConfig *config = KGlobal::config();
  config->setGroup("List Options");

  QFont defaultFont = QFont("helvetica", 12);
  QColor defaultColor = Qt::white;
  QColor defaultBGColor = Qt::gray;
  QColor defaultGridColor = Qt::black;
	
  const int NO_ROWS = (config->readEntry("RowCount", "2").toInt());
  const bool bShowGrid = config->readBoolEntry("ShowGrid", true);
  const bool bColourPerTransaction = config->readBoolEntry("ColourPerTransaction", true);

  QColorGroup g = colorGroup();
  if (bColourPerTransaction) {
    if (row< (NO_ROWS-1))
      g.setColor(QColorGroup::Base, config->readColorEntry("listBGColor", &defaultBGColor));
    else if ((row/NO_ROWS)%2)
      g.setColor(QColorGroup::Base, config->readColorEntry("listColor", &defaultColor));
    else
      g.setColor(QColorGroup::Base, config->readColorEntry("listBGColor", &defaultBGColor));
  } else {
    if (row%2)
      g.setColor(QColorGroup::Base, config->readColorEntry("listColor", &defaultColor));
    else
      g.setColor(QColorGroup::Base, config->readColorEntry("listBGColor", &defaultBGColor));
  }

  defaultGridColor = config->readColorEntry("listGridColor", &defaultGridColor);

  p->setFont(config->readFontEntry("listCellFont", &defaultFont));
  bool bLastTransaction = (row >= numRows()-2);
  if (bLastTransaction && config->readBoolEntry("TextPrompt", true)) {
    QFont qfontItalic = p->font();
//    qfontItalic.setPointSize(qfontItalic.pointSize()-2);
    qfontItalic.setWeight(QFont::Light);
    qfontItalic.setItalic(true);
    p->setFont(qfontItalic);
  }
	
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
  QRect rr3 = r;
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
  switch (col) {
    case 0:
    case 1:
    case 2:
      if (bShowGrid) {
        p->setPen(defaultGridColor);
        p->drawLine(rr.x(), 0, rr.x(), rr.height()-1);
        p->drawLine(rr.x(), rr.y(), rr.width(), 0);
        p->setPen(g.foreground());
      }
      if(intPos > -1) {
        int intMemoStart = rr.width() / 2;
        rr3.setX(2);
        rr3.setY(0);
        rr3.setWidth(intMemoStart-4);
        rr3.setHeight(rowHeight(row));
        p->drawText(rr3,Qt::AlignLeft | Qt::AlignVCenter,qstringCategory);
        if(bShowGrid) {
          p->setPen(defaultGridColor);
          p->drawLine(intMemoStart,0,intMemoStart,rr.height()-1);
          p->setPen(g.foreground());
        }
        rr3.setX(intMemoStart + 2);
        rr3.setWidth(intMemoStart-4);
        p->drawText(rr3,Qt::AlignLeft | Qt::AlignVCenter,qstringMemo);

      } else {
        p->drawText(rr2, Qt::AlignLeft | Qt::AlignVCenter,firsttext);
      }
      if(row == m_currentDateRow) {
        p->setPen(defaultGridColor);
        p->drawLine(rr.x(), 1, rr.width(), 1);
        p->setPen(g.foreground());
      }
      break;
    case 3:
      if (bShowGrid) {
        p->setPen(defaultGridColor);
        p->drawLine(rr.x(), 0, rr.x(), rr.height()-1);
        p->drawLine(rr.x(), rr.y(), rr.width(), 0);
        p->setPen(g.foreground());
      }
      p->drawText(rr2, Qt::AlignCenter | Qt::AlignVCenter,firsttext);
      if(row == m_currentDateRow) {
        p->setPen(defaultGridColor);
        p->drawLine(rr.x(), 1, rr.width(), 1);
        p->setPen(g.foreground());
      }
      break;
    case 4:
    case 5:
    case 6:
      if (bShowGrid) {
        p->setPen(defaultGridColor);
        p->drawLine(rr.x(), 0, rr.x(), rr.height()-1);
        p->drawLine(rr.x(), rr.y(), rr.width(), 0);
        p->drawLine(rr.x()+rr.width(), 0, rr.x()+rr.width(), rr.height()-1);
        p->setPen(g.foreground());
      }
      p->drawText(rr2, Qt::AlignRight | Qt::AlignVCenter,firsttext);
      if(row == m_currentDateRow) {
        p->setPen(defaultGridColor);
        p->drawLine(rr.x(), 1, rr.width(), 1);
        p->setPen(g.foreground());
      }
      break;
  }
}

void kMyMoneyTable::setCurrentDateRow(int row)
{
  m_currentDateRow = row;
}

void kMyMoneyTable::setRowOffset(int row)
{
  m_rowOffset = row;
}
/** Override the QTable member function to avoid display of focus */
void kMyMoneyTable::paintFocus(QPainter *p, const QRect &cr)
{
}

QSize kMyMoneyTable::sizeHint(void) const
{
  return QSize(760, 320);
}
