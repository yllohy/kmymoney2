/***************************************************************************
                          kendingbalancedlg.cpp
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes
    email                : mte@users.sourceforge.net
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

#include <qpixmap.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kglobal.h>
#if QT_VERSION > 300
#include <kstandarddirs.h>
#else
#include <kstddirs.h>
#endif

// ----------------------------------------------------------------------------
// Project Includes

#include "kendingbalancedlg.h"

KEndingBalanceDlg::KEndingBalanceDlg(const MyMoneyMoney& prevBal, const MyMoneyMoney& endingGuess, const QDate& statementDate, QWidget *parent, const char *name)
 : KEndingBalanceDlgDecl(parent,name,true),
   m_endingBalance(0),
   m_previousBalance(0),
   m_endingDate(QDate::currentDate())
{
   //QString filename = KGlobal::dirs()->findResource("appdata", "pics/dlg_ending_balance.png");
  m_qpixmaplabel->setPixmap(QPixmap(KGlobal::dirs()->findResource("appdata", "pics/dlg_ending_balance.png")));

	previousbalEdit->setText(prevBal.formatMoney());
	previousbalEdit->setFocus();
	previousbalEdit->setSelection(0, previousbalEdit->text().length());	
	
	endingEdit->setText(endingGuess.formatMoney());

  endingDateEdit->setDate(statementDate);
	
	connect(cancelBtn, SIGNAL(clicked()), SLOT(reject()));
	connect(okBtn, SIGNAL(clicked()), SLOT(okClicked()));
}

KEndingBalanceDlg::~KEndingBalanceDlg()
{
}

void KEndingBalanceDlg::okClicked()
{
  m_endingBalance = endingEdit->getMoneyValue();
	m_previousBalance = previousbalEdit->getMoneyValue();
  m_endingDate = endingDateEdit->getQDate();

  // removed the date check because it can't be invalid !
  accept();
}
