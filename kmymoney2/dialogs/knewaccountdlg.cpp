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
#include "../mymoney/storage/mymoneyseqaccessmgr.h"
#include "../dialogs/knewbankdlg.h"

KNewAccountDlg::KNewAccountDlg(MyMoneyAccount& account, MyMoneyFile* file, bool isEditing, QWidget *parent,
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

  if (isEditing)
  {
    accountNameEdit->setText(account.name());
    accountNoEdit->setText(account.number());
    descriptionEdit->setText(account.description());
    startDateEdit->setDate(account.openingDate());
    startBalanceEdit->setText(account.openingBalance().formatMoney());

    // Hardcoded but acceptable
    switch (account.accountType())
    {
      case MyMoneyAccount::Checkings:
        typeCombo->setCurrentItem(0);
        break;
      case MyMoneyAccount::Savings:
        typeCombo->setCurrentItem(1);
        break;
      case MyMoneyAccount::Cash:
        typeCombo->setCurrentItem(2);
        break;
      case MyMoneyAccount::CreditCard:
        typeCombo->setCurrentItem(3);
        break;
      case MyMoneyAccount::Loan:
        typeCombo->setCurrentItem(4);
        break;
      case MyMoneyAccount::CertificateDep:
        typeCombo->setCurrentItem(5);
        break;
      case MyMoneyAccount::Investment:
        typeCombo->setCurrentItem(6);
        break;
      case MyMoneyAccount::MoneyMarket:
        typeCombo->setCurrentItem(7);
        break;
      case MyMoneyAccount::Currency:
        typeCombo->setCurrentItem(8);
        break;
      default:
        typeCombo->setCurrentItem(1);
    }
  }

  // Load the institutions
  // then the accounts
  QString institutionName;
  QString accountName;

  try
  {
    if (isEditing)
      institutionName = m_file->institution(account.institutionId()).name();
    else
      institutionName = "";

    if (isEditing)
      accountName = m_file->institution(account.institutionId()).name();
    else
      accountName = "";
  }
  catch (MyMoneyException *e)
  {
    qDebug("excpetion in init for accoutn dialog: %s", e->what().latin1());
    delete e;
  }

  loadInstitutions(institutionName);
  initParentWidget(accountName);

  accountNameEdit->setFocus();
	
  if (title)
	  setCaption(title);

  connect(cancelButton, SIGNAL(clicked()), SLOT(reject()));
  connect(createButton, SIGNAL(clicked()), this, SLOT(okClicked()));
  connect(m_qcheckboxSubAccount, SIGNAL(toggled(bool)), this, SLOT(slotSubAccountsToggled(bool)));
  connect(m_qlistviewParentAccounts, SIGNAL(selectionChanged(QListViewItem*)),
    this, SLOT(slotSelectionChanged(QListViewItem*)));
  connect(m_qbuttonNew, SIGNAL(clicked()), this, SLOT(slotNewClicked()));
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

  QString institutionNameText = m_qcomboboxInstitutions->currentText();
  if (institutionNameText == i18n("<No Institution>"))
  {
      //KMessageBox::error(this, i18n("You have not specified an institution.\nPlease fill in this field"));
      KMessageBox::information(this, i18n("Do we want to force the user to use an institution?"));
      //m_qcomboboxInstitution->setFocus();
      //return;
  }
  else
  {
    try
    {
      QValueList<MyMoneyInstitution> list = m_file->institutionList();
      QValueList<MyMoneyInstitution>::ConstIterator institutionIterator;
      for (institutionIterator = list.begin(); institutionIterator != list.end(); ++institutionIterator)
      {
        if ((*institutionIterator).name() == institutionNameText)
          m_account.setInstitutionId((*institutionIterator).id());
      }
    }
    catch (MyMoneyException *e)
    {
      qDebug("Exception in account institution set: %s", e->what().latin1());
      delete e;
    }
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

void KNewAccountDlg::initParentWidget(const QString& name)
{
  MyMoneyAccount liabilityAccount = m_file->liability();
  MyMoneyAccount assetAccount = m_file->asset();
  MyMoneyAccount expenseAccount = m_file->expense();
  MyMoneyAccount incomeAccount = m_file->income();

  m_bFoundItem = false;

  // Do all 4 account roots
  try
  {
    // Asset
    KAccountListItem *assetTopLevelAccount = new KAccountListItem(m_qlistviewParentAccounts,
                      assetAccount.name(), STD_ACC_ASSET, false);

    for ( QCStringList::ConstIterator it = m_file->asset().accountList().begin();
          it != m_file->asset().accountList().end();
          ++it )
    {
      KAccountListItem *accountItem = new KAccountListItem(assetTopLevelAccount,
          m_file->account(*it).name(), m_file->account(*it).id(), false);

      if (name == m_file->account(*it).name())
      {
        m_bFoundItem = true;
        m_foundItem = accountItem;
      }

      QCStringList subAccounts = m_file->account(*it).accountList();
      if (subAccounts.count() >= 1)
      {
        showSubAccounts(subAccounts, accountItem, m_file, name);
      }
    }

    // Liability
    KAccountListItem *liabilityTopLevelAccount = new KAccountListItem(m_qlistviewParentAccounts,
                      liabilityAccount.name(), STD_ACC_LIABILITY, false);

    for ( QCStringList::ConstIterator it = m_file->liability().accountList().begin();
          it != m_file->liability().accountList().end();
          ++it )
    {
      KAccountListItem *accountItem = new KAccountListItem(liabilityTopLevelAccount,
          m_file->account(*it).name(), m_file->account(*it).id(), false);

      if (name == m_file->account(*it).name())
      {
        m_bFoundItem = true;
        m_foundItem = accountItem;
      }

      QCStringList subAccounts = m_file->account(*it).accountList();
      if (subAccounts.count() >= 1)
      {
        showSubAccounts(subAccounts, accountItem, m_file, name);
      }
    }

    // Income
    KAccountListItem *incomeTopLevelAccount = new KAccountListItem(m_qlistviewParentAccounts,
                      incomeAccount.name(), STD_ACC_INCOME, false);

    for ( QCStringList::ConstIterator it = m_file->income().accountList().begin();
          it != m_file->income().accountList().end();
          ++it )
    {
      KAccountListItem *accountItem = new KAccountListItem(incomeTopLevelAccount,
          m_file->account(*it).name(), m_file->account(*it).id(), false);

      if (name == m_file->account(*it).name())
      {
        m_bFoundItem = true;
        m_foundItem = accountItem;
      }

      QCStringList subAccounts = m_file->account(*it).accountList();
      if (subAccounts.count() >= 1)
      {
        showSubAccounts(subAccounts, accountItem, m_file, name);
      }
    }

    // Expense
    KAccountListItem *expenseTopLevelAccount = new KAccountListItem(m_qlistviewParentAccounts,
                      expenseAccount.name(), STD_ACC_EXPENSE, false);

    for ( QCStringList::ConstIterator it = m_file->expense().accountList().begin();
          it != m_file->expense().accountList().end();
          ++it )
    {
      KAccountListItem *accountItem = new KAccountListItem(expenseTopLevelAccount,
          m_file->account(*it).name(), m_file->account(*it).id(), false);

      if (name == m_file->account(*it).name())
      {
        m_bFoundItem = true;
        m_foundItem = accountItem;
      }

      QCStringList subAccounts = m_file->account(*it).accountList();
      if (subAccounts.count() >= 1)
      {
        showSubAccounts(subAccounts, accountItem, m_file, name);
      }
    }
  }
  catch (MyMoneyException *e)
  {
    qDebug("Exception in assets account refresh: %s", e->what().latin1());
    delete e;
  }

	m_qlistviewParentAccounts->setColumnWidth(0, m_qlistviewParentAccounts->width());

  if (m_bFoundItem)
  {
    m_qlistviewParentAccounts->setSelected(m_foundItem, true);
  }
}

void KNewAccountDlg::showSubAccounts(QCStringList accounts, KAccountListItem *parentItem, MyMoneyFile *file,
  const QString& name)
{
  for ( QCStringList::ConstIterator it = accounts.begin(); it != accounts.end(); ++it )
  {
    KAccountListItem *accountItem  = new KAccountListItem(parentItem,
          m_file->account(*it).name(), file->account(*it).id(), false);

      if (name == m_file->account(*it).name())
      {
        m_bFoundItem = true;
        m_foundItem = accountItem;
      }

    QCStringList subAccounts = m_file->account(*it).accountList();
    if (subAccounts.count() >= 1)
    {
      showSubAccounts(subAccounts, accountItem, m_file, name);
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
  try
  {
    //qDebug("Selected account id: %s", accountItem->accountID().data());
    m_parentAccount = m_file->account(accountItem->accountID());
    QString theText(i18n("Is a sub account of "));
    theText += m_parentAccount.name();
    m_qcheckboxSubAccount->setText(theText);
  }
  catch (MyMoneyException *e)
  {
    qDebug("This shouldn't happen! : %s", e->what().latin1());
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
    m_bSelectedParentAccount = true;
  }
  else
  {
    m_qcheckboxSubAccount->setText(i18n("Is a sub account"));
    m_bSelectedParentAccount = false;
  }
}

void KNewAccountDlg::loadInstitutions(const QString& name)
{
  int id=-1, counter=0;
  m_qcomboboxInstitutions->clear();
  // Are we forcing the user to use institutions?
  m_qcomboboxInstitutions->insertItem(i18n("<No Institution>"));
  try
  {
    QValueList<MyMoneyInstitution> list = m_file->institutionList();
    QValueList<MyMoneyInstitution>::ConstIterator institutionIterator;
    for (institutionIterator = list.begin(), counter=1; institutionIterator != list.end(); ++institutionIterator, counter++)
    {
      if ((*institutionIterator).name() == name)
        id = counter;
      m_qcomboboxInstitutions->insertItem((*institutionIterator).name());
    }

    if (id != -1)
    {
      m_qcomboboxInstitutions->setCurrentItem(id);
    }
  }
  catch (MyMoneyException *e)
  {
    qDebug("Exception in institution load: %s", e->what().latin1());
    delete e;
  }
}

void KNewAccountDlg::slotNewClicked()
{
  MyMoneyInstitution institution;

  KNewBankDlg dlg(institution, false, this, "newbankdlg");
  if (dlg.exec())
  {
    try
    {
      institution = dlg.institution();
      m_file->addInstitution(institution);
      loadInstitutions(institution.name());
    }
    catch (MyMoneyException *e)
    {
      delete e;
      KMessageBox::information(this, i18n("Cannot add institution"));
      return;
    }
  }
}
