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

class NewUserWizard;

namespace NewUserWizardPages {

/**
  * The first page of the new user wizard
  *
  * @author Thomas Baumgart
  */
class GeneralPage : public KGeneralPageDecl, public WizardPage<NewUserWizard>
{
  Q_OBJECT
public:
  GeneralPage(NewUserWizard* parent, const char* name = 0);
  KMyMoneyWizardPage* nextPage(void);
};

/**
  * The second page of the new user wizard
  *
  * @author Thomas Baumgart
  */
class CurrencyPage : public KCurrencyPageDecl, public WizardPage<NewUserWizard>
{
  Q_OBJECT
public:
  CurrencyPage(NewUserWizard* parent, const char* name = 0);
  KMyMoneyWizardPage* nextPage(void);
};

/**
  * The third page of the new user wizard
  *
  * @author Thomas Baumgart
  */
class PasswordPage : public KPasswordPageDecl, public WizardPage<NewUserWizard>
{
  Q_OBJECT
public:
  PasswordPage(NewUserWizard* parent, const char* name = 0);
};

} // namespace

#endif
