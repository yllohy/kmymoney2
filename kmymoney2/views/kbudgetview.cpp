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
// Project Includes

#include <kmymoney/mymoneyfile.h>

#include <kmymoney/kmymoneyglobalsettings.h>
#include <kmymoney/kmymoneytitlelabel.h>
#include <kmymoney/kmymoneyedit.h>

#include "kbudgetview.h"
#include "../dialogs/knewbudgetdlg.h"

// *** KBudgetListItem Implementation ***
KBudgetListItem::KBudgetListItem(KListView *parent, const MyMoneyBudget& budget) :
  KListViewItem(parent),
  m_budget(budget)
{
  setText(0, budget.name());
  setText(1, QString("%1").arg(budget.budgetstart().year()));

  // allow in column rename
  setRenameEnabled(0, true);
}

KBudgetListItem::~KBudgetListItem()
{
}

void KBudgetListItem::paintCell(QPainter *p, const QColorGroup & cg, int column, int width, int align)
{
  p->setFont(KMyMoneyGlobalSettings::listCellFont());

  QColor colour = KMyMoneySettings::listColor();
  QColor bgColour = KMyMoneySettings::listBGColor();

  QColorGroup cg2(cg);

  if (isAlternate())
    cg2.setColor(QColorGroup::Base, colour);
  else
    cg2.setColor(QColorGroup::Base, bgColour);

  QListViewItem::paintCell(p, cg2, column, width, align);
}

// *** KBudgetListItem Implementation ***
KBudgetAmountListItem::KBudgetAmountListItem(KListView *parent, KMyMoneyAccountTreeBudgetItem *account, const MyMoneyMoney& amount, const QDate& date) :
  KListViewItem(parent),
  m_account(account),
  m_amount(amount),
  m_date(date),
  m_label("")
{
  setText(0, QDate::shortMonthName(date.month()));
  setAmount( amount );
  // allow column to be renamed
  setRenameEnabled(1, true);
  setRenameEnabled(0, false);
}

KBudgetAmountListItem::KBudgetAmountListItem(KListView *parent, KMyMoneyAccountTreeBudgetItem *account, const MyMoneyMoney& amount, const QDate &date, const QString &label) :
  KListViewItem(parent),
  m_account(account),
  m_amount(amount),
  m_date(date),
  m_label(label)
{
  setText(0, label);
  setAmount( amount );
  // allow column to be renamed
  setRenameEnabled(1, true);
  setRenameEnabled(0, false);
}

KBudgetAmountListItem::~KBudgetAmountListItem()
{
}

void KBudgetAmountListItem::setAmount(const MyMoneyMoney &newamount)
{
    m_amount = newamount;
    if (m_account)
      setText(1, newamount.formatMoney(m_account->tradingSymbol(), 2));
    else
      setText(1, newamount.formatMoney("", 2));
}

void KBudgetAmountListItem::paintCell(QPainter *p, const QColorGroup & cg, int column, int width, int align)
{
  p->setFont(KMyMoneyGlobalSettings::listCellFont());

  QColor colour = KMyMoneyGlobalSettings::listColor();
  QColor bgColour = KMyMoneyGlobalSettings::listBGColor();

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
  : KBudgetViewDecl(parent,name)
{
  m_budgetAmountList->setSorting(-1);
  m_accountTree->setSorting(-1);
  m_budgetAmountList->setColumnText(0, QString(""));
  // allow in column rename
  m_budgetAmountList->setAllColumnsShowFocus(true);
  m_budgetAmountList->setTabOrderedRenaming(true);

  connect(m_budgetList, SIGNAL(rightButtonClicked(QListViewItem* , const QPoint&, int)),
    this, SLOT(slotOpenContextMenu(QListViewItem*)));
  connect(m_budgetList, SIGNAL(itemRenamed(QListViewItem*,int,const QString&)), this, SLOT(slotRenameBudget(QListViewItem*,int,const QString&)));
  connect(m_budgetList, SIGNAL(selectionChanged()), this, SLOT(slotSelectBudget()));

  //connect(m_dlYear, SIGNAL(activated(int)), this, SLOT(slotSelectYear(int)));

  connect(m_dbTimeSpan, SIGNAL(activated(int)), this, SLOT(slotSelectTimeSpan(int)));

  connect(m_accountTree, SIGNAL(selectionChanged()), this, SLOT(slotSelectObject()));

  connect(m_budgetAmountList, SIGNAL(itemRenamed(QListViewItem*,int,const QString&)), this, SLOT(slotBudgetedAmount(QListViewItem*,int,const QString&)));
  connect(m_budgetAmountList, SIGNAL(clicked(QListViewItem *)), this, SLOT(slotBudgetAmountClicked(QListViewItem *)));

  //(ace) kCategoryWidget not currently defined
  //connect(m_leAccounts, SIGNAL(signalEnter()), this, SLOT(AccountEnter()));

  connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotRefreshView()));
}

