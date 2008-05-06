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
  setForecastCycles(KMyMoneyGlobalSettings::forecastCycles());
  setAccountsCycle(KMyMoneyGlobalSettings::forecastAccountCycle());
  setHistoryStartDate(QDate::currentDate().addDays(-forecastCycles()*accountsCycle()));
  setHistoryEndDate(QDate::currentDate().addDays(-1));
  setForecastDays(KMyMoneyGlobalSettings::forecastDays());
  setBeginForecastDay(KMyMoneyGlobalSettings::beginForecastDay());
  setForecastMethod(KMyMoneyGlobalSettings::forecastMethod());
  setHistoryMethod(KMyMoneyGlobalSettings::historyMethod());
  setSkipOpeningDate(true);
}


void MyMoneyForecast::doForecast()
{
  int fDays = calculateBeginForecastDay();
  int fMethod = forecastMethod();
  int fAccCycle = accountsCycle();
  int fCycles = forecastCycles();

  //validate settings
  if(fAccCycle < 1
     || fCycles < 1
     || fDays < 1)
  {
    throw new MYMONEYEXCEPTION("Illegal settings when calling doForecast. Settings must be higher than 0");
  }

  //initialize global variables
  setForecastDays(fDays);
  setForecastStartDate(QDate::currentDate().addDays(1));
  setForecastEndDate(QDate::currentDate().addDays(fDays));
  setAccountsCycle(fAccCycle);
  setForecastCycles(fCycles);
  setHistoryStartDate(forecastCycles() * accountsCycle());
  setHistoryEndDate(QDate::currentDate().addDays(-1)); //yesterday

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
  MyMoneyTransactionFilter filter;

  filter.setDateFilter(historyStartDate(), historyEndDate());
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
        if(isForecastAccount(acc) //If it is one of the accounts we are checking, add the amount of the transaction
          && ( (acc.openingDate() < (*it_t).postDate() && skipOpeningDate())
           || !skipOpeningDate() ) ){ //dont take the opening day of the account to calculate balance
          dailyBalances balance;
          //FIXME deal with leap years
          balance = m_accountListPast[acc.id()];
          if(acc.accountType() == MyMoneyAccount::Income) {//if it is income, the balance is stored as negative number
            balance[(*it_t).postDate()] += ((*it_s).shares() * MyMoneyMoney(-1, 1));
          } else {
            balance[(*it_t).postDate()] += (*it_s).shares();
          }
          // check if this is a new account for us
          m_accountListPast[acc.id()] = balance;
        }
      }
    }
  }

  //purge those accounts with no transactions on the period
  purgeForecastAccountsList(m_accountListPast);

  //calculate running sum
  QMap<QCString, QCString>::Iterator it_n;
  for(it_n = m_nameIdx.begin(); it_n != m_nameIdx.end(); ++it_n) {
    MyMoneyAccount acc = file->account(*it_n);
    m_accountListPast[acc.id()][historyStartDate().addDays(-1)] = file->balance(acc.id(), historyStartDate().addDays(-1));
    for(QDate it_date = historyStartDate(); it_date <= historyEndDate(); ) {
      m_accountListPast[acc.id()][it_date] += m_accountListPast[acc.id()][it_date.addDays(-1)]; //Running sum
      it_date = it_date.addDays(1);
    }
  }

  //adjust value of investments to deep currency
  for ( it_n = m_nameIdx.begin(); it_n != m_nameIdx.end(); ++it_n ) {
    MyMoneyAccount acc = file->account ( *it_n );

    if ( acc.isInvest() ) {
      //get the id of the security for that account
      MyMoneySecurity undersecurity = file->security ( acc.currencyId() );
      if ( ! undersecurity.isCurrency() ) //only do it if the security is not an actual currency
      {
        MyMoneyMoney rate = MyMoneyMoney ( 1, 1 ); //set the default value
        MyMoneyPrice price;

        for ( QDate it_date = historyStartDate().addDays(-1) ; it_date <= historyEndDate();) {
          //get the price for the tradingCurrency that day
          price = file->price ( undersecurity.id(), undersecurity.tradingCurrency(), it_date );
          if ( price.isValid() )
          {
            rate = price.rate ( undersecurity.tradingCurrency() );
          }
          //value is the amount of shares multiplied by the rate of the deep currency
          m_accountListPast[acc.id() ][it_date] = m_accountListPast[acc.id() ][it_date] * rate;
          it_date = it_date.addDays(1);
        }
      }
    }
  }
}

