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

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneypricedlg.h"
#include "kupdatestockpricedlg.h"
#include "kcurrencycalculator.h"
#include "../widgets/kmymoneypriceview.h"

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
  connect(m_editButton, SIGNAL(clicked()), m_priceList, SLOT(slotEditPrice()));
  connect(m_deleteButton, SIGNAL(clicked()), m_priceList, SLOT(slotDeletePrice()));
  connect(m_newButton, SIGNAL(clicked()), m_priceList, SLOT(slotNewPrice()));
  connect(m_priceList, SIGNAL(selectionChanged(QListViewItem*)), this, SLOT(slotSelectPrice(QListViewItem*)));

  connect(m_showAllPrices, SIGNAL(toggled(bool)), m_priceList, SLOT(slotShowAllPrices(bool)));

  slotSelectPrice(0);

  // FIXME: for now, we don't have the logic to delete all prices in a given date range
  m_deleteRangeButton->setEnabled(false);
}

KMyMoneyPriceDlg::~KMyMoneyPriceDlg()
{
}

void KMyMoneyPriceDlg::slotSelectPrice(QListViewItem * item)
{
  m_currentItem = item;
  m_editButton->setEnabled(item != 0);
  m_deleteButton->setEnabled(item != 0);
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
