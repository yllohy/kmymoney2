/***************************************************************************
                          kenterscheduledialog.cpp  -  description
                             -------------------
    begin                : Mon Sep 1 2003
    copyright            : (C) 2000-2003 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
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

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>
#include <kmessagebox.h>
#include <kpushbutton.h>
#include <klistbox.h>
#include <klistview.h>
#include <kdebug.h>
#include <kio/netaccess.h>
#include <kprogress.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kequitypriceupdatedlg.h"
#include "../mymoney/mymoneyfile.h"
#include "../mymoney/mymoneyequity.h"
#include "../mymoney/mymoneyonlinepriceupdate.h"

KEquityPriceUpdateDlg::KEquityPriceUpdateDlg(QWidget *parent) :
  KEquityPriceUpdateDlgDecl(parent),
  m_pPriceUpdate(0)
{
  lvEquityList->setRootIsDecorated(false);
  lvEquityList->setColumnText(0, QString(i18n("Symbol")));
  lvEquityList->addColumn(i18n("Symbol"));
  lvEquityList->addColumn(i18n("Name"),125);
  lvEquityList->addColumn(i18n("Price"));
  lvEquityList->addColumn(i18n("Date"));
  
  // This is a "get it up and running" hack.  Will replace this in the future.
  lvEquityList->addColumn(i18n("ID"));

  lvEquityList->setMultiSelection(false);
  lvEquityList->setColumnWidthMode(0, QListView::Maximum);
  //lvEquityList->header()->setResizeEnabled(true);
  lvEquityList->setAllColumnsShowFocus(true);

  MyMoneyFile* file = MyMoneyFile::instance();
  QValueList<MyMoneyEquity> equities = file->equityList();
  qDebug("KEquityPriceUpdateDlg: Number of equity objects: %d", equities.size());

  for(QValueList<MyMoneyEquity>::ConstIterator it = equities.begin(); it != equities.end(); ++it)
  {
    qDebug("KEquityPriceUpdateDlg: Adding equity %s, symbol = %s", (*it).name().data(), (*it).tradingSymbol().data());
    KListViewItem* item = new KListViewItem(lvEquityList, (*it).tradingSymbol(), (*it).name());
    
    QMap<QDate,MyMoneyMoney> history = (*it).priceHistory();
    QMap<QDate,MyMoneyMoney>::const_iterator it_price = history.end();
    if ( it_price != history.begin() )
    {
      --it_price;
      item->setText(2,it_price.data().formatMoney());
      item->setText(3,it_price.key().toString(Qt::ISODate));
    }
    item->setText(4,(*it).id());
    lvEquityList->insertItem(item);
  }

  connect(btnOK, SIGNAL(clicked()), this, SLOT(slotOKClicked()));
  connect(btnCancel, SIGNAL(clicked()), this, SLOT(slotCancelClicked()));
  connect(btnUpdateSelected, SIGNAL(clicked()), this, SLOT(slotUpdateSelectedClicked()));
  connect(btnUpdateAll, SIGNAL(clicked()), this, SLOT(slotUpdateAllClicked()));
  
  // Not implemented yet.
  btnConfigure->hide();
  //connect(btnConfigure, SIGNAL(clicked()), this, SLOT(slotConfigureClicked()));
}

KEquityPriceUpdateDlg::~KEquityPriceUpdateDlg()
{

}

/*!
    \fn KEquityPriceUpdateDlg::logStatusMessage(const QString& message)
    Logs a message in the status window at the bottom of the dialog.
 */
void  KEquityPriceUpdateDlg::logStatusMessage(const QString& message)
{
  //inserts the message at the bottom of the listbox.
  lbStatus->insertItem(message);
}

void KEquityPriceUpdateDlg::logBeginingStatus()
{
  logStatusMessage(QString("Beginning Online Stock Update"));
}

void KEquityPriceUpdateDlg::logSummaryStatus()
{

}

