/***************************************************************************
                          kreconciledtableitem.cpp  -  description
                             -------------------
    begin                : Tue Feb 20 2001
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

#include "kreconciledtableitem.h"
#include "kmymoneysettings.h"

KReconciledTableItem::KReconciledTableItem(QTable *t, EditType et, const QString &txt)
  : QTableItem(t, QTableItem::Never, txt)
{
  setReplaceable(false);
}
/*
QWidget *KReconciledTableItem::createEditor() const
{
  return 0;
}

void KReconciledTableItem::setContentFromEditor(QWidget *w)
{
  QTableItem::setContentFromEditor(w);
}
*/
void KReconciledTableItem::setText(const QString &s)
{
  QTableItem::setText(s);
}

void KReconciledTableItem::paint(QPainter *p, const QColorGroup &cg, const QRect &cr, bool selected)
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
