/***************************************************************************
                          kbanklistitem.cpp
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

#include "kschedulelistitem.h"

KScheduleListItem::KScheduleListItem(QListView *parent, MyMoneyScheduled::s_scheduleData scheduleData, QString bankName, QString accountName )
 : QListViewItem(parent)
{
  m_data = scheduleData;
  QString cellText;

  setText(0, KGlobal::locale()->formatDate(QDate(scheduleData.m_year, scheduleData.m_month, scheduleData.m_day)));
  setText(1, bankName);
  setText(2, accountName);

  switch (scheduleData.m_type) {
    case MyMoneyScheduled::TYPE_BILL:
      cellText = i18n("Withdrawal");
      break;
    case MyMoneyScheduled::TYPE_DEPOSIT:
      cellText = i18n("Deposit");
      break;
    case MyMoneyScheduled::TYPE_TRANSFER:
      cellText = i18n("Transfer");
      break;
    default:
      cellText = i18n("Unknown");
      break;
  }
  setText(3, cellText);

  switch (scheduleData.m_occurence) {
    case MyMoneyScheduled::OCCUR_DAILY:
      cellText = i18n("Daily");
      break;
    case MyMoneyScheduled::OCCUR_WEEKLY:
      cellText = i18n("Weekly");
      break;
    case MyMoneyScheduled::OCCUR_FORTNIGHTLY:
      cellText = i18n("Fortnightly");
      break;
    case MyMoneyScheduled::OCCUR_MONTHLY:
      cellText = i18n("Monthly");
      break;
    case MyMoneyScheduled::OCCUR_QUARTER:
      cellText = i18n("Every 3 Months");
      break;
    case MyMoneyScheduled::OCCUR_FOURMONTH:
      cellText = i18n("Every 4 Months");
      break;
    case MyMoneyScheduled::OCCUR_YEARLY:
      cellText = i18n("Yearly");
      break;
    default:
      cellText = i18n("Unknown");
      break;
  }
  setText(4, cellText);
  setText(5, KGlobal::locale()->formatMoney(scheduleData.m_transaction.amount().amount()));
  setText(6, "???");
}

KScheduleListItem::~KScheduleListItem()
{
}

MyMoneyScheduled::s_scheduleData KScheduleListItem::data(void)
{
  return m_data;
}
