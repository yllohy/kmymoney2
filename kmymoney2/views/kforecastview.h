/***************************************************************************
                             kforecastview.h
                             -------------------
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

#ifndef KFORECASTVIEW_H
#define KFORECASTVIEW_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/mymoneyaccount.h>
#include <kmymoney/mymoneyutils.h>

#include "../views/kforecastviewdecl.h"
#include "../widgets/kmymoneyaccounttreeforecast.h"

/**
  * @author Alvaro Soliverez
  */

/**
  * This class implements the forecast 'view'.
  */
class KForecastView : public KForecastViewDecl
{
  Q_OBJECT
private:

public:
  KForecastView(QWidget *parent=0, const char *name=0);
  virtual ~KForecastView();

  void show(void);

public slots:
  void slotLoadForecast(void);
  void slotManualForecast(void);

protected:
  typedef enum {
    SummaryView = 0,
    ListView,
    AdvancedView,
    BudgetView,
    // insert new values above this line
    MaxViewTabs
  } ForecastViewTab;

  QMap<QCString, QCString> m_nameIdx;


  /**
    * This method loads the forecast view.
    *
    */
  void loadForecast(ForecastViewTab tab);

  /**
    * This method loads the detailed view
    *
    */
  void loadListView(void);

  /**
   * This method loads the summary view
   *
   */
  void loadSummaryView(void);

  /**
   * This method loads the advanced view
   *
   */
  void loadAdvancedView(void);

  /**
   * This method loads the budget view
   *
   */
  void loadBudgetView(void);

  /**
   * This method loads the settings from user configuration
   *
   */
  void loadForecastSettings(void);

protected slots:
  void slotTabChanged(QWidget*);
  
  /**
   * Get the list of prices for an account
   * This is used later to create an instance of KMyMoneyAccountTreeForecastItem
   *
   */
  QValueList<MyMoneyPrice> getAccountPrices(const MyMoneyAccount& acc);

private:
  void addAssetLiabilityRows(const MyMoneyForecast& forecast);
  void addIncomeExpenseRows(const MyMoneyForecast& forecast);
  void addTotalRow(KMyMoneyAccountTreeForecast* forecastList, const MyMoneyForecast& forecast);
  bool includeAccount(MyMoneyForecast& forecast, const MyMoneyAccount& acc);
  void loadAccounts(MyMoneyForecast& forecast, const MyMoneyAccount& account, KMyMoneyAccountTreeForecastItem* parentItem, int forecastType);

  bool                                m_needReload[MaxViewTabs];
  KMyMoneyAccountTreeForecastItem*    m_totalItem;
  KMyMoneyAccountTreeForecastItem*    m_assetItem;
  KMyMoneyAccountTreeForecastItem*    m_liabilityItem;
  KMyMoneyAccountTreeForecastItem*    m_incomeItem;
  KMyMoneyAccountTreeForecastItem*    m_expenseItem;


};

#endif
