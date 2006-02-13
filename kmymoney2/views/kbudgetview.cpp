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
#include <qlistbox.h>
#include <kcombobox.h>

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
// allow the lookup of a budget based on its id.

// ----------------------------------------------------------------------------
// Project Includes

#include "kbudgetview.h"
#include "../mymoney/mymoneyfile.h"
#include "../kmymoneysettings.h"
#include "../widgets/kmymoneytitlelabel.h"

// *** KBudgetListItem Implementation ***
KBudgetListItem::KBudgetListItem(KListView *parent, const MyMoneyBudget& budget) :
  KListViewItem(parent),
  m_budget(budget)
{
  setText(0, budget.name());
  // allow in column rename
  setRenameEnabled(0, true);
}

KBudgetListItem::~KBudgetListItem()
{
}

void KBudgetListItem::paintCell(QPainter *p, const QColorGroup & cg, int column, int width, int align)
{
  p->setFont(KMyMoneySettings::listCellFont());

  QColor colour = KMyMoneySettings::listColor();
  QColor bgColour = KMyMoneySettings::listBGColor();

  QColorGroup cg2(cg);

  if (isAlternate())
    cg2.setColor(QColorGroup::Base, colour);
  else
    cg2.setColor(QColorGroup::Base, bgColour);

  QListViewItem::paintCell(p, cg2, column, width, align);
}

// *** KBudgetView Implementation ***
const int KBudgetView::m_iBudgetYearsAhead = 5;
const int KBudgetView::m_iBudgetYearsBack = 3;

KBudgetView::KBudgetView(QWidget *parent, const char *name )
  : KBudgetViewDecl(parent,name),
    m_suspendUpdate(false)
{
  m_budgetAmountList->setSorting(-1);
  m_budgetList->setSorting(-1);
  titleLabel->setRightImageFile("pics/titlelabel_background.png" );

  connect(m_budgetList, SIGNAL(rightButtonClicked(QListViewItem* , const QPoint&, int)),
    this, SLOT(slotOpenContextMenu(QListViewItem*)));
  connect(m_budgetList, SIGNAL(itemRenamed(QListViewItem*,int,const QString&)), this, SLOT(slotRenameBudget(QListViewItem*,int,const QString&)));
  connect(m_budgetList, SIGNAL(selectionChanged()), this, SLOT(slotSelectBudget()));
  connect(m_dlYear, SIGNAL(activated(int)), this, SLOT(slotSelectYear(int)));

  connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotRefreshView()));
}

KBudgetView::~KBudgetView()
{
}

void KBudgetView::show()
{
  QTimer::singleShot(50, this, SLOT(slotRearrange()));
  emit signalViewActivated();
  QWidget::show();
  slotRefreshView();
}

void KBudgetView::slotRearrange(void)
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
  slotRearrange();
  ::timetrace("Done KBudgetView::slotReloadView");
}

void KBudgetView::loadBudget(void)
{
  QCString id;
  MyMoneyBudget budget;

  ::timetrace("Start KBudgetView::loadBudget");

  // remember which items are selected in the list
  if (selectedBudget(budget))
    id = budget.id();
  else
    id = NULL;

  // remember the upper left corner of the viewport
  QPoint startPoint = m_budgetList->viewportToContents(QPoint(0, 0));

  // turn off updates to avoid flickering during reload
  m_budgetList->setUpdatesEnabled(false);

  // clear the list
  m_budgetList->clear();
  m_yearList.clear();
  KBudgetListItem* currentItem = 0;

  QValueList<MyMoneyBudget> list = MyMoneyFile::instance()->budgetList();
  QValueList<MyMoneyBudget>::ConstIterator it;

  for (it = list.begin(); it != list.end(); ++it) {
    KBudgetListItem* item = new KBudgetListItem(m_budgetList, *it);

    // create a list of unique years
    if (m_yearList.findIndex((*it).budgetstart().year()) == -1 )
	m_yearList.append((*it).budgetstart().year());

    if(item->budget().id() == id)
    {
      currentItem = item;
      item->setSelected(true);
    }
  }

  QDate date = QDate::currentDate(Qt::LocalTime);
  int iStartYear = date.year() - m_iBudgetYearsBack;

  for (int i=0; i<m_iBudgetYearsAhead + m_iBudgetYearsBack; i++)
  {
    if (m_yearList.findIndex(iStartYear+i) == -1 )
	m_yearList.append(iStartYear+i);
  }

  qHeapSort( m_yearList );
  m_dlYear->clear();
  QValueList<int>::Iterator iit;
  for(iit = m_yearList.begin(); iit != m_yearList.end(); ++iit)
    m_dlYear->insertItem(QString::number(*iit));

  if (currentItem)
  {
    int iYear = currentItem->budget().budgetstart().year();
    if (m_yearList.findIndex(iYear) >= 0)
	m_dlYear->setCurrentItem(m_yearList.findIndex(iYear));

    m_budgetList->setCurrentItem(currentItem);
    
  }

  // reposition viewport
  m_budgetList->setContentsPos(startPoint.x(), startPoint.y());

  // turn updates back on
  m_budgetList->setUpdatesEnabled(true);
  m_budgetList->repaintContents();

  ::timetrace("End KBudgetView::loadBudget");
}

