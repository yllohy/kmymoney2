/***************************************************************************
                          mymoneyscheduled.cpp
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

#include "mymoneyscheduled.h"

MyMoneyScheduled::MyMoneyScheduled()
{
//  scheduledList.setAutoDelete(true);
}

MyMoneyScheduled::~MyMoneyScheduled()
{
}

bool MyMoneyScheduled::operator == (const MyMoneyScheduled& right)
{
  return scheduledList == right.scheduledList;
}

bool MyMoneyScheduled::addScheduled(const paymentTypeE paymentType, bool fixed,
  const MyMoneyScheduled::occurenceE occurence, const MyMoneyScheduled::typeE type,
  const int year, const int month, const int day, const MyMoneyTransaction transaction)
{
  s_scheduleData *sd = new s_scheduleData;
  sd->m_occurence = occurence;
  sd->m_type = type;
  sd->m_year = year;
  sd->m_month = month;
  sd->m_day = day;
  sd->m_transaction = transaction;
  sd->m_paymentType = paymentType;
  sd->m_fixed = fixed;
  scheduledList.append(sd);
}

QList<MyMoneyScheduled::s_scheduleData> MyMoneyScheduled::getScheduled(const typeE type,
  const paymentTypeE paymentType, const MyMoneyScheduled::occurenceE occurence)
{
  QList<MyMoneyScheduled::s_scheduleData> list;
  s_scheduleData *sd;
  for (sd=scheduledList.first(); sd!=0; sd=scheduledList.next()) {
    if (type == sd->m_type)
      if ( occurence != OCCUR_ANY)
        list.append(sd);
  }

  return list;
}

QList<MyMoneyScheduled::s_scheduleData> MyMoneyScheduled::getOverdue(const MyMoneyScheduled::typeE type,
  const paymentTypeE paymentType, const MyMoneyScheduled::occurenceE ocurrence)
{
}

QList<MyMoneyScheduled::s_scheduleData> MyMoneyScheduled::getScheduledDates(const MyMoneyScheduled::typeE type,
  const QDate start, const QDate end, const paymentTypeE paymentType, const MyMoneyScheduled::occurenceE ocurrence)
{
}

bool MyMoneyScheduled::anyOverdue(const paymentTypeE paymentType, const MyMoneyScheduled::typeE type)
{
}

bool MyMoneyScheduled::anyScheduled(const paymentTypeE paymentType, const MyMoneyScheduled::typeE type)
{
  if (type==TYPE_ANY)
    return scheduledList.isEmpty();
}

bool MyMoneyScheduled::anyScheduledDates(const paymentTypeE paymentType, const MyMoneyScheduled::typeE type, const QDate start, const QDate end)
{
}
