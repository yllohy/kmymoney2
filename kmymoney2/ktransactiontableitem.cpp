/***************************************************************************
                          ktransactiontableitem.cpp  -  description
                             -------------------
    begin                : Tue Jan 23 2001
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
#include <kglobal.h>
#include <klocale.h>

#include "ktransactiontableitem.h"

KTransactionTableItem::KTransactionTableItem(QTable *t, EditType et, const QString &txt, types type, MyMoneyFile *file)
  : QTableItem(t, et, txt)
{
  m_type = type;
  m_file = file;
  setReplaceable(false);
}

KTransactionTableItem::KTransactionTableItem(QTable *t, EditType et, const QString &txt)
  : QTableItem(t, et, txt)
{
  m_type=NoType;
}

QWidget *KTransactionTableItem::createEditor() const
{
//  enum types { NoType, MethodType, DateType, NumberType, MemoType, CategoryType };
  switch (m_type) {
    case MethodType:
      ((KTransactionTableItem*)this)->method_cb = new QComboBox(table()->viewport());
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
    case DateType:
      ((KTransactionTableItem*)this)->date_edit = new kMyMoneyDateInput(table()->viewport(), QDate::currentDate());
      QDate date = KGlobal::locale()->readDate(text());
      if (date.isValid())
        date_edit->setDate(date);
      return date_edit;
    case MemoType:
      ((KTransactionTableItem*)this)->memo_edit = new QLineEdit(table()->viewport());
      memo_edit->setText(text());
      return memo_edit;
    case NumberType:
      ((KTransactionTableItem*)this)->number_edit = new KIntNumInput(table()->viewport());
      number_edit->setText(text());
      return number_edit;

  }
  return 0;
}

void KTransactionTableItem::setContentFromEditor(QWidget *w)
{
  if (w->inherits("QComboBox")) {
    setText(((QComboBox*)w)->currentText());
  }
  else if (w->inherits("kMyMoneyDateInput")) {
    QDate date = ((kMyMoneyDateInput*)w)->getQDate();
    setText(KGlobal::locale()->formatDate(date, true));
  }
  else
    QTableItem::setContentFromEditor(w);
}

void KTransactionTableItem::setText(const QString &s)
{
  switch (m_type) {
    case MethodType:
      if (method_cb) {
        if (s=="Cheque")
          method_cb->setCurrentItem(0);
        else if (s=="Deposit")
          method_cb->setCurrentItem(1);
        else if (s=="Transfer")
          method_cb->setCurrentItem(2);
        else if (s=="Withdrawal")
          method_cb->setCurrentItem(3);
        else if (s=="ATM")
          method_cb->setCurrentItem(4);
      }
      break;
    case DateType:
      QDate date = KGlobal::locale()->readDate(s);
      if (date.isValid())
        date_edit->setDate(date);
      break;
  }
  QTableItem::setText(s);
}
