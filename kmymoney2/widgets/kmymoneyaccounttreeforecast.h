/***************************************************************************
                         kmymoneyaccounttreeforecast.h
                            -------------------
   begin                : Fri Aug 01 2008
   copyright            : (C) 2008 by Alvaro Soliverez
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

#ifndef KMYMONEYACCOUNTTREEFORECAST_H
#define KMYMONEYACCOUNTTREEFORECAST_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qtimer.h>
class QDragObject;

// ----------------------------------------------------------------------------
// KDE Includes

#include <klistview.h>

// ----------------------------------------------------------------------------
// Project Includes
#include "kmymoneyaccounttree.h"
#include "../../kmymoney/mymoneyforecast.h"

class KMyMoneyAccountTreeForecastItem;

class KMyMoneyAccountTreeForecast : public KMyMoneyAccountTreeBase
{
    Q_OBJECT
  public:
    KMyMoneyAccountTreeForecast(QWidget* parent = 0, const char *name = 0);
    virtual ~KMyMoneyAccountTreeForecast() {}

    void showSummary(MyMoneyForecast& forecast);
    void showDetailed(MyMoneyForecast& forecast);
    void showAdvanced(MyMoneyForecast& forecast);
    void showBudget(MyMoneyForecast& forecast);
    void showAccount(void);
    void clearColumns(void);

  public slots:
    void slotSelectObject(const QListViewItem* i);

};

class KMyMoneyAccountTreeForecastItem : public KMyMoneyAccountTreeBaseItem
{
public:

  enum EForecastViewType { eSummary = 0, eDetailed, eAdvanced, eBudget, eUndefined };

  /**
    * Constructor to be used to construct an account
    * entry object for a forecast.
    *
    * @param parent pointer to the parent KAccountListView object this entry should be
    *               added to.
    * @param account const reference to MyMoneyAccount for which
    *               the KListView entry is constructed
    * @param forecast const reference to the forecast to
    * which the account belongs
    * @param price price to be used to calculate value (defaults to 1)
    *              This is used for accounts denominated in foreign currencies or stocks
    * @param security const reference to the security used to show the value. Usually
    *                 one should pass MyMoneyFile::baseCurrency() here.
    */
  KMyMoneyAccountTreeForecastItem(KMyMoneyAccountTreeForecastItem *parent, const MyMoneyAccount& account, const MyMoneyForecast& forecast, const QValueList<MyMoneyPrice>& price = QValueList<MyMoneyPrice>(), const MyMoneySecurity& security = MyMoneySecurity(), const EForecastViewType forecastViewType = eUndefined);

  /**
    * Constructor to be used to construct an account
    * entry object for a forecast.
    *
    * @param parent pointer to the parent KAccountListView object this entry should be
    *               added to.
    * @param account const reference to MyMoneyAccount for which
    *               the KListView entry is constructed
    * @param forecast const reference to the forecast to
    * which the account belongs
    * @param security const reference to the security used to show the value. Usually
    *                 one should pass MyMoneyFile::baseCurrency() here.
    * @param name name of the account to be used instead of the one stored with @p account
    *               If empty, the one stored with @p account will be used. Default: empty
    */
  KMyMoneyAccountTreeForecastItem(KListView *parent, const MyMoneyAccount& account, const MyMoneyForecast &forecast, const MyMoneySecurity& security = MyMoneySecurity(), const QString& name = QString());

  ~KMyMoneyAccountTreeForecastItem();

  void setForecast(const MyMoneyForecast& forecast);
  void updateSummary(void);
  void updateDetailed(void);
  void updateBudget(void);
  void setDaysToBeginDay(int _days) {m_daysToBeginDay = _days;}
  void setForecastViewType(EForecastViewType forecastType) { m_forecastType = forecastType; }
  EForecastViewType forecastViewType(void) { return m_forecastType; }

protected:
   /**
    * Returns the current balance of this account.
    *
    * This is a pure virtual function, to allow subclasses to calculate
    * the balance in different ways.
    *
    * Parent items in the tree will only be recomputed if the balance() for
    * a son changes.
    * @param account Account to get the balance for
    * @return Balance of this account
    */
  MyMoneyMoney balance() const;
  void setAmount(int column, const MyMoneyMoney amount, const MyMoneySecurity security);
  void adjustParentValue(int column, const MyMoneyMoney& value);
  void setValue(int column, MyMoneyMoney amount, QDate forecastDate);

private:
  MyMoneyForecast m_forecast;
  int m_daysToBeginDay;
  QMap<int, MyMoneyMoney> m_values;
  EForecastViewType m_forecastType;
};

#endif

