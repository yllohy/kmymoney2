/***************************************************************************
                          KAccountsView.cpp
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

#include <qheader.h>
#include <qtooltip.h>
#include <qiconview.h>
#include <qpixmap.h>
#include <qtabwidget.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kglobal.h>
#include <klocale.h>
#include <klistview.h>
#include <kconfig.h>
#include <kmessagebox.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kstandarddirs.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kbanksview.h"
#include "kbanklistitem.h"
#include "../widgets/kmymoneyaccounttree.h"
#include "../widgets/kmymoneytitlelabel.h"
#include "../mymoney/mymoneyfile.h"
#include "../dialogs/knewaccountdlg.h"
#include "../dialogs/knewbankdlg.h"
#include "../kmymoneyutils.h"

KAccountsView::KAccountsView(QWidget *parent, const char *name, bool bInstitutionView)
 : KBankViewDecl(parent,name),
   m_suspendUpdate(false),
   m_bViewNormalAccountsView(bInstitutionView)
{
  titleLabel->setText(bInstitutionView ? i18n("Institutions") : i18n("Accounts"));

  accountListView->setRootIsDecorated(true);
  accountListView->setAllColumnsShowFocus(true);

  accountListView->addColumn(i18n("Account"));
  accountListView->addColumn(i18n("Transactions"));
  accountListView->addColumn(i18n("Balance"));
  accountListView->addColumn(i18n("Value"));

  accountListView->setMultiSelection(false);

  accountListView->setColumnWidthMode(0, QListView::Maximum);
  accountListView->setColumnWidthMode(1, QListView::Maximum);
  accountListView->setColumnWidthMode(2, QListView::Maximum);
  accountListView->setColumnWidthMode(3, QListView::Maximum);

  accountListView->setColumnAlignment(1, Qt::AlignRight);
  accountListView->setColumnAlignment(2, Qt::AlignRight);
  accountListView->setColumnAlignment(3, Qt::AlignRight);

  accountListView->setResizeMode(QListView::AllColumns);

  accountListView->header()->setResizeEnabled(true);

  accountListView->header()->setFont(KMyMoneyUtils::headerFont());

  if(!m_bViewNormalAccountsView) {
    // select the type the user viewed last (list or icon) but only
    // for the accounts view
    KConfig *config = KGlobal::config();
    config->setGroup("Last Use Settings");
    accountTabWidget->setCurrentPage(config->readNumEntry("KAccountsView_LastType", 0));
  } else
    accountTabWidget->setCurrentPage(0);

  connect(accountListView, SIGNAL(selectionChanged(QListViewItem*)),
    this, SLOT(slotSelectionChanged(QListViewItem*)));

  // somehow, the rightButtonClicked signal does not make it, we use
  // rightButtonPressed instead to show the context menu
  // connect(accountListView, SIGNAL(rightButtonClicked(QListViewItem* , const QPoint&, int)),
  //   this, SLOT(slotListRightMouse(QListViewItem*, const QPoint&, int)));
  connect(accountListView, SIGNAL(rightButtonPressed(QListViewItem* , const QPoint&, int)),
    this, SLOT(slotListRightMouse(QListViewItem*, const QPoint&, int)));
  connect(accountIconView, SIGNAL(rightButtonPressed(QIconViewItem*, const QPoint&)),
    this, SLOT(slotIconRightMouse(QIconViewItem*, const QPoint&)));

  connect(accountListView, SIGNAL(doubleClicked(QListViewItem*, const QPoint&, int)),
    this, SLOT(slotListDoubleClicked(QListViewItem*, const QPoint&, int)));
  connect(accountIconView, SIGNAL(doubleClicked(QIconViewItem*)),
    this, SLOT(slotIconDoubleClicked(QIconViewItem*)));

  connect(accountTabWidget, SIGNAL(currentChanged(QWidget*)),
    this, SLOT(slotViewSelected(QWidget*)));

  m_bSelectedAccount=false;
  m_bSelectedInstitution=false;
  // m_bSignals=true;

  accountIconView->clear();
  accountIconView->setSorting(-1);
  accountListView->setSorting(0);

  // don't enable the icon tab in institutions mode
  if(m_bViewNormalAccountsView) {
    accountTabWidget->removePage(accountTabWidget->page(1));
  }

  // never show a horizontal scroll bar
  //accountListView->setHScrollBarMode(QScrollView::AlwaysOff);

  MyMoneyFile::instance()->attach(MyMoneyFile::NotifyClassAccountHierarchy, this);
  MyMoneyFile::instance()->attach(MyMoneyFile::NotifyClassAccount, this);
  MyMoneyFile::instance()->attach(MyMoneyFile::NotifyClassInstitution, this);
  MyMoneyFile::instance()->attach(MyMoneyFile::NotifyClassSecurity, this);
  MyMoneyFile::instance()->attach(MyMoneyFile::NotifyClassCurrency, this);

  refresh(QCString());
}

KAccountsView::~KAccountsView()
{
  MyMoneyFile::instance()->detach(MyMoneyFile::NotifyClassAccountHierarchy, this);
  MyMoneyFile::instance()->detach(MyMoneyFile::NotifyClassAccount, this);
  MyMoneyFile::instance()->detach(MyMoneyFile::NotifyClassInstitution, this);
  MyMoneyFile::instance()->detach(MyMoneyFile::NotifyClassSecurity, this);
  MyMoneyFile::instance()->detach(MyMoneyFile::NotifyClassCurrency, this);
}

void KAccountsView::slotListDoubleClicked(QListViewItem* item, const QPoint& /* pos */, int /* c */)
{
  KAccountListItem* accountItem = static_cast<KAccountListItem*> (item);
  if(accountItem)
  {
    if ( item->childCount() )
      item->setOpen( ! item->isOpen() );
    else
    {

      MyMoneyFile *file = MyMoneyFile::instance();
      try
      {
        MyMoneyAccount account = file->account(accountItem->accountID());

        if(!file->isStandardAccount(account.id())) {
          switch(file->accountGroup(account.accountType())) {
            // the account signal will only be emitted for asset and liability accounts
            case MyMoneyAccount::Asset:
            case MyMoneyAccount::Liability:
              m_bSelectedAccount=true;
              m_bSelectedInstitution=false;
              m_selectedAccount = accountItem->accountID();
              emit accountDoubleClick();
              break;
            // the category signal will only be emitted for income & expense accounts
            case MyMoneyAccount::Income:
            case MyMoneyAccount::Expense:
              m_bSelectedAccount=true;
              m_bSelectedInstitution=false;
              m_selectedAccount = accountItem->accountID();
              emit categoryDoubleClick();
              break;

            default:
              break;
          }
        }
      }
      catch (MyMoneyException *e)
      {
        // Probably clicked on the institution in normal view
        delete e;
      }
    }
  }
}

