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
#include "kbanksview.h"
#include "kbanklistitem.h"
#include <qheader.h>
#include <kglobal.h>
#include <klocale.h>
#include <qtooltip.h>
#include <klistview.h>
#include <kconfig.h>

#include "kmymoneyfile.h"

KAccountsView::KAccountsView(QWidget *parent, const char *name)
 : KBankViewDecl(parent,name)
{
  KConfig *config = KGlobal::config();
  config->setGroup("List Options");
  m_bViewNormalAccountsView = config->readBoolEntry("NormalAccountsView", true);

  accountListView->setRootIsDecorated(true);
  accountListView->setAllColumnsShowFocus(true);
//  if (m_bViewNormalAccountsView)
//    accountListView->addColumn(i18n("Institution"));
  accountListView->addColumn(i18n("Account"));
  accountListView->addColumn(i18n("Type"));
  accountListView->addColumn(i18n("Balance"));
  accountListView->setMultiSelection(false);
  accountListView->header()->setResizeEnabled(false);
  accountListView->setColumnWidthMode(0, QListView::Manual);

  QFont defaultFont = QFont("helvetica", 12);
  accountListView->header()->setFont(config->readFontEntry("listHeaderFont", &defaultFont));

  connect(accountListView, SIGNAL(selectionChanged(QListViewItem*)),
    this, SLOT(slotSelectionChanged(QListViewItem*)));
  connect(accountListView, SIGNAL(rightButtonClicked(QListViewItem* , const QPoint&, int)),
    this, SLOT(slotListRightMouse(QListViewItem*, const QPoint&, int)));
  connect(accountListView, SIGNAL(doubleClicked(QListViewItem*, const QPoint&, int)),
    this, SLOT(slotListDoubleClicked(QListViewItem*, const QPoint&, int)));

  m_bSelectedAccount=false;
  m_bSelectedInstitution=false;
  m_bSignals=true;

  // never show a horizontal scroll bar
  //accountListView->setHScrollBarMode(QScrollView::AlwaysOff);
}

KAccountsView::~KAccountsView()
{
}

void KAccountsView::slotListDoubleClicked(QListViewItem* pItem, const QPoint& pos, int c)
{
  KAccountListItem *accountItem = (KAccountListItem*)pItem;
  if(accountItem)
  {
    // Only emit the signal if its an account
    MyMoneyFile *file = KMyMoneyFile::instance()->file();

    try
    {
      MyMoneyAccount account = file->account(accountItem->accountID());
      m_bSelectedAccount=true;
      m_bSelectedInstitution=false;
      m_selectedAccount = accountItem->accountID();
      emit accountDoubleClick();
    }
    catch (MyMoneyException *e)
    {
      // Probably clicked on the institution in normal view
      delete e;
    }
  }
}

void KAccountsView::slotListRightMouse(QListViewItem* item, const QPoint& , int col)
{
  if (item==0 || col==-1)
  {
    emit rightMouseClick();
  }
  else
  {
    KAccountListItem *accountItem = (KAccountListItem*)item;
    if (accountItem)
    {
      try
      {
        MyMoneyFile *file = KMyMoneyFile::instance()->file();
        MyMoneyAccount account = file->account(accountItem->accountID());
        
        m_bSelectedAccount=true;
        m_bSelectedInstitution=false;
        m_selectedAccount = accountItem->accountID();
        qDebug("Setting selected acc to %s", accountItem->accountID().data());

        emit accountRightMouseClick();
      }
      catch (MyMoneyException *e)
      {
        m_bSelectedAccount=false;
        m_bSelectedInstitution=true;
        // FIXME: Change KAccountListItem::accountID to id.
        qDebug("Setting selected inst to %s", accountItem->accountID().data());
        m_selectedInstitution = accountItem->accountID();

        emit bankRightMouseClick();
      }
    }
  }
}

QCString KAccountsView::currentAccount(bool& success)
{
  success=m_bSelectedAccount;
  return (success)?m_selectedAccount:"";
}

