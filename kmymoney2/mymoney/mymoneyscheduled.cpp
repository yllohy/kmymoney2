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
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes
#include <klocale.h>

// ----------------------------------------------------------------------------
// Project Includes
#include "mymoneyscheduled.h"
#include "mymoneyexception.h"
#include "mymoneyfile.h"

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
      case TYPE_ANY:
        return false;
        break;
      case TYPE_TRANSFER:
//        if (m_paymentType == STYPE_DIRECTDEPOSIT || m_paymentType == STYPE_MANUALDEPOSIT)
//          return false;
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

QDate MyMoneySchedule::nextPayment(const QDate refDate) const
{
  QDate paymentDate(m_lastPayment);

  if(!paymentDate.isValid())
    paymentDate = m_startDate;
    
  if(m_willEnd && m_endDate < refDate
  || m_startDate > refDate)
    return QDate(1900,1,1);
      
  switch (m_occurence)
  {
    case OCCUR_ONCE:
      paymentDate = m_startDate;
      break;
      
    case OCCUR_DAILY:
      paymentDate = refDate.addDays(1);
      break;
      
    case OCCUR_WEEKLY:
      while (paymentDate <= refDate)
        paymentDate = paymentDate.addDays(7);
      break;
      
    case OCCUR_FORTNIGHTLY:
    case OCCUR_EVERYOTHERWEEK:
      while (paymentDate <= refDate)
        paymentDate = paymentDate.addDays(14);
      break;

    case OCCUR_EVERYFOURWEEKS:
      while (paymentDate <= refDate)
        paymentDate = paymentDate.addDays(28);
      break;

    case OCCUR_MONTHLY:
      while (paymentDate <= refDate)
        paymentDate = paymentDate.addMonths(1);
      break;

    case OCCUR_EVERYOTHERMONTH:
      while (paymentDate <= refDate)
        paymentDate = paymentDate.addMonths(2);
      break;

    case OCCUR_EVERYTHREEMONTHS:
      while (paymentDate <= refDate)
        paymentDate = paymentDate.addMonths(3);
      break;

    case OCCUR_QUARTERLY:
    case OCCUR_EVERYFOURMONTHS:
      while (paymentDate <= refDate)
        paymentDate = paymentDate.addMonths(4);
      break;
      
    case OCCUR_TWICEYEARLY:
      while (paymentDate <= refDate)
        paymentDate = paymentDate.addMonths(6);
      break;

    case OCCUR_YEARLY:
      while (paymentDate <= refDate)
        paymentDate = paymentDate.addYears(1);
      break;

    case OCCUR_EVERYOTHERYEAR:
      while (paymentDate <= refDate)
        paymentDate = paymentDate.addYears(2);
      break;
      
    case OCCUR_ANY:
      paymentDate = QDate(1900, 1, 1);
      break;
  }
  if(paymentDate != QDate(1900, 1, 1)) {
    if(m_willEnd && paymentDate > m_endDate)
      paymentDate = QDate(1900, 1, 1);
  }
  return paymentDate;
}

