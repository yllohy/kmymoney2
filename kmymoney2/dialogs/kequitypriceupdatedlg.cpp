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
#include <kio/netaccess.h>
#include <kprogress.h>
#include <kconfig.h>
#include <kurl.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kequitypriceupdatedlg.h"
#include "../mymoney/mymoneyfile.h"
#include "../mymoney/mymoneysecurity.h"
#include "../mymoney/mymoneyonlinepriceupdate.h"

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
      // only add those securities that have information to download prices
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

  connect(&m_filter, SIGNAL(processExited(QListViewItem*, const QString&)), this, SLOT(slotInsertUpdate(QListViewItem*, const QString&)));
  connect(&m_filter, SIGNAL(receivedStderr(KProcess*, char*, int)), this, SLOT(slotReceivedErrorFromFilter(KProcess*, char*, int)));

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
    launchUpdate(item);
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
    launchUpdate( item );
  }
  else
    logErrorMessage("Security list is empty.");
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
    logStatusMessage(QString("Executing &lt;%1&gt;...").arg(url.path()));

    m_filter.clearArguments();
    m_filter << QStringList::split(" ",url.path());
    m_filter.setItem(_item);

    if(m_filter.start(KProcess::NotifyOnExit, KProcess::All))
      m_filter.resume();
    else
    {
      logErrorMessage(QString("Unable to launch: %1").arg(url.path()));
      slotInsertUpdate(_item,QString());
    }
  }
  else
  {
    logStatusMessage(QString("Fetching URL &lt;%1&gt;...").arg(url.prettyURL()));

    QString tmpFile;
    if( KIO::NetAccess::download( url, tmpFile, NULL ) )
    {
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
    else
    {
      logErrorMessage(KIO::NetAccess::lastErrorString());
      slotInsertUpdate(_item,QString());
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

    if ( gotprice && gotdate )
    {
      _item->setText(PRICE_COL, price.formatMoney());
      _item->setText(DATE_COL, date.toString(Qt::ISODate));
      logStatusMessage(i18n("Price for %1 updated").arg(_item->text(NAME_COL)));
    }
    else
    {
      logErrorMessage(i18n("Unable to update price for %1").arg(_item->text(NAME_COL)));
    }
  }

  prgOnlineProgress->advance(1);
  // _item->setSelected(false) does not cause the screen to be updated
  _item->listView()->setSelected(_item, false); // turn off selection

  // launch the NEXT one ... in case of m_fUpdateAll == false, we
  // need to parse the list to find the next selected one
  QListViewItem* next = _item->nextSibling();
  if ( !m_fUpdateAll )
  {
    while(next && !next->isSelected()) {
      prgOnlineProgress->advance(1);
      next = next->nextSibling();
    }
  }

  if (next)
    launchUpdate(next);

  else {
    // we've run past the end, reset to the default value.
    m_fUpdateAll = false;
    // force progress bar to show 100%
    prgOnlineProgress->setProgress(prgOnlineProgress->totalSteps());
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
