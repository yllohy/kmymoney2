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

#include "kmymoneypayee.h"

#include "../mymoney/mymoneyfile.h"

kMyMoneyPayee::kMyMoneyPayee(QWidget *parent, const char *name )
  : KLineEdit(parent,name)
{
  // make sure, the completion object exists
  if(compObj() == 0)
    completionObject();

  compObj()->setOrder(KCompletion::Sorted);
  setAutoDeleteCompletionObject(true);

  loadList();
}

kMyMoneyPayee::~kMyMoneyPayee()
{
}

void kMyMoneyPayee::setText(const QString& text)
{
  m_text = text;
  KLineEdit::setText(text);
}

void kMyMoneyPayee::resetText(void)
{
  KLineEdit::setText(m_text);
}

void kMyMoneyPayee::loadList(void)
{
  MyMoneyFile *file = MyMoneyFile::instance();

  // read all payee items from the MyMoneyFile objects and add them to the listbox
  QValueList<MyMoneyPayee> list = file->payeeList();
  QValueList<MyMoneyPayee>::ConstIterator it_p;
  QStringList strList;

  for(it_p = list.begin(); it_p != list.end(); ++it_p) {
    m_payeeConversionList[(*it_p).name().upper()] = (*it_p).id();
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
  if(text() != "" && compObj()->items().contains(text()) == 0)
    emit newPayee(text());
}