bool MyMoneyForecast::isForecastAccount(const MyMoneyAccount& acc)
{
  if(m_nameIdx.isEmpty())
  {
    setForecastAccountList();
  }
  QMap<QCString, QCString>::Iterator it_n;
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
  int totalWeight = 0;

  //Calculate account trends
  QMap<QCString, QCString>::Iterator it_n;
  for(it_n = m_nameIdx.begin(); it_n != m_nameIdx.end(); ++it_n) {
    MyMoneyAccount acc = file->account(*it_n);
    m_accountTrendList[acc.id()][0] = MyMoneyMoney(0,1); // for today, the trend is 0

    auxForecastTerms = forecastCycles();
    if(skipOpeningDate()) {
      if(acc.openingDate() > historyStartDate() ) { //if acc opened after forecast period
        auxForecastTerms = 1 + ((acc.openingDate().daysTo(historyEndDate()) + 1)/ accountsCycle()); // set forecastTerms to a lower value, to calculate only based on how long this account was opened
      }
    }

    switch (historyMethod())
    {
      case 1:
      { //weighted moving average
        //calculate total weight for moving average
        if(auxForecastTerms == forecastCycles()) {
          totalWeight = (auxForecastTerms * (auxForecastTerms + 1))/2; //totalWeight is the triangular number of auxForecastTerms
        } else {
        //if only taking a few periods, totalWeight is the sum of the weight for most recent periods
        for(int i = 1, w = forecastCycles(); i <= auxForecastTerms; ++i, --w)
          totalWeight += w;
        }
        for(int t_day = 1; t_day <= accountsCycle(); t_day++)
          m_accountTrendList[acc.id()][t_day] = accountWeightedMovingAverage(acc, t_day, totalWeight);
        break;
      }
      case 0:
      { //moving average
        for(int t_day = 1; t_day <= accountsCycle(); t_day++)
          m_accountTrendList[acc.id()][t_day] = accountMovingAverage(acc, t_day, auxForecastTerms); //moving average
        break;
      }
      default:
        break;
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
    || (!acc.isAssetLiability()) ) {
    //|| (acc.accountType() == MyMoneyAccount::Investment) ) {//check that it is not an Investment account and only include Stock accounts
      accList.remove(accList_t);    //remove the account if it is not of the correct type
      accList_t = accList.begin();
    } else {
      ++accList_t;
    }
  }
  return accList;
}

QValueList<MyMoneyAccount> MyMoneyForecast::accountList(void)
{
  MyMoneyFile* file = MyMoneyFile::instance();

  QValueList<MyMoneyAccount> accList;
  QCStringList emptyStringList;
  //Get all accounts from the file and check if they are present
  file->accountList(accList, emptyStringList, false);
  QValueList<MyMoneyAccount>::iterator accList_t = accList.begin();
  for(; accList_t != accList.end(); ) {
    MyMoneyAccount acc = *accList_t;
    if(!isForecastAccount( acc ) ) {
      accList.remove(accList_t);    //remove the account
      accList_t = accList.begin();
       } else {
         ++accList_t;
       }
  }
  return accList;
}

MyMoneyMoney MyMoneyForecast::calculateAccountTrend(const MyMoneyAccount& acc, int trendDays)
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

