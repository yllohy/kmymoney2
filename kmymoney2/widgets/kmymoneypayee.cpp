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

// ----------------------------------------------------------------------------
// KDE Includes

#include "kdecompat.h"

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

  loadList();
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
