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
#include "../kmymoneyutils.h"
#include "../kmymoneysettings.h"

kMyMoneyRegister::kMyMoneyRegister(int maxRpt, QWidget *parent, const char *name )
  : QTable(parent, name),
    m_ledgerLens(true),
    m_maxRpt(maxRpt),
    m_blinkTimer(parent)
{
  m_cellFont = KMyMoneyUtils::cellFont();
  setFont(m_cellFont);
  readConfig();
  m_currentTransactionIndex = 0;
  m_inlineEditMode = false;
  setSelectionMode(QTable::SingleRow);
  // resize(670,200);
  m_editWidgets.clear();
  m_parent = 0;
  m_errorColor = QColor(255, 0, 0);
  m_lastErrorColor = m_errorColor;

  m_blinkTimer.start(500);       // setup blink frequency to one hertz
  connect(&m_blinkTimer, SIGNAL(timeout()), SLOT(slotBlinkInvalid()));
}

kMyMoneyRegister::~kMyMoneyRegister()
{
}

void kMyMoneyRegister::setNumRows(int /* r */)
{
}

void kMyMoneyRegister::setTransactionCount(const int r, const bool setTransaction)
{
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

  if(setTransaction) {
    setCurrentTransactionIndex(r);
  } else if(m_currentTransactionIndex > (r-1)) {
    setCurrentTransactionIndex(r-1);
  }

  // add or remove scrollbars as required
  updateScrollBars();
}

void kMyMoneyRegister::paintFocus(QPainter* /* p */, const QRect& /* cr */)
{
}

void kMyMoneyRegister::readConfig(void)
{
  KConfig *config = KGlobal::config();
  config->setGroup("List Options");

  m_color = KMyMoneyUtils::listColour();
  m_bgColor = KMyMoneyUtils::backgroundColour();
  m_gridColor = KMyMoneyUtils::gridColour();
  m_importColor = Qt::yellow;
  m_matchColor = QColor("PaleGreen");
  
  QFont cellFont = KMyMoneyUtils::cellFont();
  m_headerFont = KMyMoneyUtils::headerFont();
  updateHeaders();

  if(cellFont != m_cellFont) {
    m_cellFont = cellFont;
    setFont(m_cellFont);
    // force loading of new font into all cells
    int rows = numRows();
    QTable::setNumRows(0);
    QTable::setNumRows(rows);
  }

  if(KMyMoneySettings::showRegisterDetailed())
    m_rpt = m_maxRpt;
  else
    m_rpt = 1;
}

void kMyMoneyRegister::setTransactionRow(const int row)
{
  int firstRow,       // first row occupied by current transaction
      lastRow;        // last row occupied by current transaction
  firstRow = m_currentTransactionIndex * m_rpt;
  if(m_ledgerLens)
    lastRow = (m_currentTransactionIndex * m_rpt) + maxRpt() - 1;
  else
    lastRow = (m_currentTransactionIndex * m_rpt) + m_rpt - 1;

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
  if(!m_parent) {
    qFatal("kMyMoneyRegister::setTransactionRow(): m_parent == 0 !  Use setParent() to set it up");
    exit(0);
  }

  m_transaction = m_parent->transaction(m_transactionIndex);
  if(m_transaction != NULL) {
    m_split = m_transaction->splitById(m_transaction->splitId());
    m_balance = m_parent->balance(m_transactionIndex);
  }
}

