/***************************************************************************
                          kaccountselectdlg.cpp  -  description
                             -------------------
    begin                : Mon Feb 10 2003
    copyright            : (C) 2000-2003 by Michael Edwardes
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

#include <qlabel.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kpushbutton.h>
#include <kmessagebox.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kaccountselectdlg.h"
#include "knewaccountwizard.h"
#include "knewaccountdlg.h"
#include "knewbankdlg.h"
#include "../mymoney/mymoneyinstitution.h"
#include "../mymoney/mymoneyfile.h"

KAccountSelectDlg::KAccountSelectDlg(const KMyMoneyUtils::categoryTypeE accountType, const QString& purpose, QWidget *parent, const char *name )
 : KAccountSelectDlgDecl(parent,name),
   m_purpose(purpose),
   m_accountType(accountType),
   m_aborted(false)
{
  // Hide the abort button. It needs to be shown on request by the caller
  // using showAbortButton()
  m_kButtonAbort->hide();
  loadAccounts();
  MyMoneyFile::instance()->attach(MyMoneyFile::NotifyClassAccount, this);
  
  connect(m_createButton, SIGNAL(clicked()), this, SLOT(slotCreateAccount()));
  connect(m_qbuttonOk, SIGNAL(clicked()), this, SLOT(accept()));
  connect(m_qbuttonCancel, SIGNAL(clicked()), this, SLOT(reject()));
  connect(m_kButtonAbort, SIGNAL(clicked()), this, SLOT(abort()));
}

KAccountSelectDlg::~KAccountSelectDlg()
{
  MyMoneyFile::instance()->detach(MyMoneyFile::NotifyClassAccount, this);
}

void KAccountSelectDlg::update(const QCString& /*id */)
{
  loadAccounts();
}

void KAccountSelectDlg::setDescription(const QString& msg)
{
  m_descLabel->setText(msg);
}

void KAccountSelectDlg::setHeader(const QString& msg)
{
  m_headerLabel->setText(msg);
}

void KAccountSelectDlg::setAccount(const MyMoneyAccount& account)
{
  int   i;
  
  m_account = account;
  if(!m_account.name().isEmpty()) {
    for(i = 0; i < m_accountComboBox->count(); ++i) {
      if(m_accountComboBox->text(i) == m_account.name()) {
        m_accountComboBox->setCurrentText(m_account.name());
        break;
      }
    }
  } else
    m_accountComboBox->setCurrentText(m_accountComboBox->text(0));
}

void KAccountSelectDlg::loadAccounts(void)
{
  QStringList strList;

  try {
    MyMoneyFile *file = MyMoneyFile::instance();

    // read all account items from the MyMoneyFile objects and add them to the listbox
    if(m_accountType & KMyMoneyUtils::liability)
      addCategories(strList, file->liability().id(), "");

    if(m_accountType & KMyMoneyUtils::asset)
      addCategories(strList, file->asset().id(), "");

    if(m_accountType & KMyMoneyUtils::expense)
      addCategories(strList, file->expense().id(), "");

    if(m_accountType & KMyMoneyUtils::income)
      addCategories(strList, file->income().id(), "");

  } catch (MyMoneyException *e) {
    qDebug("Exception '%s' thrown in %s, line %ld caught in KAccountSelectDlg::loadAccounts:%d",
      e->what().latin1(), e->file().latin1(), e->line(), __LINE__);
    delete e;
  }

  strList.sort();
  m_accountComboBox->clear();
  m_accountComboBox->insertStringList(strList);

  KConfig* config = KGlobal::config();
  config->setGroup("Last Use Settings");
  QString current = config->readEntry("LastAccountSelected_"+m_purpose);

  m_accountComboBox->setCurrentItem(0);
  if(strList.contains(current) > 0)
    m_accountComboBox->setCurrentText(current);
}

void KAccountSelectDlg::addCategories(QStringList& strList, const QCString& id, const QString& leadIn) const
{
  MyMoneyFile *file = MyMoneyFile::instance();
  QString name;

  MyMoneyAccount account = file->account(id);

  QCStringList accList = account.accountList();
  QCStringList::ConstIterator it_a;

  for(it_a = accList.begin(); it_a != accList.end(); ++it_a) {
    account = file->account(*it_a);
    strList << leadIn + account.name();
    addCategories(strList, *it_a, leadIn + account.name() + ":");
  }
}