QCString KAccountsView::currentInstitution(bool& success)
{
  success=m_bSelectedInstitution;
  return (success)?m_selectedInstitution:"";
}

void KAccountsView::refresh(const QCString& selectAccount)
{
  KConfig *config = KGlobal::config();
  config->setGroup("List Options");
  QFont defaultFont = QFont("helvetica", 12);
  accountListView->header()->setFont(config->readFontEntry("listHeaderFont", &defaultFont));
  m_bViewNormalAccountsView = config->readBoolEntry("NormalAccountsView", true);

  clear();

  MyMoneyMoney totalProfit;

  m_selectedAccount = selectAccount;

  MyMoneyFile *file = KMyMoneyFile::instance()->file();

  MyMoneyAccount liabilityAccount = file->liability();
  MyMoneyAccount assetAccount = file->asset();

  if (m_bViewNormalAccountsView)
  {
    try
    {
      QValueList<MyMoneyInstitution> list = file->institutionList();
      QValueList<MyMoneyInstitution>::ConstIterator institutionIterator;
      for (institutionIterator = list.begin(); institutionIterator != list.end(); ++institutionIterator)
      {
        KAccountListItem *topLevelInstitution = new KAccountListItem(accountListView,
                      (*institutionIterator).name(), (*institutionIterator).id());

        QCStringList accountList = (*institutionIterator).accountList();
        for ( QCStringList::ConstIterator it = accountList.begin();
              it != accountList.end();
              ++it )
        {
          KAccountListItem *accountItem = new KAccountListItem(topLevelInstitution,
              file->account(*it).name(), file->account(*it).id(), "");

          QCStringList subAccounts = file->account(*it).accountList();
          if (subAccounts.count() >= 1)
          {
            showSubAccounts(subAccounts, accountItem, file);
          }
        }

      }
    }
    catch (MyMoneyException *e)
    {
      qDebug("Exception in assets account refresh (normal view): %s", e->what().latin1());
      delete e;
    }
  }
  else  // Show new 'advanced' view
  {
      MyMoneyAccount expenseAccount = file->expense();
      MyMoneyAccount incomeAccount = file->income();

      // Do all 4 account roots
      try
      {
        // Asset
        KAccountListItem *assetTopLevelAccount = new KAccountListItem(accountListView,
                          assetAccount.name(), assetAccount.id());

        for ( QCStringList::ConstIterator it = file->asset().accountList().begin();
              it != file->asset().accountList().end();
              ++it )
        {
          KAccountListItem *accountItem = new KAccountListItem(assetTopLevelAccount,
              file->account(*it).name(), file->account(*it).id(), "");

          QCStringList subAccounts = file->account(*it).accountList();
          if (subAccounts.count() >= 1)
          {
            showSubAccounts(subAccounts, accountItem, file);
          }
        }

        // Liability
        KAccountListItem *liabilityTopLevelAccount = new KAccountListItem(accountListView,
                          liabilityAccount.name(), liabilityAccount.id());

        for ( QCStringList::ConstIterator it = file->liability().accountList().begin();
              it != file->liability().accountList().end();
              ++it )
        {
          KAccountListItem *accountItem = new KAccountListItem(liabilityTopLevelAccount,
              file->account(*it).name(), file->account(*it).id(), "");

          QCStringList subAccounts = file->account(*it).accountList();
          if (subAccounts.count() >= 1)
          {
            showSubAccounts(subAccounts, accountItem, file);
          }
        }

        // Income
        KAccountListItem *incomeTopLevelAccount = new KAccountListItem(accountListView,
                          incomeAccount.name(), incomeAccount.id());

        for ( QCStringList::ConstIterator it = file->income().accountList().begin();
              it != file->income().accountList().end();
              ++it )
        {
          KAccountListItem *accountItem = new KAccountListItem(incomeTopLevelAccount,
              file->account(*it).name(), file->account(*it).id(), "");

          QCStringList subAccounts = file->account(*it).accountList();
          if (subAccounts.count() >= 1)
          {
            showSubAccounts(subAccounts, accountItem, file);
          }
        }

        // Expense
        KAccountListItem *expenseTopLevelAccount = new KAccountListItem(accountListView,
                          expenseAccount.name(), expenseAccount.id());

        for ( QCStringList::ConstIterator it = file->expense().accountList().begin();
              it != file->expense().accountList().end();
              ++it )
        {
          KAccountListItem *accountItem = new KAccountListItem(expenseTopLevelAccount,
              file->account(*it).name(), file->account(*it).id(), "");

          QCStringList subAccounts = file->account(*it).accountList();
          if (subAccounts.count() >= 1)
          {
            showSubAccounts(subAccounts, accountItem, file);
          }
        }


      }
      catch (MyMoneyException *e)
      {
        qDebug("Exception in assets account refresh: %s", e->what().latin1());
        delete e;
      }
  }

/*
  QValueList<MyMoneyAccount> accountList = m->accountList();
  QValueList<MyMoneyAccount>::ConstIterator it;
  for (it = list.begin(); it != list.end(); ++it)
  {
    KAccountListItem *item0 = new KAccountListItem(accountListView, (*it));

    if((*it) == m_selectedAccount)
    {
      m_bSelectedAccount = true;
      item = item0;
      }
      totalProfit += account->balance();
    }
    accountListView->setOpen(item0, true);
  }

  QString s(i18n("Total Profits: "));
  s += KGlobal::locale()->formatMoney(totalProfit.amount(), "",
                                      KGlobal::locale()->fracDigits());

  totalProfitsLabel->setFont(config->readFontEntry("listCellFont", &defaultFont));
  totalProfitsLabel->setText(s);

  if (m_bSelectedBank || m_bSelectedAccount)
    accountListView->setSelected(item, true);
*/
}