void KAccountsView::slotIconDoubleClicked(QIconViewItem* item)
{

  KAccountIconItem *accountItem = static_cast<KAccountIconItem*> (item);

  if(accountItem) {
    MyMoneyFile *file = MyMoneyFile::instance();

    try {
      MyMoneyAccount account = file->account(accountItem->accountID());
      m_bSelectedAccount=true;
      m_bSelectedInstitution=false;
      m_selectedAccount = accountItem->accountID();
      emit accountDoubleClick();
    } catch (MyMoneyException *e) {
      delete e;
    }
  }
}

void KAccountsView::slotIconRightMouse(QIconViewItem* item, const QPoint&)
{
  KAccountIconItem *accountItem = static_cast<KAccountIconItem*> (item);
  if (accountItem) {
    try {
      m_bSelectedAccount=true;
      m_bSelectedInstitution=false;
      m_selectedAccount = accountItem->accountID();

      emit accountRightMouseClick();

    } catch (MyMoneyException *e) {
      delete e;
    }
  }
}

void KAccountsView::slotListRightMouse(QListViewItem* item, const QPoint& , int col)
{
  if (item==0 || col==-1) {
    emit rightMouseClick();

  } else {
    KAccountListItem* accountItem = static_cast<KAccountListItem*> (item);



    if (accountItem) {
      try {
        MyMoneyFile *file = MyMoneyFile::instance();
        MyMoneyAccount account = file->account(accountItem->accountID());


        m_bSelectedAccount=true;
        m_bSelectedInstitution=false;
        m_selectedAccount = accountItem->accountID();

        emit accountRightMouseClick();

      } catch (MyMoneyException *e) {
        m_bSelectedAccount=false;
        m_bSelectedInstitution=true;
        m_selectedInstitution = accountItem->accountID();
        delete e;

        emit bankRightMouseClick();
      }
    }
  }
}

