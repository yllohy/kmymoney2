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
#include <kglobal.h>
#include <klocale.h>
#if QT_VERSION > 300
#include <kstandarddirs.h>
#else
#include <kstddirs.h>
#endif

#include <qpixmap.h>

#include <kmessagebox.h>
#include <kglobal.h>
#include <klocale.h>
#include "kendingbalancedlg.h"

KEndingBalanceDlg::KEndingBalanceDlg(MyMoneyMoney& prevBal, MyMoneyMoney& endingGuess, QWidget *parent, const char *name)
 : KEndingBalanceDlgDecl(parent,name,true)
{
//	initDialog();
   //QString filename = KGlobal::dirs()->findResource("appdata", "pics/dlg_ending_balance.png");
  QPixmap *pm = new QPixmap(KGlobal::dirs()->findResource("appdata", "pics/dlg_ending_balance.png"));
  m_qpixmaplabel->setPixmap(*pm);

	previousbalEdit->setText(KGlobal::locale()->formatMoney(prevBal.amount(),""));
	previousbalEdit->setFocus();
	previousbalEdit->setSelection(0, previousbalEdit->text().length());	
	
	endingEdit->setText(KGlobal::locale()->formatMoney(endingGuess.amount(),""));
	
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

  // removed the date check because it can't be invalid !
  accept();
}
