/***************************************************************************
                          kequitypriceupdatedlg.cpp  -  description
                             -------------------
    begin                : Mon Sep 1 2003
    copyright            : (C) 2000-2003 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
                           Ace Jones <acejones@users.sourceforge.net>
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

#include <qlabel.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qtimer.h>
#include <qlayout.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>
#include <kmessagebox.h>
#include <kpushbutton.h>
#include <ktextedit.h>
#include <klistview.h>
#include <kdebug.h>
#include <kprogress.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kequitypriceupdatedlg.h"
#include "../mymoney/mymoneyfile.h"
#include "../mymoney/mymoneysecurity.h"
#include "../mymoney/mymoneyprice.h"

#define SYMBOL_COL      0
#define NAME_COL        1
#define PRICE_COL       2
#define DATE_COL        3
#define ID_COL          4
#define SOURCE_COL      5

KEquityPriceUpdateDlg::KEquityPriceUpdateDlg(QWidget *parent, const QCString& securityId) :
  KEquityPriceUpdateDlgDecl(parent),
  m_fUpdateAll(false)
{
  lvEquityList->setRootIsDecorated(false);
  lvEquityList->setColumnText(0, QString(i18n("Symbol")));
  lvEquityList->addColumn(i18n("Symbol"));
  lvEquityList->addColumn(i18n("Name"),125);
  lvEquityList->addColumn(i18n("Price"));
  lvEquityList->addColumn(i18n("Date"));

  // This is a "get it up and running" hack.  Will replace this in the future.
  lvEquityList->addColumn("ID");
  lvEquityList->addColumn("Source");
  lvEquityList->setColumnWidth(ID_COL, 0);

  lvEquityList->setMultiSelection(true);
  lvEquityList->setColumnWidthMode(SYMBOL_COL, QListView::Maximum);
  lvEquityList->setColumnWidthMode(ID_COL, QListView::Manual);
  lvEquityList->setAllColumnsShowFocus(true);

  btnUpdateAll->setEnabled(false);
  
  MyMoneyFile* file = MyMoneyFile::instance();
  
  // First, add each security
  QValueList<MyMoneySecurity> securities = file->securityList();
  for(QValueList<MyMoneySecurity>::ConstIterator it = securities.begin(); it != securities.end(); ++it)
  {
    // const QString& symbol = (*it).tradingSymbol();
    if ( securityId.isEmpty() || ( (*it).id() == securityId ) )
    {
      // only add those securities that have a valid source set
      if(!(*it).value("kmm-online-source").isEmpty()) {
        KListViewItem* item = new KListViewItem(lvEquityList, (*it).tradingSymbol(), (*it).name());
        MyMoneySecurity currency = MyMoneyFile::instance()->currency((*it).tradingCurrency());
        MyMoneyPrice pr = MyMoneyFile::instance()->price((*it).id(), (*it).tradingCurrency());
        if(pr.isValid()) {
          item->setText(PRICE_COL, pr.rate().formatMoney(currency.tradingSymbol()));
          item->setText(DATE_COL, pr.date().toString(Qt::ISODate));
        }
        item->setText(ID_COL,(*it).id());
        item->setText(SOURCE_COL, (*it).value("kmm-online-source"));
        btnUpdateAll->setEnabled(true);
      }
    }
  }

  // Second, add each price pair that we know about
  // typedef QMap<MyMoneySecurityPair, MyMoneyPriceEntries> MyMoneyPriceList;
  MyMoneyPriceList prices = file->priceList();
  for(MyMoneyPriceList::ConstIterator it_price = prices.begin(); it_price != prices.end(); ++it_price)
  {
    // typedef QPair<QCString, QCString> MyMoneySecurityPair;
    MyMoneySecurityPair pair = it_price.key();
    
    if ( file->security( pair.first ).isCurrency() )
    {
      KListViewItem* item = new KListViewItem(lvEquityList, 
        QString("%1 > %2").arg(pair.first,pair.second),
        i18n("%1 units in %2").arg(pair.first,pair.second));
      MyMoneyPrice pr = file->price(pair.first,pair.second);
      if(pr.isValid()) {
        item->setText(PRICE_COL, pr.rate().formatMoney(file->currency(pair.second).tradingSymbol()));
        item->setText(DATE_COL, pr.date().toString(Qt::ISODate));
      }
      item->setText(ID_COL,QString("%1 %2").arg(pair.first,pair.second));
      item->setText(SOURCE_COL,i18n("Yahoo Currency"));
      
      btnUpdateAll->setEnabled(true);
    }
  }
  
  connect(btnOK, SIGNAL(clicked()), this, SLOT(slotOKClicked()));
  connect(btnCancel, SIGNAL(clicked()), this, SLOT(slotCancelClicked()));
  connect(btnUpdateSelected, SIGNAL(clicked()), this, SLOT(slotUpdateSelectedClicked()));
  connect(btnUpdateAll, SIGNAL(clicked()), this, SLOT(slotUpdateAllClicked()));

  connect(&m_webQuote,SIGNAL(quote(const QString&,const QDate&, const MyMoneyMoney&)),
    this,SLOT(slotReceivedQuote(const QString&,const QDate&, const MyMoneyMoney&)));
  connect(&m_webQuote,SIGNAL(status(const QString&)),
    this,SLOT(logStatusMessage(const QString&)));
  connect(&m_webQuote,SIGNAL(error(const QString&)),
    this,SLOT(logErrorMessage(const QString&)));
  
  connect(lvEquityList, SIGNAL(selectionChanged()), this, SLOT(slotUpdateSelection()));

  // Not implemented yet.
  btnConfigure->hide();
  //connect(btnConfigure, SIGNAL(clicked()), this, SLOT(slotConfigureClicked()));

  if ( !securityId.isEmpty() )
  {
    btnUpdateSelected->hide();
    btnUpdateAll->hide();
    delete layout1;

    QTimer::singleShot(100,this,SLOT(slotUpdateAllClicked()));
  }

  slotUpdateSelection();
}

KEquityPriceUpdateDlg::~KEquityPriceUpdateDlg()
{

}

void KEquityPriceUpdateDlg::logErrorMessage(const QString& message)
{
  logStatusMessage(QString("<font color=\"red\"><b>") + message + QString("</b></font>"));
}

void KEquityPriceUpdateDlg::logStatusMessage(const QString& message)
{
  lbStatus->append(message);
}

void KEquityPriceUpdateDlg::slotOKClicked()
{
  // update the new prices into the equities

  MyMoneyFile* file = MyMoneyFile::instance();
  QValueList<MyMoneySecurity> equities = file->securityList();

  QListViewItem* item = lvEquityList->firstChild();
  while ( item )
  {
    MyMoneyMoney rate(item->text(PRICE_COL));
    if ( !rate.isZero() )
    {
      QCString id = item->text(ID_COL).utf8();
      
      // if the ID has a space, then this is TWO ID's, so it's a currency quote
      if ( QString(id).contains(" ") )
      {
        QStringList ids = QStringList::split(" ",QString(id));
        QCString fromid = ids[0].utf8();
        QCString toid = ids[1].utf8();        
        MyMoneyPrice price(fromid,toid,QDate().fromString(item->text(DATE_COL), Qt::ISODate),rate,item->text(SOURCE_COL));
        file->addPrice(price);
      }
      else
      // otherwise, it's a security quote
      {
        MyMoneySecurity security = MyMoneyFile::instance()->security(id);
        try {
          MyMoneyPrice price(id, security.tradingCurrency(), QDate().fromString(item->text(DATE_COL), Qt::ISODate), rate, item->text(SOURCE_COL));
  
          // TODO: Better handling of the case where there is already a price
          // for this date.  Currently, it just overrides the old value.  Really it
          // should check to see if the price is the same and prompt the user.
          MyMoneyFile::instance()->addPrice(price);
  
        } catch(MyMoneyException *e) {
          qDebug("Unable to add price information for %s", security.name().data());
          delete e;
        }
      }
    }
    item = item->nextSibling();
  }

  accept();
}

void KEquityPriceUpdateDlg::slotCancelClicked()
{
  reject();
}

void KEquityPriceUpdateDlg::slotUpdateSelection(void)
{
  btnUpdateSelected->setEnabled(false);

  QListViewItem* item = lvEquityList->firstChild();
  while ( item && !item->isSelected())
    item = item->nextSibling();

  if(item)
    btnUpdateSelected->setEnabled(true);
}

void KEquityPriceUpdateDlg::slotUpdateSelectedClicked()
{
  QListViewItem* item = lvEquityList->firstChild();
  int skipCnt = 1;
  while ( item && !item->isSelected())
  {
    skipCnt++;
    item = item->nextSibling();
  }

  if(item) {
    prgOnlineProgress->setTotalSteps(1+lvEquityList->childCount());
    prgOnlineProgress->setProgress(skipCnt);
    m_webQuote.launch(item->text(SYMBOL_COL),item->text(SOURCE_COL));
  }
  else
    logErrorMessage("No security selected.");
}

void KEquityPriceUpdateDlg::slotUpdateAllClicked()
{
  QListViewItem* item = lvEquityList->firstChild();
  if ( item )
  {
    prgOnlineProgress->setTotalSteps(1+lvEquityList->childCount());
    prgOnlineProgress->setProgress(1);
    m_fUpdateAll = true;
    m_webQuote.launch(item->text(SYMBOL_COL),item->text(SOURCE_COL));
  }
  else
    logErrorMessage("Security list is empty.");
}

void KEquityPriceUpdateDlg::slotConfigureClicked()
{
}

void KEquityPriceUpdateDlg::slotReceivedQuote(const QString& _symbol,const QDate& _date, const MyMoneyMoney& _price)
{
  QListViewItem* item = lvEquityList->findItem(_symbol,SYMBOL_COL,Qt::ExactMatch);
  QListViewItem* next = NULL;
  
  if ( item )
  {
    if ( _price.isPositive() && _date.isValid() )
    {
      QDate date = _date;
      if ( date > QDate::currentDate() )
        date = QDate::currentDate();
      
      item->setText(PRICE_COL, _price.formatMoney());
      item->setText(DATE_COL, date.toString(Qt::ISODate));
      logStatusMessage(i18n("Price for %1 updated").arg(_symbol));
    }
    else
    {
      logErrorMessage(i18n("Received an invalid price for %1, unable to update.").arg(_symbol));
    }
    
    prgOnlineProgress->advance(1);
    item->listView()->setSelected(item, false);
  
    // launch the NEXT one ... in case of m_fUpdateAll == false, we
    // need to parse the list to find the next selected one
    next = item->nextSibling();
    if ( !m_fUpdateAll )
    {
      while(next && !next->isSelected()) 
      {
        prgOnlineProgress->advance(1);
        next = next->nextSibling();
      }
    }
  }
  else
  {
    logErrorMessage(i18n("Received a price for %1, but this symbol is not on the list!  Aborting entire update.").arg(_symbol));
  }

  if (next)
  {
    m_webQuote.launch(next->text(SYMBOL_COL),next->text(SOURCE_COL));
  }
  else 
  {
    // we've run past the end, reset to the default value.
    m_fUpdateAll = false;
    // force progress bar to show 100%
    prgOnlineProgress->setProgress(prgOnlineProgress->totalSteps());
  }
}
