/***************************************************************************
                          knewequityentrydlg.cpp  -  description
                             -------------------
    begin                : Tue Jan 29 2002
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

#include <kpushbutton.h>
#include <kiconloader.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "knewequityentrydlg.h"
#include "../widgets/kmymoneyedit.h"
#include "../mymoney/mymoneymoney.h"

KNewEquityEntryDlg::KNewEquityEntryDlg(QWidget *parent, const char *name)
  : kNewEquityEntryDecl(parent, name, TRUE)
{
  edtFraction->hideCalculatorButton();
  edtFraction->setPrecision(0);
  edtFraction->loadText("100");

  connect(btnOK, SIGNAL(clicked()), this, SLOT(onOKClicked()));
  connect(btnCancel, SIGNAL(clicked()), this, SLOT(reject()));

  connect(edtFraction, SIGNAL(textChanged(const QString&)), this, SLOT(slotDataChanged()));
  connect(edtMarketSymbol, SIGNAL(textChanged(const QString&)), this, SLOT(slotDataChanged()));
  connect(edtEquityName, SIGNAL(textChanged(const QString&)), this, SLOT(slotDataChanged()));

  // add icons to buttons
  KIconLoader *il = KGlobal::iconLoader();
  KGuiItem okButtenItem( i18n("&Ok" ),
                    QIconSet(il->loadIcon("button_ok", KIcon::Small, KIcon::SizeSmall)),
                    i18n("Accepts the value and stores them"),
                    i18n("Use this to accept all values and close the dialog."));
  btnOK->setGuiItem(okButtenItem);

  KGuiItem cancelButtenItem( i18n( "&Cancel" ),
                    QIconSet(il->loadIcon("button_cancel", KIcon::Small, KIcon::SizeSmall)),
                    i18n("Cancel the operation"),
                    i18n("Use this to dismiss all the changes made in this dialog."));
  btnCancel->setGuiItem(cancelButtenItem);

  slotDataChanged();

  edtEquityName->setFocus();
}

KNewEquityEntryDlg::~KNewEquityEntryDlg()
{
}

/** No descriptions */
void KNewEquityEntryDlg::onOKClicked()
{
  m_strSymbolName = edtMarketSymbol->text();
  m_strName = edtEquityName->text();
  m_fraction = edtFraction->getMoneyValue().abs();
  accept();
}

void KNewEquityEntryDlg::setSymbolName(const QString& str)
{
  m_strSymbolName = str;
  edtMarketSymbol->setText(m_strSymbolName);
}

void KNewEquityEntryDlg::setName(const QString& str)
{
  m_strName = str;
  edtEquityName->setText(m_strName);
}

void KNewEquityEntryDlg::slotDataChanged(void)
{
  bool okEnabled = true;

  if(!edtFraction->getMoneyValue().isPositive()
  || edtMarketSymbol->text().isEmpty()
  || edtEquityName->text().isEmpty())
    okEnabled = false;

  btnOK->setEnabled(okEnabled);
}
