/***************************************************************************
                          kledgerviewasset.cpp  -  description
                             -------------------
    begin                : Mon Dec 2 2002
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

// ----------------------------------------------------------------------------
// Project Includes

#include "kledgerviewasset.h"
#include "../widgets/kmymoneytransactionform.h"
#include "../widgets/kmymoneyregister.h"
#include "../widgets/kmymoneyedit.h"
#include "../widgets/kmymoneydateinput.h"
#include "../widgets/kmymoneypayee.h"
#include "../widgets/kmymoneycategory.h"
#include "../widgets/kmymoneylineedit.h"
#include "../widgets/kmymoneycombo.h"

KLedgerViewAsset::KLedgerViewAsset(QWidget *parent, const char *name )
  : KLedgerViewCheckings(parent,name)
{
  m_register->horizontalHeader()->setLabel(4, i18n("Decrease"));
  m_register->horizontalHeader()->setLabel(5, i18n("Increase"));

  m_form->tabBar()->removeTab(m_tabCheck);
  m_form->tabBar()->removeTab(m_tabAtm);
  m_tabCheck = m_tabAtm = 0;
  m_form->tabBar()->tabAt(0)->setText(i18n("Increase"));
  m_form->tabBar()->tabAt(2)->setText(i18n("Decrease"));

  m_register->setAction(QCString(MyMoneySplit::ActionDeposit), i18n("Increase"));
  m_register->setAction(QCString(MyMoneySplit::ActionWithdrawal), i18n("Decrease"));

  m_register->repaintContents(false);

  // setup action index
  m_actionIdx[0] =
  m_actionIdx[1] =
  m_actionIdx[3] = 0;
  m_actionIdx[2] = 1;
  m_actionIdx[3] = 2;
}

KLedgerViewAsset::~KLedgerViewAsset()
{
}

void KLedgerViewAsset::slotReconciliation(void)
{
  KLedgerViewCheckings::slotReconciliation();
}

bool KLedgerViewAsset::eventFilter( QObject *o, QEvent *e )
{
  return KLedgerViewCheckings::eventFilter(o, e);
}

#include "kledgerviewasset.moc"
