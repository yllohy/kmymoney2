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

  MyMoneyFile* file = MyMoneyFile::instance();
  QValueList<MyMoneySecurity> securities = file->securityList();

  btnUpdateAll->setEnabled(false);

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
      MyMoneySecurity security = MyMoneyFile::instance()->security(id);
      try {
        MyMoneyPrice price(id, security.tradingCurrency(), QDate().fromString(item->text(DATE_COL), Qt::ISODate), rate, security.value("kmm-online-source"));

        // TODO: Better handling of the case where there is already a price
        // for this date.  Currently, it just overrides the old value.  Really it
        // should check to see if the price is the same and prompt the user.
        MyMoneyFile::instance()->addPrice(price);

      } catch(MyMoneyException *e) {
        qDebug("Unable to add price information for %s", security.name().data());
        delete e;
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

void KEquityPriceUpdateDlg::slotReceivedQuote(const QString& symbol,const QDate& date, const MyMoneyMoney& price)
{
  QListViewItem* item = lvEquityList->findItem(symbol,SYMBOL_COL,Qt::ExactMatch);
  QListViewItem* next = NULL;
  
  if ( item )
  {
    if ( price.isPositive() && date.isValid() )
    {
      item->setText(PRICE_COL, price.formatMoney());
      item->setText(DATE_COL, date.toString(Qt::ISODate));
      logStatusMessage(i18n("Price for %1 updated").arg(symbol));
    }
    else
    {
      logErrorMessage(i18n("Received an invalid price for %1, unable to update.").arg(symbol));
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
    logErrorMessage(i18n("Received a price for %1, but this symbol is not on the list!  Aborting entire update.").arg(symbol));
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
