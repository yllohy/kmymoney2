/***************************************************************************
                          mymoneyscheduled.h
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

#ifndef MYMONEYSCHEDULED_H
#define MYMONEYSCHEDULED_H

#include <qlist.h>
#include <qdatetime.h>

#include "mymoneytransaction.h"
/**
  *@author Michael Edwardes
  */

class MyMoneyScheduled {
public:
  enum occurenceE { OCCUR_DAILY, OCCUR_WEEKLY, OCCUR_FORTNIGHTLY, OCCUR_MONTHLY, OCCUR_QUARTER, OCCUR_FOURMONTH, OCCUR_YEARLY, OCCUR_ANY };
  enum typeE { TYPE_BILL, TYPE_DEPOSIT, TYPE_TRANSFER, TYPE_ANY };
  enum paymentTypeE { PAYMENT_DIRECTDEBIT, PAYMENT_DIRECTDEPOSIT, PAYMENT_MANUALDEPOSIT, PAYMENT_OTHER,
    PAYMENT_STANDINGORDER, PAYMENT_WRITECHEQUE, PAYMENT_ANY };

  struct s_scheduleData {
    int m_year, m_month, m_day;
    QDate m_lastCheck;
    occurenceE m_occurence;
    typeE m_type;
    QDate m_startDate;
    paymentTypeE m_paymentType;
    bool m_fixed;
    MyMoneyTransaction m_transaction;
  };

private:
  QList<MyMoneyScheduled::s_scheduleData> scheduledList;

public: 
	MyMoneyScheduled();
	~MyMoneyScheduled();
	
	bool operator == (const MyMoneyScheduled&);
	
	bool addScheduled(const paymentTypeE paymentType, bool fixed, const occurenceE occurence, const typeE type, const int year, const int month, const int day, const MyMoneyTransaction transaction);
  QList<MyMoneyScheduled::s_scheduleData> getScheduled(const typeE type=TYPE_ANY, const paymentTypeE paymentType=PAYMENT_ANY, const occurenceE ocurrence=OCCUR_ANY);
  QList<MyMoneyScheduled::s_scheduleData> getOverdue(const typeE type, const paymentTypeE paymentType, const occurenceE ocurrence=OCCUR_ANY);
  QList<MyMoneyScheduled::s_scheduleData> getScheduledDates(const typeE type, const QDate start, const QDate end, const paymentTypeE paymentType, const occurenceE ocurrence=OCCUR_ANY);
  bool anyOverdue(const paymentTypeE paymentType, const MyMoneyScheduled::typeE type=TYPE_ANY);
  bool anyScheduled(const paymentTypeE paymentType, const MyMoneyScheduled::typeE type=TYPE_ANY);
  bool anyScheduledDates(const paymentTypeE paymentType, const MyMoneyScheduled::typeE type, const QDate start, const QDate end);
};

#endif
