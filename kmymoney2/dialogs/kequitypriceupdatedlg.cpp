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
#include <qtimer.h>
#include <qlayout.h>

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
#include <kconfig.h>
#include <kurl.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kequitypriceupdatedlg.h"
#include "../mymoney/mymoneyfile.h"
#include "../mymoney/mymoneyequity.h"
#include "../mymoney/mymoneyonlinepriceupdate.h"

KEquityPriceUpdateDlg::KEquityPriceUpdateDlg(QWidget *parent, const QString& onlysymbol) :
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
  lvEquityList->addColumn(i18n("ID"));

  lvEquityList->setMultiSelection(false);
  lvEquityList->setColumnWidthMode(0, QListView::Maximum);
  lvEquityList->setAllColumnsShowFocus(true);

  MyMoneyFile* file = MyMoneyFile::instance();
  QValueList<MyMoneyEquity> equities = file->equityList();
  qDebug("KEquityPriceUpdateDlg: Number of equity objects: %d", equities.size());

  for(QValueList<MyMoneyEquity>::ConstIterator it = equities.begin(); it != equities.end(); ++it)
  {
    const QString& symbol = (*it).tradingSymbol().data();
    if ( onlysymbol.isEmpty() || ( symbol == onlysymbol ) )
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
  }

  connect(btnOK, SIGNAL(clicked()), this, SLOT(slotOKClicked()));
  connect(btnCancel, SIGNAL(clicked()), this, SLOT(slotCancelClicked()));
  connect(btnUpdateSelected, SIGNAL(clicked()), this, SLOT(slotUpdateSelectedClicked()));
  connect(btnUpdateAll, SIGNAL(clicked()), this, SLOT(slotUpdateAllClicked()));

  connect(&m_filter, SIGNAL(processExited(QListViewItem*, const QString&)), this, SLOT(slotInsertUpdate(QListViewItem*, const QString&)));  
  connect(&m_filter, SIGNAL(receivedStderr(KProcess*, char*, int)), this, SLOT(slotReceivedErrorFromFilter(KProcess*, char*, int)));  
    
  // Not implemented yet.
  btnConfigure->hide();
  //connect(btnConfigure, SIGNAL(clicked()), this, SLOT(slotConfigureClicked()));
  
  if ( !onlysymbol.isEmpty() )
  {
    btnUpdateSelected->hide();
    btnUpdateAll->hide();
    delete layout1;
    
    QTimer::singleShot(100,this,SLOT(slotUpdateAllClicked()));
  }
}

KEquityPriceUpdateDlg::~KEquityPriceUpdateDlg()
{

}

