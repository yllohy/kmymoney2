/***************************************************************************
                          mymoneyscheduled.cpp
                             -------------------
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
// Project Includes

#include "mymoneyscheduled.h"
#include "mymoneyexception.h"

bool MyMoneySchedule::validate(bool id_check) const
{
  /* Check the supplied instance is valid...
   *
   * To be valid it must not have the id set and have the following fields set:
   *
   * m_occurence
   * m_type
   * m_startDate
   * m_paymentType
   * m_transaction
  */
  if (  (id_check && !m_id.isEmpty()) ||
        m_occurence == OCCUR_ANY ||
        m_type == TYPE_ANY ||
        m_startDate.year() == 1900 ||
        m_paymentType == STYPE_ANY ||
        m_transaction.splitCount() == 0 ||
        m_lastPayment.year() == 1900)
  {
    return false;
  }
  else
  {
    // Check the dates
    if (!m_startDate.isValid() || !m_lastPayment.isValid())
      return false;

    // Check the payment types
    switch (m_type)
    {
      case TYPE_BILL:
        if (m_paymentType == STYPE_DIRECTDEPOSIT || m_paymentType == STYPE_MANUALDEPOSIT)
          return false;
        break;
      case TYPE_DEPOSIT:
        if (m_paymentType == STYPE_DIRECTDEBIT || m_paymentType == STYPE_WRITECHEQUE)
          return false;
        break;
      case TYPE_TRANSFER:
        ; // Do all payment types apply?
        break;
    }

    // Check the transactions remaining
    if (m_willEnd)
    {
      if (!m_endDate.isValid() || (m_transactionsRemaining == 0))
        return false;
    }

    return true;
  }
}

QDate MyMoneySchedule::nextPayment(void) const
{
  QDate today(QDate::currentDate());
  QDate paymentDate(m_lastPayment);

  switch (m_occurence)
  {
    case OCCUR_ONCE:
      return m_startDate;
      break;
    case OCCUR_DAILY:
      return today.addDays(1);
      break;
    case OCCUR_WEEKLY:
      while (paymentDate <= today)
      {
        paymentDate = paymentDate.addDays(7);
      }
      return paymentDate;
      break;
    case OCCUR_FORTNIGHTLY:
    case OCCUR_EVERYOTHERWEEK:
      while (paymentDate <= today)
      {
        paymentDate = paymentDate.addDays(14);
      }
      return paymentDate;
      break;
    case OCCUR_EVERYFOURWEEKS:
      while (paymentDate <= today)
      {
        paymentDate = paymentDate.addDays(28);
      }
      return paymentDate;
      break;
    case OCCUR_MONTHLY:
      while (paymentDate <= today)
      {
        paymentDate = paymentDate.addMonths(1);
      }
      return paymentDate;
      break;
    case OCCUR_EVERYOTHERMONTH:
      while (paymentDate <= today)
      {
        paymentDate = paymentDate.addMonths(2);
      }
      return paymentDate;
      break;
    case OCCUR_EVERYTHREEMONTHS:
      while (paymentDate <= today)
      {
        paymentDate = paymentDate.addMonths(3);
      }
      return paymentDate;
      break;
    case OCCUR_QUARTERLY:
    case OCCUR_EVERYFOURMONTHS:
      while (paymentDate <= today)
      {
        paymentDate = paymentDate.addMonths(4);
      }
      return paymentDate;
      break;
    case OCCUR_TWICEYEARLY:
      while (paymentDate <= today)
      {
        paymentDate = paymentDate.addMonths(6);
      }
      return paymentDate;
      break;
    case OCCUR_YEARLY:
      while (paymentDate <= today)
      {
        paymentDate = paymentDate.addYears(1);
      }
      return paymentDate;
      break;
    case OCCUR_EVERYOTHERYEAR:
      while (paymentDate <= today)
      {
        paymentDate = paymentDate.addYears(2);
      }
      return paymentDate;
      break;
  }

  return QDate(1900, 1, 1);
}