QCString KAccountsView::currentAccount(bool& success)
{
  success=m_bSelectedAccount;
  return (success) ? m_selectedAccount : QCString();
}

QCString KAccountsView::currentInstitution(bool& success)
{
  success=m_bSelectedInstitution;
  return (success) ? m_selectedInstitution : QCString();
}

void KAccountsView::update(const QCString& id)
{
  // to avoid constant update when a lot of accounts are added
  // (e.g. during creation of a new MyMoneyFile object when the
  // default accounts are loaded) a switch is supported to suppress
  // updates in this phase. The switch is controlled with suspendUpdate().
  if(m_suspendUpdate == false) {
    if(id == MyMoneyFile::NotifyClassAccountHierarchy
    || id == MyMoneyFile::NotifyClassCurrency
    || id == MyMoneyFile::NotifyClassSecurity
    || (id == MyMoneyFile::NotifyClassInstitution && m_bViewNormalAccountsView == true)) {
      refresh(id);
    }
    if(id == MyMoneyFile::NotifyClassAccount
    || id == MyMoneyFile::NotifyClassCurrency
    || id == MyMoneyFile::NotifyClassSecurity)
      refreshNetWorth();
  }
}

void KAccountsView::suspendUpdate(const bool suspend)
{
  KAccountListItem* item = static_cast<KAccountListItem *>(accountListView->firstChild());

  // inform all children
  while(item) {
    item->suspendUpdate(suspend);
    item = static_cast<KAccountListItem *>(item->itemBelow());
  }

  // force a refresh, if update was off
  if(m_suspendUpdate == true
  && suspend == false) {
    refresh(MyMoneyFile::NotifyClassAccountHierarchy);
    if(m_bViewNormalAccountsView == true)
      refresh(MyMoneyFile::NotifyClassInstitution);
    refreshNetWorth();
  }

  m_suspendUpdate = suspend;
}


void KAccountsView::slotRefreshView(void)
{

  refresh(m_selectedAccount);
}

void KAccountsView::refreshNetWorth(void)
{
  MyMoneyMoney netWorth;
  MyMoneyFile* file = MyMoneyFile::instance();

  MyMoneyAccount liabilityAccount = file->liability();
  MyMoneyAccount assetAccount = file->asset();

  netWorth = file->totalValue(assetAccount.id()) +
             file->totalValue(liabilityAccount.id());

  QString s(i18n("Net Worth: "));
  if(!(file->totalValueValid(assetAccount.id()) & file->totalValueValid(liabilityAccount.id())))
    s += "~ ";
  s.replace(QString(" "), QString("&nbsp;"));
  if(netWorth.isNegative()) {
    s += "<b><font color=\"red\">";
  }
  QString v(netWorth.formatMoney(file->baseCurrency().tradingSymbol()));
  s += v.replace(QString(" "), QString("&nbsp;"));
  if(netWorth.isNegative()) {
    s += "</font></b>";
  }

  totalProfitsLabel->setFont(KMyMoneyUtils::cellFont());
  totalProfitsLabel->setText(s);
}