void KBudgetView::ensureBudgetVisible(const QCString& id)
{
  for (QListViewItem * item = m_budgetList->firstChild(); item; item = item->itemBelow()) {
    KBudgetListItem* p = dynamic_cast<KBudgetListItem*>(item);
    if(p && p->budget().id() == id) {
      if(p->itemAbove())
        m_budgetList->ensureItemVisible(p->itemAbove());
      if(p->itemBelow())
        m_budgetList->ensureItemVisible(p->itemBelow());

      m_budgetList->setCurrentItem(p);      // active item and deselect all others
      m_budgetList->setSelected(p, true);   // and select it
      m_budgetList->ensureItemVisible(p);
      break;
    }
  }
}

void KBudgetView::slotRefreshView(void)
{
  if(isVisible()) {
    loadBudget();
    loadAccounts();
  } else {
    m_needReload = true;
  }
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

void KBudgetView::slotSelectBudget()
{
  MyMoneyBudget budget;
  if (!selectedBudget(budget))
    return;

  QValueList<MyMoneyBudget> budgetList;
  budgetList << budget;
  emit selectObjects(budgetList);
}

bool KBudgetView::selectedBudget(MyMoneyBudget& budget) const
{
  QListViewItemIterator it_l(m_budgetList, QListViewItemIterator::Selected);
  if (it_l.current() == 0)
    return false;

  QListViewItem* it_v = it_l.current();
  KBudgetListItem* item = dynamic_cast<KBudgetListItem*>(it_v);
  if(item)
  {
    budget = item->budget();
    if (m_yearList.findIndex(budget.budgetstart().year()) >= 0)
	m_dlYear->setCurrentItem(m_yearList.findIndex(budget.budgetstart().year()));
  }
  return true;
}

void KBudgetView::slotOpenContextMenu(QListViewItem* i)
{
  KBudgetListItem* item = dynamic_cast<KBudgetListItem*>(i);
  if (item)
    emit openContextMenu(item->budget());
  else
    emit openContextMenu(MyMoneyBudget());
}

void KBudgetView::slotStartRename(void)
{
  QListViewItemIterator it_l(m_budgetList, QListViewItemIterator::Selected);
  QListViewItem* it_v = it_l.current();
  if(it_v) {
    it_v->startRename(0);
  }
}

// This variant is only called when a single budget is selected and renamed.
void KBudgetView::slotRenameBudget(QListViewItem* p , int /* col*/ , const QString& txt)
{
  KBudgetListItem *pBudget = dynamic_cast<KBudgetListItem*> (p);
  if (!pBudget)
    return;

  //kdDebug() << "[KPayeesView::slotRenamePayee]" << endl;
  // create a copy of the new name without appended whitespaces
  QString new_name = txt.stripWhiteSpace();
  if (pBudget->budget().name() != new_name) {
    try {
      // check if we already have a payee with the new name
      try {
        // this function call will throw an exception, if the payee
        // hasn't been found.
        MyMoneyFile::instance()->budgetByName(new_name);
        // the name already exists, ask the user whether he's sure to keep the name
        if (KMessageBox::questionYesNo(this,
          i18n("A payee with the name '%1' already exists. It is not advisable to have "
            "multiple payees with the same identification name. Are you sure you would like "
            "to rename the payee?").arg(new_name)) != KMessageBox::Yes)
        {
          pBudget->setText(0,pBudget->budget().name());
          return;
        }
      } catch(MyMoneyException *e) {
        // all ok, the name is unique
        delete e;
      }

      pBudget->budget().setName(new_name);
      MyMoneyFile::instance()->modifyBudget(pBudget->budget());

      // the above call to modifyPayee will reload the view so
      // all references and pointers to the view have to be
      // re-established.

      // make sure, that the record is visible even if it moved
      // out of sight due to the rename operation
      //ensureBudgetVisible((QCString)(pBudget->budget.id()));
    } catch(MyMoneyException *e) {
      KMessageBox::detailedSorry(0, i18n("Unable to modify budget"),
        (e->what() + " " + i18n("thrown in") + " " + e->file()+ ":%1").arg(e->line()));
      delete e;
    }
  }
  else {
    pBudget->setText(0, new_name);
  }
}

void KBudgetView::slotSelectYear(int iYear)
{
  QListViewItemIterator it_l(m_budgetList, QListViewItemIterator::Selected);
  QListViewItem* it_v;
  if((it_v = it_l.current()) != 0) {
     KBudgetListItem *pBudget = dynamic_cast<KBudgetListItem*> (it_v);
     if (!pBudget)
       return;
 
    MyMoneyBudget budget = pBudget->budget();
    QDate date(m_dlYear->text(iYear).toInt(), 1, 1);
    budget.setBudgetStart(date);

    MyMoneyFile::instance()->modifyBudget(budget);
  }
}

#include "kbudgetview.moc"
