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
class GeneralLoanInfoPage;
class LoanDetailsPage;
class LoanPaymentPage;
class LoanSchedulePage;
class LoanPayoutPage;

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
  friend class GeneralLoanInfoPage;
  friend class LoanDetailsPage;
  friend class LoanPaymentPage;
  friend class LoanSchedulePage;
  friend class LoanPayoutPage;

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

  /**
   * This method returns the interest rate as factor, ie an
   * interest rate of 6.5% will be returned as 0.065
   */
  MyMoneyMoney interestRate(void) const;

protected:
  /**
   * This method returns the currently selected currency for the account
   */
  const MyMoneySecurity& currency(void) const;

  /**
   * This method returns information about the selection of the user
   * if the loan is for borrowing or lending money.
   *
   * @retval true loan is for money borrowed
   * @retval false loan is for money lent
   */
  bool moneyBorrowed() const;

signals:
  void newInstitutionClicked(MyMoneyInstitution& institution);

private:
  InstitutionPage*         m_institutionPage;
  AccountTypePage*         m_accountTypePage;
  OpeningPage*             m_openingPage;
  CreditCardSchedulePage*  m_schedulePage;
  GeneralLoanInfoPage*     m_generalLoanInfoPage;
  LoanDetailsPage*         m_loanDetailsPage;
  LoanPaymentPage*         m_loanPaymentPage;
  LoanSchedulePage*        m_loanSchedulePage;
  LoanPayoutPage*          m_loanPayoutPage;

  MyMoneyAccount           m_account;
  MyMoneySchedule          m_schedule;
};

}; // namespace


#endif
