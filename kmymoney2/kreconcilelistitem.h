/***************************************************************************
                          kreconcilelistitem.h
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes
    email                : mte@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KRECONCILELISTITEM_H
#define KRECONCILELISTITEM_H

#include <qwidget.h>
#include <qlistview.h>
#include "./mymoney/mymoneytransaction.h"

/**
  *@author Michael Edwardes
  */

class KReconcileListItem : public QListViewItem  {
//   Q_OBJECT
  MyMoneyTransaction *m_transaction;
public:
  KReconcileListItem(QListView *parent, MyMoneyTransaction *transaction );
	~KReconcileListItem();
	MyMoneyTransaction* transaction(void);
  void setReconciled(bool rec);
};

#endif