void kMyMoneyRegister::paintCell(QPainter *p, int row, int col, const QRect& r,
                                 bool /* selected */, const QColorGroup& cg,
                                 const QString& txt, const int align)
{
  int firstRow,       // first row occupied by current transaction
      lastRow;        // last row occupied by current transaction
  firstRow = m_currentTransactionIndex * m_rpt;
  if(m_ledgerLens)
    lastRow = (m_currentTransactionIndex * m_rpt) + maxRpt() - 1;
  else
    lastRow = (m_currentTransactionIndex * m_rpt) + m_rpt - 1;

  m_cg = cg;

  int adj = 0;
  if(row > lastRow && m_ledgerLens) {
    adj = m_rpt - maxRpt();
  }
  if(((row + adj)/m_rpt)%2)
    m_cg.setColor(QColorGroup::Base, m_color);
  else
    m_cg.setColor(QColorGroup::Base, m_bgColor);

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

  // this should have been called by the caller already, but we never know
  setTransactionRow(row);

  if(m_transaction != NULL) {
    if(m_transaction->value("Imported").lower() == "true") {
      m_cg.setColor(QColorGroup::Base, m_importColor);
    }
    if(m_transaction->value("MatchSelected").lower() == "true") {
      m_cg.setColor(QColorGroup::Base, m_matchColor);
    }
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
  // p->setPen(m_textColor);

  /* new stuff */

  const bool lastLine = m_ledgerLens && m_transactionIndex == m_currentTransactionIndex
                         ? m_transactionRow == maxRpt() - 1
                         : m_transactionRow == m_rpt-1;

  // if a grid is selected, we paint it right away
  if (KMyMoneySettings::showGrid()) {
    p->setPen(m_gridColor);
    p->drawLine(m_cellRect.x(), 0, m_cellRect.x(), m_cellRect.height()-1);
    if(lastLine)
      p->drawLine(m_cellRect.x(), m_cellRect.height()-1, m_cellRect.width(), m_cellRect.height()-1);
  }

  // if we paint something, that we don't know (yet), we're done
  // this applies to the very last line of the ledger which always
  // shows an empty line for new transactions to be added.
  // In case this is the current date row, we still draw the marker
  if(m_transaction == NULL) {
    if(m_transactionIndex == m_currentDateIndex && m_transactionRow == 0) {
      p->setPen(m_gridColor);
      p->drawLine(m_cellRect.x(), 0, m_cellRect.width(), 0);
      p->drawLine(m_cellRect.x(), 1, m_cellRect.width(), 1);
    }
    return;
  }

  // QColor textColor(m_textColor);
  // if it's an erronous transaction, set it to error color (which toggles ;-)  )
  if(!m_transaction->splitSum().isZero()) {
    m_textColor = m_errorColor;
  }
  p->setPen(m_textColor);

  // make sure, we clear the cell
  if(txt.isEmpty())
    p->drawText(m_textRect, align, " ");
  else
    p->drawText(m_textRect, align, txt);

  if(m_transactionIndex == m_currentDateIndex && m_transactionRow == 0) {
    p->setPen(m_gridColor);
    p->drawLine(m_cellRect.x(), 0, m_cellRect.width(), 0);
    p->drawLine(m_cellRect.x(), 1, m_cellRect.width(), 1);
    p->setPen(m_textColor);
  }
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

int kMyMoneyRegister::transactionIndex(const int row) const
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
  return idx;
}

bool kMyMoneyRegister::differentTransaction(int row) const
{
  int idx = transactionIndex(row);
  if(idx < 0)
    idx = 0;
  return (idx != m_currentTransactionIndex);
}

bool kMyMoneyRegister::setCurrentTransactionRow(const int row)
{
  return setCurrentTransactionIndex(transactionIndex(row));
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

  int wt = contentsY();           // window top
  int wh = visibleHeight();       // window height
  int lt = prev * rowHeight(0);   // top of line above lens
  int lb = next * rowHeight(0);   // bottom of line below lens

  // only update widget, if the transaction is not fully visible
  if(lt < wt || lb >= (wt + wh)) {

    if(prev >= 0)
      ensureCellVisible(prev, 0);

    ensureCellVisible(curr, 0);

    if(next < numRows())
      ensureCellVisible(next, 0);
  }
}

bool kMyMoneyRegister::eventFilter(QObject* o, QEvent* e)
{
  QKeyEvent *k = static_cast<QKeyEvent *> (e);
  bool rc = false;

  if(e->type() == QEvent::KeyPress && !m_inlineEditMode) {
    int lines = visibleHeight()/rowHeight(0);
    if(m_ledgerLens)
      lines += maxRpt()-1;
    QCString transactionId;

    rc = true;
    switch(k->key()) {
      case Qt::Key_Space:
        emit signalSpace();
        break;

      case Qt::Key_Delete:
        emit signalDelete();
        break;

      case Qt::Key_PageUp:
        setUpdatesEnabled(false);
        while(lines-- > 0)
          emit signalPreviousTransaction();
        setUpdatesEnabled(true);
        ensureTransactionVisible();
        repaintContents(false);
        break;

      case Qt::Key_PageDown:
        setUpdatesEnabled(false);
        while(lines-- > 0)
          emit signalNextTransaction();
        setUpdatesEnabled(true);
        ensureTransactionVisible();
        repaintContents(false);
        break;

      case Qt::Key_Home:
        transactionId = m_parent->transaction(0)->id();
        // tricky fall through here

      case Qt::Key_End:
        emit signalSelectTransaction(transactionId);
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
  } else if(e->type() == QEvent::KeyPress && m_inlineEditMode) {
    // suppress the F2 functionality to start editing in inline edit mode
    switch(k->key()) {
      case Key_F2:
        rc = true;
        break;
      default:
        break;
    }
  }


  // if the key has not been processed here, forward it to
  // the base class implementation
  if(rc == false) {
    if(e->type() != QEvent::KeyPress
    && e->type() != QEvent::KeyRelease) {
      rc = QTable::eventFilter(o, e);
    }
  }

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

void kMyMoneyRegister::slotBlinkInvalid(void)
{
  // setup the color invalid transactions will be drawn in
  if(m_errorColor == m_textColor) {
    m_errorColor = QColor(255, 0, 0);
  } else {
    m_errorColor = m_textColor;
  }

  // now search for invalid transactions in the visible area
  // and force to repaint them

  if(m_parent && isVisible()) {
    int c, w = 0;
    for(c = 0; c < numCols(); ++c) {
      w += columnWidth(c);
    }
    QSize size(w, rowHeight(0));

    for(int r = 0; r < numRows(); ++r) {
      MyMoneyTransaction* t = m_parent->transaction(transactionIndex(r));
      if(t && !t->splitSum().isZero()) {
        QRect cg(QPoint(0,rowPos(r)), size);
        // following line taken from QTable::repaintCell()
        QRect pr( QPoint( cg.x() - 2, cg.y() - 2 ),
                QSize( cg.width() + 4, cg.height() + 4 ) );
        repaintContents(pr, false);
      }
    }
  }
}

void kMyMoneyRegister::setCurrentDateIndex(const int idx)
{
  m_currentDateIndex = idx;
}

QSize kMyMoneyRegister::sizeHint() const
{
  return minimumSizeHint();
}

QSize kMyMoneyRegister::minimumSizeHint() const
{
  return QSize(500, 100);
}

void kMyMoneyRegister::setAction(const QCString& action, const QString& txt)
{
  m_action[action] = txt;
}

bool kMyMoneyRegister::focusNextPrevChild(bool next)
{
  return m_parent->focusNextPrevChild(next);
}


#include "kmymoneyregister.moc"
// vim:cin:si:ai:et:ts=2:sw=2:
