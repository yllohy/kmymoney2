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
#include <qcursor.h>
#include <qtimer.h>

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
#include "../dialogs/kcurrencycalculator.h"
#include "../kmymoneyutils.h"
#include "../mymoney/mymoneyfile.h"

#define COMMODITY_COL   0
#define CURRENCY_COL    1
#define DATE_COL        2
#define PRICE_COL       3
#define SOURCE_COL      4

kMyMoneyPriceItem::kMyMoneyPriceItem(KListView *view, const MyMoneyPrice& pr) :
  kMyMoneyListViewItem(view, QString(), QCString()),
  m_pr(pr)
{
  MyMoneySecurity from, to;
  KConfig *kconfig = KGlobal::config();
  kconfig->setGroup("General Options");
  int prec = kconfig->readNumEntry("PricePrecision", 4);

  m_pr = MyMoneyFile::instance()->price(m_pr.from(), m_pr.to(), m_pr.date());

  if(m_pr.isValid()) {
    QCString priceBase = m_pr.from();
    from = MyMoneyFile::instance()->security(m_pr.from());
    to = MyMoneyFile::instance()->security(m_pr.to());
    if(!to.isCurrency()) {
      from = MyMoneyFile::instance()->security(m_pr.to());
      to = MyMoneyFile::instance()->security(m_pr.from());
      priceBase = m_pr.to();
    }

    setText(COMMODITY_COL, (from.isCurrency()) ? from.id() : from.tradingSymbol());
    setText(CURRENCY_COL, to.id());
    setText(DATE_COL, KGlobal::locale()->formatDate(m_pr.date(), true));
    setText(PRICE_COL, m_pr.rate(priceBase).formatMoney("", prec));
    setText(SOURCE_COL, m_pr.source());
  }
}

kMyMoneyPriceItem::~kMyMoneyPriceItem()
{
}
#if 0
void kMyMoneyPriceItem::setPrice(const MyMoneyMoney& price)
{
  KConfig *kconfig = KGlobal::config();
  kconfig->setGroup("General Options");
  int prec = kconfig->readNumEntry("PricePrecision", 4);

  m_price = price;
  setText(PRICE_COL, price.formatMoney("", prec));
}

void kMyMoneyPriceItem::setDate(const QDate& date)
{
  m_date = date;
  setText(DATE_COL, KGlobal::locale()->formatDate(date, true));
}
#endif

int kMyMoneyPriceItem::compare(QListViewItem* i, int col, bool ascending) const
{
  kMyMoneyPriceItem* item = static_cast<kMyMoneyPriceItem*>(i);
  int rc = 0;

  switch(col) {
    case DATE_COL:   // date
      if(m_pr.date() > item->m_pr.date())
        rc = 1;
      else if(m_pr.date() < item->m_pr.date())
        rc = -1;
      break;

    case PRICE_COL:   // value
      if(m_pr.rate() > item->m_pr.rate())
        rc = 1;
      else if(m_pr.rate() < item->m_pr.rate())
        rc = -1;
      break;

    default:
      rc = QListViewItem::compare(i, col, ascending);
      break;
  }
  return rc;
}

kMyMoneyPriceView::kMyMoneyPriceView(QWidget *parent, const char *name ) :
  kMyMoneyPriceViewDecl(parent,name),
  m_dirty(false),
  m_contextMenu(0)
{
  m_priceHistory->addColumn(i18n("Commodity"));
  m_priceHistory->addColumn(i18n("Currency"));
  m_priceHistory->addColumn(i18n("Date"));
  m_priceHistory->addColumn(i18n("Price"));
  m_priceHistory->addColumn(i18n("Source"));
  m_priceHistory->setAllColumnsShowFocus(true);
  m_priceHistory->setMultiSelection(false);
  m_priceHistory->setColumnWidthMode(0, QListView::Maximum);
  m_priceHistory->setColumnWidthMode(1, QListView::Maximum);
  m_priceHistory->setShowSortIndicator(true);
  m_priceHistory->setSorting(COMMODITY_COL);

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
  connect(m_priceHistory, SIGNAL(clicked(QListViewItem*)), this, SIGNAL(selectionChanged(QListViewItem*)));

  MyMoneyFile::instance()->attach(MyMoneyFile::NotifyClassPrice, this);

  update(QCString());

  // If the widget is shown, the size must be fixed a little later
  // to be appropriate. I saw this in some other places and the only
  // way to solve this problem is to postpone the setup of the size
  // to the time when the widget is on the screen.
  resize(width()-1, height()-1);
  QTimer::singleShot(50, this, SLOT(slotTimerDone()));
}

