/***************************************************************************
                          kReconcileListitem.cpp
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
#include <kglobal.h>
#include <klocale.h>
#include "kreconcilelistitem.h"

KReconcileListItem::KReconcileListItem(QListView *parent, MyMoneyTransaction *transaction )
 : QListViewItem(parent)
{
  QString colText;

  m_transaction = transaction;

  setText(0, KGlobal::locale()->formatDate(m_transaction->date(), true));
	setText(1, m_transaction->number());
  setText(2, m_transaction->memo());
  setText(3, KGlobal::locale()->formatMoney(m_transaction->amount().amount()));

  QString tmp;
  switch (m_transaction->state()) {
    case MyMoneyTransaction::Reconciled:
      tmp = "R";
      break;
    case MyMoneyTransaction::Cleared:
      tmp = "C";
      break;
    default:
      tmp = " ";
      break;
  }

  setText(4, tmp);
}

KReconcileListItem::~KReconcileListItem()
{
}

void KReconcileListItem::setReconciled(bool rec)
{
	QString temp;
  if(rec == true)
  {
   	m_transaction->setState(MyMoneyTransaction::Reconciled);
    temp = "R";
  }
  else
  {
    m_transaction->setState(MyMoneyTransaction::Unreconciled);
    temp = " ";
  }
  setText(4,temp);

}

MyMoneyTransaction* KReconcileListItem::transaction(void)
{
  return m_transaction;
}