QValueList<QDate> MyMoneySchedule::paymentDates(const QDate& startDate, const QDate& endDate) const
{
  QDate today(endDate);
  QDate paymentDate(m_startDate);
  QValueList<QDate> theDates;

  if (m_willEnd && m_endDate < startDate || m_startDate > endDate)
    return theDates;

  switch (m_occurence)
  {
    case OCCUR_ONCE:
      if (m_startDate >= startDate && m_startDate <= endDate)
        theDates.append(m_startDate);
      break;
      
    case OCCUR_DAILY:
      paymentDate = startDate;
      while (paymentDate <= today)
      {
        paymentDate = paymentDate.addDays(1);
        theDates.append(paymentDate);
      }
      break;
      
    case OCCUR_WEEKLY:
      while (paymentDate < startDate)
        paymentDate = paymentDate.addDays(7);

      while (paymentDate <= today)
      {
        paymentDate = paymentDate.addDays(7);
        theDates.append(paymentDate);
      }
      break;
      
    case OCCUR_FORTNIGHTLY:
    case OCCUR_EVERYOTHERWEEK:
      while (paymentDate < startDate)
        paymentDate = paymentDate.addDays(14);
      while (paymentDate <= today)
      {
        paymentDate = paymentDate.addDays(14);
        theDates.append(paymentDate);
      }
      break;
      
    case OCCUR_EVERYFOURWEEKS:
      while (paymentDate < startDate)
        paymentDate = paymentDate.addDays(28);
      while (paymentDate <= today)
      {
        paymentDate = paymentDate.addDays(28);
        theDates.append(paymentDate);
      }
      break;
      
    case OCCUR_MONTHLY:
      while (paymentDate < startDate)
        paymentDate = paymentDate.addMonths(1);
      while (paymentDate <= today)
      {
        paymentDate = paymentDate.addMonths(1);
        theDates.append(paymentDate);
      }
      break;
      
    case OCCUR_EVERYOTHERMONTH:
      while (paymentDate < startDate)
        paymentDate = paymentDate.addMonths(2);
      while (paymentDate <= today)
      {
        paymentDate = paymentDate.addMonths(2);
        theDates.append(paymentDate);
      }
      break;
      
    case OCCUR_EVERYTHREEMONTHS:
      while (paymentDate < startDate)
        paymentDate = paymentDate.addMonths(3);
      while (paymentDate <= today)
      {
        paymentDate = paymentDate.addMonths(3);
        theDates.append(paymentDate);
      }
      break;
      
    case OCCUR_QUARTERLY:
    case OCCUR_EVERYFOURMONTHS:
      while (paymentDate < startDate)
        paymentDate = paymentDate.addMonths(4);
      while (paymentDate <= today)
      {
        paymentDate = paymentDate.addMonths(4);
        theDates.append(paymentDate);
      }
      break;
      
    case OCCUR_TWICEYEARLY:
      while (paymentDate < startDate)
        paymentDate = paymentDate.addMonths(6);
      while (paymentDate <= today)
      {
        paymentDate = paymentDate.addMonths(6);
        theDates.append(paymentDate);
      }
      break;
      
    case OCCUR_YEARLY:
      while (paymentDate < startDate)
        paymentDate = paymentDate.addYears(1);
      while (paymentDate <= today)
      {
        paymentDate = paymentDate.addYears(1);
        theDates.append(paymentDate);
      }
      break;
      
    case OCCUR_EVERYOTHERYEAR:
      while (paymentDate < startDate)
        paymentDate = paymentDate.addYears(2);
      while (paymentDate <= today)
      {
        paymentDate = paymentDate.addYears(2);
        theDates.append(paymentDate);
      }
      break;
    case OCCUR_ANY:
      break;
  }

  return theDates;
}

QString MyMoneySchedule::occurenceToString(void) const
{
  QString text;
  
  switch (m_occurence)
  {
    case OCCUR_ONCE:
      text = i18n("Once");
      break;
    case OCCUR_DAILY:
      text = i18n("Daily");
      break;
    case OCCUR_WEEKLY:
      text = i18n("Weekly");
      break;
    case OCCUR_FORTNIGHTLY:
      text = i18n("Fortnightly");
      break;
    case OCCUR_EVERYOTHERWEEK:
      text = i18n("Every other week");
      break;
    case OCCUR_EVERYFOURWEEKS:
      text = i18n("Every four weeks");
      break;
    case OCCUR_MONTHLY:
      text = i18n("Monthly");
      break;
    case OCCUR_EVERYOTHERMONTH:
      text = i18n("Every other month");
      break;
    case OCCUR_EVERYTHREEMONTHS:
      text = i18n("Every three months");
      break;
    case OCCUR_QUARTERLY:
      text = i18n("Quarterly");
      break;
    case OCCUR_EVERYFOURMONTHS:
      text = i18n("Every four months");
      break;
    case OCCUR_TWICEYEARLY:
      text = i18n("Twice yearly");
      break;
    case OCCUR_YEARLY:
      text = i18n("Yearly");
      break;
    case OCCUR_EVERYOTHERYEAR:
      text = i18n("Every other year");
      break;
    case OCCUR_ANY:
      text = i18n("Any (Error)");
      break;
  }

  return text;
}