KBudgetView::~KBudgetView()
{
}

void KBudgetView::show()
{
  QTimer::singleShot(50, this, SLOT(slotRearrange()));
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

  // clear the budget list
  m_budgetList->clear();

  // add the correct years to the drop down list
  QDate date = QDate::currentDate(Qt::LocalTime);
  int iStartYear = date.year() - m_iBudgetYearsBack;

  m_yearList.clear();
  //m_dlYear->clear();
  for (int i=0; i<m_iBudgetYearsAhead + m_iBudgetYearsBack; i++)
    m_yearList += QString::number(iStartYear+i);

  KBudgetListItem* currentItem = 0;

  QValueList<MyMoneyBudget> list = MyMoneyFile::instance()->budgetList();
  QValueList<MyMoneyBudget>::ConstIterator it;
  for (it = list.begin(); it != list.end(); ++it)
  {
    KBudgetListItem* item = new KBudgetListItem(m_budgetList, *it);

    // create a list of unique years
    if (m_yearList.findIndex(QString::number((*it).budgetstart().year())) == -1)
      m_yearList += QString::number((*it).budgetstart().year());

    if(item->budget().id() == id)
    {
      currentItem = item;
      item->setSelected(true);
    }
  }
  m_yearList.sort();
  //m_dlYear->insertStringList(m_yearList);

  if (currentItem)
  {
    int iYear = currentItem->budget().budgetstart().year();
    //if (m_yearList.findIndex(QString::number(iYear)) >= 0)
    //  m_dlYear->setCurrentItem(m_yearList.findIndex(QString::number(iYear)));

    m_budgetList->setCurrentItem(currentItem);
  }

  // reposition viewport
  m_budgetList->setContentsPos(startPoint.x(), startPoint.y());

  // turn updates back on
  m_budgetList->setUpdatesEnabled(true);
  m_budgetList->repaintContents();
  m_budgetList->setSorting(-1);

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
    QListViewItem * item = m_budgetList->firstChild();
    m_budgetList->setSelected(item, true);
  } else {
    m_needReload = true;
  }
}

void KBudgetView::loadAccounts(void)
{
  QMap<QCString, bool> isOpen;

  ::timetrace("start load budget account view");

  // if no budgets are selected, don't load the accounts
  MyMoneyBudget budget;
  if (!selectedBudget(budget))
    return;

  // remember the id of the current selected item
  KMyMoneyAccountTreeBudgetItem *item = m_accountTree->selectedItem();
  QCString selectedItemId = (item) ? item->id() : QCString();

  // keep a map of all 'expanded' accounts
  QListViewItemIterator it_lvi(m_accountTree);
  while(it_lvi.current()) {
    item = dynamic_cast<KMyMoneyAccountTreeBudgetItem*>(it_lvi.current());
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
    m_incomeItem = new KMyMoneyAccountTreeBudgetItem(m_accountTree, income, budget, security, i18n("Income"));
    haveUnusedBudgets |= loadSubAccounts(m_incomeItem, income.accountList(), budget);

    const MyMoneyAccount& expense = file->expense();
    m_expenseItem = new KMyMoneyAccountTreeBudgetItem(m_accountTree, expense, budget, security, i18n("Expense"));
    haveUnusedBudgets |= loadSubAccounts(m_expenseItem, expense.accountList(), budget);

  } catch(MyMoneyException *e) {
    kdDebug(2) << "Problem in accountsview: " << e->what();
    delete e;
  }

  // scan through the list of accounts and re-expand those that were
  // expanded and re-select the one that was probably selected before
  it_lvi = QListViewItemIterator(m_accountTree);
  while(it_lvi.current()) {
    item = dynamic_cast<KMyMoneyAccountTreeBudgetItem*>(it_lvi.current());
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
  slotSelectObject();

  ::timetrace("done load budgets view");
}


bool KBudgetView::loadSubAccounts(KMyMoneyAccountTreeBudgetItem* parent, const QCStringList& accountList, const MyMoneyBudget& budget)
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

    KMyMoneyAccountTreeBudgetItem *item = new KMyMoneyAccountTreeBudgetItem(parent, acc, budget, prices, security);
    unused |= loadSubAccounts(item, acc.accountList(), budget);

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
  {
    QListViewItem * item = m_budgetList->firstChild();
    KBudgetListItem* p = dynamic_cast<KBudgetListItem*>(item);
    if(p)
    {
      m_budgetList->setSelected(p, true);
    }
    else
    {
      return;
    }
  }

  loadAccounts();
  QValueList<MyMoneyBudget> budgetList;
  budgetList << budget;
  emit selectObjects(budgetList);
}

