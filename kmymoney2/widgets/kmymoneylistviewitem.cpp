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
#include "../kmymoneysettings.h"

KMyMoneyListViewItem::KMyMoneyListViewItem(QListView* parent, const QString& txt, const QString& key, const QCString& id) :
  KListViewItem(parent, txt),
  m_key(key),
  m_id(id)
{
  if(key.isEmpty())
    m_key = txt;
}

KMyMoneyListViewItem::KMyMoneyListViewItem(QListViewItem* parent, const QString& txt, const QString& key, const QCString& id) :
  KListViewItem(parent, txt),
  m_key(key),
  m_id(id)
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
  return isAlternate() ? KMyMoneySettings::listBGColor() : KMyMoneySettings::listColor();
}

#include "kmymoneylistviewitem.moc"
