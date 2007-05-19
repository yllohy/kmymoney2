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
#include <qradiobutton.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kapplication.h>
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
#include "../widgets/kmymoneycurrencyselector.h"

#include "../mymoney/mymoneyinstitution.h"
#include "../mymoney/mymoneyfile.h"

#include "../kmymoneyutils.h"

#include "ieditscheduledialog.h"
#include "knewaccountwizard.h"
#include "knewloanwizard.h"
#include "kcurrencyeditdlg.h"

KNewAccountWizard::KNewAccountWizard(QWidget *parent, const char *name )
  : KNewAccountWizardDecl(parent,name,true),
    m_accountType(MyMoneyAccount::Checkings)
{
  // keep title of payment page
  m_accountPaymentPageTitle = title(accountPaymentPage);

  // We don't need to add the default into the list (see ::help() why)
  m_helpAnchor[institutionPage] = QString("firsttime-accwiz1");
  m_helpAnchor[accountTypePage] = QString("firsttime-accwiz2");
  m_helpAnchor[accountNamePage] = QString("firsttime-accwiz3");
  m_helpAnchor[accountNumberPage] = QString("firsttime-accwiz4");
  //m_helpAnchor[brokerageAccountPage] = QString("");
  m_helpAnchor[accountDetailsPage] = QString("firsttime-accwiz5");
  m_helpAnchor[accountPaymentPage] = QString("firsttime-accwiz5.1");
  //m_helpAnchor[summaryPage] = QString("");

  m_accountListView->setRootIsDecorated(true);
  m_accountListView->setAllColumnsShowFocus(true);
  m_accountListView->addColumn(i18n("Account"));
  m_accountListView->setMultiSelection(false);
  m_accountListView->header()->setResizeEnabled(false);
  m_accountListView->setColumnWidthMode(0, QListView::Manual);
  m_accountListView->setResizeMode(QListView::AllColumns);

  m_currencyComboBox->update(QCString());

  connect(m_newInstitutionButton, SIGNAL(clicked()), this, SLOT(slotNewInstitution()));
  connect(m_accountTypeListBox, SIGNAL(highlighted(const QString &)), this, SLOT(slotAccountType(const QString &)));
  connect(reminderCheckBox, SIGNAL(toggled(bool)), estimateFrame, SLOT(setEnabled(bool)));

  connect(m_accountName, SIGNAL(textChanged(const QString&)), this, SLOT(slotCheckPageFinished()));
  connect(reminderCheckBox, SIGNAL(stateChanged(int)), this, SLOT(slotCheckPageFinished()));
  connect(m_name, SIGNAL(textChanged(const QString&)), this, SLOT(slotCheckPageFinished()));
  connect(m_payee, SIGNAL(textChanged(const QString&)), this, SLOT(slotCheckPageFinished()));
  connect(m_amount, SIGNAL(textChanged(const QString&)), this, SLOT(slotCheckPageFinished()));

  connect(m_payee, SIGNAL(createItem(const QString&, QCString&)), this, SIGNAL(createPayee(const QString&, QCString&)));
  connect(m_currencyComboBox, SIGNAL(activated(int)), this, SLOT(slotCurrencyChanged(int)));
  connect(m_priceButton, SIGNAL(clicked()), this, SLOT(slotPriceUpdate()));

  connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotReloadEditWidgets()));

  // always select the first item and show the appropriate note
  loadAccountTypes();
  loadPaymentMethods();

  slotReloadEditWidgets();

  m_accountTypeListBox->setCurrentItem(0);
  setOpeningBalance(MyMoneyMoney(0,1));
  setAccountName(QString());

  m_name->setFocus();

  slotCurrencyChanged(0);
}

KNewAccountWizard::~KNewAccountWizard()
{
}

void KNewAccountWizard::slotReloadEditWidgets(void)
{
#if 0
  // TODO: reload the account and category widgets
  // reload category widget
  KMyMoneyCategory* category = dynamic_cast<KMyMoneyCategory*>(m_editWidgets["category"]);
  QCString categoryId;
  category->selectedItem(categoryId);

  AccountSet aSet(m_objects);
  aSet.addAccountGroup(MyMoneyAccount::Asset);
  aSet.addAccountGroup(MyMoneyAccount::Liability);
  aSet.addAccountGroup(MyMoneyAccount::Income);
  aSet.addAccountGroup(MyMoneyAccount::Expense);
  if(KMyMoneySettings::expertMode())
    aSet.addAccountGroup(MyMoneyAccount::Equity);
  aSet.load(category->selector());
#endif

  // reload payee widget
  QCString payeeId;
  m_payee->selectedItem(payeeId);

  m_payee->loadPayees(MyMoneyFile::instance()->payeeList());

  if(!payeeId.isEmpty()) {
    m_payee->setSelectedItem(payeeId);
  }
}

