/***************************************************************************
                          kmymoneychecklistitem
                             -------------------
    begin                : Wed Jun 28 2006
    copyright            : (C) 2006 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
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

#include <qfont.h>
#include <qpainter.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneychecklistitem.h"
#include "../kmymoneysettings.h"

KMyMoneyCheckListItem::KMyMoneyCheckListItem(QListView* parent, const QString& txt, const QString& key, const QCString& id, Type type) :
  QCheckListItem(parent, txt, type),
  m_key(key),
  m_id(id)
{
  setOn(true);
  m_known = false;
  if(key.isEmpty())
    m_key = txt;
}

KMyMoneyCheckListItem::KMyMoneyCheckListItem(QListViewItem* parent, const QString& txt, const QString& key, const QCString& id, Type type) :
  QCheckListItem(parent, txt, type),
  m_key(key),
  m_id(id)
{
  setOn(true);
  m_known = false;
  if(key.isEmpty())
    m_key = txt;
}

KMyMoneyCheckListItem::KMyMoneyCheckListItem(QListView* parent, QListViewItem* after, const QString& txt, const QString& key, const QCString& id, Type type) :
  QCheckListItem(parent, after, txt, type),
  m_key(key),
  m_id(id)
{
  setOn(true);
  m_known = false;
  if(key.isEmpty())
    m_key = txt;
}

KMyMoneyCheckListItem::~KMyMoneyCheckListItem()
{
}

void KMyMoneyCheckListItem::stateChange(bool state)
{
  emit stateChanged(state);
}

void KMyMoneyCheckListItem::paintCell(QPainter *p, const QColorGroup &cg, int column, int width, int alignment)
{
  QColorGroup _cg = cg;
  _cg.setColor(QColorGroup::Base, backgroundColor());

  // write the groups in bold
  QFont f = p->font();
  f.setBold(!isSelectable());
  p->setFont(f);

  QCheckListItem::paintCell(p, _cg, column, width, alignment);
}

const QColor KMyMoneyCheckListItem::backgroundColor()
{
  return isAlternate() ? KMyMoneySettings::listBGColor() : KMyMoneySettings::listColor();
}

bool KMyMoneyCheckListItem::isAlternate(void)
{
// logic taken from KListViewItem::isAlternate()
  KMyMoneyCheckListItem* above;
  above = dynamic_cast<KMyMoneyCheckListItem*> (itemAbove());
  m_known = above ? above->m_known : true;
  if(m_known) {
    m_odd = above ? !above->m_odd : false;
  } else {
    KMyMoneyCheckListItem* item;
    bool previous = true;
    if(QListViewItem::parent()) {
      item = dynamic_cast<KMyMoneyCheckListItem *>(QListViewItem::parent());
      previous = item->m_odd;
      item = dynamic_cast<KMyMoneyCheckListItem *>(QListViewItem::parent()->firstChild());
    } else {
      item = dynamic_cast<KMyMoneyCheckListItem *>(listView()->firstChild());
    }
    while(item) {
      item->m_odd = previous = !previous;
      item->m_known = true;
      item = dynamic_cast<KMyMoneyCheckListItem *>(item->nextSibling());
    }
  }
  return m_odd;
}

#include "kmymoneychecklistitem.moc"
