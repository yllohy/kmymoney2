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
#include <qlabel.h>

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
#include <kmymoney/kmymoneyaccounttree.h>
#include <kmymoney/mymoneyfinancialcalculator.h>
#include <kmymoney/kmymoneychecklistitem.h>
#include <kmymoney/kmymoneylistviewitem.h>
#include <kmymoney/kcurrencycalculator.h>
#include <kmymoney/mymoneyaccount.h>
#include <kmymoney/kmymoneyglobalsettings.h>

#include "knewaccountwizard.h"
#include "knewaccountwizard_p.h"
#include <kmymoney/kguiutils.h>

#include "../../dialogs/ksplittransactiondlg.h"
#include "../../dialogs/kequitypriceupdatedlg.h"
#include "../../kmymoney2.h"

using namespace NewAccountWizard;

namespace NewAccountWizard {
  enum steps {
    StepInstitution = 1,
    StepAccount,
    StepBroker,
    StepDetails,
    StepPayments,
    StepFees,
    StepSchedule,
    StepPayout,
    StepParentAccount,
    StepFinish
  };
};
NewAccountWizard::Wizard::Wizard(QWidget *parent, const char *name, bool modal, WFlags flags)
  : KMyMoneyWizard(parent, name, modal, flags)
{
  setTitle(i18n("KMyMoney New Account Setup"));
  addStep(i18n("Institution"));
  addStep(i18n("Account"));
  addStep(i18n("Broker"));
  addStep(i18n("Details"));
  addStep(i18n("Payments"));
  addStep(i18n("Fees"));
  addStep(i18n("Schedule"));
  addStep(i18n("Payout"));
  addStep(i18n("Parent Account"));
  addStep(i18n("Finish"));
  setStepHidden(StepBroker);
  setStepHidden(StepSchedule);
  setStepHidden(StepPayout);
  setStepHidden(StepDetails);
  setStepHidden(StepPayments);
  setStepHidden(StepFees);

  m_institutionPage = new InstitutionPage(this);
  m_accountTypePage = new AccountTypePage(this);
  // Investment Pages
  m_brokeragepage = new BrokeragePage(this);
  // Credit Card Pages
  m_schedulePage = new CreditCardSchedulePage(this);
  // Loan Pages
  m_generalLoanInfoPage = new GeneralLoanInfoPage(this);
  m_loanDetailsPage = new LoanDetailsPage(this);
  m_loanPaymentPage = new LoanPaymentPage(this);
  m_loanSchedulePage = new LoanSchedulePage(this);
  m_loanPayoutPage = new LoanPayoutPage(this);
  // Not a loan page
  m_hierarchyPage = new HierarchyPage(this);
  // Finish
  m_accountSummaryPage = new AccountSummaryPage(this);

  setFirstPage(m_institutionPage);
}

void NewAccountWizard::Wizard::setAccount(const MyMoneyAccount& acc)
{
  m_account = acc;
  m_accountTypePage->setAccount(m_account);
}

const MyMoneySecurity& NewAccountWizard::Wizard::currency(void) const
{
  return m_accountTypePage->currency();
}

MyMoneyMoney NewAccountWizard::Wizard::interestRate(void) const
{
  return m_loanDetailsPage->m_interestRate->value() / MyMoneyMoney(100,1);
}

int NewAccountWizard::Wizard::precision(void) const
{
  return MyMoneyMoney::denomToPrec(currency().smallestAccountFraction());
}

const MyMoneyAccount& NewAccountWizard::Wizard::account(void)
{
  m_account = MyMoneyAccountLoan();
  m_account.setName(m_accountTypePage->m_accountName->text());
  m_account.setOpeningDate(m_accountTypePage->m_openingDate->date());
  m_account.setAccountType(m_accountTypePage->accountType());
  m_account.setInstitutionId(m_institutionPage->institution().id());
  m_account.setNumber(m_institutionPage->m_accountNumber->text());
  m_account.setValue("IBAN", m_institutionPage->m_iban->text());
  if(m_accountTypePage->m_preferredAccount->isChecked())
    m_account.setValue("PreferredAccount", "Yes");
  else
    m_account.deletePair("PreferredAccount");

  m_account.setCurrencyId(currency().id());
  if(m_account.isLoan()) {
    // in case we lend the money we adjust the account type
    if(!moneyBorrowed())
      m_account.setAccountType(MyMoneyAccount::AssetLoan);
    m_account.setLoanAmount(m_loanDetailsPage->m_loanAmount->value());
    m_account.setInterestRate(m_loanSchedulePage->firstPaymentDueDate(), m_loanDetailsPage->m_interestRate->value());
    m_account.setInterestCalculation(m_loanDetailsPage->m_paymentDue->currentItem() == 0 ? MyMoneyAccountLoan::paymentReceived : MyMoneyAccountLoan::paymentDue);
    m_account.setFixedInterestRate(m_generalLoanInfoPage->m_interestType->currentItem() == 0);
    m_account.setFinalPayment(m_loanDetailsPage->m_balloonAmount->value());
    m_account.setTerm(m_loanDetailsPage->term());
    m_account.setPeriodicPayment(m_loanDetailsPage->m_paymentAmount->value());
    m_account.setPayee(m_generalLoanInfoPage->m_payee->selectedItem());
    m_account.setInterestCompounding(m_generalLoanInfoPage->m_compoundFrequency->currentItem());

    if(!m_account.fixedInterestRate()) {
      m_account.setNextInterestChange(m_generalLoanInfoPage->m_interestChangeDateEdit->date());
      m_account.setInterestChangeFrequency(m_generalLoanInfoPage->m_interestFrequencyAmountEdit->value(), m_generalLoanInfoPage->m_interestFrequencyUnitEdit->currentItem());
    }
  }
  return m_account;
}

