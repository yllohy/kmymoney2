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

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyscheduled.h"
#include "mymoneyexception.h"
#include "mymoneyfile.h"

MyMoneySchedule::MyMoneySchedule()
{
  // Set up the default values
  m_occurence = OCCUR_ANY;
  m_type = TYPE_ANY;
  m_paymentType = STYPE_ANY;
  m_fixed = false;
  m_autoEnter = false;
  m_startDate = QDate();
  m_endDate = QDate();
  m_lastPayment = QDate();
  m_id = "";
}

MyMoneySchedule::MyMoneySchedule(const QString& name, typeE type,
                                 occurenceE occurence, paymentTypeE paymentType,
                                 const QDate& startDate,
                                 const QDate& endDate,
                                 bool fixed, bool autoEnter) {
  // Set up the default values
  m_name = name;
  m_occurence = occurence;
  m_type = type;
  m_paymentType = paymentType;
  m_fixed = fixed;
  m_autoEnter = autoEnter;
  m_startDate = startDate;
  m_endDate = endDate;
  m_lastPayment = QDate();
  m_id = "";
}


void MyMoneySchedule::setStartDate(const QDate& date)
{
  m_startDate = date;
}

void MyMoneySchedule::setPaymentType(paymentTypeE type)
{
  m_paymentType = type;
}

void MyMoneySchedule::setFixed(bool fixed)
{
  m_fixed = fixed;
}

void MyMoneySchedule::setTransaction(const MyMoneyTransaction& transaction)
{
  m_transaction = transaction;
}

void MyMoneySchedule::setEndDate(const QDate& date)
{
  m_endDate = date;
}

void MyMoneySchedule::setAutoEnter(bool autoenter)
{
  m_autoEnter = autoenter;
}

void MyMoneySchedule::setLastPayment(const QDate& date)
{
  // Delete all payments older than date
  QValueList<QDate>::Iterator it;
  QValueList<QDate> delList;
  
  for (it=m_recordedPayments.begin(); it!=m_recordedPayments.end(); ++it)
  {
    if (*it < date)
      delList.append(*it);
  }

  for (it=delList.begin(); it!=delList.end(); ++it)
  {
    m_recordedPayments.remove(*it);
  }

  m_lastPayment = date;
}

void MyMoneySchedule::setName(const QString& nm)
{
  m_name = nm;
}

void MyMoneySchedule::setOccurence(occurenceE occ)
{
  m_occurence = occ;
}

void MyMoneySchedule::setType(typeE type)
{
  m_type = type;
}

void MyMoneySchedule::validate(bool id_check) const
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
   *   the transaction must contain at least one split (two is better ;-)  )
   */
  if (id_check && !m_id.isEmpty())
    throw new MYMONEYEXCEPTION("ID for schedule not empty when required");

  if(m_occurence == OCCUR_ANY)
    throw new MYMONEYEXCEPTION("Invalid occurence type for schedule");

  if(m_type == TYPE_ANY)
    throw new MYMONEYEXCEPTION("Invalid type for schedule");
  
  if(!m_startDate.isValid())
    throw new MYMONEYEXCEPTION("Invalid start date for schedule");
    
  if(m_paymentType == STYPE_ANY)
    throw new MYMONEYEXCEPTION("Invalid payment type for schedule");

  if(m_transaction.splitCount() == 0)
    throw new MYMONEYEXCEPTION("Scheduled transaction does not contain splits");
  
  // Check the payment types
  switch (m_type)
  {
    case TYPE_BILL:
      if (m_paymentType == STYPE_DIRECTDEPOSIT || m_paymentType == STYPE_MANUALDEPOSIT)
        throw new MYMONEYEXCEPTION("Invalid payment type for bills");
      break;

    case TYPE_DEPOSIT:
      if (m_paymentType == STYPE_DIRECTDEBIT || m_paymentType == STYPE_WRITECHEQUE)
        throw new MYMONEYEXCEPTION("Invalid payment type for deposits");
      break;

    case TYPE_ANY:
      throw new MYMONEYEXCEPTION("Invalid type ANY");
      break;

    case TYPE_TRANSFER:
//        if (m_paymentType == STYPE_DIRECTDEPOSIT || m_paymentType == STYPE_MANUALDEPOSIT)
//          return false;
      break;
  }
}

QDate MyMoneySchedule::nextPayment(const QDate& refDate) const
{
  QDate paymentDate(m_lastPayment);

  // if there never was a payment, then the next payment date must
  // be identical to the start date of the payments.
  
  if(!paymentDate.isValid()) {
    paymentDate = m_startDate;
    
    // if the reference date is invalid, then that's what we're looking for
    if(!refDate.isValid())
      return paymentDate;
  }

  // if the enddate is valid and it is before the reference date,
  // then there will be no more payments.
  if(m_endDate.isValid() && m_endDate < refDate) {
    
    return QDate();
  }

  switch (m_occurence)
  {
    case OCCUR_ONCE:
      // if the lastPayment is already set, then there will be no more payments
      // otherwise, the start date is the payment date
      if(m_lastPayment.isValid())
        return QDate();
      // if the only payment should have been prior to the reference date,
      // then don't show it
      if(m_startDate < refDate)
        return QDate();
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
      paymentDate = QDate();
      break;
  }
  if(paymentDate.isValid()) {
    if(m_endDate.isValid() && paymentDate > m_endDate)
      paymentDate = QDate();
  }
  return paymentDate;
}

