/***************************************************************************
                          ktransactionlistitem.cpp
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
#include <kglobal.h>
#include <klocale.h>
#include "ktransactionlistitem.h"

KTransactionListItem::KTransactionListItem(QListView *parent, MyMoneyTransaction transaction, MyMoneyMoney previousAmount )
 : QListViewItem(parent)
{
  QString colText;

  m_transaction = transaction;

  switch (m_transaction.method()) {
    case MyMoneyTransaction::Cheque:
      setText(0, "Cheque");
      break;
    case MyMoneyTransaction::Deposit:
      setText(0, "Deposit");
      break;
    case MyMoneyTransaction::Transfer:
      setText(0, "Transfer");
      break;
    case MyMoneyTransaction::Withdrawal:
      setText(0, "Withdrawal");
      break;
    case MyMoneyTransaction::ATM:
      setText(0, "ATM");
      break;
  }
  setText(1, KGlobal::locale()->formatDate(m_transaction.date(), true));
  setText(2, m_transaction.number());
  setText(3, m_transaction.memo());
  setText(4, m_transaction.categoryMajor());

  QString cLet;
  switch (m_transaction.state()) {
    case MyMoneyTransaction::Cleared:
      setText(5, "C");
      break;
    case MyMoneyTransaction::Reconciled:
      setText(5, "R");
      break;
    default:
      setText(5, " ");
      break;
  }

  MyMoneyMoney l_balance = previousAmount;

  if (m_transaction.type()==MyMoneyTransaction::Credit) {
    l_balance += m_transaction.amount();
  }
  else {
    l_balance -= m_transaction.amount();
  }

  setText(6, ((m_transaction.type()==MyMoneyTransaction::Credit) ? KGlobal::locale()->formatMoney(m_transaction.amount().amount()) : QString("")));
  setText(7, ((m_transaction.type()==MyMoneyTransaction::Debit) ? KGlobal::locale()->formatMoney(m_transaction.amount().amount()) : QString("")));
  setText(8, KGlobal::locale()->formatMoney(l_balance.amount()));
}

KTransactionListItem::~KTransactionListItem()
{
}

MyMoneyTransaction KTransactionListItem::transaction(void)
{
  return m_transaction;
}

void KTransactionListItem::paintCell( QPainter * p, const QColorGroup & cg, int column, int width, int align )
{
  // Future addition for in place editing ?
/*
  switch (column) {
    case 0:
      qDebug("Painting for column 0");
      break;
    case 1:
      qDebug("Painting for column 1");
      break;
    case 2:
      qDebug("Painting for column 2");
      break;
    case 3:
      qDebug("Painting for column 3");
      break;
    case 4:
      qDebug("Painting for column 4");
      break;
    case 5:
      qDebug("Painting for column 5");
      break;
    case 6:
      qDebug("Painting for column 6");
      break;
    case 7:
      qDebug("Painting for column 7");
      break;
    case 8:
      qDebug("Painting for column 8");
      break;
  }
*/
  QListViewItem::paintCell(p, cg, column, width, align);
}
