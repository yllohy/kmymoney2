/***************************************************************************
                          mymoneyscheduled.cpp
                             -------------------
    copyright            : (C) 2000-2002 by Michael Edwardes
                           (C) 2007 by Thomas Baumgart
    email                : mte@users.sourceforge.net
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
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

MyMoneySchedule::MyMoneySchedule() :
  MyMoneyObject()
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
  m_weekendOption = MoveNothing;
}

MyMoneySchedule::MyMoneySchedule(const QString& name, typeE type,
                                 occurenceE occurence, paymentTypeE paymentType,
                                 const QDate& /* startDate */,
                                 const QDate& endDate,
                                 bool fixed, bool autoEnter) :
  MyMoneyObject()
{
  // Set up the default values
  m_name = name;
  m_occurence = occurence;
  m_type = type;
  m_paymentType = paymentType;
  m_fixed = fixed;
  m_autoEnter = autoEnter;
  m_startDate = QDate();
  m_endDate = endDate;
  m_lastPayment = QDate();
  m_weekendOption = MoveNothing;
}

MyMoneySchedule::MyMoneySchedule(const QDomElement& node) :
  MyMoneyObject(node)
{
  if("SCHEDULED_TX" != node.tagName())
    throw new MYMONEYEXCEPTION("Node was not SCHEDULED_TX");

  m_name = node.attribute("name");
  m_startDate = stringToDate(node.attribute("startDate"));
  m_endDate = stringToDate(node.attribute("endDate"));
  m_lastPayment = stringToDate(node.attribute("lastPayment"));

  m_type = static_cast<MyMoneySchedule::typeE>(node.attribute("type").toInt());
  m_paymentType = static_cast<MyMoneySchedule::paymentTypeE>(node.attribute("paymentType").toInt());
  m_occurence = static_cast<MyMoneySchedule::occurenceE>(node.attribute("occurence").toInt());

  m_autoEnter = static_cast<bool>(node.attribute("autoEnter").toInt());
  m_fixed = static_cast<bool>(node.attribute("fixed").toInt());
  m_weekendOption = static_cast<MyMoneySchedule::weekendOptionE>(node.attribute("weekendOption").toInt());

  // read in the associated transaction
  QDomNodeList nodeList = node.elementsByTagName("TRANSACTION");
  if(nodeList.count() == 0)
    throw new MYMONEYEXCEPTION("SCHEDULED_TX has no TRANSACTION node");

  setTransaction(MyMoneyTransaction(nodeList.item(0).toElement(), false));

  // some old versions did not remove the entry date and post date fields
  // in the schedule. So if this is the case, we deal with a very old transaction
  // and can't use the post date field as next due date. Hence, we wipe it out here
  if(m_transaction.entryDate().isValid()) {
    m_transaction.setPostDate(QDate());
    m_transaction.setEntryDate(QDate());
  }

  // readin the recorded payments
  nodeList = node.elementsByTagName("PAYMENTS");
  if(nodeList.count() > 0) {
    nodeList = nodeList.item(0).toElement().elementsByTagName("PAYMENT");
    for(unsigned int i = 0; i < nodeList.count(); ++i) {
      m_recordedPayments << stringToDate(nodeList.item(i).toElement().attribute("date"));
    }
  }

  // if the next due date is not set (comes from old version)
  // then set it up the old way
  if(!nextDueDate().isValid() && !m_lastPayment.isValid()) {
    m_transaction.setPostDate(m_startDate);
    // clear it, because the schedule has never been used
    m_startDate = QDate();
  }

  // There are reports that lastPayment and nextDueDate are identical or
  // that nextDueDate is older than lastPayment. This could
  // be caused by older versions of the application. In this case, we just
  // clear out the nextDueDate and let it calculate from the lastPayment.
  if(nextDueDate().isValid() && nextDueDate() <= m_lastPayment) {
    m_transaction.setPostDate(QDate());
  }

  if(!nextDueDate().isValid()) {
    m_transaction.setPostDate(m_startDate);
    m_transaction.setPostDate(nextPayment(m_lastPayment.addDays(1)));
  }
}

