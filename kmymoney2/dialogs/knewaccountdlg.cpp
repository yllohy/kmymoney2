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

// ----------------------------------------------------------------------------
// QT Includes

#include <qpixmap.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qheader.h>
#include <qtooltip.h>
#include <qcheckbox.h>
#include <qtimer.h>

// ----------------------------------------------------------------------------
// KDE Headers

#include <klocale.h>
#include <kglobal.h>
#include <kmessagebox.h>
#include <kcombobox.h>
#include <klistview.h>
#include <kconfig.h>

#if QT_VERSION > 300
#include <kstandarddirs.h>
#else
#include <kstddirs.h>
#endif

// ----------------------------------------------------------------------------
// Project Includes

#include "knewaccountdlg.h"
#include "../widgets/kmymoneyedit.h"
#include "../widgets/kmymoneydateinput.h"
#include "../mymoney/mymoneyexception.h"
#include "../mymoney/mymoneyfile.h"
#include "../dialogs/knewbankdlg.h"
#include "../views/kbanklistitem.h"
#include "../views/kmymoneyfile.h"
#include "../kmymoneyutils.h"

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
  m_qlistviewParentAccounts->header()->setResizeEnabled(true);
  m_qlistviewParentAccounts->setColumnWidthMode(0, QListView::Maximum);
  m_qlistviewParentAccounts->setEnabled(false);
  // never show the horizontal scroll bar
  m_qlistviewParentAccounts->setHScrollBarMode(QScrollView::AlwaysOff);
  
  m_qcheckboxSubAccount->setText(i18n("Is a sub account"));
  
  KConfig *config = KGlobal::config();
  QFont defaultFont = QFont("helvetica", 12);
  m_qlistviewParentAccounts->header()->setFont(config->readFontEntry("listHeaderFont", &defaultFont));

  accountNameEdit->setText(account.name());
  if (isEditing)
    descriptionEdit->setText(account.description());

  typeCombo->setEnabled(true);
  
  if (categoryEditor)
  {
    m_qcheckboxSubAccount->setChecked(true);
    m_qcheckboxSubAccount->setEnabled(false);
    m_qlistviewParentAccounts->setEnabled(true);
    startDateEdit->setEnabled(false);
    startBalanceEdit->setEnabled(false);
    accountNoEdit->setEnabled(false);
    m_qcomboboxInstitutions->setEnabled(false);
    m_qbuttonNew->setEnabled(false);

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
    if (isEditing)
    {
      typeCombo->setEnabled(false);
    }
    m_qcheckboxPreferred->hide();
  }
  else
  {
    typeCombo->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Checkings));
    typeCombo->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Savings));
    typeCombo->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Cash));
    typeCombo->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::CreditCard));
    typeCombo->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Loan));
    typeCombo->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::CertificateDep));
    typeCombo->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Investment));
    typeCombo->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::MoneyMarket));
    typeCombo->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Currency));
    typeCombo->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Asset));
    typeCombo->insertItem(KMyMoneyUtils::accountTypeToString(MyMoneyAccount::Liability));

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
      case MyMoneyAccount::Asset:
        typeCombo->setCurrentItem(9);
        break;
      case MyMoneyAccount::Liability:
        typeCombo->setCurrentItem(10);
        break;
    }

    if (isEditing)
    {
      startDateEdit->setDate(account.openingDate());
      startBalanceEdit->setText(account.openingBalance().formatMoney());
      accountNoEdit->setText(account.number());
      m_qcheckboxPreferred->setChecked(account.value("PreferredAccount") == "Yes");
      typeCombo->setEnabled(false);
    }
  }

  // Load the institutions
  // then the accounts
  QString institutionName;

  MyMoneyFile *file = MyMoneyFile::instance();

  try
  {
    if (isEditing && account.institutionId() != "")
      institutionName = file->institution(account.institutionId()).name();
    else
      institutionName = "";
  }
  catch (MyMoneyException *e)
  {
    qDebug("exception in init for account dialog: %s", e->what().latin1());
    delete e;
  }

  initParentWidget(account.parentAccountId(), account.id());

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

  // make sure our account does not have an id
  if(!isEditing)
    m_account.setAccountId("");
  
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

  MyMoneyAccount::accountTypeE acctype;
  if (!m_categoryEditor)
  {
    acctype = KMyMoneyUtils::stringToAccountType(typeCombo->currentText());
  }
  else
  {
    acctype = MyMoneyFile::instance()->accountGroup(parent.accountType());
  }
  m_account.setAccountType(acctype);

  m_account.setDescription(descriptionEdit->text());

  if (!m_categoryEditor)
  {
    m_account.setOpeningBalance(startBalanceEdit->getMoneyValue());
    m_account.setOpeningDate(startDateEdit->getQDate());
    if(m_qcheckboxPreferred->isChecked())
      m_account.setValue("PreferredAccount", "Yes");
    else
      m_account.deletePair("PreferredAccount");
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

    switch (file->accountGroup(m_account.accountType()))
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
      default:
        qDebug("Seems we have an account that hasn't been mapped to the top four");
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
    parentAccount();
    parentId = m_parentAccount.id();
    delete e;
  }
  m_bSelectedParentAccount = true;

  // Now scan all 4 account roots to load the list and mark the parent
  try
  {
    if (!m_categoryEditor)
    {
      // Asset
      KAccountListItem *assetTopLevelAccount = new KAccountListItem(m_qlistviewParentAccounts,
                        assetAccount);

      if (parentId == assetAccount.id())
        m_parentItem = assetTopLevelAccount;

      for ( QCStringList::ConstIterator it = file->asset().accountList().begin();
            it != file->asset().accountList().end();
            ++it )
      {
        KAccountListItem *accountItem = new KAccountListItem(assetTopLevelAccount,
            file->account(*it));

        QCString id = file->account(*it).id();
        if(parentId == id) {
          m_parentItem = accountItem;
        } else if(accountId == id) {
          accountItem->setSelectable(false);
          m_accountItem = accountItem;
        }

        QCStringList subAccounts = file->account(*it).accountList();
        if (subAccounts.count() >= 1)
        {
          showSubAccounts(subAccounts, accountItem, parentId, accountId);
        }
      }

      // Liability
      KAccountListItem *liabilityTopLevelAccount = new KAccountListItem(m_qlistviewParentAccounts,
                        liabilityAccount);

      if (parentId == liabilityAccount.id())
        m_parentItem = liabilityTopLevelAccount;

      for ( QCStringList::ConstIterator it = file->liability().accountList().begin();
            it != file->liability().accountList().end();
            ++it )
      {
        KAccountListItem *accountItem = new KAccountListItem(liabilityTopLevelAccount,
            file->account(*it));

        QCString id = file->account(*it).id();
        if(parentId == id) {
          m_parentItem = accountItem;
        } else if(accountId == id) {
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
    else
    {
      // Income
      KAccountListItem *incomeTopLevelAccount = new KAccountListItem(m_qlistviewParentAccounts,
                        incomeAccount);

      if (parentId == incomeAccount.id())
        m_parentItem = incomeTopLevelAccount;

      for ( QCStringList::ConstIterator it = file->income().accountList().begin();
            it != file->income().accountList().end();
            ++it )
      {
        KAccountListItem *accountItem = new KAccountListItem(incomeTopLevelAccount,
            file->account(*it));

        QCString id = file->account(*it).id();
        if(parentId == id) {
          m_parentItem = accountItem;
        } else if(accountId == id) {
          accountItem->setSelectable(false);
          m_accountItem = accountItem;
        }

        QCStringList subAccounts = file->account(*it).accountList();
        if (subAccounts.count() >= 1)
        {
          showSubAccounts(subAccounts, accountItem, parentId, accountId);
        }
      }

      // Expense
      KAccountListItem *expenseTopLevelAccount = new KAccountListItem(m_qlistviewParentAccounts,
                        expenseAccount);

      if (parentId == expenseAccount.id())
        m_parentItem = expenseTopLevelAccount;

      for ( QCStringList::ConstIterator it = file->expense().accountList().begin();
            it != file->expense().accountList().end();
            ++it )
      {
        KAccountListItem *accountItem = new KAccountListItem(expenseTopLevelAccount,
            file->account(*it));

        QCString id = file->account(*it).id();
        if(parentId == id) {
          m_parentItem = accountItem;
        } else if(accountId == id) {
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
  catch (MyMoneyException *e)
  {
    qDebug("Exception in assets account refresh: %s", e->what().latin1());
    delete e;
  }

  m_qlistviewParentAccounts->setColumnWidth(0, m_qlistviewParentAccounts->width());

  if (m_parentItem)
  {
    QString theText(i18n("Is a sub account of "));
    theText += m_parentAccount.name();
    m_qcheckboxSubAccount->setText(theText);
    m_qcheckboxSubAccount->setChecked(true);
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
    QString theText(i18n("Is a sub account of "));
    theText += m_parentAccount.name();
    m_qcheckboxSubAccount->setText(theText);
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
