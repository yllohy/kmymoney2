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

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneycombo.h"

kMyMoneyCombo::kMyMoneyCombo(QWidget *w, const char *name)
  : KComboBox(w, name)
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

bool kMyMoneyCombo::eventFilter(QObject *o, QEvent *e)
{
  bool rc = KComboBox::eventFilter(o, e);

  if(rc == false) {
    if(e->type() == QEvent::KeyPress) {
      QKeyEvent *k = static_cast<QKeyEvent *> (e);
      rc = true;
      switch(k->key()) {
        case Qt::Key_Return:
        case Qt::Key_Enter:
          emit signalEnter();
          break;

        case Qt::Key_Escape:
          emit signalEsc();
          break;

        case Qt::Key_Tab:
          rc = false;
          if(k->state() & Qt::ShiftButton)
            emit signalBackTab();
          else
            emit signalTab();
          break;

        case Qt::Key_Backtab:
          rc = false;
          emit signalBackTab();
          break;

        default:
          rc = false;
      }
    }
  }
  return rc;
/*
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
*/
}

void kMyMoneyCombo::loadCurrentItem(const int item)
{
  m_item = item;
  resetCurrentItem();
}

void kMyMoneyCombo::resetCurrentItem(void)
{
  setCurrentItem(m_item);
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

  // if not found, select the first one
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

void kMyMoneyCombo::focusOutEvent(QFocusEvent *ev)
{
  // if the current text is not in the list of
  // possible completions, we have a new payee
  // and signal that to the outside world.
  if(currentItem() != m_item) {
    emit selectionChanged(currentItem());
  }
  KComboBox::focusOutEvent(ev);
}
