/***************************************************************************
                          kledgerviewcheckings.cpp  -  description
                             -------------------
    begin                : Sun Jul 14 2002
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

#include <qlayout.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>
#include <kpushbutton.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kledgerviewcheckings.h"

KLedgerViewCheckings::KLedgerViewCheckings(QWidget *parent, const char *name )
  : KLedgerView(parent,name)
{
  m_register = new kMyMoneyRegisterCheckings(this);
  m_register->setView(this);

  QGridLayout* formLayout = new QGridLayout( this, 1, 1, 11, 6, "Form1Layout");
  QVBoxLayout* buttonLayout = new QVBoxLayout( 0, 0, 6, "Layout2");
  QVBoxLayout* ledgerLayout = new QVBoxLayout( 0, 0, 6, "Layout3");

  KPushButton* detailsButton = new KPushButton(this, "detailsButton" );
  detailsButton->setText(i18n("Account Details"));
  buttonLayout->addWidget(detailsButton);

  KPushButton* reconcileButton = new KPushButton(this, "reconcileButton");
  reconcileButton->setText(i18n("Reconcile ..."));
  buttonLayout->addWidget(reconcileButton);

  QSpacerItem* spacer = new QSpacerItem( 20, 20,
                   QSizePolicy::Minimum, QSizePolicy::Expanding );
  buttonLayout->addItem( spacer );

  formLayout->addLayout( buttonLayout, 0, 1 );

  ledgerLayout->addWidget(m_register);
  //ledgerLayout->addWidget(m_form);

  formLayout->addLayout( ledgerLayout, 0, 0);

  m_register->setNumCols(7);
  m_register->setCurrentCell(0, 1);
  m_register->horizontalHeader()->setLabel(0, i18n("Nr."));
	m_register->horizontalHeader()->setLabel(1, i18n("Date"));
	m_register->horizontalHeader()->setLabel(2, i18n("Payee"));
	m_register->horizontalHeader()->setLabel(3, i18n("C"));
	m_register->horizontalHeader()->setLabel(4, i18n("Payment"));
	m_register->horizontalHeader()->setLabel(5, i18n("Deposit"));
	m_register->horizontalHeader()->setLabel(6, i18n("Balance"));
	m_register->setSelectionMode(QTable::NoSelection);
 	m_register->setLeftMargin(0);
	m_register->verticalHeader()->hide();
  m_register->setColumnStretchable(0, false);
  m_register->setColumnStretchable(1, false);
	m_register->setColumnStretchable(2, false);
  m_register->setColumnStretchable(3, false);
	m_register->setColumnStretchable(4, false);
  m_register->setColumnStretchable(5, false);
	m_register->setColumnStretchable(6, false);
		
	m_register->horizontalHeader()->setResizeEnabled(false);
	m_register->horizontalHeader()->setMovingEnabled(false);
}

KLedgerViewCheckings::~KLedgerViewCheckings()
{
  delete m_register;
}

void KLedgerViewCheckings::refreshView(void)
{
  KLedgerView::refreshView();
}

void KLedgerViewCheckings::resizeEvent(QResizeEvent*)
{
  int w = m_register->visibleWidth();

  int m_debitWidth = 100;
  int m_creditWidth = 100;
  int m_balanceWidth = 100;

  m_register->setColumnWidth(0, 100);
  m_register->adjustColumn(1);
  m_register->setColumnWidth(3, 20);
  m_register->setColumnWidth(4, m_debitWidth);
  m_register->setColumnWidth(5, m_creditWidth);
  m_register->setColumnWidth(6, m_balanceWidth);

  for(int i = 0; i < 7; ++i) {
    switch(i) {
      default:
        w -= m_register->columnWidth(i);
        break;
      case 2:     // skip the one, we want to set
        break;
    }
  }
  m_register->setColumnWidth(2, w);
}

void KLedgerViewCheckings::show()
{
  KLedgerView::show();
  resizeEvent(NULL);
}

