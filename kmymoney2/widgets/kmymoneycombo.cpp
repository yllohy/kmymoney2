/***************************************************************************
                          kmymoneycombo.cpp  -  description
                             -------------------
    begin                : Sat May 5 2001
    copyright            : (C) 2001 by Michael Edwardes
    email                : mte@users.sourceforge.net
                             Javier Campos Morales <javi_c@ctv.es>
                             Felix Rodriguez <frodriguez@mail.wesleyan.edu>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kmymoneycombo.h"

kMyMoneyCombo::kMyMoneyCombo(QWidget *w)
  : KComboBox(w)
{
  init();
}

kMyMoneyCombo::kMyMoneyCombo(bool rw, QWidget *w)
  : KComboBox(rw, w)
{
  init();
}

void kMyMoneyCombo::init(void)
{
  connect(this, SIGNAL(activated(int)), SLOT(slotCheckValidSelection(int)));
}

kMyMoneyCombo::~kMyMoneyCombo()
{
}

/** No descriptions */
bool kMyMoneyCombo::eventFilter(QObject *o, QEvent *e)
{
  if(e->type() == QEvent::FocusOut) {
    emit signalFocusOut();

  } else if(e->type() == QEvent::KeyRelease) {
    QKeyEvent *k = static_cast<QKeyEvent *> (e);
    if((k->key() == Qt::Key_Return) ||
      (k->key() == Qt::Key_Enter)) {
      emit signalEnter();
      emit signalNextTransaction();
    }

  } else if(e->type() == QEvent::KeyPress) {
    QKeyEvent *k = static_cast<QKeyEvent *> (e);
    if(k->key() == Qt::Key_Backtab ||
       (k->key() == Qt::Key_Tab &&
       (k->state() & Qt::ShiftButton)) ) {
      emit signalBackTab();

    } else if(k->key() == Qt::Key_Tab) {
      emit signalTab();
    }
  }
  return KComboBox::eventFilter(o,e);
}

void kMyMoneyCombo::setCurrentItem(const QString& str)
{
  int i=0;

  for(; i < count(); ++i) {
    if(str == text(i)) {
      KComboBox::setCurrentItem(i);
      break;
    }
  }

  if(i == count()) {
    KComboBox::setCurrentItem(0);
  }
}

void kMyMoneyCombo::slotCheckValidSelection(int id)
{
  QString txt = text(id);
  if(txt.left(4) == "--- "
  && txt.right(4) == " ---") {
    if(id < count()-1) {
      KComboBox::setCurrentItem(id+1);
    } else {
      KComboBox::setCurrentItem(id-1);
    }
  }
}