QValueList<QDate> MyMoneySchedule::paymentDates(const QDate& startDate, const QDate& endDate) const
{
  QDate today(endDate);
  QDate paymentDate(startDate);
  QValueList<QDate> theDates;

  if (m_startDate < startDate || m_startDate > endDate)
    return theDates;

  switch (m_occurence)
  {
    case OCCUR_ONCE:
      if (m_startDate >= startDate && m_startDate <= endDate)
        theDates.append(m_startDate);
      break;
    case OCCUR_DAILY:
      while (paymentDate <= today)
      {
        paymentDate = paymentDate.addDays(1);
        theDates.append(paymentDate);
      }
      break;
    case OCCUR_WEEKLY:
      while (paymentDate <= today)
      {
        paymentDate = paymentDate.addDays(7);
        theDates.append(paymentDate);
      }
      break;
    case OCCUR_FORTNIGHTLY:
    case OCCUR_EVERYOTHERWEEK:
      while (paymentDate <= today)
      {
        paymentDate = paymentDate.addDays(14);
        theDates.append(paymentDate);
      }
      break;
    case OCCUR_EVERYFOURWEEKS:
      while (paymentDate <= today)
      {
        paymentDate = paymentDate.addDays(28);
        theDates.append(paymentDate);
      }
      break;
    case OCCUR_MONTHLY:
      while (paymentDate <= today)
      {
        paymentDate = paymentDate.addMonths(1);
        theDates.append(paymentDate);
      }
      break;
    case OCCUR_EVERYOTHERMONTH:
      while (paymentDate <= today)
      {
        paymentDate = paymentDate.addMonths(2);
        theDates.append(paymentDate);
      }
      break;
    case OCCUR_EVERYTHREEMONTHS:
      while (paymentDate <= today)
      {
        paymentDate = paymentDate.addMonths(3);
        theDates.append(paymentDate);
      }
      break;
    case OCCUR_QUARTERLY:
    case OCCUR_EVERYFOURMONTHS:
      while (paymentDate <= today)
      {
        paymentDate = paymentDate.addMonths(4);
        theDates.append(paymentDate);
      }
      break;
    case OCCUR_TWICEYEARLY:
      while (paymentDate <= today)
      {
        paymentDate = paymentDate.addMonths(6);
        theDates.append(paymentDate);
      }
      break;
    case OCCUR_YEARLY:
      while (paymentDate <= today)
      {
        paymentDate = paymentDate.addYears(1);
        theDates.append(paymentDate);
      }
      break;
    case OCCUR_EVERYOTHERYEAR:
      while (paymentDate <= today)
      {
        paymentDate = paymentDate.addYears(2);
        theDates.append(paymentDate);
      }
      break;
  }

  return theDates;
}


MyMoneyScheduled* MyMoneyScheduled::m_instance = 0;

MyMoneyScheduled* MyMoneyScheduled::instance()
{
  if(m_instance == 0) {
    m_instance = new MyMoneyScheduled;
  }
  return m_instance;
}

MyMoneyScheduled::MyMoneyScheduled()
{
    m_nextId = 1;
}

MyMoneyScheduled::~MyMoneyScheduled()
{
}

QString MyMoneyScheduled::addSchedule(const MyMoneySchedule& schedule)
{
  if (!schedule.validate())
  {
    throw new MYMONEYEXCEPTION("Invalid schedule instance when adding to collection.");
    return "";
  }

  QString newId;
  newId.sprintf("SCHED%05d", m_nextId++);

  MyMoneySchedule schedCopy(schedule);
  schedCopy.setId(newId);

  m_scheduled.insert(newId, schedCopy);

  return newId;
}

void MyMoneyScheduled::removeSchedule(const QString& scheduleId)
{
  if ((m_scheduled.find(scheduleId)) == m_scheduled.end())
  {
    throw new MYMONEYEXCEPTION("Supplied schedule ID does not exist in removeSchedule");
    return;
  }

  m_scheduled.remove(scheduleId);
}

QString MyMoneyScheduled::replaceSchedule(const QString& scheduleId, const MyMoneySchedule& schedule)
{
  QMap<QString, MyMoneySchedule>::Iterator it = m_scheduled.find(scheduleId);
  if (it == m_scheduled.end())
  {
    throw new MYMONEYEXCEPTION("Supplied schedule ID does not exist in replaceSchedule");
    return "";
  }

  if (!schedule.validate(false))
  {
    throw new MYMONEYEXCEPTION("Invalid schedule instance when replacing in collection.");
    return "";
  }

  m_scheduled.insert(scheduleId, schedule, true);

  return scheduleId;
}

MyMoneySchedule MyMoneyScheduled::getSchedule(const QString& scheduleId)
{
  MyMoneySchedule sched;
  
  QMap<QString, MyMoneySchedule>::Iterator it = m_scheduled.find(scheduleId);
  if (it == m_scheduled.end())
  {
    throw new MYMONEYEXCEPTION("Supplied schedule ID does not exist in removeSchedule");
    return sched;
  }

  return it.data();
}

