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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

// ----------------------------------------------------------------------------
// QT Includes

#include <qlineedit.h>
#include <qlabel.h>
#include <qpixmap.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kglobal.h>
#include <klocale.h>
#if QT_VERSION > 300
#include <kstandarddirs.h>
#else
#include <kstddirs.h>
#endif

#include <kmessagebox.h>
#include <kpushbutton.h>
#include <kiconloader.h>
#include <kguiitem.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "knewbankdlg.h"
#include "kadvancedbanksettingsdlg.h"

KNewBankDlg::KNewBankDlg(MyMoneyInstitution& institution,  bool /*isEditing*/, QWidget *parent, const char *name)
  : KNewBankDlgDecl(parent,name,true), m_institution(institution)
{
  KIconLoader* il = KGlobal::iconLoader();
  KGuiItem okButtenItem( i18n("&Ok" ),
                    QIconSet(il->loadIcon("button_ok", KIcon::Small, KIcon::SizeSmall)),
                    i18n("Accepts the entered data and stores it"),
                    i18n("Use this to accept the data."));
  okBtn->setGuiItem(okButtenItem);

  KGuiItem cancelButtenItem( i18n( "&Cancel" ),
                    QIconSet(il->loadIcon("button_cancel", KIcon::Small, KIcon::SizeSmall)),
                    i18n("Reject all changes to the data and closes the dialog"),
                    i18n("Use this to reject all changes."));
  cancelBtn->setGuiItem(cancelButtenItem);

  nameEdit->setFocus();
	nameEdit->setText(institution.name());
	cityEdit->setText(institution.city());
	streetEdit->setText(institution.street());
	postcodeEdit->setText(institution.postcode());
	telephoneEdit->setText(institution.telephone());
	managerEdit->setText(institution.manager());
	sortCodeEdit->setText(institution.sortcode());

  m_ofxSettings = m_institution.ofxConnectionSettings();
                
#ifndef USE_OFX_DIRECTCONNECT
  // the 'advanced' button will take the user to set up their OFX Direct Connect
  // settings, which is only available if KMM is compiled with the new OFX libraries.
  advancedBtn->hide();
#endif

  connect(okBtn, SIGNAL(clicked()), SLOT(okClicked()));
  connect(cancelBtn, SIGNAL(clicked()), SLOT(reject()));
  connect(advancedBtn, SIGNAL(clicked()), SLOT(advancedClicked()));
  connect(nameEdit, SIGNAL(textChanged ( const QString & )), SLOT(institutionNameChanged( const QString &)));
  institutionNameChanged( nameEdit->text());
}

void KNewBankDlg::institutionNameChanged( const QString &_text)
{
	okBtn->setEnabled( !_text.isEmpty() );
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

	m_institution.setName(nameEdit->text());
	m_institution.setTown(cityEdit->text());
	m_institution.setStreet(streetEdit->text());
	m_institution.setPostcode(postcodeEdit->text());
	m_institution.setTelephone(telephoneEdit->text());
	m_institution.setManager(managerEdit->text());
	m_institution.setSortcode(sortCodeEdit->text());

        m_institution.setOfxConnectionSettings(m_ofxSettings);
  accept();
}

void KNewBankDlg::advancedClicked()
{
  // bring up the Advanced Bank Settings dialog
  KAdvancedBankSettingsDlg dlg(this,"advancedbanksettings");
  dlg.setValues(m_ofxSettings);
  if ( dlg.exec() )
    m_ofxSettings = dlg.values();
}

MyMoneyInstitution KNewBankDlg::institution(void)
{
  return m_institution;
}


#include "knewbankdlg.moc"
