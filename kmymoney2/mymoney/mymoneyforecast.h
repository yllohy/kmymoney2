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
#include "mymoneybudget.h"

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
   * Returns the list of accounts to be forecast.
   */
  QValueList<MyMoneyAccount> accountList(void);

  /**
   * Returns the balance trend for account @a acc based on a number of days @p forecastDays
   * Collects and processes all transactions in the past for the
   * same period of forecast and calculates the balance trend
   */
  static MyMoneyMoney calculateAccountTrend(const MyMoneyAccount& acc, int forecastDays);

  /**
   * Returns the forecast balance trend for account @a acc for day @p QDate
   */
  MyMoneyMoney forecastBalance(const MyMoneyAccount& acc, QDate forecastDate);

  /**
   * Returns the forecast balance trend for account @a acc for offset @p int
   * offset is days from current date, inside forecast days.
   * Returns 0 if offset not in range of forecast days.
   */
  MyMoneyMoney forecastBalance(const MyMoneyAccount& acc, int offset);

  /**
   * Returns true if an account @a acc is an account to be forecast
   */
  bool isForecastAccount(const MyMoneyAccount& acc);

  /**
   * returns the number of days when a given account is forecast to be below minimum balance
   * returns -1 if it will not be below minimum balance in the forecast period
   */
  int daysToMinimumBalance(const MyMoneyAccount& acc);

  /**
   * returns the number of days when a given account is forecast to be below zero if it is an asset accounts
   * or above zero if it is a liability account
   * returns -1 if it will not happen in the forecast period
   */
  int daysToZeroBalance(const MyMoneyAccount& acc);

  /**
   * amount of variation of a given account in one cycle
   */
  MyMoneyMoney accountCycleVariation(const MyMoneyAccount& acc);

  /**
   * amount of variation of a given account for the whole forecast period
   */
  MyMoneyMoney accountTotalVariation(const MyMoneyAccount& acc);

  /**
   * returns a list of the dates where the account was on its lowest balance in each cycle
   */
  QValueList<QDate> accountMinimumBalanceDateList(const MyMoneyAccount& acc);

  /**
   * returns a list of the dates where the account was on its highest balance in each cycle
   */
  QValueList<QDate> accountMaximumBalanceDateList(const MyMoneyAccount& acc);

  /**
   * returns the average balance of the account within the forecast period
   */
  MyMoneyMoney accountAverageBalance(const MyMoneyAccount& acc);

  /**
   * creates a budget based on the history of a given timeframe
   */
  void createBudget(MyMoneyBudget& budget, QDate historyStart, QDate historyEnd, QDate budgetStart, QDate budgetEnd, const bool returnBudget);

  /**
   * number of days to go back in history to calculate forecast
   */
  int historyDays(void) const { return (m_historyStartDate.daysTo(m_historyEndDate) + 1); }

  void setAccountsCycle(int accountsCycle)   { m_accountsCycle = accountsCycle; }
  void setForecastCycles(int forecastCycles)   { m_forecastCycles = forecastCycles; }
  void setForecastDays(int forecastDays)   { m_forecastDays = forecastDays; }
  void setBeginForecastDate(QDate beginForecastDate) { m_beginForecastDate = beginForecastDate; }
  void setBeginForecastDay(int beginDay)   { m_beginForecastDay = beginDay; }
  void setForecastMethod(int forecastMethod) { m_forecastMethod = forecastMethod; }
  void setHistoryStartDate(QDate historyStartDate) { m_historyStartDate = historyStartDate; }
  void setHistoryEndDate(QDate historyEndDate) { m_historyEndDate = historyEndDate; }
  void setHistoryStartDate(int daysToStartDate) { setHistoryStartDate(QDate::currentDate().addDays(-daysToStartDate)); }
  void setHistoryEndDate(int daysToEndDate) { setHistoryEndDate(QDate::currentDate().addDays(-daysToEndDate)); }
  void setForecastStartDate(QDate _startDate) { m_forecastStartDate = _startDate; }
  void setForecastEndDate(QDate _endDate) { m_forecastEndDate = _endDate; }
  void setSkipOpeningDate(bool _skip) { m_skipOpeningDate = _skip; }
  void setHistoryMethod(int historyMethod) { m_historyMethod = historyMethod; }
  void setIncludeUnusedAccounts(bool _bool) { m_includeUnusedAccounts = _bool; }
  void setForecastDone(bool _bool) { m_forecastDone = _bool; }
  void setIncludeFutureTransactions(bool _bool) { m_includeFutureTransactions = _bool; }
  void setIncludeScheduledTransactions(bool _bool) { m_includeScheduledTransactions = _bool; }

  int accountsCycle(void) const   { return m_accountsCycle; }
  int forecastCycles(void) const   { return m_forecastCycles; }
  int forecastDays(void) const { return m_forecastDays; }
  QDate beginForecastDate(void) const   { return m_beginForecastDate; }
  int beginForecastDay(void) const   { return m_beginForecastDay; }
  int forecastMethod(void) const   { return m_forecastMethod; }
  QDate historyStartDate(void) const { return m_historyStartDate; }
  QDate historyEndDate(void) const { return m_historyEndDate; }
  QDate forecastStartDate(void) const { return m_forecastStartDate; }
  QDate forecastEndDate(void) const { return m_forecastEndDate; }
  bool skipOpeningDate(void) const { return m_skipOpeningDate; }
  int historyMethod(void) const   { return m_historyMethod; }
  bool isIncludingUnusedAccounts(void) const { return m_includeUnusedAccounts; }
  bool isForecastDone(void) const { return m_forecastDone; }
  bool isIncludingFutureTransactions(void) const { return m_includeFutureTransactions; }
  bool isIncludingScheduledTransactions(void) const { return m_includeScheduledTransactions; }

