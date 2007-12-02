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
  m_summaryList->setResizeMode(QListView::AllColumns);
  
}

void KForecastView::slotLoadForecast(void)
{
  m_needReload[ListView] = true;
  m_needReload[SummaryView] = true;
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
  accList = forecast.forecastAccountList();
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

  int dateColumn = m_forecastList->addColumn(i18n("Date"), -1);

  QMap<QString, QCString>::ConstIterator it_n;
  for(it_n = nameIdx.begin(); it_n != nameIdx.end(); ++it_n) {
    MyMoneyAccount acc = file->account(*it_n);
    m_forecastList->addColumn(acc.name(), -1);
  }

  m_forecastList->addColumn(i18n("Total Forecast"), -1);

  m_forecastList->setSorting(-1);

  KListViewItem *forecastItem = 0;

  for(int it_f=0;it_f <= forecast.forecastDays(); ++it_f){
    QDate forecastDate = QDate::currentDate().addDays(it_f);
    QString dateString = forecastDate.toString(Qt::LocalDate);
    forecastItem = new KListViewItem(m_forecastList, forecastItem);
    forecastItem->setText(dateColumn, dateString);
    MyMoneyMoney totalAmountMM = MyMoneyMoney::MyMoneyMoney(0,1);
    QString totalAmount;

    int it_c = 1; // iterator for the columns of the listview
    QMap<QString, QCString>::ConstIterator it_nc;
    for(it_nc = nameIdx.begin(); it_nc != nameIdx.end(); ++it_nc) {
      MyMoneyAccount acc = file->account(*it_nc);
      MyMoneySecurity currency = file->security(acc.currencyId());
      QString amount;

      //MyMoneyMoney amountMM = accountList[acc.id()][it_f];
      MyMoneyMoney amountMM;
      amountMM = forecast.forecastBalance(acc, QDate::currentDate().addDays(it_f));
      totalAmountMM += amountMM;
      amount = amountMM.formatMoney(currency.tradingSymbol());
      forecastItem->setText((dateColumn+it_c), amount);
      it_c++;
    }
    totalAmount = totalAmountMM.formatMoney(baseCurrency.tradingSymbol());
    forecastItem->setText((dateColumn+it_c), totalAmount);
  }

  //adjust the width all columns
  for(int col=1; col < m_forecastList->columns(); ++col) {
    m_forecastList->setColumnWidthMode(col, QListView::Maximum);
    m_forecastList->setColumnAlignment(col, Qt::AlignRight);
    m_forecastList->adjustColumn(col);
  }
  m_forecastList->show();
}

