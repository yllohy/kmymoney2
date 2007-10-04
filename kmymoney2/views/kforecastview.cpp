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

// ----------------------------------------------------------------------------
// Project IncludesAccounts

#include <kmymoney/mymoneyfile.h>
#include "kforecastview.h"
#include "../kmymoneyglobalsettings.h"
#include "../kmymoney2.h"
#include "../kmymoneyutils.h"
#include "../mymoney/mymoneyforecast.h"


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

  //loadListView();

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
  m_forecastList->setResizeMode(QListView::LastColumn);
}

void KForecastView::slotLoadForecast(void)
{
  m_needReload[ListView] = true;
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

  int dateColumn = m_forecastList->addColumn("Date");

  QMap<QString, QCString>::ConstIterator it_n;
  for(it_n = nameIdx.begin(); it_n != nameIdx.end(); ++it_n) {
    MyMoneyAccount acc = file->account(*it_n);
    m_forecastList->addColumn(acc.name());
  }

  m_forecastList->addColumn(i18n("Total Forecast"));

  m_forecastList->setSorting(-1);

  QListViewItem *forecastItem = 0;

  for(int it_f=0;it_f <= forecast.forecastDays(); ++it_f){
    QDate forecastDate = QDate::currentDate().addDays(it_f);
    QString dateString = forecastDate.toString(Qt::LocalDate);
    forecastItem = new QListViewItem(m_forecastList, forecastItem);
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
  m_forecastList->show();
}


#include "kforecastview.moc"
