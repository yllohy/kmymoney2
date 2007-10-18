/***************************************************************************
                          mymoneyforecast.cpp
                             -------------------
    begin                : Wed May 30 2007
    copyright            : (C) 2007 by Alvaro Soliverez
    email                : asoliverez@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// ----------------------------------------------------------------------------
// QT Includes

#include <qstring.h>
#include <qdatetime.h>

// ----------------------------------------------------------------------------
// KDE Includes
#include <kdebug.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "../kmymoneyutils.h"
#include "../kmymoneyglobalsettings.h"
#include "mymoneyfile.h"
#include "mymoneyforecast.h"
#include "mymoneytransactionfilter.h"



MyMoneyForecast::MyMoneyForecast()
{
  setForecastCycles(1);
  setAccountsCycle(1);
  setForecastDays(1);
}


void MyMoneyForecast::doForecast()
{

  int fDays = KMyMoneyGlobalSettings::forecastDays();
  int fMethod = KMyMoneyGlobalSettings::forecastMethod();
  int fAccCycle = KMyMoneyGlobalSettings::forecastAccountCycle();
  int fCycles = KMyMoneyGlobalSettings::forecastCycles();

  //validate settings
  if(fAccCycle < 1
     || fCycles < 1
     || fDays < 1)
  {
  throw new MYMONEYEXCEPTION("Illegal settings when calling doForecast. Settings must be higher than 0");
  }

  //initialize global variables
  setForecastDays(fDays);
  setAccountsCycle(fAccCycle);
  setForecastCycles(fCycles);

  //clear all data before calculating
  m_accountListPast.clear();
  m_accountList.clear();
  m_accountTrendList.clear();

  //set forecast accounts
  setForecastAccountList();

  switch(fMethod)
  {
    case SCHEDULED:
      doFutureScheduledForecast();
      break;
    case HISTORIC:
      pastTransactions();
      calculateDailyBalances();
      break;
    default:

      break;
  }
}

MyMoneyForecast::~MyMoneyForecast()
{
}

void MyMoneyForecast::pastTransactions()
{
  MyMoneyFile* file = MyMoneyFile::instance();
  QDate startDate = QDate::currentDate().addDays(-(historyDays()));
  QDate endDate = QDate::currentDate().addDays(-1);

  MyMoneyTransactionFilter filter;

  filter.setDateFilter(startDate, endDate);
  filter.setReportAllSplits(false);

  QValueList<MyMoneyTransaction> transactions = file->transactionList(filter);
  QValueList<MyMoneyTransaction>::const_iterator it_t = transactions.begin();

  //Check past transactions
  for(; it_t != transactions.end(); ++it_t ) {
    const QValueList<MyMoneySplit>& splits = (*it_t).splits();
    QValueList<MyMoneySplit>::const_iterator it_s = splits.begin();
    for(; it_s != splits.end(); ++it_s ) {
      if(!(*it_s).shares().isZero()) {
        MyMoneyAccount acc = file->account((*it_s).accountId());
        if(isForecastAccount(acc)) { //If it is one of the accounts we are checking, add the amount of the transaction
          dailyBalances balance;
          balance = m_accountListPast[acc.id()];
          int offset = startDate.daysTo((*it_t).postDate())+1;
          balance[offset] += (*it_s).shares();
          // check if this is a new account for us
          m_accountListPast[acc.id()] = balance;
        }
      }
    }
  }

  QMap<QString, QCString>::Iterator it_n;
  for(it_n = m_nameIdx.begin(); it_n != m_nameIdx.end(); ++it_n) {
    MyMoneyAccount acc = file->account(*it_n);
    m_accountListPast[acc.id()][0] = file->balance(acc.id(), startDate.addDays(-1)); //balance of startdate is the balance at the end of the day before
    for(int it_f = 1; it_f <= historyDays(); ++it_f) {
      m_accountListPast[acc.id()][it_f] += m_accountListPast[acc.id()][(it_f-1)]; //Running sum
    }

  }
}

bool MyMoneyForecast::isForecastAccount(MyMoneyAccount acc)
{
  if(m_nameIdx.isEmpty())
  {
    setForecastAccountList();
  }
  QMap<QString, QCString>::Iterator it_n;
  for(it_n = m_nameIdx.begin(); it_n != m_nameIdx.end(); ++it_n) {
    if(*it_n == acc.id())
    {
      return true;
    }
  }
  return false;
}

void MyMoneyForecast::calculateAccountTrendList()
{
  MyMoneyFile* file = MyMoneyFile::instance();
  int auxForecastTerms;

  //Calculate account trends
  QMap<QString, QCString>::Iterator it_n;
  for(it_n = m_nameIdx.begin(); it_n != m_nameIdx.end(); ++it_n) {
    MyMoneyAccount acc = file->account(*it_n);
    m_accountTrendList[acc.id()][0] = MyMoneyMoney(0,1); // for today, the trend is 0

    if(acc.openingDate().daysTo(QDate::currentDate()) < historyDays()) { //if acc opened after forecast period
      auxForecastTerms = 1 + (acc.openingDate().daysTo(QDate::currentDate()) / accountsCycle()); // set forecastTerms to a lower value, to calculate only based on how much this account was opened
    } else {
      auxForecastTerms = forecastCycles();
    }

    for(int t_day = 1; t_day <= accountsCycle(); t_day++) {
      m_accountTrendList[acc.id()][t_day] = calculateAccountDailyTrend(acc, t_day, accountsCycle(), auxForecastTerms);
    }
  }
}


QValueList<MyMoneyAccount> MyMoneyForecast::forecastAccountList(void)
{
  MyMoneyFile* file = MyMoneyFile::instance();

  QValueList<MyMoneyAccount> accList;
  QCStringList emptyStringList;
  //Get all accounts from the file and check if they are of the right type to calculate forecast
  file->accountList(accList, emptyStringList, false);
  QValueList<MyMoneyAccount>::iterator accList_t = accList.begin();
  for(; accList_t != accList.end(); ) {
    MyMoneyAccount acc = *accList_t;
    if(acc.isClosed()             //check the account is not closed
    || (!acc.isAssetLiability())
    || (acc.accountType() == MyMoneyAccount::Investment) //check that it is not an Investment account nor a Stock account
    || (acc.isInvest())) { //They could be included but you have to manage how to display the account name and how to calculate the balance
      accList.remove(accList_t);    //remove the account if it is not of the correct type
      accList_t = accList.begin();
    } else {
      ++accList_t;
    }
  }
  return accList;
}


MyMoneyMoney MyMoneyForecast::calculateAccountTrend(MyMoneyAccount acc, int trendDays)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  MyMoneyTransactionFilter filter;
  MyMoneyMoney netIncome;
  QDate startDate;
  QDate openingDate = acc.openingDate();

  //validate arguments
  if(trendDays < 1)
  {
    throw new MYMONEYEXCEPTION("Illegal arguments when calling calculateAccountTrend. trendDays must be higher than 0");
  }

  //If it is a new account, we dont take into account the first day
  //because it is usually a weird one and it would mess up the trend
  if(openingDate.daysTo(QDate::currentDate())<trendDays){
    startDate = (acc.openingDate()).addDays(1);
  }
  else {
    startDate = QDate::currentDate().addDays(-trendDays);
  }
  //get all transactions for the period
  filter.setDateFilter(startDate, QDate::currentDate());
  if(acc.accountGroup() == MyMoneyAccount::Income
     || acc.accountGroup() == MyMoneyAccount::Expense) {
    filter.addCategory(acc.id());
     } else {
       filter.addAccount(acc.id());
     }

  filter.setReportAllSplits(false);

  QValueList<MyMoneyTransaction> transactions = file->transactionList(filter);
  QValueList<MyMoneyTransaction>::const_iterator it_t = transactions.begin();



  //add all transactions for that account
  for(; it_t != transactions.end(); ++it_t ) {
    const QValueList<MyMoneySplit>& splits = (*it_t).splits();
    QValueList<MyMoneySplit>::const_iterator it_s = splits.begin();
    for(; it_s != splits.end(); ++it_s ) {
      if(!(*it_s).shares().isZero()) {
        if(acc.id()==(*it_s).accountId()) netIncome += (*it_s).value();
      }
    }
  }

  //calculate trend of the account in the past period
  MyMoneyMoney accTrend;

  if(openingDate.daysTo(QDate::currentDate())<trendDays){ //dont take into account the first day of the account
    accTrend = netIncome/MyMoneyMoney(openingDate.daysTo(QDate::currentDate())-1,1);
  }
  else {
    accTrend = netIncome/MyMoneyMoney(trendDays,1);
  }
  return accTrend;
}

MyMoneyMoney MyMoneyForecast::calculateAccountDailyTrend(MyMoneyAccount acc, int trendDay, int forecastTerm, int forecastTerms)
{
  //Calculate a daily trend for the account based on the accounts of a given number of terms
  //With a term of 1 month and 3 terms, it calculates the trend taking the transactions occured at that day and the day before,
  //for the last 3 months

  MyMoneyMoney balanceVariation;

  for(int it_terms=0; (trendDay+(forecastTerm*it_terms)) <= historyDays(); ++it_terms) //sum for each term
  {
    MyMoneyMoney balanceBefore = m_accountListPast[acc.id()][(trendDay+(forecastTerm*it_terms)-1)]; //get balance for the day before
    MyMoneyMoney balanceAfter = m_accountListPast[acc.id()][(trendDay+(forecastTerm*it_terms))];
    balanceVariation += (balanceAfter - balanceBefore); //add the balance variation between days
  }
  //calculate average of the variations
  return balanceVariation / MyMoneyMoney(forecastTerms,1);
}

void MyMoneyForecast::calculateDailyBalances()
{
  MyMoneyFile* file = MyMoneyFile::instance();

  calculateAccountTrendList();

  //Calculate account daily balances
  QMap<QString, QCString>::ConstIterator it_n;
  for(it_n = m_nameIdx.begin(); it_n != m_nameIdx.end(); ++it_n) {
    MyMoneyAccount acc = file->account(*it_n);

    m_accountList[acc.id()][0] = file->balance(acc.id(), QDate::currentDate());//Get current account balance

    for(int f_day = 1; f_day <= forecastDays();) {
      for(int t_day = 1; t_day <= accountsCycle(); ++t_day, ++f_day) {
        MyMoneyMoney balanceDayBefore = m_accountList[acc.id()][(f_day-1)];//balance of the day before
        MyMoneyMoney accountDailyTrend = m_accountTrendList[acc.id()][t_day]; //trend for that day
        //balance of the day is the balance of the day before multiplied by the trend for the day
        m_accountList[acc.id()][f_day] = balanceDayBefore;
        m_accountList[acc.id()][f_day] += accountDailyTrend; //movement trend for that particular day
        //m_accountList[acc.id()][f_day] += m_accountListPast[acc.id()][f_day]; //movement trend for that particular day
      }
    }
  }
}

MyMoneyMoney MyMoneyForecast::forecastBalance(MyMoneyAccount acc, QDate forecastDate)
{
  int offset = QDate::currentDate().daysTo(forecastDate);

  return forecastBalance(acc, offset);
}

/**
 * Returns the forecast balance trend for account @a acc for offset @p int
 * offset is days from current date, inside forecast days.
 * Returns 0 if offset not in range of forecast days.
 */

