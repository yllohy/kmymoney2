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
#include "../views/kbanklistitem.h"

#include <qheader.h>
#include <qtooltip.h>
#include <klistview.h>
#include <kconfig.h>
#include <qcheckbox.h>
#include "../mymoney/mymoneyexception.h"

KNewAccountDlg::KNewAccountDlg(MyMoneyAccount& account, MyMoneyFile* file, QWidget *parent,
    const char *name, const char *title)
  : KNewAccountDlgDecl(parent,name,true), m_account(account)
{
  m_file = file;

  QString filename = KGlobal::dirs()->findResource("appdata", "pics/dlg_new_account.png");
  QPixmap *pm = new QPixmap(filename);
  m_qpixmaplabel->setPixmap(*pm);

	m_qlistviewParentAccounts->setRootIsDecorated(true);
	m_qlistviewParentAccounts->setAllColumnsShowFocus(true);
	m_qlistviewParentAccounts->addColumn(i18n("Account"));
	m_qlistviewParentAccounts->setMultiSelection(false);
	m_qlistviewParentAccounts->header()->setResizeEnabled(false);
	m_qlistviewParentAccounts->setColumnWidthMode(0, QListView::Manual);
  m_qlistviewParentAccounts->setEnabled(false);

  m_qcheckboxSubAccount->setText(i18n("Is a sub account"));

	KConfig *config = KGlobal::config();
  QFont defaultFont = QFont("helvetica", 12);
  m_qlistviewParentAccounts->header()->setFont(config->readFontEntry("listHeaderFont", &defaultFont));

  try
  {
    institutionNameLabel->setText(m_file->institution(account.institutionId()).name());
    accountNameEdit->setText(account.name());
    accountNoEdit->setText(account.number());
    descriptionEdit->setText(account.description());
    startDateEdit->setDate(account.openingDate());
    startBalanceEdit->setText(account.openingBalance().formatMoney());
  }
  catch (MyMoneyException *e)
  {
    qDebug("unhandled excpetion in KNewAccountDlg: %s", e->what().latin1());
    delete e;
  }

  initParentWidget();
  //parentAccountWidget->setParentAccount(account.parentAccountId());

  accountNameEdit->setFocus();
	
  if (title)
	  setCaption(title);

  connect(cancelButton, SIGNAL(clicked()), SLOT(reject()));
  connect(createButton, SIGNAL(clicked()), this, SLOT(okClicked()));
  connect(m_qcheckboxSubAccount, SIGNAL(toggled(bool)), this, SLOT(slotSubAccountsToggled(bool)));
  connect(m_qlistviewParentAccounts, SIGNAL(selectionChanged(QListViewItem*)),
    this, SLOT(slotSelectionChanged(QListViewItem*)));
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

  MyMoneyAccount::accountTypeE accType;

  // Hardcoded but acceptable
  switch (typeCombo->currentItem()+1)
  {
    case 1:
      accType = MyMoneyAccount::Checkings;
      break;
    case 2:
      accType = MyMoneyAccount::Savings;
      break;
    case 3:
      accType = MyMoneyAccount::Cash;
      break;
    case 4:
      accType = MyMoneyAccount::CreditCard;
      break;
    case 5:
      accType = MyMoneyAccount::Loan;
      break;
    case 6:
      accType = MyMoneyAccount::CertificateDep;
      break;
    case 7:
      accType = MyMoneyAccount::Investment;
      break;
    case 8:
      accType = MyMoneyAccount::MoneyMarket;
      break;
    case 9:
      accType = MyMoneyAccount::Currency;
      break;
    default:
      accType = MyMoneyAccount::UnknownAccountType;
  }

	m_account.setAccountType(accType);
  m_account.setDescription(descriptionEdit->text());
  m_account.setOpeningBalance(startBalanceEdit->getMoneyValue());
  m_account.setOpeningDate(startDateEdit->getQDate());

  accept();
}

MyMoneyAccount KNewAccountDlg::account(void)
{
  return m_account;
}

const MyMoneyAccount KNewAccountDlg::parentAccount(void)
{
  if (m_bSelectedParentAccount)
  {
    return m_parentAccount;
  }
  else  // Build the parent for them, force parent...
  {
    MyMoneyAccount account;
    switch (m_file->accountGroup(m_account.accountType()))
    {
      case MyMoneyAccount::Asset:
        account = m_file->asset();
        break;
      case MyMoneyAccount::Liability:
        account = m_file->liability();
        break;
      case MyMoneyAccount::Income:
        account = m_file->income();
        break;
      case MyMoneyAccount::Expense:
        account = m_file->expense();
        break;
    }
    return account;
  }
}

