/***************************************************************************
                          kfindtransactiondlg.cpp
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
#include <kcombobox.h>

#include "kfindtransactiondlg.h"

KFindTransactionDlg::KFindTransactionDlg(QWidget *parent, const char *name)
 : KFindTransactionDlgDecl(parent,name,false)
{
//	initDialog();
	
//	startDateInput = new kMyMoneyDateInput(datesGroup, QDate::currentDate());
//	startDateInput->setGeometry(80, 20, 120, 20);
//	endDateInput = new kMyMoneyDateInput(datesGroup, QDate::currentDate());
//	endDateInput->setGeometry(80, 50, 120, 20);
	
//	moneyEdit = new kMyMoneyEdit(amountGroup);
//	moneyEdit->setGeometry(80, 50, 120, 20);
	
	amountCombo->setEnabled(false);
	creditCombo->setEnabled(false);
	statusCombo->setEnabled(false);
	descriptionEdit->setEnabled(false);
	numberEdit->setEnabled(false);
  moneyEdit->setEnabled(false);
  startDateInput->setEnabled(false);
  endDateInput->setEnabled(false);
  descriptionRegExpCheck->setEnabled(false);
  numberRegExpCheck->setEnabled(false);
	
	connect(dateRadio, SIGNAL(toggled(bool)), this, SLOT(dateToggled(bool)));
	connect(amountRadio, SIGNAL(toggled(bool)), this, SLOT(amountToggled(bool)));
	connect(creditRadio, SIGNAL(toggled(bool)), this, SLOT(creditToggled(bool)));
	connect(statusRadio, SIGNAL(toggled(bool)), this, SLOT(statusToggled(bool)));
	connect(descriptionRadio, SIGNAL(toggled(bool)), this, SLOT(descriptionToggled(bool)));
	connect(numberRadio, SIGNAL(toggled(bool)), this, SLOT(numberToggled(bool)));
	
	connect(searchBtn, SIGNAL(clicked()), this, SLOT(searchClicked()));
	connect(closeBtn, SIGNAL(clicked()), this, SLOT(closeClicked()));
}

KFindTransactionDlg::~KFindTransactionDlg()
{
}

void KFindTransactionDlg::closeClicked()
{
  hide();
//  accept();
}

void KFindTransactionDlg::dateToggled(bool on)
{
  startDateInput->setEnabled(on);
  endDateInput->setEnabled(on);
}

void KFindTransactionDlg::amountToggled(bool on)
{
  amountCombo->setEnabled(on);
  moneyEdit->setEnabled(on);
}

void KFindTransactionDlg::creditToggled(bool on)
{
  creditCombo->setEnabled(on);
}

void KFindTransactionDlg::statusToggled(bool on)
{
  statusCombo->setEnabled(on);
}

void KFindTransactionDlg::descriptionToggled(bool on)
{
  descriptionEdit->setEnabled(on);
  descriptionRegExpCheck->setEnabled(on);
}

void KFindTransactionDlg::numberToggled(bool on)
{
  numberEdit->setEnabled(on);
  numberRegExpCheck->setEnabled(on);
}

void KFindTransactionDlg::searchClicked()
{
  emit searchReady();
}

void KFindTransactionDlg::data(
	bool& doDate,
	bool& doAmount,
	bool& doCredit,
	bool& doStatus,
	bool& doDescription,
	bool& doNumber,
	QString& amountID,
	QString& creditID,
	QString& statusID,
	QString& description,
	QString& number,
  MyMoneyMoney& money,
  QDate& startDate,
  QDate& endDate,
  bool& descriptionRegExp,
  bool& numberRegExp )
{
  doDate = dateRadio->isChecked();
  doAmount = amountRadio->isChecked();
  doCredit = creditRadio->isChecked();
  doStatus = statusRadio->isChecked();
  doDescription = descriptionRadio->isChecked();
  doNumber = numberRadio->isChecked();
  amountID = amountCombo->currentText();
  creditID = creditCombo->currentText();
  statusID = statusCombo->currentText();
  description = descriptionEdit->text();
  number = numberEdit->text();
  MyMoneyMoney tmp(moneyEdit->getMoneyValue());
  money = tmp;
  startDate = startDateInput->getQDate();
  endDate = endDateInput->getQDate();
  descriptionRegExp = descriptionRegExpCheck->isChecked();
  numberRegExp = numberRegExpCheck->isChecked();
}