void KAccountSelectDlg::slotCreateInstitution(void)
{
  MyMoneyInstitution institution;

  KNewBankDlg dlg(institution, false, this, "newbankdlg");
  if (dlg.exec()) {
    try {
      MyMoneyFile* file = MyMoneyFile::instance();

      institution = dlg.institution();

      file->addInstitution(institution);
    } catch (MyMoneyException *e) {
      KMessageBox::information(this, i18n("Cannot add institution: ")+e->what());
      delete e;
      return;
    }
  }
}

void KAccountSelectDlg::slotCreateAccount(void)
{
  MyMoneyAccount newAccount;
  MyMoneyAccount parentAccount;
  int dialogResult;
  const bool isCategory = m_accountType & (KMyMoneyUtils::expense | KMyMoneyUtils::income);

  KConfig *config = KGlobal::config();
  config->setGroup("General Options");
  if(config->readBoolEntry("NewAccountWizard", true) == true
  && !isCategory) {
    // wizard selected
    KNewAccountWizard wizard(0);
    connect(&wizard, SIGNAL(newInstitutionClicked()), this, SLOT(slotCreateInstitution()));
    
    wizard.setAccountName(m_account.name());
    wizard.setAccountType(m_account.accountType());
    wizard.setOpeningBalance(m_account.openingBalance());
    wizard.setOpeningDate(m_account.openingDate());
    if((dialogResult = wizard.exec()) == QDialog::Accepted) {
      newAccount = wizard.account();
      parentAccount = wizard.parentAccount();
    }
  } else {
    // regular dialog selected
    MyMoneyAccount account(m_account);
    KNewAccountDlg dialog(account, false, isCategory, 0, "hi", i18n("Create a new Account"));

    if((dialogResult = dialog.exec()) == QDialog::Accepted) {
      newAccount = dialog.account();
      newAccount.setParentAccountId("");  // make sure, it's not set for adding
      parentAccount = dialog.parentAccount();
    }
  }

  if(dialogResult == QDialog::Accepted) {
    // if the account name contains one or more colons, we
    // need to create a hierarchy.
    int pos;
    MyMoneyFile *file = MyMoneyFile::instance();
    
    try
    {
      while((pos = newAccount.name().find(':')) != -1) {
        QString part = newAccount.name().left(pos);
        QString remainder = newAccount.name().mid(pos+1);
        newAccount.setName(part);

        file->addAccount(newAccount, parentAccount);
        parentAccount = newAccount;
        newAccount.setParentAccountId("");  // make sure, there's no parent
        newAccount.setAccountId("");        // and no id set for adding
        newAccount.setName(remainder);
      }
      
      file->addAccount(newAccount, parentAccount);

      // reload widget with new accounts
      loadAccounts();
      
      if(isCategory)
        m_accountComboBox->setCurrentItem(file->accountToCategory(newAccount.id()));
      else
        m_accountComboBox->setCurrentItem(newAccount.name());
      accept();
    }
    catch (MyMoneyException *e)
    {
      QString message("Unable to add account: ");
      message += e->what();
      KMessageBox::information(this, message);
      delete e;
    }
  }

/*  
  KNewAccountWizard wizard(0);
  
  wizard.setAccountName(m_account.name());
  wizard.setAccountType(m_account.accountType());
  wizard.setOpeningBalance(m_account.openingBalance());
  wizard.setOpeningDate(m_account.openingDate());
  if(wizard.exec()) {
    MyMoneyAccount newAccount = wizard.account();
    MyMoneyAccount parentAccount = wizard.parentAccount();

    // The dialog/wizard doesn't check the parent.
    // An exception will be thrown on the next line instead.
    try
    {
      MyMoneyFile::instance()->addAccount(newAccount, parentAccount);

      m_accountComboBox->setCurrentText(wizard.account().name());
      accept();
    }
    catch (MyMoneyException *e)
    {
      QString message("Unable to add account: ");
      message += e->what();
      KMessageBox::information(this, message);
      delete e;
    }
  }
*/
}

void KAccountSelectDlg::abort(void)
{
  m_aborted = true;
  reject();
}

void KAccountSelectDlg::setMode(const int mode)
{
  m_mode = mode ? 1 : 0;
}

void KAccountSelectDlg::showAbortButton(const bool visible)
{
  if(visible)
    m_kButtonAbort->show();
  else
    m_kButtonAbort->hide();
}

int KAccountSelectDlg::exec(void)
{
  int rc = Rejected;
  
  if(m_mode == 1) {
    slotCreateAccount();
    rc = result();
  }
  if(rc != Accepted)
    rc = KAccountSelectDlgDecl::exec();
    
  return rc;
}
