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

kMyMoneyRegister::kMyMoneyRegister(QWidget *parent, const char *name )
  : QTable(parent,name)
{
  readConfig();
  m_currentDateRow = -1;
  m_lastTransactionIndex = -1;
}

kMyMoneyRegister::~kMyMoneyRegister()
{
}

void kMyMoneyRegister::setNumRows(int r)
{
}

void kMyMoneyRegister::setTransactionCount(int r)
{
  setUpdatesEnabled( false );

  int irows = r * m_rpt;

  QTable::setNumRows(irows);

/*
  // the following code fragment resizes the rows to match the current
  // font height. Unfortunately, this slows down the display process
  // significantly when displaying 1500+ transactions.

  QFontMetrics fm( m_font );
  int height = fm.height()+4;

  for(int i = 0; i < irows; ++i)
    verticalHeader()->resizeSection(i, height);
*/

  setUpdatesEnabled( true );

  // add or remove scrollbars as required
  updateScrollBars();

  m_lastTransactionIndex = -1;
}

void kMyMoneyRegister::readConfig(void)
{
  KConfig *config = KGlobal::config();
  config->setGroup("List Options");

  m_font = QFont("helvetica", 8);
  m_color = Qt::white;
  m_bgColor = Qt::gray;
  m_gridColor = Qt::black;

  m_bgColor = config->readColorEntry("listBGColor", &m_bgColor);
  m_color = config->readColorEntry("listColor", &m_color);
  m_gridColor = config->readColorEntry("listGridColor", &m_gridColor);

  m_font = config->readFontEntry("listCellFont", &m_font);

  m_rpt = config->readEntry("RowCount", "1").toInt();
  m_showGrid = config->readBoolEntry("ShowGrid", true);
  m_colorPerTransaction = config->readBoolEntry("ColourPerTransaction", true);
}

void kMyMoneyRegister::paintCell(QPainter *p, int row, int col, const QRect& r,
                                 bool selected, const QColorGroup& cg)
{
  m_cg = cg;

  if (m_colorPerTransaction) {
    if((row/m_rpt)%2)
      m_cg.setColor(QColorGroup::Base, m_color);
    else
      m_cg.setColor(QColorGroup::Base, m_bgColor);

  } else {
    if (row%2)
      m_cg.setColor(QColorGroup::Base, m_color);
    else
      m_cg.setColor(QColorGroup::Base, m_bgColor);
  }

  p->setFont(m_font);

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

  QBrush backgroundBrush(m_cg.base());
  p->fillRect(m_cellRect,backgroundBrush);
  p->setPen(m_cg.foreground());

  m_transactionIndex = row/m_rpt;
  m_transactionRow = row%m_rpt;
  if(m_transactionIndex != m_lastTransactionIndex) {
    m_transaction = m_view->transaction(m_transactionIndex);
    m_split = m_transaction->split(m_view->accountId());
    m_balance = m_view->balance(m_transactionIndex);
    m_lastTransactionIndex = m_transactionIndex;
  }
}