void KNewAccountWizard::next()
{
  openingBalanceLabel->setText(i18n("What is the opening balance and date of this account?"));

  setAppropriate(accountDetailsPage, true);
  setAppropriate(accountPaymentPage, false);
  setAppropriate(brokerageAccountPage, false);

  switch(m_accountType) {
    case MyMoneyAccount::Cash:
    case MyMoneyAccount::Asset:
      setAppropriate(accountNumberPage, false);
      break;

    case MyMoneyAccount::Investment:
      openingBalanceLabel->setText(i18n("What is the opening balance and date of this brokerage account?"));
      setAppropriate(brokerageAccountPage, true);
      setAppropriate(accountNumberPage, false);
      break;

    case MyMoneyAccount::CreditCard:
      setAppropriate(accountPaymentPage, true);
      break;

    default:
      setAppropriate(accountNumberPage, m_institutionComboBox->currentText() != "");
      break;
  }

  if(currentPage() == accountTypePage && m_accountType == MyMoneyAccount::Loan) {
    int rc;

    // we logically never come back to this wizard, so we hide it.
    hide();

    KNewLoanWizard* loanWizard = new KNewLoanWizard(0);
    connect(loanWizard, SIGNAL(newCategory(MyMoneyAccount&)), this, SIGNAL(newCategory(MyMoneyAccount&)));
    connect(loanWizard, SIGNAL(createPayee(const QString&, QCString&)), this, SIGNAL(createPayee(const QString&, QCString&)));

    if((rc = loanWizard->exec()) == QDialog::Accepted) {
      m_account = loanWizard->account();
      // copy relevant data into my widgets so that
      // accept() will not modify anything later on.
      m_accountType = m_account.accountType();
      m_accountName->setText(m_account.name());
      m_accountNumber->setText(QString());
      // FIXME: opening balance not support anymore
      m_openingBalance->setValue(m_account.openingBalance());
      m_openingDate->setDate(m_account.openingDate());
      m_preferredAccount->setChecked(false);

      m_schedule = loanWizard->schedule();
      if(loanWizard->initialPaymentDate().isValid()
      && !loanWizard->initialPaymentAccount().isEmpty()) {
        m_account.setValue("kmm-loan-payment-date", loanWizard->initialPaymentDate().toString(Qt::ISODate));
        m_account.setValue("kmm-loan-payment-acc", loanWizard->initialPaymentAccount());
      }
    }
    delete loanWizard;

    if(rc == QDialog::Accepted)
      accept();
    else
      reject();

  } else {
    if(currentPage() == brokerageAccountPage) {
      setAppropriate(accountDetailsPage, m_brokerageYesButton->isChecked());
    }

    KNewAccountWizardDecl::next();

    if(currentPage() == accountNamePage) {
      m_accountName->setFocus();

    } else if(currentPage() == accountNumberPage) {
      m_accountNumber->setFocus();

    } else if(currentPage() == accountDetailsPage) {
      KLineEdit* edit = m_openingBalance->lineedit();
      if(edit->hasFocus() && !edit->text().isEmpty() && !edit->hasSelectedText())
        edit->selectAll();
    }
    // setup the availability of widgets on the selected page
    slotCheckPageFinished();
  }
}

