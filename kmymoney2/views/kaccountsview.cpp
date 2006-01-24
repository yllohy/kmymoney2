/***************************************************************************
                          kaccountsview.cpp
                             -------------------
    copyright            : (C) 2005 by Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
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
#include <qtabwidget.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kdebug.h>
#include <klocale.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/mymoneyfile.h>
#include "kaccountsview.h"
#include "../kmymoneysettings.h"
#include "../kmymoney2.h"

KAccountsView::KAccountsView(QWidget *parent, const char *name) :
  KAccountsViewDecl(parent,name),
  m_assetItem(0),
  m_liabilityItem(0),
  m_needReload(false)
{
  // FIXME the code for the icon view is not present yet
  m_tab->setCurrentPage(0);
  m_tab->setTabEnabled(m_tab->page(1), false);

  connect(m_accountTree, SIGNAL(selectObject(const MyMoneyObject&)), this, SIGNAL(selectObject(const MyMoneyObject&)));
  connect(m_accountTree, SIGNAL(openContextMenu(const MyMoneyObject&)), this, SIGNAL(openContextMenu(const MyMoneyObject&)));
  connect(m_accountTree, SIGNAL(valueChanged(void)), this, SLOT(slotUpdateNetWorth(void)));
  connect(m_accountTree, SIGNAL(openObject(const MyMoneyObject&)), this, SIGNAL(openObject(const MyMoneyObject&)));
  connect(m_accountTree, SIGNAL(reparent(const MyMoneyAccount&, const MyMoneyAccount&)), this, SIGNAL(reparent(const MyMoneyAccount&, const MyMoneyAccount&)));

  connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotLoadAccounts()));
}

KAccountsView::~KAccountsView()
{
}

void KAccountsView::show(void)
{
  if(m_needReload) {
    loadAccounts();
    m_needReload = false;
  }

  // don't forget base class implementation
  KAccountsViewDecl::show();

  // if we have a selected account, let the application know about it
  KMyMoneyAccountTreeItem *item = m_accountTree->selectedItem();
  if(item) {
    emit selectObject(item->itemObject());
  }
}

void KAccountsView::slotLoadAccounts(void)
{
  if(isVisible()) {
    loadAccounts();
  } else {
    m_needReload = true;
  }
}

void KAccountsView::loadAccounts(void)
{
  QMap<QCString, bool> isOpen;

  ::timetrace("start load accounts view");
  // remember the id of the current selected item
  KMyMoneyAccountTreeItem *item = m_accountTree->selectedItem();
  QCString selectedItemId = (item) ? item->id() : QCString();

  // keep a map of all 'expanded' accounts
  QListViewItemIterator it_lvi(m_accountTree);
  while(it_lvi.current()) {
    item = dynamic_cast<KMyMoneyAccountTreeItem*>(it_lvi.current());
    if(item && item->isOpen()) {
      isOpen[item->id()] = true;
    }
    ++it_lvi;
  }

  // remember the upper left corner of the viewport
  QPoint startPoint = m_accountTree->viewportToContents(QPoint(0, 0));

  // turn off updates to avoid flickering during reload
  m_accountTree->setUpdatesEnabled(false);

  // clear the current contents and recreate it
  m_accountTree->clear();
  m_accountMap.clear();
  m_securityMap.clear();
  m_transactionCountMap.clear();

  // make sure, the pointers are not pointing to some deleted object
  m_assetItem = m_liabilityItem = 0;

  MyMoneyFile* file = MyMoneyFile::instance();

  QValueList<MyMoneyAccount> alist = file->accountList();
  QValueList<MyMoneyAccount>::const_iterator it_a;
  for(it_a = alist.begin(); it_a != alist.end(); ++it_a) {
    m_accountMap[(*it_a).id()] = *it_a;
  }

  QValueList<MyMoneySecurity> slist = file->currencyList();
  slist += file->securityList();
  QValueList<MyMoneySecurity>::const_iterator it_s;
  for(it_s = slist.begin(); it_s != slist.end(); ++it_s) {
    m_securityMap[(*it_s).id()] = *it_s;
  }

  m_transactionCountMap = file->transactionCountMap();

  m_accountTree->setBaseCurrency(file->baseCurrency());

  bool haveUnusedCategories = false;

  // create the items
  try {
    const MyMoneySecurity& security = file->baseCurrency();
    m_accountTree->setBaseCurrency(security);

    const MyMoneyAccount& asset = file->asset();
    m_assetItem = new KMyMoneyAccountTreeItem(m_accountTree, asset, security, i18n("Asset"));
    loadSubAccounts(m_assetItem, asset.accountList());

    const MyMoneyAccount& liability = file->liability();
    m_liabilityItem = new KMyMoneyAccountTreeItem(m_accountTree, liability, security, i18n("Liability"));
    loadSubAccounts(m_liabilityItem, liability.accountList());

    const MyMoneyAccount& income = file->income();
    KMyMoneyAccountTreeItem *incomeItem = new KMyMoneyAccountTreeItem(m_accountTree, income, security, i18n("Income"));
    haveUnusedCategories |= loadSubAccounts(incomeItem, income.accountList());

    const MyMoneyAccount& expense = file->expense();
    KMyMoneyAccountTreeItem *expenseItem = new KMyMoneyAccountTreeItem(m_accountTree, expense, security, i18n("Expense"));
    haveUnusedCategories |= loadSubAccounts(expenseItem, expense.accountList());

    if(KMyMoneySettings::expertMode()) {
      const MyMoneyAccount& equity = file->equity();
      KMyMoneyAccountTreeItem *equityItem = new KMyMoneyAccountTreeItem(m_accountTree, equity, security, i18n("Equity"));
      loadSubAccounts(equityItem, equity.accountList());
    }

  } catch(MyMoneyException *e) {
    kdDebug(2) << "Problem in accountsview: " << e->what();
    delete e;
  }

  // scan through the list of accounts and re-expand those that were
  // expanded and re-select the one that was probably selected before
  it_lvi = QListViewItemIterator(m_accountTree);
  while(it_lvi.current()) {
    item = dynamic_cast<KMyMoneyAccountTreeItem*>(it_lvi.current());
    if(item) {
      if(item->id() == selectedItemId)
        m_accountTree->setSelected(item, true);
      if(isOpen.find(item->id()) != isOpen.end())
        item->setOpen(true);
    }
    ++it_lvi;
  }

  // reposition viewport
  m_accountTree->setContentsPos(startPoint.x(), startPoint.y());

  // turn updates back on
  m_accountTree->setUpdatesEnabled(true);
  m_accountTree->repaintContents();

  // update the hint if categories are hidden
  m_hiddenCategories->setShown(haveUnusedCategories);

  // clear the current contents
  m_accountMap.clear();
  m_securityMap.clear();
  m_transactionCountMap.clear();
  ::timetrace("done load accounts view");
}

bool KAccountsView::loadSubAccounts(KMyMoneyAccountTreeItem* parent, const QCStringList& accountList)
{
  MyMoneyFile* file = MyMoneyFile::instance();

  bool unused = false;

  QCStringList::const_iterator it_a;
  for(it_a = accountList.begin(); it_a != accountList.end(); ++it_a) {
    const MyMoneyAccount& acc = m_accountMap[*it_a];
    QValueList<MyMoneyPrice> prices;
    MyMoneySecurity security = file->baseCurrency();
    try {
      if(acc.accountType() == MyMoneyAccount::Stock) {
        security = m_securityMap[acc.currencyId()];
        prices += file->price(acc.currencyId(), security.tradingCurrency());
        if(security.tradingCurrency() != file->baseCurrency().id()) {
          MyMoneySecurity sec = m_securityMap[security.tradingCurrency()];
          prices += file->price(sec.id(), file->baseCurrency().id());
        }
      } else if(acc.currencyId() != file->baseCurrency().id()) {
        if(acc.currencyId() != file->baseCurrency().id()) {
          security = m_securityMap[acc.currencyId()];
          prices += file->price(acc.currencyId(), file->baseCurrency().id());
        }
      }

    } catch(MyMoneyException *e) {
      kdDebug(2) << __PRETTY_FUNCTION__ << " caught exception while adding " << acc.name() << "[" << acc.id() << "]: " << e->what();
      delete e;
    }

    KMyMoneyAccountTreeItem* item = new KMyMoneyAccountTreeItem(parent, acc, prices, security);
    unused |= loadSubAccounts(item, acc.accountList());

    // no child accounts and no transactions in this account means 'unused'
    bool thisUnused = (!item->firstChild()) && (m_transactionCountMap[acc.id()] == 0);

    // In case of a category which is unused and we are requested to suppress
    // the display of those,
    if(acc.accountGroup() == MyMoneyAccount::Income
    || acc.accountGroup() == MyMoneyAccount::Expense) {
      if(KMyMoneySettings::hideUnusedCategory() && thisUnused) {
        unused = true;
        delete item;
      }
    }
  }
  return unused;
}

void KAccountsView::slotUpdateNetWorth(void)
{
  if(!m_assetItem || !m_liabilityItem)
    return;

  MyMoneyMoney netWorth = m_assetItem->totalValue() - m_liabilityItem->totalValue();

  QString s(i18n("Net Worth: "));

  // FIXME figure out how to deal with the approximate
  // if(!(file->totalValueValid(assetAccount.id()) & file->totalValueValid(liabilityAccount.id())))
  //  s += "~ ";

  s.replace(QString(" "), QString("&nbsp;"));
  if(netWorth.isNegative()) {
    s += "<b><font color=\"red\">";
  }
  const MyMoneySecurity& sec = MyMoneyFile::instance()->baseCurrency();
  QString v(netWorth.formatMoney(sec.tradingSymbol(), MyMoneyMoney::denomToPrec(sec.smallestAccountFraction())));
  s += v.replace(QString(" "), QString("&nbsp;"));
  if(netWorth.isNegative()) {
    s += "</font></b>";
  }

  m_totalProfitsLabel->setFont(KMyMoneySettings::listCellFont());
  m_totalProfitsLabel->setText(s);
}

#include "kaccountsview.moc"
