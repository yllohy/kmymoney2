/***************************************************************************
                          kcategoriesview.cpp  -  description
                             -------------------
    begin                : Sun Jan 20 2002
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

// ----------------------------------------------------------------------------
// QT Includes

#include <qheader.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qdatetime.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klistview.h>
#include <klocale.h>
#include <kglobal.h>
#if QT_VERSION > 300
#include <kstandarddirs.h>
#else
#include <kstddirs.h>
#endif

#include <kmessagebox.h>
#include <kconfig.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "../mymoney/mymoneyfile.h"
#include "../views/kmymoneyfile.h"
#include "../dialogs/knewaccountdlg.h"
#include "kcategoriesview.h"
#include "kbanklistitem.h"

KCategoriesView::KCategoriesView(QWidget *parent, const char *name )
  : kCategoriesViewDecl(parent,name)
{
  categoryListView->setRootIsDecorated(true);
  categoryListView->addColumn(i18n("Category"));
  categoryListView->addColumn(i18n("Type"));

  // FIXME: We could make this configurable
  categoryListView->addColumn(i18n("Balance"));

  categoryListView->setMultiSelection(false);
  categoryListView->setColumnWidthMode(0, QListView::Maximum);
  categoryListView->setColumnWidthMode(1, QListView::Maximum);
  categoryListView->setColumnWidthMode(2, QListView::Maximum);
  categoryListView->header()->setResizeEnabled(true);

  categoryListView->setAllColumnsShowFocus(true);

  categoryListView->setColumnAlignment(2, Qt::AlignRight);
  categoryListView->setResizeMode(QListView::AllColumns);

  // never show a horizontal scroll bar
  categoryListView->setHScrollBarMode(QScrollView::AlwaysOff);

  categoryListView->setSorting(0);

  readConfig();

  connect(categoryListView, SIGNAL(selectionChanged(QListViewItem*)),
    this, SLOT(slotSelectionChanged(QListViewItem*)));
  connect(categoryListView, SIGNAL(rightButtonPressed(QListViewItem* , const QPoint&, int)),
    this, SLOT(slotListRightMouse(QListViewItem*, const QPoint&, int)));
  connect(buttonEdit, SIGNAL(clicked()), this, SLOT(slotEditClicked()));
  connect(buttonNew, SIGNAL(clicked()), this, SLOT(slotNewClicked()));
  connect(buttonDelete, SIGNAL(clicked()), this, SLOT(slotDeleteClicked()));

  m_suspendUpdate = false;

  MyMoneyFile::instance()->attach(MyMoneyFile::NotifyClassAccountHierarchy, this);
}

KCategoriesView::~KCategoriesView()
{
  MyMoneyFile::instance()->detach(MyMoneyFile::NotifyClassAccountHierarchy, this);
  writeConfig();
}

void KCategoriesView::slotRefreshView(void)
{
  KConfig *config = KGlobal::config();
  config->setGroup("List Options");
  QFont defaultFont = QFont("helvetica", 12);
  categoryListView->header()->setFont(config->readFontEntry("listHeaderFont", &defaultFont));
  m_hideCategory = config->readBoolEntry("HideUnusedCategory", false);
  bool accountUsed;

  categoryListView->clear();
  m_accountMap.clear();
  m_transactionCountMap = MyMoneyFile::instance()->transactionCountMap();

  MyMoneyFile *file = MyMoneyFile::instance();

  try
  {
    MyMoneyAccount expenseAccount = file->expense();
    MyMoneyAccount incomeAccount = file->income();

    // Income
    KAccountListItem *incomeTopLevelAccount = new KAccountListItem(categoryListView,
              incomeAccount);

    QValueList<MyMoneyAccount> list = file->accountList();
    QValueList<MyMoneyAccount>::ConstIterator it_a;

    for(it_a = list.begin(); it_a != list.end(); ++it_a)
      m_accountMap[(*it_a).id()] = *it_a;

    for ( QCStringList::ConstIterator it = incomeAccount.accountList().begin();
          it != incomeAccount.accountList().end();
          ++it )
    {
      KAccountListItem *accountItem = new KAccountListItem(incomeTopLevelAccount,
            m_accountMap[*it]);

      accountUsed = m_transactionCountMap[*it] > 0;

      QCStringList subAccounts = m_accountMap[*it].accountList();
      if (subAccounts.count() >= 1)
      {
        accountUsed |= showSubAccounts(subAccounts, accountItem, i18n("Income"));
      }
      if(accountUsed == false && m_hideCategory == true) {
        // in case hide category is on and the account or any of it's
        // subaccounts has no split, we can safely remove it and all
        // it's sub-ordinate accounts from the list
        delete accountItem;
      }
    }

    // Expense
    KAccountListItem *expenseTopLevelAccount = new KAccountListItem(categoryListView,
              expenseAccount);

    for ( QCStringList::ConstIterator it = expenseAccount.accountList().begin();
          it != expenseAccount.accountList().end();
          ++it )
    {
      KAccountListItem *accountItem = new KAccountListItem(expenseTopLevelAccount,
            m_accountMap[*it]);

      accountUsed = m_transactionCountMap[*it] > 0;

      QCStringList subAccounts = m_accountMap[*it].accountList();
      if (subAccounts.count() >= 1)
      {
        accountUsed |= showSubAccounts(subAccounts, accountItem, i18n("Expense"));
      }
      if(accountUsed == false && m_hideCategory == true) {
        // in case hide category is on and the account or any of it's
        // subaccounts has no split, we can safely remove it and all
        // it's sub-ordinate accounts from the list
        delete accountItem;
      }
    }

    categoryListView->setOpen(incomeTopLevelAccount, true);
    categoryListView->setOpen(expenseTopLevelAccount, true);
  }
  catch (MyMoneyException *e)
  {
    qDebug("Exception in assets account refresh: %s", e->what().latin1());
    delete e;
  }

  // free some memory (we don't need this map anymore)
  m_accountMap.clear();
}

const bool KCategoriesView::showSubAccounts(const QCStringList& accounts, KAccountListItem *parentItem,
  const QString& typeName)
{
  bool accountUsed = false;

  for ( QCStringList::ConstIterator it = accounts.begin(); it != accounts.end(); ++it )
  {
    KAccountListItem *accountItem  = new KAccountListItem(parentItem,
                                                          m_accountMap[*it]);
    accountUsed = m_transactionCountMap[*it] > 0;

    QCStringList subAccounts = m_accountMap[*it].accountList();
    if (subAccounts.count() >= 1)
    {
      accountUsed |= showSubAccounts(subAccounts, accountItem, typeName);
    }
  }
  return accountUsed;
}

void KCategoriesView::show()
{
  emit signalViewActivated();
  QWidget::show();
}

void KCategoriesView::resizeEvent(QResizeEvent* e)
{
  // categoryListView->setColumnWidth(0, categoryListView->width()-185);
  // categoryListView->setColumnWidth(1, 100);
  // categoryListView->setColumnWidth(2, 70);

  // call base class resizeEvent()

  kCategoriesViewDecl::resizeEvent(e);
}

void KCategoriesView::slotNewClicked()
{
  MyMoneyAccount account;

  KNewAccountDlg dialog(account, false, true, this, "hi", i18n("Create A New Category"));

  if (dialog.exec())
  {
    try
    {
      MyMoneyFile* file = MyMoneyFile::instance();

      MyMoneyAccount newAccount = dialog.account();
      MyMoneyAccount parentAccount = dialog.parentAccount();
      file->addAccount(newAccount, parentAccount);
//      categoryListView->clear();
//      refresh();
    }
    catch (MyMoneyException *e)
    {
      QString message("Unable to add account: ");
      message += e->what();
      KMessageBox::information(this, message);
      delete e;
      return;
    }
  }

}

void KCategoriesView::slotDeleteClicked(MyMoneyAccount& account)
{
  QString prompt = i18n("Do you really want to delete the category '%1'")
    .arg(account.name());

  if ((KMessageBox::questionYesNo(this, prompt)) == KMessageBox::Yes) {
    try {
      MyMoneyFile *file = MyMoneyFile::instance();
      file->removeAccount(account);
    }
    catch (MyMoneyException *e)
    {
      QString message(i18n("Unable to remove category: "));
      message += e->what();
      KMessageBox::error(this, message);
      delete e;
    }
  }  
}

void KCategoriesView::slotDeleteClicked(void)
{
  KAccountListItem *item = (KAccountListItem *)categoryListView->selectedItem();
  if (!item)
    return;

  try
  {
    MyMoneyFile *file = MyMoneyFile::instance();
    MyMoneyAccount account = file->account(item->accountID());
    slotDeleteClicked(account);
  }
  catch (MyMoneyException *e)
  {
    QString message(i18n("Unable to remove category: "));
    message += e->what();
    KMessageBox::error(this, message);
    delete e;
  }
}

void KCategoriesView::slotSelectionChanged(QListViewItem* item)
{
  KAccountListItem *kitem = (KAccountListItem *)item;
  if (!kitem)
  {
    buttonEdit->setEnabled(false);
    buttonDelete->setEnabled(false);
  }
  else
  {
    buttonEdit->setEnabled(true);
    buttonDelete->setEnabled(true);
  }
}

void KCategoriesView::slotEditClicked(MyMoneyAccount& account)
{
  try {
    KNewAccountDlg dlg(account, true, true, this, "hi", i18n("Edit a category"));

    if (dlg.exec())
    {
      account = dlg.account();
      MyMoneyAccount parent = dlg.parentAccount();

      MyMoneyFile* file = MyMoneyFile::instance();
      
      // we need to reparent first, as modify will check for same type
      if(account.parentAccountId() != parent.id()) {
        file->reparentAccount(account, parent);
      }
      file->modifyAccount(account);
    }
  }
  catch (MyMoneyException *e)
  {
    QString errorString = i18n("Cannot edit category: ");
    errorString += e->what();
    KMessageBox::error(this, errorString);
    delete e;
  }
}

void KCategoriesView::slotEditClicked(void)
{
  KAccountListItem *item = (KAccountListItem *)categoryListView->selectedItem();
  if (!item)
    return;

  try
  {
    MyMoneyFile* file = MyMoneyFile::instance();
    MyMoneyAccount account = file->account(item->accountID());

    slotEditClicked(account);
  }
  catch (MyMoneyException *e)
  {
    QString errorString = i18n("Cannot edit category: ");
    errorString += e->what();
    KMessageBox::error(this, errorString);
    delete e;
  }
}

void KCategoriesView::readConfig(void)
{
  KConfig *config = KGlobal::config();
  config->setGroup("Last Use Settings");
  m_lastCat = config->readEntry("KCategoriesView_LastCategory");
}

void KCategoriesView::writeConfig(void)
{
  KAccountListItem *item = (KAccountListItem *)categoryListView->selectedItem();
  if (!item)
    return;

  KConfig *config = KGlobal::config();
  config->setGroup("Last Use Settings");
  config->writeEntry("KCategoriesView_LastCategory", item->text(0));
  config->sync();
}

void KCategoriesView::update(const QCString& /* id */)
{
  // to avoid constant update when a lot of accounts are added
  // (e.g. during creation of a new MyMoneyFile object when the
  // default accounts are loaded) a switch is supported to suppress
  // updates in this phase. The switch is controlled with suspendUpdate().
  if(m_suspendUpdate == false) {
    slotRefreshView();
  }
}

void KCategoriesView::suspendUpdate(const bool suspend)
{
  // force a refresh, if update was off
  if(m_suspendUpdate == true
  && suspend == false)
    slotRefreshView();

  m_suspendUpdate = suspend;
}

const QCString KCategoriesView::currentAccount(bool& success) const
{
  KAccountListItem *item = (KAccountListItem *)categoryListView->selectedItem();
  if (!item) {
    success = false;
    return QCString("");
  }

  success=true;
  return item->accountID();
}

void KCategoriesView::slotListRightMouse(QListViewItem* item, const QPoint& , int col)
{
  if (item != 0 && col != -1) {
    categoryListView->setSelected(item, true);
    emit categoryRightMouseClick();
  }
}
