/***************************************************************************
                          knewfiledlg.h
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

#ifndef KNEWFILEDLG_H
#define KNEWFILEDLG_H
/*
//Generated area. DO NOT EDIT!!!(begin)
#include <qwidget.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qgroupbox.h>
#include <qpushbutton.h>
//Generated area. DO NOT EDIT!!!(end)
*/
#include <qdialog.h>
#include <klocale.h>

#include "knewfiledlgdecl.h"

// This dialog lets the user create/edit a file.
// Use the second constructor to edit a file.
class KNewFileDlg : public KNewFileDlgDecl  {
   Q_OBJECT
public: 
	KNewFileDlg(QWidget *parent=0, const char *name=0, const char *title=0,
	  const char *okName="Create");
  KNewFileDlg(QString a_name, QString userName, QString userStreet,
    QString userTown, QString userCounty, QString userPostcode, QString userTelephone,
    QString userEmail, QWidget *parent=0, const char *name=0, const char *title=0,
    const char *okName="Create");
	~KNewFileDlg();

protected: 
/*
	void initDialog();
	//Generated area. DO NOT EDIT!!!(begin)
	QLabel *QLabel_1;
	QLineEdit *userNameEdit;
	QGroupBox *QGroupBox_1;
	QLabel *QLabel_4;
	QLabel *QLabel_5;
	QLabel *QLabel_6;
	QLabel *QLabel_7;
	QLabel *QLabel_8;
	QLineEdit *streetEdit;
	QLineEdit *townEdit;
	QLineEdit *countyEdit;
	QLineEdit *postcodeEdit;
	QLineEdit *telephoneEdit;
	QLabel *QLabel_9;
	QLineEdit *emailEdit;
	QLineEdit *nameEdit;
	QLabel *QLabel_3;
	QPushButton *okBtn;
	QPushButton *cancelBtn;
	//Generated area. DO NOT EDIT!!!(end)
*/	
public:
	QString userNameText;
	QString nameText;
  QString userStreetText;
  QString userTownText;
  QString userCountyText;
  QString userPostcodeText;
  QString userTelephoneText;
  QString userEmailText;

protected slots:
  void okClicked();

};

#endif
