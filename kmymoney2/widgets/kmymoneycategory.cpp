/***************************************************************************
                          kmymoneycategory.cpp  -  description
                             -------------------
    begin                : Sun Aug 11 2002
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

#include "kmymoneycategory.h"
#include "../mymoney/mymoneyfile.h"

kMyMoneyCategory::kMyMoneyCategory(QWidget *parent, const char *name, const categoryTypeE categoryType)
  : KLineEdit(parent,name)
{
  // make sure, the completion object exists
  if(compObj() == 0)
    completionObject();

  compObj()->setOrder(KCompletion::Sorted);
  setCompletionMode(KGlobalSettings::CompletionPopupAuto);

  // set the standard value for substring completion, as we
  // fake that with every key entered
  setKeyBinding(SubstringCompletion, KShortcut("Ctrl+T"));

  loadList(categoryType);
}

kMyMoneyCategory::~kMyMoneyCategory()
{
}

void kMyMoneyCategory::keyPressEvent( QKeyEvent * ev)
{
  KLineEdit::keyPressEvent(ev);
  if(ev->isAccepted()) {
    // if the key was accepted by KLineEdit, we fake a substring completion
    // which we set previously to Ctrl+T.
    QKeyEvent evc(QEvent::KeyPress, Qt::Key_T, 0, Qt::ControlButton);
    KLineEdit::keyPressEvent(&evc);
  }
}

void kMyMoneyCategory::loadText(const QString& text)
{
  m_text = text;
  setText(text);
}

void kMyMoneyCategory::resetText(void)
{
  setText(m_text);
}


void kMyMoneyCategory::addCategories(QStringList& strList, const QCString& id, const QString& leadIn)
{
  MyMoneyFile *file = MyMoneyFile::instance();
  QString name;

  MyMoneyAccount account = file->account(id);

  QCStringList accList = account.accountList();
  QCStringList::ConstIterator it_a;

  for(it_a = accList.begin(); it_a != accList.end(); ++it_a) {
    account = file->account(*it_a);
    strList << leadIn + account.name();
    addCategories(strList, *it_a, leadIn + account.name() + ":");
  }
}

void kMyMoneyCategory::loadList(const categoryTypeE type)
{
  QStringList strList;

  try {
    MyMoneyFile *file = MyMoneyFile::instance();

    // read all account items from the MyMoneyFile objects and add them to the listbox
    m_accountList = file->accountList();

    if(type & liability)
      addCategories(strList, file->liability().id(), "");

    if(type & asset)
      addCategories(strList, file->asset().id(), "");

    if(type & expense)
      addCategories(strList, file->expense().id(), "");

    if(type & income)
      addCategories(strList, file->income().id(), "");

  } catch (MyMoneyException *e) {
    qDebug("Exception '%s' thrown in %s, line %ld caught in kMyMoneyCategory::loadList",
      e->what().latin1(), e->file().latin1(), e->line());
    delete e;
  }

  // construct the list of completion items
  compObj()->setItems(strList);
  compObj()->setIgnoreCase(true);
}

void kMyMoneyCategory::focusOutEvent(QFocusEvent *ev)
{
  // if the current text is not in the list of
  // possible completions, we have a new category
  // and signal that to the outside world.
  if(text() != "" && compObj()->items().contains(text()) == 0)
    emit newCategory(text());

  if(text() != m_text) {
    emit categoryChanged(text());
  }
}