MyMoneyTransaction NewAccountWizard::Wizard::payoutTransaction(void)
{
  MyMoneyTransaction t;
  if(m_account.isLoan()                                       // we're creating a loan
  && openingBalance().isZero()                                // and don't have an opening balance
  && !m_loanPayoutPage->m_noPayoutTransaction->isChecked()) { // and the user wants to have a payout transaction
    t.setPostDate(m_loanPayoutPage->m_payoutDate->date());
    t.setCommodity(m_account.currencyId());
    MyMoneySplit s;
    s.setAccountId(m_account.id());
    s.setShares(m_loanDetailsPage->m_loanAmount->value());
    if(moneyBorrowed())
      s.setShares(-s.shares());
    s.setValue(s.shares());
    t.addSplit(s);

    s.clearId();
    s.setValue(-s.value());
    s.setAccountId(m_loanPayoutPage->payoutAccountId());
    MyMoneyMoney shares;
    KCurrencyCalculator::setupSplitPrice(shares, t, s, QMap<QCString, MyMoneyMoney>(), this);
    s.setShares(shares);
    t.addSplit(s);
  }
  return t;
}

const MyMoneyAccount& NewAccountWizard::Wizard::parentAccount(void)
{
  return m_accountTypePage->allowsParentAccount()
         ? m_hierarchyPage->parentAccount()
         : ( m_accountTypePage->accountType() == MyMoneyAccount::Loan
           ? m_generalLoanInfoPage->parentAccount()
           : m_accountTypePage->parentAccount() );
}

MyMoneyAccount NewAccountWizard::Wizard::brokerageAccount(void) const
{
  MyMoneyAccount account;
  if(m_account.accountType() == MyMoneyAccount::Investment
  && m_brokeragepage->m_createBrokerageButton->isChecked()) {
    account.setName(m_account.brokerageName());
    account.setAccountType(MyMoneyAccount::Checkings);
    account.setInstitutionId(m_account.institutionId());
    account.setOpeningDate(m_account.openingDate());
    account.setCurrencyId(m_brokeragepage->m_brokerageCurrency->security().id());
    if(m_brokeragepage->m_accountNumber->isEnabled() && !m_brokeragepage->m_accountNumber->text().isEmpty())
      account.setNumber(m_brokeragepage->m_accountNumber->text());
    if(m_brokeragepage->m_iban->isEnabled() && !m_brokeragepage->m_iban->text().isEmpty())
      account.setValue("IBAN", m_brokeragepage->m_iban->text());
  }
  return account;
}

const MyMoneySchedule& NewAccountWizard::Wizard::schedule(void)
{
  m_schedule = MyMoneySchedule();

  if(!m_account.id().isEmpty()) {
    if(m_schedulePage->m_reminderCheckBox->isChecked() && (m_account.accountType() == MyMoneyAccount::CreditCard)) {
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

    } else if(m_account.isLoan()) {
      m_schedule.setName(i18n("Loan payment for %1").arg(m_accountTypePage->m_accountName->text()));
      m_schedule.setType(MyMoneySchedule::TYPE_LOANPAYMENT);
      if(moneyBorrowed())
        m_schedule.setPaymentType(MyMoneySchedule::STYPE_DIRECTDEBIT);
      else
        m_schedule.setPaymentType(MyMoneySchedule::STYPE_DIRECTDEPOSIT);

      m_schedule.setFixed(true);
      m_schedule.setOccurence(m_generalLoanInfoPage->m_paymentFrequency->currentItem());

      MyMoneyTransaction t;
      MyMoneySplit s;
      // payment split
      s.setPayeeId(m_generalLoanInfoPage->m_payee->selectedItem());
      s.setAccountId(m_loanSchedulePage->m_paymentAccount->selectedItem());
      s.setMemo(i18n("Loan payment"));
      if(moneyBorrowed()) {
        s.setShares(-(m_loanPaymentPage->basePayment() + m_loanPaymentPage->additionalFees()));
      } else {
        s.setShares(m_loanPaymentPage->basePayment() + m_loanPaymentPage->additionalFees());
      }
      s.setValue(s.shares());
      t.addSplit(s);

      // principal split
      s.clearId();
      s.setAccountId(m_account.id());
      s.setShares(MyMoneyMoney::autoCalc);
      s.setValue(MyMoneyMoney::autoCalc);
      s.setMemo(i18n("Amortization"));
      s.setAction(MyMoneySplit::ActionAmortization);
      t.addSplit(s);

      // interest split
      s.clearId();
      s.setAccountId(m_loanSchedulePage->m_interestCategory->selectedItem());
      s.setShares(MyMoneyMoney::autoCalc);
      s.setValue(MyMoneyMoney::autoCalc);
      s.setMemo(i18n("Interest"));
      s.setAction(MyMoneySplit::ActionInterest);
      t.addSplit(s);

      // additional fee splits
      QValueList<MyMoneySplit> additionalSplits;
      m_loanPaymentPage->additionalFeesSplits(additionalSplits);
      QValueList<MyMoneySplit>::const_iterator it;
      MyMoneyMoney factor(moneyBorrowed() ? 1 : -1, 1);

      for(it = additionalSplits.begin(); it != additionalSplits.end(); ++it) {
        s = (*it);
        s.clearId();
        s.setShares(s.shares() * factor);
        s.setValue(s.value() * factor);
        t.addSplit(s);
      }

      // setup the next due date
      t.setPostDate(m_loanSchedulePage->firstPaymentDueDate());
      m_schedule.setTransaction(t);
    }
  }
  return m_schedule;
}

MyMoneyMoney NewAccountWizard::Wizard::openingBalance(void) const
{
  // equity accounts don't have an opening balance
  if(m_accountTypePage->accountType() == MyMoneyAccount::Equity)
    return MyMoneyMoney();

  if(m_accountTypePage->accountType() == MyMoneyAccount::Loan) {
    if(m_generalLoanInfoPage->recordAllPayments())
      return MyMoneyMoney(0, 1);
    if(moneyBorrowed())
      return -(m_generalLoanInfoPage->m_openingBalance->value());
    return m_generalLoanInfoPage->m_openingBalance->value();
  }
  return m_accountTypePage->m_openingBalance->value();
}

