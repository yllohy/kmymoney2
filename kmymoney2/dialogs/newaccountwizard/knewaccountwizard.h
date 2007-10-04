/***************************************************************************
                             knewaccountwizard.h
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

#ifndef KNEWACCOUNTWIZARD_H
#define KNEWACCOUNTWIZARD_H

// ----------------------------------------------------------------------------
// QT Includes

class QString;

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/kmymoneywizard.h>

/**
  * @author Thomas Baumgart
  */
namespace NewAccountWizard {

class AccountTypePage;
class InstitutionPage;
class OpeningPage;
class CreditCardSchedulePage;

/**
  * @author Thomas Baumgart
  *
  * This class implements the new account wizard which is used to gather
  * the required information from the user to create a new account
  */
class Wizard : public KMyMoneyWizard
{
  friend class AccountTypePage;
  friend class InstitutionPage;
  friend class OpeningPage;
  friend class CreditCardSchedulePage;

  Q_OBJECT
public:
  Wizard(QWidget* parent = 0, const char* name = 0, bool modal = false, WFlags flags = 0);

  /**
    * Returns the information about the account as entered by
    * the user.
    */
  const MyMoneyAccount& account(void);

  /**
   * Method to load the generated account information back into the widget
   */
  void setAccount(const MyMoneyAccount& acc);

  /**
    * Returns the information about the parent account as entered by
    * the user.
    * @note For now it's either fixed as Asset or Liability. We will provide
    * user selected parent accounts later.
    */
  const MyMoneyAccount& parentAccount(void);

  /**
   * Returns information about the schedule. If the returned value
   * equals MyMoneySchedule() then the user did not select to create
   * a schedule.
   */
  const MyMoneySchedule& schedule(void);

  /**
   * This method returns the value of the opening balance
   * entered by the user
   */
  MyMoneyMoney openingBalance(void);

signals:
  void newInstitutionClicked(MyMoneyInstitution& institution);

private:
  InstitutionPage*         m_institutionPage;
  AccountTypePage*         m_accountTypePage;
  OpeningPage*             m_openingPage;
  CreditCardSchedulePage*  m_schedulePage;

  MyMoneyAccount           m_account;
  MyMoneySchedule          m_schedule;
};

}; // namespace


#endif
