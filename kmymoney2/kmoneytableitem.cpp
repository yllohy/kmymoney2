/***************************************************************************
                          kmoneytableitem.cpp  -  description
                             -------------------
    begin                : Mon Feb 12 2001
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
#include "kmoneytableitem.h"
#include "kmymoneysettings.h"

KMoneyTableItem::KMoneyTableItem(QTable *t, EditType et, const QString &txt)
  : QTableItem(t, et, txt), money_edit(0)
{
  setReplaceable(false);
}

QWidget *KMoneyTableItem::createEditor() const
{
  ((KMoneyTableItem*)this)->money_edit = new kMyMoneyEdit(table()->viewport());
  qDebug("text() == %s", text().latin1());
  bool ok=false;
  double amount = KGlobal::locale()->readMoney(text(), &ok);
  if (ok) {
    money_edit->setText(QString::number(amount, 'f', KGlobal::locale()->fracDigits()));
    money_edit->selectAll();
  }

  return money_edit;
}

void KMoneyTableItem::setContentFromEditor(QWidget *w)
{
  if (w->inherits("kMyMoneyEdit")) {
    m_money = ((kMyMoneyEdit*)w)->getMoneyValue();
    setText(KGlobal::locale()->formatMoney(m_money.amount()));
  }
  else
    QTableItem::setContentFromEditor(w);
}

void KMoneyTableItem::setText(const QString &s)
{
  if (money_edit) {
    money_edit->setText(s);
  }
  QTableItem::setText(s);
}

void KMoneyTableItem::paint(QPainter *p, const QColorGroup &cg, const QRect &cr, bool selected)
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
