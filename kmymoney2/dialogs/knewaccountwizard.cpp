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

#include <klocale.h>
#include <klistbox.h>
#include <kcombobox.h>
#include <kpushbutton.h>
#include <ktextbrowser.h>
#include <klineedit.h>

#include "../widgets/kmymoneyedit.h"
#include "../mymoney/mymoneyinstitution.h"
#include "../mymoney/mymoneyfile.h"
#include "knewaccountwizard.h"

KNewAccountWizard::KNewAccountWizard(QWidget *parent, const char *name )
  : KNewAccountWizardDecl(parent,name,true)
{
  connect(newInstitutionButton, SIGNAL(clicked()), this, SLOT(slotNewInstitution()));
  connect(accountTypeListBox, SIGNAL(highlighted(const QString &)), this, SLOT(slotAccountType(const QString &)));
}

KNewAccountWizard::~KNewAccountWizard()
{
}

void KNewAccountWizard::next()
{
  if(currentPage() == accountNamePage) {
    QString type = accountTypeListBox->currentText();
    if(type == i18n("Cash")
    || type == i18n("Asset")) {
      setAppropriate(accountNumberPage, false);
    } else {
      setAppropriate(accountNumberPage, institutionComboBox->currentText() != "");
    }
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
  setFinishEnabled(accountDetailsPage, true);

  // currently, we don't have help :-(
  setHelpEnabled(institutionPage, false);
  setHelpEnabled(accountTypePage, false);
  setHelpEnabled(accountNamePage, false);
  setHelpEnabled(accountNumberPage, false);
  setHelpEnabled(accountDetailsPage, false);

  // always start on first page
  showPage(institutionPage);

  setBackEnabled(institutionPage, false);

  loadInstitutionList();
  loadAccountTypes();

  // always select the first item and show the appropriate note
  accountTypeListBox->setCurrentItem(0);
  slotAccountType(accountTypeListBox->currentText());

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

void KNewAccountWizard::loadAccountTypes(void)
{
  accountTypeListBox->clear();

  accountTypeListBox->insertItem(i18n("Checkings"));
  accountTypeListBox->insertItem(i18n("Savings"));
  accountTypeListBox->insertItem(i18n("Credit Card"));
  accountTypeListBox->insertItem(i18n("Cash"));
  accountTypeListBox->insertItem(i18n("Loan"));
  accountTypeListBox->insertItem(i18n("Investment"));
  accountTypeListBox->insertItem(i18n("Money Market"));
  accountTypeListBox->insertItem(i18n("Currency"));
  accountTypeListBox->insertItem(i18n("Asset"));
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

  } else {
    txt += i18n("Explanation is not yet available!");
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

