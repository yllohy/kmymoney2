/***************************************************************************
                          kmymoneypriceview.cpp  -  description
                             -------------------
    begin                : Wed Mar 24 2004
    copyright            : (C) 2000-2004 by Michael Edwardes
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

#include <qheader.h>
#include <qlayout.h>
#include <qcursor.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>
#include <kglobal.h>
#include <klistview.h>
#include <kiconloader.h>
#include <kconfig.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneypriceview.h"
#include "../dialogs/kupdatestockpricedlg.h"
#include "../kmymoneyutils.h"

kMyMoneyPriceItem::kMyMoneyPriceItem(KListView *view, const QDate& date, const MyMoneyMoney& price) :
  kMyMoneyListViewItem(view, KGlobal::locale()->formatDate(date, true), QCString())
{
  m_date = date;
  setPrice(price);
}

kMyMoneyPriceItem::~kMyMoneyPriceItem()
{
}

void kMyMoneyPriceItem::setPrice(const MyMoneyMoney& price)
{
  KConfig *kconfig = KGlobal::config();
  kconfig->setGroup("General Options");
  int prec = kconfig->readNumEntry("PricePrecision", 4);
  
  m_price = price;
  setText(1, price.formatMoney("", prec));
}

void kMyMoneyPriceItem::setDate(const QDate& date)
{
  m_date = date;
  setText(0, KGlobal::locale()->formatDate(date, true));
}

int kMyMoneyPriceItem::compare(QListViewItem* i, int col, bool /* ascending */) const
{
  kMyMoneyPriceItem* item = static_cast<kMyMoneyPriceItem*>(i);
  int rc = 0;

  switch(col) {
    case 0:   // date
      if(m_date > item->m_date)
        rc = 1;
      else if(m_date < item->m_date)
        rc = -1;
      break;
      
    case 1:   // value
      if(m_price > item->m_price)
        rc = 1;
      else if(m_price < item->m_price)
        rc = -1;
      break;
  }
  return rc;
}

kMyMoneyPriceView::kMyMoneyPriceView(QWidget *parent, const char *name ) :
  kMyMoneyPriceViewDecl(parent,name),
  m_dirty(false),
  m_contextMenu(0)
{
  m_priceHistory->addColumn(i18n("Date"));
  m_priceHistory->addColumn(i18n("Price"));
  m_priceHistory->setAllColumnsShowFocus(true);
  m_priceHistory->setMultiSelection(false);
  m_priceHistory->setColumnWidthMode(0, QListView::Maximum);
  m_priceHistory->setColumnWidthMode(1, QListView::Maximum);
  m_priceHistory->setColumnAlignment(0, Qt::AlignRight);
  m_priceHistory->setColumnAlignment(1, Qt::AlignRight);
  
  m_priceHistory->header()->setFont(KMyMoneyUtils::headerFont());

  KIconLoader *kiconloader = KGlobal::iconLoader();
  
  m_contextMenu = new KPopupMenu(this);
  m_contextMenu->insertTitle(i18n("Price Options"));
  m_contextMenu->insertItem(kiconloader->loadIcon("filenew", KIcon::Small),
                        i18n("New ..."),
                        this, SLOT(slotAddPrice()));
                        
  m_contextMenu->insertItem(kiconloader->loadIcon("edit", KIcon::Small),
                        i18n("Edit ..."),
                        this, SLOT(slotEditPrice()));
                        
  m_contextMenu->insertItem(kiconloader->loadIcon("delete", KIcon::Small),
                        i18n("Delete ..."),
                        this, SLOT(slotDeletePrice()));

  connect(m_priceHistory, SIGNAL(rightButtonPressed(QListViewItem* , const QPoint&, int)),
          this, SLOT(slotListClicked(QListViewItem*, const QPoint&, int)));
}

kMyMoneyPriceView::~kMyMoneyPriceView()
{
}

void kMyMoneyPriceView::setHistory(const QMap<QDate,MyMoneyMoney>& history)
{
  QMap<QDate,MyMoneyMoney>::ConstIterator it;

  m_priceHistory->clear();
  m_dirty = false;

  for(it = history.begin(); it != history.end(); ++it) {
    new kMyMoneyPriceItem(m_priceHistory, it.key(), (*it));
  }
}

const QMap<QDate, MyMoneyMoney> kMyMoneyPriceView::history(void) const
{
  QMap<QDate, MyMoneyMoney> list;
  QListViewItem* it;
  for(it = m_priceHistory->firstChild(); it; it = it->nextSibling()) {
    kMyMoneyPriceItem* item = static_cast<kMyMoneyPriceItem*>(it);
    list[item->date()] = item->price();
  }
  return list;
}

void kMyMoneyPriceView::resizeEvent(QResizeEvent* /* e*/)
{
  int w = m_priceHistory->visibleWidth()/2;

  m_priceHistory->setColumnWidth(0, w);
  m_priceHistory->setColumnWidth(1, w);
}

void kMyMoneyPriceView::slotListClicked(QListViewItem* item, const QPoint&, int)
{
  int editId = m_contextMenu->idAt(2);
  int delId = m_contextMenu->idAt(3);
  
  m_contextMenu->setItemEnabled(editId, item != 0);
  m_contextMenu->setItemEnabled(delId, item != 0);
  m_contextMenu->exec(QCursor::pos());
}

void kMyMoneyPriceView::slotAddPrice(void)
{
  KUpdateStockPriceDlg dlg(this);
  if(dlg.exec()) {
    QListViewItem* it;
    for(it = m_priceHistory->firstChild(); it; it = it->nextSibling()) {
      kMyMoneyPriceItem* item = static_cast<kMyMoneyPriceItem*>(it);
      if(item->date() == dlg.getDate()) {
        item->setPrice(dlg.getPrice());
        break;
      }
    }
    if(!it) {
      new kMyMoneyPriceItem(m_priceHistory, dlg.getDate(), dlg.getPrice());
    }
    m_dirty = true;
  }
}

void kMyMoneyPriceView::slotEditPrice(void)
{
  kMyMoneyPriceItem* item = static_cast<kMyMoneyPriceItem*>(m_priceHistory->selectedItem());
  if(item) {
    KConfig *kconfig = KGlobal::config();
    kconfig->setGroup("General Options");
    int prec = kconfig->readNumEntry("PricePrecision", 4);

    KUpdateStockPriceDlg dlg(item->date(), item->price().formatMoney("", prec), this);
    if(dlg.exec()) {
      item->setDate(dlg.getDate());
      item->setPrice(dlg.getPrice());
      m_dirty = true;
    }    
  }
}

void kMyMoneyPriceView::slotDeletePrice(void)
{
  kMyMoneyPriceItem* item = static_cast<kMyMoneyPriceItem*>(m_priceHistory->selectedItem());
  if(item) {
    m_priceHistory->removeItem(item);
    m_dirty = true;
  }
}