const QPixmap KAccountsView::accountImage(const MyMoneyAccount::accountTypeE type) const
{
  QPixmap rc;
  switch(type) {
    default:
      if(MyMoneyFile::instance()->accountGroup(type) == MyMoneyAccount::Asset)
        rc = DesktopIcon("account-types_asset");
      else
        rc = DesktopIcon("account-types_liability");
      break;

    case MyMoneyAccount::Investment:
      rc = DesktopIcon("account-types_investments");
      break;

    case MyMoneyAccount::Checkings:
      rc = DesktopIcon("account-types_checking");
      break;
    case MyMoneyAccount::Savings:
      rc = DesktopIcon("account-types_savings");
      break;

    case MyMoneyAccount::AssetLoan:
    case MyMoneyAccount::Loan:
      rc = DesktopIcon("account-types_loan");
      break;

    case MyMoneyAccount::CreditCard:
      rc = DesktopIcon("account-types_credit-card");
      break;

    case MyMoneyAccount::Asset:
      rc = DesktopIcon("account-types_asset");
      break;

    case MyMoneyAccount::Cash:
      rc = DesktopIcon("account-types_cash");
      break;
  }

  return rc;
}

/*
void KAccountsView::fillTransactionCountMap(void)
{
  QValueList<MyMoneyTransaction> list;
  QValueList<MyMoneyTransaction>::ConstIterator it_t;
  QValueList<MyMoneySplit>::ConstIterator it_s;

  list = MyMoneyFile::instance()->transactionList();
  m_transactionCountMap.clear();

  // scan all transactions
  for(it_t = list.begin(); it_t != list.end(); ++it_t) {
    // scan all splits of this transaction
    for(it_s = (*it_t).splits().begin(); it_s != (*it_t).splits().end(); ++it_s) {
      m_transactionCountMap[(*it_s).accountId()]++;
    }

  }
}
*/

void KAccountsView::fillAccountMap(void)
{
  QValueList<MyMoneyAccount> accountList;
  accountList = MyMoneyFile::instance()->accountList();


  m_accountMap.clear();

  QValueList<MyMoneyAccount>::ConstIterator it_a;
  for(it_a = accountList.begin(); it_a != accountList.end(); ++it_a)
    m_accountMap[(*it_a).id()] = *it_a;
}


