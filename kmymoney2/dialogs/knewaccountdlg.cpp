/***************************************************************************
                          knewaccountdlg.cpp
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes
                               2004 by Thomas Baumgart
    email                : mte@users.sourceforge.net
                           ipwizard@users.sourceforge.net
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

#include <qpixmap.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qheader.h>
#include <qtooltip.h>
#include <qcheckbox.h>
#include <qtimer.h>
#include <qtabwidget.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>

// ----------------------------------------------------------------------------
// KDE Headers

#include <klocale.h>
#include <kglobal.h>
#include <kmessagebox.h>
#include <kcombobox.h>
#include <klistview.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <kguiitem.h>
#include <kpushbutton.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "knewaccountdlg.h"
#include "../widgets/kmymoneyedit.h"
#include "../widgets/kmymoneydateinput.h"
#include "../widgets/kmymoneycurrencyselector.h"
#include "../widgets/kmymoneyequity.h"
#include "../widgets/kmymoneyaccountselector.h"

#include "../mymoney/mymoneyexception.h"
#include "../mymoney/mymoneyfile.h"
#include "../dialogs/knewbankdlg.h"
#include "../views/kbanklistitem.h"
#include "../views/kmymoneyfile.h"
#include "../kmymoneyutils.h"

#define TAB_GENERAL      0
#define TAB_TAX          1

KNewAccountDlg::KNewAccountDlg(const MyMoneyAccount& account, bool isEditing, bool categoryEditor, QWidget *parent,
    const char *name, const char *title)
  : KNewAccountDlgDecl(parent,name,true),
    m_account(account),
    m_bSelectedParentAccount(false),
    m_categoryEditor(categoryEditor),
    m_isEditing(isEditing)
{
  QString columnName = ( (categoryEditor) ? i18n("Categories") : i18n("Accounts") );

  m_qlistviewParentAccounts->setRootIsDecorated(true);
  m_qlistviewParentAccounts->setAllColumnsShowFocus(true);
  m_qlistviewParentAccounts->addColumn(columnName);
  m_qlistviewParentAccounts->setMultiSelection(false);
  m_qlistviewParentAccounts->header()->setResizeEnabled(true);
  m_qlistviewParentAccounts->setColumnWidthMode(0, QListView::Maximum);
  m_qlistviewParentAccounts->setEnabled(false);
  // never show the horizontal scroll bar
  m_qlistviewParentAccounts->setHScrollBarMode(QScrollView::AlwaysOff);

  m_subAccountLabel->setText(i18n("Is a sub account"));

  m_qlistviewParentAccounts->header()->setFont(KMyMoneyUtils::headerFont());

  accountNameEdit->setText(account.name());
  descriptionEdit->setText(account.description());

  typeCombo->setEnabled(true);
  MyMoneyFile *file = MyMoneyFile::instance();

  if (categoryEditor)
  {

    m_qlistviewParentAccounts->setEnabled(true);
    startDateEdit->setEnabled(false);
    accountNoEdit->setEnabled(false);

    m_institutionBox->hide();

    typeCombo->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Income));
    typeCombo->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Expense));

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
    if (m_isEditing)
    {
      typeCombo->setEnabled(false);
    }
    m_qcheckboxPreferred->hide();
    m_currency->setEnabled(false);
    m_equityText->hide();
    m_equity->hide();

    m_qcheckboxTax->setChecked(account.value("Tax") == "Yes");
    loadVatAccounts();
  }
  else
  {
    QWidget* tab = m_tab->page(TAB_TAX);
    if(tab) {
      m_tab->removePage(tab);
    }

    typeCombo->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Checkings));
    typeCombo->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Savings));
    typeCombo->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Cash));
    typeCombo->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::CreditCard));
    typeCombo->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Loan));
    typeCombo->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Investment));
    typeCombo->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Asset));
    typeCombo->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Liability));
    typeCombo->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Stock));
/*
    typeCombo->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::CertificateDep));
    typeCombo->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::MoneyMarket));
    typeCombo->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Currency));
*/

    // Hardcoded but acceptable
    switch (account.accountType())
    {
      default:
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
      case MyMoneyAccount::Investment:
        typeCombo->setCurrentItem(5);
        break;
      case MyMoneyAccount::Asset:
        typeCombo->setCurrentItem(6);
        break;
      case MyMoneyAccount::Liability:
        typeCombo->setCurrentItem(7);
        break;
      case MyMoneyAccount::Stock:
        m_institutionBox->hide();
        typeCombo->setCurrentItem(8);
        break;
/*
      case MyMoneyAccount::CertificateDep:
        typeCombo->setCurrentItem(5);
        break;
      case MyMoneyAccount::MoneyMarket:
        typeCombo->setCurrentItem(7);
        break;
      case MyMoneyAccount::Currency:
        typeCombo->setCurrentItem(8);
        break;
*/
    }

    if(!m_account.openingDate().isValid())
      m_account.setOpeningDate(QDate::currentDate());

    startDateEdit->setDate(m_account.openingDate());
    accountNoEdit->setText(account.number());
    m_qcheckboxPreferred->setChecked(account.value("PreferredAccount") == "Yes");

    if(account.accountType() == MyMoneyAccount::Stock) {
      m_equity->loadEquity(account.currencyId());
    } else {
      m_currency->setSecurity(file->currency(account.currencyId()));
    }

    // we do not allow to change the account type once an account
    // was created. Same applies to currency.
    if (m_isEditing)
    {
      typeCombo->setEnabled(false);
      m_currency->setEnabled(false);
    }
    if(m_account.accountType() == MyMoneyAccount::Stock) {
      typeCombo->setEnabled(false);
      m_qcheckboxPreferred->hide();
      m_currencyText->hide();
      m_currency->hide();
    } else {
      m_equityText->hide();
      m_equity->hide();
    }

    m_qcheckboxTax->hide();
  }

  // Load the institutions
  // then the accounts
  QString institutionName;

  try
  {
    if (m_isEditing && !account.institutionId().isEmpty())
      institutionName = file->institution(account.institutionId()).name();
    else
      institutionName = QString();
  }
  catch (MyMoneyException *e)
  {
    qDebug("exception in init for account dialog: %s", e->what().latin1());
    delete e;
  }

  initParentWidget(account.parentAccountId(), account.id());

  if(m_account.accountType() == MyMoneyAccount::Stock)
    m_qlistviewParentAccounts->setEnabled(false);

  if (!categoryEditor)
    loadInstitutions(institutionName);

  accountNameEdit->setFocus();

  if (title)
    setCaption(title);

  // load button icons
  KIconLoader* il = KGlobal::iconLoader();
  KGuiItem cancelButtenItem( i18n( "&Cancel" ),
                      QIconSet(il->loadIcon("button_cancel", KIcon::Small, KIcon::SizeSmall)),
                      i18n("Reject any changes"),
                      i18n("Use this to abort the account/category dialog"));
  cancelButton->setGuiItem(cancelButtenItem);

  KGuiItem okButtenItem( i18n( "&OK" ),
                      QIconSet(il->loadIcon("button_ok", KIcon::Small, KIcon::SizeSmall)),
                      i18n("Accept modifications"),
                      i18n("Use this to accept the data and possibly create the account/category"));
  createButton->setGuiItem(okButtenItem);

  connect(cancelButton, SIGNAL(clicked()), SLOT(reject()));
  connect(createButton, SIGNAL(clicked()), this, SLOT(okClicked()));
  connect(m_qlistviewParentAccounts, SIGNAL(selectionChanged(QListViewItem*)),
    this, SLOT(slotSelectionChanged(QListViewItem*)));
  connect(m_qbuttonNew, SIGNAL(clicked()), this, SLOT(slotNewClicked()));
  connect(typeCombo, SIGNAL(activated(const QString&)),
    this, SLOT(slotAccountTypeChanged(const QString&)));

  connect(accountNameEdit, SIGNAL(textChanged(const QString&)), this, SLOT(slotCheckFinished()));
  // connect(m_equity, SIGNAL(equityChanged(const QCString&)), this, SLOT(slotCheckFinished()));
  connect(m_equity, SIGNAL(textChanged(const QString&)), this, SLOT(slotCheckFinished()));

  connect(m_vatCategory, SIGNAL(toggled(bool)), this, SLOT(slotVatChanged(bool)));
  connect(m_vatAssignment, SIGNAL(toggled(bool)), this, SLOT(slotVatAssignmentChanged(bool)));
  connect(m_vatCategory, SIGNAL(toggled(bool)), this, SLOT(slotCheckFinished()));
  connect(m_vatAssignment, SIGNAL(toggled(bool)), this, SLOT(slotCheckFinished()));
  connect(m_vatRate, SIGNAL(textChanged(const QString&)), this, SLOT(slotCheckFinished()));
  connect(m_vatAccount, SIGNAL(stateChanged()), this, SLOT(slotCheckFinished()));

  m_vatCategory->setChecked(false);
  m_vatAssignment->setChecked(false);

  // make sure our account does not have an id and no parent assigned
  // and certainly no children in case we create a new account
  if(!m_isEditing) {
    m_account.setAccountId(QCString());
    m_account.setParentAccountId(QCString());
    QCStringList::ConstIterator it;
    while((it = m_account.accountList().begin()) != m_account.accountList().end())
      m_account.removeAccountId(*it);

    if(m_parentItem == 0) {
      // force loading of initial parent
      m_account.setAccountType(MyMoneyAccount::UnknownAccountType);
      MyMoneyAccount::_accountTypeE type = account.accountType();
      if(type == MyMoneyAccount::UnknownAccountType)
        type = MyMoneyAccount::Checkings;
      slotAccountTypeChanged(KMyMoneyUtils::accountTypeToString(type));
    }
  } else {
    if(categoryEditor) {
      if(!m_account.value("VatRate").isEmpty()) {
        m_vatCategory->setChecked(true);
        m_vatRate->setValue(MyMoneyMoney(m_account.value("VatRate"))*MyMoneyMoney(100,1));
      } else {
        if(!m_account.value("VatAccount").isEmpty()) {
          QCString accId = m_account.value("VatAccount").latin1();
          try {
            // make sure account exists
            MyMoneyFile::instance()->account(accId);
            m_vatAssignment->setChecked(true);
            m_vatAccount->setSelected(accId);
            m_grossAmount->setChecked(true);
            if(m_account.value("VatAmount") == "Net")
              m_netAmount->setChecked(true);
          } catch(MyMoneyException *e) {
            delete e;
          }
        }
      }
    }
  }
  slotVatChanged(m_vatCategory->isChecked());
  slotVatAssignmentChanged(m_vatAssignment->isChecked());
  slotCheckFinished();

  // using a timeout is the only way, I got the 'ensureItemVisible'
  // working when creating the dialog. I assume, this
  // has something to do with the delayed update of the display somehow.
  QTimer::singleShot(50, this, SLOT(timerDone()));
}