void KEquityPriceUpdateDlg::slotOKClicked()
{
  // update the new prices into the equities
  
  MyMoneyFile* file = MyMoneyFile::instance();
  QValueList<MyMoneyEquity> equities = file->equityList();

  QListViewItem* item = lvEquityList->firstChild();
  while ( item )
  {
    MyMoneyMoney price(item->text(2).toDouble());
    if ( !price.isZero() )
    {
      QCString id = item->text(4).utf8();
      MyMoneyEquity equity = MyMoneyFile::instance()->equity(id);
      QMap<QDate,MyMoneyMoney> history = equity.priceHistory();
      QDate date = QDate().fromString(item->text(3),Qt::ISODate);
      if ( ! history.contains( date ) )
      {
        // TODO: Better handling of the case where there is already a price
        // for this date.  Currently, it just skips the update.  Really it
        // should check to see if the price is the same and prompt the user.
        equity.editPriceHistory(date,price);
        file->modifyEquity(equity);
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

bool KEquityPriceUpdateDlg::fetchUpdate(const QString& _symbol,QPair<QDate,MyMoneyMoney>& _result)
{
  bool gotprice = false;
  bool gotdate = false;

  // TODO: Make these user-configurable  
  QString source("http://finance.yahoo.com/d/quotes.csv?s=%1&f=sl1d1");
  QRegExp symbolRegExp("\"([^,\"]*)\",.*");
  QRegExp dateRegExp("[^,]*,[^,]*,\"([^\"]*)\"");
  QRegExp priceRegExp("[^,]*,([^,]*),.*");
  
  QString url = source.arg(_symbol);
  logStatusMessage(QString("Searching URL <%1>...").arg(url));
  
  QString tmpFile;
  if( KIO::NetAccess::download( KURL(url), tmpFile, NULL ) )
  {
    logStatusMessage(QString("Downloaded %1").arg(tmpFile));
    QFile f(tmpFile);
    if ( f.open( IO_ReadOnly ) )
    {
      QString data = QTextStream(&f).read();
      
      if( symbolRegExp.search(data) > -1)
        logStatusMessage(QString("Symbol found: %1").arg(symbolRegExp.cap(1)));
  
      if(priceRegExp.search(data)> -1)
      {
        gotprice = true;
        _result.second = MyMoneyMoney(priceRegExp.cap(1).toDouble()).toString();
        logStatusMessage(QString("Price found: %1").arg(_result.second.toString()));
      }
      
      if(dateRegExp.search(data) > -1)
      {
        QString datestr = dateRegExp.cap(1);
        // TODO: Fix this temporary hack.  We know yahoo returns mm/dd/yyyy
        QRegExp dateparse("([0-9]+)/([0-9]+)/([0-9]+)");
        if ( dateparse.search( datestr ) > -1 )
        {
          gotdate = true;
          _result.first = QDate( dateparse.cap(3).toInt(), dateparse.cap(1).toInt(), dateparse.cap(2).toInt() );
          logStatusMessage(QString("Date found: %1").arg(_result.first.toString()));;
        }
      }
      f.close();  
    }
    KIO::NetAccess::removeTempFile( tmpFile );
  }
  return gotprice && gotdate;
}

void KEquityPriceUpdateDlg::slotUpdateSelectedClicked()
{
#if 0  
  if(!m_pPriceUpdate)
  {
    m_pPriceUpdate = new MyMoneyOnlinePriceUpdate();
  }

  //m_pPriceUpdate->
#else
  prgOnlineProgress->setTotalSteps(2);
  prgOnlineProgress->setProgress(1);

  QListViewItem* item = lvEquityList->selectedItem();
  if ( item )
  {
    QString symbol = item->text(0);
    QPair<QDate,MyMoneyMoney> update;
    if ( fetchUpdate(symbol,update) )
    {
      item->setText(2,update.second.formatMoney());
      item->setText(3,update.first.toString(Qt::ISODate));
    }
    else
    {
      item->setText(2,"Error");
      item->setText(3,"Unable to update");
    }
  }
  prgOnlineProgress->advance(1);
#endif
}

void KEquityPriceUpdateDlg::slotUpdateAllClicked()
{
  qDebug("KEquityPriceUpdateDlg: Updating All");
#if 0

  if(!m_pPriceUpdate)
  {
    m_pPriceUpdate = new MyMoneyOnlinePriceUpdate();
  }
  
  m_pPriceUpdate->getWebServiceQuote(QString("RHAT"));
  
  /*QStringList list;
  QPtrList<QListViewItem> selectedItems = lvEquityList->selectedItems();
  for(QPtrList<QListViewItem>::ConstIterator it = selectedItems.begin(); it != selectedItems.end(); ++it)
  {
    qDebug("KEquityPriceUpdateDlg: Updating %s", (*it)->text(0).data());
    QListViewItem* item = (*it);
    list.push_back(item->text(0));
  }
  
  int result = m_pPriceUpdate->getQuotes(list);


  int result = m_pPriceUpdate->getQuotes(list);*/
#else
  prgOnlineProgress->setTotalSteps(1+lvEquityList->childCount());
  prgOnlineProgress->setProgress(1);
  QListViewItem* item = lvEquityList->firstChild();
  while ( item )
  {
    QString symbol = item->text(0);
    QPair<QDate,MyMoneyMoney> update;
    if ( fetchUpdate(symbol,update) )
    {
      item->setText(2,update.second.formatMoney());
      item->setText(3,update.first.toString(Qt::ISODate));
    }
    else
    {
      item->setText(2,"Error");
      item->setText(3,"Unable to update");
    }
      
    prgOnlineProgress->advance(1);
    
    item = item->nextSibling();
  }
  prgOnlineProgress->setProgress(prgOnlineProgress->totalSteps());
#endif
}

void KEquityPriceUpdateDlg::slotConfigureClicked()
{

}
