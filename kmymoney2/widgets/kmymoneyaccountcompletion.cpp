/***************************************************************************
                          kmymoneyaccountcompletion.cpp  -  description
                             -------------------
    begin                : Mon Apr 26 2004
    copyright            : (C) 2000-2004 by Michael Edwardes
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

#include <qapplication.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes


#include "kmymoneyaccountcompletion.h"

#define MAX_ITEMS   16

kMyMoneyAccountCompletion::kMyMoneyAccountCompletion(QWidget *parent, const char *name ) :
  kMyMoneyCompletion(parent, name)
{
  m_accountType = (KMyMoneyUtils::categoryTypeE) 0x0f;
  m_accountSelector = new kMyMoneyAccountSelector(this, 0, 0, false);

  connectSignals(static_cast<QWidget*> (m_accountSelector), m_accountSelector->listView());
}

kMyMoneyAccountCompletion::~kMyMoneyAccountCompletion()
{
}

void kMyMoneyAccountCompletion::show(void)
{
  int  count;

  count = loadList(m_accountType);
  if(!m_id.isEmpty())
    m_accountSelector->setSelected(m_id);

  // make sure we increase the count by the account groups
  for(int mask = 0x01; mask != KMyMoneyUtils::last; mask <<= 1) {
    if(m_accountType & mask)
      ++count;
  }
  adjustSize(count);

  kMyMoneyCompletion::show();
}

void kMyMoneyAccountCompletion::slotMakeCompletion(const QString& txt)
{
  if(txt.isEmpty() || txt.length() == 0)
    return;

  QString account(txt);
  int pos = txt.findRev(':');
  if(pos != -1) {
    account = txt.mid(pos+1);
  }

  if(m_parent && m_parent->isVisible() && !isVisible())
    show();

  int count = m_accountSelector->slotMakeCompletion(account);

  if(count != 0) {
    // don't forget the four group lines
    adjustSize(count+4);
  } else {
    hide();
  }
}