MyMoneyPrice NewAccountWizard::Wizard::conversionRate(void) const
{
  if(MyMoneyFile::instance()->baseCurrency().id() == m_accountTypePage->m_currencyComboBox->security().id())
    return MyMoneyPrice(MyMoneyFile::instance()->baseCurrency().id(),
                        m_accountTypePage->m_currencyComboBox->security().id(),
                        m_accountTypePage->m_openingDate->date(),
                        MyMoneyMoney(1,1),
                        i18n("User"));
  return MyMoneyPrice(MyMoneyFile::instance()->baseCurrency().id(),
                      m_accountTypePage->m_currencyComboBox->security().id(),
                      m_accountTypePage->m_openingDate->date(),
                      m_accountTypePage->m_conversionRate->value(),
                      i18n("User"));
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
  WizardPage<Wizard>(StepInstitution, this, wizard, name),
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

  emit m_wizard->createInstitution(institution);

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
  WizardPage<Wizard>(StepAccount, this, wizard, name),
  m_showPriceWarning(true)
{
  m_typeSelection->insertItem(i18n("Checking"), MyMoneyAccount::Checkings);
  m_typeSelection->insertItem(i18n("Savings"), MyMoneyAccount::Savings);
  m_typeSelection->insertItem(i18n("Credit Card"), MyMoneyAccount::CreditCard);
  m_typeSelection->insertItem(i18n("Cash"), MyMoneyAccount::Cash);
  m_typeSelection->insertItem(i18n("Loan"), MyMoneyAccount::Loan);
  m_typeSelection->insertItem(i18n("Investment"), MyMoneyAccount::Investment);
  m_typeSelection->insertItem(i18n("Asset"), MyMoneyAccount::Asset);
  m_typeSelection->insertItem(i18n("Liability"), MyMoneyAccount::Liability);
  if(KMyMoneyGlobalSettings::expertMode()) {
    m_typeSelection->insertItem(i18n("Equity"), MyMoneyAccount::Equity);
  }

  m_typeSelection->setCurrentItem(MyMoneyAccount::Checkings);

  m_currencyComboBox->setSecurity(MyMoneyFile::instance()->baseCurrency());

  m_mandatoryGroup->add(m_accountName);
  m_mandatoryGroup->add(m_conversionRate->lineedit());

  m_conversionRate->setPrecision(KMyMoneyGlobalSettings::pricePrecision());
  m_conversionRate->setValue(MyMoneyMoney(1,1));
  slotUpdateCurrency();

  connect(m_typeSelection, SIGNAL(itemSelected(int)), this, SLOT(slotUpdateType(int)));
  connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotLoadWidgets()));
  connect(m_currencyComboBox, SIGNAL(activated(int)), this, SLOT(slotUpdateCurrency()));
  connect(m_conversionRate, SIGNAL(textChanged(const QString&)), this, SLOT(slotUpdateConversionRate(const QString&)));
  connect(m_conversionRate, SIGNAL(valueChanged(const QString&)), this, SLOT(slotPriceWarning()));
  connect(m_onlineQuote, SIGNAL(clicked()), this, SLOT(slotGetOnlineQuote()));
}

void AccountTypePage::slotUpdateType(int i)
{
  hideShowPages(static_cast<MyMoneyAccount::accountTypeE> (i));
  m_openingBalance->setDisabled(static_cast<MyMoneyAccount::accountTypeE> (i) == MyMoneyAccount::Equity);
}

void AccountTypePage::hideShowPages(MyMoneyAccount::accountTypeE accountType) const
{
  bool hideSchedulePage = (accountType != MyMoneyAccount::CreditCard)
                       && (accountType != MyMoneyAccount::Loan);
  bool hideLoanPage     = (accountType != MyMoneyAccount::Loan);
  m_wizard->setStepHidden(StepDetails, hideLoanPage);
  m_wizard->setStepHidden(StepPayments, hideLoanPage);
  m_wizard->setStepHidden(StepFees, hideLoanPage);
  m_wizard->setStepHidden(StepSchedule, hideSchedulePage);
  m_wizard->setStepHidden(StepPayout, (accountType != MyMoneyAccount::Loan));
  m_wizard->setStepHidden(StepBroker, accountType != MyMoneyAccount::Investment);
  m_wizard->setStepHidden(StepParentAccount, accountType == MyMoneyAccount::Loan);
  // Force an update of the steps in case the list has changed
  m_wizard->reselectStep();
};

KMyMoneyWizardPage* AccountTypePage::nextPage(void) const
{
  if(accountType() == MyMoneyAccount::Loan)
    return m_wizard->m_generalLoanInfoPage;
  if(accountType() == MyMoneyAccount::CreditCard)
    return m_wizard->m_schedulePage;
  if(accountType() == MyMoneyAccount::Investment)
    return m_wizard->m_brokeragepage;
  return m_wizard->m_hierarchyPage;
}

void AccountTypePage::slotUpdateCurrency(void)
{
  MyMoneyAccount acc;
  acc.setAccountType(accountType());

  m_openingBalance->setPrecision(MyMoneyMoney::denomToPrec(acc.fraction()));

  bool show =  m_currencyComboBox->security().id() != MyMoneyFile::instance()->baseCurrency().id();
  m_conversionLabel->setShown(show);
  m_conversionRate->setShown(show);
  m_conversionExample->setShown(show);
  m_onlineQuote->setShown(show);
  m_conversionRate->setEnabled(show);       // make sure to include/exclude in mandatoryGroup
  m_mandatoryGroup->changed();
  slotUpdateConversionRate(m_conversionRate->lineedit()->text());
}

void AccountTypePage::slotGetOnlineQuote(void)
{
  QCString id = MyMoneyFile::instance()->baseCurrency().id()+" "+m_currencyComboBox->security().id();
  KEquityPriceUpdateDlg dlg(this, id);
  if(dlg.exec() == QDialog::Accepted) {
    MyMoneyPrice price = dlg.price(id);
    if(price.isValid()) {
      m_conversionRate->setValue(price.rate(m_currencyComboBox->security().id()));
      if(price.date() != m_openingDate->date()) {
        priceWarning(true);
      }
    }
  }
}

void AccountTypePage::slotPriceWarning(void)
{
  priceWarning(false);
}

void AccountTypePage::priceWarning(bool always)
{
  if(m_showPriceWarning || always) {
    KMessageBox::information(this, i18n("Please make sure to enter the correct conversion for the selected opening date. If you requested an online quote it might be provided for a different date."), i18n("Check date"));
  }
  m_showPriceWarning = false;
}

void AccountTypePage::slotUpdateConversionRate(const QString& txt)
{
  m_conversionExample->setText(i18n("1 %1 equals %2").
      arg(MyMoneyFile::instance()->baseCurrency().tradingSymbol()).
      arg(MyMoneyMoney(txt).formatMoney(m_currencyComboBox->security().tradingSymbol(), KMyMoneyGlobalSettings::pricePrecision())));
}

