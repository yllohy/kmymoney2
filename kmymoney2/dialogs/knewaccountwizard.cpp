/***************************************************************************
                          knewaccountwizard.cpp  -  description
                             -------------------
    begin                : Thu Jul 4 2002
    copyright            : (C) 2000-2002 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// ----------------------------------------------------------------------------
// QT Includes

#include <qcheckbox.h>
#include <qheader.h>
#include <qlabel.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>
#include <klistbox.h>
#include <kcombobox.h>
#include <kpushbutton.h>
#include <ktextbrowser.h>
#include <klineedit.h>
#include <kconfig.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "../widgets/kmymoneyedit.h"
#include "../widgets/kmymoneydateinput.h"
#include "../widgets/kmymoneycombo.h"
#include "../widgets/kmymoneycategory.h"
#include "../widgets/kmymoneypayee.h"
#include "../widgets/kmymoneycurrencyselector.h"

#include "../mymoney/mymoneyinstitution.h"
#include "../mymoney/mymoneyfile.h"

#include "../kmymoneyutils.h"

#include "ieditscheduledialog.h"
#include "knewaccountwizard.h"
#include "knewloanwizard.h"

KNewAccountWizard::KNewAccountWizard(QWidget *parent, const char *name )
  : KNewAccountWizardDecl(parent,name,true),
    m_accountType(MyMoneyAccount::Checkings)
{
  // keep title of payment page
  m_accountPaymentPageTitle = title(accountPaymentPage);

  accountListView->setRootIsDecorated(true);
  accountListView->setAllColumnsShowFocus(true);
  accountListView->addColumn(i18n("Account"));
  accountListView->setMultiSelection(false);
  accountListView->header()->setResizeEnabled(false);
  accountListView->setColumnWidthMode(0, QListView::Manual);
  accountListView->setResizeMode(QListView::AllColumns);

  connect(newInstitutionButton, SIGNAL(clicked()), this, SLOT(slotNewInstitution()));
  connect(accountTypeListBox, SIGNAL(highlighted(const QString &)), this, SLOT(slotAccountType(const QString &)));
  connect(reminderCheckBox, SIGNAL(toggled(bool)), estimateFrame, SLOT(setEnabled(bool)));

  connect(accountName, SIGNAL(textChanged(const QString&)), this, SLOT(slotCheckPageFinished()));
  connect(reminderCheckBox, SIGNAL(stateChanged(int)), this, SLOT(slotCheckPageFinished()));
  connect(m_name, SIGNAL(textChanged(const QString&)), this, SLOT(slotCheckPageFinished()));
  connect(m_payee, SIGNAL(textChanged(const QString&)), this, SLOT(slotCheckPageFinished()));
  connect(m_amount, SIGNAL(textChanged(const QString&)), this, SLOT(slotCheckPageFinished()));
  
  connect(m_payee, SIGNAL(newPayee(const QString&)), this, SLOT(slotNewPayee(const QString&)));
  
  // always select the first item and show the appropriate note
  accountTypeListBox->setCurrentItem(0);

  m_name->setFocus();
}

KNewAccountWizard::~KNewAccountWizard()
{
}

void KNewAccountWizard::next()
{
  switch(m_accountType) {
    case MyMoneyAccount::Cash:
    case MyMoneyAccount::Asset:
      if(indexOf(accountPaymentPage) != -1) {
        removePage(accountPaymentPage);
      }
      setAppropriate(accountNumberPage, false);
      setFinishEnabled(accountDetailsPage, true);
      break;

    case MyMoneyAccount::CreditCard:
      if(indexOf(accountPaymentPage) == -1) {
        loadPaymentMethods();
        addPage(accountPaymentPage, m_accountPaymentPageTitle);
      }
      setAppropriate(accountPaymentPage, true);
      setFinishEnabled(accountPaymentPage, true);
      setFinishEnabled(accountDetailsPage, false);
      break;

    default:
      setAppropriate(accountNumberPage, institutionComboBox->currentText() != "");
      if(indexOf(accountPaymentPage) != -1) {
        removePage(accountPaymentPage);
      }
      setFinishEnabled(accountDetailsPage, true);
      break;
  }

  if(currentPage() == accountTypePage && m_accountType == MyMoneyAccount::Loan) {
    int rc;
    
    // we logically never come back to this wizard, so we hide it.
    hide();

    KNewLoanWizard* loanWizard = new KNewLoanWizard(this);
    if((rc = loanWizard->exec()) == QDialog::Accepted) {
      m_account = loanWizard->account();
      // copy relevant data into my widgets so that
      // accept() will not modify anything later on.
      m_accountType = m_account.accountType();
      accountName->setText(m_account.name());
      accountNumber->setText(QString());
      openingBalance->setText(m_account.openingBalance().formatMoney());
      openingDate->setDate(m_account.openingDate());
      preferredAccount->setChecked(false);

      m_schedule = loanWizard->schedule();
    }
    delete loanWizard;

    if(rc == QDialog::Accepted)
      accept();
    else
      reject();
      
  } else {
    KNewAccountWizardDecl::next();
    
    // setup the availability of widgets on the selected page
    slotCheckPageFinished();
  }
}

void KNewAccountWizard::accept()
{
  if(appropriate(accountNumberPage) == false)
    accountNumber->setText(QString());

  // setup account
  m_account.setAccountType(m_accountType);
  m_account.setName(accountName->text());
  m_account.setNumber(accountNumber->text());
  m_account.setCurrencyId(m_currencyComboBox->currency().id());

  if(!institutionComboBox->currentText().isEmpty()) {
    QValueList<MyMoneyInstitution> list;
    QValueList<MyMoneyInstitution>::ConstIterator it;

    list = MyMoneyFile::instance()->institutionList();
    for(it = list.begin(); it != list.end(); ++it)
      if((*it).name() == institutionComboBox->currentText())
        m_account.setInstitutionId((*it).id());
  }
  m_account.setOpeningBalance(MyMoneyMoney(openingBalance->text()));
  m_account.setOpeningDate(openingDate->getQDate());
  if(preferredAccount->isChecked())
    m_account.setValue("PreferredAccount", "Yes");

  // setup parent account
  MyMoneyAccount::accountTypeE type = MyMoneyFile::instance()->accountGroup(m_accountType);
  switch(type) {
    default:
    case MyMoneyAccount::Asset:
      m_parent = MyMoneyFile::instance()->asset();
      break;
    case MyMoneyAccount::Liability:
      m_parent = MyMoneyFile::instance()->liability();
      break;
  }

  if (m_accountType == MyMoneyAccount::CreditCard && reminderCheckBox->isChecked())
  {
    // Make sure we have input
    if (m_amount->text().isEmpty())
    {
      KMessageBox::error(this, i18n("Please enter the payment amount."));
      m_amount->setFocus();
      return;
    }

    if (m_name->text().isEmpty())
    {
      KMessageBox::error(this, i18n("Please enter the schedule name."));
      m_name->setFocus();
      return;
    }

    if (m_payee->text().isEmpty())
    {
      KMessageBox::error(this, i18n("Please enter the payee name."));
      m_payee->setFocus();
      return;
    }
    
    KAccountListItem *item = (KAccountListItem*)accountListView->selectedItem();
    if (!item)
    {
      KMessageBox::error(this, i18n("Please select the account."));
      accountListView->setFocus();
      return;
    }

    // Create the schedule transaction
    QCString payeeId;
  
    try
    {
      payeeId = MyMoneyFile::instance()->payeeByName(m_payee->text()).id();
    }
    catch (MyMoneyException *e)
    {
      MyMoneyPayee payee(m_payee->text());
      MyMoneyFile::instance()->addPayee(payee);
      payeeId = payee.id();
      delete e;
    }
    
    MyMoneySplit s1, s2;
    s1.setValue(m_amount->getMoneyValue());
    s2.setValue(-s1.value());
    s1.setAction(MyMoneySplit::ActionTransfer);
    s2.setAction(MyMoneySplit::ActionTransfer);
    s1.setAccountId(QCString()/*this_account?*/);  // This needs to be set by caller  (see KMyMoneyView::accountNew)
    s2.setAccountId(item->accountID());
    s1.setPayeeId(payeeId);
    s2.setPayeeId(payeeId);

    MyMoneyTransaction t;
    t.addSplit(s1);
    t.addSplit(s2);

    MyMoneySchedule::paymentTypeE paymentType;
    
    switch (m_method->currentItem())
    {
      case 0:
        paymentType = MyMoneySchedule::STYPE_DIRECTDEBIT;
        break;
      case 1:
        paymentType = MyMoneySchedule::STYPE_DIRECTDEPOSIT;
        break;
      case 2:
        paymentType = MyMoneySchedule::STYPE_MANUALDEPOSIT;
        break;
      case 3:
        paymentType = MyMoneySchedule::STYPE_WRITECHEQUE;
        break;
      case 4:
        paymentType = MyMoneySchedule::STYPE_OTHER;
        break;
      default:
        paymentType = MyMoneySchedule::STYPE_DIRECTDEBIT;
        break;
    }

    m_schedule = MyMoneySchedule(  m_name->text(),
                                   MyMoneySchedule::TYPE_TRANSFER,
                                   MyMoneySchedule::OCCUR_MONTHLY,
                                   paymentType,
                                   m_date->getQDate(),
                                   QDate(),
                                   false,
                                   false);

    m_schedule.setTransaction(t);
  }

  KNewAccountWizardDecl::accept();
}

