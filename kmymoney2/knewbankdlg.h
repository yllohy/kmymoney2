/***************************************************************************
                          knewbankdlg.h
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

#ifndef KNEWBANKDLG_H
#define KNEWBANKDLG_H
/*
//Generated area. DO NOT EDIT!!!(begin)
#include <qwidget.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
//Generated area. DO NOT EDIT!!!(end)
*/
#include <klocale.h>
#include <qdialog.h>

#include <mymoney/mymoneymoney.h>

#include "knewbankdlgdecl.h"

// This dialog lets the user create or edit
// a bank.
// Use the second constructor to edit the bank.
class KNewBankDlg : public KNewBankDlgDecl  {
   Q_OBJECT

public:
	KNewBankDlg(QWidget *parent=0, const char *name=0);
  KNewBankDlg(QString b_name, QString b_sortCode, QString b_city,
    QString b_street, QString b_postcode, QString b_telephone, QString b_manager,
    QString title, QWidget *parent=0, const char *name=0);
	~KNewBankDlg();

	QString m_name;
	QString m_street;
	QString m_city;
	QString m_postcode;
	QString m_telephone;
	QString m_managerName;
        QString m_sortCode;

protected slots:
  void okClicked();

protected: 
/*
	void initDialog();
	//Generated area. DO NOT EDIT!!!(begin)
	QGroupBox *QGroupBox_1;
	QLabel *QLabel_7;
	QGroupBox *QGroupBox_2;
	QLabel *QLabel_5;
	QLineEdit *nameEdit;
	QLineEdit *cityEdit;
	QLineEdit *streetEdit;
	QLineEdit *postcodeEdit;
	QLineEdit *telephoneEdit;
	QLineEdit *managerEdit;
	QPushButton *cancelBtn;
	QPushButton *okBtn;
	QLabel *QLabel_1;
	QLabel *QLabel_3;
	QLabel *QLabel_4;
	QLabel *QLabel_6;
	//Generated area. DO NOT EDIT!!!(end)
*/
};

#endif
