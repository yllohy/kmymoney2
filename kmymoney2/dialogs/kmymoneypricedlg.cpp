/***************************************************************************
                          kmymoneypricedlg.cpp
                             -------------------
    begin                : Wed Nov 24 2004
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

#include <qcheckbox.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kpushbutton.h>
#include <klistview.h>
#include <kiconloader.h>
#include <kguiitem.h>
#include <kmessagebox.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneypricedlg.h"
#include "kupdatestockpricedlg.h"
#include "kcurrencycalculator.h"
#include "../widgets/kmymoneypriceview.h"
#include "kequitypriceupdatedlg.h"
#include <kmymoney/kmymoneycurrencyselector.h>
#include <kmymoney/mymoneyfile.h>

#include "../kmymoneyglobalsettings.h"

#define COMMODITY_COL     0
#define CURRENCY_COL      1
#define DATE_COL          2
#define PRICE_COL         3
#define SOURCE_COL        4

KMyMoneyPriceDlg::KMyMoneyPriceDlg(QWidget* parent, const char *name) :
  KMyMoneyPriceDlgDecl(parent, name)
{
  KIconLoader *il = KGlobal::iconLoader();
  KGuiItem removeButtenItem( i18n( "&Delete" ),
                    QIconSet(il->loadIcon("delete", KIcon::Small, KIcon::SizeSmall)),
                    i18n("Delete this entry"),
                    i18n("Remove this price item from the file"));
  m_deleteButton->setGuiItem(removeButtenItem);

  KGuiItem newButtenItem( i18n( "&New" ),
                    QIconSet(il->loadIcon("file_new", KIcon::Small, KIcon::SizeSmall)),
                    i18n("Add a new entry"),
                    i18n("Create a new price entry."));
  m_newButton->setGuiItem(newButtenItem);

  KGuiItem editButtenItem( i18n( "&Edit" ),
                    QIconSet(il->loadIcon("edit", KIcon::Small, KIcon::SizeSmall)),
                    i18n("Modify the selected entry"),
                    i18n("Change the details of selected price information."));
  m_editButton->setGuiItem(editButtenItem);

  KGuiItem okButtenItem( i18n("&Close" ),
                    QIconSet(il->loadIcon("button_ok", KIcon::Small, KIcon::SizeSmall)),
                    i18n("Close the dialog"),
                    i18n("Use this to close the dialog and return to the application."));
  m_closeButton->setGuiItem(okButtenItem);

  connect(m_closeButton, SIGNAL(clicked()), this, SLOT(accept()));
  connect(m_editButton, SIGNAL(clicked()), this, SLOT(slotEditPrice()));
  connect(m_priceList, SIGNAL(editPrice()), this, SLOT(slotEditPrice()));
  connect(m_deleteButton, SIGNAL(clicked()), this, SLOT(slotDeletePrice()));
  connect(m_priceList, SIGNAL(deletePrice()), this, SLOT(slotDeletePrice()));
  connect(m_newButton, SIGNAL(clicked()), this, SLOT(slotNewPrice()));
  connect(m_priceList, SIGNAL(newPrice()), this, SLOT(slotNewPrice()));
  connect(m_priceList, SIGNAL(selectionChanged(QListViewItem*)), this, SLOT(slotSelectPrice(QListViewItem*)));
  connect(m_onlineQuoteButton, SIGNAL(clicked()), this, SLOT(slotOnlinePriceUpdate()));
  connect(m_priceList, SIGNAL(onlinePriceUpdate()), this, SLOT(slotOnlinePriceUpdate()));

  slotSelectPrice(0);

  // FIXME: for now, we don't have the logic to delete all prices in a given date range
  m_deleteRangeButton->setEnabled(false);
}

KMyMoneyPriceDlg::~KMyMoneyPriceDlg()
{
}

void KMyMoneyPriceDlg::slotLoadWidgets(void)
{
  m_priceList->clear();

  MyMoneyPriceList list = MyMoneyFile::instance()->priceList();
  MyMoneyPriceList::ConstIterator it_l;
  for(it_l = list.begin(); it_l != list.end(); ++it_l) {
    MyMoneyPriceEntries::ConstIterator it_e;
    if(m_showAllPrices->isChecked()) {
      for(it_e = (*it_l).begin(); it_e != (*it_l).end(); ++it_e) {
        new KMyMoneyPriceItem(m_priceList, *it_e);
      }
    } else {
      if((*it_l).count() > 0) {
        it_e = (*it_l).end();
        --it_e;
        new KMyMoneyPriceItem(m_priceList, *it_e);
      }
    }
  }
}

void KMyMoneyPriceDlg::slotSelectPrice(QListViewItem * item)
{
  m_currentItem = item;
  m_editButton->setEnabled(item != 0);
  m_deleteButton->setEnabled(item != 0);

  // Modification of automatically added entries is not allowed
  if(item) {
    KMyMoneyPriceItem* priceitem = dynamic_cast<KMyMoneyPriceItem*>(item);
    if(priceitem && (priceitem->price().source() == "KMyMoney")) {
      m_editButton->setEnabled(false);
      m_deleteButton->setEnabled(false);
    }
  }
}

void KMyMoneyPriceDlg::slotNewPrice(void)
{
  KUpdateStockPriceDlg dlg(this);
  KMyMoneyPriceItem* item = dynamic_cast<KMyMoneyPriceItem*>(m_priceList->selectedItem());
  if(item) {
    MyMoneySecurity security;
    security = MyMoneyFile::instance()->security(item->price().from());
    dlg.m_security->setSecurity(security);
    security = MyMoneyFile::instance()->security(item->price().to());
    dlg.m_currency->setSecurity(security);
  }

  if(dlg.exec()) {
    MyMoneyPrice price(dlg.m_security->security().id(), dlg.m_currency->security().id(), dlg.date(), MyMoneyMoney(1,1));
    KMyMoneyPriceItem* p = new KMyMoneyPriceItem(m_priceList, price);
    m_priceList->setSelected(p, true);
    // If the user cancels the following operation, we delete the new item
    // and re-select any previously selected one
    if(slotEditPrice() == QDialog::Rejected) {
      delete p;
      if(item)
        m_priceList->setSelected(item, true);
    }
  }
}

int KMyMoneyPriceDlg::slotEditPrice(void)
{
  int rc = QDialog::Rejected;
  KMyMoneyPriceItem* item = dynamic_cast<KMyMoneyPriceItem*>(m_priceList->selectedItem());
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


void KMyMoneyPriceDlg::slotDeletePrice(void)
{
  KMyMoneyPriceItem* item = dynamic_cast<KMyMoneyPriceItem*>(m_priceList->selectedItem());
  if(item) {
    if(KMessageBox::questionYesNo(this, i18n("Do you really want to delete the selected price entry?"), i18n("Delete price information"), KStdGuiItem::yes(), KStdGuiItem::no(), "DeletePrice") == KMessageBox::Yes) {
      MyMoneyFileTransaction ft;
      try {
        MyMoneyFile::instance()->removePrice(item->price());
        ft.commit();
      } catch(MyMoneyException *e) {
        qDebug("Cannot delete price");
        delete e;
      }
    }
  }
}

void KMyMoneyPriceDlg::slotOnlinePriceUpdate(void)
{
  KMyMoneyPriceItem* item = dynamic_cast<KMyMoneyPriceItem*>(m_priceList->selectedItem());
  if(item)
  {
    KEquityPriceUpdateDlg dlg(this, (item->text(COMMODITY_COL)+" "+item->text(CURRENCY_COL)).utf8());
    dlg.exec();
  } else {
    KEquityPriceUpdateDlg dlg(this);
    dlg.exec();
  }
}

// This function is not needed.  However, removing the KUpdateStockPriceDlg
// instantiation below causes link failures:
//
// kmymoney2/widgets/kmymoneypriceview.cpp:179: undefined reference to
// `KUpdateStockPriceDlg::KUpdateStockPriceDlg[in-charge](QWidget*, char const*)'
// kmymoney2/widgets/kmymoneypriceview.cpp:204: undefined reference to
// `KUpdateStockPriceDlg::KUpdateStockPriceDlg[in-charge](QDate const&, QString const&, QWidget*, char const*)'
void KEditEquityEntryDlg_useless(void)
{
  delete new KUpdateStockPriceDlg();
}

// Make sure, that these definitions are only used within this file
// this does not seem to be necessary, but when building RPMs the
// build option 'final' is used and all CPP files are concatenated.
// So it could well be, that in another CPP file these definitions
// are also used.
#undef COMMODITY_COL
#undef CURRENCY_COL
#undef DATE_COL
#undef PRICE_COL
#undef SOURCE_COL


#include "kmymoneypricedlg.moc"
