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


#include "kmymoneyequitycompletion.h"

#define MAX_ITEMS   16

kMyMoneyEquityCompletion::kMyMoneyEquityCompletion(QWidget *parent, const char *name ) :
  kMyMoneyCompletion(parent, name)
{
  m_equitySelector = new kMyMoneyEquitySelector(this, 0, 0);

  connectSignals(static_cast<QWidget*> (m_equitySelector), m_equitySelector->listView());
}

kMyMoneyEquityCompletion::~kMyMoneyEquityCompletion()
{
}

void kMyMoneyEquityCompletion::show(void)
{
  int  count;

  count = loadList();
  if(!m_id.isEmpty())
    m_equitySelector->setSelected(m_id);

  // make sure we increase the count by the equity header group
  adjustSize(count+1);

  kMyMoneyCompletion::show();
}

void kMyMoneyEquityCompletion::slotMakeCompletion(const QString& txt)
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

  int count = m_equitySelector->slotMakeCompletion(account);

  if(count != 0) {
    // don't forget the four group lines
    adjustSize(count+4);
  } else {
    hide();
  }
}