int KNewAccountWizard::exec()
{
  int rc;
  
  accountListView->header()->setFont(KMyMoneyUtils::headerFont());

  // currently, we don't have help :-(
  setHelpEnabled(institutionPage, false);
  setHelpEnabled(accountTypePage, false);
  setHelpEnabled(accountNamePage, false);
  setHelpEnabled(accountNumberPage, false);
  setHelpEnabled(accountDetailsPage, false);
  setHelpEnabled(accountPaymentPage, false);

  setFinishEnabled(institutionPage, false);
  setFinishEnabled(accountTypePage, false);
  setFinishEnabled(accountNamePage, false);
  setFinishEnabled(accountNumberPage, false);
  setFinishEnabled(accountDetailsPage, false);
  setFinishEnabled(accountPaymentPage, true);

  // always start on first page
  showPage(institutionPage);

  setBackEnabled(institutionPage, false);

  loadInstitutionList();
  loadAccountTypes();
  loadAccountList();

  // always check the payment reminder
  reminderCheckBox->setChecked(true);
  estimateFrame->setEnabled(true);

  // reset everything else if not preset
  accountNumber->setText(QString());

  rc = KNewAccountWizardDecl::exec();

  // always select the first item and show the appropriate note
  accountTypeListBox->setCurrentItem(0);

  return rc;
}

