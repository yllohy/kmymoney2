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

MyMoneyScheduledTransaction::MyMoneyScheduledTransaction()
{
}

MyMoneyScheduledTransaction::MyMoneyScheduledTransaction(MyMoneyAccount *parent, const long id, transactionMethod methodType, const QString& number, const QString& memo,
  const MyMoneyMoney& amount, const QDate& date, const QString& categoryMajor, const QString& categoryMinor, const QString& atmName,
  const QString& fromTo, const QString& bankFrom, const QString& bankTo, stateE state)
{
}

MyMoneyScheduledTransaction::~MyMoneyScheduledTransaction()
{
}


MyMoneyScheduledList::MyMoneyScheduledList()
{
//  scheduledList.setAutoDelete(true);
}

MyMoneyScheduledList::~MyMoneyScheduledList()
{
}

bool MyMoneyScheduledList::operator == (const MyMoneyScheduledList& right)
{
  return scheduledList == right.scheduledList;
}

bool MyMoneyScheduledList::addScheduled(const MyMoneyScheduledTransaction& transaction)
{
/*
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
*/
}

QList<MyMoneyScheduledTransaction> MyMoneyScheduledList::getScheduled(
  const MyMoneyScheduledTransaction::typeE type,
  const MyMoneyScheduledTransaction::paymentTypeE paymentType,
  const MyMoneyScheduledTransaction::occurenceE occurence)
{
/*
  QList<MyMoneyScheduled::s_scheduleData> list;
  s_scheduleData *sd;
  for (sd=scheduledList.first(); sd!=0; sd=scheduledList.next()) {
    if (type == sd->m_type)
      if ( occurence != OCCUR_ANY)
        list.append(sd);
  }

  return list;
*/
}

QList<MyMoneyScheduledTransaction> MyMoneyScheduledList::getOverdue(
  const MyMoneyScheduledTransaction::typeE type,
  const MyMoneyScheduledTransaction::paymentTypeE paymentType,
  const MyMoneyScheduledTransaction::occurenceE ocurrence)
{
}

bool MyMoneyScheduledList::anyOverdue(
  const MyMoneyScheduledTransaction::paymentTypeE paymentType,
  const MyMoneyScheduledTransaction::typeE type)
{
}

bool MyMoneyScheduledList::anyScheduled(
  const MyMoneyScheduledTransaction::paymentTypeE paymentType,
  const MyMoneyScheduledTransaction::typeE type)
{
  if (type==MyMoneyScheduledTransaction::TYPE_ANY)
    return scheduledList.isEmpty();
}
