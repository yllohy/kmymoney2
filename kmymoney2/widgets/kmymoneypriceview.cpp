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
#include <qcheckbox.h>
#include <qgroupbox.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>
#include <kglobal.h>
#include <klistview.h>
#include <kiconloader.h>
#include <kconfig.h>
#include <kmessagebox.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneypriceview.h"
#include "../widgets/kmymoneycurrencyselector.h"
#include "../dialogs/kupdatestockpricedlg.h"
#include "../dialogs/kcurrencycalculator.h"
#include "../dialogs/kequitypriceupdatedlg.h"
#include "../kmymoneysettings.h"
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

  if(!m_pr.isValid())
    m_pr = MyMoneyFile::instance()->price(m_pr.from(), m_pr.to(), m_pr.date());

  if(m_pr.isValid()) {
    QCString priceBase = m_pr.to();
    from = MyMoneyFile::instance()->security(m_pr.from());
    to = MyMoneyFile::instance()->security(m_pr.to());
    if(!to.isCurrency()) {
      from = MyMoneyFile::instance()->security(m_pr.to());
      to = MyMoneyFile::instance()->security(m_pr.from());
      priceBase = m_pr.from();
    }

    setText(COMMODITY_COL, (from.isCurrency()) ? from.id() : from.tradingSymbol());
    setText(CURRENCY_COL, to.id());
    setText(DATE_COL, KGlobal::locale()->formatDate(m_pr.date(), true));
    setText(PRICE_COL, m_pr.rate(priceBase).formatMoney("", prec));
    setText(SOURCE_COL, m_pr.source());
  }
}

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
  m_contextMenu(0),
  m_showAll(false)
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
                        this, SLOT(slotNewPrice()));

  m_contextMenu->insertItem(kiconloader->loadIcon("edit", KIcon::Small),
                        i18n("Edit ..."),
                        this, SLOT(slotEditPrice()));

  m_contextMenu->insertItem(kiconloader->loadIcon("connect_creating", KIcon::Small),
                        i18n("Online Price Update ..."),
                        this, SLOT(slotOnlinePriceUpdate()));

  m_contextMenu->insertItem(kiconloader->loadIcon("delete", KIcon::Small),
                        i18n("Delete ..."),
                        this, SLOT(slotDeletePrice()));

  connect(m_priceHistory, SIGNAL(rightButtonClicked(QListViewItem* , const QPoint&, int)),
          this, SLOT(slotListClicked(QListViewItem*, const QPoint&, int)));
  connect(m_priceHistory, SIGNAL(selectionChanged(QListViewItem*)), this, SIGNAL(selectionChanged(QListViewItem*)));

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
    if(m_showAll) {
      for(it_e = (*it_l).begin(); it_e != (*it_l).end(); ++it_e) {
        new kMyMoneyPriceItem(m_priceHistory, *it_e);
      }
    } else {
      if((*it_l).count() > 0) {
        it_e = (*it_l).end();
        --it_e;
        new kMyMoneyPriceItem(m_priceHistory, *it_e);
      }
    }
  }
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
  int updateId = m_contextMenu->idAt(3);
  int delId = m_contextMenu->idAt(4);

  m_contextMenu->setItemEnabled(editId, item != 0);
  m_contextMenu->setItemEnabled(delId, item != 0);

  kMyMoneyPriceItem* priceitem = dynamic_cast<kMyMoneyPriceItem*>(m_priceHistory->selectedItem());
  if(priceitem) {
    MyMoneySecurity security;
    security = MyMoneyFile::instance()->security(priceitem->price().from());
    m_contextMenu->setItemEnabled(updateId, security.isCurrency() );

    // Modification of automatically added entries is not allowed
    if(priceitem->price().source() == "KMyMoney") {
      m_contextMenu->setItemEnabled(editId, false);
      m_contextMenu->setItemEnabled(updateId, false);
      m_contextMenu->setItemEnabled(delId, false);
    }
  }
  else
    m_contextMenu->setItemEnabled(updateId, false );

  m_contextMenu->exec(QCursor::pos());
}

void kMyMoneyPriceView::slotNewPrice(void)
{
  KUpdateStockPriceDlg dlg(this);
  kMyMoneyPriceItem* item = dynamic_cast<kMyMoneyPriceItem*>(m_priceHistory->selectedItem());
  if(item) {
    MyMoneySecurity security;
    security = MyMoneyFile::instance()->security(item->price().from());
    dlg.m_security->setSecurity(security);
    security = MyMoneyFile::instance()->security(item->price().to());
    dlg.m_currency->setSecurity(security);
  }
  if(dlg.exec()) {
    MyMoneyPrice price(dlg.m_security->security().id(), dlg.m_currency->security().id(), dlg.date(), MyMoneyMoney(1,1));
    kMyMoneyPriceItem* p = new kMyMoneyPriceItem(m_priceHistory, price);
    m_priceHistory->setSelected(p, true);
    // If the user cancels the following operation, we delete the new item
    // and re-select any previously selected one
    if(slotEditPrice() == QDialog::Rejected) {
      delete p;
      if(item)
        m_priceHistory->setSelected(item, true);
    }
  }
}

int kMyMoneyPriceView::slotEditPrice(void)
{
  int rc = QDialog::Rejected;
  kMyMoneyPriceItem* item = dynamic_cast<kMyMoneyPriceItem*>(m_priceHistory->selectedItem());
  if(item) {
    MyMoneySecurity from(MyMoneyFile::instance()->security(item->price().from()));
    MyMoneySecurity to(MyMoneyFile::instance()->security(item->price().to()));
    signed64 fract = MyMoneyMoney::precToDenom(KMyMoneySettings::pricePrecision());

    KCurrencyCalculator calc(from,
                             to,
                             MyMoneyMoney(1,1),
                             item->price().rate(),
                             item->price().date(),
                             fract,
                             this, "currencyCalculator");
    // we always want to update the price, that's why we're here
    calc.m_updateButton->setChecked(true);
    calc.m_updateButton->hide();

    rc = calc.exec();
  }
  return rc;
}

void kMyMoneyPriceView::slotDeletePrice(void)
{
  kMyMoneyPriceItem* item = dynamic_cast<kMyMoneyPriceItem*>(m_priceHistory->selectedItem());
  if(item) {
    if(KMessageBox::questionYesNo(this, i18n("Do you really want to delete the selected price entry?"), i18n("Delete price information"), KStdGuiItem::yes(), KStdGuiItem::no(), "DeletePrice") == KMessageBox::Yes)
      MyMoneyFile::instance()->removePrice(item->price());
  }
}

void kMyMoneyPriceView::slotShowAllPrices(bool enabled)
{
  if(m_showAll != enabled) {
    m_showAll = enabled;
    update(QCString());
  }
}

void kMyMoneyPriceView::slotOnlinePriceUpdate(void)
{
  kMyMoneyPriceItem* item = dynamic_cast<kMyMoneyPriceItem*>(m_priceHistory->selectedItem());
  if(item)
  {
    KEquityPriceUpdateDlg dlg(this, (item->text(COMMODITY_COL)+" "+item->text(CURRENCY_COL)).utf8());
    dlg.exec();
  }
}


#include "kmymoneypriceview.moc"
