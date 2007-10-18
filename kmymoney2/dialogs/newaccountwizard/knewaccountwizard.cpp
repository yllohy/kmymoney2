/***************************************************************************
                             knewaccountwizard.cpp
                             -------------------
    begin                : Tue Sep 25 2006
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

#include <locale.h>

// ----------------------------------------------------------------------------
// QT Includes

#include <qcheckbox.h>
#include <qfocusdata.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>
#include <klistbox.h>
#include <klineedit.h>
#include <kmessagebox.h>
#include <knuminput.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/kmymoneycombo.h>
#include <kmymoney/kmymoneyedit.h>
#include <kmymoney/kmymoneycategory.h>
#include <kmymoney/mymoneyfile.h>
#include <kmymoney/kmymoneycurrencyselector.h>
#include <kmymoney/kmymoneyaccountselector.h>
#include <kmymoney/mymoneyfinancialcalculator.h>

#include "knewaccountwizard.h"
#include "knewaccountwizard_p.h"
#include <kmymoney/kguiutils.h>

#include "../ksplittransactiondlg.h"
#include "../../kmymoney2.h"

using namespace NewAccountWizard;

NewAccountWizard::Wizard::Wizard(QWidget *parent, const char *name, bool modal, WFlags flags)
  : KMyMoneyWizard(parent, name, modal, flags)
{
  setTitle(i18n("KMyMoney New Account Setup"));
  addStep(i18n("Institution"));
  addStep(i18n("Type"));
  addStep(i18n("Details"));
  addStep(i18n("Schedule"));
  setStepHidden(4);
  addStep(i18n("Finish"));

  m_institutionPage = new InstitutionPage(this);
  m_accountTypePage = new AccountTypePage(this);
  m_openingPage = new OpeningPage(this);
  m_schedulePage = new CreditCardSchedulePage(this);
  m_generalLoanInfoPage = new GeneralLoanInfoPage(this);
  m_loanDetailsPage = new LoanDetailsPage(this);
  m_loanPaymentPage = new LoanPaymentPage(this);
  m_loanSchedulePage = new LoanSchedulePage(this);
  m_loanPayoutPage = new LoanPayoutPage(this);

  setFirstPage(m_institutionPage);
}

void NewAccountWizard::Wizard::setAccount(const MyMoneyAccount& acc)
{
  m_account = acc;
}

const MyMoneySecurity& NewAccountWizard::Wizard::currency(void) const
{
  return m_accountTypePage->currency();
}

MyMoneyMoney NewAccountWizard::Wizard::interestRate(void) const
{
  return m_loanDetailsPage->m_interestRate->value() / MyMoneyMoney(100,1);
}

const MyMoneyAccount& NewAccountWizard::Wizard::account(void)
{
  m_account = MyMoneyAccount();
  m_account.setName(m_accountTypePage->m_accountName->text());
  m_account.setOpeningDate(m_accountTypePage->m_openingDate->date());
  m_account.setAccountType(m_accountTypePage->accountType());
  m_account.setInstitutionId(m_institutionPage->institution().id());
  m_account.setNumber(m_institutionPage->m_accountNumber->text());
  m_account.setValue("IBAN", m_institutionPage->m_iban->text());

  m_account.setCurrencyId(currency().id());

  if(m_account.accountType() == MyMoneyAccount::Loan) {
    KMessageBox::error(this, QString("This intermediate version of KMyMoney cannot create loan accounts. We are currently moving from an old code base to new functionality and this part has not yet ported over. Sorry, but we lack some time."));
    m_account = MyMoneyAccount();
  }

  return m_account;
}

const MyMoneyAccount& NewAccountWizard::Wizard::parentAccount(void)
{
  switch(m_accountTypePage->accountType()) {
    case MyMoneyAccount::CreditCard:
    case MyMoneyAccount::Loan:
    case MyMoneyAccount::Liability:
      return MyMoneyFile::instance()->liability();
      break;
    default:
      break;
  }
  return MyMoneyFile::instance()->asset();
}

const MyMoneySchedule& NewAccountWizard::Wizard::schedule(void)
{
  m_schedule = MyMoneySchedule();

  if(m_schedulePage->m_reminderCheckBox->isChecked()
  && m_account.accountType() == MyMoneyAccount::CreditCard
  && !m_account.id().isEmpty()) {
    m_schedule.setName(m_schedulePage->m_name->text());
    m_schedule.setType(MyMoneySchedule::TYPE_TRANSFER);
    m_schedule.setPaymentType(static_cast<MyMoneySchedule::paymentTypeE>(m_schedulePage->m_method->currentItem()));
    m_schedule.setFixed(false);
    m_schedule.setOccurence(MyMoneySchedule::OCCUR_MONTHLY);
    MyMoneyTransaction t;
    MyMoneySplit s;
    s.setPayeeId(m_schedulePage->m_payee->selectedItem());
    s.setAccountId(m_schedulePage->m_paymentAccount->selectedItem());
    s.setMemo(i18n("Credit card payment"));
    s.setShares(-m_schedulePage->m_amount->value());
    s.setValue(s.shares());
    t.addSplit(s);

    s.clearId();
    s.setAccountId(m_account.id());
    s.setShares(m_schedulePage->m_amount->value());
    s.setValue(s.shares());
    t.addSplit(s);

    // setup the next due date
    t.setPostDate(m_schedulePage->m_date->date());
    m_schedule.setTransaction(t);
  }
  return m_schedule;
}

MyMoneyMoney NewAccountWizard::Wizard::openingBalance(void)
{
  return m_openingPage->m_openingBalance->value();
}

bool NewAccountWizard::Wizard::moneyBorrowed(void) const
{
  return m_generalLoanInfoPage->m_loanDirection->currentItem() == 0;
}

class NewAccountWizard::InstitutionPagePrivate
{
public:
  InstitutionPagePrivate() {}
  QValueList<MyMoneyInstitution>  m_list;
};

InstitutionPage::InstitutionPage(Wizard* wizard, const char* name) :
  KInstitutionPageDecl(wizard),
  WizardPage<Wizard>(1, this, wizard, name),
  d(new InstitutionPagePrivate())
{
  connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotLoadWidgets()));
  connect(m_newInstitutionButton, SIGNAL(clicked()), this, SLOT(slotNewInstitution()));
  connect(m_institutionComboBox, SIGNAL(activated(int)), this, SLOT(slotSelectInstitution(int)));

  slotLoadWidgets();
  m_institutionComboBox->setCurrentItem(0);
  slotSelectInstitution(0);
}

InstitutionPage::~InstitutionPage()
{
  delete d;
}

void InstitutionPage::slotLoadWidgets(void)
{
  m_institutionComboBox->clear();

  d->m_list.clear();
  MyMoneyFile::instance()->institutionList(d->m_list);
  qHeapSort(d->m_list);

  QValueList<MyMoneyInstitution>::const_iterator it_l;
  m_institutionComboBox->insertItem("");
  for(it_l = d->m_list.begin(); it_l != d->m_list.end(); ++it_l) {
    m_institutionComboBox->insertItem((*it_l).name());
  }
}

void InstitutionPage::slotNewInstitution(void)
{
  MyMoneyInstitution institution;

  emit m_wizard->newInstitutionClicked(institution);

  if(!institution.id().isEmpty()) {
    QValueList<MyMoneyInstitution>::const_iterator it_l;
    int i = 0;
    for(it_l = d->m_list.begin(); it_l != d->m_list.end(); ++it_l) {
      if((*it_l).id() == institution.id()) {
        // select the item and remember that the very first one is the empty item
        m_institutionComboBox->setCurrentItem(i+1);
        slotSelectInstitution(i+1);
        m_accountNumber->setFocus();
        break;
      }
      ++i;
    }
  }
}

void InstitutionPage::slotSelectInstitution(int id)
{
  m_accountNumber->setEnabled(id != 0);
  m_iban->setEnabled(id != 0);
}

const MyMoneyInstitution& InstitutionPage::institution(void) const
{
  static MyMoneyInstitution emptyInstitution;
  if(m_institutionComboBox->currentItem() == 0)
    return emptyInstitution;

  return d->m_list[m_institutionComboBox->currentItem()-1];
}

KMyMoneyWizardPage* InstitutionPage::nextPage(void) const
{
  return m_wizard->m_accountTypePage;
}

AccountTypePage::AccountTypePage(Wizard* wizard, const char* name) :
  KAccountTypePageDecl(wizard),
  WizardPage<Wizard>(2, this, wizard, name)
{
  m_typeSelection->insertItem(i18n("Checking"), MyMoneyAccount::Checkings);
  m_typeSelection->insertItem(i18n("Savings"), MyMoneyAccount::Savings);
  m_typeSelection->insertItem(i18n("Credit Card"), MyMoneyAccount::CreditCard);
  m_typeSelection->insertItem(i18n("Cash"), MyMoneyAccount::Cash);
  m_typeSelection->insertItem(i18n("Loan"), MyMoneyAccount::Loan);
  m_typeSelection->insertItem(i18n("Investment"), MyMoneyAccount::Investment);
  m_typeSelection->insertItem(i18n("Asset"), MyMoneyAccount::Asset);
  m_typeSelection->insertItem(i18n("Liability"), MyMoneyAccount::Liability);

  m_typeSelection->setCurrentItem(MyMoneyAccount::Checkings);

  m_currencyComboBox->setSecurity(MyMoneyFile::instance()->baseCurrency());

  m_mandatoryGroup->add(m_accountName);
  connect(m_typeSelection, SIGNAL(highlighted(int)), object(), SIGNAL(completeStateChanged()));
  connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotLoadWidgets()));
}

KMyMoneyWizardPage* AccountTypePage::nextPage(void) const
{
  if(accountType() == MyMoneyAccount::Loan)
    return m_wizard->m_generalLoanInfoPage;
  return m_wizard->m_openingPage;
}

void AccountTypePage::slotLoadWidgets(void)
{
  m_currencyComboBox->update(QCString("x"));
}


void AccountTypePage::leavePage(void)
{
  m_wizard->setStepHidden(4);
}

bool AccountTypePage::isComplete(void) const
{
  bool rc = KMyMoneyWizardPage::isComplete();

  if(!rc) {
    QToolTip::add(m_wizard->m_nextButton, i18n("No account name supplied"));
  }

  bool hideSchedulePage = (accountType() != MyMoneyAccount::CreditCard)
                       && (accountType() != MyMoneyAccount::Loan);
  m_wizard->setStepHidden(4, hideSchedulePage);
  return rc;
}

MyMoneyAccount::accountTypeE AccountTypePage::accountType(void) const
{
  return static_cast<MyMoneyAccount::accountTypeE>(m_typeSelection->currentItem());
}

const MyMoneySecurity& AccountTypePage::currency(void) const
{
  return m_currencyComboBox->security();
}


OpeningPage::OpeningPage(Wizard* wizard, const char* name) :
  KOpeningPageDecl(wizard),
  WizardPage<Wizard>(3, this, wizard, name)
{
}

void OpeningPage::enterPage(void)
{
  m_openingDateDisplay->setText(KGlobal::locale()->formatDate(m_wizard->m_accountTypePage->m_openingDate->date()));
}

KMyMoneyWizardPage* OpeningPage::nextPage(void) const
{
  return (m_wizard->m_accountTypePage->accountType() == MyMoneyAccount::CreditCard) ? m_wizard->m_schedulePage : 0;
}

CreditCardSchedulePage::CreditCardSchedulePage(Wizard* wizard, const char* name) :
  KSchedulePageDecl(wizard),
  WizardPage<Wizard>(4, this, wizard, name)
{
  m_mandatoryGroup->add(m_name);
  m_mandatoryGroup->add(m_payee);
  m_mandatoryGroup->add(m_amount->lineedit());
  m_mandatoryGroup->add(m_paymentAccount);
  connect(m_paymentAccount, SIGNAL(itemSelected(const QCString&)), object(), SIGNAL(completeStateChanged()));
  connect(m_payee, SIGNAL(itemSelected(const QCString&)), object(), SIGNAL(completeStateChanged()));
  connect(m_date, SIGNAL(dateChanged(const QDate&)), object(), SIGNAL(completeStateChanged()));

  connect(m_payee, SIGNAL(createItem(const QString&, QCString&)), wizard, SIGNAL(createPayee(const QString&, QCString&)));

  m_reminderCheckBox->setChecked(true);
  connect(m_reminderCheckBox, SIGNAL(toggled(bool)), object(), SIGNAL(completeStateChanged()));

  connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotLoadWidgets()));

  m_method->insertItem(i18n("Other"), MyMoneySchedule::STYPE_OTHER);
  m_method->insertItem(i18n("Write check"), MyMoneySchedule::STYPE_WRITECHEQUE);
  m_method->insertItem(i18n("Manual deposit"), MyMoneySchedule::STYPE_MANUALDEPOSIT);
  m_method->insertItem(i18n("Direct deposit"), MyMoneySchedule::STYPE_DIRECTDEPOSIT);
  m_method->insertItem(i18n("Direct debit"), MyMoneySchedule::STYPE_DIRECTDEBIT);
  m_method->setCurrentItem(MyMoneySchedule::STYPE_DIRECTDEBIT);

  slotLoadWidgets();
}

void CreditCardSchedulePage::show(void)
{
  if(m_name->text().isEmpty())
    m_name->setText(i18n("CreditCard %1 monthly payment").arg(m_wizard->m_accountTypePage->m_accountName->text()));
  KSchedulePageDecl::show();
}

bool CreditCardSchedulePage::isComplete(void) const
{
  bool rc = true;
  QString msg = i18n("Finish entry and create account");
  if(m_reminderCheckBox->isChecked()) {
    msg = i18n("Finish entry and create account and schedule");
    if(m_date->date() > m_wizard->m_accountTypePage->m_openingDate->date()) {
      rc = false;
      msg = i18n("Next due date is prior to opening date");
    }
    if(m_paymentAccount->selectedItem().isEmpty()) {
      rc = false;
      msg = i18n("No account selected");
    }
    if(m_amount->lineedit()->text().isEmpty()) {
      rc = false;
      msg = i18n("No amount for payment selected");
    }
    if(m_payee->selectedItem().isEmpty()) {
      rc = false;
      msg = i18n("No payee for payment selected");
    }
    if(m_name->text().isEmpty()) {
      rc = false;
      msg = i18n("No name assigned for schedule");
    }
  }
  QToolTip::add(m_wizard->m_finishButton, msg);

  return rc;
}

void CreditCardSchedulePage::slotLoadWidgets(void)
{
  AccountSet set;
  set.addAccountGroup(MyMoneyAccount::Asset);
  set.load(m_paymentAccount->selector());

  m_payee->loadPayees(MyMoneyFile::instance()->payeeList());
}

GeneralLoanInfoPage::GeneralLoanInfoPage(Wizard* wizard, const char* name) :
  KGeneralLoanInfoPageDecl(wizard),
  WizardPage<Wizard>(3, this, wizard, name),
  m_firstTime(true)
{
  m_mandatoryGroup->add(m_payee);

  // remove the unsupported payment frequencies and setup default
  m_paymentFrequency->removeItem(MyMoneySchedule::OCCUR_ONCE);
  m_paymentFrequency->removeItem(MyMoneySchedule::OCCUR_EVERYOTHERYEAR);
  m_paymentFrequency->setCurrentItem(MyMoneySchedule::OCCUR_MONTHLY);

  slotLoadWidgets();

  connect(m_payee, SIGNAL(createItem(const QString&, QCString&)), wizard, SIGNAL(createPayee(const QString&, QCString&)));
  connect(m_anyPayments, SIGNAL(activated(int)), object(),  SIGNAL(completeStateChanged()));
  connect(m_recordings, SIGNAL(activated(int)), object(), SIGNAL(completeStateChanged()));

  connect(m_interestType, SIGNAL(activated(int)), object(),  SIGNAL(completeStateChanged()));
  connect(m_interestChangeDateEdit, SIGNAL(dateChanged(const QDate&)), object(),  SIGNAL(completeStateChanged()));
  connect(m_openingBalance, SIGNAL(textChanged(const QString&)), object(),  SIGNAL(completeStateChanged()));

  connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotLoadWidgets()));
}

KMyMoneyWizardPage* GeneralLoanInfoPage::nextPage(void) const
{
  return m_wizard->m_loanDetailsPage;
}

bool GeneralLoanInfoPage::recordAllPayments(void) const
{
  bool rc = true;     // all payments
  if(m_recordings->isEnabled()) {
    if(m_recordings->currentItem() != 0)
      rc = false;
  }
  return rc;
}

void GeneralLoanInfoPage::enterPage(void)
{
  if(m_firstTime) {
    // setup default dates to last of this month and one year on top
    QDate firstDay(QDate::currentDate().year(), QDate::currentDate().month(), 1);
    m_firstPaymentDate->setDate(firstDay.addMonths(1).addDays(-1));
    m_interestChangeDateEdit->setDate(m_firstPaymentDate->date().addYears(1));;
    m_firstTime = false;
  }
}

bool GeneralLoanInfoPage::isComplete(void) const
{
  bool rc = KMyMoneyWizardPage::isComplete();
  if(!rc) {
    QToolTip::add(m_wizard->m_nextButton, i18n("No payee supplied"));
  }

  // fixup availability of items on this page
  m_recordings->setDisabled(m_anyPayments->currentItem() == 0);

  m_interestFrequencyAmountEdit->setDisabled(m_interestType->currentItem() == 0);
  m_interestFrequencyUnitEdit->setDisabled(m_interestType->currentItem() == 0);
  m_interestChangeDateEdit->setDisabled(m_interestType->currentItem() == 0);

  m_openingBalance->setDisabled(recordAllPayments());

  if(m_openingBalance->isEnabled() && m_openingBalance->lineedit()->text().length() == 0) {
    rc = false;
    QToolTip::add(m_wizard->m_nextButton, i18n("No opening balance supplied"));
  }

  if(rc
  && (m_interestType->currentItem() != 0)
  && (m_interestChangeDateEdit->date() <= m_firstPaymentDate->date())) {
    rc = false;
    QToolTip::add(m_wizard->m_nextButton, i18n("An interest change can only happen after the first payment"));
  }
  return rc;
}

void GeneralLoanInfoPage::slotLoadWidgets(void)
{
  m_payee->loadPayees(MyMoneyFile::instance()->payeeList());
}

LoanDetailsPage::LoanDetailsPage(Wizard* wizard, const char* name) :
  KLoanDetailsPageDecl(wizard),
  WizardPage<Wizard>(3, this, wizard, name),
  m_needCalculate(true)
{
  // force the balloon payment to zero (default)
  m_balloonAmount->setValue(MyMoneyMoney());

  // we need to remove a bunch of entries of the payment frequencies
  m_termUnit->clear();
  m_termUnit->insertItem(i18n("Months"), MyMoneySchedule::OCCUR_MONTHLY);
  m_termUnit->insertItem(i18n("Years"), MyMoneySchedule::OCCUR_YEARLY);
  m_termUnit->insertItem(i18n("Payments"), MyMoneySchedule::OCCUR_ONCE);

  connect(m_paymentDue, SIGNAL(activated(int)), this, SLOT(slotValuesChanged()));

  connect(m_termAmount, SIGNAL(valueChanged(int)), this, SLOT(slotValuesChanged()));
  connect(m_termUnit, SIGNAL(highlighted(int)), this, SLOT(slotValuesChanged()));
  connect(m_loanAmount, SIGNAL(textChanged(const QString&)), this, SLOT(slotValuesChanged()));
  connect(m_interestRate, SIGNAL(textChanged(const QString&)), this, SLOT(slotValuesChanged()));
  connect(m_paymentAmount, SIGNAL(textChanged(const QString&)), this, SLOT(slotValuesChanged()));
  connect(m_balloonAmount, SIGNAL(textChanged(const QString&)), this, SLOT(slotValuesChanged()));

  connect(m_calculateButton, SIGNAL(clicked()), this, SLOT(slotCalculate()));
}

void LoanDetailsPage::slotValuesChanged(void)
{
  m_needCalculate = true;
  m_wizard->completeStateChanged();
}

void LoanDetailsPage::slotCalculate(void)
{
  MyMoneyFinancialCalculator calc;
  long double val;
  int PF;
  QString result;
  bool moneyBorrowed = m_wizard->moneyBorrowed();
  bool moneyLend = !moneyBorrowed;

  // FIXME: for now, we only support interest calculation at the end of the period
  calc.setBep();
  // FIXME: for now, we only support periodic compounding
  calc.setDisc();

  PF = m_wizard->m_generalLoanInfoPage->m_paymentFrequency->eventsPerYear();

  if(PF == 0)
    return;

  calc.setPF(PF);

  // FIXME: for now we only support compounding frequency == payment frequency
  calc.setCF(PF);


  if(!m_loanAmount->lineedit()->text().isEmpty()) {
    val = static_cast<long double> (m_loanAmount->value().abs().toDouble());
    if(moneyBorrowed)
      val = -val;
    calc.setPv(val);
  }

  if(!m_interestRate->lineedit()->text().isEmpty()) {
    val = static_cast<long double> (m_interestRate->value().abs().toDouble());
    calc.setIr(val);
  }

  if(!m_paymentAmount->lineedit()->text().isEmpty()) {
    val = static_cast<long double> (m_paymentAmount->value().abs().toDouble());
    if(moneyLend)
      val = -val;
    calc.setPmt(val);
  }

  if(!m_balloonAmount->lineedit()->text().isEmpty()) {
    val = static_cast<long double> (m_balloonAmount->value().abs().toDouble());
    if(moneyLend)
      val = -val;
    calc.setFv(val);
  }

  if(m_termAmount->value() != 0) {
    calc.setNpp(static_cast<long double>(term()));
  }

  // setup of parameters is done, now do the calculation
  try {
    if(m_loanAmount->lineedit()->text().isEmpty()) {
      // calculate the amount of the loan out of the other information
      val = calc.presentValue();
      m_loanAmount->loadText(MyMoneyMoney(static_cast<double>(val)).abs().formatMoney());
      result = i18n("KMyMoney has calculated the amount of the loan as %1.")
          .arg(m_loanAmount->lineedit()->text());

    } else if(m_interestRate->lineedit()->text().isEmpty()) {
      // calculate the interest rate out of the other information
      val = calc.interestRate();
      m_interestRate->loadText(MyMoneyMoney(static_cast<double>(val)).abs().formatMoney("", 3));
      result = i18n("KMyMoney has calculated the interest rate to %1%.")
          .arg(m_interestRate->lineedit()->text());

    } else if(m_paymentAmount->lineedit()->text().isEmpty()) {
      // calculate the periodical amount of the payment out of the other information
      val = calc.payment();
      m_paymentAmount->setValue(MyMoneyMoney(static_cast<double>(val)).abs());
      // reset payment as it might have changed due to rounding
      val = static_cast<long double> (m_paymentAmount->value().abs().toDouble());
      if(moneyLend)
        val = -val;
      calc.setPmt(val);

      result = i18n("KMyMoney has calculated a periodic payment of %1 to cover principal and interest.")
          .arg(m_paymentAmount->lineedit()->text());

      val = calc.futureValue();
      if((moneyBorrowed && val < 0 && fabsl(val) >= fabsl(calc.payment()))
          || (moneyLend && val > 0 && fabs(val) >= fabs(calc.payment()))) {
        calc.setNpp(calc.npp()-1);
        // updateTermWidgets(calc.npp());
        val = calc.futureValue();
        MyMoneyMoney refVal(static_cast<double>(val));
        m_balloonAmount->loadText(refVal.abs().formatMoney());
        result += QString(" ");
        result += i18n("The number of payments has been decremented and the balloon payment has been modified to %1.")
            .arg(m_balloonAmount->lineedit()->text());
          } else if((moneyBorrowed && val < 0 && fabsl(val) < fabsl(calc.payment()))
                     || (moneyLend && val > 0 && fabs(val) < fabs(calc.payment()))) {
                       m_balloonAmount->loadText(MyMoneyMoney(0,1).formatMoney());
                     } else {
                       MyMoneyMoney refVal(static_cast<double>(val));
                       m_balloonAmount->loadText(refVal.abs().formatMoney());
                       result += i18n("The balloon payment has been modified to %1.")
                           .arg(m_balloonAmount->lineedit()->text());
                     }

    } else if(m_termAmount->value() == 0) {
      // calculate the number of payments out of the other information
      val = calc.numPayments();
      if(val == 0)
        throw new MYMONEYEXCEPTION("incorrect fincancial calculation");

      // if the number of payments has a fractional part, then we
      // round it to the smallest integer and calculate the balloon payment
      result = i18n("KMyMoney has calculated the term of your loan as %1. ")
          .arg(updateTermWidgets(floorl(val)));

      if(val != floorl(val)) {
        calc.setNpp(floorl(val));
        val = calc.futureValue();
        MyMoneyMoney refVal(static_cast<double>(val));
        m_balloonAmount->loadText(refVal.abs().formatMoney());
        result += i18n("The balloon payment has been modified to %1.")
            .arg(m_balloonAmount->lineedit()->text());
      }

    } else {
      // calculate the future value of the loan out of the other information
      val = calc.futureValue();

      // we differentiate between the following cases:
      // a) the future value is greater than a payment
      // b) the future value is less than a payment or the loan is overpaid
      // c) all other cases
      //
      // a) means, we have paid more than we owed. This can't be
      // b) means, we paid more than we owed but the last payment is
      //    less in value than regular payments. That means, that the
      //    future value is to be treated as  (fully payed back)
      // c) the loan is not payed back yet
      if((moneyBorrowed && val < 0 && fabsl(val) > fabsl(calc.payment()))
      || (moneyLend && val > 0 && fabs(val) > fabs(calc.payment()))) {
      // case a)
        qDebug("Future Value is %Lf", val);
        throw new MYMONEYEXCEPTION("incorrect fincancial calculation");

      } else if((moneyBorrowed && val < 0 && fabsl(val) <= fabsl(calc.payment()))
             || (moneyLend && val > 0 && fabs(val) <= fabs(calc.payment()))) {
      // case b)
        val = 0;
      }

      MyMoneyMoney refVal(static_cast<double>(val));
      result = i18n("KMyMoney has calculated a balloon payment of %1 for this loan.")
          .arg(refVal.abs().formatMoney());

      if(!m_balloonAmount->lineedit()->text().isEmpty()) {
        if((m_balloonAmount->value().abs() - refVal.abs()).abs().toDouble() > 1) {
          throw new MYMONEYEXCEPTION("incorrect fincancial calculation");
        }
        result = i18n("KMyMoney has successfully verified your loan information.");
      }
      m_balloonAmount->loadText(refVal.abs().formatMoney());
    }

  } catch (MyMoneyException *e) {
    delete e;
    KMessageBox::error(0,
                       i18n("You have entered mis-matching information. Please modify "
                           "your figures or leave one value empty "
                           "to let KMyMoney calculate it for you"),
                           i18n("Calculation error"));
    return;
  }

  result += i18n("\n\nAccept this or modify the loan information and recalculate.");

  KMessageBox::information(0, result, i18n("Calculation successful"));
  m_needCalculate = false;

  // now update change
  m_wizard->completeStateChanged();
}

int LoanDetailsPage::term(void) const
{
  int factor = 0;

  if(m_termAmount->value() != 0) {
    factor = 1;
    switch(m_termUnit->currentItem()) {
      case MyMoneySchedule::OCCUR_YEARLY: // years
        factor = 12;
        // tricky fall through here

      case MyMoneySchedule::OCCUR_MONTHLY: // months
        factor *= 30;
        factor *= m_termAmount->value();
        // factor now is the duration in days. we divide this by the
        // payment frequency and get the number of payments
        factor /= m_wizard->m_generalLoanInfoPage->m_paymentFrequency->daysBetweenEvents();
        break;

      default:
        qDebug("Unknown term unit %d in LoanDetailsPage::term(). Using payments.", m_termUnit->currentItem());
        // tricky fall through here

      case MyMoneySchedule::OCCUR_ONCE: // payments
        factor = m_termAmount->value();
        break;
    }
  }
  return factor;
}

QString LoanDetailsPage::updateTermWidgets(const long double val)
{
  long long vl = static_cast<long long>(floorl(val));

  QString valString;
  MyMoneySchedule::occurenceE unit = m_termUnit->currentItem();

  if((unit == MyMoneySchedule::OCCUR_MONTHLY)
  && ((vl % 12) == 0)) {
    vl /= 12;
    unit = MyMoneySchedule::OCCUR_YEARLY;
  }

  switch(unit) {
    case MyMoneySchedule::OCCUR_MONTHLY:
      valString = i18n("one month", "%n months", vl);
      m_termUnit->setCurrentItem(MyMoneySchedule::OCCUR_MONTHLY);
      break;
    case MyMoneySchedule::OCCUR_YEARLY:
      valString = i18n("one year", "%n years", vl);
      m_termUnit->setCurrentItem(MyMoneySchedule::OCCUR_YEARLY);
      break;
    default:
      valString = i18n("one payment", "%n payments", vl);
      m_termUnit->setCurrentItem(MyMoneySchedule::OCCUR_ONCE);
      break;
  }
  m_termAmount->setValue(vl);
  return valString;
}

bool LoanDetailsPage::isComplete(void) const
{
  // bool rc = KMyMoneyWizardPage::isComplete();

  int fieldCnt = 0;
  QWidget* calculatedField = 0;

  if(m_loanAmount->lineedit()->text().length() > 0) {
    fieldCnt++;
  } else {
    calculatedField = m_loanAmount;
  }

  if(m_interestRate->lineedit()->text().length() > 0) {
    fieldCnt++;
  } else {
    calculatedField = m_interestRate;
  }

  if(m_termAmount->value() != 0) {
    fieldCnt++;
  } else {
    calculatedField = m_termAmount;
  }

  if(m_paymentAmount->lineedit()->text().length() > 0) {
    fieldCnt++;
  } else {
    calculatedField = m_paymentAmount;
  }

  if(m_balloonAmount->lineedit()->text().length() > 0) {
    fieldCnt++;
  } else {
    calculatedField = m_balloonAmount;
  }

  if(fieldCnt == 5)
    calculatedField = 0;

  m_calculateButton->setEnabled(fieldCnt == 4 || (fieldCnt == 5 && m_needCalculate));

  m_calculateButton->setAutoDefault(false);
  m_calculateButton->setDefault(false);
  if(m_needCalculate && fieldCnt == 4) {
    QToolTip::add(m_wizard->m_nextButton, i18n("Press Calculate to verify the values"));
    m_calculateButton->setAutoDefault(true);
    m_calculateButton->setDefault(true);
  } else if(fieldCnt != 5) {
    QToolTip::add(m_wizard->m_nextButton, i18n("Not all details supplied"));
    m_calculateButton->setAutoDefault(true);
    m_calculateButton->setDefault(true);
  }
  m_wizard->m_nextButton->setAutoDefault(!m_calculateButton->autoDefault());
  m_wizard->m_nextButton->setDefault(!m_calculateButton->autoDefault());

  return (fieldCnt == 5) && !m_needCalculate;
}

KMyMoneyWizardPage* LoanDetailsPage::nextPage(void) const
{
  return m_wizard->m_loanPaymentPage;
}


class NewAccountWizard::LoanPaymentPagePrivate
{
public:
  MyMoneyAccount      phonyAccount;
  MyMoneyTransaction  additionalFeesTransaction;
  MyMoneyMoney        additionalFees;
};

LoanPaymentPage::LoanPaymentPage(Wizard* wizard, const char* name) :
  KLoanPaymentPageDecl(wizard),
  WizardPage<Wizard>(3, this, wizard, name),
  d(new LoanPaymentPagePrivate)
{
  d->phonyAccount = MyMoneyAccount(QCString("Phony-ID"), MyMoneyAccount());
  MyMoneySplit split;
  split.setAccountId(d->phonyAccount.id());
  split.setValue(0);
  split.setShares(0);
  d->additionalFeesTransaction.addSplit(split);


  connect(m_additionalFeesButton, SIGNAL(clicked()), this, SLOT(slotAdditionalFees()));
}

LoanPaymentPage::~LoanPaymentPage()
{
  delete d;
}

MyMoneyMoney LoanPaymentPage::basePayment(void) const
{
  return m_wizard->m_loanDetailsPage->m_paymentAmount->value();
}

void LoanPaymentPage::updateAmounts(void)
{
  m_additionalFees->setText(d->additionalFees.formatMoney(m_wizard->currency().tradingSymbol()));
  m_totalPayment->setText((basePayment() + d->additionalFees).formatMoney(m_wizard->currency().tradingSymbol()));
}

void LoanPaymentPage::enterPage(void)
{
  m_basePayment->setText(basePayment().formatMoney(m_wizard->currency().tradingSymbol()));
  updateAmounts();
}

void LoanPaymentPage::slotAdditionalFees(void)
{
  QMap<QCString, MyMoneyMoney> priceInfo;
  KSplitTransactionDlg* dlg = new KSplitTransactionDlg(d->additionalFeesTransaction, d->phonyAccount, false, !m_wizard->moneyBorrowed(), MyMoneyMoney(0), priceInfo);

  // connect(dlg, SIGNAL(newCategory(MyMoneyAccount&)), this, SIGNAL(newCategory(MyMoneyAccount&)));

  if(dlg->exec() == QDialog::Accepted) {
    d->additionalFeesTransaction = dlg->transaction();
    // sum up the additional fees
    QValueList<MyMoneySplit>::ConstIterator it;

    d->additionalFees = MyMoneyMoney(0);
    for(it = d->additionalFeesTransaction.splits().begin(); it != d->additionalFeesTransaction.splits().end(); ++it) {
      if((*it).accountId() != d->phonyAccount.id()) {
        d->additionalFees += (*it).shares();
      }
    }
    updateAmounts();
  }

  delete dlg;
}

KMyMoneyWizardPage* LoanPaymentPage::nextPage(void) const
{
  return m_wizard->m_loanSchedulePage;
}


LoanSchedulePage::LoanSchedulePage(Wizard* wizard, const char* name) :
  KLoanSchedulePageDecl(wizard),
  WizardPage<Wizard>(4, this, wizard, name)
{
  m_mandatoryGroup->add(m_interestCategory->lineEdit());
  m_mandatoryGroup->add(m_paymentAccount->lineEdit());

  connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotLoadWidgets()));
  slotLoadWidgets();
}

void LoanSchedulePage::enterPage(void)
{
  m_firstPaymentDueDate->setDisabled(m_wizard->m_generalLoanInfoPage->recordAllPayments());
}

void LoanSchedulePage::slotLoadWidgets(void)
{
  AccountSet set;
  if(m_wizard->moneyBorrowed())
    set.addAccountGroup(MyMoneyAccount::Expense);
  else
    set.addAccountGroup(MyMoneyAccount::Income);
  set.load(m_interestCategory->selector());

  set.clear();
  set.addAccountGroup(MyMoneyAccount::Asset);
  set.load(m_paymentAccount->selector());
}

KMyMoneyWizardPage* LoanSchedulePage::nextPage(void) const
{
  return m_wizard->m_loanPayoutPage;
}

LoanPayoutPage::LoanPayoutPage(Wizard* wizard, const char* name) :
  KLoanPayoutPageDecl(wizard),
  WizardPage<Wizard>(5, this, wizard, name)
{
  KIconLoader* il = KGlobal::iconLoader();
  KGuiItem createAssetButtenItem( i18n( "&Create..." ),
                                  QIconSet(il->loadIcon("filenew", KIcon::Small, KIcon::SizeSmall)),
                                  i18n("Create a new asset account"),
                                  i18n("If the asset account does not yet exist, press this button to create it."));
  m_createAssetButton->setGuiItem(createAssetButtenItem);
  QToolTip::add(m_createAssetButton, createAssetButtenItem.toolTip());
  QWhatsThis::add(m_createAssetButton, createAssetButtenItem.whatsThis());

  connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotLoadWidgets()));
  slotLoadWidgets();
}

void LoanPayoutPage::slotLoadWidgets(void)
{
  AccountSet set;
  set.addAccountGroup(MyMoneyAccount::Asset);
  set.load(m_assetAccount->selector());
}

void LoanPayoutPage::enterPage(void)
{
  m_payoutDetailFrame->setDisabled(m_noPayoutTransaction->isChecked());
}

KMyMoneyWizardPage* LoanPayoutPage::nextPage(void) const
{
  return 0;
}

bool LoanPayoutPage::isComplete(void) const
{
  // for now, don't allow to finish the wizard at this point
  QToolTip::add(m_wizard->m_finishButton, i18n("This is work in progress. You are currently not able to create loan accounts with this version of the software."));
  return false;
}
#include "knewaccountwizard.moc"
