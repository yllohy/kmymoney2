/***************************************************************************
                         kmymoneyaccounttree.h  -  description
                            -------------------
   begin                : Sat Jan 1 2005
   copyright            : (C) 2005 by Thomas Baumgart
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

#ifndef KMYMONEYACCOUNTTREE_H
#define KMYMONEYACCOUNTTREE_H

// ----------------------------------------------------------------------------
// QT Includes

class QDragObject;

// ----------------------------------------------------------------------------
// KDE Includes

#include <klistview.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "../mymoney/mymoneyaccount.h"

class kMyMoneyAccountTree : public KListView
{
  Q_OBJECT
public:
  kMyMoneyAccountTree(QWidget* parent, const char *name);
  virtual ~kMyMoneyAccountTree() {};

protected:
  virtual bool acceptDrag (QDropEvent* event) const;
  virtual void startDrag();
  // virtual void contentsDropEvent(QDropEvent*);

protected slots:
  void slotObjectDropped(QDropEvent* event, QListViewItem* parent, QListViewItem* after);
};

#endif

