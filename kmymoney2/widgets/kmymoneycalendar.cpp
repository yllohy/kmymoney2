/***************************************************************************
                          kmymoneycalendar.cpp  -  description
                             -------------------
    begin                : Wed Jul 2 2003
    copyright            : (C) 2000-2003 by Michael Edwardes
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
#include <qpainter.h>
#include <qdrawutil.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes


#include "kmymoneycalendar.h"

kMyMoneyCalendar::kMyMoneyCalendar(QWidget *parent, const char *name )
  : QGridView(parent,name), m_type(MONTHLY)
{
  initType();
}

kMyMoneyCalendar::~kMyMoneyCalendar()
{
}

void kMyMoneyCalendar::paintCell(QPainter *p, int row, int col)
{
  QColorGroup cg = colorGroup();
  QBrush b(white);
  
  qDrawShadeRect(p, 0, 0, cellWidth(), cellHeight(), cg, true, 1, 0, &b);
}

void kMyMoneyCalendar::resizeEvent(QResizeEvent* e)
{
  setCellWidth(width()/m_cols);
  setCellHeight(height()/m_rows);
}

void kMyMoneyCalendar::initType()
{
  if (m_type == MONTHLY)
  {
    QDate today = QDate::currentDate();
    m_cols = 7;
    m_rows = today.daysInMonth() / 7;
  }
  else
  {
    m_rows = m_cols = 1;
  }

  setNumRows(m_rows);
  setNumCols(m_cols);
  setCellWidth(width()/m_cols);
  setCellHeight(height()/m_rows);
}
