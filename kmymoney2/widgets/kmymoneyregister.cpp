/***************************************************************************
                          kmymoneyregister.cpp  -  description
                             -------------------
    begin                : Sun Jul 14 2002
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

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyregister.h"
#include "../views/kledgerview.h"

kMyMoneyRegister::kMyMoneyRegister(int maxRpt, QWidget *parent, const char *name )
  : QTable(parent, name),
    m_ledgerLens(true),
    m_maxRpt(maxRpt)
{
  readConfig();
  m_currentDateRow = -1;
  m_lastTransactionIndex = -1;
  m_currentTransactionIndex = 0;
  m_inlineEditMode = false;
  setSelectionMode(QTable::SingleRow);

  m_editWidgets.clear();
}

kMyMoneyRegister::~kMyMoneyRegister()
{
}

void kMyMoneyRegister::setNumRows(int r)
{
}

void kMyMoneyRegister::setTransactionCount(const int r, const bool setTransaction)
{
  //setUpdatesEnabled( false );

  int irows = r * m_rpt;

  if(m_ledgerLens == true) {
    irows = (r-1)*m_rpt + maxRpt();
  }
  QTable::setNumRows(irows);

  QFontMetrics fm( m_cellFont );
  int height = fm.lineSpacing()+6;

  verticalHeader()->setUpdatesEnabled(false);

  for(int i = 0; i < irows; ++i)
    verticalHeader()->resizeSection(i, height);

  verticalHeader()->setUpdatesEnabled(true);
  //setUpdatesEnabled( true );

  // add or remove scrollbars as required
  updateScrollBars();

  m_lastTransactionIndex = -1;
}

/** Override the QTable member function to avoid display of focus */
void kMyMoneyRegister::paintFocus(QPainter *p, const QRect &cr)
{
}

void kMyMoneyRegister::readConfig(void)
{
  KConfig *config = KGlobal::config();
  config->setGroup("List Options");

  m_cellFont = QFont("helvetica", 10);
  m_color = Qt::white;
  m_bgColor = Qt::gray;
  m_gridColor = Qt::black;

  m_bgColor = config->readColorEntry("listBGColor", &m_bgColor);
  m_color = config->readColorEntry("listColor", &m_color);
  m_gridColor = config->readColorEntry("listGridColor", &m_gridColor);

  m_cellFont = config->readFontEntry("listCellFont", &m_cellFont);
  setFont(m_cellFont);
  m_headerFont = config->readFontEntry("listHeaderFont", &m_headerFont);
  updateHeaders();

  // force loading of new font into all cells
  int rows = numRows();
  QTable::setNumRows(0);
  QTable::setNumRows(rows);

  // m_rpt = config->readEntry("RowCount", "1").toInt();
  config->deleteEntry("RowCount");
  if(config->readBoolEntry("ShowRegisterDetailed", false))
    m_rpt = m_maxRpt;
  else
    m_rpt = 1;

  m_showGrid = config->readBoolEntry("ShowGrid", true);
  m_colorPerTransaction = config->readBoolEntry("ColourPerTransaction", true);
}