kMyMoneyPriceView::~kMyMoneyPriceView()
{
  MyMoneyFile::instance()->detach(MyMoneyFile::NotifyClassPrice, this);
}

void kMyMoneyPriceView::slotTimerDone(void)
{
  // the resize operation does the trick to adjust
  // all widgets in the view to the size they should
  // have and show up correctly. Don't ask me, why
  // this is, but it cured the problem (ipwizard).
  resize(width()+1, height()+1);
}

void kMyMoneyPriceView::update(const QCString& /* id */)
{
  m_priceHistory->clear();

  MyMoneyPriceList list = MyMoneyFile::instance()->priceList();
  MyMoneyPriceList::ConstIterator it_l;
  for(it_l = list.begin(); it_l != list.end(); ++it_l) {
    MyMoneyPriceEntries::ConstIterator it_e;
    for(it_e = (*it_l).begin(); it_e != (*it_l).end(); ++it_e) {
      new kMyMoneyPriceItem(m_priceHistory, *it_e);
    }
  }
#if 0
// FIXME pricelist
  QValueList<MyMoneyPrice> priceList = MyMoneyFile::instance()->priceList();
  QValueList<MyMoneyPrice>::ConstIterator it;

  m_priceHistory->clear();
  for(it = priceList.begin(); it != priceList.end(); ++it) {
    new kMyMoneyPriceItem(m_priceHistory, *it);
  }
#endif
}

void kMyMoneyPriceView::setHistory(const QMap<QDate,MyMoneyMoney>& history)
{
#if 0
  QMap<QDate,MyMoneyMoney>::ConstIterator it;

  m_priceHistory->clear();
  m_dirty = false;

  for(it = history.begin(); it != history.end(); ++it) {
    new kMyMoneyPriceItem(m_priceHistory, it.key(), (*it));
  }
#endif
}

const QMap<QDate, MyMoneyMoney> kMyMoneyPriceView::history(void) const
{
  QMap<QDate, MyMoneyMoney> list;
#if 0
  QListViewItem* it;
  for(it = m_priceHistory->firstChild(); it; it = it->nextSibling()) {
    kMyMoneyPriceItem* item = dynamic_cast<kMyMoneyPriceItem*>(it);
    Q_CHECK_PTR(item);
    list[item->date()] = item->price();
  }
#endif
  return list;
}

void kMyMoneyPriceView::resizeEvent(QResizeEvent* e)
{
  int w = m_priceHistory->visibleWidth()/5;

  m_priceHistory->setColumnWidth(0, w);
  m_priceHistory->setColumnWidth(1, w);
  m_priceHistory->setColumnWidth(2, w);
  m_priceHistory->setColumnWidth(3, w);
  m_priceHistory->setColumnWidth(4, w);
  m_priceHistory->resizeContents(
    m_priceHistory->visibleWidth(),
    m_priceHistory->contentsHeight());

  kMyMoneyPriceViewDecl::resizeEvent(e);
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
#if 0
  KUpdateStockPriceDlg dlg(this);
  if(dlg.exec()) {
    QListViewItem* it;
    for(it = m_priceHistory->firstChild(); it; it = it->nextSibling()) {
      kMyMoneyPriceItem* item = dynamic_cast<kMyMoneyPriceItem*>(it);
      Q_CHECK_PTR(item);
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
#endif
}

void kMyMoneyPriceView::slotEditPrice(void)
{
  kMyMoneyPriceItem* item = dynamic_cast<kMyMoneyPriceItem*>(m_priceHistory->selectedItem());
  if(item) {
    KConfig *kconfig = KGlobal::config();
    kconfig->setGroup("General Options");
    int prec = kconfig->readNumEntry("PricePrecision", 4);

    MyMoneySecurity from(MyMoneyFile::instance()->security(item->price().from()));
    MyMoneySecurity to(MyMoneyFile::instance()->security(item->price().to()));
    int fract = to.smallestAccountFraction();

    KCurrencyCalculator calc(from,
                             to,
                             MyMoneyMoney(1,1),
                             item->price().rate(),
                             item->price().date(),
                             fract,
                             this, "currencyCalculator");

    if(calc.exec() == QDialog::Accepted) {
    }
#if 0
    KUpdateStockPriceDlg dlg(QDate(), "", this);
    if(dlg.exec()) {
      item->setDate(dlg.getDate());
      item->setPrice(dlg.getPrice());
      m_dirty = true;
    }
#endif
  }
}

void kMyMoneyPriceView::slotDeletePrice(void)
{
  kMyMoneyPriceItem* item = dynamic_cast<kMyMoneyPriceItem*>(m_priceHistory->selectedItem());
  if(item) {
    m_priceHistory->removeItem(item);
    delete item;
    m_dirty = true;
  }
}