MyMoneySchedule::MyMoneySchedule(const QCString& id, const MyMoneySchedule& right) :
  MyMoneyObject(id)
{
  *this = right;
  setId(id);
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
  MyMoneyTransaction t = transaction;
  // don't allow a transaction that has no due date
  // if we get something like that, then we use the
  // the current next due date. If that is also invalid
  // we can't help it.
  if(!t.postDate().isValid()) {
    t.setPostDate(m_transaction.postDate());
  }
  if(!t.postDate().isValid())
    return;

  // make sure to clear out some unused information in scheduled transactions
  // we need to do this for the case that the transaction passed as argument
  // is a matched or imported transaction.
  QValueList<MyMoneySplit> splits = t.splits();
  if(splits.count() > 0) {
    QValueList<MyMoneySplit>::const_iterator it_s;
    for(it_s = splits.begin(); it_s != splits.end(); ++it_s) {
      MyMoneySplit s = *it_s;
      // clear out the bankID
      if(!(*it_s).bankID().isEmpty()) {
        s.setBankID(QCString());
        t.modifySplit(s);
      }

      // only clear payees from second split onwards
      if(it_s == splits.begin())
        continue;

      if(!(*it_s).payeeId().isEmpty()) {
        s.setPayeeId(QCString());
        t.modifySplit(s);
      }
    }
  }

  m_transaction = t;
  // make sure that the transaction does not have an id so that we can enter
  // it into the engine
  m_transaction.clearId();
}

void MyMoneySchedule::setEndDate(const QDate& date)
{
  m_endDate = date;
}

void MyMoneySchedule::setAutoEnter(bool autoenter)
{
  m_autoEnter = autoenter;
}

const QDate& MyMoneySchedule::startDate(void) const
{
  if(m_startDate.isValid())
    return m_startDate;
  return nextDueDate();
}

const QDate& MyMoneySchedule::nextDueDate(void) const
{
  return m_transaction.postDate();
}

QDate MyMoneySchedule::adjustedNextDueDate(void) const
{
  QDate date(nextDueDate());

  if(isFinished())
    return QDate();

  if(weekendOption() != MyMoneySchedule::MoveNothing) {
    int dayOfWeek = date.dayOfWeek();
    if (dayOfWeek >= 6) {
      if (weekendOption() == MyMoneySchedule::MoveFriday) {
        if (dayOfWeek == 7)
          date = date.addDays(-2);
        else
          date = date.addDays(-1);
      } else {
        if (dayOfWeek == 6)
          date = date.addDays(2);
        else
          date = date.addDays(1);
      }
    }
  }
  return date;
}

void MyMoneySchedule::setNextDueDate(const QDate& date)
{
  m_transaction.setPostDate(date);
  if(date.isValid())
    m_startDate = date;
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
  if(!m_startDate.isValid())
    m_startDate = date;
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

  if(!nextDueDate().isValid())
    throw new MYMONEYEXCEPTION("Invalid next due date for schedule");

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

    case TYPE_LOANPAYMENT:
      break;
  }
}