void kMyMoneyRegister::paintCell(QPainter *p, int row, int col, const QRect& r,
                                 bool selected, const QColorGroup& cg)
{
  int firstRow,       // first row occupied by current transaction
      lastRow;        // last row occupied by current transaction
  firstRow = m_currentTransactionIndex * m_rpt;
  if(m_ledgerLens)
    lastRow = (m_currentTransactionIndex * m_rpt) + maxRpt() - 1;
  else
    lastRow = (m_currentTransactionIndex * m_rpt) + m_rpt - 1;

  m_cg = cg;

  if (m_colorPerTransaction) {
    int adj = 0;
    if(row > lastRow && m_ledgerLens) {
      adj = m_rpt - maxRpt();
    }
    if(((row + adj)/m_rpt)%2)
      m_cg.setColor(QColorGroup::Base, m_color);
    else
      m_cg.setColor(QColorGroup::Base, m_bgColor);

  } else {
    if (row%2)
      m_cg.setColor(QColorGroup::Base, m_color);
    else
      m_cg.setColor(QColorGroup::Base, m_bgColor);
  }

  p->setFont(m_cellFont);

  m_cellRect = r;
  m_cellRect.setX(0);
  m_cellRect.setY(0);
  m_cellRect.setWidth(columnWidth(col));
  m_cellRect.setHeight(rowHeight(row));

  m_textRect = r;
  m_textRect.setX(2);
  m_textRect.setY(0);
  m_textRect.setWidth(columnWidth(col)-4);
  m_textRect.setHeight(rowHeight(row));

  m_transactionIndex = row/m_rpt;
  m_transactionRow = row%m_rpt;

  if(m_ledgerLens) {
    if(row >= firstRow && row <= lastRow) {
      m_transactionIndex = m_currentTransactionIndex;
      m_transactionRow = row - firstRow;
    } else if(row > lastRow && m_ledgerLens) {
      m_transactionIndex = (row - maxRpt() + m_rpt) / m_rpt;
      m_transactionRow = (row - maxRpt() + m_rpt) % m_rpt;
    }
  }

  if(m_transactionIndex != m_lastTransactionIndex) {
    m_transaction = m_view->transaction(m_transactionIndex);
    if(m_transaction != NULL) {
      m_split = m_transaction->split(m_view->accountId());
      m_balance = m_view->balance(m_transactionIndex);
    }
    m_lastTransactionIndex = m_transactionIndex;
  }

  if(m_transactionIndex == m_currentTransactionIndex) {
    QBrush backgroundBrush(m_cg.highlight());
    p->fillRect(m_cellRect,backgroundBrush);
    m_textColor = m_cg.highlightedText();

  } else {
    QBrush backgroundBrush(m_cg.base());
    p->fillRect(m_cellRect,backgroundBrush);
    m_textColor = m_cg.text();
  }
  p->setPen(m_textColor);
}

void kMyMoneyRegister::contentsMouseDoubleClickEvent( QMouseEvent *e)
{
  int tmpRow = rowAt( e->pos().y() );
  int tmpCol = columnAt( e->pos().x() );

  emit doubleClicked( tmpRow, tmpCol, e->button(), e->pos() );
}

void kMyMoneyRegister::contentsMouseReleaseEvent( QMouseEvent* e )
{
  QTable::contentsMouseReleaseEvent(e);
/*
  if(e->button != LeftButton)
    int row = rowAt(e->pos().y()) / m_rpt;
    emit clicked( row, columnAt(e->pos().x()), e->button(), e->pos() );
  }
*/
}

bool kMyMoneyRegister::setCurrentTransactionRow(const int row)
{
  int firstRow,       // first row occupied by current transaction
      lastRow;        // last row occupied by current transaction
  int idx;
  firstRow = m_currentTransactionIndex * m_rpt;
  if(m_ledgerLens)
    lastRow = (m_currentTransactionIndex * m_rpt) + maxRpt() - 1;
  else
    lastRow = (m_currentTransactionIndex * m_rpt) + m_rpt - 1;

  idx = row/m_rpt;

  if(m_ledgerLens) {
    if(row >= firstRow && row <= lastRow) {
      idx = m_currentTransactionIndex;
    } else if(row > lastRow && m_ledgerLens) {
      idx = (row - maxRpt() + m_rpt) / m_rpt;
    }
  }
  return setCurrentTransactionIndex(idx);
}


bool kMyMoneyRegister::setCurrentTransactionIndex(int idx)
{
  if(idx < 0)
    idx = 0;

  bool rc = (idx != m_currentTransactionIndex);

  m_currentTransactionIndex = idx;
  setCurrentCell(idx * rpt(), 0);

  return rc;
}

QWidget* kMyMoneyRegister::createEditor(int row, int col, bool initFromCell) const
{
  if(!cellWidget(row, col))
    return 0;

  return QTable::createEditor(row, col, initFromCell);
}

void kMyMoneyRegister::setInlineEditingMode(const bool editing)
{
  m_inlineEditMode = editing;
}

