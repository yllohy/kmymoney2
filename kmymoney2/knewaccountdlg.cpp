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
#include "knewaccountdlg.h"

KNewAccountDlg::KNewAccountDlg(QWidget *parent, const char *name, const char *title,
  const char *okName)
  : KNewAccountDlgDecl(parent,name,true)
{
//	initDialog();
  createButton->setText(okName);

	if (title)
	  setCaption(title);

//	startDateEdit = new kMyMoneyDateInput(startGroup, QDate::currentDate());
//	startDateEdit->setGeometry(60, 20, 120, 30);

//  startBalanceEdit = new kMyMoneyEdit(startGroup);
//	startBalanceEdit->setGeometry(60, 50, 120, 30);

  connect(cancelButton, SIGNAL(clicked()), SLOT(reject()));
  connect(createButton, SIGNAL(clicked()), this, SLOT(okClicked()));
}

KNewAccountDlg::KNewAccountDlg(QString m_name, QString no,
  MyMoneyAccount::accountTypeE type, QString description,
  QWidget *parent, const char *name, const char *title, const char *okName)
  : KNewAccountDlgDecl(parent,name,true)
{
//	initDialog();

  accountNameEdit->setText(m_name);
  accountNoEdit->setText(no);

  if (type==MyMoneyAccount::Current) {
    currentRadio->setChecked(true);
    savingsRadio->setChecked(false);
  }
  else if (type==MyMoneyAccount::Savings) {
    currentRadio->setChecked(false);
    savingsRadio->setChecked(true);
  } else {
    currentRadio->setChecked(false);
    savingsRadio->setChecked(false);
  }

  createButton->setText(okName);

	if (title)
	  setCaption(title);

  descriptionEdit->setText(description);

//	startDateEdit = new kMyMoneyDateInput(startGroup, QDate::currentDate());
//	startDateEdit->setGeometry(60, 20, 120, 30);

//	startBalanceEdit = new kMyMoneyEdit(startGroup);
//	startBalanceEdit->setGeometry(60, 50, 120, 30);

	startDateEdit->hide();
	startBalanceEdit->hide();

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
  if (currentRadio->isChecked())
    type = MyMoneyAccount::Current;
  else
    type = MyMoneyAccount::Savings;

  descriptionText = descriptionEdit->text();
  startBalance = startBalanceEdit->getMoneyValue();
  startDate = startDateEdit->getQDate();
  accept();
}