QDate MyMoneySchedule::nextPayment(const QDate& refDate) const
{
#if 0
  QDate paymentDate(m_lastPayment);

  // if there never was a payment, then the next payment date must
  // be identical to the start date of the payments.

  if(!paymentDate.isValid()) {
    paymentDate = m_startDate;

    // if the reference date is invalid, then that's what we're looking for
    if(!refDate.isValid())
    {
      if (m_recordedPayments.contains(paymentDate))
        return QDate();

      return paymentDate;

    } else {
      // if the first payment date is past the given ref date,
      // then that's what we're looking for
      if(paymentDate > refDate)
        return paymentDate;
    }
  }
#endif

  // if the enddate is valid and it is before the reference date,
  // then there will be no more payments.
  if(m_endDate.isValid() && m_endDate < refDate) {
    return QDate();
  }

  QDate paymentDate(nextDueDate());

  if(refDate >= paymentDate) {
    switch (m_occurence)
    {
      case OCCUR_ONCE:
        // if the lastPayment is already set, then there will be no more payments
        // otherwise, the start date is the payment date
        if(m_lastPayment.isValid())
          return QDate();
        // if the only payment should have been prior to the reference date,
        // then don't show it
        if(nextDueDate() < refDate)
          return QDate();
        paymentDate = nextDueDate();
        break;

      case OCCUR_DAILY:
        if (paymentDate <= refDate)
          paymentDate = refDate.addDays(1);
        else
          paymentDate = paymentDate.addDays(1);
        break;

      case OCCUR_WEEKLY:
        do {
          paymentDate = paymentDate.addDays(7);
        }
        while (paymentDate <= refDate);
        break;

      case OCCUR_FORTNIGHTLY:
      case OCCUR_EVERYOTHERWEEK:
        do
        {
          paymentDate = paymentDate.addDays(14);
        }
        while (paymentDate <= refDate);
        break;

      case OCCUR_EVERYTHREEWEEKS:
	do {
          paymentDate = paymentDate.addDays(21);
	}
	while (paymentDate <= refDate);
	break;

        case OCCUR_EVERYTHIRTYDAYS:
        do
        {
          paymentDate = paymentDate.addDays(30);
        }
        while (paymentDate <= refDate);
        break;

      case OCCUR_EVERYFOURWEEKS:
        do
        {
          paymentDate = paymentDate.addDays(28);
        }
        while (paymentDate <= refDate);
        break;

      case OCCUR_MONTHLY:
        do {
          paymentDate = paymentDate.addMonths(1);
          fixDate(paymentDate);
        }
        while (paymentDate <= refDate);
        break;

      case OCCUR_EVERYEIGHTWEEKS:
        do
        {
          paymentDate = paymentDate.addDays(56);
        }
        while (paymentDate <= refDate);
        break;

      case OCCUR_EVERYOTHERMONTH:
        do {
          paymentDate = paymentDate.addMonths(2);
          fixDate(paymentDate);
        }
        while (paymentDate <= refDate);
        break;

      case OCCUR_QUARTERLY:
      case OCCUR_EVERYTHREEMONTHS:
        do {
          paymentDate = paymentDate.addMonths(3);
          fixDate(paymentDate);
        }
        while (paymentDate <= refDate);
        break;

      case OCCUR_EVERYFOURMONTHS:
        do {
          paymentDate = paymentDate.addMonths(4);
          fixDate(paymentDate);
        }
        while (paymentDate <= refDate);
        break;

      case OCCUR_TWICEYEARLY:
        do {
          paymentDate = paymentDate.addMonths(6);
          fixDate(paymentDate);
        }
        while (paymentDate <= refDate);
        break;

      case OCCUR_YEARLY:
        do {
          paymentDate = paymentDate.addYears(1);
          fixDate(paymentDate);
        }
        while (paymentDate <= refDate);
        break;

      case OCCUR_EVERYOTHERYEAR:
        do {
          paymentDate = paymentDate.addYears(2);
          fixDate(paymentDate);
        }
        while (paymentDate <= refDate);
        break;

      case OCCUR_ANY:
        paymentDate = QDate();
        break;
    }
  }
  if(paymentDate.isValid()) {
    if(m_endDate.isValid() && paymentDate > m_endDate)
      paymentDate = QDate();
  }

  if (paymentDate.isValid() && m_recordedPayments.contains(paymentDate))
    paymentDate = nextPayment(paymentDate);

  return paymentDate;
}

