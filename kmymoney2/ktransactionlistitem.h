/***************************************************************************
                          ktransactionlistitem.h
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes
    email                : Michael.Edwardes@students.dmu.ac.uk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KTRANSACTIONLISTITEM_H
#define KTRANSACTIONLISTITEM_H

#include <qwidget.h>
#include <qlistview.h>
#include "./mymoney/mymoneytransaction.h"

/**
  *@author Michael Edwardes
  */

class KTransactionListItem : public QListViewItem  {
//   Q_OBJECT
  MyMoneyTransaction m_transaction;
protected:
  void paintCell( QPainter * p, const QColorGroup & cg, int column, int width, int align );

public:
  KTransactionListItem(QListView *parent, MyMoneyTransaction transaction, MyMoneyMoney previousAmount );
	~KTransactionListItem();
	MyMoneyTransaction transaction(void);
};

#endif