MyMoneyMoney MyMoneyForecast::accountMovingAverage(const MyMoneyAccount acc, const int trendDay, const int forecastTerms)
{
  //Calculate a daily trend for the account based on the accounts of a given number of terms
  //With a term of 1 month and 3 terms, it calculates the trend taking the transactions occured at that day and the day before,
  //for the last 3 months
  MyMoneyMoney balanceVariation;

  for(int it_terms = 0; (trendDay+(accountsCycle()*it_terms)) <= historyDays(); ++it_terms) //sum for each term
  {
    MyMoneyMoney balanceBefore = m_accountListPast[acc.id()][historyStartDate().addDays(trendDay+(accountsCycle()*it_terms)-2)]; //get balance for the day before
    MyMoneyMoney balanceAfter = m_accountListPast[acc.id()][historyStartDate().addDays(trendDay+(accountsCycle()*it_terms)-1)];
    balanceVariation += (balanceAfter - balanceBefore); //add the balance variation between days
  }
  //calculate average of the variations
  return balanceVariation / MyMoneyMoney(forecastTerms,1);
}

MyMoneyMoney MyMoneyForecast::accountWeightedMovingAverage(const MyMoneyAccount acc, const int trendDay, const int totalWeight)
{
  MyMoneyMoney balanceVariation;

  for(int it_terms = 0, weight = 1; (trendDay+(accountsCycle()*it_terms)) <= historyDays(); ++it_terms, ++weight) //sum for each term multiplied by weight
  {
    MyMoneyMoney balanceBefore = m_accountListPast[acc.id()][historyStartDate().addDays(trendDay+(accountsCycle()*it_terms)-2)]; //get balance for the day before
    MyMoneyMoney balanceAfter = m_accountListPast[acc.id()][historyStartDate().addDays(trendDay+(accountsCycle()*it_terms)-1)];
    balanceVariation += ( (balanceAfter - balanceBefore) * MyMoneyMoney(weight, 1) ); //add the balance variation between days multiplied by its weight
  }
  //calculate average of the variations
  return balanceVariation / MyMoneyMoney(totalWeight, 1);
}

void MyMoneyForecast::calculateDailyBalances()
{
  MyMoneyFile* file = MyMoneyFile::instance();

  calculateAccountTrendList();

  //Calculate account daily balances
  QMap<QCString, QCString>::ConstIterator it_n;
  for(it_n = m_nameIdx.begin(); it_n != m_nameIdx.end(); ++it_n) {
    MyMoneyAccount acc = file->account(*it_n);

    //Get current account balance
    if ( acc.isInvest() ) { //investments require special treatment
      //get the security id of that account
      MyMoneySecurity undersecurity = file->security ( acc.currencyId() );
      if ( ! undersecurity.isCurrency() ) //only do it if the security is not an actual currency
      {
        MyMoneyMoney rate = MyMoneyMoney ( 1, 1 ); //set the default value
        //get te
        MyMoneyPrice price = file->price ( undersecurity.id(), undersecurity.tradingCurrency(), QDate::currentDate() );
        if ( price.isValid() )
        {
          rate = price.rate ( undersecurity.tradingCurrency() );
        }
        m_accountList[acc.id()][QDate::currentDate()] = file->balance(acc.id(), QDate::currentDate()) * rate;
      }
    } else {
      m_accountList[acc.id()][QDate::currentDate()] = file->balance(acc.id(), QDate::currentDate());
    }

    for(QDate f_day = forecastStartDate(); f_day <= forecastEndDate(); ) {
      for(int t_day = 1; t_day <= accountsCycle(); ++t_day) {
        MyMoneyMoney balanceDayBefore = m_accountList[acc.id()][(f_day.addDays(-1))];//balance of the day before
        MyMoneyMoney accountDailyTrend = m_accountTrendList[acc.id()][t_day]; //trend for that day
        //balance of the day is the balance of the day before multiplied by the trend for the day
        m_accountList[acc.id()][f_day] = balanceDayBefore;
        m_accountList[acc.id()][f_day] += accountDailyTrend; //movement trend for that particular day
        //m_accountList[acc.id()][f_day] += m_accountListPast[acc.id()][f_day.addDays(-historyDays())];
        f_day = f_day.addDays(1);
      }
    }
  }
}

