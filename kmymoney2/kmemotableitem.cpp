/***************************************************************************
                          kmemotableitem.cpp  -  description
                             -------------------
    begin                : Fri Jan 26 2001
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

#include "kmemotableitem.h"
#include "kmymoneysettings.h"

KMemoTableItem::KMemoTableItem(QTable *t, EditType et, const QString &txt)
  : QTableItem(t, et, txt), memo_edit(0)
{
  setReplaceable(false);
}

QWidget *KMemoTableItem::createEditor() const
{
  ((KMemoTableItem*)this)->memo_edit = new QLineEdit(table()->viewport());
  memo_edit->setText(text());
  memo_edit->selectAll();
  return memo_edit;
}

void KMemoTableItem::setContentFromEditor(QWidget *w)
{
  if (w->inherits("QLineEdit")) {
    setText(((QLineEdit*)w)->text());
  }
  else
    QTableItem::setContentFromEditor(w);
}

void KMemoTableItem::setText(const QString &s)
{
  if (memo_edit)
    memo_edit->setText(s);
  QTableItem::setText(s);
}

void KMemoTableItem::paint(QPainter *p, const QColorGroup &cg, const QRect &cr, bool selected)
{
  KMyMoneySettings *p_settings = KMyMoneySettings::singleton();

  QColorGroup g(cg);
  if ((row()%2) && p_settings)
    g.setColor(QColorGroup::Base, p_settings->lists_BGColor());
  else if (p_settings)
    g.setColor(QColorGroup::Base, p_settings->lists_color());

  if (p_settings)
    p->setFont(p_settings->lists_cellFont());
  QTableItem::paint(p, g, cr, selected);
}
