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

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>
#include <klistbox.h>
#include <klineedit.h>
#include <kmessagebox.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/kmymoneycombo.h>
#include <kmymoney/kmymoneyedit.h>
#include <kmymoney/kmymoneycategory.h>
#include <kmymoney/mymoneyfile.h>
#include <kmymoney/kmymoneycurrencyselector.h>
#include <kmymoney/kmymoneyaccountselector.h>

#include "knewaccountwizard.h"
#include "knewaccountwizard_p.h"
#include <kmymoney/kguiutils.h>

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
  addStep(i18n("Finish"));

  m_institutionPage = new InstitutionPage(this);
  m_accountTypePage = new AccountTypePage(this);
  m_openingPage = new OpeningPage(this);
  m_schedulePage = new CreditCardSchedulePage(this);

  setFirstPage(m_institutionPage);
}

void NewAccountWizard::Wizard::setAccount(const MyMoneyAccount& acc)
{
  m_account = acc;
}

const MyMoneyAccount& NewAccountWizard::Wizard::account(void)
{
  m_account = MyMoneyAccount();
  m_account.setName(m_accountTypePage->m_accountName->text());
  m_account.setOpeningDate(m_openingPage->m_openingDate->date());
  m_account.setAccountType(m_accountTypePage->accountType());
  m_account.setInstitutionId(m_institutionPage->institution().id());
  m_account.setNumber(m_institutionPage->m_accountNumber->text());
  m_account.setValue("IBAN", m_institutionPage->m_iban->text());

  m_account.setCurrencyId(m_openingPage->currencyId());

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
    m_schedule.setPaymentType(static_cast<MyMoneySchedule::paymentTypeE>(m_schedulePage->m_method->item()));
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
  connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotLoadWidget()));
  connect(m_newInstitutionButton, SIGNAL(clicked()), this, SLOT(slotNewInstitution()));
  connect(m_institutionComboBox, SIGNAL(activated(int)), this, SLOT(slotSelectInstitution(int)));

  slotLoadWidget();
  m_institutionComboBox->setCurrentItem(0);
  slotSelectInstitution(0);
}

InstitutionPage::~InstitutionPage()
{
  delete d;
}

void InstitutionPage::slotLoadWidget(void)
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
  m_typeSelection->setCurrentItem(0);

  m_mandatoryGroup = new kMandatoryFieldGroup(this);
  m_mandatoryGroup->add(m_accountName);
  connect(m_mandatoryGroup, SIGNAL(stateChanged()), object(), SIGNAL(completeStateChanged()));
}

KMyMoneyWizardPage* AccountTypePage::nextPage(void) const
{
  return m_wizard->m_openingPage;
}

bool AccountTypePage::isComplete(void) const
{
  bool rc = true;
  QString msg = i18n("Continue with next page");
  if(m_accountName->text().isEmpty()) {
    rc = false;
    msg = i18n("No account name supplied");
  }
  QToolTip::add(m_wizard->m_nextButton, msg);
  return rc;
}

MyMoneyAccount::accountTypeE AccountTypePage::accountType(void) const
{
  MyMoneyAccount::accountTypeE rc = MyMoneyAccount::UnknownAccountType;

  switch(m_typeSelection->currentItem()) {
    case 0:
      rc = MyMoneyAccount::Checkings;
      break;
    case 1:
      rc = MyMoneyAccount::Savings;
      break;
    case 2:
      rc = MyMoneyAccount::CreditCard;
      break;
    case 3:
      rc = MyMoneyAccount::Cash;
      break;
    case 4:
      rc = MyMoneyAccount::Loan;
      break;
    case 5:
      rc = MyMoneyAccount::Investment;
      break;
    case 6:
      rc = MyMoneyAccount::Asset;
      break;
    case 7:
      rc = MyMoneyAccount::Liability;
      break;
  }
  return rc;
}

OpeningPage::OpeningPage(Wizard* wizard, const char* name) :
  KOpeningPageDecl(wizard),
  WizardPage<Wizard>(3, this, wizard, name)
{
  connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotLoadWidget()));
  m_currencyComboBox->setSecurity(MyMoneyFile::instance()->baseCurrency());
}

void OpeningPage::slotLoadWidget(void)
{
  m_currencyComboBox->update(QCString("x"));
}

KMyMoneyWizardPage* OpeningPage::nextPage(void) const
{
  return (m_wizard->m_accountTypePage->accountType() == MyMoneyAccount::CreditCard) ? m_wizard->m_schedulePage : 0;
}

const QCString& OpeningPage::currencyId(void) const
{
  return m_currencyComboBox->security().id();
}

CreditCardSchedulePage::CreditCardSchedulePage(Wizard* wizard, const char* name) :
  KSchedulePageDecl(wizard),
  WizardPage<Wizard>(4, this, wizard, name)
{
  m_mandatoryGroup = new kMandatoryFieldGroup(this);
  m_mandatoryGroup->add(m_name);
  m_mandatoryGroup->add(m_payee);
  m_mandatoryGroup->add(m_amount->lineedit());
  m_mandatoryGroup->add(m_paymentAccount);
  connect(m_mandatoryGroup, SIGNAL(stateChanged()), object(), SIGNAL(completeStateChanged()));
  connect(m_paymentAccount, SIGNAL(itemSelected(const QCString&)), object(), SIGNAL(completeStateChanged()));
  connect(m_payee, SIGNAL(itemSelected(const QCString&)), object(), SIGNAL(completeStateChanged()));
  connect(m_date, SIGNAL(dateChanged(const QDate&)), object(), SIGNAL(completeStateChanged()));

  m_reminderCheckBox->setChecked(true);
  connect(m_reminderCheckBox, SIGNAL(toggled(bool)), object(), SIGNAL(completeStateChanged()));

  connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotLoadWidget()));

  m_method->insertItem(i18n("Other"), MyMoneySchedule::STYPE_OTHER);
  m_method->insertItem(i18n("Write check"), MyMoneySchedule::STYPE_WRITECHEQUE);
  m_method->insertItem(i18n("Manual deposit"), MyMoneySchedule::STYPE_MANUALDEPOSIT);
  m_method->insertItem(i18n("Direct deposit"), MyMoneySchedule::STYPE_DIRECTDEPOSIT);
  m_method->insertItem(i18n("Direct debit"), MyMoneySchedule::STYPE_DIRECTDEBIT);
  m_method->setItem(MyMoneySchedule::STYPE_DIRECTDEBIT);

  slotLoadWidget();
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
    if(m_date->date() > m_wizard->m_openingPage->m_openingDate->date()) {
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

void CreditCardSchedulePage::slotLoadWidget(void)
{
  AccountSet set;
  set.addAccountGroup(MyMoneyAccount::Asset);
  set.load(m_paymentAccount->selector());

  m_payee->loadPayees(MyMoneyFile::instance()->payeeList());
}


#include "knewaccountwizard.moc"
