/***************************************************************************
                          knewbankdlg.h
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

#ifndef KNEWBANKDLG_H
#define KNEWBANKDLG_H

#include <klocale.h>
#include <qdialog.h>

#include "../mymoney/mymoneyinstitution.h"

#include "knewbankdlgdecl.h"

// This dialog lets the user create or edit
// a bank.
// Use the second constructor to edit the bank.
class KNewBankDlg : public KNewBankDlgDecl  {
   Q_OBJECT

private:
  MyMoneyInstitution m_institution;

public:
  KNewBankDlg(MyMoneyInstitution& institution, QWidget *parent, const char *name);
	~KNewBankDlg();
  MyMoneyInstitution KNewBankDlg::institution(void);

protected slots:
  void okClicked();
};

#endif
