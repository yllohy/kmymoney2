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

// ----------------------------------------------------------------------------
// QT Includes

#include <qglobal.h>

#if QT_VERSION > 300
#include <qpainter.h>
#endif

// ----------------------------------------------------------------------------
// KDE Includes

#include <kconfig.h>
#include <kglobal.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneysplittable.h"
#include "../kmymoneyutils.h"

kMyMoneySplitTable::kMyMoneySplitTable(QWidget *parent, const char *name ) :
  QTable(parent,name),
  m_currentRow(0),
  m_maxRows(0),
  m_inlineEditMode(false)
{
  setVScrollBarMode(QScrollView::AlwaysOn);
  // never show a horizontal scroll bar
  setHScrollBarMode(QScrollView::AlwaysOff);

  for(int i = 0; i < 3; ++i)
    m_colWidget[i] = 0;
}

kMyMoneySplitTable::~kMyMoneySplitTable()
{
}

void kMyMoneySplitTable::paintCell(QPainter *p, int row, int col, const QRect& r, bool /*selected*/)
{
  KConfig *config = KGlobal::config();
  config->setGroup("List Options");

  QFont defaultFont = QFont("helvetica", 12);
  QColor defaultColor = KMyMoneyUtils::defaultListColour();
  QColor defaultBGColor = KMyMoneyUtils::defaultBackgroundColour();
  QColor defaultGridColor = KMyMoneyUtils::defaultGridColour();
	
  const bool bShowGrid = config->readBoolEntry("ShowGrid", true);

  QColorGroup g = colorGroup();
  QColor textColor;

  if (row%2)
    g.setColor(QColorGroup::Base, config->readColorEntry("listColor", &defaultColor));
  else
    g.setColor(QColorGroup::Base, config->readColorEntry("listBGColor", &defaultBGColor));

  defaultGridColor = config->readColorEntry("listGridColor", &defaultGridColor);

  p->setFont(config->readFontEntry("listCellFont", &defaultFont));

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


  if(row == m_currentRow) {
    QBrush backgroundBrush(g.highlight());
    textColor = g.highlightedText();
    p->fillRect(rr,backgroundBrush);

  } else {
    QBrush backgroundBrush(g.base());
    textColor = g.text();
    p->fillRect(rr,backgroundBrush);
  }

  if (bShowGrid) {
    p->setPen(defaultGridColor);
    if(col != 0)
      p->drawLine(rr.x(), 0, rr.x(), rr.height()-1);    // left frame
    p->drawLine(rr.x(), rr.y(), rr.width(), 0);         // bottom frame
    p->setPen(textColor);
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
void kMyMoneySplitTable::paintFocus(QPainter * /* p */, const QRect & /*cr*/)
{
}

void kMyMoneySplitTable::columnWidthChanged(int col)
{
  for (int i=0; i<numRows(); i++)
    updateCell(i, col);
}

void kMyMoneySplitTable::endEdit(int row, int col, bool accept, bool replace )
{
  if(m_inlineEditMode) {
    QTable::endEdit(row, col, accept, replace);
    emit signalCancelEdit(m_key);
  }
}


bool kMyMoneySplitTable::eventFilter(QObject *o, QEvent *e)
{
  bool rc = false;

  if(e->type() == QEvent::KeyPress && !m_inlineEditMode) {
    QKeyEvent *k = static_cast<QKeyEvent *> (e);
    rc = true;
    switch(k->key()) {
      case Qt::Key_Up:
      case Qt::Key_Down:
      case Qt::Key_Home:
      case Qt::Key_End:
        emit signalNavigationKey(k->key());
        break;

      case Qt::Key_Return:
      case Qt::Key_Enter:
        emit signalEnter();
        break;

      case Qt::Key_Escape:
        emit signalEsc();
        break;

      case Qt::Key_Delete:
        emit signalDelete();
        break;

      case Qt::Key_Tab:
        if((k->state() & Qt::ShiftButton) == 0) {
          signalTab();
        }
        break;

      default:
        rc = false;
        break;
    }
  }

  // if the event has not been processed here, forward it to
  // the base class implementation
  if(rc == false) {
    if(e->type() == QEvent::KeyPress
    || e->type() == QEvent::KeyRelease) {
      m_key = (static_cast<QKeyEvent *> (e))->key();
      rc = false;
    } else
      rc = QTable::eventFilter(o, e);
  }

  return rc;
}


void kMyMoneySplitTable::contentsMousePressEvent( QMouseEvent* e )
{
  emit clicked( rowAt(e->pos().y()), columnAt(e->pos().x()), e->button(), e->pos() );
}

void kMyMoneySplitTable::contentsMouseReleaseEvent( QMouseEvent* /* e */ )
{
}

void kMyMoneySplitTable::contentsMouseDoubleClickEvent( QMouseEvent *e )
{
  emit doubleClicked(rowAt(e->pos().y()), columnAt(e->pos().x()), e->button(), e->pos() );
}

void kMyMoneySplitTable::setCurrentCell(int row, int col)
{
  if(row > m_maxRows)
    row = m_maxRows;
  QTable::setCurrentCell(row, col);
}

void kMyMoneySplitTable::setCurrentRow(int row)
{
  m_currentRow = row;
  setCurrentCell(row, 0);
}

void kMyMoneySplitTable::setMaxRows(int row)
{
  m_maxRows = row;
}

void kMyMoneySplitTable::setInlineEditingMode(const bool editing)
{
  m_inlineEditMode = editing;
}

void kMyMoneySplitTable::setNumRows(int irows)
{
  QTable::setNumRows(irows);

  KConfig *config = KGlobal::config();
  config->setGroup("List Options");
  QFont font = QFont("helvetica", 12);
  font = config->readFontEntry("listCellFont", &font);
  QFontMetrics fm( font );
  int height = fm.lineSpacing()+2;

  verticalHeader()->setUpdatesEnabled(false);

  for(int i = 0; i < irows; ++i)
    verticalHeader()->resizeSection(i, height);

  verticalHeader()->setUpdatesEnabled(true);

  // add or remove scrollbars as required
  updateScrollBars();
}

