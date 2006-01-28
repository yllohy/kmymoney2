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
#include <kiconloader.h>
#include <kguiitem.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kaccountselectdlg.h"
#include "knewaccountwizard.h"
#include "knewaccountdlg.h"
#include "knewbankdlg.h"
#include "../mymoney/mymoneyinstitution.h"
#include "../mymoney/mymoneyfile.h"
#include "../widgets/kmymoneyaccountselector.h"

KAccountSelectDlg::KAccountSelectDlg(const KMyMoneyUtils::categoryTypeE accountType, const QString& purpose, QWidget *parent, const char *name )
 : KAccountSelectDlgDecl(parent, name),
   m_purpose(purpose),
   m_accountType(accountType),
   m_aborted(false)
{
  // Hide the abort button. It needs to be shown on request by the caller
  // using showAbortButton()
  m_kButtonAbort->hide();

  // Exclude stock accounts.  You can't import a statement into a stock account.
  m_accountSelector->loadList(accountType,MyMoneyAccount::Stock);

  KIconLoader* il = KGlobal::iconLoader();
  KGuiItem skipButtonItem( i18n( "&Skip" ),
                    QIconSet(il->loadIcon("redo", KIcon::Small, KIcon::SizeSmall)),
                    i18n("Skip this transaction"),
                    i18n("Use this to skip importing this transaction and proceed with the next one."));
  m_qbuttonCancel->setGuiItem(skipButtonItem);

  KGuiItem createButtenItem( i18n( "&Create..." ),
                      QIconSet(il->loadIcon("filenew", KIcon::Small, KIcon::SizeSmall)),
                      i18n("Create a new account/category"),
                      i18n("Use this to add a new account/category to the file"));
  m_createButton->setGuiItem(createButtenItem);

  KGuiItem okButtenItem( i18n("&Ok" ),
                    QIconSet(il->loadIcon("button_ok", KIcon::Small, KIcon::SizeSmall)),
                    i18n("Accepts the selected action and continues"),
                    i18n("Use this to accept the selection and continue processing the transaction"));
  m_qbuttonOk->setGuiItem(okButtenItem);

  KGuiItem abortButtenItem( i18n("&Abort" ),
                    QIconSet(il->loadIcon("stop", KIcon::Small, KIcon::SizeSmall)),
                    i18n("Abort the import operation and dismiss all changes"),
                    i18n("Use this to abort the import. Your financial data will be in the state before you started the QIF import."));
  m_kButtonAbort->setGuiItem(abortButtenItem);


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
  m_accountSelector->loadList(m_accountType);
}

void KAccountSelectDlg::setDescription(const QString& msg)
{
  m_descLabel->setText(msg);
}

void KAccountSelectDlg::setHeader(const QString& msg)
{
  m_headerLabel->setText(msg);
}

void KAccountSelectDlg::setAccount(const MyMoneyAccount& account, const QCString& id)
{
  m_account = account;
  m_accountSelector->setSelected(id);
}

void KAccountSelectDlg::slotCreateInstitution(void)
{
  MyMoneyInstitution institution;

  KNewBankDlg dlg(institution, this);
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

  if(!isCategory) {
    // wizard selected
    KNewAccountWizard* wizard = new KNewAccountWizard(this);
    connect(wizard, SIGNAL(newInstitutionClicked()), this, SLOT(slotCreateInstitution()));
    // FIXME: connect the newCategory() signal as well

    wizard->setAccountName(m_account.name());
    wizard->setAccountType(m_account.accountType());
    wizard->setOpeningBalance(m_account.openingBalance());
    wizard->setOpeningDate(m_account.openingDate());
    if((dialogResult = wizard->exec()) == QDialog::Accepted) {
      newAccount = wizard->account();
      // keep a possible description field
      newAccount.setDescription(m_account.description());
      parentAccount = wizard->parentAccount();

      MyMoneyFile* file = MyMoneyFile::instance();
      MyMoneySchedule newSchedule = wizard->schedule();
      try {
        file->schedule(newSchedule.id());
        file->modifySchedule(newSchedule);
        newAccount.setValue("schedule", newSchedule.id());
      } catch (MyMoneyException *e) {
        try {
          file->addSchedule(newSchedule);
          newAccount.setValue("schedule", newSchedule.id());
        } catch (MyMoneyException *f) {
          qDebug("Cannot add schedule: '%s'", f->what().data());
          delete f;
        }
        delete e;
      }
    }
    delete wizard;
  } else {
    // regular dialog selected
    MyMoneyAccount account(m_account);
    KNewAccountDlg dialog(account, false, isCategory, 0, "hi", i18n("Create a new Account"));

    if((dialogResult = dialog.exec()) == QDialog::Accepted) {
      newAccount = dialog.account();
      newAccount.setParentAccountId(QCString());  // make sure, it's not set for adding
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
        newAccount.setParentAccountId(QCString());  // make sure, there's no parent
        newAccount.clearId();                       // and no id set for adding
        newAccount.setName(remainder);
      }

      file->addAccount(newAccount, parentAccount);
      m_accountSelector->loadList(m_accountType);
      m_accountSelector->setSelected(newAccount.id());
/*
      // widgets are updated in update() by engine's notification
      if(isCategory)
        m_accountComboBox->setCurrentItem(file->accountToCategory(newAccount.id()));
      else
        m_accountComboBox->setCurrentItem(newAccount.name());
*/
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
  if(rc != Accepted) {
    m_createButton->setFocus();
    rc = KAccountSelectDlgDecl::exec();
  }
  return rc;
}

const QCString KAccountSelectDlg::selectedAccount(void) const
{
  QCString rc;
  if(!m_accountSelector->selectedAccounts().isEmpty())
    rc = m_accountSelector->selectedAccounts().first();
  return rc;
}

#include "kaccountselectdlg.moc"
