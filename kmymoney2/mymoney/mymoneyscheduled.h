/***************************************************************************
                          mymoneyscheduled.h
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
#ifndef MYMONEYSCHEDULED_H
#define MYMONEYSCHEDULED_H

#include <qlist.h>
#include <qdatetime.h>

#include "mymoneytransaction.h"

/**
  *@author Michael Edwardes
  *
  * For now these two can stay in here but i'll separate them out at some point.
  */
class MyMoneyScheduledTransaction : public MyMoneyTransaction {
public:
  enum occurenceE {
    OCCUR_ONCE, OCCUR_DAILY, OCCUR_WEEKLY, OCCUR_OTHERWEEK, OCCUR_TWICEMONTHLY, OCCUR_FOURWEEKLY,
    OCCUR_MONTHLY, OCCUR_OTHERMONTH, OCCUR_THREEMONTH, OCCUR_FOURMONTH, OCCUR_TWICEYEAR,
    OCCUR_YEARLY, OCCUR_OTHERYEAR, OCCUR_ANY };

  enum typeE { TYPE_BILL, TYPE_DEPOSIT, TYPE_TRANSFER, TYPE_ANY };

  enum paymentTypeE { PAYMENT_CHEQUE, PAYMENT_DEPOSIT, PAYMENT_TRANSFER, PAYMENT_WITHDRAWAL, PAYMENT_ATM,
    PAYMENT_ANY };

private:
    int m_nYear, m_nMonth, m_nDay;
    QDate m_lastCheck;
    occurenceE m_occurence;
    typeE m_type;
    QDate m_startDate;
    paymentTypeE m_paymentType;
    bool m_fixed, m_bAuto, m_bEnd;
    long m_lNoLeft;
    QDate m_qdateFinal;

public:
  MyMoneyScheduledTransaction();
  MyMoneyScheduledTransaction(MyMoneyAccount *parent, const long id, transactionMethod methodType, const QString& number, const QString& memo,
                     const MyMoneyMoney& amount, const QDate& date, const QString& categoryMajor, const QString& categoryMinor, const QString& atmName,
                     const QString& fromTo, const QString& bankFrom, const QString& bankTo, stateE state);
  ~MyMoneyScheduledTransaction();
};

class MyMoneyScheduledList {
private:
  QList<MyMoneyScheduledTransaction> scheduledList;

public: 
  MyMoneyScheduledList();
  ~MyMoneyScheduledList();

  bool operator == (const MyMoneyScheduledList&);

  bool addScheduled(const MyMoneyScheduledTransaction& transaction);

  QList<MyMoneyScheduledTransaction> getScheduled(const MyMoneyScheduledTransaction::typeE type=MyMoneyScheduledTransaction::TYPE_ANY,
    const MyMoneyScheduledTransaction::paymentTypeE paymentType=MyMoneyScheduledTransaction::PAYMENT_ANY,
    const MyMoneyScheduledTransaction::occurenceE ocurrence=MyMoneyScheduledTransaction::OCCUR_ANY);

  QList<MyMoneyScheduledTransaction> getOverdue(const MyMoneyScheduledTransaction::typeE type=MyMoneyScheduledTransaction::TYPE_ANY,
    const MyMoneyScheduledTransaction::paymentTypeE paymentType=MyMoneyScheduledTransaction::PAYMENT_ANY,
    const MyMoneyScheduledTransaction::occurenceE ocurrence=MyMoneyScheduledTransaction::OCCUR_ANY);

  bool anyOverdue(const MyMoneyScheduledTransaction::paymentTypeE paymentType=MyMoneyScheduledTransaction::PAYMENT_ANY,
    const MyMoneyScheduledTransaction::typeE type=MyMoneyScheduledTransaction::TYPE_ANY);

  bool anyScheduled(const MyMoneyScheduledTransaction::paymentTypeE paymentType=MyMoneyScheduledTransaction::PAYMENT_ANY,
    const MyMoneyScheduledTransaction::typeE type=MyMoneyScheduledTransaction::TYPE_ANY);
};

#endif