void AccountTypePage::slotLoadWidgets(void)
{
  m_currencyComboBox->update(QCString("x"));
}

bool AccountTypePage::isComplete(void) const
{
  // check that the conversion rate is positive if enabled
  bool rc = !m_conversionRate->isVisible() || (!m_conversionRate->value().isZero() && !m_conversionRate->value().isNegative());
  if(!rc) {
    QToolTip::add(m_wizard->m_nextButton, i18n("Conversion rate is not positive"));

  } else {
    rc = KMyMoneyWizardPage::isComplete();

    if(!rc) {
      QToolTip::add(m_wizard->m_nextButton, i18n("No account name supplied"));
    }
  }
  hideShowPages(accountType());
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

void AccountTypePage::setAccount(const MyMoneyAccount& acc)
{
  m_typeSelection->setCurrentItem(acc.accountType());
  m_openingDate->setDate(acc.openingDate());
  m_accountName->setText(acc.name());
}

const MyMoneyAccount& AccountTypePage::parentAccount(void)
{
  switch(accountType()) {
    case MyMoneyAccount::CreditCard:
    case MyMoneyAccount::Liability:
    case MyMoneyAccount::Loan: // Can be either but we return liability here
      return MyMoneyFile::instance()->liability();
      break;
    case MyMoneyAccount::Equity:
      return MyMoneyFile::instance()->equity();
    default:
      break;
  }
  return MyMoneyFile::instance()->asset();
}

bool AccountTypePage::allowsParentAccount(void) const
{
  return accountType() != MyMoneyAccount::Loan;
}

BrokeragePage::BrokeragePage(Wizard* wizard, const char* name) :
  KBrokeragePageDecl(wizard),
  WizardPage<Wizard>(StepBroker, this, wizard, name)
{
  connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotLoadWidgets()));
}

void BrokeragePage::slotLoadWidgets(void)
{
  m_brokerageCurrency->update(QCString("x"));
}

void BrokeragePage::enterPage(void)
{
  // assign the currency of the investment account to the
  // brokerage account if nothing else has ever been selected
  if(m_brokerageCurrency->security().id().isEmpty()) {
    m_brokerageCurrency->setSecurity(m_wizard->m_accountTypePage->m_currencyComboBox->security());
  }

  // check if the institution relevant fields should be enabled or not
  bool enabled = m_wizard->m_institutionPage->m_accountNumber->isEnabled();
  m_accountNumberLabel->setEnabled(enabled);
  m_accountNumber->setEnabled(enabled);
  m_ibanLabel->setEnabled(enabled);
  m_iban->setEnabled(enabled);
}

KMyMoneyWizardPage* BrokeragePage::nextPage(void) const
{
  return m_wizard->m_hierarchyPage;
}

CreditCardSchedulePage::CreditCardSchedulePage(Wizard* wizard, const char* name) :
  KSchedulePageDecl(wizard),
  WizardPage<Wizard>(StepSchedule, this, wizard, name)
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

  m_method->insertItem(i18n("Write check"), MyMoneySchedule::STYPE_WRITECHEQUE);
#if 0
  m_method->insertItem(i18n("Direct debit"), MyMoneySchedule::STYPE_DIRECTDEBIT);
  m_method->insertItem(i18n("Bank transfer"), MyMoneySchedule::STYPE_BANKTRANSFER);
#endif
  m_method->insertItem(i18n("Standing order"), MyMoneySchedule::STYPE_STANDINGORDER);
  m_method->insertItem(i18n("Manual deposit"), MyMoneySchedule::STYPE_MANUALDEPOSIT);
  m_method->insertItem(i18n("Direct deposit"), MyMoneySchedule::STYPE_DIRECTDEPOSIT);
  m_method->insertItem(i18n("Other"), MyMoneySchedule::STYPE_OTHER);
  m_method->setCurrentItem(MyMoneySchedule::STYPE_DIRECTDEBIT);

  slotLoadWidgets();
}

void CreditCardSchedulePage::enterPage(void)
{
  if(m_name->text().isEmpty())
    m_name->setText(i18n("CreditCard %1 monthly payment").arg(m_wizard->m_accountTypePage->m_accountName->text()));
}

