/***************************************************************************
                          kfindtransactiondlg.h
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes
    email                : Michael.Edwardes@students.dmu.ac.uk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KFINDTRANSACTIONDLG_H
#define KFINDTRANSACTIONDLG_H

//Generated area. DO NOT EDIT!!!(begin)
#include <qwidget.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qpushbutton.h>
//Generated area. DO NOT EDIT!!!(end)

#include <klocale.h>
#include <qdialog.h>

#include "widgets/kmymoneyedit.h"
#include "widgets/kmymoneydateinput.h"

#include "kfindtransactiondlgdecl.h"

/**
  *@author Michael Edwardes
  */

class KFindTransactionDlg : public KFindTransactionDlgDecl  {
   Q_OBJECT
public: 
	KFindTransactionDlg(QWidget *parent=0, const char *name=0);
	~KFindTransactionDlg();
  void data(
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
    bool& numberRegExp );

protected slots:
  void closeClicked();
  void dateToggled(bool on);
  void amountToggled(bool on);
  void creditToggled(bool on);
  void statusToggled(bool on);
  void descriptionToggled(bool on);
  void numberToggled(bool on);
  void searchClicked();

signals:
  void searchReady();

protected:
/*
	void initDialog();
	//Generated area. DO NOT EDIT!!!(begin)
	QGroupBox *datesGroup;
	QLabel *QLabel_2;
	QLabel *QLabel_3;
	QCheckBox *dateRadio;
	QCheckBox *amountRadio;
	QGroupBox *amountGroup;
	QComboBox *amountCombo;
	QCheckBox *creditRadio;
	QComboBox *creditCombo;
	QCheckBox *statusRadio;
	QComboBox *statusCombo;
	QCheckBox *descriptionRadio;
	QLineEdit *descriptionEdit;
	QCheckBox *numberRadio;
	QLineEdit *numberEdit;
	QPushButton *searchBtn;
	QPushButton *closeBtn;
	QCheckBox *descriptionRegExpCheck;
	QCheckBox *numberRegExpCheck;
	//Generated area. DO NOT EDIT!!!(end)
*/
private:
//  kMyMoneyEdit *moneyEdit;
//  kMyMoneyDateInput *startDateInput;
//  kMyMoneyDateInput *endDateInput;
};

#endif
