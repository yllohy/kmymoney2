/***************************************************************************
                          knewfiledlg.cpp
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
#include <kglobal.h>
#include <klocale.h>
#if QT_VERSION > 300
#include <kstandarddirs.h>
#else
#include <kstddirs.h>
#endif
#include <kstdguiitem.h>
#include <kpushbutton.h>

#include <qpixmap.h>
#include <kmessagebox.h>
#include <qlineedit.h>
#include <qlabel.h>

#include "knewfiledlg.h"

KNewFileDlg::KNewFileDlg(QWidget *parent, const char *name, const char *title)
  : KNewFileDlgDecl(parent,name,true)
{
  okBtn->setGuiItem(KStdGuiItem::ok());
  cancelBtn->setGuiItem(KStdGuiItem::cancel());

  QString filename = KGlobal::dirs()->findResource("appdata", "pics/dlg_edit_file.png");
  QPixmap pm(filename);
  m_qpixmaplabel->setPixmap(pm);

	if (title)
	  setCaption(title);

	userNameEdit->setFocus();

	connect(cancelBtn, SIGNAL(clicked()), this, SLOT(reject()));
	connect(okBtn, SIGNAL(clicked()), this, SLOT(okClicked()));
}

KNewFileDlg::KNewFileDlg(QString userName, QString userStreet,
  QString userTown, QString userCounty, QString userPostcode, QString userTelephone,
  QString userEmail, QWidget *parent, const char *name, const char *title)
  : KNewFileDlgDecl(parent,name,true)
{
  okBtn->setGuiItem(KStdGuiItem::ok());
  cancelBtn->setGuiItem(KStdGuiItem::cancel());

  QString filename = KGlobal::dirs()->findResource("appdata", "pics/view_info.png");
  m_qpixmaplabel->setPixmap(QPixmap(filename));

  userNameEdit->setText(userName);
  streetEdit->setText(userStreet);
  townEdit->setText(userTown);
  countyEdit->setText(userCounty);
  postcodeEdit->setText(userPostcode);
  telephoneEdit->setText(userTelephone);
  emailEdit->setText(userEmail);

	if (title)
	  setCaption(title);

	userNameEdit->setFocus();

	connect(cancelBtn, SIGNAL(clicked()), this, SLOT(reject()));
	connect(okBtn, SIGNAL(clicked()), this, SLOT(okClicked()));
}

KNewFileDlg::~KNewFileDlg(){
}

void KNewFileDlg::okClicked()
{
  userNameText = userNameEdit->text();
  userStreetText = streetEdit->text();
  userTownText = townEdit->text();
  userCountyText = countyEdit->text();
  userPostcodeText = postcodeEdit->text();
  userTelephoneText = telephoneEdit->text();
  userEmailText = emailEdit->text();

  accept();
}