bool KBudgetView::selectedBudget(MyMoneyBudget& budget) const
{
  m_accountTree->setEnabled(false);

  QListViewItemIterator it_l(m_budgetList, QListViewItemIterator::Selected);
  if (it_l.current() == 0)
    return false;

  QListViewItem* it_v = it_l.current();
  KBudgetListItem* item = dynamic_cast<KBudgetListItem*>(it_v);
  if(item)
  {
    budget = item->budget();
    //if (m_yearList.findIndex(QString::number(budget.budgetstart().year())) >= 0)
      //m_dlYear->setCurrentItem(m_yearList.findIndex(QString::number(budget.budgetstart().year())));
  }
  m_accountTree->setEnabled(true);

  return true;
}

KMyMoneyAccountTreeBudgetItem* KBudgetView::selectedAccount(void) const
{
  m_dbTimeSpan->setEnabled(false);
  m_budgetAmountList->setEnabled(false);

  QListViewItemIterator it_l(m_accountTree, QListViewItemIterator::Selected);
  if (it_l.current() == 0)
    return NULL;

  QListViewItem* it_v = it_l.current();
  KMyMoneyAccountTreeBudgetItem* item = dynamic_cast<KMyMoneyAccountTreeBudgetItem*>(it_v);
  if(item)
  {
    if(item->id() == MyMoneyFile::instance()->expense().id()
    || item->id() == MyMoneyFile::instance()->income().id())
      return NULL;

    m_dbTimeSpan->setEnabled(true);
    m_budgetAmountList->setEnabled(true);

    return item;
  }
  return NULL;
}

void KBudgetView::slotOpenContextMenu(QListViewItem* i)
{
  m_accountTree->setUpdatesEnabled(false);

  KBudgetListItem* item = dynamic_cast<KBudgetListItem*>(i);
  if (item)
    emit openContextMenu(item->budget());
  else
    emit openContextMenu(MyMoneyBudget());

  m_accountTree->setUpdatesEnabled(true);
}

void KBudgetView::slotStartRename(void)
{
  QListViewItemIterator it_l(m_budgetList, QListViewItemIterator::Selected);
  QListViewItem* it_v;
  if((it_v = it_l.current()) != 0) {
    it_v->startRename(0);
  }
}

// This variant is only called when a single budget is selected and renamed.
void KBudgetView::slotRenameBudget(QListViewItem* p , int /*col*/, const QString& txt)
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
          i18n("A budget with the name '%1' already exists. It is not advisable to have "
            "multiple budgets with the same identification name. Are you sure you would like "
            "to rename the budget?").arg(new_name)) != KMessageBox::Yes)
        {
          p->setText(0,pBudget->budget().name());
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
  MyMoneyBudget budget;
  if (!selectedBudget(budget))
    return;

  //QDate date(m_dlYear->text(iYear).toInt(), 1, 1);
  QDate date(iYear, 1, 1);
  budget.setBudgetStart(date);

  MyMoneyFile::instance()->modifyBudget(budget);
}

void KBudgetView::slotSelectTimeSpan(int iTimeSpan)
{
  MyMoneyBudget budget;
  if (!selectedBudget(budget))
    return;

  KMyMoneyAccountTreeBudgetItem *account;
  if ((account=selectedAccount()) == NULL)
    return;

  MyMoneyBudget::AccountGroup accountGroup;
  accountGroup.setId( account->id() );

  switch (iTimeSpan)
  {
    case eMonthByMonth:
      accountGroup.setBudgetLevel(MyMoneyBudget::AccountGroup::eMonthByMonth);
      break;
    case eYearly:
      accountGroup.setBudgetLevel(MyMoneyBudget::AccountGroup::eYearly);
      break;
    case eMonthly:
      accountGroup.setBudgetLevel(MyMoneyBudget::AccountGroup::eMonthly);
      break;
    default:
      return;
  }

  setTimeSpan(account, accountGroup, iTimeSpan);

  budget.setAccount(accountGroup, account->id());
  MyMoneyFile::instance()->modifyBudget(budget);
}