void KForecastView::loadSummaryView(void)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  QValueList<MyMoneyAccount> accList;
  int dropMinimum;
  int dropZero;
  bool negative = false;

  MyMoneySecurity baseCurrency = file->baseCurrency();

  MyMoneyForecast forecast;

  //Get all accounts of the right type to calculate forecast
  forecast.doForecast();
  accList = forecast.forecastAccountList();
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
  for(int i = 0; (i*forecast.accountsCycle()) <= forecast.forecastDays(); ++i) {
    QString columnName =  i18n("%1 days").arg(i*forecast.accountsCycle(), 0, 10);
    m_summaryList->addColumn(columnName, -1);
  }

  //add variation columns
  m_summaryList->addColumn(i18n("Total variation"), -1);

  //align columns
  for(int i = 1; i < m_summaryList->columns(); ++i) {
    m_summaryList->setColumnAlignment(i, Qt::AlignRight);
  }

  /*QMap<QString, QCString>::ConstIterator it_n;
  for(it_n = nameIdx.begin(); it_n != nameIdx.end(); ++it_n) {
    MyMoneyAccount acc = file->account(*it_n);
    m_forecastList->addColumn(acc.name());
  }*/

  m_summaryList->setSorting(-1);
  m_adviceList->setSorting(-1);

  KMyMoneyForecastListViewItem *summaryItem = 0;
  KMyMoneyForecastListViewItem *adviceItem = 0;
  QMap<QDate, MyMoneyMoney> cycleBalance;

  QMap<QString, QCString>::ConstIterator it_nc;
  for(it_nc = nameIdx.begin(); it_nc != nameIdx.end(); ++it_nc) {

    MyMoneyAccount acc = file->account(*it_nc);
    MyMoneySecurity currency = file->security(acc.currencyId());
    QString amount;
    QString vAmount;
    MyMoneyMoney vAmountMM;
    MyMoneyMoney tempVAmountMM;

    summaryItem = new KMyMoneyForecastListViewItem(m_summaryList, summaryItem, false);
    summaryItem->setText(accountColumn, acc.name());
    int it_c = 1; // iterator for the columns of the listview

    for(int i = 0; (i*forecast.accountsCycle()) <= forecast.forecastDays(); ++i) {
      QDate summaryDate = QDate::currentDate().addDays(i*forecast.accountsCycle());

      MyMoneyMoney amountMM;
      amountMM = forecast.forecastBalance(acc, summaryDate);

      //calculate the balance in base currency for the total row
      if(acc.currencyId() != file->baseCurrency().id()) {
        ReportAccount repAcc = ReportAccount(acc.id());
        MyMoneyMoney curPrice = repAcc.baseCurrencyPrice(summaryDate);
        MyMoneyMoney baseAmountMM = amountMM * curPrice;
        cycleBalance[summaryDate] += baseAmountMM;
        
      } else {
        cycleBalance[summaryDate] += amountMM;
      }

      //calculate variation
      if (i == 0) tempVAmountMM = amountMM;

      vAmountMM += (amountMM - tempVAmountMM);
      tempVAmountMM = amountMM;
      amount = amountMM.formatMoney(currency.tradingSymbol());
      summaryItem->setText((accountColumn+it_c), amount);
      it_c++;
    }

    //calculate and add variation per cycle
    summaryItem->setNegative(vAmountMM.isNegative()); 
    vAmount = vAmountMM.formatMoney(currency.tradingSymbol());
    summaryItem->setText((accountColumn+it_c), vAmount);

  }
  //add total row
  summaryItem = new KMyMoneyForecastListViewItem(m_summaryList, summaryItem, false);
  summaryItem->setText(accountColumn, i18n("Total"));
  int i;
  for(i = 0; (i*forecast.accountsCycle()) <= forecast.forecastDays(); ++i) {
    QDate summaryDate = QDate::currentDate().addDays(i*forecast.accountsCycle());
    MyMoneyMoney amountMM = cycleBalance[summaryDate];
    QString amount = amountMM.formatMoney(file->baseCurrency().tradingSymbol());
    summaryItem->setText((i+1), amount);
  }
  //calculate total variation
  QString totalVarAmount;
  MyMoneyMoney totalVarAmountMM = cycleBalance[QDate::currentDate().addDays((i-1)*forecast.accountsCycle())]-cycleBalance[QDate::currentDate()];
  summaryItem->setNegative(totalVarAmountMM.isNegative()); 
  totalVarAmount = totalVarAmountMM.formatMoney(file->baseCurrency().tradingSymbol());
  summaryItem->setText((i+1), totalVarAmount);

    //Add comments to the advice list
  for(it_nc = nameIdx.begin(); it_nc != nameIdx.end(); ++it_nc) {

    MyMoneyAccount acc = file->account(*it_nc);
    MyMoneySecurity currency = file->security(acc.currencyId());

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
          msg = i18n("The balance of %1 is below the minimum balance %2 today.").arg(acc.name()).arg(minBalance.formatMoney(currency.tradingSymbol()));
          negative = true;
          break;
        default:
          msg = i18n("The balance of %1 will drop below the minimum balance %2 in %3 days.").arg(acc.name()).arg(minBalance.formatMoney(currency.tradingSymbol())).arg(dropMinimum-1);
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
          msg = i18n("The balance of %1 is below %2 today.").arg(acc.name()).arg(MyMoneyMoney().formatMoney(currency.tradingSymbol()));
          negative = true;
          break;
        }
        if(acc.accountGroup() == MyMoneyAccount::Liability) {
          msg = i18n("The balance of %1 is above %2 today.").arg(acc.name()).arg(MyMoneyMoney().formatMoney(currency.tradingSymbol()));
          negative = false;
          break;
        }
        break;
      default:
        if(acc.accountGroup() == MyMoneyAccount::Asset) {
          msg = i18n("The balance of %1 will drop below %2 in %3 days.").arg(acc.name()).arg(MyMoneyMoney().formatMoney(currency.tradingSymbol())).arg(dropZero);
          negative = true;
          break;
        }
        if(acc.accountGroup() == MyMoneyAccount::Liability) {
          msg = i18n("The balance of %1 will raise above %2 in %3 days.").arg(acc.name()).arg(MyMoneyMoney().formatMoney(currency.tradingSymbol())).arg(dropZero);
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
      msg = i18n("The account %1 is decreasing %2 per cycle.").arg(acc.name()).arg(accCycleVariation.formatMoney(currency.tradingSymbol()));
      negative = true;
    }
    
    if(!msg.isEmpty()) {
      adviceItem = new KMyMoneyForecastListViewItem(m_adviceList, adviceItem, negative);
      adviceItem->setText(0, msg);
    }
  }
  m_forecastList->show();
}



#include "kforecastview.moc"