void KNewAccountWizard::loadInstitutionList(void)
{
  institutionComboBox->clear();

  QValueList<MyMoneyInstitution> list;
  QValueList<MyMoneyInstitution>::ConstIterator it;

  institutionComboBox->insertItem(QString());
  list = MyMoneyFile::instance()->institutionList();
  for(it = list.begin(); it != list.end(); ++it)
  {
    institutionComboBox->insertItem((*it).name());
  }

  institutionComboBox->setCurrentText(m_institution.name());
}

void KNewAccountWizard::loadSubAccountList(KListView* parent, const QCString& accountId)
{
  QValueList<MyMoneyAccount>::ConstIterator it;

  it = findAccount(accountId);
  KAccountListItem *topLevelAccount = new KAccountListItem(parent, *it);

  QCStringList::ConstIterator it_s;
  for(it_s = (*it).accountList().begin(); it_s != (*it).accountList().end(); ++it_s) {
    loadSubAccountList(topLevelAccount, (*it_s));
  }
}

void KNewAccountWizard::loadSubAccountList(KAccountListItem* parent, const QCString& accountId)
{
  QValueList<MyMoneyAccount>::ConstIterator it;

  it = findAccount(accountId);
  KAccountListItem *topLevelAccount = new KAccountListItem(parent, *it);

  QCStringList::ConstIterator it_s;
  for(it_s = (*it).accountList().begin(); it_s != (*it).accountList().end(); ++it_s) {
    loadSubAccountList(topLevelAccount, (*it_s));
  }
}

QValueList<MyMoneyAccount>::ConstIterator KNewAccountWizard::findAccount(const QCString& accountId) const
{
  QValueList<MyMoneyAccount>::ConstIterator it;

  for(it = m_accountList.begin(); it != m_accountList.end(); ++it) {
    if((*it).id() == accountId) {
      break;
    }
  }
  return it;
}

void KNewAccountWizard::loadAccountList(void)
{
  accountListView->clear();

  m_accountList = MyMoneyFile::instance()->accountList();

  MyMoneyAccount acc = MyMoneyFile::instance()->asset();

  QCStringList::ConstIterator it_s;
  for(it_s = acc.accountList().begin(); it_s != acc.accountList().end(); ++it_s) {
    loadSubAccountList(accountListView, (*it_s));
  }
}

void KNewAccountWizard::loadAccountTypes(void)
{
  accountTypeListBox->clear();

  accountTypeListBox->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Checkings));
  accountTypeListBox->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Savings));
  accountTypeListBox->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::CreditCard));
  accountTypeListBox->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Cash));
  accountTypeListBox->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Loan));
  
/*
  // accountTypeListBox->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::AssetLoan));
  accountTypeListBox->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::MoneyMarket));
  accountTypeListBox->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Currency));
*/
  accountTypeListBox->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Investment));
  accountTypeListBox->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Asset));
  accountTypeListBox->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Liability));
}

void KNewAccountWizard::slotNewInstitution(void)
{
  emit newInstitutionClicked();
  loadInstitutionList();
}