QValueList<QDate> MyMoneySchedule::paymentDates(const QDate& startDate, const QDate& endDate) const
{
  QDate paymentDate(m_startDate);
  QValueList<QDate> theDates;

  // if the period specified by the parameters and the period
  // defined for this schedule, then the list remains emtpy
  if ((willEnd() && m_endDate < startDate)
  || m_startDate > endDate)
    return theDates;

  switch (m_occurence)
  {
    case OCCUR_ONCE:
      if (m_startDate >= startDate && m_startDate <= endDate)
        theDates.append(m_startDate);
      break;
      
    case OCCUR_DAILY:
      while (paymentDate <= endDate)
      {
        paymentDate = paymentDate.addDays(1);
        theDates.append(paymentDate);
      }
      break;
      
    case OCCUR_WEEKLY:
      while (paymentDate < startDate)
        paymentDate = paymentDate.addDays(7);

      while (paymentDate <= endDate)
      {
        paymentDate = paymentDate.addDays(7);
        theDates.append(paymentDate);
      }
      break;
      
    case OCCUR_FORTNIGHTLY:
    case OCCUR_EVERYOTHERWEEK:
      while (paymentDate < startDate)
        paymentDate = paymentDate.addDays(14);
      while (paymentDate <= endDate)
      {
        paymentDate = paymentDate.addDays(14);
        theDates.append(paymentDate);
      }
      break;
      
    case OCCUR_EVERYFOURWEEKS:
      while (paymentDate < startDate)
        paymentDate = paymentDate.addDays(28);
      while (paymentDate <= endDate)
      {
        paymentDate = paymentDate.addDays(28);
        theDates.append(paymentDate);
      }
      break;
      
    case OCCUR_MONTHLY:
      while (paymentDate < startDate)
        paymentDate = paymentDate.addMonths(1);
      while (paymentDate <= endDate)
      {
        paymentDate = paymentDate.addMonths(1);
        theDates.append(paymentDate);
      }
      break;
      
    case OCCUR_EVERYOTHERMONTH:
      while (paymentDate < startDate)
        paymentDate = paymentDate.addMonths(2);
      while (paymentDate <= endDate)
      {
        paymentDate = paymentDate.addMonths(2);
        theDates.append(paymentDate);
      }
      break;
      
    case OCCUR_EVERYTHREEMONTHS:
      while (paymentDate < startDate)
        paymentDate = paymentDate.addMonths(3);
      while (paymentDate <= endDate)
      {
        paymentDate = paymentDate.addMonths(3);
        theDates.append(paymentDate);
      }
      break;
      
    case OCCUR_QUARTERLY:
    case OCCUR_EVERYFOURMONTHS:
      while (paymentDate < startDate)
        paymentDate = paymentDate.addMonths(4);
      while (paymentDate <= endDate)
      {
        paymentDate = paymentDate.addMonths(4);
        theDates.append(paymentDate);
      }
      break;
      
    case OCCUR_TWICEYEARLY:
      while (paymentDate < startDate)
        paymentDate = paymentDate.addMonths(6);
      while (paymentDate <= endDate)
      {
        paymentDate = paymentDate.addMonths(6);
        theDates.append(paymentDate);
      }
      break;
      
    case OCCUR_YEARLY:
      while (paymentDate < startDate)
        paymentDate = paymentDate.addYears(1);
      while (paymentDate <= endDate)
      {
        paymentDate = paymentDate.addYears(1);
        theDates.append(paymentDate);
      }
      break;
      
    case OCCUR_EVERYOTHERYEAR:
      while (paymentDate < startDate)
        paymentDate = paymentDate.addYears(2);
      while (paymentDate <= endDate)
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


bool MyMoneySchedule::operator <(const MyMoneySchedule& right)
{
  return nextPayment(m_lastPayment) < right.nextPayment(right.m_lastPayment);  
}

bool MyMoneySchedule::operator ==(const MyMoneySchedule& right)
{
  if (  m_occurence == right.m_occurence &&
        m_type == right.m_type &&
        m_startDate == right.m_startDate &&
        m_paymentType == right.m_paymentType &&
        m_fixed == right.m_fixed &&
        m_transaction == right.m_transaction &&
        m_endDate == right.m_endDate &&
        m_autoEnter == right.m_autoEnter &&
        m_id == right.m_id &&
        m_lastPayment == right.m_lastPayment &&
        m_name == right.m_name)
    return true;
  return false;
}

int MyMoneySchedule::transactionsRemaining(void) const
{
  int counter=0;

  if (m_endDate.isValid())
  {
    QValueList<QDate> dates = paymentDates(m_lastPayment, m_endDate);
    // Dont include the last payment so -1
    counter = dates.count();
  }
  return counter;
}

const MyMoneyAccount MyMoneySchedule::account(int cnt) const
{
  QValueList<MyMoneySplit> splits = m_transaction.splits();
  QValueList<MyMoneySplit>::ConstIterator it;
  MyMoneyFile* file = MyMoneyFile::instance();
  MyMoneyAccount acc;

  // search the first asset or liability account    
  for(it = splits.begin(); it != splits.end() && (acc.id().isEmpty() || cnt); ++it) {
    acc = file->account((*it).accountId());
    switch(file->accountGroup(acc.accountType())) {
      case MyMoneyAccount::Liability:
      case MyMoneyAccount::Asset:
        --cnt;
        break;
      default:
        break;
    }
  }

  return it != splits.end() ? acc : MyMoneyAccount();
}

QDate MyMoneySchedule::dateAfter(int transactions) const
{
  int counter=1;
  QDate paymentDate(m_startDate);

  if (transactions<=0)
    return paymentDate;

  switch (m_occurence)
  {
    case OCCUR_ONCE:
      break;

    case OCCUR_DAILY:
      while (counter++ < transactions)
        paymentDate = paymentDate.addDays(1);
      break;

    case OCCUR_WEEKLY:
      while (counter++ < transactions)
        paymentDate = paymentDate.addDays(7);
      break;

    case OCCUR_FORTNIGHTLY:
    case OCCUR_EVERYOTHERWEEK:
      while (counter++ < transactions)
        paymentDate = paymentDate.addDays(14);
      break;

    case OCCUR_EVERYFOURWEEKS:
      while (counter++ < transactions)
        paymentDate = paymentDate.addDays(28);
      break;

    case OCCUR_MONTHLY:
      while (counter++ < transactions)
        paymentDate = paymentDate.addMonths(1);
      break;

    case OCCUR_EVERYOTHERMONTH:
      while (counter++ < transactions)
        paymentDate = paymentDate.addMonths(2);
      break;

    case OCCUR_EVERYTHREEMONTHS:
      while (counter++ < transactions)
        paymentDate = paymentDate.addMonths(3);
      break;

    case OCCUR_QUARTERLY:
    case OCCUR_EVERYFOURMONTHS:
      while (counter++ < transactions)
        paymentDate = paymentDate.addMonths(4);
      break;

    case OCCUR_TWICEYEARLY:
      while (counter++ < transactions)
        paymentDate = paymentDate.addMonths(6);
      break;

    case OCCUR_YEARLY:
      while (counter++ < transactions)
        paymentDate = paymentDate.addYears(1);
      break;

    case OCCUR_EVERYOTHERYEAR:
      while (counter++ < transactions)
        paymentDate = paymentDate.addYears(2);
      break;
    case OCCUR_ANY:
      break;
  }

  return paymentDate;
}

bool MyMoneySchedule::isOverdue() const
{
  if (isFinished())
    return false;

  bool bOverdue = true;

  // Check the payment dates first
  QValueList<QDate> datesBeforeToday = paymentDates(m_startDate, QDate::currentDate().addDays(-1));
  if (datesBeforeToday.count() == 0)
  {
    bOverdue = false;
  }
  else if (datesBeforeToday.count() == 1)
  {
    if (nextPayment(m_lastPayment).isValid() &&
        (nextPayment(m_lastPayment) >= QDate::currentDate()))
      bOverdue = false;
  }
  else
  {          
    // Check the dates
    // Remove all dates before m_lastPayment
    QValueList<QDate> delList;
    QValueList<QDate>::ConstIterator it;

    for (it=datesBeforeToday.begin(); it!=datesBeforeToday.end(); ++it)
    {
      if (*it <= m_lastPayment)
        delList.append(*it);
    }
    for (it=delList.begin(); it!=delList.end(); ++it)
    {
      datesBeforeToday.remove(*it);
    }

    // Remove nextPayment (lastPayments returns it?)
    if (datesBeforeToday.contains(nextPayment(m_lastPayment)))
      datesBeforeToday.remove(nextPayment(m_lastPayment));
    
    for (it=m_recordedPayments.begin(); it!=m_recordedPayments.end(); ++it)
    {
      if (datesBeforeToday.contains(*it))
        datesBeforeToday.remove(*it);
    }

    if (datesBeforeToday.contains(m_lastPayment))
      datesBeforeToday.remove(m_lastPayment);

    // Now finally check
    if (datesBeforeToday.count() == 0)
      bOverdue = false;
  }

  return bOverdue;  
}

bool MyMoneySchedule::isFinished() const
{
  if (m_endDate.isValid() && m_lastPayment >= m_endDate)
    return true;
    
  return false;
}

bool MyMoneySchedule::hasRecordedPayment(const QDate& date) const
{
  // m_lastPayment should always be > recordedPayments()
  if (m_lastPayment.isValid() && m_lastPayment >= date)
    return true;
    
  if (m_recordedPayments.contains(date))
    return true;
    
  return false;
}

void MyMoneySchedule::recordPayment(const QDate& date)
{
  m_recordedPayments.append(date);
}