void KAccountsView::refresh(const QCString& selectAccount)
{
  accountListView->header()->setFont(KMyMoneyUtils::headerFont());

  KConfig *config = KGlobal::config();
  config->setGroup("List Options");
  m_hideCategory = config->readBoolEntry("HideUnusedCategory", false);
  bool accountUsed;

  // Disable the note about hidden categories
  m_hiddenCategories->hide();

  clear();

  m_selectedAccount = selectAccount;

  fillAccountMap();
  m_transactionCountMap = MyMoneyFile::instance()->transactionCountMap();

  MyMoneyFile *file = MyMoneyFile::instance();

  if (m_bViewNormalAccountsView) {
    accountListView->header()->setLabel(0, i18n("Institution"));

    try {
      // scan for all asset/liability accounts w/o an institution
      KAccountListItem *topLevelInstitution = new KAccountListItem(accountListView,
                       i18n("Accounts with no institution assigned"));
      topLevelInstitution->setPixmap(0, QPixmap(KGlobal::dirs()->findResource("appdata",QString( "icons/hicolor/22x22/actions/%1.png").arg("bank"))));
      QValueList<MyMoneyAccount> acclist = file->accountList();
      QValueList<MyMoneyAccount>::ConstIterator accountIterator;
      MyMoneyMoney balance;
      bool balanceValid = true;

      for(accountIterator = acclist.begin();
          accountIterator != acclist.end();
          ++accountIterator) {

        // Don't show stock accounts, they will be handled with the
        // investment accounts further down
        if((*accountIterator).accountType() == MyMoneyAccount::Stock) {
          continue;
        }

        switch((*accountIterator).accountGroup()) {
          case MyMoneyAccount::Asset:
          case MyMoneyAccount::Liability:
            if((*accountIterator).institutionId().isEmpty()) {
              KAccountListItem *accountItem;
              accountItem = new KAccountListItem(topLevelInstitution, (*accountIterator));
              accountItem->setText(1, QString("%1").arg(m_transactionCountMap[(*accountIterator).id()]));
              // new KAccountIconItem(accountIconView, (*accountIterator),
              //   accountImage((*accountIterator).accountType()));
              if((*accountIterator).accountType() == MyMoneyAccount::Investment) {
                balance += file->totalValue((*accountIterator).id());
                if(!file->totalValueValid((*accountIterator).id()))
                  balanceValid = false;
              } else {
                balance += file->accountValue((*accountIterator).id());
                if(!file->accountValueValid((*accountIterator).id()))
                  balanceValid = false;
              }
            }
            break;
          default:
            break;
        }
      }

      topLevelInstitution->setValue(balance, balanceValid);

      // in case there is no account w/o reference to an institution
      // we can safely remove this entry to avoid user's confusion
      if(topLevelInstitution->childCount() == 0) {
        delete topLevelInstitution;
      }

      QValueList<MyMoneyInstitution> list = file->institutionList();
      QValueList<MyMoneyInstitution>::ConstIterator institutionIterator;
      for (institutionIterator = list.begin();

            institutionIterator != list.end();
            ++institutionIterator) {

        MyMoneyMoney balance;
        bool balanceValid = true;
        KAccountListItem *topLevelInstitution = new KAccountListItem(accountListView,
                      *institutionIterator);

        QCStringList accountList = (*institutionIterator).accountList();
        balance = MyMoneyMoney(0, 1);
        for ( QCStringList::ConstIterator it = accountList.begin();
              it != accountList.end();
              ++it ) {
          // don't show stock accounts
          if(m_accountMap[*it].accountType() == MyMoneyAccount::Stock)
            continue;

          KAccountListItem *accountItem = new KAccountListItem(topLevelInstitution,
              m_accountMap[*it]);
          accountItem->setText(1, QString("%1").arg(m_transactionCountMap[*it]));

          if(m_accountMap[*it].accountType() == MyMoneyAccount::Investment) {
            balance += file->totalValue(*it);
            if(!file->totalValueValid(*it))
              balanceValid = false;
          } else {
            balance += file->accountValue(*it);
            if(!file->accountValueValid(*it))
              balanceValid = false;
          }
        }
        topLevelInstitution->setValue(balance, balanceValid);
      }

    } catch (MyMoneyException *e) {
      qDebug("Exception in assets account refresh (normal view): %s", e->what().latin1());
      delete e;
    }

  } else {       // Show new 'advanced' view
    accountListView->header()->setLabel(0, i18n("Account"));
    // Do all 5 account roots
    try {
      // Asset
      MyMoneyAccount assetAccount = file->asset();
      KAccountListItem *assetTopLevelAccount = new KAccountListItem(accountListView,
            assetAccount);

      for ( QCStringList::ConstIterator it = assetAccount.accountList().begin();
            it != assetAccount.accountList().end();
            ++it ) {

        // don't show stock accounts in non-expert mode
        if((m_accountMap[*it].accountType() == MyMoneyAccount::Stock)
        && !KMyMoneyUtils::isExpertMode())
          continue;

        KAccountListItem *accountItem = new KAccountListItem(assetTopLevelAccount,
            m_accountMap[*it]);
        accountItem->setText(1, QString("%1").arg(m_transactionCountMap[*it]));

        new KAccountIconItem(accountIconView,
            m_accountMap[*it],
            accountImage(m_accountMap[*it].accountType()));

        QCStringList subAccounts = m_accountMap[*it].accountList();
        if (subAccounts.count() >= 1) {
          showSubAccounts(subAccounts, accountItem, true);
        }
      }

      // Liability
      MyMoneyAccount liabilityAccount = file->liability();
      KAccountListItem *liabilityTopLevelAccount = new KAccountListItem(accountListView,
            liabilityAccount);

      for ( QCStringList::ConstIterator it = liabilityAccount.accountList().begin();
            it != liabilityAccount.accountList().end();
            ++it ) {

        // don't show stock accounts in non-expert mode
        if((m_accountMap[*it].accountType() == MyMoneyAccount::Stock)
        && !KMyMoneyUtils::isExpertMode())
          continue;

        KAccountListItem *accountItem = new KAccountListItem(liabilityTopLevelAccount,
            m_accountMap[*it]);
        accountItem->setText(1, QString("%1").arg(m_transactionCountMap[*it]));

        new KAccountIconItem(accountIconView,

            m_accountMap[*it],
            accountImage(m_accountMap[*it].accountType()));

        QCStringList subAccounts = m_accountMap[*it].accountList();
        if (subAccounts.count() >= 1) {
          showSubAccounts(subAccounts, accountItem, true);
        }
      }

      // Income
      MyMoneyAccount incomeAccount = file->income();
      KAccountListItem *incomeTopLevelAccount = new KAccountListItem(accountListView,
            incomeAccount);


      for ( QCStringList::ConstIterator it = incomeAccount.accountList().begin();
            it != incomeAccount.accountList().end();
            ++it ) {
        KAccountListItem *accountItem = new KAccountListItem(incomeTopLevelAccount,
            m_accountMap[*it]);
        accountItem->setText(1, QString("%1").arg(m_transactionCountMap[*it]));

        accountUsed = m_transactionCountMap[*it] > 0;

        QCStringList subAccounts = m_accountMap[*it].accountList();
        if (subAccounts.count() >= 1) {
          accountUsed |= showSubAccounts(subAccounts, accountItem, false);
        }
        if(accountUsed == false && m_hideCategory == true) {
          // in case hide category is on and the account or any of it's
          // subaccounts has no split, we can safely remove it and all
          // it's sub-ordinate accounts from the list
          delete accountItem;
          m_hiddenCategories->show();
        }
      }

      // Expense
      MyMoneyAccount expenseAccount = file->expense();
      KAccountListItem *expenseTopLevelAccount = new KAccountListItem(accountListView,
            expenseAccount);

      for ( QCStringList::ConstIterator it = expenseAccount.accountList().begin();
            it != expenseAccount.accountList().end();
            ++it ) {
        KAccountListItem *accountItem = new KAccountListItem(expenseTopLevelAccount,
            m_accountMap[*it]);
        accountItem->setText(1, QString("%1").arg(m_transactionCountMap[*it]));

        accountUsed = m_transactionCountMap[*it] > 0;

        QCStringList subAccounts = m_accountMap[*it].accountList();

        if (subAccounts.count() >= 1) {
          accountUsed |= showSubAccounts(subAccounts, accountItem, false);
        }
        if(accountUsed == false && m_hideCategory == true) {
          // in case hide category is on and the account or any of it's
          // subaccounts has no split, we can safely remove it and all
          // it's sub-ordinate accounts from the list
          delete accountItem;
          m_hiddenCategories->show();
        }
      }

      // Equity
      if(KMyMoneyUtils::isExpertMode()) {
        MyMoneyAccount equityAccount = file->equity();
        KAccountListItem *equityTopLevelAccount = new KAccountListItem(accountListView,
              equityAccount);

        for ( QCStringList::ConstIterator it = equityAccount.accountList().begin();
              it != equityAccount.accountList().end();
              ++it ) {
          KAccountListItem *accountItem = new KAccountListItem(equityTopLevelAccount,
              m_accountMap[*it]);
          accountItem->setText(1, QString("%1").arg(m_transactionCountMap[*it]));

          accountUsed = m_transactionCountMap[*it] > 0;

          QCStringList subAccounts = m_accountMap[*it].accountList();

          if (subAccounts.count() >= 1) {
            accountUsed |= showSubAccounts(subAccounts, accountItem, false);
          }
        }
      }

    } catch (MyMoneyException *e) {
      qDebug("Exception in assets account refresh: %s", e->what().latin1());
      delete e;
    }
  }

  m_accountMap.clear();
  m_transactionCountMap.clear();

  refreshNetWorth();

/*

  if (m_bSelectedBank || m_bSelectedAccount)
    accountListView->setSelected(item, true);
*/
}

