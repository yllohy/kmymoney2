/***************************************************************************
                          knewbankdlg.cpp
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

#include <qpixmap.h>

#include <kmessagebox.h>

#include <qpushbutton.h>
#include <qlineedit.h>
#include <qlabel.h>

#include "knewbankdlg.h"

KNewBankDlg::KNewBankDlg(QWidget *parent, const char *name)
  : KNewBankDlgDecl(parent,name,true)
{
  QString filename = KGlobal::dirs()->findResource("appdata", "pics/dlg_new_institution.png");
  QPixmap pm(filename);
  m_qpixmaplabel->setPixmap(pm);

  nameEdit->setFocus();
	connect(okBtn, SIGNAL(clicked()), SLOT(okClicked()));
	connect(cancelBtn, SIGNAL(clicked()), SLOT(reject()));
}

KNewBankDlg::KNewBankDlg(QString b_name, QString b_sortCode, QString b_city,
  QString b_street, QString b_postcode, QString b_telephone, QString b_manager,
  QString title, QWidget *parent, const char *name)
  : KNewBankDlgDecl(parent, name, true)
{
  QString filename = KGlobal::dirs()->findResource("appdata", "pics/dlg_new_institution.png");
  QPixmap *pm = new QPixmap(filename);
  m_qpixmaplabel->setPixmap(*pm);
	setCaption(title);

	nameEdit->setText(b_name);
	streetEdit->setText(b_street);
	cityEdit->setText(b_city);
	postcodeEdit->setText(b_postcode);
	telephoneEdit->setText(b_telephone);
        sortCodeEdit->setText(b_sortCode);

	managerEdit->setText(b_manager);

  nameEdit->setFocus();
	connect(okBtn, SIGNAL(clicked()), SLOT(okClicked()));
	connect(cancelBtn, SIGNAL(clicked()), SLOT(reject()));
}

KNewBankDlg::~KNewBankDlg()
{
}

void KNewBankDlg::okClicked()
{
  if (nameEdit->text().isEmpty()) {
    KMessageBox::information(this, i18n("The institution name field is empty.  Please enter the name."), i18n("Adding New Institution"));
    nameEdit->setFocus();
    return;
  }

  m_name = nameEdit->text();
  m_city = cityEdit->text();
  m_street = streetEdit->text();
  m_postcode = postcodeEdit->text();
  m_telephone = telephoneEdit->text();
  m_managerName = managerEdit->text();
  m_sortCode = sortCodeEdit->text();
  accept();
}
