/***************************************************************************
                             knewaccountwizard_p.h
                             -------------------
    begin                : Tue Sep 25 2007
    copyright            : (C) 2007 Thomas Baumgart
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

#ifndef KNEWACCOUNTWIZARD_P_H
#define KNEWACCOUNTWIZARD_P_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qcheckbox.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kcombobox.h>
#include <klineedit.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/kmymoneywizard.h>
#include <kmymoney/kmymoneydateinput.h>
#include <kmymoney/kmymoneycurrencyselector.h>
#include <kmymoney/mymoneyaccount.h>

#include "kinstitutionpagedecl.h"
#include "kaccounttypepagedecl.h"
#include "kopeningpagedecl.h"
#include "kschedulepagedecl.h"


class Wizard;
class kMandatoryFieldGroup;
class MyMoneyInstitution;

namespace NewAccountWizard {

class InstitutionPagePrivate;

class InstitutionPage : public KInstitutionPageDecl, public WizardPage<Wizard>
{
  Q_OBJECT
public:
  InstitutionPage(Wizard* parent, const char* name = 0);
  ~InstitutionPage();
  KMyMoneyWizardPage* nextPage(void) const;

  QWidget* initialFocusWidget(void) const { return m_institutionComboBox; }

  /**
    * Returns the information about an institution if entered by
    * the user. If the id field is empty, then he did not enter
    * such information.
    */
  const MyMoneyInstitution& institution(void) const;

private slots:
  void slotLoadWidget(void);
  void slotNewInstitution(void);
  void slotSelectInstitution(int id);

private:
  InstitutionPagePrivate*  d;
};

class AccountTypePage : public KAccountTypePageDecl, public WizardPage<Wizard>
{
  Q_OBJECT
public:
  AccountTypePage(Wizard* parent, const char* name = 0);
  virtual bool isComplete(void) const;
  KMyMoneyWizardPage* nextPage(void) const;

  QWidget* initialFocusWidget(void) const { return m_accountName; }

  MyMoneyAccount::accountTypeE accountType(void) const;

private:
  kMandatoryFieldGroup* m_mandatoryGroup;
};

class OpeningPage : public KOpeningPageDecl, public WizardPage<Wizard>
{
  Q_OBJECT
public:
  OpeningPage(Wizard* parent, const char* name = 0);
  KMyMoneyWizardPage* nextPage(void) const;

  QWidget* initialFocusWidget(void) const { return m_currencyComboBox; }
  const QCString& currencyId(void) const;

private slots:
  void slotLoadWidget(void);
};

class CreditCardSchedulePage : public KSchedulePageDecl, public WizardPage<Wizard>
{
  Q_OBJECT
public:
  CreditCardSchedulePage(Wizard* parent, const char* name = 0);
  virtual bool isComplete(void) const;

  QWidget* initialFocusWidget(void) const { return m_reminderCheckBox; }

public slots:
    virtual void show(void);

private slots:
  void slotLoadWidget(void);

private:
  kMandatoryFieldGroup* m_mandatoryGroup;
};

} // namespace

#endif
