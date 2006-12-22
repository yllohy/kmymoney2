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
#include <qrect.h>
#include <qpainter.h>
#include <qpalette.h>

// ----------------------------------------------------------------------------
// KDE Includes
#include <klocale.h>

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

  // force update of hint
  if(text().isEmpty())
    repaint();
}

void kMyMoneyLineEdit::drawContents( QPainter *p)
{
  KLineEdit::drawContents(p);

  if(text().isEmpty() && !m_hint.isEmpty() && !hasFocus()) {
    const int innerMargin = 1;

    // the following 5 lines are taken from QLineEdit::drawContents()
    QRect cr = contentsRect();
    QFontMetrics fm = fontMetrics();
    QRect lineRect( cr.x() + innerMargin, cr.y() + (cr.height() - fm.height() + 1) / 2,
                    cr.width() - 2*innerMargin, fm.height() );
    QPoint topLeft = lineRect.topLeft() - QPoint(0, -fm.ascent());

    p->save();
    QFont f = p->font();
    f.setItalic(true);
    f.setWeight(QFont::Light);
    p->setFont(f);
    p->setPen(palette().disabled().text());

    p->drawText(topLeft, m_hint);

    p->restore();
  }
}

#include "kmymoneylineedit.moc"