void KBudgetView::setTimeSpan(KMyMoneyAccountTreeBudgetItem *account, MyMoneyBudget::AccountGroup &accountGroup, int iTimeSpan)
{
  MyMoneyBudget budget;
  if (!selectedBudget(budget))
    return;

  MyMoneyMoney amount;  MyMoneyBudget::PeriodGroup period;
  QDate date = budget.budgetstart();

  m_budgetAmountList->clear();
  switch (iTimeSpan)
  {
    case eMonthByMonth:
      // FIXME: this should not be hard coded to the 12th month
      // we should make this flexible for a later implementation
      // when the budget doesn't start on Jan. 1
      date.setYMD(date.year(), 12, 1);
      m_budgetAmountList->setColumnText(0, i18n("Month"));
      for (int i=0; i<12; i++)
      {
        MyMoneyBudget::PeriodGroup period = accountGroup.getPeriod(date);
        amount = period.amount();
        new KBudgetAmountListItem(m_budgetAmountList, account, amount, date);
        date = date.addMonths(-1);
      }
      break;
    case eYearly:
      period = accountGroup.getPeriod(date);
      amount = period.amount();
      new KBudgetAmountListItem(m_budgetAmountList, account, amount, date, i18n("Yearly Amount"));
      m_budgetAmountList->setColumnText(0, QString(""));
      break;
    case eMonthly:
      period = accountGroup.getPeriod(date);
      amount = period.amount();
      new KBudgetAmountListItem(m_budgetAmountList, account, amount, date, i18n("Monthly Amount"));
      m_budgetAmountList->setColumnText(0, QString(""));
      break;
    default:
      return;
  }
}

void KBudgetView::slotSelectObject()
{
  m_budgetAmountList->clear();

  MyMoneyBudget budget;
  if (!selectedBudget( budget ) )
    return;

  // turn off updates to avoid flickering during reload
  m_accountTree->setUpdatesEnabled(false);

  KMyMoneyAccountTreeBudgetItem *account;
  if ((account=selectedAccount()) == NULL)
  {
    QListViewItem *i = m_accountTree->firstChild();
    account = dynamic_cast<KMyMoneyAccountTreeBudgetItem*>(i);
    if (account)
    {
      m_accountTree->setSelected(account, true);
      m_accountTree->setOpen(account, true);

    }
    else
    {
      m_accountTree->setUpdatesEnabled(true);
      return;
    }
  }

  if (account)
  {
    QCString id = account->id();
    //(marce) This updates the name of the category in the panel on the right of the view
    m_leAccounts->setText(MyMoneyFile::instance()->account(id).name());


    //(ace) kCategoryWidget not currently defined
    //m_leAccounts->blockSignals(true);
    //m_leAccounts->loadAccount(id);
    //m_leAccounts->blockSignals(false);

    MyMoneyBudget::AccountGroup budgetAccount = budget.account( id );
    if ( id != budgetAccount.id() )
    {
      m_dbTimeSpan->setCurrentItem(eMonthly);
      setTimeSpan(account, budgetAccount, eMonthly);
      m_accountTree->setUpdatesEnabled(true);
      return;
    }

    switch (budgetAccount.budgetlevel())
    {
      case MyMoneyBudget::AccountGroup::eMonthly:
        m_dbTimeSpan->setCurrentItem(eMonthly);
        setTimeSpan(account, budgetAccount, eMonthly);
        break;
      case MyMoneyBudget::AccountGroup::eYearly:
        m_dbTimeSpan->setCurrentItem(eYearly);
        setTimeSpan(account, budgetAccount, eYearly);
        break;
      case MyMoneyBudget::AccountGroup::eMonthByMonth:
        m_dbTimeSpan->setCurrentItem(eMonthByMonth);
        setTimeSpan(account, budgetAccount, eMonthByMonth);
        break;
      default:
        m_accountTree->setUpdatesEnabled(true);
        return;
    }

    QListViewItemIterator it(m_budgetAmountList);
    while(it.current())
    {
      KBudgetAmountListItem *item = dynamic_cast<KBudgetAmountListItem*>(it.current());
      if(item)
      {
        MyMoneyBudget::PeriodGroup period = budgetAccount.getPeriod(item->date());
        item->setAmount(period.amount());
      }
      ++it;
    }
  }
  m_accountTree->setUpdatesEnabled(true);
}