bool CreditCardSchedulePage::isComplete(void) const
{
  bool rc = true;
  QString msg = i18n("Finish entry and create account");
  if(m_reminderCheckBox->isChecked()) {
    msg = i18n("Finish entry and create account and schedule");
    if(m_date->date() < m_wizard->m_accountTypePage->m_openingDate->date()) {
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

KMyMoneyWizardPage* CreditCardSchedulePage::nextPage(void) const
{
  return m_wizard->m_hierarchyPage;
}

GeneralLoanInfoPage::GeneralLoanInfoPage(Wizard* wizard, const char* name) :
  KGeneralLoanInfoPageDecl(wizard),
  WizardPage<Wizard>(StepDetails, this, wizard, name),
  m_firstTime(true)
{
  m_mandatoryGroup->add(m_payee);

  // remove the unsupported payment and compounding frequencies and setup default
  m_paymentFrequency->removeItem(MyMoneySchedule::OCCUR_ONCE);
  m_paymentFrequency->removeItem(MyMoneySchedule::OCCUR_EVERYOTHERYEAR);
  m_paymentFrequency->setCurrentItem(MyMoneySchedule::OCCUR_MONTHLY);
  m_compoundFrequency->removeItem(MyMoneySchedule::OCCUR_ONCE);
  m_compoundFrequency->removeItem(MyMoneySchedule::OCCUR_EVERYOTHERYEAR);
  m_compoundFrequency->setCurrentItem(MyMoneySchedule::OCCUR_MONTHLY);

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
  m_wizard->setStepHidden(StepPayout, !m_wizard->openingBalance().isZero());
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

const MyMoneyAccount& GeneralLoanInfoPage::parentAccount(void)
{
  return ( m_loanDirection->currentItem() == 0 )
         ? MyMoneyFile::instance()->liability()
         : MyMoneyFile::instance()->asset();
}

void GeneralLoanInfoPage::slotLoadWidgets(void)
{
  m_payee->loadPayees(MyMoneyFile::instance()->payeeList());
}

LoanDetailsPage::LoanDetailsPage(Wizard* wizard, const char* name) :
  KLoanDetailsPageDecl(wizard),
  WizardPage<Wizard>(StepPayments, this, wizard, name),
  m_needCalculate(true)
{
  // force the balloon payment to zero (default)
  m_balloonAmount->setValue(MyMoneyMoney());

  connect(m_paymentDue, SIGNAL(activated(int)), this, SLOT(slotValuesChanged()));

  connect(m_termAmount, SIGNAL(valueChanged(int)), this, SLOT(slotValuesChanged()));
  connect(m_termUnit, SIGNAL(highlighted(int)), this, SLOT(slotValuesChanged()));
  connect(m_loanAmount, SIGNAL(textChanged(const QString&)), this, SLOT(slotValuesChanged()));
  connect(m_interestRate, SIGNAL(textChanged(const QString&)), this, SLOT(slotValuesChanged()));
  connect(m_paymentAmount, SIGNAL(textChanged(const QString&)), this, SLOT(slotValuesChanged()));
  connect(m_balloonAmount, SIGNAL(textChanged(const QString&)), this, SLOT(slotValuesChanged()));

  connect(m_calculateButton, SIGNAL(clicked()), this, SLOT(slotCalculate()));
}

void LoanDetailsPage::enterPage(void)
{
  // we need to remove a bunch of entries of the payment frequencies
  m_termUnit->clear();

  m_mandatoryGroup->clear();
  if(!m_wizard->openingBalance().isZero()) {
    m_mandatoryGroup->add(m_loanAmount->lineedit());
    if(m_loanAmount->lineedit()->text().length() == 0) {
      m_loanAmount->setValue(m_wizard->openingBalance().abs());
    }
  }

  switch(m_wizard->m_generalLoanInfoPage->m_paymentFrequency->currentItem()) {
    default:
      m_termUnit->insertItem(i18n("Payments"), MyMoneySchedule::OCCUR_ONCE);
      m_termUnit->setCurrentItem(MyMoneySchedule::OCCUR_ONCE);
      break;
    case MyMoneySchedule::OCCUR_MONTHLY:
      m_termUnit->insertItem(i18n("Months"), MyMoneySchedule::OCCUR_MONTHLY);
      m_termUnit->insertItem(i18n("Years"), MyMoneySchedule::OCCUR_YEARLY);
      m_termUnit->setCurrentItem(MyMoneySchedule::OCCUR_MONTHLY);
      break;
    case MyMoneySchedule::OCCUR_YEARLY:
      m_termUnit->insertItem(i18n("Years"), MyMoneySchedule::OCCUR_YEARLY);
      m_termUnit->setCurrentItem(MyMoneySchedule::OCCUR_YEARLY);
      break;
  }
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
  int PF, CF;
  QString result;
  bool moneyBorrowed = m_wizard->moneyBorrowed();
  bool moneyLend = !moneyBorrowed;

  // FIXME: for now, we only support interest calculation at the end of the period
  calc.setBep();
  // FIXME: for now, we only support periodic compounding
  calc.setDisc();

  PF = m_wizard->m_generalLoanInfoPage->m_paymentFrequency->eventsPerYear();
  CF = m_wizard->m_generalLoanInfoPage->m_compoundFrequency->eventsPerYear();

  if(PF == 0 || CF == 0)
    return;

  calc.setPF(PF);
  calc.setCF(CF);


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
      m_loanAmount->loadText(MyMoneyMoney(static_cast<double>(val)).abs().formatMoney("", m_wizard->precision()));
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
        m_balloonAmount->loadText(refVal.abs().formatMoney("", m_wizard->precision()));
        result += QString(" ");
        result += i18n("The number of payments has been decremented and the balloon payment has been modified to %1.")
            .arg(m_balloonAmount->lineedit()->text());
          } else if((moneyBorrowed && val < 0 && fabsl(val) < fabsl(calc.payment()))
                     || (moneyLend && val > 0 && fabs(val) < fabs(calc.payment()))) {
                       m_balloonAmount->loadText(MyMoneyMoney(0,1).formatMoney("", m_wizard->precision()));
                     } else {
                       MyMoneyMoney refVal(static_cast<double>(val));
                       m_balloonAmount->loadText(refVal.abs().formatMoney("", m_wizard->precision()));
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
        m_balloonAmount->loadText(refVal.abs().formatMoney("", m_wizard->precision()));
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
          .arg(refVal.abs().formatMoney("", m_wizard->precision()));

      if(!m_balloonAmount->lineedit()->text().isEmpty()) {
        if((m_balloonAmount->value().abs() - refVal.abs()).abs().toDouble() > 1) {
          throw new MYMONEYEXCEPTION("incorrect fincancial calculation");
        }
        result = i18n("KMyMoney has successfully verified your loan information.");
      }
      m_balloonAmount->loadText(refVal.abs().formatMoney("", m_wizard->precision()));
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
  MyMoneySplit        phonySplit;
  MyMoneyTransaction  additionalFeesTransaction;
  MyMoneyMoney        additionalFees;
};

LoanPaymentPage::LoanPaymentPage(Wizard* wizard, const char* name) :
  KLoanPaymentPageDecl(wizard),
  WizardPage<Wizard>(StepFees, this, wizard, name),
  d(new LoanPaymentPagePrivate)
{
  d->phonyAccount = MyMoneyAccount(QCString("Phony-ID"), MyMoneyAccount());

  d->phonySplit.setAccountId(d->phonyAccount.id());
  d->phonySplit.setValue(0);
  d->phonySplit.setShares(0);

  d->additionalFeesTransaction.addSplit(d->phonySplit);

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

MyMoneyMoney LoanPaymentPage::additionalFees(void) const
{
  return d->additionalFees;
}

void LoanPaymentPage::additionalFeesSplits(QValueList<MyMoneySplit>& list)
{
  list.clear();

  QValueList<MyMoneySplit>::ConstIterator it;
  for(it = d->additionalFeesTransaction.splits().begin(); it != d->additionalFeesTransaction.splits().end(); ++it) {
    if((*it).accountId() != d->phonyAccount.id()) {
      list << (*it);
    }
  }
}

void LoanPaymentPage::updateAmounts(void)
{
  m_additionalFees->setText(d->additionalFees.formatMoney(m_wizard->currency().tradingSymbol(), m_wizard->precision()));
  m_totalPayment->setText((basePayment() + d->additionalFees).formatMoney(m_wizard->currency().tradingSymbol(), m_wizard->precision()));
}

void LoanPaymentPage::enterPage(void)
{
  const MyMoneySecurity& currency = m_wizard->currency();

  m_basePayment->setText(basePayment().formatMoney(currency.tradingSymbol(), m_wizard->precision()));
  d->phonyAccount.setCurrencyId(currency.id());
  d->additionalFeesTransaction.setCommodity(currency.id());

  updateAmounts();
}

void LoanPaymentPage::slotAdditionalFees(void)
{
  QMap<QCString, MyMoneyMoney> priceInfo;
  KSplitTransactionDlg* dlg = new KSplitTransactionDlg(d->additionalFeesTransaction, d->phonySplit, d->phonyAccount, false, !m_wizard->moneyBorrowed(), MyMoneyMoney(0), priceInfo);

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
  WizardPage<Wizard>(StepSchedule, this, wizard, name)
{
  m_mandatoryGroup->add(m_interestCategory->lineEdit());
  m_mandatoryGroup->add(m_paymentAccount->lineEdit());
  connect(m_interestCategory, SIGNAL(createItem(const QString&, QCString&)), this, SLOT(slotCreateCategory(const QString&, QCString&)));
  connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotLoadWidgets()));
}

void LoanSchedulePage::slotCreateCategory(const QString& name, QCString& id)
{
  MyMoneyAccount acc, parent;
  acc.setName(name);

  if(m_wizard->moneyBorrowed())
    parent = MyMoneyFile::instance()->expense();
  else
    parent = MyMoneyFile::instance()->income();

  emit m_wizard->createCategory(acc, parent);

  // return id
  id = acc.id();
}

QDate LoanSchedulePage::firstPaymentDueDate(void) const
{
  if(m_firstPaymentDueDate->isEnabled())
    return m_firstPaymentDueDate->date();
  return m_wizard->m_generalLoanInfoPage->m_firstPaymentDate->date();
}

void LoanSchedulePage::enterPage(void)
{
  m_interestCategory->setFocus();
  m_firstPaymentDueDate->setDisabled(m_wizard->m_generalLoanInfoPage->recordAllPayments());
  slotLoadWidgets();
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
  // if the balance widget of the general loan info page is enabled and
  // the value is not zero, then the payout already happened and we don't
  // aks for it.
  if(m_wizard->openingBalance().isZero())
    return m_wizard->m_loanPayoutPage;
  return m_wizard->m_accountSummaryPage;
}

LoanPayoutPage::LoanPayoutPage(Wizard* wizard, const char* name) :
  KLoanPayoutPageDecl(wizard),
  WizardPage<Wizard>(StepPayout, this, wizard, name)
{
  m_mandatoryGroup->add(m_assetAccount->lineEdit());
  m_mandatoryGroup->add(m_loanAccount->lineEdit());

  KIconLoader* il = KGlobal::iconLoader();
  KGuiItem createAssetButtenItem( i18n( "&Create..." ),
                                  QIconSet(il->loadIcon("filenew", KIcon::Small, KIcon::SizeSmall)),
                                  i18n("Create a new asset account"),
                                  i18n("If the asset account does not yet exist, press this button to create it."));
  m_createAssetButton->setGuiItem(createAssetButtenItem);
  QToolTip::add(m_createAssetButton, createAssetButtenItem.toolTip());
  QWhatsThis::add(m_createAssetButton, createAssetButtenItem.whatsThis());
  connect(m_createAssetButton, SIGNAL(clicked()), this, SLOT(slotCreateAssetAccount()));

  connect(m_noPayoutTransaction, SIGNAL(toggled(bool)), this, SLOT(slotButtonsToggled()));
  connect(m_refinanceLoan, SIGNAL(toggled(bool)), this, SLOT(slotButtonsToggled()));

  connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotLoadWidgets()));
  slotLoadWidgets();
}

void LoanPayoutPage::slotButtonsToggled(void)
{
  // we don't go directly, as the order of the emission of signals to slots is
  // not defined. Using a single shot timer postpones the call of m_mandatoryGroup::changed()
  // until the next round of the main loop so we can be sure to see all relevant changes
  // that happened in the meantime (eg. widgets are enabled and disabled)
  QTimer::singleShot(0, m_mandatoryGroup, SLOT(changed()));
}

void LoanPayoutPage::slotCreateAssetAccount(void)
{
  MyMoneyAccount acc;
  acc.setAccountType(MyMoneyAccount::Asset);
  acc.setOpeningDate(m_wizard->m_accountTypePage->m_openingDate->date());

  emit m_wizard->createAccount(acc);

  if(!acc.id().isEmpty()) {
    m_assetAccount->setSelectedItem(acc.id());
  }
}

void LoanPayoutPage::slotLoadWidgets(void)
{
  AccountSet set;
  set.addAccountGroup(MyMoneyAccount::Asset);
  set.load(m_assetAccount->selector());

  set.clear();
  set.addAccountType(MyMoneyAccount::Loan);
  set.load(m_loanAccount->selector());
}

void LoanPayoutPage::enterPage(void)
{
  // only allow to create new asset accounts for liability loans
  m_createAssetButton->setEnabled(m_wizard->moneyBorrowed());
  m_refinanceLoan->setEnabled(m_wizard->moneyBorrowed());
  if(!m_wizard->moneyBorrowed()) {
    m_refinanceLoan->setChecked(false);
  }
  m_payoutDetailFrame->setDisabled(m_noPayoutTransaction->isChecked());
}

KMyMoneyWizardPage* LoanPayoutPage::nextPage(void) const
{
  return m_wizard->m_accountSummaryPage;
}

bool LoanPayoutPage::isComplete(void) const
{
  return KMyMoneyWizardPage::isComplete() | m_noPayoutTransaction->isChecked();
}

const QCString& LoanPayoutPage::payoutAccountId(void) const
{
  if(m_refinanceLoan->isChecked()) {
    return m_loanAccount->selectedItem();
  } else {
    return m_assetAccount->selectedItem();
  }
}

HierarchyPage::HierarchyPage(Wizard* wizard, const char* name) :
  KHierarchyPageDecl(wizard),
  WizardPage<Wizard>(StepParentAccount, this, wizard, name)
{
  m_mandatoryGroup->add(m_qlistviewParentAccounts);
  m_qlistviewParentAccounts->setEnabled(true);
  m_qlistviewParentAccounts->setRootIsDecorated(true);
  m_qlistviewParentAccounts->setAllColumnsShowFocus(true);
  m_qlistviewParentAccounts->addColumn("Accounts");
  m_qlistviewParentAccounts->setMultiSelection(false);
  m_qlistviewParentAccounts->header()->setResizeEnabled(true);
  m_qlistviewParentAccounts->setColumnWidthMode(0, QListView::Maximum);
  // never show the horizontal scroll bar
  // m_qlistviewParentAccounts->setHScrollBarMode(QScrollView::AlwaysOff);
}

void HierarchyPage::enterPage(void)
{
  // Ensure that the list reflects the Account Type
  // - if the type has changed
  // - - clear the list
  // - - populate the account list (also occurs first time we come here)
  MyMoneyAccount topAccount = m_wizard->m_accountTypePage->parentAccount();

  // If the list was not populated with this top account we populate it now
  if ( &m_topAccount == NULL || m_topAccount.id() != topAccount.id())
  {
    if ( &m_topAccount != NULL )
    {
      // If the list has alrady been populated clear it
      if ( (*m_qlistviewParentAccounts).childCount() > 0 )
        (*m_qlistviewParentAccounts).clear();
    }

    // Add the Tree for the Top Account
    KMyMoneyAccountTreeItem *topAccountTree = buildAccountTree(m_qlistviewParentAccounts, topAccount, false);
    topAccountTree->setOpen(true);
    // Record the top account used to populate the list
    m_topAccount = topAccount;
  }
}

KMyMoneyAccountTreeItem* HierarchyPage::buildAccountTree
    ( KMyMoneyAccountTreeBase* parent
    , const MyMoneyAccount& account
    , bool open ) const
{
  // Recursively add child accounts to the list
  if ( account.accountType() == MyMoneyAccount::Investment)
    return NULL;
  KMyMoneyAccountTreeItem* childItem = new KMyMoneyAccountTreeItem( parent, account );
  if ( open )
    childItem->setOpen(true);
  for ( QCStringList::ConstIterator it = account.accountList().begin();
        it != account.accountList().end();
        ++it )
  {
    MyMoneyAccount acc = MyMoneyFile::instance()->account(*it);
    if(acc.isClosed())
      continue;
    if(acc.accountType() == MyMoneyAccount::Investment)
      continue;
    buildAccountTree( childItem, acc, open );
  }
  return childItem;
}

KMyMoneyAccountTreeItem* HierarchyPage::buildAccountTree
    ( KMyMoneyAccountTreeItem* parent
    , const MyMoneyAccount& account
    , bool open ) const
{
  // Recursively add child accounts to the list
  if ( account.accountType() == MyMoneyAccount::Investment)
    return NULL;
  KMyMoneyAccountTreeItem* childItem = new KMyMoneyAccountTreeItem( parent, account );
  if ( open )
    childItem->setOpen(true);
  for ( QCStringList::ConstIterator it = account.accountList().begin();
        it != account.accountList().end();
        ++it )
  {
    MyMoneyAccount acc = MyMoneyFile::instance()->account(*it);
    if(acc.isClosed())
      continue;
    if (account.accountType() == MyMoneyAccount::Investment)
      continue;
    buildAccountTree( childItem, acc, open );
  }
  return childItem;
}

KMyMoneyWizardPage* HierarchyPage::nextPage(void) const
{
  return m_wizard->m_accountSummaryPage;
}

const MyMoneyAccount& HierarchyPage::parentAccount(void)
{
  // TODO
  // Instead of returning the Parent Account we can simply
  // return the account associated with the current item
  // in the ListView
  KMyMoneyAccountTreeItem* item = dynamic_cast<KMyMoneyAccountTreeItem*>(m_qlistviewParentAccounts->currentItem());
  return dynamic_cast<const MyMoneyAccount&>(item->itemObject());
}

AccountSummaryPage::AccountSummaryPage(Wizard* wizard, const char* name) :
  KAccountSummaryPageDecl(wizard),
  WizardPage<Wizard>(StepFinish, this, wizard, name)
{
  m_dataList->setSorting(-1);
  m_dataList->setColumnWidthMode(1, QListView::Maximum);
  m_dataList->setResizeMode(QListView::LastColumn);
}

void AccountSummaryPage::enterPage(void)
{
  MyMoneyAccount acc = m_wizard->account();
  MyMoneySecurity sec = m_wizard->currency();
  // assign an id to the account inside the wizard which is required for a schedule
  // get the schedule and clear the id again in the wizards object.
  MyMoneyAccount tmp(QCString("Phony-ID"), acc);
  m_wizard->setAccount(tmp);
  MyMoneySchedule sch = m_wizard->schedule();
  m_wizard->setAccount(acc);

  m_dataList->clear();

  // Account data
  QListViewItem* group = new KMyMoneyCheckListItem(m_dataList, i18n("Account information"), QString(), QCString(), QCheckListItem::RadioButtonController);
  group->setOpen(true);
  QListViewItem* p;
  p = new KListViewItem(group, i18n("Name"), acc.name());
  if(!acc.isLoan())
    p = new KListViewItem(group, p, i18n("Subaccount of"),
                          m_wizard->parentAccount().name());
  if(acc.accountType() == MyMoneyAccount::AssetLoan)
    p = new KListViewItem(group, p, i18n("Type"), i18n("Loan"));
  else
    p = new KListViewItem(group, p, i18n("Type"), m_wizard->m_accountTypePage->m_typeSelection->currentText());
  p = new KListViewItem(group, p, i18n("Currency"), m_wizard->currency().name());
  p = new KListViewItem(group, p, i18n("Opening date"), KGlobal::locale()->formatDate(acc.openingDate()));
  if(m_wizard->currency().id() != MyMoneyFile::instance()->baseCurrency().id()) {
    p = new KListViewItem(group, p, i18n("Conversion rate"), m_wizard->conversionRate().rate(QCString()).formatMoney("", KMyMoneyGlobalSettings::pricePrecision()));
  }
  if(!acc.isLoan() || !m_wizard->openingBalance().isZero())
    p = new KListViewItem(group, p, i18n("Opening balance"), m_wizard->openingBalance().formatMoney(acc, sec));

  if(!m_wizard->m_institutionPage->institution().id().isEmpty()) {
    p = new KListViewItem(group, p, i18n("Institution"), m_wizard->m_institutionPage->institution().name());
    if(!acc.number().isEmpty()) {
      p = new KListViewItem(group, p, i18n("Number"), acc.number());
    }
    if(!acc.value("IBAN").isEmpty()) {
      p = new KListViewItem(group, p, i18n("IBAN"), acc.value("IBAN"));
    }
  }

  if(acc.accountType() == MyMoneyAccount::Investment) {
    if(m_wizard->m_brokeragepage->m_createBrokerageButton->isChecked()) {
      group = new KMyMoneyCheckListItem(m_dataList, group, i18n("Brokerage Account"), QString(), QCString(), QCheckListItem::RadioButtonController);
      group->setOpen(true);
      p = new KListViewItem(group, p, i18n("Name"), QString("%1 (Brokerage)").arg(acc.name()));
      p = new KListViewItem(group, p, i18n("Currency"), m_wizard->m_brokeragepage->m_brokerageCurrency->security().name());
      if(m_wizard->m_brokeragepage->m_accountNumber->isEnabled() && !m_wizard->m_brokeragepage->m_accountNumber->text().isEmpty())
        p = new KListViewItem(group, p, i18n("Number"), m_wizard->m_brokeragepage->m_accountNumber->text());
      if(m_wizard->m_brokeragepage->m_iban->isEnabled() && !m_wizard->m_brokeragepage->m_iban->text().isEmpty())
        p = new KListViewItem(group, p, i18n("IBAN"), m_wizard->m_brokeragepage->m_iban->text());
    }
  }

  // Loan
  if(acc.isLoan()) {
    group = new KMyMoneyCheckListItem(m_dataList, group, i18n("Loan information"), QString(), QCString(), QCheckListItem::RadioButtonController);
    group->setOpen(true);
    if(m_wizard->moneyBorrowed()) {
      p = new KListViewItem(group, p, i18n("Amount borrowed"), m_wizard->m_loanDetailsPage->m_loanAmount->value().formatMoney(m_wizard->currency().tradingSymbol(), m_wizard->precision()));
    } else {
      p = new KListViewItem(group, p, i18n("Amount lent"), m_wizard->m_loanDetailsPage->m_loanAmount->value().formatMoney(m_wizard->currency().tradingSymbol(), m_wizard->precision()));
    }
    p = new KListViewItem(group, p, i18n("Interest rate"), QString("%1 %").arg(m_wizard->m_loanDetailsPage->m_interestRate->value().formatMoney("", 3)));
    p = new KListViewItem(group, p, i18n("Interest rate is"), m_wizard->m_generalLoanInfoPage->m_interestType->currentText());
    p = new KListViewItem(group, p, i18n("Principal and interest"), m_wizard->m_loanDetailsPage->m_paymentAmount->value().formatMoney(acc, sec));
    p = new KListViewItem(group, p, i18n("Additional fees"), m_wizard->m_loanPaymentPage->additionalFees().formatMoney(acc, sec));
    p = new KListViewItem(group, p, i18n("Payment frequency"), m_wizard->m_generalLoanInfoPage->m_paymentFrequency->currentText());
    p = new KListViewItem(group, p, i18n("Payment account"), m_wizard->m_loanSchedulePage->m_paymentAccount->currentText());

    if(!m_wizard->m_loanPayoutPage->m_noPayoutTransaction->isChecked() && m_wizard->openingBalance().isZero()) {
      group = new KMyMoneyCheckListItem(m_dataList, group, i18n("Payout information"), QString(), QCString(), QCheckListItem::RadioButtonController);
      group->setOpen(true);
      if(m_wizard->m_loanPayoutPage->m_refinanceLoan->isChecked()) {
        p = new KListViewItem(group, p, i18n("Refinance"), m_wizard->m_loanPayoutPage->m_loanAccount->currentText());
      } else {
        if(m_wizard->moneyBorrowed())
          p = new KListViewItem(group, p, i18n("Transfer amount to"), m_wizard->m_loanPayoutPage->m_assetAccount->currentText());
        else
          p = new KListViewItem(group, p, i18n("Transfer amount from"), m_wizard->m_loanPayoutPage->m_assetAccount->currentText());
      }
      p = new KListViewItem(group, p, i18n("Payment date"), KGlobal::locale()->formatDate(m_wizard->m_loanPayoutPage->m_payoutDate->date()));
    }
  }

  // Schedule
  if(!(sch == MyMoneySchedule())) {
    group = new KMyMoneyCheckListItem(m_dataList, group, i18n("Schedule information"), QString(), QCString(), QCheckListItem::RadioButtonController);
    group->setOpen(true);
    p = new KListViewItem(group, i18n("Name"), sch.name());
    if(acc.accountType() == MyMoneyAccount::CreditCard) {
      MyMoneyAccount paymentAccount = MyMoneyFile::instance()->account(m_wizard->m_schedulePage->m_paymentAccount->selectedItem());
      p = new KListViewItem(group, p, i18n("Occurence"), i18n("Monthly"));
      p = new KListViewItem(group, p, i18n("Paid from"), paymentAccount.name());
      p = new KListViewItem(group, p, i18n("Pay to"), m_wizard->m_schedulePage->m_payee->currentText());
      p = new KListViewItem(group, p, i18n("Amount"), m_wizard->m_schedulePage->m_amount->value().formatMoney(acc, sec));
      p = new KListViewItem(group, p, i18n("First payment due"), KGlobal::locale()->formatDate(sch.nextDueDate()));
      p = new KListViewItem(group, p, i18n("Payment method"), m_wizard->m_schedulePage->m_method->currentText());
    }
    if(acc.isLoan()) {
      p = new KListViewItem(group, p, i18n("Occurence"), m_wizard->m_generalLoanInfoPage->m_paymentFrequency->currentText());
      p = new KListViewItem(group, p, i18n("Amount"), (m_wizard->m_loanPaymentPage->basePayment() + m_wizard->m_loanPaymentPage->additionalFees()).formatMoney(acc, sec));
      p = new KListViewItem(group, p, i18n("First payment due"), KGlobal::locale()->formatDate(m_wizard->m_loanSchedulePage->firstPaymentDueDate()));
    }
  }
}

#include "knewaccountwizard.moc"