void KNewAccountWizard::accept()
{
  if(appropriate(accountNumberPage) == false)
    m_accountNumber->setText(QString());

  // setup account
  m_account.setAccountType(m_accountType);
  m_account.setName(m_accountName->text());
  m_account.setNumber(m_accountNumber->text());
  m_account.setCurrencyId(m_currencyComboBox->security().id());

  m_brokerage.setAccountType(MyMoneyAccount::Checkings);
  m_brokerage.setCurrencyId(m_currencyComboBox->security().id());
  m_brokerage.setNumber(m_accountNumber->text());

  if(!m_institutionComboBox->currentText().isEmpty()) {
    QValueList<MyMoneyInstitution> list;
    QValueList<MyMoneyInstitution>::ConstIterator it;

    list = MyMoneyFile::instance()->institutionList();
    for(it = list.begin(); it != list.end(); ++it) {
      if((*it).name() == m_institutionComboBox->currentText()) {
        m_account.setInstitutionId((*it).id());
        m_brokerage.setInstitutionId((*it).id());
      }
    }
  }
  if(m_accountType == MyMoneyAccount::Investment) {
    if(m_brokerageYesButton->isChecked()) {
      m_brokerage.setName(m_account.name()+i18n(" (Brokerage)"));
    }
    m_brokerage.setOpeningDate(m_openingDate->date());
  } else {
    // m_account.setOpeningBalance(MyMoneyMoney(openingBalance->text()));
    m_account.setOpeningDate(m_openingDate->date());
  }

  if(m_preferredAccount->isChecked())
    m_account.setValue("PreferredAccount", "Yes");

  // setup parent account
  switch(MyMoneyAccount::accountGroup(m_accountType)) {
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

    if (m_payee->currentText().isEmpty())
    {
      KMessageBox::error(this, i18n("Please enter the payee name."));
      m_payee->setFocus();
      return;
    }

    KMyMoneyAccountTreeItem *item = dynamic_cast<KMyMoneyAccountTreeItem *>(m_accountListView->selectedItem());
    if (!item)
    {
      KMessageBox::error(this, i18n("Please select the account."));
      m_accountListView->setFocus();
      return;
    }

    // Create the schedule transaction
    QCString payeeId;
    m_payee->selectedItem(payeeId);

    MyMoneySplit s1, s2;
    s1.setValue(m_amount->value());
    s2.setValue(-s1.value());
    s1.setAction(MyMoneySplit::ActionTransfer);
    s2.setAction(MyMoneySplit::ActionTransfer);
    s1.setAccountId(QCString()/*this_account?*/);  // This needs to be set by caller  (see KMyMoneyView::accountNew)
    s2.setAccountId(item->id());
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
                                   m_date->date(),
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

  m_accountListView->header()->setFont(KMyMoneyUtils::headerFont());

  // currently, we don't have help for these pages :-(
  setHelpEnabled(brokerageAccountPage, false);
  setHelpEnabled(summaryPage, false);

  setFinishEnabled(institutionPage, false);
  setFinishEnabled(accountTypePage, false);
  setFinishEnabled(accountNamePage, false);
  setFinishEnabled(accountNumberPage, false);
  setFinishEnabled(accountDetailsPage, false);
  setFinishEnabled(accountPaymentPage, false);
  setFinishEnabled(brokerageAccountPage, false);
  setFinishEnabled(summaryPage, true);

  // always start on first page
  showPage(institutionPage);

  setBackEnabled(institutionPage, false);

  loadInstitutionList();
  loadAccountList();

  // always check the payment reminder
  reminderCheckBox->setChecked(true);
  estimateFrame->setEnabled(true);

  // reset everything else if not preset
  m_accountNumber->setText(QString());
  m_brokerage = MyMoneyAccount();

  rc = KNewAccountWizardDecl::exec();

  // always select the first item and show the appropriate note
  m_accountTypeListBox->setCurrentItem(0);

  return rc;
}

void KNewAccountWizard::loadInstitutionList(void)
{
  m_institutionComboBox->clear();

  QValueList<MyMoneyInstitution> list;
  QValueList<MyMoneyInstitution>::ConstIterator it;

  m_institutionComboBox->insertItem(QString());
  list = MyMoneyFile::instance()->institutionList();
  for(it = list.begin(); it != list.end(); ++it)
  {
    m_institutionComboBox->insertItem((*it).name());
  }

  m_institutionComboBox->setCurrentText(m_institution.name());
}

void KNewAccountWizard::loadSubAccountList(KListView* parent, const QCString& accountId)
{
  QValueList<MyMoneyAccount>::ConstIterator it;

  it = findAccount(accountId);
  KMyMoneyAccountTreeItem *topLevelAccount = new KMyMoneyAccountTreeItem(parent, *it);

  QCStringList::ConstIterator it_s;
  for(it_s = (*it).accountList().begin(); it_s != (*it).accountList().end(); ++it_s) {
    loadSubAccountList(topLevelAccount, (*it_s));
  }
}

void KNewAccountWizard::loadSubAccountList(KMyMoneyAccountTreeItem* parent, const QCString& accountId)
{
  QValueList<MyMoneyAccount>::ConstIterator it;

  it = findAccount(accountId);
  KMyMoneyAccountTreeItem *topLevelAccount = new KMyMoneyAccountTreeItem(parent, *it);

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
  m_accountListView->clear();

  m_accountList = MyMoneyFile::instance()->accountList();

  MyMoneyAccount acc = MyMoneyFile::instance()->asset();

  QCStringList::ConstIterator it_s;
  for(it_s = acc.accountList().begin(); it_s != acc.accountList().end(); ++it_s) {
    loadSubAccountList(m_accountListView, (*it_s));
  }
}

void KNewAccountWizard::loadAccountTypes(void)
{
  m_accountTypeListBox->clear();

  m_accountTypeListBox->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Checkings));
  m_accountTypeListBox->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Savings));
  m_accountTypeListBox->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::CreditCard));
  m_accountTypeListBox->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Cash));
  m_accountTypeListBox->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Loan));