void KNewAccountWizard::slotAccountType(const QString& sel)
{
  QString txt = "<h2><center>" + sel + "</center></h2><p><p>";

  if(sel == KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Checkings)) {
    txt += i18n(
      "Use the checkings account type to manage "
      "activities on your checkings account e.g. payments, cheques and cash card "
      "purchases."
    );
    m_accountType = MyMoneyAccount::Checkings;

  } else if(sel == KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Savings)) {
    txt += i18n(
      "Use the savings account type to manage "
      "activities on your savings account."
    );
    m_accountType = MyMoneyAccount::Savings;

  } else if(sel == KMyMoneyUtils::accountTypeToString(MyMoneyAccount::CreditCard)) {
    txt += i18n(
      "Use the credit card account type to manage "
      "activities on your credit card."
    );
    m_accountType = MyMoneyAccount::CreditCard;

  } else if(sel == KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Cash)) {
    txt += i18n(
      "Use the cash account type to manage "
      "activities in your wallet."
    );
    m_accountType = MyMoneyAccount::Cash;

  } else if(sel == KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Asset)) {
    txt += i18n(
      "Use the asset account type to manage "
      "assets (e.g. your house, car or art collection)."
    );
    m_accountType = MyMoneyAccount::Asset;

  } else if(sel == KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Liability)) {
    txt += i18n(
      "Use the liability account type to manage "
      "any type of liability except amortization loans. Use it for "
      "taxes you owe or money you borrowed from friends. For amortization loans "
      "like mortgages you should create a loan account."
    );
    m_accountType = MyMoneyAccount::Liability;
    
  } else if(sel == KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Loan)) {
    txt += i18n(
      "Use the loan account type to manage amortization loans "
      "(e.g. mortgages, car loan, money you lend, private loans etc.)."
    );
    m_accountType = MyMoneyAccount::Loan;
/*
  } else if(sel == KMyMoneyUtils::accountTypeToString(MyMoneyAccount::AssetLoan)) {
    txt += i18n(
      "Use the investment loan account type to manage money you lend to people "
      "and from which you expect an interest payment. (e.g. private loans, etc.)."
    );
    m_accountType = MyMoneyAccount::Loan;
*/
  } else if(sel == KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Investment)) {
    txt += i18n(
      "Use the investment account to manage your stock and mutual fund account."
    );
    m_accountType = MyMoneyAccount::Investment;
  } else {
    txt += i18n("Explanation is not yet available! UnknownAccountType will be set");
    m_accountType = MyMoneyAccount::UnknownAccountType;
  }
  explanationTextBox->setText(txt);
}

void KNewAccountWizard::setAccountName(const QString& name)
{
  accountName->setText(name);
}

void KNewAccountWizard::setOpeningBalance(const MyMoneyMoney& balance)
{
  openingBalance->setText(balance.formatMoney());
}

void KNewAccountWizard::setOpeningDate(const QDate& date)
{
  if(date.isValid())
    openingDate->setDate(date);
  else
    openingDate->setDate(QDate::currentDate());
}

void KNewAccountWizard::setAccountType(const MyMoneyAccount::accountTypeE type)
{
  int i;
  
  for(i = accountTypeListBox->count()-1; i > 0; --i) {
    if(accountTypeListBox->text(i) == KMyMoneyUtils::accountTypeToString(type))
      break;
  }
  // FIXME: I have no idea how to get the selection bar back to the window.
  accountTypeListBox->setSelected(i, true);
  // accountTypeListBox->setCurrentItem(i);
}

void KNewAccountWizard::loadPaymentMethods()
{
  m_method->clear();
  m_method->insertItem(i18n("Direct Debit"));
  m_method->insertItem(i18n("Direct Deposit"));
  m_method->insertItem(i18n("Manual Deposit"));
  m_method->insertItem(i18n("Write Cheque"));
  m_method->insertItem(i18n("Other"));
}

void KNewAccountWizard::showPage(QWidget* page)
{
  if (page == accountTypePage)
    setAccountType(m_accountType);

  KNewAccountWizardDecl::showPage(page);
}

void KNewAccountWizard::slotCheckPageFinished(void)
{
  nextButton()->setEnabled(true);
  finishButton()->setEnabled(true);
  
  if(currentPage() == accountNamePage) {
    if(accountName->text().isEmpty())
      nextButton()->setEnabled(false);
      
  } else if(currentPage() == accountPaymentPage) {
    if(reminderCheckBox->isChecked()) {
      if(m_amount->text().isEmpty()
      || m_name->text().isEmpty()
      || m_payee->text().isEmpty()
      || accountListView->selectedItem() == 0) {
        finishButton()->setEnabled(false);
      }
    }
  }
}

void KNewAccountWizard::slotNewPayee(const QString& payeeName)
{
  KMyMoneyUtils::newPayee(this, m_payee, payeeName);
}

