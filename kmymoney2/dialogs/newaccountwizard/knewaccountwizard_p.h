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
#include <kmymoney/kmymoneyedit.h>
#include <kmymoney/kmymoneycategory.h>

#include "kinstitutionpagedecl.h"
#include "kaccounttypepagedecl.h"
#include "kopeningpagedecl.h"
#include "kschedulepagedecl.h"
#include "kgeneralloaninfopagedecl.h"
#include "kloandetailspagedecl.h"
#include "kloanpaymentpagedecl.h"
#include "kloanschedulepagedecl.h"
#include "kloanpayoutpagedecl.h"
#include "kaccountsummarypagedecl.h"

class Wizard;
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
  void slotLoadWidgets(void);
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
  void leavePage(void);

  QWidget* initialFocusWidget(void) const { return m_accountName; }

  MyMoneyAccount::accountTypeE accountType(void) const;
  const MyMoneySecurity& currency(void) const;

  void setAccount(const MyMoneyAccount& acc);

private slots:
  void slotLoadWidgets(void);
};

class OpeningPage : public KOpeningPageDecl, public WizardPage<Wizard>
{
  Q_OBJECT
public:
  OpeningPage(Wizard* parent, const char* name = 0);
  KMyMoneyWizardPage* nextPage(void) const;
  void enterPage(void);

  QWidget* initialFocusWidget(void) const { return m_openingBalance; }
};

class CreditCardSchedulePage : public KSchedulePageDecl, public WizardPage<Wizard>
{
  Q_OBJECT
public:
  CreditCardSchedulePage(Wizard* parent, const char* name = 0);
  KMyMoneyWizardPage* nextPage(void) const;
  virtual bool isComplete(void) const;
  void enterPage(void);

  QWidget* initialFocusWidget(void) const { return m_reminderCheckBox; }

private slots:
  void slotLoadWidgets(void);
};



class GeneralLoanInfoPage : public KGeneralLoanInfoPageDecl, public WizardPage<Wizard>
{
  Q_OBJECT
public:
  GeneralLoanInfoPage(Wizard* parent, const char* name = 0);
  KMyMoneyWizardPage* nextPage(void) const;
  virtual bool isComplete(void) const;
  void enterPage(void);

  QWidget* initialFocusWidget(void) const { return m_loanDirection; }

  /**
   * Returns @p true if the user decided to record all payments, @p false otherwise.
   */
  bool recordAllPayments(void) const;

private slots:
  void slotLoadWidgets(void);

private:
  bool      m_firstTime;
};

class LoanDetailsPage : public KLoanDetailsPageDecl, public WizardPage<Wizard>
{
  Q_OBJECT
public:
  LoanDetailsPage(Wizard* parent, const char* name = 0);
  KMyMoneyWizardPage* nextPage(void) const;
  virtual bool isComplete(void) const;

  QWidget* initialFocusWidget(void) const { return m_paymentDue; }

private:
  /**
   * This method returns the number of payments depending on
   * the settings of m_termAmount and m_termUnit widgets
   */
  int term(void) const;

  /**
   * This method is used to update the term widgets
   * according to the length of the given @a term.
   * The term is also converted into a string and returned.
   */
  QString updateTermWidgets(const long double term);

private:
  bool                m_needCalculate;

private slots:
  void slotValuesChanged(void);
  void slotCalculate(void);
};

class LoanPaymentPagePrivate;
class LoanPaymentPage : public KLoanPaymentPageDecl, public WizardPage<Wizard>
{
  Q_OBJECT
public:
  LoanPaymentPage(Wizard* parent, const char* name = 0);
  ~LoanPaymentPage();

  KMyMoneyWizardPage* nextPage(void) const;

  void enterPage(void);

  /**
   * This method returns the sum of the additional fees
   */
  MyMoneyMoney additionalFees(void) const;

  /**
   * This method returns the base payment, that's principal and interest
   */
  MyMoneyMoney basePayment(void) const;

  /**
   * This method returns the splits that make up the additional fees in @p list.
   * @note The splits may contain assigned ids which the caller must remove before
   * adding the splits to a MyMoneyTransaction object.
   */
  void additionalFeesSplits(QValueList<MyMoneySplit>& list);

protected slots:
  void slotAdditionalFees(void);

protected:
  void updateAmounts(void);

private:
  LoanPaymentPagePrivate* d;
};

class LoanSchedulePage : public KLoanSchedulePageDecl, public WizardPage<Wizard>
{
  Q_OBJECT
public:
  LoanSchedulePage(Wizard* parent, const char* name = 0);
  void enterPage(void);

  KMyMoneyWizardPage* nextPage(void) const;

  /**
   * This method returns the due date of the first payment to be recorded.
   */
  QDate firstPaymentDueDate(void) const;

private slots:
  void slotLoadWidgets(void);
  void slotCreateCategory(const QString& name, QCString& id);
};


class LoanPayoutPage : public KLoanPayoutPageDecl, public WizardPage<Wizard>
{
  Q_OBJECT
public:
  LoanPayoutPage(Wizard* parent, const char* name = 0);
  void enterPage(void);
  virtual bool isComplete(void) const;

  KMyMoneyWizardPage* nextPage(void) const;

private slots:
  void slotLoadWidgets(void);
  void slotCreateAssetAccount(void);
  void slotButtonsToggled(void);
};

class AccountSummaryPage : public KAccountSummaryPageDecl, public WizardPage<Wizard>
{
  Q_OBJECT
public:
  AccountSummaryPage(Wizard* parent, const char* name = 0);
  void enterPage(void);
};

} // namespace

#endif