void KNewAccountDlg::timerDone(void)
{
  if(m_accountItem) m_qlistviewParentAccounts->ensureItemVisible(m_accountItem);
  if(m_parentItem) m_qlistviewParentAccounts->ensureItemVisible(m_parentItem);
  // KNewAccountDlgDecl::resizeEvent(0);
  m_qlistviewParentAccounts->setColumnWidth(0, m_qlistviewParentAccounts->visibleWidth());
  m_qlistviewParentAccounts->repaintContents(false);
}

KNewAccountDlg::~KNewAccountDlg()
{
}

void KNewAccountDlg::okClicked()
{
  MyMoneyFile* file = MyMoneyFile::instance();

  QString accountNameText = accountNameEdit->text();
  if (accountNameText.isEmpty())
  {
    KMessageBox::error(this, i18n("You have not specified a name.\nPlease fill in this field."));
    accountNameEdit->setFocus();
    return;
  }

  MyMoneyAccount parent = parentAccount();
  if (parent.name().length() == 0)
  {
    KMessageBox::error(this, i18n("Please select a parent account."));
    return;
  }

  if (!m_categoryEditor)
  {
    QString institutionNameText = m_qcomboboxInstitutions->currentText();
    if (institutionNameText != i18n("<No Institution>"))
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

  MyMoneyAccount::accountTypeE acctype;
  if (!m_categoryEditor)
  {
    acctype = KMyMoneyUtils::stringToAccountType(typeCombo->currentText());
    // If it's a loan, check if the parent is asset or liability. In
    // case of asset, we change the account type to be AssetLoan
    if(acctype == MyMoneyAccount::Loan
    && file->accountGroup(parent.accountType()) == MyMoneyAccount::Asset)
      acctype = MyMoneyAccount::AssetLoan;

    if(!file->nameToAccount(accountNameText).isEmpty()
    && (file->nameToAccount(accountNameText) != m_account.id())) {
      KMessageBox::error(this, i18n("Account with that name already exists."));
      return;
    }
  }
  else
  {
    acctype = MyMoneyFile::instance()->accountGroup(parent.accountType());
    if(!file->categoryToAccount(accountNameText).isEmpty()
    && (file->categoryToAccount(accountNameText) != m_account.id())) {
      KMessageBox::error(this, i18n("Category with that name already exists."));
      return;
    }
  }
  m_account.setAccountType(acctype);

  m_account.setDescription(descriptionEdit->text());

  if (!m_categoryEditor)
  {
    m_account.setOpeningDate(startDateEdit->getQDate());
    if(m_account.accountType() == MyMoneyAccount::Stock) {
      m_account.setCurrencyId(m_equity->id());
    } else {
      m_account.setCurrencyId(m_currency->security().id());
    }

    if(m_qcheckboxPreferred->isChecked())
      m_account.setValue("PreferredAccount", "Yes");
    else
      m_account.deletePair("PreferredAccount");
  }
  else
  {
    KConfig *kconfig = KGlobal::config();
    kconfig->setGroup("List Options");
    if(kconfig->readBoolEntry("HideUnusedCategory", false) == true) {
      KMessageBox::information(this, i18n("You have selected to suppress the display of unused categories in the KMyMoney configuration dialog. The category you just created will therefore only be shown if it is used. Otherwise, it will be hidden in the accounts/categories view."), i18n("Hidden categories"), "NewHiddenCategory");
    }
  }

  if (m_categoryEditor)
  {
    if ( m_qcheckboxTax->isChecked())
      m_account.setValue("Tax", "Yes");
    else
      m_account.deletePair("Tax");

    m_account.deletePair("VatAccount");
    m_account.deletePair("VatAmount");
    m_account.deletePair("VatRate");

    if(m_vatCategory->isChecked()) {
      m_account.setValue("VatRate", (m_vatRate->value().abs() / MyMoneyMoney(100,1)).toString());
    } else {
      if(m_vatAssignment->isChecked()) {
        m_account.setValue("VatAccount", m_vatAccount->selectedAccounts().first());
        if(m_netAmount->isChecked())
          m_account.setValue("VatAmount", "Net");
      }
    }
  }

  accept();
}

const MyMoneyAccount& KNewAccountDlg::account(void) const
{
  return m_account;
}

const MyMoneyAccount& KNewAccountDlg::parentAccount(void)
{
  if (!m_bSelectedParentAccount)
  {
    MyMoneyFile *file = MyMoneyFile::instance();

    switch (m_account.accountGroup())
    {
      case MyMoneyAccount::Asset:
        m_parentAccount = file->asset();
        break;
      case MyMoneyAccount::Liability:
        m_parentAccount = file->liability();
        break;
      case MyMoneyAccount::Income:
        m_parentAccount = file->income();
        break;
      case MyMoneyAccount::Expense:
        m_parentAccount = file->expense();
        break;
      case MyMoneyAccount::Equity:
        m_parentAccount = file->equity();
        break;
      default:
        qDebug("Seems we have an account that hasn't been mapped to the top five");
        if(m_categoryEditor)
          m_parentAccount = file->income();
        else
          m_parentAccount = file->asset();
    }
  }
  return m_parentAccount;
}

void KNewAccountDlg::initParentWidget(QCString parentId, const QCString& accountId)
{
  MyMoneyFile *file = MyMoneyFile::instance();

  MyMoneyAccount liabilityAccount = file->liability();
  MyMoneyAccount assetAccount = file->asset();
  MyMoneyAccount expenseAccount = file->expense();
  MyMoneyAccount incomeAccount = file->income();
  MyMoneyAccount equityAccount = file->equity();

  m_parentItem = 0;
  m_accountItem = 0;

  // Determine the parent account
  try
  {
    m_parentAccount = file->account(parentId);
  }
  catch (MyMoneyException *e)
  {
    m_bSelectedParentAccount = false;
    m_parentAccount = MyMoneyAccount();
    if(m_account.accountType() != MyMoneyAccount::UnknownAccountType) {
      parentAccount();
      parentId = m_parentAccount.id();
    }
    delete e;
  }
  m_bSelectedParentAccount = true;

  // extract the account type from the combo box
  MyMoneyAccount::accountTypeE type;
  MyMoneyAccount::accountTypeE groupType;
  type = KMyMoneyUtils::stringToAccountType(typeCombo->currentText());
  groupType = file->accountGroup(type);

  m_qlistviewParentAccounts->clear();

  // Now scan all 4 account roots to load the list and mark the parent
  try
  {
    if (!m_categoryEditor)
    {
      if(groupType == MyMoneyAccount::Asset || type == MyMoneyAccount::Loan) {
        // Asset
        KAccountListItem *assetTopLevelAccount = new KAccountListItem(m_qlistviewParentAccounts,
                          assetAccount);

        if(m_parentAccount.id().isEmpty()) {
          m_parentAccount = assetAccount;
          parentId = m_parentAccount.id();
        }

        if (parentId == assetAccount.id())
          m_parentItem = assetTopLevelAccount;

        assetTopLevelAccount->setOpen(true);

        for ( QCStringList::ConstIterator it = assetAccount.accountList().begin();
              it != assetAccount.accountList().end();
              ++it )
        {
          KAccountListItem *accountItem = new KAccountListItem(assetTopLevelAccount,
              file->account(*it));

          QCString id = file->account(*it).id();
          if(parentId == id) {
            m_parentItem = accountItem;
          } else if(accountId == id) {
            if(m_isEditing)
              accountItem->setSelectable(false);
            m_accountItem = accountItem;
          }

          QCStringList subAccounts = file->account(*it).accountList();
          if (subAccounts.count() >= 1)
          {
            showSubAccounts(subAccounts, accountItem, parentId, accountId);
          }
        }
      }

      if(groupType == MyMoneyAccount::Liability) {
        // Liability
        KAccountListItem *liabilityTopLevelAccount = new KAccountListItem(m_qlistviewParentAccounts,
                          liabilityAccount);

        if(m_parentAccount.id().isEmpty()) {
          m_parentAccount = liabilityAccount;
          parentId = m_parentAccount.id();
        }

        if (parentId == liabilityAccount.id())
          m_parentItem = liabilityTopLevelAccount;

        liabilityTopLevelAccount->setOpen(true);

        for ( QCStringList::ConstIterator it = liabilityAccount.accountList().begin();
              it != liabilityAccount.accountList().end();
              ++it )
        {
          KAccountListItem *accountItem = new KAccountListItem(liabilityTopLevelAccount,
              file->account(*it));

          QCString id = file->account(*it).id();
          if(parentId == id) {
            m_parentItem = accountItem;
          } else if(accountId == id) {
            if(m_isEditing)
              accountItem->setSelectable(false);
            m_accountItem = accountItem;
          }

          QCStringList subAccounts = file->account(*it).accountList();
          if (subAccounts.count() >= 1)
          {
            showSubAccounts(subAccounts, accountItem, parentId, accountId);
          }
        }
      }
    }
    else
    {
      if(groupType == MyMoneyAccount::Income) {
        // Income
        KAccountListItem *incomeTopLevelAccount = new KAccountListItem(m_qlistviewParentAccounts,
                          incomeAccount);

        if(m_parentAccount.id().isEmpty()) {
          m_parentAccount = incomeAccount;
          parentId = m_parentAccount.id();
        }

        if (parentId == incomeAccount.id())
          m_parentItem = incomeTopLevelAccount;

        incomeTopLevelAccount->setOpen(true);

        for ( QCStringList::ConstIterator it = incomeAccount.accountList().begin();
              it != incomeAccount.accountList().end();
              ++it )
        {
          KAccountListItem *accountItem = new KAccountListItem(incomeTopLevelAccount,
              file->account(*it));

          QCString id = file->account(*it).id();
          if(parentId == id) {
            m_parentItem = accountItem;
          } else if(accountId == id) {
            if(m_isEditing)
              accountItem->setSelectable(false);
            m_accountItem = accountItem;
          }

          QCStringList subAccounts = file->account(*it).accountList();
          if (subAccounts.count() >= 1)
          {
            showSubAccounts(subAccounts, accountItem, parentId, accountId);
          }
        }
      }

      if(groupType == MyMoneyAccount::Expense) {
        // Expense
        KAccountListItem *expenseTopLevelAccount = new KAccountListItem(m_qlistviewParentAccounts,
                          expenseAccount);

        if(m_parentAccount.id().isEmpty()) {
          m_parentAccount = expenseAccount;
          parentId = m_parentAccount.id();
        }

        if (parentId == expenseAccount.id())
          m_parentItem = expenseTopLevelAccount;

        expenseTopLevelAccount->setOpen(true);

        for ( QCStringList::ConstIterator it = expenseAccount.accountList().begin();
              it != expenseAccount.accountList().end();
              ++it )
        {
          KAccountListItem *accountItem = new KAccountListItem(expenseTopLevelAccount,
              file->account(*it));

          QCString id = file->account(*it).id();
          if(parentId == id) {
            m_parentItem = accountItem;
          } else if(accountId == id) {
            if(m_isEditing)
              accountItem->setSelectable(false);
            m_accountItem = accountItem;
          }

          QCStringList subAccounts = file->account(*it).accountList();
          if (subAccounts.count() >= 1)
          {
            showSubAccounts(subAccounts, accountItem, parentId, accountId);
          }
        }
      }
    }
  }
  catch (MyMoneyException *e)
  {
    qDebug("Exception in assets account refresh: %s", e->what().latin1());
    delete e;
  }

  m_qlistviewParentAccounts->setColumnWidth(0, m_qlistviewParentAccounts->width());

  if (m_parentItem)
  {
    m_subAccountLabel->setText(i18n("Is a sub account of %1").arg(m_parentAccount.name()));
    m_qlistviewParentAccounts->setOpen(m_parentItem, true);
    m_qlistviewParentAccounts->setSelected(m_parentItem, true);
  }

  m_qlistviewParentAccounts->setEnabled(true);
}

void KNewAccountDlg::showSubAccounts(QCStringList accounts, KAccountListItem *parentItem,
                                     const QCString& parentId, const QCString& accountId)
{
  MyMoneyFile *file = MyMoneyFile::instance();

  for ( QCStringList::ConstIterator it = accounts.begin(); it != accounts.end(); ++it )
  {
    KAccountListItem *accountItem  = new KAccountListItem(parentItem,
          file->account(*it));

    QCString id = file->account(*it).id();
    if(parentId == id) {
      m_parentItem = accountItem;
    } else if(accountId == id) {
      if(m_isEditing)
        accountItem->setSelectable(false);
      m_accountItem = accountItem;
    }

    QCStringList subAccounts = file->account(*it).accountList();
    if (subAccounts.count() >= 1)
    {
      showSubAccounts(subAccounts, accountItem, parentId, accountId);
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
    m_subAccountLabel->setText(i18n("Is a sub account of %1").arg(m_parentAccount.name()));
    if(m_qlistviewParentAccounts->isEnabled()) {
      m_bSelectedParentAccount = true;
    }
  }
  catch (MyMoneyException *e)
  {
    qDebug("This shouldn't happen! : %s", e->what().latin1());
    delete e;
  }
}

void KNewAccountDlg::loadVatAccounts(void)
{
  QValueList<MyMoneyAccount> list = MyMoneyFile::instance()->accountList();
  QValueList<MyMoneyAccount>::Iterator it;
  QCStringList loadListExpense;
  QCStringList loadListIncome;
  for(it = list.begin(); it != list.end(); ++it) {
    if(!(*it).value("VatRate").isEmpty()) {
      if((*it).accountType() == MyMoneyAccount::Expense)
        loadListExpense += (*it).id();
      else
        loadListIncome += (*it).id();
    }
  }
  m_vatAccount->loadList(i18n("Income"), loadListIncome, true);
  m_vatAccount->loadList(i18n("Expense"), loadListExpense, false);
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

void KNewAccountDlg::slotAccountTypeChanged(const QString& typeStr)
{
  MyMoneyAccount::accountTypeE type;
  MyMoneyAccount::accountTypeE oldType;
  MyMoneyFile* file = MyMoneyFile::instance();

  type = KMyMoneyUtils::stringToAccountType(typeStr);
  try {
    oldType = m_account.accountType();
    if(oldType != type) {
      QCString parentId;
      switch(file->accountGroup(type)) {
        case MyMoneyAccount::Asset:
          parentId = file->asset().id();
          break;
        case MyMoneyAccount::Liability:
          parentId = file->liability().id();
          break;
        case MyMoneyAccount::Expense:
          parentId = file->expense().id();
          break;
        case MyMoneyAccount::Income:
          parentId = file->income().id();
          break;
        default:
          qWarning("Unknown account group in KNewAccountDlg::slotAccountTypeChanged()");
          break;
      }
      initParentWidget(parentId, QCString());
    }
  } catch(MyMoneyException *e) {
    delete e;
    qWarning("Unexpected exception in KNewAccountDlg::slotAccountTypeChanged()");
  }
}

void KNewAccountDlg::slotCheckFinished(void)
{
  bool showButton = true;

  if(accountNameEdit->text().length() == 0) {
    showButton = false;
  }

  if(m_account.accountType() == MyMoneyAccount::Stock) {
    if(m_equity->text().length() == 0) {
      showButton = false;
    }
  }

  if(m_vatCategory->isChecked() && m_vatRate->value() <= MyMoneyMoney(0)) {
    showButton = false;
  } else {
    if(m_vatAssignment->isChecked() && m_vatAccount->selectedAccounts().isEmpty())
      showButton = false;
  }
  createButton->setEnabled(showButton);
}

void KNewAccountDlg::slotVatChanged(bool state)
{
  if(state) {
    m_vatCategoryFrame->show();
    m_vatAssignmentFrame->hide();
  } else {
    m_vatCategoryFrame->hide();
    m_vatAssignmentFrame->show();
  }
}

void KNewAccountDlg::slotVatAssignmentChanged(bool state)
{
  m_vatAccount->setEnabled(state);
  m_amountGroup->setEnabled(state);
}
