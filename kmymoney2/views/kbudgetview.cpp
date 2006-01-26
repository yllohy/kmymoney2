/***************************************************************************
                          kbudgetview.cpp
                          ---------------
    begin                : Thu Jan 10 2006
    copyright            : (C) 2006 by Darren Gould
    email                : darren_gould@gmx.de
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

#include <qpushbutton.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qmultilineedit.h>
#include <qpixmap.h>
#include <qtabwidget.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>
#include <kmessagebox.h>
#include <kpushbutton.h>
#include <kiconloader.h>
#include <kguiitem.h>
#include <kstandarddirs.h>
#include <kdebug.h>

// ----------------------------------------------------------------------------
// STL includes
#include <algorithm>
// for std::find since the find() functions provided by QValueList do not
// allow the lookup of a payee based on its id.

// ----------------------------------------------------------------------------
// Project Includes

#include "kbudgetview.h"
#include "../mymoney/mymoneyfile.h"
#include "../kmymoneysettings.h"
#include "../dialogs/knewbudgetdlg.h"

// *** KBudgetView Implementation ***

KBudgetView::KBudgetView(QWidget *parent, const char *name )
  : KBudgetViewDecl(parent,name),
    m_suspendUpdate(false)
{
  MyMoneyFile::instance()->attach(MyMoneyFile::NotifyClassPayeeSet, this);
}

KBudgetView::~KBudgetView()
{
  MyMoneyFile::instance()->detach(MyMoneyFile::NotifyClassPayeeSet, this);
}

void KBudgetView::update(const QCString & /*id*/)
{
  if(m_suspendUpdate == false)
    slotRefreshView();
}

void KBudgetView::show()
{
  QTimer::singleShot(50, this, SLOT(rearrange()));
  emit signalViewActivated();
  QWidget::show();
}

void KBudgetView::rearrange(void)
{
  resizeEvent(0);
}

void KBudgetView::resizeEvent(QResizeEvent* ev)
{
  // resize the register
  KBudgetViewDecl::resizeEvent(ev);
}

void KBudgetView::slotReloadView(void)
{
  ::timetrace("Start KBudgetView::slotReloadView");
  rearrange();
  ::timetrace("Done KBudgetView::slotReloadView");
}

void KBudgetView::slotLoadAccounts(void)
{
  if(isVisible()) {
    loadAccounts();
  } else {
    m_needReload = true;
  }
}


void KBudgetView::slotRefreshView(void)
{

}

void KBudgetView::loadAccounts(void)
{
  QMap<QCString, bool> isOpen;

  ::timetrace("start load budgets view");
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
  m_incomeItem = m_expenseItem = 0;

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

  bool haveUnusedBudgets = false;

  // create the items
  try {
    const MyMoneySecurity& security = file->baseCurrency();
    m_accountTree->setBaseCurrency(security);

    const MyMoneyAccount& income = file->income();
    m_incomeItem = new KMyMoneyAccountTreeItem(m_accountTree, income, security, i18n("Income"));
    haveUnusedBudgets |= loadSubAccounts(m_incomeItem, income.accountList());

    const MyMoneyAccount& expense = file->expense();
    m_expenseItem = new KMyMoneyAccountTreeItem(m_accountTree, expense, security, i18n("Expense"));
    haveUnusedBudgets |= loadSubAccounts(m_expenseItem, expense.accountList());

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

  // update the hint if budget are hidden
  //m_hiddenBudgets->setShown(haveUnusedBudgets);

  ::timetrace("done load budgets view");
}


bool KBudgetView::loadSubAccounts(KMyMoneyAccountTreeItem* parent, const QCStringList& accountList)
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

    // no child accounts and not transactions in this account means 'unused'
    bool thisUnused = (!item->firstChild()) && (m_transactionCountMap[acc.id()] == 0);

    // In case of a budget which is unused and we are requested to suppress
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


void KBudgetView::m_bNewBudget_clicked()
{
  m_currentState.setName("Budget for 2005");
  MyMoneyFile::instance()->addBudget(m_currentState);

  KNewBudgetDlg *dlg = new KNewBudgetDlg(this, QString("New Budget"));
  if (dlg->exec())
  {
  }
  delete dlg;
}

#include "kbudgetview.moc"
