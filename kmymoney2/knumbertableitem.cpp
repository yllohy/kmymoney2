/***************************************************************************
                          knumbertableitem.cpp  -  description
                             -------------------
    begin                : Fri Jan 26 2001
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
#include "knumbertableitem.h"
#include "kmymoneysettings.h"

KNumberTableItem::KNumberTableItem(QTable *t, EditType et, const QString &txt)
  : QTableItem(t, et, txt), number_edit(0)
{
  setReplaceable(false);
}

QWidget *KNumberTableItem::createEditor() const
{
  ((KNumberTableItem*)this)->number_edit = new QLineEdit(table()->viewport());
  number_edit->setText(text());
  number_edit->selectAll();
  return number_edit;
}

void KNumberTableItem::setContentFromEditor(QWidget *w)
{
  if (w->inherits("QLineEdit")) {
    setText(((QLineEdit*)w)->text());
  }
  else
    QTableItem::setContentFromEditor(w);
}

void KNumberTableItem::setText(const QString &s)
{
//  if (number_edit)
//    number_edit->setText(s);
  QTableItem::setText(s);
}

void KNumberTableItem::paint(QPainter *p, const QColorGroup &cg, const QRect &cr, bool selected)
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
