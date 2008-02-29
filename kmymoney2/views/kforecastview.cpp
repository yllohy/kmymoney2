/***************************************************************************
                          kforecastview.cpp
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

// ----------------------------------------------------------------------------
// QT Includes

#include <qtabwidget.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kdebug.h>
#include <klocale.h>
#include <klistview.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/mymoneyfile.h>
#include "kforecastview.h"
#include "../kmymoneyglobalsettings.h"
#include "../kmymoney2.h"
#include "../kmymoneyutils.h"
#include "../mymoney/mymoneyforecast.h"
#include "../widgets/kmymoneyforecastlistviewitem.h"
#include "../reports/reportaccount.h"

using namespace reports;

KForecastView::KForecastView(QWidget *parent, const char *name) :
  KForecastViewDecl(parent,name)
{
  for(int i=0; i < MaxViewTabs; ++i)
    m_needReload[i] = false;

  KConfig *config = KGlobal::config();
  config->setGroup("Last Use Settings");
  m_tab->setCurrentPage(config->readNumEntry("KForecastView_LastType", 0));

  connect(m_tab, SIGNAL(currentChanged(QWidget*)), this, SLOT(slotTabChanged(QWidget*)));

  connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotLoadForecast()));

  m_forecastList->setAllColumnsShowFocus(true);
  m_summaryList->setAllColumnsShowFocus(true);
  m_adviceList->setAllColumnsShowFocus(true);
  m_advancedList->setAllColumnsShowFocus(true);
}

KForecastView::~KForecastView()
{
}

void KForecastView::slotTabChanged(QWidget* _tab)
{
  ForecastViewTab tab = static_cast<ForecastViewTab>(m_tab->indexOf(_tab));

  // remember this setting for startup
  KConfig *config = KGlobal::config();
  config->setGroup("Last Use Settings");
  config->writeEntry("KForecastView_LastType", tab);

  loadForecast(tab);

}

void KForecastView::loadForecast(ForecastViewTab tab)
{
  if(m_needReload[tab]) {
    switch(tab) {
      case ListView:
        loadListView();
        break;
      case SummaryView:
        loadSummaryView();
        break;
      case AdvancedView:
        loadAdvancedView();
        break;
      default:
        break;
    }
    m_needReload[tab] = false;
  }
}

void KForecastView::show(void)
{
  // don't forget base class implementation
  KForecastViewDecl::show();
  slotTabChanged(m_tab->currentPage());
}

void KForecastView::slotLoadForecast(void)
{
  m_needReload[SummaryView] = true;
  m_needReload[ListView] = true;
  m_needReload[AdvancedView] = true;
  if(isVisible())
    slotTabChanged(m_tab->currentPage());
}

void KForecastView::loadListView(void)
{
  MyMoneyFile* file = MyMoneyFile::instance();

  QValueList<MyMoneyAccount> accList;
  MyMoneySecurity baseCurrency = file->baseCurrency();

  MyMoneyForecast forecast;

  //Get all accounts of the right type to calculate forecast
  forecast.doForecast();
  accList = forecast.accountList();
  QValueList<MyMoneyAccount>::const_iterator accList_t = accList.begin();
  for(; accList_t != accList.end(); ++accList_t ) {
    MyMoneyAccount acc = *accList_t;
    if(nameIdx[acc.name()] != acc.id()) { //Check if the account is there
        nameIdx[acc.name()] = acc.id();
    }
  }

  //clear the list, including columns
  m_forecastList->clear();
  for(;m_forecastList->columns() > 0;) {
    m_forecastList->removeColumn(0);
  }

  //add first column of both lists
  int accountColumn = m_forecastList->addColumn(i18n("Account"), -1);

  //add cycle interval columns
  m_forecastList->addColumn(i18n("Current"), -1);
  for(int i = 1; i <= forecast.forecastDays(); ++i) {
    QDate forecastDate = QDate::currentDate().addDays(i);
    QString columnName =  forecastDate.toString(Qt::LocalDate);
    m_forecastList->addColumn(columnName, -1);
  }

  //add variation columns
  m_forecastList->addColumn(i18n("Total variation"), -1);

  //align columns
  for(int i = 1; i < m_forecastList->columns(); ++i) {
    m_forecastList->setColumnAlignment(i, Qt::AlignRight);
  }

  m_forecastList->setSorting(-1);

  KMyMoneyForecastListViewItem *forecastItem = 0;
  QMap<QDate, MyMoneyMoney> cycleBalance;

  QMap<QString, QCString>::ConstIterator it_nc;
  for(it_nc = nameIdx.begin(); it_nc != nameIdx.end(); ++it_nc) {

    MyMoneyAccount acc = file->account(*it_nc);
    MyMoneySecurity currency;
    if(acc.isInvest()) {
      MyMoneySecurity underSecurity = file->security(acc.currencyId());
      currency = file->security(underSecurity.tradingCurrency());
    } else {
        currency = file->security(acc.currencyId());
    }
    QString amount;
    QString vAmount;
    MyMoneyMoney vAmountMM;

    forecastItem = new KMyMoneyForecastListViewItem(m_forecastList, forecastItem, false);
    forecastItem->setText(accountColumn, acc.name());
    int it_c = 1; // iterator for the columns of the listview

    for(int i = 0; i <= forecast.forecastDays(); ++i) {
      QDate forecastDate = QDate::currentDate().addDays(i);

      MyMoneyMoney amountMM;
      amountMM = forecast.forecastBalance(acc, forecastDate);

      //calculate the balance in base currency for the total row
      if(acc.currencyId() != file->baseCurrency().id()) {
        ReportAccount repAcc = ReportAccount(acc.id());
        MyMoneyMoney curPrice = repAcc.baseCurrencyPrice(forecastDate);
        MyMoneyMoney baseAmountMM = amountMM * curPrice;
        cycleBalance[forecastDate] += baseAmountMM;

      } else {
        cycleBalance[forecastDate] += amountMM;
      }

      amount = amountMM.formatMoney(acc, currency);
      forecastItem->setText((accountColumn+it_c), amount, amountMM.isNegative()); //sets the text and negative color
      it_c++;
    }

    //calculate and add variation per cycle
    vAmountMM = forecast.accountTotalVariation(acc);
    vAmount = vAmountMM.formatMoney(acc, currency);
    forecastItem->setText((accountColumn+it_c), vAmount, vAmountMM.isNegative());

  }
  //add total row
  forecastItem = new KMyMoneyForecastListViewItem(m_forecastList, forecastItem, false);
  forecastItem->setText(accountColumn, i18n("Total"));
  int i;
  int prec = MyMoneyMoney::denomToPrec(file->baseCurrency().smallestAccountFraction());
  for(i = 0; i <= forecast.forecastDays(); ++i) {
    QDate forecastDate = QDate::currentDate().addDays(i);
    MyMoneyMoney amountMM = cycleBalance[forecastDate];
    QString amount = amountMM.formatMoney(file->baseCurrency().tradingSymbol(), prec);
    forecastItem->setText((i+1), amount, amountMM.isNegative());
  }
  //calculate total variation
  QString totalVarAmount;
  MyMoneyMoney totalVarAmountMM = cycleBalance[QDate::currentDate().addDays(i-1)]-cycleBalance[QDate::currentDate()];
  totalVarAmount = totalVarAmountMM.formatMoney(file->baseCurrency().tradingSymbol(), prec);
  forecastItem->setText((i+1), totalVarAmount);

  m_forecastList->show();
}

void KForecastView::loadSummaryView(void)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  QValueList<MyMoneyAccount> accList;
  int dropMinimum;
  int dropZero;
  bool negative = false;
  int daysToBeginDay;

  MyMoneySecurity baseCurrency = file->baseCurrency();

  MyMoneyForecast forecast;

  //Get all accounts of the right type to calculate forecast
  forecast.doForecast();
  accList = forecast.accountList();
  QValueList<MyMoneyAccount>::const_iterator accList_t = accList.begin();
  for(; accList_t != accList.end(); ++accList_t ) {
    MyMoneyAccount acc = *accList_t;
    if(nameIdx[acc.name()] != acc.id()) { //Check if the account is there
      nameIdx[acc.name()] = acc.id();
    }
  }

  //clear the list, including columns
  m_summaryList->clear();
  for(;m_summaryList->columns() > 0;) {
    m_summaryList->removeColumn(0);
  }

  //clear the list, including columns
  m_adviceList->clear();
  for(;m_adviceList->columns() > 0;) {
    m_adviceList->removeColumn(0);
  }

  //add first column of both lists
  int accountColumn = m_summaryList->addColumn(i18n("Account"), -1);
  m_adviceList->addColumn("", -1);

  //add cycle interval columns
  m_summaryList->addColumn(i18n("Current"), -1);

  //if beginning of forecast is today, set the begin day to next cycle to avoid repeating the first cycle
  if(QDate::currentDate() < forecast.beginForecastDate()) {
    daysToBeginDay = QDate::currentDate().daysTo(forecast.beginForecastDate());
  } else {
    daysToBeginDay = forecast.accountsCycle();
  }
  for(int i = 0; ((i*forecast.accountsCycle())+daysToBeginDay) <= forecast.forecastDays(); ++i) {
    int intervalDays = ((i*forecast.accountsCycle())+daysToBeginDay);
    QString columnName =  i18n("%1 days").arg(intervalDays, 0, 10);
    m_summaryList->addColumn(columnName, -1);
  }

  //add variation columns
  m_summaryList->addColumn(i18n("Total variation"), -1);

  //align columns
  for(int i = 1; i < m_summaryList->columns(); ++i) {
    m_summaryList->setColumnAlignment(i, Qt::AlignRight);
  }

  m_summaryList->setSorting(-1);
  m_adviceList->setSorting(-1);

  KMyMoneyForecastListViewItem *summaryItem = 0;
  KMyMoneyForecastListViewItem *adviceItem = 0;
  QMap<QDate, MyMoneyMoney> cycleBalance;

  QMap<QString, QCString>::ConstIterator it_nc;
  for(it_nc = nameIdx.begin(); it_nc != nameIdx.end(); ++it_nc) {

    const MyMoneyAccount& acc = file->account(*it_nc);
    MyMoneySecurity currency;
    QString amount;
    QString vAmount;
    MyMoneyMoney vAmountMM;
    ReportAccount repAcc;
    if(acc.currencyId() != file->baseCurrency().id())
      repAcc = ReportAccount(acc.id());

    //change currency to deep currency if account is an investment
    if(acc.isInvest()) {
      MyMoneySecurity underSecurity = file->security(acc.currencyId());
      currency = file->security(underSecurity.tradingCurrency());
    } else {
      currency = file->security(acc.currencyId());
    }


    summaryItem = new KMyMoneyForecastListViewItem(m_summaryList, summaryItem, false);
    summaryItem->setText(accountColumn, acc.name());
    int it_c = 1; // iterator for the columns of the listview

    //add current balance column
    QDate summaryDate = QDate::currentDate();

    MyMoneyMoney amountMM;
    amountMM = forecast.forecastBalance(acc, summaryDate);

    //calculate the balance in base currency for the total row
    if(acc.currencyId() != file->baseCurrency().id()) {
      MyMoneyMoney curPrice = repAcc.baseCurrencyPrice(summaryDate);
      MyMoneyMoney baseAmountMM = amountMM * curPrice;
      cycleBalance[summaryDate] += baseAmountMM;
    } else {
      cycleBalance[summaryDate] += amountMM;
    }
    amount = amountMM.formatMoney(acc, currency);
    summaryItem->setText((accountColumn+it_c), amount);
    it_c++;

    //iterate through all other columns
    for(int i = 0; ((i*forecast.accountsCycle())+daysToBeginDay) <= forecast.forecastDays(); ++i) {
      int intervalDays = ((i*forecast.accountsCycle())+daysToBeginDay);
      QDate summaryDate = QDate::currentDate().addDays(intervalDays);

      amountMM = forecast.forecastBalance(acc, summaryDate);

      //calculate the balance in base currency for the total row
      if(acc.currencyId() != file->baseCurrency().id()) {
        MyMoneyMoney curPrice = repAcc.baseCurrencyPrice(summaryDate);
        MyMoneyMoney baseAmountMM = amountMM * curPrice;
        cycleBalance[summaryDate] += baseAmountMM;

      } else {
        cycleBalance[summaryDate] += amountMM;
      }

      amount = amountMM.formatMoney(acc, currency);
      summaryItem->setText((accountColumn+it_c), amount);
      it_c++;
    }

    //calculate and add variation per cycle
    vAmountMM = forecast.accountTotalVariation(acc);
    summaryItem->setNegative(vAmountMM.isNegative());
    vAmount = vAmountMM.formatMoney(acc, currency);
    summaryItem->setText((accountColumn+it_c), vAmount);

  }
  //add total row
  summaryItem = new KMyMoneyForecastListViewItem(m_summaryList, summaryItem, false);
  summaryItem->setText(accountColumn, i18n("Total"));
  int i;

  //add current total
  int prec = MyMoneyMoney::denomToPrec(file->baseCurrency().smallestAccountFraction());
  summaryItem->setText(1, cycleBalance[QDate::currentDate()].formatMoney(file->baseCurrency().tradingSymbol(), prec));

  //add interval totals
  for(i = 0; ((i*forecast.accountsCycle())+daysToBeginDay) <= forecast.forecastDays(); ++i) {
    int intervalDays = ((i*forecast.accountsCycle())+daysToBeginDay);
    QDate summaryDate = QDate::currentDate().addDays(intervalDays);
    MyMoneyMoney amountMM = cycleBalance[summaryDate];
    QString amount = amountMM.formatMoney(file->baseCurrency().tradingSymbol(), prec);
    summaryItem->setText((i+2), amount);
  }
  //calculate total variation
  QString totalVarAmount;
  MyMoneyMoney totalVarAmountMM = cycleBalance[QDate::currentDate().addDays(forecast.forecastDays())]-cycleBalance[QDate::currentDate()];
  summaryItem->setNegative(totalVarAmountMM.isNegative());
  totalVarAmount = totalVarAmountMM.formatMoney(file->baseCurrency().tradingSymbol(), prec);
  summaryItem->setText((i+2), totalVarAmount);

  //Add comments to the advice list
  for(it_nc = nameIdx.begin(); it_nc != nameIdx.end(); ++it_nc) {

    const MyMoneyAccount& acc = file->account(*it_nc);
    MyMoneySecurity currency;

    //change currency to deep currency if account is an investment
    if(acc.isInvest()) {
      MyMoneySecurity underSecurity = file->security(acc.currencyId());
      currency = file->security(underSecurity.tradingCurrency());
    } else {
      currency = file->security(acc.currencyId());
    }

    //Check if the account is going to be below zero or below the minimal balance in the forecast period
    QString minimumBalance = acc.value("minimumBalance");
    MyMoneyMoney minBalance = MyMoneyMoney(minimumBalance);

    //Check if the account is going to be below minimal balance
    dropMinimum = forecast.daysToMinimumBalance(acc);

    //Check if the account is going to be below zero in the future
    dropZero = forecast.daysToZeroBalance(acc);


    // spit out possible warnings
    QString msg;

    // if a minimum balance has been specified, an appropriate warning will
    // only be shown, if the drop below 0 is on a different day or not present

    if(dropMinimum != -1
       && !minBalance.isZero()
       && (dropMinimum < dropZero
       || dropZero == -1)) {
      switch(dropMinimum) {
        case -1:
          break;
        case 0:
          msg = i18n("The balance of %1 is below the minimum balance %2 today.").arg(acc.name()).arg(minBalance.formatMoney(acc, currency));
          negative = true;
          break;
        default:
          msg = i18n("The balance of %1 will drop below the minimum balance %2 in %3 days.").arg(acc.name()).arg(minBalance.formatMoney(acc, currency)).arg(dropMinimum-1);
          negative = true;
      }

      if(!msg.isEmpty()) {
        adviceItem = new KMyMoneyForecastListViewItem(m_adviceList, adviceItem, negative);
        adviceItem->setText(0, msg);
      }
    }

    // a drop below zero is always shown
    msg = QString();
    switch(dropZero) {
      case -1:
        break;
      case 0:
        if(acc.accountGroup() == MyMoneyAccount::Asset) {
          msg = i18n("The balance of %1 is below %2 today.").arg(acc.name()).arg(MyMoneyMoney().formatMoney(acc, currency));
          negative = true;
          break;
        }
        if(acc.accountGroup() == MyMoneyAccount::Liability) {
          msg = i18n("The balance of %1 is above %2 today.").arg(acc.name()).arg(MyMoneyMoney().formatMoney(acc, currency));
          negative = false;
          break;
        }
        break;
      default:
        if(acc.accountGroup() == MyMoneyAccount::Asset) {
          msg = i18n("The balance of %1 will drop below %2 in %3 days.").arg(acc.name()).arg(MyMoneyMoney().formatMoney(acc, currency)).arg(dropZero);
          negative = true;
          break;
        }
        if(acc.accountGroup() == MyMoneyAccount::Liability) {
          msg = i18n("The balance of %1 will raise above %2 in %3 days.").arg(acc.name()).arg(MyMoneyMoney().formatMoney(acc, currency)).arg(dropZero);
          negative = false;
          break;
        }
    }
    if(!msg.isEmpty()) {
      adviceItem = new KMyMoneyForecastListViewItem(m_adviceList, adviceItem, negative);
      adviceItem->setText(0, msg);
    }

    //advice about trends
    msg = QString();
    MyMoneyMoney accCycleVariation = forecast.accountCycleVariation(acc);
    if (accCycleVariation < MyMoneyMoney(0, 1)) {
      msg = i18n("The account %1 is decreasing %2 per cycle.").arg(acc.name()).arg(accCycleVariation.formatMoney(acc, currency));
      negative = true;
    }

    if(!msg.isEmpty()) {
      adviceItem = new KMyMoneyForecastListViewItem(m_adviceList, adviceItem, negative);
      adviceItem->setText(0, msg);
    }
  }
  m_summaryList->show();
  m_adviceList->show();
}

void KForecastView::loadAdvancedView(void)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  QValueList<MyMoneyAccount> accList;
  MyMoneySecurity baseCurrency = file->baseCurrency();
  MyMoneyForecast forecast;
  int daysToBeginDay;

  //Get all accounts of the right type to calculate forecast
  forecast.doForecast();
  accList = forecast.accountList();
  QValueList<MyMoneyAccount>::const_iterator accList_t = accList.begin();
  for(; accList_t != accList.end(); ++accList_t ) {
    MyMoneyAccount acc = *accList_t;
    if(nameIdx[acc.name()] != acc.id()) { //Check if the account is there
      nameIdx[acc.name()] = acc.id();
    }
  }
  //clear the list, including columns
  m_advancedList->clear();
  for(;m_advancedList->columns() > 0;) {
    m_advancedList->removeColumn(0);
  }

  //add first column of both lists
  int accountColumn = m_advancedList->addColumn(i18n("Account"), -1);

  //if beginning of forecast is today, set the begin day to next cycle to avoid repeating the first cycle
  if(QDate::currentDate() < forecast.beginForecastDate()) {
    daysToBeginDay = QDate::currentDate().daysTo(forecast.beginForecastDate());
  } else {
    daysToBeginDay = forecast.accountsCycle();
  }

  //add columns
  for(int i = 1; ((i * forecast.accountsCycle()) + daysToBeginDay) <= forecast.forecastDays(); ++i) {
    int col = m_advancedList->addColumn(i18n("Min Bal %1").arg(i), -1);
    m_advancedList->setColumnAlignment(col, Qt::AlignRight);
    m_advancedList->addColumn(i18n("Min Date %1").arg(i), -1);
  }
  for(int i = 1; ((i * forecast.accountsCycle()) + daysToBeginDay) <= forecast.forecastDays(); ++i) {
    int col = m_advancedList->addColumn(i18n("Max Bal %1").arg(i), -1);
    m_advancedList->setColumnAlignment(col, Qt::AlignRight);
    m_advancedList->addColumn(i18n("Max Date %1").arg(i), -1);
  }
  int col = m_advancedList->addColumn(i18n("Average"), -1);
  m_advancedList->setColumnAlignment(col, Qt::AlignRight);
  m_advancedList->setSorting(-1);
  KMyMoneyForecastListViewItem *advancedItem = 0;

  QMap<QString, QCString>::ConstIterator it_nc;
  for(it_nc = nameIdx.begin(); it_nc != nameIdx.end(); ++it_nc) {
    const MyMoneyAccount& acc = file->account(*it_nc);
    QString amount;
    MyMoneyMoney amountMM;
    MyMoneySecurity currency;

    //change currency to deep currency if account is an investment
    if(acc.isInvest()) {
      MyMoneySecurity underSecurity = file->security(acc.currencyId());
      currency = file->security(underSecurity.tradingCurrency());
    } else {
      currency = file->security(acc.currencyId());
    }


    advancedItem = new KMyMoneyForecastListViewItem(m_advancedList, advancedItem, false);
    advancedItem->setText(accountColumn, acc.name());
    int it_c = 1; // iterator for the columns of the listview

    //get minimum balance list
    QValueList<QDate> minBalanceList = forecast.accountMinimumBalanceDateList(acc);
    QValueList<QDate>::Iterator t_min;
    for(t_min = minBalanceList.begin(); t_min != minBalanceList.end() ; ++t_min)
    {
      QDate minDate = *t_min;
      amountMM = forecast.forecastBalance(acc, minDate);

      amount = amountMM.formatMoney(acc, currency);
      advancedItem->setText(it_c, amount, amountMM.isNegative());
      it_c++;

      QString dateString = minDate.toString(Qt::LocalDate);
      advancedItem->setText(it_c, dateString, amountMM.isNegative());
      it_c++;
    }

    //get maximum balance list
    QValueList<QDate> maxBalanceList = forecast.accountMaximumBalanceDateList(acc);
    QValueList<QDate>::Iterator t_max;
    for(t_max = maxBalanceList.begin(); t_max != maxBalanceList.end() ; ++t_max)
    {
      QDate maxDate = *t_max;
      amountMM = forecast.forecastBalance(acc, maxDate);

      amount = amountMM.formatMoney(acc, currency);
      advancedItem->setText(it_c, amount, amountMM.isNegative());
      it_c++;

      QString dateString = maxDate.toString(Qt::LocalDate);
      advancedItem->setText(it_c, dateString, amountMM.isNegative());
      it_c++;
    }
    //get average balance
    amountMM = forecast.accountAverageBalance(acc);
    amount = amountMM.formatMoney(acc, currency);
    advancedItem->setText(it_c, amount, amountMM.isNegative());
    it_c++;
  }
  m_advancedList->show();
}


#include "kforecastview.moc"
