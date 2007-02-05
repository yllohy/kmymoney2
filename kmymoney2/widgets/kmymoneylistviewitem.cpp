/***************************************************************************
                          kmymoneylistviewitem  -  description
                             -------------------
    begin                : Wed Jun 28 2006
    copyright            : (C) 2000-2006 by Michael Edwardes
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

#include <qpalette.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneylistviewitem.h"
#include "kmymoneychecklistitem.h"
#include "../kmymoneyglobalsettings.h"

KMyMoneyListViewItem::KMyMoneyListViewItem(QListView* parent, const QString& txt, const QString& key, const QCString& id) :
  KListViewItem(parent, txt),
  m_key(key),
  m_id(id),
  m_isKnown(0),
  m_isOdd(0)
{
  if(key.isEmpty())
    m_key = txt;
}

KMyMoneyListViewItem::KMyMoneyListViewItem(QListViewItem* parent, const QString& txt, const QString& key, const QCString& id) :
  KListViewItem(parent, txt),
  m_key(key),
  m_id(id),
  m_isKnown(0),
  m_isOdd(0)
{
  if(key.isEmpty())
    m_key = txt;
}

KMyMoneyListViewItem::~KMyMoneyListViewItem()
{
}

void KMyMoneyListViewItem::paintCell(QPainter *p, const QColorGroup &cg, int column, int width, int alignment)
{
  QColorGroup _cg = cg;
  _cg.setColor(QColorGroup::Base, backgroundColor());

  // make sure to bypass KListViewItem::paintCell() as
  // we don't like it's logic - that's why we do this
  // here ;-)    (ipwizard)
  QListViewItem::paintCell(p, _cg, column, width, alignment);
}

const QColor KMyMoneyListViewItem::backgroundColor()
{
  return isAlternate() ? KMyMoneyGlobalSettings::listBGColor() : KMyMoneyGlobalSettings::listColor();
}

bool KMyMoneyListViewItem::isAlternate(void)
{
// logic taken from KListViewItem::isAlternate()
  KMyMoneyCheckListItem* ciAbove;
  KMyMoneyListViewItem* liAbove;
  ciAbove = dynamic_cast<KMyMoneyCheckListItem*> (itemAbove());
  liAbove = dynamic_cast<KMyMoneyListViewItem*> (itemAbove());

  m_isKnown = ciAbove ? ciAbove->m_isKnown : (liAbove ? liAbove->m_isKnown : true);
  if(m_isKnown) {
    m_isOdd = ciAbove ? !ciAbove->m_isOdd : (liAbove ? !liAbove->m_isOdd : false);
  } else {
    KMyMoneyCheckListItem* clItem;
    KMyMoneyListViewItem* liItem;
    bool previous = true;
    if(QListViewItem::parent()) {
      clItem = dynamic_cast<KMyMoneyCheckListItem *>(QListViewItem::parent());
      liItem = dynamic_cast<KMyMoneyListViewItem*>(QListViewItem::parent());
      if(clItem)
        previous = clItem->m_isOdd;
      else
        previous = liItem->m_isOdd;
      clItem = dynamic_cast<KMyMoneyCheckListItem *>(QListViewItem::parent()->firstChild());
      liItem = dynamic_cast<KMyMoneyListViewItem*>(QListViewItem::parent()->firstChild());
    } else {
      clItem = dynamic_cast<KMyMoneyCheckListItem *>(listView()->firstChild());
      liItem = dynamic_cast<KMyMoneyListViewItem*>(listView()->firstChild());
    }
    while(clItem || liItem) {
      if(clItem) {
        clItem->m_isOdd = previous = !previous;
        clItem->m_isKnown = true;
        clItem = dynamic_cast<KMyMoneyCheckListItem *>(clItem->nextSibling());
        liItem = dynamic_cast<KMyMoneyListViewItem *>(clItem->nextSibling());
      } else if(liItem) {
        liItem->m_isOdd = previous = !previous;
        liItem->m_isKnown = true;
        clItem = dynamic_cast<KMyMoneyCheckListItem *>(liItem->nextSibling());
        liItem = dynamic_cast<KMyMoneyListViewItem *>(liItem->nextSibling());
      }
    }
  }
  return m_isOdd;
}

#include "kmymoneylistviewitem.moc"
