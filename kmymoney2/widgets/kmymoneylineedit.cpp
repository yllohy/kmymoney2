/***************************************************************************
                          kmymoneylineedit.cpp  -  description
                             -------------------
    begin                : Wed May 9 2001
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

#include "kmymoneylineedit.h"

kMyMoneyLineEdit::kMyMoneyLineEdit(QWidget *w):KLineEdit(w)
{
	setAlignment(AlignRight | AlignVCenter);
}

kMyMoneyLineEdit::~kMyMoneyLineEdit()
{
}

/** No descriptions */
bool kMyMoneyLineEdit::eventFilter(QObject *o , QEvent *e )
{
  if(e->type() == QEvent::KeyRelease) {
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
  return KLineEdit::eventFilter(o,e);
}

void kMyMoneyLineEdit::resetText(void)
{
  setText(m_text);
}

void kMyMoneyLineEdit::loadText(const QString& text)
{
  m_text = text;
  setText(text);
}

void kMyMoneyLineEdit::focusOutEvent(QFocusEvent *ev)
{
  // if the current text is not in the list of
  // possible completions, we have a new payee
  // and signal that to the outside world.
  if(text() != m_text) {
    emit lineChanged(text());
  }
  KLineEdit::focusOutEvent(ev);
}
