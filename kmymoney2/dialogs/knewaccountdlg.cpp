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
#include "../views/kmymoneyfile.h"

KNewAccountDlg::KNewAccountDlg(MyMoneyAccount& account, bool isEditing, bool categoryEditor, QWidget *parent,
    const char *name, const char *title)
  : KNewAccountDlgDecl(parent,name,true),
    m_account(account),
    m_bSelectedParentAccount(false),
    m_categoryEditor(categoryEditor)
{
  QCString pngFile = ( (categoryEditor) ? "pics/dlg_edit_category.png" : "pics/dlg_new_account.png" );
  QString columnName = ( (categoryEditor) ? i18n("Categories") : i18n("Accounts") );

  QString filename = KGlobal::dirs()->findResource("appdata", pngFile);
  m_qpixmaplabel->setPixmap(QPixmap(filename));

	m_qlistviewParentAccounts->setRootIsDecorated(true);
	m_qlistviewParentAccounts->setAllColumnsShowFocus(true);
	m_qlistviewParentAccounts->addColumn(columnName);
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
    descriptionEdit->setText(account.description());
  }

  if (categoryEditor)
  {
    m_qcheckboxSubAccount->setChecked(true);
    m_qcheckboxSubAccount->setEnabled(false);
    m_qlistviewParentAccounts->setEnabled(true);
    startDateEdit->setEnabled(false);
    startBalanceEdit->setEnabled(false);
    accountNoEdit->setEnabled(false);
    m_qcomboboxInstitutions->setEnabled(false);
    typeCombo->setEnabled(false);
    m_qbuttonNew->setEnabled(false);

    if (isEditing)
    {
      // Hardcoded but acceptable
      switch (account.accountType())
      {
        case MyMoneyAccount::Income:
          typeCombo->setCurrentItem(0);
          break;
        case MyMoneyAccount::Expense:
          typeCombo->setCurrentItem(1);
          break;
        default:
          typeCombo->setCurrentItem(0);
      }
    }
  }
  else
  {
    typeCombo->insertItem(i18n("Checking"));
    typeCombo->insertItem(i18n("Savings"));
    typeCombo->insertItem(i18n("Cash"));
    typeCombo->insertItem(i18n("Credit Card"));
    typeCombo->insertItem(i18n("Loan"));
    typeCombo->insertItem(i18n("Certificate of Deposit"));
    typeCombo->insertItem(i18n("Investment"));
    typeCombo->insertItem(i18n("Money Market"));
    typeCombo->insertItem(i18n("Currency"));
    typeCombo->insertItem(i18n("Income"));
    typeCombo->insertItem(i18n("Expense"));
    typeCombo->insertItem(i18n("Asset"));
    typeCombo->insertItem(i18n("Liability"));

    if (isEditing)
    {
      startDateEdit->setDate(account.openingDate());
      startBalanceEdit->setText(account.openingBalance().formatMoney());
      accountNoEdit->setText(account.number());

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
        case MyMoneyAccount::Income:
          typeCombo->setCurrentItem(9);
          break;
        case MyMoneyAccount::Expense:
          typeCombo->setCurrentItem(10);
          break;
        case MyMoneyAccount::Asset:
          typeCombo->setCurrentItem(11);
          break;
        case MyMoneyAccount::Liability:
          typeCombo->setCurrentItem(12);
          break;
        default:
          typeCombo->setCurrentItem(0);
      }
    }
  }

  // Load the institutions
  // then the accounts
  QString institutionName;
  QString accountName;

  MyMoneyFile *file = MyMoneyFile::instance();

  try
  {
    if (isEditing)
      institutionName = file->institution(account.institutionId()).name();
    else
      institutionName = "";
  }
  catch (MyMoneyException *e)
  {
    qDebug("exception in init for account dialog: %s", e->what().latin1());
    delete e;
  }

  try
  {
    if (isEditing)
      accountName = file->account(account.parentAccountId()).name();
    else
      accountName = "";
  }
  catch (MyMoneyException *e)
  {
    qDebug("exception in init for account dialog: %s", e->what().latin1());
    delete e;
  }

  initParentWidget(accountName);

  if (!categoryEditor)
    loadInstitutions(institutionName);

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
  QString accountNameText = accountNameEdit->text();
  if (accountNameText.isEmpty())
  {
    KMessageBox::error(this, i18n("You have not specified a name.\nPlease fill in this field."));
    accountNameEdit->setFocus();
    return;
  }

  MyMoneyAccount parent = parentAccount();
  if (m_qcheckboxSubAccount->isChecked() && (parent.name().length() == 0))
  {
    KMessageBox::error(this, i18n("Please select a parent account."));
    return;
  }

  if (!m_categoryEditor)
  {
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
        MyMoneyFile *file = MyMoneyFile::instance();

        QValueList<MyMoneyInstitution> list = file->institutionList();
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
  }

  m_account.setName(accountNameText);
  m_account.setNumber(accountNoEdit->text());

  MyMoneyAccount::accountTypeE accType;

  // Hardcoded but acceptable
  if (!m_categoryEditor)
  {
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
      case 10:
        accType = MyMoneyAccount::Income;
        break;
      case 11:
        accType = MyMoneyAccount::Expense;
        break;
      case 12:
        accType = MyMoneyAccount::Asset;
        break;
      case 13:
        accType = MyMoneyAccount::Liability;
        break;
      default:
        accType = MyMoneyAccount::UnknownAccountType;
    }
  }
  else
  {
    accType = MyMoneyFile::instance()->accountGroup(parent.accountType());
  }

	m_account.setAccountType(accType);
  m_account.setDescription(descriptionEdit->text());

  if (!m_categoryEditor)
  {
    m_account.setOpeningBalance(startBalanceEdit->getMoneyValue());
    m_account.setOpeningDate(startDateEdit->getQDate());
  }

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
    MyMoneyFile *file = MyMoneyFile::instance();

    MyMoneyAccount account;
    switch (m_file->accountGroup(m_account.accountType()))
    {
      case MyMoneyAccount::Asset:
        account = file->asset();
        break;
      case MyMoneyAccount::Liability:
        account = file->liability();
        break;
      case MyMoneyAccount::Income:
        account = file->income();
        break;
      case MyMoneyAccount::Expense:
        account = file->expense();
        break;
      default:
        //qDebug("Seems we have an account that hasn't been mapped to the top four");
        account = file->asset();
    }
    return account;
  }
}