MyMoneyMoney MyMoneyForecast::forecastBalance ( MyMoneyAccount acc, int offset )
{
  dailyBalances balance;
  MyMoneyMoney MM_amount;

  //Check if acc is not a forecast account or date is in the past, return 0
  if ( !isForecastAccount ( acc )
          || offset < 0
          || offset > forecastDays() )
  {
    return MM_amount;
  }

  balance = m_accountList[acc.id() ];
  if ( balance.contains ( offset ) )
  { //if the date is not in the forecast, it returns 0
    MM_amount = m_accountList[acc.id() ][offset];
  }
  return MM_amount;
}

void MyMoneyForecast::doFutureScheduledForecast()
{
  MyMoneyFile* file = MyMoneyFile::instance();

  QDate endDate = QDate::currentDate().addDays(forecastDays());
  MyMoneyTransactionFilter filter;

  // collect and process all transactions that have already been entered but
  // are located in the future.
  filter.setDateFilter(QDate::currentDate().addDays(1), endDate);
  filter.setReportAllSplits(false);

  QValueList<MyMoneyTransaction> transactions = file->transactionList(filter);
  QValueList<MyMoneyTransaction>::const_iterator it_t = transactions.begin();

  for(; it_t != transactions.end(); ++it_t ) {
    const QValueList<MyMoneySplit>& splits = (*it_t).splits();
    QValueList<MyMoneySplit>::const_iterator it_s = splits.begin();
    for(; it_s != splits.end(); ++it_s ) {
      if(!(*it_s).shares().isZero()) {
        MyMoneyAccount acc = file->account((*it_s).accountId());
        if(isForecastAccount(acc)) {
          dailyBalances balance;
          balance = m_accountList[acc.id()];
          int offset = QDate::currentDate().daysTo((*it_t).postDate());
          balance[offset] += (*it_s).shares();
          // check if this is a new account for us
          if(m_nameIdx[acc.name()] != acc.id()) {
            m_nameIdx[acc.name()] = acc.id();
            balance[0] = file->balance(acc.id(), QDate::currentDate());
          }
        m_accountList[acc.id()] = balance;
        }
      }
    }
  }

#if 0
  QFile trcFile("forecast.csv");
  trcFile.open(IO_WriteOnly);
  QTextStream s(&trcFile);

  {
    s << "Already present transactions\n";
    QMap<QCString, dailyBalances>::Iterator it_a;
    QMap<QString, QCString>::ConstIterator it_n;
    for(it_n = m_nameIdx.begin(); it_n != m_nameIdx.end(); ++it_n) {
      MyMoneyAccount acc = file->account(*it_n);
      it_a = m_accountList.find(*it_n);
      s << "\"" << acc.name() << "\",";
      for(int i = 0; i < 90; ++i) {
        s << "\"" << (*it_a)[i].formatMoney("") << "\",";
      }
      s << "\n";
    }
  }
#endif

  // now process all the schedules that may have an impact
  QValueList<MyMoneySchedule> schedule;

  schedule = file->scheduleList("", MyMoneySchedule::TYPE_ANY, MyMoneySchedule::OCCUR_ANY, MyMoneySchedule::STYPE_ANY,
                                              QDate::currentDate(), endDate);
  if(schedule.count() > 0) {
    QValueList<MyMoneySchedule>::Iterator it;
    do {
      qBubbleSort(schedule);
      it = schedule.begin();
      if(it == schedule.end())
        break;

      QDate nextDate = (*it).nextPayment((*it).lastPayment());
      if(!nextDate.isValid()) {
        schedule.remove(it);
        continue;
      }

      if (nextDate > endDate) {
        // We're done with this schedule, let's move on to the next
        schedule.remove(it);
        continue;
      }

      // found the next schedule. process it

      MyMoneyAccount acc = (*it).account();

      if(!acc.id().isEmpty()) {
        try {
          if(acc.accountType() != MyMoneyAccount::Investment) {
            dailyBalances balance;
            balance = m_accountList[acc.id()];
            MyMoneyTransaction t = (*it).transaction();

            // only process the entry, if it is still active
            if(!(*it).isFinished() && nextDate != QDate()) {
              // make sure we have all 'starting balances' so that the autocalc works
              QValueList<MyMoneySplit>::const_iterator it_s;
              QMap<QCString, MyMoneyMoney> balanceMap;

              for(it_s = t.splits().begin(); it_s != t.splits().end(); ++it_s ) {
                MyMoneyAccount acc = file->account((*it_s).accountId());
                if(isForecastAccount(acc)) {
                  // check if this is a new account for us
/*                  if(m_nameIdx[acc.name()] != acc.id()) {
                    m_nameIdx[acc.name()] = acc.id();
                    m_accountList[acc.id()][0] = file->balance(acc.id());
                  }*/
                  int offset = QDate::currentDate().daysTo(nextDate);
                  if(offset <= 0) {  // collect all overdues on the first day
                    offset = 1;
                  }      QString amount;
                  dailyBalances balance;
                  balance = m_accountList[acc.id()];
                  for(int i = 0; i < offset; ++i) {
                    balanceMap[acc.id()] += m_accountList[acc.id()][i];
                  }
                }
              }

              // take care of the autoCalc stuff
              KMyMoneyUtils::calculateAutoLoan(*it, t, balanceMap);

              // now add the splits to the balances
              for(it_s = t.splits().begin(); it_s != t.splits().end(); ++it_s ) {
                MyMoneyAccount acc = file->account((*it_s).accountId());
                QString amount;
                dailyBalances balance;
                balance = m_accountList[acc.id()];
                if(isForecastAccount(acc)) {
                  dailyBalances balance;
                  balance = m_accountList[acc.id()];
                  int offset = QDate::currentDate().daysTo(nextDate);
                  if(offset <= 0) {  // collect all overdues on the first day
                    offset = 1;
                  }
                  balance[offset] += (*it_s).value();
                  m_accountList[acc.id()] = balance;
                }
              }
            }
          }
          (*it).setLastPayment(nextDate);

        } catch(MyMoneyException* e) {
          kdDebug(2) << __func__ << " Schedule " << (*it).id() << " (" << (*it).name() << "): " << e->what() << endl;

          schedule.remove(it);
          delete e;
        }
      } else {
        // remove schedule from list
        schedule.remove(it);
      }
    }
    while(1);
  }


#if 0
  {
  s << "\n\nAdded scheduled transactions\n";
  QMap<QCString, dailyBalances>::Iterator it_a;
  QMap<QString, QCString>::ConstIterator it_n;
  for(it_n = m_nameIdx.begin(); it_n != m_nameIdx.end(); ++it_n) {
    MyMoneyAccount acc = file->account(*it_n);
    it_a = m_accountList.find(*it_n);
    s << "\"" << acc.name() << "\",";
    for(int i = 0; i < 90; ++i) {
      s << "\"" << (*it_a)[i].formatMoney("") << "\",";
    }
    s << "\n";
  }
  }
#endif

  //Calculate account daily balances
  QMap<QString, QCString>::ConstIterator it_n;
  for(it_n = m_nameIdx.begin(); it_n != m_nameIdx.end(); ++it_n) {
    MyMoneyAccount acc = file->account(*it_n);

    m_accountList[acc.id()][0] = file->balance(acc.id(), QDate::currentDate());//Get current account balance

    for(int f_day = 1; f_day <= forecastDays(); ++f_day) {
      MyMoneyMoney balanceDayBefore = m_accountList[acc.id()][(f_day-1)];//balance of the day before
      m_accountList[acc.id()][f_day] += balanceDayBefore; //running sum
    }
  }
}