const bool KAccountsView::showSubAccounts(const QCStringList& accounts, KAccountListItem *parentItem, const bool used)
{
  bool  accountUsed = used;

  for ( QCStringList::ConstIterator it = accounts.begin(); it != accounts.end(); ++it ) {

    // don't show stock accounts in non-expert mode
    if((m_accountMap[*it].accountType() == MyMoneyAccount::Stock)
    && !KMyMoneyUtils::isExpertMode())
      continue;

    KAccountListItem *accountItem  = new KAccountListItem(parentItem,
          m_accountMap[*it]);

    accountItem->setText(1, QString("%1").arg(m_transactionCountMap[*it]));

    accountUsed |= (m_transactionCountMap[*it] > 0);

    QCStringList subAccounts = m_accountMap[*it].accountList();
    if (subAccounts.count() >= 1) {
      accountUsed |= showSubAccounts(subAccounts, accountItem, used) || m_transactionCountMap[*it] > 0;
    }

    if(accountUsed == false && m_hideCategory == true) {
      // in case hide category is on and the account or any of it's
      // subaccounts has no split, we can safely remove it and all
      // it's sub-ordinate accounts from the list
      delete accountItem;
    }
  }
  return accountUsed;
}

void KAccountsView::clear(void)
{
  accountListView->clear();
  accountIconView->clear();


  m_bSelectedAccount = false;
  m_bSelectedInstitution=false;
}

