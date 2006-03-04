/***************************************************************************
                             knewuserwizard_p.h
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

#ifndef KNEWUSERWIZARD_P_H
#define KNEWUSERWIZARD_P_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/kmymoneywizard.h>

#include "kgeneralpagedecl.h"
#include "kcurrencypagedecl.h"
#include "kpasswordpagedecl.h"

/**
  * @author Thomas Baumgart
  */

namespace NewUserWizard {

class Wizard;

class WizardPage : public KMyMoneyWizardPage
{
public:
  WizardPage(unsigned int step, QWidget* widget, Wizard* parent, const char* name);

protected:
  Wizard*    m_wizard;
};

/**
  * The first page of the new user wizard
  */
class GeneralPage : public KGeneralPageDecl, public WizardPage
{
  Q_OBJECT
public:
  GeneralPage(Wizard* parent, const char* name = 0);
  KMyMoneyWizardPage* nextPage(void);
};

/**
  * The second page of the new user wizard
  */
class CurrencyPage : public KCurrencyPageDecl, public WizardPage
{
  Q_OBJECT
public:
  CurrencyPage(Wizard* parent, const char* name = 0);
  KMyMoneyWizardPage* nextPage(void);
};

/**
  * The third page of the new user wizard
  */
class PasswordPage : public KPasswordPageDecl, public WizardPage
{
  Q_OBJECT
public:
  PasswordPage(Wizard* parent, const char* name = 0);
};

} // namespace

#endif