void KEquityPriceUpdateDlg::logStatusMessage(const QString& message)
{
  lbStatus->insertItem(message);
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

void KEquityPriceUpdateDlg::slotUpdateSelectedClicked()
{
  qDebug("KEquityPriceUpdateDlg: Updating Selected");
  QListViewItem* item = lvEquityList->selectedItem();
  if ( item )
  {
    prgOnlineProgress->setTotalSteps(2);
    prgOnlineProgress->setProgress(1);
    launchUpdate(item);
  }
  else
    logStatusMessage("No equity selected.");
}

void KEquityPriceUpdateDlg::slotUpdateAllClicked()
{
  qDebug("KEquityPriceUpdateDlg: Updating All");
  QListViewItem* item = lvEquityList->firstChild();
  if ( item )
  {
    prgOnlineProgress->setTotalSteps(1+lvEquityList->childCount());
    prgOnlineProgress->setProgress(1);
    m_fUpdateAll = true;
    launchUpdate( item );
  }
  else
    logStatusMessage("Equity list is empty.");
}

void KEquityPriceUpdateDlg::slotConfigureClicked()
{
}

void KEquityPriceUpdateDlg::launchUpdate(QListViewItem* _item )
{
  KConfig *kconfig = KGlobal::config();
  kconfig->setGroup("Online Quotes Options");
  QString source(kconfig->readEntry("URL","http://finance.yahoo.com/d/quotes.csv?s=%1&f=sl1d1"));
  KURL url = KURL::fromPathOrURL(source.arg(_item->text(0)));
  
  if ( url.isLocalFile() )
  {
    logStatusMessage(QString("Executing <%1>...").arg(url.path()));
    
    m_filter.clearArguments();
    m_filter << QStringList::split(" ",url.path());
    m_filter.setItem(_item);
    
    if(m_filter.start(KProcess::NotifyOnExit, KProcess::All)) 
      m_filter.resume();
    else
    {
      logStatusMessage(QString("Unable to launch: %1").arg(url.path()));
      slotInsertUpdate(_item,QString());
    }
  }
  else
  {
    logStatusMessage(QString("Fetching URL <%1>...").arg(url.prettyURL()));
    
    QString tmpFile;
    if( KIO::NetAccess::download( url, tmpFile, NULL ) )
    {
      qDebug(QString("Downloaded %1").arg(tmpFile));
      QFile f(tmpFile);
      if ( f.open( IO_ReadOnly ) )
      {
        QString quote = QTextStream(&f).read();
        f.close();
        slotInsertUpdate(_item,quote);
      }
      else
      {
        slotInsertUpdate(_item,QString());
      }
      KIO::NetAccess::removeTempFile( tmpFile );
    }
  }
}

void KEquityPriceUpdateDlg::slotInsertUpdate(QListViewItem* _item, const QString& _quotedata )
{
  bool gotprice = false;
  bool gotdate = false;
  MyMoneyMoney price;
  QDate date;
  
  if ( ! _quotedata.isEmpty() )
  {  
    KConfig *kconfig = KGlobal::config();
    kconfig->setGroup("Online Quotes Options");
    QRegExp symbolRegExp(kconfig->readEntry("SymbolRegex","\"([^,\"]*)\",.*"));
    QRegExp dateRegExp(kconfig->readEntry("PriceRegex","[^,]*,[^,]*,\"([^\"]*)\""));
    QRegExp priceRegExp(kconfig->readEntry("DateRegex","[^,]*,([^,]*),.*"));

    if( symbolRegExp.search(_quotedata) > -1)
      logStatusMessage(QString("Symbol found: %1").arg(symbolRegExp.cap(1)));

    if(priceRegExp.search(_quotedata)> -1)
    {
      gotprice = true;
      price = MyMoneyMoney(priceRegExp.cap(1).toDouble()).toString();
      logStatusMessage(QString("Price found: %1").arg(price.toString()));
    }
    
    if(dateRegExp.search(_quotedata) > -1)
    {
      QString datestr = dateRegExp.cap(1);
      // TODO: Fix this temporary hack.  We know yahoo returns mm/dd/yyyy
      QRegExp dateparse("([0-9]+)/([0-9]+)/([0-9]+)");
      if ( dateparse.search( datestr ) > -1 )
      {
        gotdate = true;
        date = QDate( dateparse.cap(3).toInt(), dateparse.cap(1).toInt(), dateparse.cap(2).toInt() );
        logStatusMessage(QString("Date found: %1").arg(date.toString()));;
      }
    }  
  }
  
  if ( gotprice && gotdate )
  {
    _item->setText(2,price.formatMoney());
    _item->setText(3,date.toString(Qt::ISODate));
  }
  else
  {
    _item->setText(2,"Error");
    _item->setText(3,"Unable to update");
  }     
  
  prgOnlineProgress->advance(1);
  
  // launch the NEXT one ... ONLY if this is an "update all" situation
  if ( m_fUpdateAll )
  {
    QListViewItem* next = _item->nextSibling();
    if (next)
      launchUpdate(next);  
    else
      // we've run past the end, reset to the default value.
      m_fUpdateAll = false;
  }
}

void KEquityPriceUpdateDlg::slotReceivedErrorFromFilter(KProcess*, char* _pcbuffer, int _nbufferlen)
{
  QByteArray data;
  data.duplicate(_pcbuffer, _nbufferlen);
  logStatusMessage(QString(data));
}

KEquityPriceUpdateProcess::KEquityPriceUpdateProcess(void): m_item(NULL)
{
  connect(this, SIGNAL(receivedStdout(KProcess*, char*, int)), this, SLOT(slotReceivedDataFromFilter(KProcess*, char*, int)));
  connect(this, SIGNAL(processExited(KProcess*)), this, SLOT(slotProcessExited(KProcess*)));
}

void KEquityPriceUpdateProcess::slotReceivedDataFromFilter(KProcess* /*_process*/, char* _pcbuffer, int _nbufferlen)
{
  QByteArray data;
  data.duplicate(_pcbuffer, _nbufferlen);

  m_string += QString(data);
}

void KEquityPriceUpdateProcess::slotProcessExited(KProcess*)
{
  emit processExited(m_item,m_string);
  m_string.truncate(0);
}
