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

#include <kconfig.h>
#include <kglobal.h>
#include <kpushbutton.h>
#include <kmessagebox.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kaccountselectdlg.h"
#include "knewaccountwizard.h"
#include "../mymoney/mymoneyfile.h"

KAccountSelectDlg::KAccountSelectDlg(const QString& purpose, QWidget *parent, const char *name )
 : KAccountSelectDlgDecl(parent,name),
   m_purpose(purpose)
{
  loadAccounts();
  MyMoneyFile::instance()->attach(MyMoneyFile::NotifyClassAccount, this);
  
  connect(m_createButton, SIGNAL(clicked()), this, SLOT(slotCreateAccount()));
  connect(m_qbuttonOk, SIGNAL(clicked()), this, SLOT(accept()));
  connect(m_qbuttonCancel, SIGNAL(clicked()), this, SLOT(reject()));
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
  m_account = account;
}

void KAccountSelectDlg::loadAccounts(void)
{
  QStringList strList;

  try {
    MyMoneyFile *file = MyMoneyFile::instance();

    // read all account items from the MyMoneyFile objects and add them to the listbox
    addCategories(strList, file->liability().id(), "");
    addCategories(strList, file->asset().id(), "");

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

void KAccountSelectDlg::slotCreateAccount(void)
{
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
