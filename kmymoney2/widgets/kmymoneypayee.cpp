/***************************************************************************
                          kmymoneypayee.cpp  -  description
                             -------------------
    begin                : Sat Aug 10 2002
    copyright            : (C) 2000-2002 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
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
// #include "kdecompat.h"

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneypayee.h"
#include "../mymoney/mymoneyfile.h"

kMyMoneyPayee::kMyMoneyPayee(QWidget *parent, const char *name )
  : KLineEdit(parent,name)
{
  // make sure, the completion object exists
  if(compObj() == 0)
    completionObject();

  compObj()->setOrder(KCompletion::Sorted);
#if KDE_IS_VERSION(3,2,0)
  setCompletionMode(KGlobalSettings::CompletionPopup);
#else
  setCompletionMode(KGlobalSettings::CompletionPopupAuto);
#endif
  setAutoDeleteCompletionObject(true);

  // set the standard value for substring completion, as we
  // fake that with every key entered. see also
  // code in keyPressEvent()
  setKeyBinding(SubstringCompletion, KShortcut("Ctrl+T"));

  // if used from within Qt Designer library loadList() will fail
  // but we don't care.
  try {
    loadList();
  } catch(MyMoneyException *e) {
    delete e;
  }
}

kMyMoneyPayee::~kMyMoneyPayee()
{
}

void kMyMoneyPayee::loadText(const QString& text)
{
  m_text = text;
  setText(text);
}

void kMyMoneyPayee::resetText(void)
{
  setText(m_text);
}

void kMyMoneyPayee::loadList(void)
{
  MyMoneyFile *file = MyMoneyFile::instance();

  // read all payee items from the MyMoneyFile objects and add them to the listbox
  QValueList<MyMoneyPayee> list = file->payeeList();
  QValueList<MyMoneyPayee>::ConstIterator it_p;
  QStringList strList;

  for(it_p = list.begin(); it_p != list.end(); ++it_p) {
    strList << (*it_p).name();
  }

  // construct the list of completion items
  compObj()->setItems(strList);
  compObj()->setIgnoreCase(true);
}

void kMyMoneyPayee::focusOutEvent(QFocusEvent *ev)
{
  // if the current text is not in the list of
  // possible completions, we have a new payee
  // and signal that to the outside world.
  if(!text().isEmpty() && compObj()->items().contains(text()) == 0)
    emit newPayee(text());

  if(text() != m_text) {
    emit payeeChanged(text());
  }
  KLineEdit::focusOutEvent(ev);

  // force update of hint
  if(text().isEmpty())
    repaint();
}

void kMyMoneyPayee::keyPressEvent( QKeyEvent * ev)
{
  KLineEdit::keyPressEvent(ev);
  if(ev->isAccepted()) {
    // if the key was accepted by KLineEdit, we fake a substring completion
    // which we set previously to Ctrl+T.
    QKeyEvent evc(QEvent::KeyPress, Qt::Key_T, 0, Qt::ControlButton);
    KLineEdit::keyPressEvent(&evc);
  }
}


void kMyMoneyPayee::drawContents( QPainter *p)
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

#include "kmymoneypayee.moc"