MyMoneyMoney MyMoneyForecast::forecastBalance(const MyMoneyAccount& acc, QDate forecastDate)
{

  dailyBalances balance;
  MyMoneyMoney MM_amount;

  //Check if acc is not a forecast account, return 0
  if ( !isForecastAccount ( acc ) )
  {
    return MM_amount;
  }

  balance = m_accountList[acc.id() ];
  if ( balance.contains ( forecastDate ) )
  { //if the date is not in the forecast, it returns 0
    MM_amount = m_accountList[acc.id() ][forecastDate];
  }
  return MM_amount;
}

/**
 * Returns the forecast balance trend for account @a acc for offset @p int
 * offset is days from current date, inside forecast days.
 * Returns 0 if offset not in range of forecast days.
 */
MyMoneyMoney MyMoneyForecast::forecastBalance (const MyMoneyAccount& acc, int offset )
{
  QDate forecastDate = QDate::currentDate().addDays(offset);
  return forecastBalance(acc, forecastDate);
}

void MyMoneyForecast::doFutureScheduledForecast(void)
{
  MyMoneyFile* file = MyMoneyFile::instance();

  //QDate endDate = forecastEndDate();
  MyMoneyTransactionFilter filter;

  // collect and process all transactions that have already been entered but
  // are located in the future.
  filter.setDateFilter(forecastStartDate(), forecastEndDate());
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
          balance[(*it_t).postDate()] += (*it_s).shares();
          // check if this is a new account for us
          if(m_nameIdx[acc.id()] != acc.id()) {
            m_nameIdx[acc.id()] = acc.id();
            balance[QDate::currentDate()] = file->balance(acc.id(), QDate::currentDate());
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
    QMap<QCString, QCString>::ConstIterator it_n;
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
                                QDate::currentDate(), forecastEndDate());
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

      if (nextDate > forecastEndDate()) {
        // We're done with this schedule, let's move on to the next
        schedule.remove(it);
        continue;
      }

      // found the next schedule. process it

      MyMoneyAccount acc = (*it).account();

      if(!acc.id().isEmpty()) {
        try {
          if(acc.accountType() != MyMoneyAccount::Investment) {
            MyMoneyTransaction t = (*it).transaction();

            // only process the entry, if it is still active
            if(!(*it).isFinished() && nextDate != QDate()) {
              // make sure we have all 'starting balances' so that the autocalc works
              QValueList<MyMoneySplit>::const_iterator it_s;
              QMap<QCString, MyMoneyMoney> balanceMap;

              for(it_s = t.splits().begin(); it_s != t.splits().end(); ++it_s ) {
                MyMoneyAccount acc = file->account((*it_s).accountId());
                if(isForecastAccount(acc)) {
                  // check if this is a new account for us and if so, keep the
                  // current balance in current date of the array
                  if(m_accountList[acc.id()][QDate::currentDate()].isZero())
                    m_accountList[acc.id()][QDate::currentDate()] = file->balance(acc.id(), QDate::currentDate());

                  // collect all overdues on the first day
                  QDate forecastDate = nextDate;
                  if(QDate::currentDate() >= nextDate)
                    forecastDate = QDate::currentDate().addDays(1);

                  dailyBalances balance;
                  balance = m_accountList[acc.id()];
                  for(QDate f_day = QDate::currentDate(); f_day < forecastDate; ) {
                    balanceMap[acc.id()] += m_accountList[acc.id()][f_day];
                    f_day = f_day.addDays(1);
                  }
                }
              }

              // take care of the autoCalc stuff
              KMyMoneyUtils::calculateAutoLoan(*it, t, balanceMap);

              // now add the splits to the balances
              for(it_s = t.splits().begin(); it_s != t.splits().end(); ++it_s ) {
                MyMoneyAccount acc = file->account((*it_s).accountId());
                if(isForecastAccount(acc)) {
                  dailyBalances balance;
                  balance = m_accountList[acc.id()];
                  //int offset = QDate::currentDate().daysTo(nextDate);
                  //if(offset <= 0) {  // collect all overdues on the first day
                  //  offset = 1;
                  //}
                  // collect all overdues on the first day
                  QDate forecastDate = nextDate;
                  if(QDate::currentDate() >= nextDate)
                    forecastDate = QDate::currentDate().addDays(1);

                  balance[forecastDate] += (*it_s).value();
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
  QMap<QCString, QCString>::ConstIterator it_n;
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
  //do not show accounts with no transactions
  purgeForecastAccountsList(m_accountList);

  //Calculate account daily balances
  QMap<QCString, QCString>::ConstIterator it_n;
  for(it_n = m_nameIdx.begin(); it_n != m_nameIdx.end(); ++it_n) {
    MyMoneyAccount acc = file->account(*it_n);

    m_accountList[acc.id()][QDate::currentDate()] = file->balance(acc.id(), QDate::currentDate());//Get current account balance

    for(QDate f_day = forecastStartDate(); f_day <= forecastEndDate(); ) {
      MyMoneyMoney balanceDayBefore = m_accountList[acc.id()][(f_day.addDays(-1))];//balance of the day before
      m_accountList[acc.id()][f_day] += balanceDayBefore; //running sum
      f_day = f_day.addDays(1);
    }
  }

    //adjust value of investments to deep currency
  for ( it_n = m_nameIdx.begin(); it_n != m_nameIdx.end(); ++it_n ) {
    MyMoneyAccount acc = file->account ( *it_n );

    if ( acc.isInvest() ) {
      //get the id of the security for that account
      MyMoneySecurity undersecurity = file->security ( acc.currencyId() );
      if ( ! undersecurity.isCurrency() ) //only do it if the security is not an actual currency
      {
        MyMoneyMoney rate = MyMoneyMoney ( 1, 1 ); //set the default value
        MyMoneyPrice price;

        for (QDate it_day = QDate::currentDate(); it_day <= forecastEndDate(); ) {
          //get the price for the tradingCurrency that day
          price = file->price ( undersecurity.id(), undersecurity.tradingCurrency(), it_day );
          if ( price.isValid() )
          {
            rate = price.rate ( undersecurity.tradingCurrency() );
          }
          //value is the amount of shares multiplied by the rate of the deep currency
          m_accountList[acc.id() ][it_day] = m_accountList[acc.id() ][it_day] * rate;
          it_day = it_day.addDays(1);
        }
      }
    }
  }
}

int MyMoneyForecast::daysToMinimumBalance(const MyMoneyAccount& acc)
{
  QString minimumBalance = acc.value("minBalanceAbsolute");
  MyMoneyMoney minBalance = MyMoneyMoney(minimumBalance);
  dailyBalances balance;

  //Check if acc is not a forecast account, return -1
  if(!isForecastAccount(acc)) {
    return -1;
  }

  balance = m_accountList[acc.id()];

  for(QDate it_day = QDate::currentDate() ; it_day <= forecastEndDate(); ) {
    if(minBalance > balance[it_day]) {
      return QDate::currentDate().daysTo(it_day);
    }
    it_day = it_day.addDays(1);
  }
  return -1;
}

int MyMoneyForecast::daysToZeroBalance(const MyMoneyAccount& acc)
{
  dailyBalances balance;

  //Check if acc is not a forecast account, return -1
  if(!isForecastAccount(acc)) {
    return -2;
  }

  balance = m_accountList[acc.id()];

  if (acc.accountGroup() == MyMoneyAccount::Asset) {
    for (QDate it_day = QDate::currentDate() ; it_day <= forecastEndDate(); )
    {
      if ( balance[it_day] < MyMoneyMoney ( 0, 1 ) )
      {
        return QDate::currentDate().daysTo(it_day);
      }
      it_day = it_day.addDays(1);
    }
  } else if (acc.accountGroup() == MyMoneyAccount::Liability) {
    for (QDate it_day = QDate::currentDate() ; it_day <= forecastEndDate(); )
    {
      if ( balance[it_day] > MyMoneyMoney ( 0, 1 ) )
      {
        return QDate::currentDate().daysTo(it_day);
      }
      it_day = it_day.addDays(1);
    }
  }
  return -1;
}

void MyMoneyForecast::setForecastAccountList(void)
{

  //get forecast accounts
  QValueList<MyMoneyAccount> accList;
  accList = forecastAccountList();

  QValueList<MyMoneyAccount>::const_iterator accList_t = accList.begin();
  for(; accList_t != accList.end(); ++accList_t ) {
    MyMoneyAccount acc = *accList_t;
    // check if this is a new account for us
    if(m_nameIdx[acc.id()] != acc.id()) {
      m_nameIdx[acc.id()] = acc.id();
    }
  }
}

MyMoneyMoney MyMoneyForecast::accountCycleVariation(const MyMoneyAccount& acc)
{
  MyMoneyMoney cycleVariation;

  if (forecastMethod() == HISTORIC) {
    for(int t_day = 1; t_day <= accountsCycle() ; ++t_day) {
      cycleVariation += m_accountTrendList[acc.id()][t_day];
    }
  }
  return cycleVariation;
}

MyMoneyMoney MyMoneyForecast::accountTotalVariation(const MyMoneyAccount& acc)
{
  MyMoneyMoney totalVariation;

  totalVariation = forecastBalance(acc, forecastEndDate()) - forecastBalance(acc, QDate::currentDate());
  return totalVariation;
}

QValueList<QDate> MyMoneyForecast::accountMinimumBalanceDateList(const MyMoneyAccount& acc)
{
  QValueList<QDate> minBalanceList;
  int daysToBeginDay;

  daysToBeginDay = QDate::currentDate().daysTo(beginForecastDate());

  for(int t_cycle = 0; ((t_cycle * accountsCycle()) + daysToBeginDay) < forecastDays() ; ++t_cycle) {
    MyMoneyMoney minBalance = forecastBalance(acc, (t_cycle * accountsCycle() + daysToBeginDay));
    QDate minDate = QDate::currentDate().addDays(t_cycle * accountsCycle() + daysToBeginDay);
    for(int t_day = 1; t_day <= accountsCycle() ; ++t_day) {
      if( minBalance > forecastBalance(acc, (t_cycle * accountsCycle()) + daysToBeginDay + t_day) ) {
        minBalance = forecastBalance(acc, (t_cycle * accountsCycle()) + daysToBeginDay + t_day );
        minDate = QDate::currentDate().addDays( (t_cycle * accountsCycle()) + daysToBeginDay + t_day);
      }
    }
    minBalanceList.append(minDate);
  }
  return minBalanceList;
}

QValueList<QDate> MyMoneyForecast::accountMaximumBalanceDateList(const MyMoneyAccount& acc)
{
  QValueList<QDate> maxBalanceList;
  int daysToBeginDay;

  daysToBeginDay = QDate::currentDate().daysTo(beginForecastDate());

  for(int t_cycle = 0; ((t_cycle * accountsCycle()) + daysToBeginDay) < forecastDays() ; ++t_cycle) {
    MyMoneyMoney maxBalance = forecastBalance(acc, ((t_cycle * accountsCycle()) + daysToBeginDay));
    QDate maxDate = QDate::currentDate().addDays((t_cycle * accountsCycle()) + daysToBeginDay);

    for(int t_day = 0; t_day < accountsCycle() ; ++t_day) {
      if( maxBalance < forecastBalance(acc, (t_cycle * accountsCycle()) + daysToBeginDay + t_day) ) {
        maxBalance = forecastBalance(acc, (t_cycle * accountsCycle()) + daysToBeginDay + t_day );
        maxDate = QDate::currentDate().addDays((t_cycle * accountsCycle()) + daysToBeginDay + t_day);
      }
    }
    maxBalanceList.append(maxDate);
  }
  return maxBalanceList;
}

MyMoneyMoney MyMoneyForecast::accountAverageBalance(const MyMoneyAccount& acc)
{
  MyMoneyMoney totalBalance;
  for(int f_day = 1; f_day <= forecastDays() ; ++f_day) {
    totalBalance += forecastBalance(acc, f_day);
  }
  return totalBalance / MyMoneyMoney( forecastDays(), 1);
}

int MyMoneyForecast::calculateBeginForecastDay()
{
  int fDays = forecastDays();
  int beginDay = beginForecastDay();
  int accCycle = accountsCycle();
  QDate beginDate;

  //if 0, beginDate is current date and forecastDays remains unchanged
  if(beginDay == 0) {
    setBeginForecastDate(QDate::currentDate());
    return fDays;
  }

  //adjust if beginDay more than days of current month
  if(QDate::currentDate().daysInMonth() < beginDay)
    beginDay = QDate::currentDate().daysInMonth();

  //if beginDay still to come, calculate and return
  if(QDate::currentDate().day() <= beginDay) {
    beginDate = QDate( QDate::currentDate().year(), QDate::currentDate().month(), beginDay);
    fDays += QDate::currentDate().daysTo(beginDate);
    setBeginForecastDate(beginDate);
    return fDays;
  }

  //adjust beginDay for next month
  if(QDate::currentDate().addMonths(1).daysInMonth() < beginDay)
    beginDay = QDate::currentDate().addMonths(1).daysInMonth();

  //if beginDay of next month comes before 1 interval, use beginDay next month
  if(QDate::currentDate().addDays(accCycle) >=
       (QDate(QDate::currentDate().addMonths(1).year(), QDate::currentDate().addMonths(1).month(), 1).addDays(beginDay-1) ) )
  {
    beginDate = QDate(QDate::currentDate().addMonths(1).year(), QDate::currentDate().addMonths(1).month(), 1).addDays(beginDay-1);
    fDays += QDate::currentDate().daysTo(beginDate);
  }
  else //add intervals to current beginDay and take the first after current date
  {
    beginDay = ((((QDate::currentDate().day()-beginDay)/accCycle) + 1) * accCycle) + beginDay;
    beginDate = QDate::currentDate().addDays(beginDay - QDate::currentDate().day());
    fDays += QDate::currentDate().daysTo(beginDate);
  }

  setBeginForecastDate(beginDate);
  return fDays;
}

void MyMoneyForecast::purgeForecastAccountsList(QMap<QCString, dailyBalances>& accountList)
{
  QMap<QCString, QCString>::Iterator it_n;
  for ( it_n = m_nameIdx.begin(); it_n != m_nameIdx.end(); ) {
    if(!accountList.contains(*it_n)) {
      m_nameIdx.remove(it_n);
      it_n = m_nameIdx.begin();
    } else
      ++it_n;
  }
}

void MyMoneyForecast::createBudget ( MyMoneyBudget& budget, QDate historyStart, QDate historyEnd, QDate budgetStart, QDate budgetEnd, const bool returnBudget )
{
  // clear all data except the id and name
  QString name = budget.name();
  budget = MyMoneyBudget(budget.id(), MyMoneyBudget());
  budget.setName(name);

  //check parameters
  if ( historyStart > historyEnd ||
       budgetStart > budgetEnd ||
       budgetStart <= historyEnd )
  {
    throw new MYMONEYEXCEPTION ( "Illegal parameters when trying to create budget" );
  }

  //set start date to 1st of month and end dates to last day of month, since we deal with full months in budget
  historyStart = QDate ( historyStart.year(), historyStart.month(), 1 );
  historyEnd = QDate ( historyEnd.year(), historyEnd.month(), historyEnd.daysInMonth() );
  budgetStart = QDate ( budgetStart.year(), budgetStart.month(), 1 );
  budgetEnd = QDate ( budgetEnd.year(), budgetEnd.month(), budgetEnd.daysInMonth() );

  //set forecast parameters
  setHistoryStartDate ( historyStart );
  setHistoryEndDate ( historyEnd );
  setForecastStartDate ( budgetStart );
  setForecastEndDate ( budgetEnd );
  setForecastDays ( budgetStart.daysTo ( budgetEnd ) + 1 );
  if ( budgetStart.daysTo ( budgetEnd ) > historyStart.daysTo ( historyEnd ) ) { //if history period is shorter than budget, use that one as the trend length
    setAccountsCycle ( historyStart.daysTo ( historyEnd ) ); //we set the accountsCycle to the base timeframe we will use to calculate the average (eg. 180 days, 365, etc)
  } else { //if one timeframe is larger than the other, but not enough to be 1 time larger, we take the lowest value
    setAccountsCycle ( budgetStart.daysTo ( budgetEnd ) );
  }
  setForecastCycles ( ( historyStart.daysTo ( historyEnd ) / accountsCycle() ) );
  if ( forecastCycles() == 0 ) //the cycles must be at least 1
    setForecastCycles ( 1 );

  //do not skip opening date
  setSkipOpeningDate ( false );

  //clear and set accounts list we are going to use. Actually categories, in this case
  m_nameIdx.clear();
  setBudgetAccountList();

  pastTransactions(); //get all transactions for history period

  calculateAccountTrendList();

  calculateMonthlyBalances();

  if ( returnBudget ) { //only fill the budget if it is going to be used
    //setup the budget itself
    MyMoneyFile* file = MyMoneyFile::instance();
    budget.setBudgetStart ( budgetStart );

    //go through all the accounts and add them to budget
    QMap<QCString, QCString>::ConstIterator it_nc;
    for ( it_nc = m_nameIdx.begin(); it_nc != m_nameIdx.end(); ++it_nc ) {
      MyMoneyAccount acc = file->account ( *it_nc );

      MyMoneyBudget::AccountGroup budgetAcc;
      budgetAcc.setId ( acc.id() );
      budgetAcc.setBudgetLevel ( MyMoneyBudget::AccountGroup::eMonthByMonth );

      for ( QDate f_date = forecastStartDate(); f_date <= forecastEndDate(); ) {
        MyMoneyBudget::PeriodGroup period;

        //add period to budget account
        period.setStartDate ( f_date );
        period.setAmount ( forecastBalance ( acc, f_date ) );
        budgetAcc.addPeriod ( f_date, period );

        f_date = f_date.addMonths ( 1 ); //next month
      }
      budget.setAccount ( budgetAcc, acc.id() ); //add budget account to budget
    }
  }
}

void MyMoneyForecast::setBudgetAccountList(void)
{
  //get budget accounts
  QValueList<MyMoneyAccount> accList;
  accList = budgetAccountList();

  QValueList<MyMoneyAccount>::const_iterator accList_t = accList.begin();
  for(; accList_t != accList.end(); ++accList_t ) {
    MyMoneyAccount acc = *accList_t;
      // check if this is a new account for us
    if(m_nameIdx[acc.id()] != acc.id()) {
      m_nameIdx[acc.id()] = acc.id();
    }
  }
}

QValueList<MyMoneyAccount> MyMoneyForecast::budgetAccountList(void)
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
       || (!acc.isIncomeExpense()) ) {
      accList.remove(accList_t);    //remove the account if it is not of the correct type
      accList_t = accList.begin();
       } else {
         ++accList_t;
       }
  }
  return accList;
}

void MyMoneyForecast::calculateMonthlyBalances()
{
  MyMoneyFile* file = MyMoneyFile::instance();

  //Calculate account monthly balances
  QMap<QCString, QCString>::ConstIterator it_n;
  for(it_n = m_nameIdx.begin(); it_n != m_nameIdx.end(); ++it_n) {
    MyMoneyAccount acc = file->account(*it_n);

    for( QDate f_date = forecastStartDate(); f_date <= forecastEndDate(); ) {
      for(int f_day = 1; f_day <= accountsCycle() && f_date <= forecastEndDate(); ++f_day) {
        MyMoneyMoney accountDailyTrend = m_accountTrendList[acc.id()][f_day]; //trend for that day
        //check for leap year
        if(f_date.month() == 2 && f_date.day() == 29)
          f_date = f_date.addDays(1); //skip 1 day
        m_accountList[acc.id()][QDate(f_date.year(), f_date.month(), 1)] += accountDailyTrend; //movement trend for that particular day
        f_date = f_date.addDays(1);
      }
    }
  }
}