QValueList<QString> MyMoneyScheduled::getScheduled(const MyMoneySchedule::typeE type,
  const MyMoneySchedule::paymentTypeE paymentType,
  const MyMoneySchedule::occurenceE occurence)
{
  QValueList<QString> idList;

  QMap<QString, MyMoneySchedule>::Iterator it;
  for (it = m_scheduled.begin(); it != m_scheduled.end(); ++it)
  {
    if (  (type==MyMoneySchedule::TYPE_ANY || it.data().type() == type) &&
          (paymentType==MyMoneySchedule::STYPE_ANY || it.data().paymentType() == paymentType) &&
          (occurence==MyMoneySchedule::OCCUR_ANY || it.data().occurence() == occurence) )
    {
      idList.append(it.key());
    }
  }

  return idList;
}

QValueList<QString> MyMoneyScheduled::getScheduled(const QDate& startDate, const QDate& endDate,
  const MyMoneySchedule::typeE type,
  const MyMoneySchedule::paymentTypeE paymentType,
  const MyMoneySchedule::occurenceE occurence)
{
  QValueList<QString> idList;

  QMap<QString, MyMoneySchedule>::Iterator it;
  for (it = m_scheduled.begin(); it != m_scheduled.end(); ++it)
  {
    if (  (type==MyMoneySchedule::TYPE_ANY || it.data().type() == type) &&
          (paymentType==MyMoneySchedule::STYPE_ANY || it.data().paymentType() == paymentType) &&
          (occurence==MyMoneySchedule::OCCUR_ANY || it.data().occurence() == occurence) )
    {
      QString s;
      s.sprintf("\nchecking %s, %s", startDate.toString().latin1(), endDate.toString().latin1());
      QValueList<QDate> payments = it.data().paymentDates(startDate, endDate);
      if (payments.size()>=1)
        idList.append(it.key());
    }
  }

  return idList;
}

QValueList<QString> MyMoneyScheduled::getOverdue(const MyMoneySchedule::typeE type,
  const MyMoneySchedule::paymentTypeE paymentType,
  const MyMoneySchedule::occurenceE occurence)
{
  // Expensive but works for now.  Could maybe put a helper method in
  // MyMoneySchedule
  QValueList<QString> theList;
  
  QMap<QString, MyMoneySchedule>::Iterator it;
  for (it = m_scheduled.begin(); it != m_scheduled.end(); ++it)
  {
    if (  (type==MyMoneySchedule::TYPE_ANY || it.data().type() == type) &&
          (paymentType==MyMoneySchedule::STYPE_ANY || it.data().paymentType() == paymentType) &&
          (occurence==MyMoneySchedule::OCCUR_ANY || it.data().occurence() == occurence) )
    {
      QValueList<QDate> list = it.data().paymentDates(it.data().lastPayment(), QDate::currentDate());
      if (list.size() >= 2)
      {
        theList.append(it.key());
      }
    }
  }

  return theList;
}

bool MyMoneyScheduled::anyOverdue(const MyMoneySchedule::typeE type,
  const MyMoneySchedule::occurenceE occurence,
  const MyMoneySchedule::paymentTypeE paymentType)
{
  // Expensive but works for now.  Could maybe put a helper method in
  // MyMoneySchedule
  QMap<QString, MyMoneySchedule>::Iterator it;
  for (it = m_scheduled.begin(); it != m_scheduled.end(); ++it)
  {
    if (  (type==MyMoneySchedule::TYPE_ANY || it.data().type() == type) &&
          (paymentType==MyMoneySchedule::STYPE_ANY || it.data().paymentType() == paymentType) &&
          (occurence==MyMoneySchedule::OCCUR_ANY || it.data().occurence() == occurence) )
    {
      QValueList<QDate> list = it.data().paymentDates(it.data().lastPayment(), QDate::currentDate());
      if (list.size() >= 2)
      {
        // abort early...
        return true;
      }
    }
  }

  return false;
}

bool MyMoneyScheduled::anyScheduled(const MyMoneySchedule::typeE type,
  const MyMoneySchedule::occurenceE occurence,
  const MyMoneySchedule::paymentTypeE paymentType)
{
  unsigned int nCounter=0;
  
  QMap<QString, MyMoneySchedule>::Iterator it;
  for (it = m_scheduled.begin(); it != m_scheduled.end(); ++it)
  {
    if (  (type==MyMoneySchedule::TYPE_ANY || it.data().type() == type) &&
          (occurence==MyMoneySchedule::OCCUR_ANY || it.data().occurence() == occurence) &&
          (paymentType==MyMoneySchedule::STYPE_ANY || it.data().paymentType() == paymentType) )
    {
      nCounter++;
    }
  }

  return (nCounter > 0);
}