/*
  // accountTypeListBox->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::AssetLoan));
  accountTypeListBox->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::MoneyMarket));
  accountTypeListBox->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Currency));
*/
  m_accountTypeListBox->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Investment));
  m_accountTypeListBox->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Asset));
  m_accountTypeListBox->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Liability));
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
      "Use the checking account type to manage "
      "activities on your checking account e.g. payments, checks and cash card "
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
      "Use the investment account to manage your stock, mutual fund and other investments."
    );
    m_accountType = MyMoneyAccount::Investment;
  } else {
    txt += i18n("Explanation is not yet available! UnknownAccountType will be set");
    m_accountType = MyMoneyAccount::UnknownAccountType;
  }
  m_explanationTextBox->setText(txt);
}

void KNewAccountWizard::setAccountName(const QString& name)
{
  m_accountName->setText(name);
}

void KNewAccountWizard::setOpeningBalance(const MyMoneyMoney& balance)
{
  m_openingBalance->setValue(balance);
}

MyMoneyMoney KNewAccountWizard::openingBalance(void) const
{
  return m_openingBalance->value();
}

void KNewAccountWizard::setOpeningDate(const QDate& date)
{
  if(date.isValid())
    m_openingDate->setDate(date);
  else
    m_openingDate->setDate(QDate::currentDate());
}

void KNewAccountWizard::setAccountType(const MyMoneyAccount::accountTypeE type)
{
  int i;

  for(i = m_accountTypeListBox->count()-1; i > 0; --i) {
    if(m_accountTypeListBox->text(i) == KMyMoneyUtils::accountTypeToString(type))
      break;
  }
  m_accountTypeListBox->setSelected(i, true);
}

void KNewAccountWizard::loadPaymentMethods()
{
  m_method->clear();
  m_method->insertItem(i18n("Direct Debit"));
  m_method->insertItem(i18n("Direct Deposit"));
  m_method->insertItem(i18n("Manual Deposit"));
  m_method->insertItem(i18n("Write Check"));
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
    if(m_accountName->text().isEmpty())
      nextButton()->setEnabled(false);

  } else if(currentPage() == accountPaymentPage) {
    if(reminderCheckBox->isChecked()) {
      if(m_amount->text().isEmpty()
      || m_name->text().isEmpty()
      || m_payee->currentText().isEmpty()
      || m_accountListView->selectedItem() == 0) {
        finishButton()->setEnabled(false);
      }
    }
  }
  else if(currentPage() == summaryPage) {
    finishButton()->setDefault(true);
  }
}

void KNewAccountWizard::slotCurrencyChanged(int)
{
  m_priceButton->setEnabled(m_currencyComboBox->security().id() != MyMoneyFile::instance()->baseCurrency().id());
}

void KNewAccountWizard::slotPriceUpdate(void)
{
  KCurrencyEditDlg dlg(this, "KCurrencyEditDlg");
  dlg.slotSelectCurrency(m_currencyComboBox->security().id());
  dlg.exec();
}

void KNewAccountWizard::help(void)
{
  QString anchor = m_helpAnchor[currentPage()];
  if(anchor.isEmpty())
    anchor = QString("firsttime-4");

  kapp->invokeHelp(anchor);
}

#include "knewaccountwizard.moc"
