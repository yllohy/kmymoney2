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

// ----------------------------------------------------------------------------
// Project Includes

#include "../widgets/kmymoneyedit.h"
#include "../widgets/kmymoneydateinput.h"
#include "../widgets/kmymoneycombo.h"
#include "../widgets/kmymoneycategory.h"
#include "../widgets/kmymoneypayee.h"
#include "../mymoney/mymoneyinstitution.h"
#include "../mymoney/mymoneyfile.h"
#include "../kmymoneyutils.h"
#include "ieditscheduledialog.h"
#include "knewaccountwizard.h"

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

  // always select the first item and show the appropriate note
  accountTypeListBox->setCurrentItem(0);

  // FIXME: we don't have currency support, so we hide the two widgets
  // that have been prepared for this support
  m_currencyLabel->hide();
  m_currencyComboBox->hide();
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

  KNewAccountWizardDecl::next();
}

void KNewAccountWizard::accept()
{
  if(appropriate(accountNumberPage) == false)
    accountNumber->setText("");

  // setup account
  m_account.setAccountType(m_accountType);
  m_account.setName(accountName->text());
  m_account.setNumber(accountNumber->text());

  if(institutionComboBox->currentText() != "") {
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

    if (m_category->text().isEmpty())
    {
      KMessageBox::error(this, i18n("Please enter the category."));
      m_category->setFocus();
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
    QCString categoryId;
    QCString payeeId;
  
    try
    {
      categoryId = MyMoneyFile::instance()->categoryToAccount(m_category->text());
      if (categoryId.isEmpty())
      {
        int y = KMessageBox::questionYesNo(this, QString(i18n("Category '%1' does not exist.  Add it?")).arg(m_category->text()));
        if (y == KMessageBox::No)
        {
          m_category->setText("");
          m_category->setFocus();
          return;
        }
        categoryId = MyMoneyFile::instance()->createCategory(MyMoneyFile::instance()->expense(), m_category->text());
      }
    }
    catch (MyMoneyException *e)
    {
      KMessageBox::error(this, i18n("Unable to create category.  Please manually edit the schedule later."));
      delete e;
    }

    try
    {
      payeeId = MyMoneyFile::instance()->payeeByName(m_payee->text()).id();
    }
    catch (MyMoneyException *e)
    {
      MyMoneyPayee payee(m_payee->text());
      MyMoneyFile::instance()->addPayee(payee);
      payeeId = payee.id();
    }
    
    MyMoneySplit s1, s2;
    s1.setValue(m_amount->getMoneyValue());
    s2.setValue(-s1.value());
    s1.setAction(MyMoneySplit::ActionWithdrawal);
    s2.setAction(MyMoneySplit::ActionDeposit);
    s1.setAccountId(item->accountID());
    s2.setAccountId(categoryId);
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
        paymentType = MyMoneySchedule::STYPE_WRITECHEQUE;
        break;
      case 2:
        paymentType = MyMoneySchedule::STYPE_OTHER;
        break;
      default:
        paymentType = MyMoneySchedule::STYPE_DIRECTDEBIT;
        break;
    }

    MyMoneySchedule paymentSchedule(  m_name->text(),
                                      MyMoneySchedule::TYPE_BILL,
                                      MyMoneySchedule::OCCUR_MONTHLY,
                                      paymentType,
                                      m_date->getQDate(),
                                      QDate(),
                                      false,
                                      false);

    paymentSchedule.setTransaction(t);

    try
    {
      MyMoneyFile::instance()->addSchedule(paymentSchedule);
    } catch (MyMoneyException *e)
    {
      KMessageBox::information(this, i18n("Unable to add schedule: "), e->what());
      delete e;
    }
  }

  KNewAccountWizardDecl::accept();
}

int KNewAccountWizard::exec()
{
  int rc;
  
  KConfig *config = KGlobal::config();
  config->setGroup("List Options");
  QFont defaultFont = QFont("helvetica", 12);
  accountListView->header()->setFont(config->readFontEntry("listHeaderFont", &defaultFont));

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
  accountNumber->setText("");

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

  institutionComboBox->insertItem("");
  list = MyMoneyFile::instance()->institutionList();
  for(it = list.begin(); it != list.end(); ++it)
    institutionComboBox->insertItem((*it).name());
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
  accountTypeListBox->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Investment));
  accountTypeListBox->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::MoneyMarket));
  accountTypeListBox->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Currency));
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
      "activities on your checkings account e.g. payments, cheques and ec-card "
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
      "liabilities (e.g. xxxx)."
    );
    m_accountType = MyMoneyAccount::Liability;

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
  m_method->insertItem(i18n("Write Cheque"));
  m_method->insertItem(i18n("Other"));
}
