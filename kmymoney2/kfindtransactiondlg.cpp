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

KFindTransactionDlg::KFindTransactionDlg(MyMoneyFile *file, QWidget *parent, const char *name)
 : KFindTransactionDlgDecl(parent,name,false)
{
  m_filePointer = file;
	
	readConfig();

	connect(dateRadio, SIGNAL(toggled(bool)), this, SLOT(dateToggled(bool)));
	connect(amountRadio, SIGNAL(toggled(bool)), this, SLOT(amountToggled(bool)));
	connect(creditRadio, SIGNAL(toggled(bool)), this, SLOT(creditToggled(bool)));
	connect(statusRadio, SIGNAL(toggled(bool)), this, SLOT(statusToggled(bool)));
	connect(descriptionRadio, SIGNAL(toggled(bool)), this, SLOT(descriptionToggled(bool)));
	connect(numberRadio, SIGNAL(toggled(bool)), this, SLOT(numberToggled(bool)));
	connect(payeeRadio, SIGNAL(toggled(bool)), this, SLOT(payeeToggled(bool)));
	connect(categoryRadio, SIGNAL(toggled(bool)), this, SLOT(categoryToggled(bool)));
	
	connect(searchBtn, SIGNAL(clicked()), this, SLOT(searchClicked()));
	connect(closeBtn, SIGNAL(clicked()), this, SLOT(closeClicked()));
}

KFindTransactionDlg::~KFindTransactionDlg()
{
  writeConfig();
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

void KFindTransactionDlg::payeeToggled(bool on)
{
  payeeEdit->setEnabled(on);
  payeeRegExpCheck->setEnabled(on);
}

void KFindTransactionDlg::categoryToggled(bool on)
{
  categoryCombo->clear();
  QString theText;
  QListIterator<MyMoneyCategory> categoryIterator = m_filePointer->categoryIterator();
  for ( ; categoryIterator.current(); ++categoryIterator) {
    MyMoneyCategory *category = categoryIterator.current();
    theText = category->name().latin1();
    categoryCombo->insertItem(theText);
    for ( QStringList::Iterator it = category->minorCategories().begin(); it != category->minorCategories().end(); ++it ) {
      theText = category->name();
  		theText += ":";
	  	theText += *it;
      categoryCombo->insertItem(theText);
    }
  }
  categoryCombo->setEnabled(on);
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
  	bool& doPayee,
  	bool& doCategory,
  	QString& amountID,
  	QString& creditID,
  	QString& statusID,
  	QString& description,
  	QString& number,
    MyMoneyMoney& money,
    QDate& startDate,
    QDate& endDate,
    QString& payee,
    QString& category,
    bool& descriptionRegExp,
    bool& numberRegExp,
    bool& payeeRegExp )
{
  doDate = dateRadio->isChecked();
  doAmount = amountRadio->isChecked();
  doCredit = creditRadio->isChecked();
  doStatus = statusRadio->isChecked();
  doDescription = descriptionRadio->isChecked();
  doNumber = numberRadio->isChecked();
  doPayee = payeeRadio->isChecked();
  doCategory = categoryRadio->isChecked();
  amountID = amountCombo->currentText();
  creditID = creditCombo->currentText();
  statusID = statusCombo->currentText();
  description = descriptionEdit->text();
  number = numberEdit->text();
  MyMoneyMoney tmp(moneyEdit->getMoneyValue());
  money = tmp;
  startDate = startDateInput->getQDate();
  endDate = endDateInput->getQDate();
  payee = payeeEdit->text();
  category = categoryCombo->currentText();
  descriptionRegExp = descriptionRegExpCheck->isChecked();
  numberRegExp = numberRegExpCheck->isChecked();
  payeeRegExp = payeeRegExpCheck->isChecked();
}

void KFindTransactionDlg::readConfig(void)
{
/*
  config->setGroup("Last Use Settings");
  txtFileExport->setText(config->readEntry("KExportDlg_LastFile"));
  cbxAccount->setChecked(config->readBoolEntry("KExportDlg_AccountOpt", true));
  cbxCategories->setChecked(config->readBoolEntry("KExportDlg_CatOpt", true));
  dateStartDate->setDate(config->readDateTimeEntry("KExportDlg_StartDate"));
  dateEndDate->setDate(config->readDateTimeEntry("KExportDlg_EndDate"));
*/
}

void KFindTransactionDlg::writeConfig(void)
{
/*
  KConfig *config = KGlobal::config();
  config->setGroup("Last Use Settings");
  config->writeEntry("KExportDlg_LastFile", txtFileExport->text()));
  config->writeEntry("KExportDlg_AccountOpt", cbxAccount->isChecked());
  config->writeEntry("KExportDlg_CatOpt", cbxCategories->isChecked());
  config->writeEntry("KExportDlg_StartDate", QDateTime(dateStartDate->getQDate()));
  config->writeEntry("KExportDlg_EndDate", QDateTime(dateEndDate->getQDate()));

  config->sync();
*/
}
