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

// ----------------------------------------------------------------------------
// KDE Includes

#include <kpushbutton.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneypricedlg.h"
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
  connect(m_closeButton, SIGNAL(clicked()), this, SLOT(accept()));
  connect(m_modifyButton, SIGNAL(clicked()), m_priceList, SLOT(slotEditPrice()));
  connect(m_priceList, SIGNAL(selectionChanged(QListViewItem*)), this, SLOT(slotSelectPrice(QListViewItem*)));

  slotSelectPrice(0);
}

KMyMoneyPriceDlg::~KMyMoneyPriceDlg()
{
}

void KMyMoneyPriceDlg::slotSelectPrice(QListViewItem * item)
{
  m_currentItem = item;
  m_modifyButton->setEnabled(item != 0);
  m_deleteButton->setEnabled(item != 0);
  m_deleteRangeButton->setEnabled(item != 0);
}
