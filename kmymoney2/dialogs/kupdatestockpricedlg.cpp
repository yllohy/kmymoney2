/***************************************************************************
                          kupdatestockpricedlg.cpp  -  description
                             -------------------
    begin                : Thu Feb 7 2002
    copyright            : (C) 2000-2002 by Michael Edwardes
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

// ----------------------------------------------------------------------------
// KDE Includes

#include <kiconloader.h>
#include <kguiitem.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kupdatestockpricedlg.h"
#include "../widgets/kmymoneycurrencyselector.h"

KUpdateStockPriceDlg::KUpdateStockPriceDlg(QWidget* parent,  const char* name) :
  kUpdateStockPriceDecl(parent, name, true)
{
  m_date->setDate(QDate::currentDate());
  init();
}

KUpdateStockPriceDlg::KUpdateStockPriceDlg(const QDate& date, const MyMoneyPrice& price, QWidget* parent,  const char* name) :
  kUpdateStockPriceDecl(parent, name, true)
{
  m_date->setDate(date);

  init();
}

KUpdateStockPriceDlg::~KUpdateStockPriceDlg()
{
}

void KUpdateStockPriceDlg::init()
{
  KIconLoader* il = KGlobal::iconLoader();
  KGuiItem okButtenItem( i18n("&Ok" ),
                    QIconSet(il->loadIcon("button_ok", KIcon::Small, KIcon::SizeSmall)),
                    i18n("Accepts the entered data and stores it"),
                    i18n("Use this to accept the data."));
  m_okButton->setGuiItem(okButtenItem);

  KGuiItem cancelButtenItem( i18n( "&Cancel" ),
                    QIconSet(il->loadIcon("button_cancel", KIcon::Small, KIcon::SizeSmall)),
                    i18n("Reject all changes to the data and closes the dialog"),
                    i18n("Use this to reject all changes."));
  m_cancelButton->setGuiItem(cancelButtenItem);

  connect(m_okButton, SIGNAL(clicked()), this, SLOT(accept()));
  connect(m_cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

  connect(m_commodity, SIGNAL(activated(int)), this, SLOT(slotCheckData()));
  connect(m_currency, SIGNAL(activated(int)), this, SLOT(slotCheckData()));

  slotCheckData();
}

int KUpdateStockPriceDlg::exec(void)
{
  slotCheckData();
  return kUpdateStockPriceDecl::exec();
}

void KUpdateStockPriceDlg::slotCheckData(void)
{
  QCString from = m_commodity->security().id();
  QCString to   = m_currency->security().id();

  m_okButton->setEnabled(!from.isEmpty() && !to.isEmpty() && from != to);
}







