QString MyMoneySchedule::paymentMethodToString(void) const
{
  QString text;

  switch (m_paymentType)
  {
    case STYPE_DIRECTDEBIT:
      text = i18n("Direct debit");
      break;
    case STYPE_DIRECTDEPOSIT:
      text = i18n("Direct deposit");
      break;
    case STYPE_MANUALDEPOSIT:
      text = i18n("Manual deposit");
      break;
    case STYPE_OTHER:
      text = i18n("Other");
      break;
    case STYPE_WRITECHEQUE:
      text = i18n("Write cheque");
      break;
    case STYPE_ANY:
      text = i18n("Any (Error)");
      break;
  }

  return text;
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

QString MyMoneyScheduled::addSchedule(const QCString& accountId, const MyMoneySchedule& schedule)
{
  if (!schedule.validate())
  {
    throw new MYMONEYEXCEPTION("Invalid schedule instance when adding to collection.");
    return "";
  }

  QCString newId;

  try
  {
    MyMoneyFile::instance()->account(accountId);

    newId.sprintf("SCHED%05d", m_nextId++);

    MyMoneySchedule schedCopy(schedule);
    schedCopy.setId(newId);

    QMap<QString, MyMoneySchedule> scheduled;
    if (m_accountsScheduled.contains(accountId))
    {
      scheduled = m_accountsScheduled[accountId];
    }

    scheduled.insert(newId, schedCopy);

    // will replace the old one if it exists
    m_accountsScheduled.insert(accountId, scheduled, true);
    
  } catch (MyMoneyException *e)
  {
    QString s(i18n("Invalid accountId passed to MyMoneyScheduled: "));
    s += e->what();
    delete e;
    throw new MYMONEYEXCEPTION(s);
  }

  return newId;
}

void MyMoneyScheduled::removeSchedule(const QCString& accountId, const QString& scheduleId)
{
  try
  {
    MyMoneyFile::instance()->account(accountId);

    QMap<QString, MyMoneySchedule> scheduled;
    if (m_accountsScheduled.contains(accountId))
    {
      scheduled = m_accountsScheduled[accountId];
    }
    else
    {
      throw new MYMONEYEXCEPTION("Supplied accountId does not exist in schedules");
      return;
    }
    
    if ((scheduled.find(scheduleId)) == scheduled.end())
    {
      throw new MYMONEYEXCEPTION("Supplied schedule ID does not exist in removeSchedule");
      return;
    }

    scheduled.remove(scheduleId);

    // Will replace
    m_accountsScheduled.insert(accountId, scheduled, true);
    
  } catch (MyMoneyException *e)
  {
    QString s(i18n("Exception in MyMoneyScheduled: "));
    s += e->what();
    delete e;
    throw new MYMONEYEXCEPTION(s);
  }
}

QString MyMoneyScheduled::replaceSchedule(const QCString& accountId, const QString& scheduleId, const MyMoneySchedule& schedule)
{
  try
  {
    MyMoneyFile::instance()->account(accountId);

    QMap<QString, MyMoneySchedule> scheduled;
    if (m_accountsScheduled.contains(accountId))
    {
      scheduled = m_accountsScheduled[accountId];
    }
    else
    {
      throw new MYMONEYEXCEPTION("Supplied accountId does not exist in schedules");
      return "";
    }

    if ((scheduled.find(scheduleId)) == scheduled.end())
    {
      throw new MYMONEYEXCEPTION("Supplied schedule ID does not exist in removeSchedule");
      return "";
    }

    if (!schedule.validate(false))
    {
      throw new MYMONEYEXCEPTION("Invalid schedule instance when replacing in collection.");
      return "";
    }

    scheduled.insert(scheduleId, schedule, true);

    // Will replace
    m_accountsScheduled.insert(accountId, scheduled, true);

  } catch (MyMoneyException *e)
  {
    QString s(i18n("Exception in MyMoneyScheduled: "));
    s += e->what();
    delete e;
    throw new MYMONEYEXCEPTION(s);
  }

  return scheduleId;
}

MyMoneySchedule MyMoneyScheduled::getSchedule(const QCString& accountId, const QString& scheduleId)
{
  MyMoneySchedule sched;
  QMap<QString, MyMoneySchedule>::Iterator it;
  
  try
  {
    MyMoneyFile::instance()->account(accountId);

    QMap<QString, MyMoneySchedule> scheduled;
    if (m_accountsScheduled.contains(accountId))
    {
      scheduled = m_accountsScheduled[accountId];
    }
    else
    {
      throw new MYMONEYEXCEPTION("Supplied accountId does not exist in schedules");
      return sched;
    }

    it = scheduled.find(scheduleId);
    if (it == scheduled.end())
    {
      throw new MYMONEYEXCEPTION("Supplied schedule ID does not exist in removeSchedule");
      return sched;
    }
  } catch (MyMoneyException *e)
  {
    QString s(i18n("Exception in MyMoneyScheduled: "));
    s += e->what();
    delete e;
    throw new MYMONEYEXCEPTION(s);
  }

  return it.data();
}

QStringList MyMoneyScheduled::getScheduled(const QCString& accountId, const MyMoneySchedule::typeE type,
  const MyMoneySchedule::paymentTypeE paymentType,
  const MyMoneySchedule::occurenceE occurence)
{
  QStringList idList;
  QMap<QString, MyMoneySchedule>::Iterator it;

  try
  {
    MyMoneyFile::instance()->account(accountId);

    QMap<QString, MyMoneySchedule> scheduled;
    if (m_accountsScheduled.contains(accountId))
    {
      scheduled = m_accountsScheduled[accountId];
    }
    else
    {
      throw new MYMONEYEXCEPTION("Supplied accountId does not exist in schedules");
      return idList;
    }

    for (it = scheduled.begin(); it != scheduled.end(); ++it)
    {
      if (  (type==MyMoneySchedule::TYPE_ANY || it.data().type() == type) &&
            (paymentType==MyMoneySchedule::STYPE_ANY || it.data().paymentType() == paymentType) &&
            (occurence==MyMoneySchedule::OCCUR_ANY || it.data().occurence() == occurence) )
      {
        idList.append(it.key());
      }
    }
  } catch (MyMoneyException *e)
  {
    QString s(i18n("Exception in MyMoneyScheduled: "));
    s += e->what();
    delete e;
    throw new MYMONEYEXCEPTION(s);
  }

  return idList;
}

QStringList MyMoneyScheduled::getScheduled(const QCString& accountId, const QDate& startDate, const QDate& endDate,
  const MyMoneySchedule::typeE type,
  const MyMoneySchedule::paymentTypeE paymentType,
  const MyMoneySchedule::occurenceE occurence)
{
  QStringList idList;
  QMap<QString, MyMoneySchedule>::Iterator it;

  try
  {
    MyMoneyFile::instance()->account(accountId);

    QMap<QString, MyMoneySchedule> scheduled;
    if (m_accountsScheduled.contains(accountId))
    {
      scheduled = m_accountsScheduled[accountId];
    }
    else
    {
      throw new MYMONEYEXCEPTION("Supplied accountId does not exist in schedules");
      return idList;
    }

    for (it = scheduled.begin(); it != scheduled.end(); ++it)
    {
      if (  (type==MyMoneySchedule::TYPE_ANY || it.data().type() == type) &&
            (paymentType==MyMoneySchedule::STYPE_ANY || it.data().paymentType() == paymentType) &&
            (occurence==MyMoneySchedule::OCCUR_ANY || it.data().occurence() == occurence) )
      {
        QValueList<QDate> payments = it.data().paymentDates(startDate, endDate);
        if (payments.size()>=1)
          idList.append(it.key());
      }
    }
  } catch (MyMoneyException *e)
  {
    QString s(i18n("Exception in MyMoneyScheduled: "));
    s += e->what();
    delete e;
    throw new MYMONEYEXCEPTION(s);
  }

  return idList;
}

QStringList MyMoneyScheduled::getOverdue(const QCString& accountId,
  const MyMoneySchedule::typeE type,
  const MyMoneySchedule::paymentTypeE paymentType,
  const MyMoneySchedule::occurenceE occurence)
{
  QStringList idList;
  QMap<QString, MyMoneySchedule>::Iterator it;

  try
  {
    MyMoneyFile::instance()->account(accountId);

    QMap<QString, MyMoneySchedule> scheduled;
    if (m_accountsScheduled.contains(accountId))
    {
      scheduled = m_accountsScheduled[accountId];
    }
    else
    {
      throw new MYMONEYEXCEPTION("Supplied accountId does not exist in schedules");
      return idList;
    }

    for (it = scheduled.begin(); it != scheduled.end(); ++it)
    {
      if (  (type==MyMoneySchedule::TYPE_ANY || it.data().type() == type) &&
            (paymentType==MyMoneySchedule::STYPE_ANY || it.data().paymentType() == paymentType) &&
            (occurence==MyMoneySchedule::OCCUR_ANY || it.data().occurence() == occurence) )
      {
        QValueList<QDate> list = it.data().paymentDates(it.data().lastPayment(), QDate::currentDate());
        if (list.size() >= 2)
        {
          idList.append(it.key());
        }
      }
    }
  } catch (MyMoneyException *e)
  {
    QString s(i18n("Exception in MyMoneyScheduled: "));
    s += e->what();
    delete e;
    throw new MYMONEYEXCEPTION(s);
  }

  return idList;
}

bool MyMoneyScheduled::anyOverdue(const QCString& accountId, const MyMoneySchedule::typeE type,
  const MyMoneySchedule::occurenceE occurence,
  const MyMoneySchedule::paymentTypeE paymentType)
{
  QMap<QString, MyMoneySchedule>::Iterator it;

  try
  {
    MyMoneyFile::instance()->account(accountId);

    QMap<QString, MyMoneySchedule> scheduled;
    if (m_accountsScheduled.contains(accountId))
    {
      scheduled = m_accountsScheduled[accountId];
    }
    else
    {
      throw new MYMONEYEXCEPTION("Supplied accountId does not exist in schedules");
      return false;
    }

    for (it = scheduled.begin(); it != scheduled.end(); ++it)
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
  } catch (MyMoneyException *e)
  {
    QString s(i18n("Exception in MyMoneyScheduled: "));
    s += e->what();
    delete e;
    throw new MYMONEYEXCEPTION(s);
  }

  return false;
}

bool MyMoneyScheduled::anyScheduled(const QCString& accountId,
  const MyMoneySchedule::typeE type,
  const MyMoneySchedule::occurenceE occurence,
  const MyMoneySchedule::paymentTypeE paymentType)
{
  QMap<QString, MyMoneySchedule>::Iterator it;

  try
  {
    MyMoneyFile::instance()->account(accountId);

    QMap<QString, MyMoneySchedule> scheduled;
    if (m_accountsScheduled.contains(accountId))
    {
      scheduled = m_accountsScheduled[accountId];
    }
    else
    {
      throw new MYMONEYEXCEPTION("Supplied accountId does not exist in schedules");
      return false;
    }

    for (it = scheduled.begin(); it != scheduled.end(); ++it)
    {
      if (  (type==MyMoneySchedule::TYPE_ANY || it.data().type() == type) &&
            (occurence==MyMoneySchedule::OCCUR_ANY || it.data().occurence() == occurence) &&
            (paymentType==MyMoneySchedule::STYPE_ANY || it.data().paymentType() == paymentType) )
      {
        return true;
      }
    }
  } catch (MyMoneyException *e)
  {
    QString s(i18n("Exception in MyMoneyScheduled: "));
    s += e->what();
    delete e;
    throw new MYMONEYEXCEPTION(s);
  }

  return false;
}

unsigned int MyMoneyScheduled::count(const QCString& accountId)
{
  try
  {
    MyMoneyFile::instance()->account(accountId);

    if (m_accountsScheduled.contains(accountId))
      return m_accountsScheduled[accountId].count();
    
  } catch (MyMoneyException *e)
  {
    QString s(i18n("Exception in MyMoneyScheduled: "));
    s += e->what();
    delete e;
    throw new MYMONEYEXCEPTION(s);
  }

  return 0;
}

QString MyMoneySchedule::typeToString(void) const
{
  switch (m_type)
  {
    case TYPE_BILL:
      return i18n("Bill");
    case TYPE_DEPOSIT:
      return i18n("Deposit");
    case TYPE_TRANSFER:
      return i18n("Transfer");
    case TYPE_ANY:
      return i18n("Unknown");
  }
}