void KAccountsView::showSubAccounts(QCStringList accounts, KAccountListItem *parentItem, MyMoneyFile *file)
{
  for ( QCStringList::ConstIterator it = accounts.begin(); it != accounts.end(); ++it )
  {
    KAccountListItem *accountItem  = new KAccountListItem(parentItem,
          file->account(*it).name(), file->account(*it).id(), "");

    QCStringList subAccounts = file->account(*it).accountList();
    if (subAccounts.count() >= 1)
    {
      showSubAccounts(subAccounts, accountItem, file);
    }
  }
}

void KAccountsView::clear(void)
{
  accountListView->clear();
  m_bSelectedAccount = false;
  m_bSelectedInstitution=false;
}

void KAccountsView::resizeEvent(QResizeEvent* e)
{
  accountListView->setColumnWidth(0, 400);
  accountListView->setColumnWidth(1,150);
  int totalWidth=accountListView->width();
  accountListView->setColumnWidth(2, totalWidth-550-5);

  // call base class resizeEvent()
  //KBankViewDecl::resizeEvent(e);
}

void KAccountsView::slotSelectionChanged(QListViewItem *item)
{
  KAccountListItem *accountItem = (KAccountListItem*)item;
  if (accountItem)
  {
    MyMoneyFile *file = KMyMoneyFile::instance()->file();

    try
    {
      MyMoneyAccount account = file->account(accountItem->accountID());
      m_bSelectedAccount=true;
      m_selectedAccount = accountItem->accountID();
      qDebug("2: Setting acc to %s", accountItem->accountID().data());
      //emit accountSelected();
    }
    catch (MyMoneyException *e)
    {
      // Probably clicked on the institution in normal view
      m_bSelectedAccount=false;
      m_bSelectedInstitution=true;
      // FIXME: Change KAccountListItem::accountID to id.
      qDebug("2: Setting inst to %s", accountItem->accountID().data());
      m_selectedInstitution = accountItem->accountID();

      delete e;
    }
  }
}

void KAccountsView::show()
{
  if (m_bSignals)
    emit signalViewActivated();
  QWidget::show();
}

void KAccountsView::setSignals(bool enable)
{
  m_bSignals=enable;
}
