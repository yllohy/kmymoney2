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

kMyMoneyLineEdit::kMyMoneyLineEdit(QWidget *w, const char* name, int alignment)
  : KLineEdit(w, name)
{
  setAlignment(alignment);
}

kMyMoneyLineEdit::~kMyMoneyLineEdit()
{
}

/** No descriptions */
bool kMyMoneyLineEdit::eventFilter(QObject *o , QEvent *e )
{
  bool rc = KLineEdit::eventFilter(o, e);

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