QValueList<QDate> MyMoneySchedule::paymentDates(const QDate& _startDate, const QDate& _endDate) const
{
  QDate paymentDate(nextDueDate());
  QValueList<QDate> theDates;

  QDate endDate(_endDate);
  if ( willEnd() && m_endDate < endDate )
    endDate = m_endDate;

  // if the period specified by the parameters and the period
  // defined for this schedule don't overlap, then the list remains empty
  if ((willEnd() && m_endDate < _startDate)
  || startDate() > endDate)
    return theDates;

  switch (m_occurence)
  {
    case OCCUR_ONCE:
      if (startDate() >= _startDate && startDate() <= endDate)
        theDates.append(startDate());
      break;

    case OCCUR_DAILY:
      if (paymentDate < _startDate)
        paymentDate = _startDate;
      while (paymentDate <= endDate)
      {
        theDates.append(paymentDate);
        paymentDate = paymentDate.addDays(1);
      }
      break;

    case OCCUR_WEEKLY:
      while (paymentDate < _startDate)
        paymentDate = paymentDate.addDays(7);
      while (paymentDate <= endDate)
      {
        theDates.append(paymentDate);
        paymentDate = paymentDate.addDays(7);
      }
      break;

    case OCCUR_FORTNIGHTLY:
    case OCCUR_EVERYOTHERWEEK:
      while (paymentDate < _startDate)
        paymentDate = paymentDate.addDays(14);
      while (paymentDate <= endDate)
      {
        theDates.append(paymentDate);
        paymentDate = paymentDate.addDays(14);
      }
      break;

    case OCCUR_EVERYTHREEWEEKS:
      while (paymentDate < _startDate)
        paymentDate = paymentDate.addDays(21);
      while (paymentDate <= endDate)
      {
        theDates.append(paymentDate);
        paymentDate = paymentDate.addDays(21);
      }
      break;

    case OCCUR_EVERYTHIRTYDAYS:
      while (paymentDate < _startDate)
        paymentDate = paymentDate.addDays(30);
      while (paymentDate <= endDate)
      {
        theDates.append(paymentDate);
        paymentDate = paymentDate.addDays(30);
      }
      break;

    case OCCUR_EVERYFOURWEEKS:
      while (paymentDate < _startDate)
        paymentDate = paymentDate.addDays(28);
      while (paymentDate <= endDate)
      {
        theDates.append(paymentDate);
        paymentDate = paymentDate.addDays(28);
      }
      break;

    case OCCUR_MONTHLY:
      while (paymentDate < _startDate) {
        paymentDate = paymentDate.addMonths(1);
        fixDate(paymentDate);
      }
      while (paymentDate <= endDate)
      {
        theDates.append(paymentDate);
        paymentDate = paymentDate.addMonths(1);
        fixDate(paymentDate);
      }
      break;

    case OCCUR_EVERYEIGHTWEEKS:
      while (paymentDate < _startDate)
        paymentDate = paymentDate.addDays(56);
      while (paymentDate <= endDate)
      {
        theDates.append(paymentDate);
        paymentDate = paymentDate.addDays(56);
      }
      break;

    case OCCUR_EVERYOTHERMONTH:
      while (paymentDate < _startDate) {
        paymentDate = paymentDate.addMonths(2);
        fixDate(paymentDate);
      }
      while (paymentDate <= endDate)
      {
        theDates.append(paymentDate);
        paymentDate = paymentDate.addMonths(2);
        fixDate(paymentDate);
      }
      break;

    case OCCUR_QUARTERLY:
    case OCCUR_EVERYTHREEMONTHS:
      while (paymentDate < _startDate) {
        paymentDate = paymentDate.addMonths(3);
        fixDate(paymentDate);
      }
      while (paymentDate <= endDate)
      {
        theDates.append(paymentDate);
        paymentDate = paymentDate.addMonths(3);
        fixDate(paymentDate);
      }
      break;

    case OCCUR_EVERYFOURMONTHS:
      while (paymentDate < _startDate) {
        paymentDate = paymentDate.addMonths(4);
        fixDate(paymentDate);
      }
      while (paymentDate <= endDate)
      {
        theDates.append(paymentDate);
        paymentDate = paymentDate.addMonths(4);
        fixDate(paymentDate);
      }
      break;

    case OCCUR_TWICEYEARLY:
      while (paymentDate < _startDate) {
        paymentDate = paymentDate.addMonths(6);
        fixDate(paymentDate);
      }
      while (paymentDate <= endDate)
      {
        theDates.append(paymentDate);
        paymentDate = paymentDate.addMonths(6);
        fixDate(paymentDate);
      }
      break;

    case OCCUR_YEARLY:
      while (paymentDate < _startDate) {
        paymentDate = paymentDate.addYears(1);
        fixDate(paymentDate);
      }
      while (paymentDate <= endDate)
      {
        theDates.append(paymentDate);
        paymentDate = paymentDate.addYears(1);
        fixDate(paymentDate);
      }
      break;

    case OCCUR_EVERYOTHERYEAR:
      while (paymentDate < _startDate) {
        paymentDate = paymentDate.addYears(2);
        fixDate(paymentDate);
      }
      while (paymentDate <= endDate)
      {
        theDates.append(paymentDate);
        paymentDate = paymentDate.addYears(2);
        fixDate(paymentDate);
      }
      break;

    case OCCUR_ANY:
      break;
  }

  return theDates;
}


