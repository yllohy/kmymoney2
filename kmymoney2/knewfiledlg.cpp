/***************************************************************************
                          knewfiledlg.cpp
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
#include <kmessagebox.h>
#include <qpushbutton.h>
#include <qlineedit.h>

#include "knewfiledlg.h"

KNewFileDlg::KNewFileDlg(QWidget *parent, const char *name, const char *title,
  const char *okName)
  : KNewFileDlgDecl(parent,name,true)
{
//	initDialog();
	
	okBtn->setText(okName);
	if (title)
	  setCaption(title);
	
	connect(cancelBtn, SIGNAL(clicked()), this, SLOT(reject()));
	connect(okBtn, SIGNAL(clicked()), this, SLOT(okClicked()));
}

KNewFileDlg::KNewFileDlg(QString a_name, QString userName, QString userStreet,
  QString userTown, QString userCounty, QString userPostcode, QString userTelephone,
  QString userEmail, QWidget *parent, const char *name, const char *title, const char *okName)
  : KNewFileDlgDecl(parent,name,true)
{
//	initDialog();
	
	nameEdit->setText(a_name);
  userNameEdit->setText(userName);
  streetEdit->setText(userStreet);
  townEdit->setText(userTown);
  countyEdit->setText(userCounty);
  postcodeEdit->setText(userPostcode);
  telephoneEdit->setText(userTelephone);
  emailEdit->setText(userEmail);
  okBtn->setText(okName);

	if (title)
	  setCaption(title);
	
	connect(cancelBtn, SIGNAL(clicked()), this, SLOT(reject()));
	connect(okBtn, SIGNAL(clicked()), this, SLOT(okClicked()));
}

KNewFileDlg::~KNewFileDlg(){
}

void KNewFileDlg::okClicked()
{
  nameText = nameEdit->text();
  if (nameText.isEmpty()) {
    KMessageBox::error(this, i18n("You have not specified a name.\n\nPlease fill in this field"));
    nameEdit->setFocus();
    return;
  }
  userNameText = userNameEdit->text();
  userStreetText = streetEdit->text();
  userTownText += townEdit->text();
  userCountyText += countyEdit->text();
  userPostcodeText += postcodeEdit->text();
  userTelephoneText += telephoneEdit->text();
  userEmailText = emailEdit->text();

  accept();
}

//#include "knewfiledlg.moc"
