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

#include <qstring.h>

#include <kpushbutton.h>
 
#include "kupdatestockpricedlg.h"

KUpdateStockPriceDlg::KUpdateStockPriceDlg(QWidget* parent,  const char* name) : kUpdateStockPriceDecl(parent,name,TRUE)
{
  init();  
}

KUpdateStockPriceDlg::KUpdateStockPriceDlg(const QString& strDate, const QString& strPrice, QWidget* parent,  const char* name) : kUpdateStockPriceDecl(parent,name,TRUE)
{
  m_date = strDate;
  m_price = strPrice;
  init();
}

KUpdateStockPriceDlg::~KUpdateStockPriceDlg()
{

}

void KUpdateStockPriceDlg::init()
{
  connect(btnOK, SIGNAL(clicked()), this, SLOT(slotOkClicked()));
  connect(btnCancel, SIGNAL(clicked()), this, SLOT(slotCancelClicked()));
}

void KUpdateStockPriceDlg::slotOkClicked()
{
  accept();
}

void KUpdateStockPriceDlg::slotCancelClicked()
{
  reject();
}








































