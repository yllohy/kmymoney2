/***************************************************************************
                          kmymoneylistviewitem.h
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

#ifndef KMYMONEYLISTVIEWITEM_H
#define KMYMONEYLISTVIEWITEM_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qobject.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klistview.h>

// ----------------------------------------------------------------------------
// Project Includes

class KMyMoneyCheckListItem;

/**
  * This class implements a derived version of a QListViewItem that
  * allows the storage of an engine object id with the object
  *
  * @author Thomas Baumgart
  */
class KMyMoneyListViewItem : public QObject, public KListViewItem
{
  friend class KMyMoneyCheckListItem;

  Q_OBJECT
public:
  KMyMoneyListViewItem(QListView *parent, const QString& txt, const QString& key, const QCString& id);
  KMyMoneyListViewItem(QListViewItem *parent, const QString& txt, const QString& key, const QCString& id);
  ~KMyMoneyListViewItem();

  const QCString& id(void) const { return m_id; };

  /**
    * use my own paint method
    */
  void paintCell(QPainter *p, const QColorGroup &cg, int column, int width, int alignment);

  /**
    * use my own backgroundColor method
    */
  const QColor backgroundColor();

  /**
    * This method returns a const reference to the key passed to the constructor
    */
  const QString& key(void) const { return m_key; }


  /**
    * Reimplemented for internal reasons
    */
  bool isAlternate(void);

private:
  QString              m_key;
  QCString             m_id;
  // copied from KListViewItem()
  unsigned int         m_isOdd : 1;
  unsigned int         m_isKnown : 1;
  unsigned int         m_unused : 30;

};

#endif
