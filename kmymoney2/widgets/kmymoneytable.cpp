/***************************************************************************
                          kmymoneytable.cpp  -  description
                             -------------------
    begin                : Wed Feb 21 2001
    copyright            : (C) 2001 by Michael Edwardes
    email                : Michael.Edwardes@students.dmu.ac.uk
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