void KNewAccountDlg::initParentWidget(const QString& name)
{
  MyMoneyFile *file = MyMoneyFile::instance();

  MyMoneyAccount liabilityAccount = file->liability();
  MyMoneyAccount assetAccount = file->asset();
  MyMoneyAccount expenseAccount = file->expense();
  MyMoneyAccount incomeAccount = file->income();

  m_bFoundItem = false;

  // Do all 4 account roots
  try
  {
    if (!m_categoryEditor)
    {
      // Asset
      KAccountListItem *assetTopLevelAccount = new KAccountListItem(m_qlistviewParentAccounts,
                        assetAccount);

      if (name.length()>=1 && name == assetAccount.name())
      {
        m_bFoundItem = true;
        m_foundItem = assetTopLevelAccount;
      }

      for ( QCStringList::ConstIterator it = file->asset().accountList().begin();
            it != file->asset().accountList().end();
            ++it )
      {
        KAccountListItem *accountItem = new KAccountListItem(assetTopLevelAccount,
            file->account(*it));

        if (name.length()>=1 && name == file->account(*it).name())
        {
          m_bFoundItem = true;
          m_foundItem = accountItem;
        }

        QCStringList subAccounts = file->account(*it).accountList();
        if (subAccounts.count() >= 1)
        {
          showSubAccounts(subAccounts, accountItem, file, name);
        }
      }

      // Liability
      KAccountListItem *liabilityTopLevelAccount = new KAccountListItem(m_qlistviewParentAccounts,
                        liabilityAccount);

      if (name.length()>=1 && name == liabilityAccount.name())
      {
        m_bFoundItem = true;
        m_foundItem = liabilityTopLevelAccount;
      }

      for ( QCStringList::ConstIterator it = file->liability().accountList().begin();
            it != file->liability().accountList().end();
            ++it )
      {
        KAccountListItem *accountItem = new KAccountListItem(liabilityTopLevelAccount,
            file->account(*it));

        if (name.length()>=1 && name == file->account(*it).name())
        {
          m_bFoundItem = true;
          m_foundItem = accountItem;
        }

        QCStringList subAccounts = file->account(*it).accountList();
        if (subAccounts.count() >= 1)
        {
          showSubAccounts(subAccounts, accountItem, file, name);
        }
      }
    }

    // Income
    KAccountListItem *incomeTopLevelAccount = new KAccountListItem(m_qlistviewParentAccounts,
                      incomeAccount);

    if (name.length()>=1 && name == incomeAccount.name())
    {
      m_bFoundItem = true;
      m_foundItem = incomeTopLevelAccount;
    }

    for ( QCStringList::ConstIterator it = file->income().accountList().begin();
          it != file->income().accountList().end();
          ++it )
    {
      KAccountListItem *accountItem = new KAccountListItem(incomeTopLevelAccount,
          file->account(*it));

      if (name.length()>=1 && name == file->account(*it).name())
      {
        m_bFoundItem = true;
        m_foundItem = accountItem;
      }

      QCStringList subAccounts = file->account(*it).accountList();
      if (subAccounts.count() >= 1)
      {
        showSubAccounts(subAccounts, accountItem, file, name);
      }
    }

    // Expense
    KAccountListItem *expenseTopLevelAccount = new KAccountListItem(m_qlistviewParentAccounts,
                      expenseAccount);

    if (name.length()>=1 && name == expenseAccount.name())
    {
      m_bFoundItem = true;
      m_foundItem = expenseTopLevelAccount;
    }

    for ( QCStringList::ConstIterator it = file->expense().accountList().begin();
          it != file->expense().accountList().end();
          ++it )
    {
      KAccountListItem *accountItem = new KAccountListItem(expenseTopLevelAccount,
          file->account(*it));

      if (name.length()>=1 && name == file->account(*it).name())
      {
        m_bFoundItem = true;
        m_foundItem = accountItem;
      }

      QCStringList subAccounts = file->account(*it).accountList();
      if (subAccounts.count() >= 1)
      {
        showSubAccounts(subAccounts, accountItem, file, name);
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
    QString parentAccount;

    try
    {
      parentAccount = file->account(m_foundItem->accountID()).name();
    }
    catch (MyMoneyException *e)
    {
      qDebug("Exception in select account item in the new account dialog: %s", e->what().latin1());
      delete e;
    }

    QString theText(i18n("Is a sub account of "));
    theText += parentAccount;
    m_qcheckboxSubAccount->setText(theText);
    m_qcheckboxSubAccount->setChecked(true);

    m_qlistviewParentAccounts->setEnabled(true);

    m_qlistviewParentAccounts->setOpen(m_foundItem, true);
    m_qlistviewParentAccounts->setSelected(m_foundItem, true);
  }
}

void KNewAccountDlg::showSubAccounts(QCStringList accounts, KAccountListItem *parentItem, MyMoneyFile *file,
  const QString& name)
{
  for ( QCStringList::ConstIterator it = accounts.begin(); it != accounts.end(); ++it )
  {
    KAccountListItem *accountItem  = new KAccountListItem(parentItem,
          file->account(*it));

      if (name.length()>=1 && name == file->account(*it).name())
      {
        m_bFoundItem = true;
        m_foundItem = accountItem;
      }

    QCStringList subAccounts = file->account(*it).accountList();
    if (subAccounts.count() >= 1)
    {
      showSubAccounts(subAccounts, accountItem, file, name);
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
    MyMoneyFile *file = MyMoneyFile::instance();

    //qDebug("Selected account id: %s", accountItem->accountID().data());
    m_parentAccount = file->account(accountItem->accountID());
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
    MyMoneyFile *file = MyMoneyFile::instance();

    QValueList<MyMoneyInstitution> list = file->institutionList();
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
      MyMoneyFile *file = MyMoneyFile::instance();

      institution = dlg.institution();
      file->addInstitution(institution);
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
