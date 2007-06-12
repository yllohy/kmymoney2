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

#include "kaccountpagedecl.h"
#include "kpreferencepagedecl.h"
#include "kfilepagedecl.h"

#include "../wizardpages/userinfo.h"
#include "../wizardpages/currency.h"
#include "../wizardpages/accounts.h"

#include <kmymoney/mymoneytemplate.h>

class Wizard;
class kMandatoryFieldGroup;

namespace NewUserWizard {

/**
  * The first page of the new user wizard
  *
  * @author Thomas Baumgart
  */
class GeneralPage : public UserInfo, public WizardPage<Wizard>
{
  Q_OBJECT
public:
  GeneralPage(Wizard* parent, const char* name = 0);
  KMyMoneyWizardPage* nextPage(void) const;
};

/**
  * The second page of the new user wizard
  *
  * @author Thomas Baumgart
  */
class CurrencyPage : public Currency, public WizardPage<Wizard>
{
  Q_OBJECT
public:
  CurrencyPage(Wizard* parent, const char* name = 0);
  KMyMoneyWizardPage* nextPage(void) const;
};

/**
  * The third page of the new user wizard collecting information
  * about the checking account
  */
class AccountPage : public KAccountPageDecl, public WizardPage<Wizard>
{
  Q_OBJECT
public:
  AccountPage(Wizard* parent, const char* name = 0);
  KMyMoneyWizardPage* nextPage(void) const;

  virtual bool isComplete(void) const;

private:
  kMandatoryFieldGroup*  m_mandatoryGroup;
};

/**
  * The fourth page of the new user wizard collecting information
  * about the account templates.
  *
  * @author Thomas Baumgart
  */
class CategoriesPage : public Accounts, public WizardPage<Wizard>
{
  Q_OBJECT
public:
  CategoriesPage(Wizard* parent, const char* name = 0);
  KMyMoneyWizardPage* nextPage(void) const;

protected:
  void loadTemplateList(void);
  QListViewItem* hierarchyItem(const QString& parent, const QString& name);

protected slots:
  void slotLoadHierarchy(void);

private:
  QMap<QString, MyMoneyTemplate>    m_templates;
  QMap<QString, QListViewItem*>     m_templateHierarchy;
};

/**
  * Wizard page to allow changing the preferences during setup
  *
  * @author Thomas Baumgart
  */
class PreferencePage : public KPreferencePageDecl, public WizardPage<Wizard>
{
  Q_OBJECT
public:
  PreferencePage(Wizard* parent, const char* name = 0);
  KMyMoneyWizardPage* nextPage(void) const;
};

/**
  * Wizard page to allow selecting the filename
  *
  * @author Thomas Baumgart
  */
class FilePage : public KFilePageDecl, public WizardPage<Wizard>
{
  Q_OBJECT
public:
  FilePage(Wizard* parent, const char* name = 0);

  virtual bool isComplete(void) const;

private:
  kMandatoryFieldGroup*  m_mandatoryGroup;
};

} // namespace

#endif