private:

  enum EForecastMethod {eScheduled = 0, eHistoric = 1 };

  /**
   * daily balances of an account
   */
  typedef QMap<QDate, MyMoneyMoney> dailyBalances;

  /**
   * map of trends of an account
   */
  typedef QMap<int, MyMoneyMoney> trendBalances;

  /**
   * Returns the list of accounts to be forecast. Only Asset and Liability are returned.
   */
  static QValueList<MyMoneyAccount> forecastAccountList(void);

  /**
   * Returns the list of accounts to create a budget. Only Income and Expenses are returned.
   */
  QValueList<MyMoneyAccount> budgetAccountList(void);

  /**
   * calculate daily forecast balance based on historic transactions
   */
  void calculateHistoricDailyBalances(void);

  /**
   * calculate monthly budget balance based on historic transactions
   */
  void calculateHistoricMonthlyBalances();

  /**
   * calculate monthly budget balance based on historic transactions
   */
  void calculateScheduledMonthlyBalances();

  /**
   * calculate forecast based on future and scheduled transactions
   */
  void doFutureScheduledForecast(void);

  /**
   * add future transactions to forecast
   */
  void addFutureTransactions(void);

  /**
   * add scheduled transactions to forecast
   */
  void addScheduledTransactions (void);

  /**
   * calculate daily forecast balance based on future and scheduled transactions
   */
  void calculateScheduledDailyBalances(void);

  /**
   * set the starting balance for an accounts
   */
  void setStartingBalance(const MyMoneyAccount& acc);

  /**
   * Returns the day moving average for the account @a acc based on the daily balances of a given number of @p forecastTerms
   * It returns the moving average for a given @p trendDay of the forecastTerm
   * With a term of 1 month and 3 terms, it calculates the trend taking the transactions occured
   * at that day and the day before,for the last 3 months
   */
  MyMoneyMoney accountMovingAverage(const MyMoneyAccount& acc, const int trendDay, const int forecastTerms);

  /**
   * Returns the weighted moving average for a given @p trendDay
   */
  MyMoneyMoney accountWeightedMovingAverage(const MyMoneyAccount& acc, const int trendDay, const int totalWeight);

  /**
   * Returns the linear regression for a given @p trendDay
   */
  MyMoneyMoney accountLinearRegression(const MyMoneyAccount &acc, const int trendDay, const int totalWeight, const MyMoneyMoney meanTerms);

  /**
   * calculate daily forecast trend based on historic transactions
   */
  void calculateAccountTrendList(void);

  /**
   * set the internal list of accounts to be forecast
   */
  void setForecastAccountList(void);

  /**
   * set the internal list of accounts to create a budget
   */
  void setBudgetAccountList(void);

  /**
   * get past transactions for the accounts to be forecast
   */
  void pastTransactions(void);

  /**
   * calculate the day to start forecast and sets the begin date
   * The quantity of forecast days will be counted from this date
   * Depends on the values of begin day and accounts cycle
   * The rules to calculate begin day are as follows:
   * - if beginDay is 0, begin date is current date
   * - if the day of the month set by beginDay has not passed, that will be used
   * - if adding an account cycle to beginDay, will not go past the beginDay of next month,
   *   that date will be used, otherwise it will add account cycle to beginDay until it is past current date
   * It returns the total amount of Forecast Days from current date.
   */
  int calculateBeginForecastDay();

  /**
   * remove accounts from the list if the accounts has no transactions in the forecast timeframe.
   * Used for scheduled-forecast method.
   */
  void purgeForecastAccountsList(QMap<QString, dailyBalances>& accountList);

  /**
   * daily forecast balance of accounts
   */
  QMap<QString, dailyBalances> m_accountList;

  /**
   * daily past balance of accounts
   */
  QMap<QString, dailyBalances> m_accountListPast;

  /**
   * daily forecast trends of accounts
   */
  QMap<QString, trendBalances> m_accountTrendList;

  /**
   * list of forecast accounts
   */
  QMap<QString, QString> m_nameIdx;

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

  /**
   * date to start forecast
   */
  QDate m_beginForecastDate;

  /**
   * day to start forecast
   */
  int m_beginForecastDay;

  /**
   * forecast method
   */
  int m_forecastMethod;

  /**
   * history method
   */
  int m_historyMethod;

  /**
   * start date of history
   */
  QDate m_historyStartDate;

  /**
   * end date of history
   */
  QDate m_historyEndDate;

  /**
   * start date of forecast
   */
  QDate m_forecastStartDate;

  /**
   * end date of forecast
   */
  QDate m_forecastEndDate;

  /**
   * skip opening date when fetching transactions of an account
   */
  bool m_skipOpeningDate;

  /**
   * include accounts with no transactions in the forecast timeframe. default is false.
   */
  bool m_includeUnusedAccounts;

  /**
   * forecast already done
   */
  bool m_forecastDone;

  /**
   * include future transactions when doing a scheduled-based forecast
   */
  bool m_includeFutureTransactions;

  /**
   * include scheduled transactions when doing a scheduled-based forecast
   */
  bool m_includeScheduledTransactions;

};

#endif // MYMONEYFORECAST_H

