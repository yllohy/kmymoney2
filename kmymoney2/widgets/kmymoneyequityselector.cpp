/***************************************************************************
                          kmymoneyequityselector.cpp  -  description
                             -------------------
    begin                : Wed 02 June 2004
    copyright            : (C) 2004 by Thomas Baumgart
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

#include <klocale.h>
#include <kpushbutton.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyequityselector.h"
#include "../mymoney/mymoneyfile.h"

kMyMoneyEquitySelector::kMyMoneyEquitySelector(QWidget *parent, const char *name, QWidget::WFlags flags) :
  kMyMoneyAccountSelector(parent, name, flags, false)
{
}

kMyMoneyEquitySelector::~kMyMoneyEquitySelector()
{
}

const int kMyMoneyEquitySelector::loadList(void)
{
  QValueList<MyMoneySecurity> list;
  QValueList<MyMoneySecurity>::ConstIterator it_l;
  MyMoneyFile* file = MyMoneyFile::instance();
  int count = 0;

  m_listView->clear();

  if(m_selMode == QListView::Multi) {
    m_incomeCategoriesButton->hide();
    m_expenseCategoriesButton->hide();
  }

  kMyMoneyCheckListItem* item = 0;
  item = new kMyMoneyCheckListItem(m_listView, i18n("Securities"), QCString(), QCheckListItem::Controller);
  list = file->securityList();

  item->setSelectable(false);
  item->setOpen(true);
  // scan all matching equities found in the engine
  for(it_l = list.begin(); it_l != list.end(); ++it_l) {
    ++count;
    newEntryFactory(item, (*it_l).tradingSymbol(), (*it_l).id());
  }

  if(m_listView->firstChild()) {
    m_listView->setCurrentItem(m_listView->firstChild());
    m_listView->clearSelection();
  }
  QWidget::update();
  return count;
}
