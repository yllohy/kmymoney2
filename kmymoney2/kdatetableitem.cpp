/***************************************************************************
                          kdatetableitem.cpp  -  description
                             -------------------
    begin                : Tue Jan 23 2001
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
#include <klocale.h>
#include <qdatetime.h>

#include "kdatetableitem.h"
#include "kmymoneysettings.h"

KDateTableItem::KDateTableItem(QTable *t, EditType et, const QString &txt)
  : QTableItem(t, et, txt), date_edit(0)
{
  setReplaceable(false);
}

QWidget *KDateTableItem::createEditor() const
{
  ((KDateTableItem*)this)->date_edit = new kMyMoneyDateInput(table()->viewport(), QDate::currentDate());
  QDate date = KGlobal::locale()->readDate(text());
  if (date.isValid())
    date_edit->setDate(date);
  return date_edit;
}

void KDateTableItem::setContentFromEditor(QWidget *w)
{
  if (w->inherits("kMyMoneyDateInput")) {
    m_date = ((kMyMoneyDateInput*)w)->getQDate();
    setText(KGlobal::locale()->formatDate(m_date, true));
  }
  else
    QTableItem::setContentFromEditor(w);
}

void KDateTableItem::setText(const QString &s)
{
  QDate date = KGlobal::locale()->readDate(s);
  if (date.isValid() && date_edit)
    date_edit->setDate(date);
  QTableItem::setText(s);
}

QDate KDateTableItem::date(void)
{
  return m_date;
}

void KDateTableItem::paint(QPainter *p, const QColorGroup &cg, const QRect &cr, bool selected)
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