void KAccountsView::resizeEvent(QResizeEvent* e)
{
  // call base class resizeEvent()
  KBankViewDecl::resizeEvent(e);
}

void KAccountsView::slotSelectionChanged(QListViewItem *item)
{

  KAccountListItem *accountItem = (KAccountListItem*)item;
  if (accountItem)
  {
    MyMoneyFile *file = MyMoneyFile::instance();

    try
    {
      MyMoneyAccount account = file->account(accountItem->accountID());
      m_bSelectedAccount=true;
      m_selectedAccount = accountItem->accountID();
      //emit accountSelected();
    }
    catch (MyMoneyException *e)
    {
      // Probably clicked on the institution in normal view
      m_bSelectedAccount=false;
      m_bSelectedInstitution=true;
      // FIXME: Change KAccountListItem::accountID to id.
      m_selectedInstitution = accountItem->accountID();

      delete e;
    }
  }
}

void KAccountsView::show()
{

  emit signalViewActivated();

  QWidget::show();
}

void KAccountsView::slotViewSelected(QWidget* /* view */)
{
  KConfig *config = KGlobal::config();
  config->setGroup("Last Use Settings");
  config->writeEntry("KAccountsView_LastType", accountTabWidget->currentPageIndex());
  config->sync();
}

void KAccountsView::slotEditClicked(void)

{
  QCString  accountId;

  if(accountTabWidget->currentPageIndex() == 0) {   // ListView?
    KAccountListItem *item = (KAccountListItem *)accountListView->currentItem();
    if(!item)
      return;
    accountId = item->accountID();

  } else {
    KAccountIconItem *item = (KAccountIconItem *)accountIconView->currentItem();
    if(!item)
      return;
    accountId = item->accountID();
  }

  try
  {
    MyMoneyFile* file = MyMoneyFile::instance();
    MyMoneyAccount account = file->account(accountId);

    KNewAccountDlg dlg(account, true, false, this, "hi", i18n("Edit an account"));

    if (dlg.exec())
    {
      MyMoneyAccount account = dlg.account();
      MyMoneyAccount parent = dlg.parentAccount();

      file->modifyAccount(account);
      if(account.parentAccountId() != parent.id()) {
        file->reparentAccount(account, parent);
      }
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

void KAccountsView::slotDeleteClicked(void)
{
  QCString  accountId;

  if(accountTabWidget->currentPageIndex() == 0) {   // ListView?
    KAccountListItem *item = (KAccountListItem *)accountListView->currentItem();
    if(!item)
      return;
    accountId = item->accountID();

  } else {
    KAccountIconItem *item = (KAccountIconItem *)accountIconView->currentItem();
    if(!item)
      return;
    accountId = item->accountID();
  }

  try
  {
    MyMoneyFile *file = MyMoneyFile::instance();
    MyMoneyAccount account = file->account(accountId);
    QString prompt = i18n("Do you really want to delete the account '%1'")
      .arg(account.name());

    if ((KMessageBox::questionYesNo(this, prompt)) == KMessageBox::Yes) {
      MyMoneyFile *file = MyMoneyFile::instance();
      file->removeAccount(account);
    }
  }
  catch (MyMoneyException *e)
  {
    QString message(i18n("Unable to remove account: "));
    message += e->what();
    KMessageBox::error(this, message);
    delete e;
  }
}