int MyMoneyForecast::daysToMinimumBalance(MyMoneyAccount acc) {
  QString minimumBalance = acc.value("minBalanceAbsolute");
  MyMoneyMoney minBalance = MyMoneyMoney(minimumBalance);
  dailyBalances balance;

  //Check if acc is not a forecast account, return -1
  if(!isForecastAccount(acc)) {
    return -1;
  }

  balance = m_accountList[acc.id()];

  int it_b;
  for(it_b = 0 ; it_b < forecastDays(); ++it_b) {
    MyMoneyMoney amount = balance[it_b];
    if(minBalance > balance[it_b]) {
      return it_b;
    }
  }
  return -1;
}

int MyMoneyForecast::daysToZeroBalance(MyMoneyAccount acc) {
  dailyBalances balance;

  //Check if acc is not a forecast account, return -1
  if(!isForecastAccount(acc)) {
    return -2;
  }

  balance = m_accountList[acc.id()];

  if (acc.accountGroup() == MyMoneyAccount::Asset) {
  int it_b;
    for (it_b = 0 ; it_b < forecastDays(); ++it_b )
    {
      MyMoneyMoney amount = balance[it_b];
      if ( balance[it_b] < MyMoneyMoney ( 0, 1 ) )
      {
        return it_b;
      }
    }
  } else if (acc.accountGroup() == MyMoneyAccount::Liability) {
    int it_b;
    for (it_b = 0  ; it_b < forecastDays() ; ++it_b )
    {
      if ( balance[it_b] > MyMoneyMoney ( 0, 1 ) )
      {
        return it_b;
      }
    }
  }
  return -1;
}


int MyMoneyForecast::historyDays() const
{
  return accountsCycle() * forecastCycles();
}

void MyMoneyForecast::setAccountsCycle(int accountsCycle)
{
  m_accountsCycle = accountsCycle;
}

int MyMoneyForecast::accountsCycle() const
{
  return m_accountsCycle;
}

void MyMoneyForecast::setForecastCycles(int forecastCycles)
{
  m_forecastCycles = forecastCycles;
}

int MyMoneyForecast::forecastCycles() const
{
  return m_forecastCycles;
}

void MyMoneyForecast::setForecastDays(int forecastDays)
{
  m_forecastDays = forecastDays;
}

int MyMoneyForecast::forecastDays() const
{
  return m_forecastDays;
}

void MyMoneyForecast::setForecastAccountList()
{

  //get forecast accounts
  QValueList<MyMoneyAccount> accList;
  accList = forecastAccountList();

  QValueList<MyMoneyAccount>::const_iterator accList_t = accList.begin();
  for(; accList_t != accList.end(); ++accList_t ) {
    MyMoneyAccount acc = *accList_t;
      // check if this is a new account for us
    if(m_nameIdx[acc.name()] != acc.id()) {
      m_nameIdx[acc.name()] = acc.id();
    }
  }

}
