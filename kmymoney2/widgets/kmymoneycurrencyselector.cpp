/***************************************************************************
                          kmymoneycurrencyselector.cpp  -  description
                             -------------------
    begin                : Tue Apr 6 2004
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

#include <qpixmap.h>
#include <qbitmap.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kstandarddirs.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneycurrencyselector.h"

kMyMoneyCurrencySelector::kMyMoneyCurrencySelector(QWidget *parent, const char *name ) :
  KComboBox(parent,name)
{
  update(QCString());
}

kMyMoneyCurrencySelector::~kMyMoneyCurrencySelector()
{
}

void kMyMoneyCurrencySelector::update(const QCString& id)
{
  MyMoneyCurrency curr = MyMoneyFile::instance()->baseCurrency();
  QCString baseCurrency = curr.id();

  if(!id.isEmpty())
    curr = m_currency;

  this->clear();
  QValueList<MyMoneyCurrency> list = MyMoneyFile::instance()->currencyList();
  QValueList<MyMoneyCurrency>::ConstIterator it;

  // construct a transparent 16x16 pixmap
  QPixmap empty(16, 16);
  empty.setMask(QBitmap(16, 16, true));

  int itemId = 0;
  int selectedItemId = 0;
  for(it = list.begin(); it != list.end(); ++it) {
    if((*it).id() == baseCurrency) {
      insertItem(QPixmap( locate("icon","hicolor/16x16/apps/kmymoney2.png")),
                          (*it).name(), itemId);
    } else {
      insertItem(empty, (*it).name(), itemId);
    }

    if(curr.id() == (*it).id()) {
      selectedItemId = itemId;
      m_currency = (*it);
    }

    itemId++;
  }
  setCurrentItem(selectedItemId);
}

const MyMoneyCurrency kMyMoneyCurrencySelector::currency(void) const
{
  QValueList<MyMoneyCurrency> list = MyMoneyFile::instance()->currencyList();
  return list[currentItem()];
}

void kMyMoneyCurrencySelector::setCurrency(const MyMoneyCurrency& currency)
{
  m_currency = currency;
  update(QCString("x"));
}
