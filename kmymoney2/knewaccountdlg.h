/***************************************************************************
                          knewaccountdlg.h
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

#ifndef KNEWACCOUNTDLG_H
#define KNEWACCOUNTDLG_H

#include <qdialog.h>
#include <klocale.h>

#include <mymoney/mymoneyaccount.h>
#include <mymoney/mymoneymoney.h>
#include "widgets/kmymoneyedit.h"
#include "widgets/kmymoneydateinput.h"

#include "knewaccountdlgdecl.h"

// This dialog lets you create/edit an account.
// Use the second constructor to edit an account.
class KNewAccountDlg : public KNewAccountDlgDecl  {
   Q_OBJECT
public:
	KNewAccountDlg(QString institution, QWidget *parent=0, const char *name=0, const char *title=0);
  KNewAccountDlg(QString institution, QString m_name, QString no,
    MyMoneyAccount::accountTypeE type, QString description,
    QDate openingDate, MyMoneyMoney openingBalance,
    QWidget *parent, const char *name, const char *title);
	~KNewAccountDlg();

public:
  // Read from these to get the new values.
  QString accountNameText;
  QString accountNoText;
  MyMoneyAccount::accountTypeE type;
  QString descriptionText;
  MyMoneyMoney startBalance;
  QDate startDate;

protected slots:
  void okClicked();
};

#endif
