/***************************************************************************
                          kimportverifydlg.cpp  -  description
                             -------------------
    begin                : Mon Jun 9 2003
    copyright            : (C) 2000-2003 by Thomas Baumgart
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

#include <qpushbutton.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kimportverifydlg.h"
#include "../views/kgloballedgerview.h"
#include "../mymoney/mymoneyfile.h"

KImportVerifyDlg::KImportVerifyDlg(const MyMoneyAccount& account, QWidget *parent, const char *name)
 : KImportVerifyDlgDecl(parent,name,true),
   m_account(account)
{
  m_ledgerView->slotAccountSelected(account.name());
  connect(buttonOk, SIGNAL(clicked()), this, SLOT(slotOkClicked()));
}

KImportVerifyDlg::~KImportVerifyDlg()
{
}

void KImportVerifyDlg::slotOkClicked(void)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  QValueList<MyMoneyTransaction> list = file->transactionList(m_account.id());
  QValueList<MyMoneyTransaction>::Iterator it_t;

  for(it_t = list.begin(); it_t != list.end(); ++it_t) {
    if((*it_t).value("Imported") != "") {
      (*it_t).deletePair("Imported");
      file->modifyTransaction(*it_t);
    }
  }
  
  accept();  
}