bool MyMoneySchedule::operator <(const MyMoneySchedule& right)
{
  return nextDueDate() < right.nextDueDate();
}

bool MyMoneySchedule::operator ==(const MyMoneySchedule& right)
{
  if (  MyMoneyObject::operator==(right) &&
        m_occurence == right.m_occurence &&
        m_type == right.m_type &&
        m_startDate == right.m_startDate &&
        m_paymentType == right.m_paymentType &&
        m_fixed == right.m_fixed &&
        m_transaction == right.m_transaction &&
        m_endDate == right.m_endDate &&
        m_autoEnter == right.m_autoEnter &&
        m_lastPayment == right.m_lastPayment &&
        ((m_name.length() == 0 && right.m_name.length() == 0) || (m_name == right.m_name)))
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

MyMoneyAccount MyMoneySchedule::account(int cnt) const
{
  QValueList<MyMoneySplit> splits = m_transaction.splits();
  QValueList<MyMoneySplit>::ConstIterator it;
  MyMoneyFile* file = MyMoneyFile::instance();
  MyMoneyAccount acc;

  // search the first asset or liability account
  for(it = splits.begin(); it != splits.end() && (acc.id().isEmpty() || cnt); ++it) {
    try {
      acc = file->account((*it).accountId());
      if(acc.isAssetLiability())
          --cnt;

      if(!cnt)
        return acc;
    } catch(MyMoneyException *e) {
      qWarning("Schedule '%s' references unknown account '%s'", id().data(),   (*it).accountId().data());
      delete e;
      return MyMoneyAccount();
    }
  }

  return MyMoneyAccount();
}

QDate MyMoneySchedule::dateAfter(int transactions) const
{
  int counter=1;
  QDate paymentDate(startDate());

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

    case OCCUR_EVERYTHREEWEEKS:
      while (counter++ < transactions)
        paymentDate = paymentDate.addDays(21);
      break;

    case OCCUR_EVERYTHIRTYDAYS:
      while (counter++ < transactions)
        paymentDate = paymentDate.addDays(30);
      break;

    case OCCUR_EVERYFOURWEEKS:
      while (counter++ < transactions)
        paymentDate = paymentDate.addDays(28);
      break;

    case OCCUR_MONTHLY:
      while (counter++ < transactions)
        paymentDate = paymentDate.addMonths(1);
      break;

    case OCCUR_EVERYEIGHTWEEKS:
      while (counter++ < transactions)
        paymentDate = paymentDate.addDays(56);
      break;

    case OCCUR_EVERYOTHERMONTH:
      while (counter++ < transactions)
        paymentDate = paymentDate.addMonths(2);
      break;

    case OCCUR_QUARTERLY:
    case OCCUR_EVERYTHREEMONTHS:
      while (counter++ < transactions)
        paymentDate = paymentDate.addMonths(3);
      break;

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

  if(nextDueDate() >= QDate::currentDate())
    return false;

  return true;

#if 0
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
#endif
}

bool MyMoneySchedule::isFinished() const
{
  if(!m_lastPayment.isValid())
    return false;

  if (m_endDate.isValid()) {
    if(m_lastPayment >= m_endDate
    || !nextDueDate().isValid()
    || nextDueDate() > m_endDate)
      return true;
  }

  // Check to see if its a once off payment
  if (m_occurence == MyMoneySchedule::OCCUR_ONCE)
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

void MyMoneySchedule::setWeekendOption(const weekendOptionE option)
{
  // make sure only valid values are used. Invalid defaults to MoveNothing.
  switch(option) {
    case MoveFriday:
    case MoveMonday:
      m_weekendOption = option;
      break;

    default:
      m_weekendOption = MoveNothing;
      break;
  }
}

void MyMoneySchedule::fixDate(QDate& date) const
{
  QDate fixDate(m_startDate);
  if(fixDate.isValid()
  && date.day() != fixDate.day()
  && QDate::isValid(date.year(), date.month(), fixDate.day())) {
    date.setYMD(date.year(), date.month(), fixDate.day());
  }
}

void MyMoneySchedule::writeXML(QDomDocument& document, QDomElement& parent) const
{
  QDomElement el = document.createElement("SCHEDULED_TX");

  writeBaseXML(document, el);

  el.setAttribute("name", m_name);
  el.setAttribute("type", m_type);
  el.setAttribute("occurence", m_occurence);
  el.setAttribute("paymentType", m_paymentType);
  el.setAttribute("startDate", dateToString(m_startDate));
  el.setAttribute("endDate", dateToString(m_endDate));
  el.setAttribute("fixed", m_fixed);
  el.setAttribute("autoEnter", m_autoEnter);
  el.setAttribute("lastPayment", dateToString(m_lastPayment));
  el.setAttribute("weekendOption", m_weekendOption);

  //store the payment history for this scheduled task.
  QValueList<QDate> payments = recordedPayments();
  QValueList<QDate>::ConstIterator it;
  QDomElement paymentsElement = document.createElement("PAYMENTS");
  for (it=payments.begin(); it!=payments.end(); ++it) {
    QDomElement paymentEntry = document.createElement("PAYMENT");
    paymentEntry.setAttribute("date", dateToString(*it));
    paymentsElement.appendChild(paymentEntry);
  }
  el.appendChild(paymentsElement);

  //store the transaction data for this task.
  m_transaction.writeXML(document, el);

  parent.appendChild(el);
}

bool MyMoneySchedule::hasReferenceTo(const QCString& id) const
{
  return m_transaction.hasReferenceTo(id);
}

QString MyMoneySchedule::occurenceToString(occurenceE occurence)
{
  QString occurenceString = I18N_NOOP("Any");

  if(occurence == MyMoneySchedule::OCCUR_ONCE)
    occurenceString = I18N_NOOP("Once");
  else if(occurence == MyMoneySchedule::OCCUR_DAILY)
    occurenceString = I18N_NOOP("Daily");
  else if(occurence == MyMoneySchedule::OCCUR_WEEKLY)
    occurenceString = I18N_NOOP("Weekly");
  else if(occurence == MyMoneySchedule::OCCUR_FORTNIGHTLY)
    occurenceString = I18N_NOOP("Fortnightly");
  else if(occurence == MyMoneySchedule::OCCUR_EVERYOTHERWEEK)
    occurenceString = I18N_NOOP("Every other week");
  else if(occurence == MyMoneySchedule::OCCUR_EVERYTHREEWEEKS)
    occurenceString = I18N_NOOP("Every three weeks");
  else if(occurence == MyMoneySchedule::OCCUR_EVERYFOURWEEKS)
    occurenceString = I18N_NOOP("Every four weeks");
  else if(occurence == MyMoneySchedule::OCCUR_EVERYTHIRTYDAYS)
    occurenceString = I18N_NOOP("Every thirty days");
  else if(occurence == MyMoneySchedule::OCCUR_MONTHLY)
    occurenceString = I18N_NOOP("Monthly");
  else if(occurence == MyMoneySchedule::OCCUR_EVERYEIGHTWEEKS)
    occurenceString = I18N_NOOP("Every eight weeks");
  else if(occurence == MyMoneySchedule::OCCUR_EVERYOTHERMONTH)
    occurenceString = I18N_NOOP("Every two months");
  else if(occurence == MyMoneySchedule::OCCUR_EVERYTHREEMONTHS)
    occurenceString = I18N_NOOP("Every three months");
  else if(occurence == MyMoneySchedule::OCCUR_QUARTERLY)
    occurenceString = I18N_NOOP("Quarterly");
  else if(occurence == MyMoneySchedule::OCCUR_EVERYFOURMONTHS)
    occurenceString = I18N_NOOP("Every four months");
  else if(occurence == MyMoneySchedule::OCCUR_TWICEYEARLY)
    occurenceString = I18N_NOOP("Twice yearly");
  else if(occurence == MyMoneySchedule::OCCUR_YEARLY)
    occurenceString = I18N_NOOP("Yearly");
  else if(occurence == MyMoneySchedule::OCCUR_EVERYOTHERYEAR)
    occurenceString = I18N_NOOP("Every other year");
  return occurenceString;
}

QString MyMoneySchedule::scheduleTypeToString(MyMoneySchedule::typeE type)
{
  QString text;

  switch (type) {
    case MyMoneySchedule::TYPE_BILL:
      text = I18N_NOOP("Bill");
      break;
    case MyMoneySchedule::TYPE_DEPOSIT:
      text = I18N_NOOP("Deposit");
      break;
    case MyMoneySchedule::TYPE_TRANSFER:
      text = I18N_NOOP("Transfer");
      break;
    case MyMoneySchedule::TYPE_LOANPAYMENT:
      text = I18N_NOOP("Loan payment");
      break;
    case MyMoneySchedule::TYPE_ANY:
    default:
      text = I18N_NOOP("Unknown");
  }
  return text;
}


QString MyMoneySchedule::paymentMethodToString(MyMoneySchedule::paymentTypeE paymentType)
{
  QString text;

  switch (paymentType) {
    case MyMoneySchedule::STYPE_DIRECTDEBIT:
      text = I18N_NOOP("Direct debit");
      break;
    case MyMoneySchedule::STYPE_DIRECTDEPOSIT:
      text = I18N_NOOP("Direct deposit");
      break;
    case MyMoneySchedule::STYPE_MANUALDEPOSIT:
      text = I18N_NOOP("Manual deposit");
      break;
    case MyMoneySchedule::STYPE_OTHER:
      text = I18N_NOOP("Other");
      break;
    case MyMoneySchedule::STYPE_WRITECHEQUE:
      text = I18N_NOOP("Write check");
      break;
    case MyMoneySchedule::STYPE_STANDINGORDER:
      text = I18N_NOOP("Standing order");
      break;
    case MyMoneySchedule::STYPE_BANKTRANSFER:
      text = I18N_NOOP("Bank transfer");
      break;
    case MyMoneySchedule::STYPE_ANY:
      text = I18N_NOOP("Any (Error)");
      break;
  }
  return text;
}

QString MyMoneySchedule::weekendOptionToString(MyMoneySchedule::weekendOptionE weekendOption)
{
  QString text;

  switch (weekendOption) {
    case MyMoneySchedule::MoveFriday:
      text = I18N_NOOP("Change the date to the previous Friday");
      break;
    case MyMoneySchedule::MoveMonday:
      text = I18N_NOOP("Change the date to the next Monday");
      break;
    case MyMoneySchedule::MoveNothing:
      text = I18N_NOOP("Do Nothing");
      break;
  }
  return text;
}

