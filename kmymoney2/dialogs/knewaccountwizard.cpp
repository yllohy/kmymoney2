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

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>
#include <klistbox.h>
#include <kcombobox.h>
#include <kpushbutton.h>
#include <ktextbrowser.h>
#include <klineedit.h>
#include <kconfig.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "../widgets/kmymoneyedit.h"
#include "../widgets/kmymoneydateinput.h"
#include "../mymoney/mymoneyinstitution.h"
#include "../mymoney/mymoneyfile.h"
#include "../kmymoneyutils.h"
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

  KNewAccountWizardDecl::accept();
}

int KNewAccountWizard::exec()
{
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

  // always select the first item and show the appropriate note
  accountTypeListBox->setCurrentItem(0);
  slotAccountType(accountTypeListBox->currentText());

  // always check the payment reminder
  reminderCheckBox->setChecked(true);
  estimateFrame->setEnabled(true);

  // reset everything else if not preset
  accountNumber->setText("");

  return KNewAccountWizardDecl::exec();
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
}

void KNewAccountWizard::slotNewInstitution(void)
{
  emit newInstitutionClicked();
  loadInstitutionList();
}

void KNewAccountWizard::slotAccountType(const QString& sel)
{
  QString txt = "<h2><center>" + sel + "</center></h2><p><p>";

  if(sel == i18n("Checkings")) {
    txt += i18n(
      "Use the checkings account type to manage "
      "activities on your checkings account e.g. payments, cheques and ec-card "
      "purchases."
    );
    m_accountType = MyMoneyAccount::Checkings;

  } else if(sel == i18n("Savings")) {
    txt += i18n(
      "Use the savings account type to manage "
      "activities on your savings account."
    );
    m_accountType = MyMoneyAccount::Savings;

  } else if(sel == i18n("Credit Card")) {
    txt += i18n(
      "Use the credit card account type to manage "
      "activities on your credit card."
    );
    m_accountType = MyMoneyAccount::CreditCard;

  } else if(sel == i18n("Cash")) {
    txt += i18n(
      "Use the cash account type to manage "
      "activities in your wallet."
    );
    m_accountType = MyMoneyAccount::Cash;

  } else if(sel == i18n("Asset")) {
    txt += i18n(
      "Use the asset account type to manage "
      "assets (e.g. your house, car or art collection)."
    );
    m_accountType = MyMoneyAccount::Asset;

  } else {
    txt += i18n("Explanation is not yet available! UnknownAccountType will be set");
    m_accountType = MyMoneyAccount::UnknownAccountType;
  }
  explanationTextBox->setText(txt);
}

const MyMoneyAccount KNewAccountWizard::account(void) const
{
  MyMoneyAccount account;
  account.setAccountType(m_accountType);
  account.setName(accountName->text());
  account.setNumber(accountNumber->text());

  if(institutionComboBox->currentText() != "") {
    QValueList<MyMoneyInstitution> list;
    QValueList<MyMoneyInstitution>::ConstIterator it;

    list = MyMoneyFile::instance()->institutionList();
    for(it = list.begin(); it != list.end(); ++it)
      if((*it).name() == institutionComboBox->currentText())
        account.setInstitutionId((*it).id());
  }
  account.setOpeningBalance(MyMoneyMoney(openingBalance->text()));

  return account;
}

const MyMoneyAccount KNewAccountWizard::parentAccount(void) const
{
  MyMoneyAccount::accountTypeE type = MyMoneyFile::instance()->accountGroup(m_accountType);
  MyMoneyAccount parent;
  switch(type) {
    default:
    case MyMoneyAccount::Asset:
      parent = MyMoneyFile::instance()->asset();
      break;
    case MyMoneyAccount::Liability:
      parent = MyMoneyFile::instance()->liability();
      break;
  }
  return parent;
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
  openingDate->setDate(date);
}

void KNewAccountWizard::setAccountType(const MyMoneyAccount::accountTypeE type)
{
  int i;
  for(i = accountTypeListBox->count()-1; i > 0; --i) {
    if(accountTypeListBox->text(i) == KMyMoneyUtils::accountTypeToString(type))
      break;
  }
  accountTypeListBox->setCurrentItem(i);
}