void KNewAccountDlg::initParentWidget()
{
  MyMoneyAccount liabilityAccount = m_file->liability();
  MyMoneyAccount assetAccount = m_file->asset();
  MyMoneyAccount expenseAccount = m_file->expense();
  MyMoneyAccount incomeAccount = m_file->income();

  // Do all 4 account roots
  try
  {
    // Asset
    KAccountListItem *assetTopLevelAccount = new KAccountListItem(m_qlistviewParentAccounts,
                      assetAccount.name(), assetAccount.id());

    for ( QCStringList::ConstIterator it = m_file->asset().accountList().begin();
          it != m_file->asset().accountList().end();
          ++it )
    {
      KAccountListItem *accountItem = new KAccountListItem(assetTopLevelAccount,
          m_file->account(*it).name(), m_file->account(*it).id(), false);

      QCStringList subAccounts = m_file->account(*it).accountList();
      if (subAccounts.count() >= 1)
      {
        showSubAccounts(subAccounts, accountItem, m_file);
      }
    }

    // Liability
    KAccountListItem *liabilityTopLevelAccount = new KAccountListItem(m_qlistviewParentAccounts,
                      liabilityAccount.name(), liabilityAccount.id());

    for ( QCStringList::ConstIterator it = m_file->liability().accountList().begin();
          it != m_file->liability().accountList().end();
          ++it )
    {
      KAccountListItem *accountItem = new KAccountListItem(liabilityTopLevelAccount,
          m_file->account(*it).name(), m_file->account(*it).id(), false);

      QCStringList subAccounts = m_file->account(*it).accountList();
      if (subAccounts.count() >= 1)
      {
        showSubAccounts(subAccounts, accountItem, m_file);
      }
    }

    // Income
    KAccountListItem *incomeTopLevelAccount = new KAccountListItem(m_qlistviewParentAccounts,
                      incomeAccount.name(), incomeAccount.id());

    for ( QCStringList::ConstIterator it = m_file->income().accountList().begin();
          it != m_file->income().accountList().end();
          ++it )
    {
      KAccountListItem *accountItem = new KAccountListItem(incomeTopLevelAccount,
          m_file->account(*it).name(), m_file->account(*it).id(), false);

      QCStringList subAccounts = m_file->account(*it).accountList();
      if (subAccounts.count() >= 1)
      {
        showSubAccounts(subAccounts, accountItem, m_file);
      }
    }

    // Expense
    KAccountListItem *expenseTopLevelAccount = new KAccountListItem(m_qlistviewParentAccounts,
                      expenseAccount.name(), expenseAccount.id());

    for ( QCStringList::ConstIterator it = m_file->expense().accountList().begin();
          it != m_file->expense().accountList().end();
          ++it )
    {
      KAccountListItem *accountItem = new KAccountListItem(expenseTopLevelAccount,
          m_file->account(*it).name(), m_file->account(*it).id(), false);

      QCStringList subAccounts = m_file->account(*it).accountList();
      if (subAccounts.count() >= 1)
      {
        showSubAccounts(subAccounts, accountItem, m_file);
      }
    }
  }
  catch (MyMoneyException *e)
  {
    qDebug("Exception in assets account refresh: %s", e->what().latin1());
    delete e;
  }

	m_qlistviewParentAccounts->setColumnWidth(0, m_qlistviewParentAccounts->width());
}

void KNewAccountDlg::showSubAccounts(QCStringList accounts, KAccountListItem *parentItem, MyMoneyFile *file)
{
  for ( QCStringList::ConstIterator it = accounts.begin(); it != accounts.end(); ++it )
  {
    KAccountListItem *accountItem  = new KAccountListItem(parentItem,
          m_file->account(*it).name(), file->account(*it).id(), false);

    QCStringList subAccounts = m_file->account(*it).accountList();
    if (subAccounts.count() >= 1)
    {
      showSubAccounts(subAccounts, accountItem, m_file);
    }
  }
}

void KNewAccountDlg::resizeEvent(QResizeEvent* e)
{
	m_qlistviewParentAccounts->setColumnWidth(0, m_qlistviewParentAccounts->width());

	// call base class resizeEvent()
	KNewAccountDlgDecl::resizeEvent(e);
}

void KNewAccountDlg::slotSelectionChanged(QListViewItem *item)
{
  KAccountListItem *accountItem = (KAccountListItem*)item;
  m_bSelectedParentAccount = true;
  try
  {
    qDebug("Selected account id: %s", accountItem->accountID().data());
    qDebug("asset acount id: %s", m_file->asset().id().data());
    m_parentAccount = m_file->account(accountItem->accountID());
    QString theText(i18n("Is a sub account of "));
    theText += m_parentAccount.name();
    m_qcheckboxSubAccount->setText(theText);
  }
  catch (MyMoneyException *e)
  {
    qDebug("This shouldn't happen!");
    delete e;
  }
}

void KNewAccountDlg::slotSubAccountsToggled(bool on)
{
  m_qlistviewParentAccounts->setEnabled(on);
  if (on)
  {
    QString theText(i18n("Is a sub account of "));
    theText += m_parentAccount.name();
    m_qcheckboxSubAccount->setText(theText);
  }
  else
  {
    m_qcheckboxSubAccount->setText(i18n("Is a sub account"));
  }
}
