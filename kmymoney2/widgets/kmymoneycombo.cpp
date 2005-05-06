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
#include "../mymoney/mymoneyfile.h"

kMyMoneyCombo::kMyMoneyCombo(QWidget *w, const char *name)
  : KComboBox(w, name)
{
  init();
}

kMyMoneyCombo::kMyMoneyCombo(bool rw, QWidget *w, const char *name)
  : KComboBox(rw, w, name)
{
  init();
}

void kMyMoneyCombo::init(void)
{
  m_type = NONE;
  connect(this, SIGNAL(activated(int)), SLOT(slotCheckValidSelection(int)));
}

kMyMoneyCombo::~kMyMoneyCombo()
{
}

void kMyMoneyCombo::loadCurrentItem(const int item)
{
  m_prevItem = m_item = item;
  resetCurrentItem();
}

void kMyMoneyCombo::resetCurrentItem(void)
{
  m_prevItem = m_item;
  setCurrentItem(m_item);
}

void kMyMoneyCombo::setCurrentItem(const QString& str)
{
  int i=0;

  for(; i < count(); ++i) {
    if(str == text(i)) {
      KComboBox::setCurrentItem(i);
      m_prevItem = i;
      break;
    }
  }

  // if not found, select the first one
  if(i == count()) {
    qDebug("kMyMoneyCombo::setCurrentItem: '%s' not found", str.latin1());
    KComboBox::setCurrentItem(0);
    m_prevItem = 0;
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
  if(currentItem() != m_prevItem) {
    emit selectionChanged(currentItem());
    m_prevItem = currentItem();
  }
  KComboBox::focusOutEvent(ev);
}

void kMyMoneyCombo::loadAccounts(bool asset, bool liability)
{
  try
  {
    MyMoneyFile* file = MyMoneyFile::instance();

    MyMoneyAccount acc;
    QCStringList::ConstIterator it_s;

    if (asset)
    {
      acc = file->asset();
      for(it_s = acc.accountList().begin(); it_s != acc.accountList().end(); ++it_s)
      {
        MyMoneyAccount a = file->account(*it_s);
        KComboBox::insertItem(a.name());
      }
    }

    if (liability)
    {
      acc = file->liability();
      for(it_s = acc.accountList().begin(); it_s != acc.accountList().end(); ++it_s)
      {
        MyMoneyAccount a = file->account(*it_s);
        KComboBox::insertItem(a.name());
      }
    }

    m_type = ACCOUNT;
  }
  catch (MyMoneyException *e)
  {
    delete e;
  }
}

QCString kMyMoneyCombo::currentAccountId(void)
{
  try
  {
    if (m_type == ACCOUNT)
      return MyMoneyFile::instance()->nameToAccount(currentText());
  }
  catch (MyMoneyException *e)
  {
    delete e;
  }

  return "";
}