void kMyMoneyRegister::ensureTransactionVisible(void)
{
  int prev, curr, next;

  // first row of current transaction
  curr = currentTransactionIndex() * rpt();

  // first row of previous transaction
  prev = curr - rpt();

  // last row of next transaction
  if(m_ledgerLens == true)
    next = curr + rpt() + maxRpt() - 1;
  else
    next = curr + rpt()*2 - 1;

  if(prev >= 0)
    ensureCellVisible(prev, 0);

  ensureCellVisible(curr, 0);

  if(next < numRows())
    ensureCellVisible(next, 0);
}

bool kMyMoneyRegister::eventFilter(QObject* o, QEvent* e)
{
  bool rc = false;

  if(e->type() == QEvent::KeyPress && !m_inlineEditMode) {
    QKeyEvent *k = static_cast<QKeyEvent *> (e);
    rc = true;
    switch(k->key()) {
      case Qt::Key_Return:
      case Qt::Key_Enter:
        emit signalEnter();
        break;

      case Qt::Key_Escape:
        emit signalEsc();
        break;

      case Qt::Key_Up:
        emit signalPreviousTransaction();
        break;

      case Qt::Key_Down:
        emit signalNextTransaction();
        break;

      default:
        rc = false;
        break;
    }
  }

  // if the key has not been processed here, forward it to
  // the base class implementation
  if(rc == false)
    rc = QTable::eventFilter(o, e);

  return rc;
}

void kMyMoneyRegister::setLedgerLens(const bool enabled)
{
  m_ledgerLens = enabled;
}

void kMyMoneyRegister::insertWidget(int row, int col, QWidget* w)
{
  int   myRow;

  if(row < 0 || col < 0 || row >= numRows() || col >= numCols())
    return;

  if(row < m_currentTransactionIndex * m_rpt)
    return;

  if(m_ledgerLens) {
    if(row >= (m_currentTransactionIndex * m_rpt) + maxRpt())
      return;
    myRow = row % maxRpt();
  } else {
    if(row >= (m_currentTransactionIndex * (m_rpt+1)))
      return;
    myRow = row % m_rpt;
  }
  m_editWidgets.insert(indexOf(myRow, col), w);
}

QWidget* kMyMoneyRegister::cellWidget(int row, int col) const
{
  int   myRow;

  if(row < 0 || col < 0 || row >= numRows() || col >= numCols())
    return 0;

  if(row < m_currentTransactionIndex * m_rpt)
    return 0;

  if(m_ledgerLens) {
    if(row >= (m_currentTransactionIndex * m_rpt) + maxRpt())
      return 0;
    myRow = row % maxRpt();
  } else {
    if(row >= (m_currentTransactionIndex * (m_rpt+1)))
      return 0;
    myRow = row % m_rpt;
  }

  return m_editWidgets[indexOf(myRow, col)];
}


void kMyMoneyRegister::clearCellWidget(int row, int col)
{
  int   myRow;

  if(row < 0 || col < 0 || row >= numRows() || col >= numCols())
    return;

  if(row < m_currentTransactionIndex * m_rpt)
    return;

  if(m_ledgerLens) {
    if(row >= (m_currentTransactionIndex * m_rpt) + maxRpt())
      return;
    myRow = row % maxRpt();
  } else {
    if(row >= (m_currentTransactionIndex * (m_rpt+1)))
      return;
    myRow = row % m_rpt;
  }
  QWidget* w = cellWidget(row, col);

  if( w ) {
    w->removeEventFilter(this);
    w->deleteLater();
  }
  m_editWidgets.setAutoDelete(false);
  m_editWidgets.remove(indexOf(myRow, col));
  m_editWidgets.setAutoDelete(true);
}

void kMyMoneyRegister::setNumCols(int c)
{
  m_editWidgets.resize(maxRpt() * c);

  for(int i=0; i < maxRpt(); ++i)
    for(int j=0; j < c; ++j)
      m_editWidgets.insert(indexOf(i, j), 0);

  QTable::setNumCols(c);

  updateHeaders();
}

void kMyMoneyRegister::updateHeaders(void)
{
  QFontMetrics fm( m_headerFont );
  int height = fm.lineSpacing()+6;

  horizontalHeader()->setMinimumHeight(height);
  horizontalHeader()->setMaximumHeight(height);
  horizontalHeader()->setFont(m_headerFont);
}