void KBudgetView::slotBudgetedAmount(QListViewItem *p, int col, const QString& newamount)
{
  KBudgetAmountListItem *item = dynamic_cast<KBudgetAmountListItem*>(p);
  if (item && col == 1)
  {
    int pos=0;
    QString amnt = newamount;
    QObject* obj = dynamic_cast<QObject*>(m_budgetAmountList);
    if (obj)
    {
      kMyMoneyMoneyValidator validator(obj);
      if ( validator.validate( amnt, pos ) == QValidator::Acceptable )
      {
        MyMoneyBudget budget;
        if (!selectedBudget(budget))
          return;

        KMyMoneyAccountTreeBudgetItem *account;
        if ((account=selectedAccount()) == NULL)
          return;

        item->setAmount(newamount);

        MyMoneyBudget::AccountGroup accountGroup = budget.account(account->id());
        accountGroup.setId( account->id() );
        switch( m_dbTimeSpan->currentItem() )
        {
          case eMonthly:      accountGroup.setBudgetLevel(MyMoneyBudget::AccountGroup::eMonthly);
          break;
          case eMonthByMonth: accountGroup.setBudgetLevel(MyMoneyBudget::AccountGroup::eMonthByMonth);
          break;
          case eYearly:       accountGroup.setBudgetLevel(MyMoneyBudget::AccountGroup::eYearly);
          break;
        }

        MyMoneyBudget::PeriodGroup period;
        period.setAmount(item->amount());
        period.setDate(item->date());
        accountGroup.addPeriod(item->date(), period);
        budget.setAccount(accountGroup, account->id());

        MyMoneyFile::instance()->modifyBudget(budget);
      }
      else
        item->setAmount(item->amount());
    }
  }
}

void KBudgetView::AccountEnter()
{
  MyMoneyBudget budget;
  if (!selectedBudget( budget ) )
    return;

  //(ace) kCategoryWidget not currently defined
  KMyMoneyAccountTreeBudgetItem *item = NULL; //dynamic_cast<KMyMoneyAccountTreeBudgetItem*> (m_accountTree->findItem(m_leAccounts->selectedAccountId()));
  if (item)
  {
    m_accountTree->setCurrentItem(item);
    m_accountTree->setOpen(item, true);
  }
}

void KBudgetView::slotBudgetAmountClicked(QListViewItem *item)
{
  if(item)
  {
    item->startRename(1);
  }
}

void KBudgetView::bNewBudget_clicked()
{
  KNewBudgetDlg* mydialog = new KNewBudgetDlg( this, i18n("New budget dialog"));

  if(mydialog->exec() == QDialog::Accepted) {
    MyMoneyBudget budget;
    QDate mydate = QDate(mydialog->getYear().toInt(),1 ,1);

    budget.setName(mydialog->getName());
    budget.setBudgetStart(mydate);

   //TODO: check that no other budget in the storage with the same name and year exists
    MyMoneyFile::instance()->addBudget(budget);
  }
  delete mydialog;
}

void KBudgetView::bEditBudget_clicked()
{
  slotStartRename( );
}

void KBudgetView::bDeleteBudget_clicked()
{
  QListViewItemIterator it_l(m_budgetList, QListViewItemIterator::Selected);
  QListViewItem* it_v;
  if((it_v = it_l.current()) != 0) {
    //The item in the list "it_v" is the budget the user has selected (highlighted in the list)
    //First, we take budget list from the storage
    QValueList<MyMoneyBudget> allBudgetsList = MyMoneyFile::instance()->budgetList();
    //Then we travel the list (=iterate) in a quest for the first budget with the same name and year than our one.
    QValueList<MyMoneyBudget>::iterator it_allBL = allBudgetsList.begin();
    for (it_allBL = allBudgetsList.begin(); it_allBL != allBudgetsList.end(); it_allBL++){
      if(((*it_allBL).name() == it_v->text(0))
      && ((*it_allBL).budgetstart().year() == it_v->text(1).toInt())) {
        //with the budget located at it_allBL, request confirmation from user
        QString prompt = QString("<qt>")+i18n("Do you really want to remove the budget <b>%1</b> for year <b>%2</b>").arg((*it_allBL).name()).arg((*it_allBL).budgetstart().year())+QString("</qt>");
        if (KMessageBox::questionYesNo(this, prompt, i18n("Remove Budget"))==KMessageBox::No)
          return;

        // now the obliteration operation itself
        try {
          MyMoneyFile::instance()->removeBudget(*it_allBL);
        } catch (MyMoneyException *e) {
          KMessageBox::detailedSorry(0, i18n("Unable to remove budget(s)"), (e->what() + " " + i18n("thrown in") + " " + e->file()+ ":%1").arg(e->line()));
          delete e;
        }
      }
    }
  }
}


#include "kbudgetview.moc"
