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

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>
#include <kmessagebox.h>
#include <kpushbutton.h>
#include <klistbox.h>
#include <klistview.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kequitypriceupdatedlg.h"
#include "../mymoney/mymoneyfile.h"
#include "../mymoney/mymoneyequity.h"
#include "../mymoney/mymoneyonlinepriceupdate.h"

KEquityPriceUpdateDlg::KEquityPriceUpdateDlg(QWidget *parent) : KEquityPriceUpdateDlgDecl(parent)
{
  lvEquityList->setRootIsDecorated(true);
  lvEquityList->setColumnText(0, QString(i18n("Symbol")));
  lvEquityList->addColumn(i18n("Name"));
  lvEquityList->addColumn(i18n("Symbol"));

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
    KListViewItem* item = new KListViewItem(lvEquityList, (*it).name(), (*it).tradingSymbol());
    lvEquityList->insertItem(item);
  }
  
  connect(btnOK, SIGNAL(clicked()), this, SLOT(slotOKClicked()));
  connect(btnCancel, SIGNAL(clicked()), this, SLOT(slotCancelClicked()));
  connect(btnUpdateSelected, SIGNAL(clicked()), this, SLOT(slotUpdateSelectedClicked()));
  connect(btnUpdateAll, SIGNAL(clicked()), this, SLOT(slotUpdateAllClicked()));
  connect(btnConfigure, SIGNAL(clicked()), this, SLOT(slotConfigureClicked()));
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
  accept();
}

void KEquityPriceUpdateDlg::slotCancelClicked()
{
  reject();
}

void KEquityPriceUpdateDlg::slotUpdateSelectedClicked()
{
  if(!m_pPriceUpdate)
  {
    m_pPriceUpdate = new MyMoneyOnlinePriceUpdate();
  }
  
  //m_pPriceUpdate->
}

void KEquityPriceUpdateDlg::slotUpdateAllClicked()
{
  qDebug("KEquityPriceUpdateDlg: Updating All");
  
  if(!m_pPriceUpdate)
  {
    m_pPriceUpdate = new MyMoneyOnlinePriceUpdate();
  }
  
  QStringList list;
  QPtrList<QListViewItem> selectedItems = lvEquityList->selectedItems();
  for(QPtrList<QListViewItem>::ConstIterator it = selectedItems.begin(); it != selectedItems.end(); ++it)
  {
    qDebug("KEquityPriceUpdateDlg: Updating %s", (*it)->text(0).data());
    QListViewItem* item = (*it);
    list.push_back(item->text(0));
  }
  
  int result = m_pPriceUpdate->getQuotes(list);
}

void KEquityPriceUpdateDlg::slotConfigureClicked()
{

}
