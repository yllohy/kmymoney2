/***************************************************************************
                          kscheduledlistitem.cpp  -  description
                             -------------------
    begin                : Sun Jan 27 2002
    copyright            : (C) 2000-2002 by Michael Edwardes
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

// ----------------------------------------------------------------------------
// Project Includes

#include "kscheduledlistitem.h"
#include "../mymoney/mymoneyfile.h"

KScheduledListItem::KScheduledListItem(QListView *parent, const char *name )
 : QListViewItem(parent,name)
{
  setText(0, name);
}

KScheduledListItem::KScheduledListItem(KScheduledListItem *parent, const QCString accountId, const MyMoneySchedule& schedule)
 : QListViewItem(parent)
{
//  type, payee, amount, due date, freq, payment method.
  try
  {
    m_id = schedule.id();
    MyMoneyTransaction transaction = schedule.transaction();
    setText(0, schedule.name());
    setText(1, MyMoneyFile::instance()->payee(transaction.split(accountId).payeeId()).name());
    MyMoneyMoney amount = transaction.split(accountId).value();
    if (amount < 0)
      amount = -amount;
    setText(2, amount.formatMoney());
    setText(3, schedule.nextPayment().toString());
    setText(4, schedule.occurenceToString());
    setText(5, schedule.paymentMethodToString());
  }
  catch (MyMoneyException *e)
  {
    setText(0, "Error:");
    setText(1, e->what());
    delete e;
  }
}

KScheduledListItem::~KScheduledListItem()
{
}
