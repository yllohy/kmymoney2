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

//Generated area. DO NOT EDIT!!!(begin)
#include <qwidget.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qgroupbox.h>
#include <qpushbutton.h>
//Generated area. DO NOT EDIT!!!(end)

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
	KNewAccountDlg(QWidget *parent=0, const char *name=0, const char *title=0, const char *okName="Create");
  KNewAccountDlg(QString m_name, QString no,
    MyMoneyAccount::accountTypeE type, QString description,
    QWidget *parent, const char *name, const char *title, const char *okName);
	~KNewAccountDlg();


protected: 
/*
	void initDialog();
	//Generated area. DO NOT EDIT!!!(begin)
	QLabel *accountNameLabel;
	QLineEdit *accountNameEdit;
	QLineEdit *sortCodeEdit;
	QLabel *accountNoLabel;
	QLineEdit *accountNoEdit;
	QButtonGroup *typesGroup;
	QRadioButton *currentRadio;
	QRadioButton *savingsRadio;
	QGroupBox *startGroup;
	QLabel *startDateLabel;
	QLabel *startBalanceLabel;
	QLineEdit *descriptionEdit;
	QPushButton *createButton;
	QPushButton *cancelButton;
	QLabel *descriptionLabel;
	QLabel *sortCodeLabel;
	//Generated area. DO NOT EDIT!!!(end)
*/
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

private:
//  kMyMoneyEdit *startBalanceEdit;
//  kMyMoneyDateInput *startDateEdit;
};

#endif
