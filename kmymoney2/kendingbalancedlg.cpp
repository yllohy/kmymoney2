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
#include <kmessagebox.h>
#include <kglobal.h>
#include <klocale.h>
#include "kendingbalancedlg.h"

KEndingBalanceDlg::KEndingBalanceDlg(MyMoneyMoney& prevBal, MyMoneyMoney& endingGuess, QWidget *parent, const char *name)
 : KEndingBalanceDlgDecl(parent,name,true)
{
//	initDialog();
	
	previousbalEdit->setText(KGlobal::locale()->formatMoney(prevBal.amount()));
	previousbalEdit->setFocus();
	previousbalEdit->setSelection(0, KGlobal::locale()->formatNumber(prevBal.amount()).length());	
	
	endingEdit->setText(KGlobal::locale()->formatNumber(endingGuess.amount()));
	
	connect(cancelBtn, SIGNAL(clicked()), SLOT(reject()));
	connect(okBtn, SIGNAL(clicked()), SLOT(okClicked()));
}

KEndingBalanceDlg::~KEndingBalanceDlg()
{
}

void KEndingBalanceDlg::okClicked()
{
  endingBalance = endingEdit->getMoneyValue();
	previousBalance = previousbalEdit->getMoneyValue();
  endingDate = endingDateEdit->getQDate();

  if (!endingDate.isValid()) {
    KMessageBox::information(this, i18n("Please enter a valid date"));
    // Return the focus
    endingDateEdit->setFocus();
    return;
  }

  accept();
}
