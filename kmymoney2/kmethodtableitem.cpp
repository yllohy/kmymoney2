/***************************************************************************
                          kmethodtableitem.cpp  -  description
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
#include "kmethodtableitem.h"
#include "kmymoneysettings.h"

KMethodTableItem::KMethodTableItem(QTable *t, EditType et, const QString &txt)
  : QTableItem(t, et, txt), method_cb(0)
{
  setReplaceable(false);
}
/*
QWidget *KMethodTableItem::createEditor() const
{
      ((KMethodTableItem*)this)->method_cb = new QComboBox(table()->viewport());
      method_cb->insertItem("Cheque");
      method_cb->insertItem("Deposit");
      method_cb->insertItem("Transfer");
      method_cb->insertItem("Withdrawal");
      method_cb->insertItem("ATM");
      if (text()=="Cheque")
        method_cb->setCurrentItem(0);
      else if (text()=="Deposit")
        method_cb->setCurrentItem(1);
      else if (text()=="Transfer")
        method_cb->setCurrentItem(2);
      else if (text()=="Withdrawal")
        method_cb->setCurrentItem(3);
      else if (text()=="ATM")
        method_cb->setCurrentItem(4);
      return method_cb;
}

void KMethodTableItem::setContentFromEditor(QWidget *w)
{
  if (w->inherits("QComboBox")) {
    setText(((QComboBox*)w)->currentText());
  }
  else
    QTableItem::setContentFromEditor(w);
}
*/
void KMethodTableItem::setText(const QString &s)
{
      if (method_cb) {
        if (s=="Cheque") {
          method_cb->setCurrentItem(0);
          m_method = MyMoneyTransaction::Cheque;
        }
        else if (s=="Deposit") {
          method_cb->setCurrentItem(1);
          m_method = MyMoneyTransaction::Deposit;
        }
        else if (s=="Transfer") {
          method_cb->setCurrentItem(2);
          m_method = MyMoneyTransaction::Transfer;
        }
        else if (s=="Withdrawal") {
          method_cb->setCurrentItem(3);
          m_method = MyMoneyTransaction::Withdrawal;
        }
        else if (s=="ATM") {
          method_cb->setCurrentItem(4);
          m_method = MyMoneyTransaction::ATM;
        }
      }
  QTableItem::setText(s);
}

MyMoneyTransaction::transactionMethod KMethodTableItem::method(void)
{
  return m_method;
}

void KMethodTableItem::paint(QPainter *p, const QColorGroup &cg, const QRect &cr, bool selected)
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
