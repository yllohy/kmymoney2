/***************************************************************************
                             knewuserwizard.cpp
                             -------------------
    begin                : Sat Feb 18 2006
    copyright            : (C) 2006 Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "knewuserwizard.h"
#include "knewuserwizard_p.h"

using namespace NewUserWizard;

WizardPage::WizardPage(unsigned int step, QWidget* widget, Wizard* parent, const char* name) :
  KMyMoneyWizardPage(step, widget, name),
  m_wizard(parent)
{
}

Wizard::Wizard(QWidget *parent, const char *name, bool modal, WFlags flags)
  : KMyMoneyWizard(parent, name, modal, flags)
{
  setTitle(i18n("KMyMoney New User Setup"));
  addStep(i18n("Personal Data"));
  addStep(i18n("Select Currency"));
  addStep(i18n("Set Password"));

  m_generalPage = new GeneralPage(this);
  m_currencyPage = new CurrencyPage(this);
  m_passwordPage = new PasswordPage(this);

  setFirstPage(m_generalPage);
}

GeneralPage::GeneralPage(Wizard* wizard, const char* name) :
  KGeneralPageDecl(wizard),
  WizardPage(1, this, wizard, name)
{
}

KMyMoneyWizardPage* GeneralPage::nextPage(void)
{
  return m_wizard->m_currencyPage;
}

CurrencyPage::CurrencyPage(Wizard* wizard, const char* name) :
  KCurrencyPageDecl(wizard),
  WizardPage(2, this, wizard, name)
{
}

KMyMoneyWizardPage* CurrencyPage::nextPage(void)
{
  return m_wizard->m_passwordPage;
}

PasswordPage::PasswordPage(Wizard* wizard, const char* name) :
  KPasswordPageDecl(wizard),
  WizardPage(3, this, wizard, name)
{
}

#include "knewuserwizard.moc"
