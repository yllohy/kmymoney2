/***************************************************************************
                          knewaccountdlg.cpp
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
#include <qpushbutton.h>
#include <kglobal.h>
#include <klocale.h>

#include "knewaccountdlg.h"

KNewAccountDlg::KNewAccountDlg(QWidget *parent, const char *name, const char *title)
  : KNewAccountDlgDecl(parent,name,true)
{
	if (title)
	  setCaption(title);

  accountNameEdit->setFocus();

  connect(cancelButton, SIGNAL(clicked()), SLOT(reject()));
  connect(createButton, SIGNAL(clicked()), this, SLOT(okClicked()));
}

KNewAccountDlg::KNewAccountDlg(QString m_name, QString no,
  MyMoneyAccount::accountTypeE type, QString description,
  QDate openingDate, MyMoneyMoney openingBalance,
  QWidget *parent, const char *name, const char *title)
  : KNewAccountDlgDecl(parent,name,true)
{
  accountNameEdit->setText(m_name);
  accountNoEdit->setText(no);

	if (title)
	  setCaption(title);

  descriptionEdit->setText(description);

  accountNameEdit->setFocus();

  startDateEdit->setDate(openingDate);
  startBalanceEdit->setText(KGlobal::locale()->formatNumber(openingBalance.amount()));

  connect(cancelButton, SIGNAL(clicked()), SLOT(reject()));
  connect(createButton, SIGNAL(clicked()), this, SLOT(okClicked()));
}

KNewAccountDlg::~KNewAccountDlg()
{
}

void KNewAccountDlg::okClicked()
{
  accountNameText = accountNameEdit->text();
  if (accountNameText.isEmpty()) {
    KMessageBox::error(this, i18n("You have not specified a name.\nPlease fill in this field"));
    accountNameEdit->setFocus();
    return;
  }
  accountNoText = accountNoEdit->text();

  type = MyMoneyAccount::Current;

  descriptionText = descriptionEdit->text();
  startBalance = startBalanceEdit->getMoneyValue();
  startDate = startDateEdit->getQDate();
  accept();
}
