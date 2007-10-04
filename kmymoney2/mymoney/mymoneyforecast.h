/***************************************************************************
                          mymoneyforecast.h
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

#ifndef MYMONEYFORECAST_H
#define MYMONEYFORECAST_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// ----------------------------------------------------------------------------
// QT Includes

#include <qmap.h>
#include <qvaluelist.h>
#include <qstring.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/mymoneyobject.h>
#include <kmymoney/mymoneyaccount.h>
#include <kmymoney/mymoneymoney.h>
#include <kmymoney/export.h>

/**
  *
  *
  * @author Alvaro Soliverez <asoliverez@gmail.com>
  */

class MyMoneyForecast
{
public:
  MyMoneyForecast();
  ~MyMoneyForecast();

  /**
   * calculate forecast based on historic transactions
   */
  void doForecast();

  /**
   * Returns the list of accounts to be forecast. Only Asset and Liability are returned.
   * Investment accounts are not forecast due to further complications in its calculations.
   */
  static QValueList<MyMoneyAccount> forecastAccountList(void);

  /**
   * Returns the balance trend for account @a acc based on a number of days @p forecastDays
   * Collects and processes all transactions in the past for the
   * same period of forecast and calculates the balance trend
   */
  static MyMoneyMoney calculateAccountTrend(MyMoneyAccount acc, int forecastDays);

  /**
   * Returns the forecast balance trend for account @a acc for day @p QDate
   */
  MyMoneyMoney forecastBalance(MyMoneyAccount acc, QDate forecastDate);

  /**
   * Returns the forecast balance trend for account @a acc for offset @p int
   * offset is days from current date, inside forecast days.
   * Returns 0 if offset not in range of forecast days.
   */
  MyMoneyMoney forecastBalance(MyMoneyAccount acc, int offset);

  /**
   * Returns true if an account @a acc is an account to be forecast
   */
  bool isForecastAccount(MyMoneyAccount acc);

  /**
   * returns the number of days when a given account is forecast to be below minimum balance
   * returns -1 if it will not be below minimum balance in the forecast period
   */
  int daysToMinimumBalance(MyMoneyAccount acc);

  /**
   * returns the number of days when a given account is forecast to be below zero if it is an asset accounts
   * or above zero if it is a liability account
   * returns -1 if it will not happen in the forecast period
   */
  int daysToZeroBalance(MyMoneyAccount acc);

  /**
   * number of days to go back in history to calculate forecast
   */
  int historyDays(void) const;

  void setAccountsCycle(int accountsCycle);

  int accountsCycle(void) const;

  void setForecastCycles(int forecastCycles);

  int forecastCycles(void) const;

  void setForecastDays(int forecastDays);

  int forecastDays(void) const;

private:

  static const int SCHEDULED = 0;

  static const int HISTORIC = 1;

  /**
   * calculate daily forecast balance based on historic transactions
   */
  void calculateDailyBalances(void);

  /**
   * calculate forecast based on future and scheduled transactions
   */
  void doFutureScheduledForecast(void);

  /**
   * Returns the day trend for the account @a acc based on the daily balances of a given number of @p forecastTerms
   * It returns the trend for a given @p trendDay of the forecastTerm
   * With a term of 1 month and 3 terms, it calculates the trend taking the transactions occured
   * at that day and the day before,for the last 3 months
   */
  MyMoneyMoney calculateAccountDailyTrend(MyMoneyAccount acc, int trendDay, int forecastTerm, int forecastTerms);

  /**
   * calculate daily forecast trend based on historic transactions
   */
  void calculateAccountTrendList(void);

  /**
   * set the internal list of accounts to be forecast
   */
  void setForecastAccountList(void);

  /**
   * get past transactions for the accounts to be forecast
   */
  void pastTransactions(void);

  /**
   * daily balances of an account
   */
  typedef QMap<int, MyMoneyMoney> dailyBalances;

  /**
   * daily forecast balance of accounts
   */
  QMap<QCString, dailyBalances> m_accountList;

  /**
   * daily past balance of accounts
   */
  QMap<QCString, dailyBalances> m_accountListPast;

  /**
   * daily forecast trends of accounts
   */
  QMap<QCString, dailyBalances> m_accountTrendList;

  /**
   * list of forecast accounts
   */
  QMap<QString, QCString> m_nameIdx;

  /**
   * cycle of accounts in days
   */

  int m_accountsCycle;

  /**
   * number of cycles to use in forecast
   */
  int m_forecastCycles;

  /**
   * number of days to forecast
   */
  int m_forecastDays;

};

#endif // MYMONEYFORECAST_H

