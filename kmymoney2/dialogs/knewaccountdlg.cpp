/***************************************************************************
                          knewaccountdlg.cpp
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes
    email                : mte@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <kglobal.h>
#include <klocale.h>
#if QT_VERSION > 300
#include <kstandarddirs.h>
#else
#include <kstddirs.h>
#endif

#include <qpixmap.h>

#include <kmessagebox.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <kglobal.h>
#include <klocale.h>
#include <qcombobox.h>
#include <kcombobox.h>
#include "knewaccountdlg.h"

KNewAccountDlg::KNewAccountDlg(MyMoneyAccount& account, MyMoneyFile* file, QWidget *parent,
    const char *name, const char *title)
  : KNewAccountDlgDecl(parent,name,true), m_account(account)
{
  m_file = file;

  QString filename = KGlobal::dirs()->findResource("appdata", "pics/dlg_new_account.png");
  QPixmap *pm = new QPixmap(filename);
  m_qpixmaplabel->setPixmap(*pm);

  institutionNameLabel->setText(m_file->institution(account.institutionId()).name());
  accountNameEdit->setText(account.name());
  accountNoEdit->setText(account.number());
  descriptionEdit->setText(account.description());
  startDateEdit->setDate(account.openingDate());
  startBalanceEdit->setText(account.openingBalance().formatMoney());
  //parentAccountWidget->setParentAccount(account.parentAccountId());
  
  accountNameEdit->setFocus();
	
  if (title)
	  setCaption(title);

  connect(cancelButton, SIGNAL(clicked()), SLOT(reject()));
  connect(createButton, SIGNAL(clicked()), this, SLOT(okClicked()));
}

KNewAccountDlg::~KNewAccountDlg()
{
}

void KNewAccountDlg::okClicked()
{
  // KMyMoneyView will check that the parent exists
  // when adding the account.
  //m_parentAccount = parentAccountWidget->parentAccount();

  QString accountNameText = accountNameEdit->text();
  if (accountNameText.isEmpty())
  {
    KMessageBox::error(this, i18n("You have not specified a name.\nPlease fill in this field"));
    accountNameEdit->setFocus();
    return;
  }

  m_account.setName(accountNameText);
  m_account.setNumber(accountNoEdit->text());
	int currentItem = typeCombo->currentItem();
	m_account.setAccountType(static_cast<MyMoneyAccount::accountTypeE>(currentItem));
  m_account.setDescription(descriptionEdit->text());
  m_account.setOpeningBalance(startBalanceEdit->getMoneyValue());
  m_account.setOpeningDate(startDateEdit->getQDate());

  accept();
}

MyMoneyAccount KNewAccountDlg::account(void)
{
  return m_account;
}

MyMoneyAccount KNewAccountDlg::parentAccount(void)
{
  return m_parentAccount;
}

