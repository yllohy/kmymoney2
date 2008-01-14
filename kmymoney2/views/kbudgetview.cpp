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
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qtooltip.h>

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
#include <kcalendarsystem.h>
#include <kcombobox.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/mymoneyfile.h>

#include <kmymoney/kmymoneyglobalsettings.h>
#include <kmymoney/kmymoneytitlelabel.h>
#include <kmymoney/kmymoneyedit.h>
#include <kmymoney/kbudgetvalues.h>
#include "../widgets/kmymoneyaccounttreebudget.h"
#include "kbudgetview.h"
#include "../dialogs/knewbudgetdlg.h"
#include "../kmymoney2.h"

// *** KBudgetListItem Implementation ***
KBudgetListItem::KBudgetListItem(KListView *parent, const MyMoneyBudget& budget) :
  KListViewItem(parent),
  m_budget(budget)
{
  setText(0, budget.name());
  setText(1, QString("%1").arg(budget.budgetStart().year()));

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


// *** KBudgetView Implementation ***
const int KBudgetView::m_iBudgetYearsAhead = 5;
const int KBudgetView::m_iBudgetYearsBack = 3;

KBudgetView::KBudgetView(QWidget *parent, const char *name ) :
  KBudgetViewDecl(parent,name),
  m_inSelection(false)
{
  m_accountTree->setSorting(-1);

  KIconLoader* il = KGlobal::iconLoader();
  KGuiItem updateButtenItem( i18n("" ),
                             QIconSet(il->loadIcon("button_ok", KIcon::Small, KIcon::SizeSmall)),
                             i18n("Accepts the entered values and stores the budget"),
                             i18n("Use this to store the modified data."));
  m_updateButton->setGuiItem(updateButtenItem);
  KGuiItem resetButtenItem( i18n("" ),
                             QIconSet(il->loadIcon("undo", KIcon::Small, KIcon::SizeSmall)),
                             i18n("Revert budget to last saved state"),
                             i18n("Use this to discard the modified data."));
  m_resetButton->setGuiItem(resetButtenItem);


  connect(m_budgetList, SIGNAL(rightButtonClicked(QListViewItem* , const QPoint&, int)),
    this, SLOT(slotOpenContextMenu(QListViewItem*)));
  connect(m_budgetList, SIGNAL(itemRenamed(QListViewItem*,int,const QString&)), this, SLOT(slotRenameBudget(QListViewItem*,int,const QString&)));
  connect(m_budgetList, SIGNAL(selectionChanged()), this, SLOT(slotSelectBudget()));

  connect(m_cbBudgetSubaccounts, SIGNAL(clicked()), this, SLOT(cb_includesSubaccounts_clicked()));

  connect(m_accountTree, SIGNAL(selectionChanged(QListViewItem*)), this, SLOT(slotSelectAccount(QListViewItem*)));

  // connect the buttons to the actions. Make sure the enabled state
  // of the actions is reflected by the buttons
  connect(m_buttonNewBudget, SIGNAL(clicked()), kmymoney2->action("budget_new"), SLOT(activate()));
  connect(kmymoney2->action("budget_new"), SIGNAL(enabled(bool)), m_buttonNewBudget, SLOT(setEnabled(bool)));
  connect(m_buttonEditBudget, SIGNAL(clicked()), kmymoney2->action("budget_rename"), SLOT(activate()));
  connect(kmymoney2->action("budget_rename"), SIGNAL(enabled(bool)), m_buttonEditBudget, SLOT(setEnabled(bool)));
  connect(m_buttonDeleteBudget, SIGNAL(clicked()), kmymoney2->action("budget_delete"), SLOT(activate()));
  connect(kmymoney2->action("budget_delete"), SIGNAL(enabled(bool)), m_buttonDeleteBudget, SLOT(setEnabled(bool)));

  connect(m_budgetValue, SIGNAL(valuesChanged()), this, SLOT(slotBudgetedAmountChanged()));

  connect(m_updateButton, SIGNAL(pressed()), this, SLOT(slotUpdateBudget()));
  QToolTip::add(m_updateButton, updateButtenItem.toolTip());

  connect(m_resetButton, SIGNAL(pressed()), this, SLOT(slotResetBudget()));
  QToolTip::add(m_resetButton, resetButtenItem.toolTip());

  // setup initial state
  m_buttonNewBudget->setEnabled(kmymoney2->action("budget_new")->isEnabled());
  m_buttonEditBudget->setEnabled(kmymoney2->action("budget_rename")->isEnabled());
  m_buttonDeleteBudget->setEnabled(kmymoney2->action("budget_delete")->isEnabled());

  // make sure to use the previous settings. If no settings are found
  // we use equal distribution of all fields as an initial setting
  // For some reason, if the view is never selected with this code, it
  // stores a value of 32 for the columns. We have to detect that as well.
  m_accountTree->setColumnWidth(0, 0);
  m_accountTree->restoreLayout(KGlobal::config(), "Budget Account View Settings");
  if(m_accountTree->columnWidth(0) < 60) {
    m_accountTree->setResizeMode(QListView::AllColumns);
  }

  connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotRefreshView()));
}

KBudgetView::~KBudgetView()
{
  m_accountTree->saveLayout(KGlobal::config(), "Budget Account View Settings");
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

void KBudgetView::loadBudgets(void)
{
  QCString id;

  ::timetrace("Start KBudgetView::loadBudgets");

  // remember which item is currently selected
  id = m_budget.id();

  // remember the upper left corner of the viewport
  QPoint startPoint = m_budgetList->viewportToContents(QPoint(0, 0));

  // turn off updates to avoid flickering during reload
  m_budgetList->setUpdatesEnabled(false);

  // clear the budget list
  m_budgetList->clear();
  m_budgetValue->clear();

  // add the correct years to the drop down list
  QDate date = QDate::currentDate(Qt::LocalTime);
  int iStartYear = date.year() - m_iBudgetYearsBack;

  m_yearList.clear();
  for (int i=0; i<m_iBudgetYearsAhead + m_iBudgetYearsBack; i++)
    m_yearList += QString::number(iStartYear+i);

  KBudgetListItem* currentItem = 0;

  QValueList<MyMoneyBudget> list = MyMoneyFile::instance()->budgetList();
  QValueList<MyMoneyBudget>::ConstIterator it;
  for (it = list.begin(); it != list.end(); ++it)
  {
    KBudgetListItem* item = new KBudgetListItem(m_budgetList, *it);

    // create a list of unique years
    if (m_yearList.findIndex(QString::number((*it).budgetStart().year())) == -1)
      m_yearList += QString::number((*it).budgetStart().year());

    if(item->budget().id() == id) {
      m_budget = (*it);
      currentItem = item;
      item->setSelected(true);
    }
  }
  m_yearList.sort();

  if (currentItem) {
    m_budgetList->setCurrentItem(currentItem);
  }

  // reposition viewport
  m_budgetList->setContentsPos(startPoint.x(), startPoint.y());

  // turn updates back on
  m_budgetList->setUpdatesEnabled(true);
  m_budgetList->repaintContents();
  m_budgetList->setSorting(-1);

  // reset the status of the buttons
  m_updateButton->setEnabled(false);
  m_resetButton->setEnabled(false);

  // make sure the world around us knows what we have selected
  slotSelectBudget();

  ::timetrace("End KBudgetView::loadBudgets");
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
    if(m_inSelection)
      QTimer::singleShot(0, this, SLOT(slotRefreshView()));
    else
      loadBudgets();
    // the following code seems useless, because if a budget was selected and it
    // is not the first one in the list the one remembered during loadBudget will
    // be forgotten with that code. So I commented it out (ipwizard)
#if 0
    QListViewItem * item = m_budgetList->firstChild();
    m_budgetList->setSelected(item, true);
#endif
  } else {
    m_needReload = true;
  }
}

void KBudgetView::loadAccounts(void)
{
  QMap<QCString, bool> isOpen;

  ::timetrace("start load budget account view");

  // if no budgets are selected, don't load the accounts
  // and clear out the previously shown list
  if (m_budget.id().isEmpty()) {
    m_accountTree->clear();
    m_budgetValue->clear();
    m_updateButton->setEnabled(false);
    m_resetButton->setEnabled(false);
    ::timetrace("done load budgets view");
    return;
  }

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
  m_transactionCountMap.clear();

  // make sure, the pointers are not pointing to some deleted object
  m_incomeItem = m_expenseItem = 0;

  MyMoneyFile* file = MyMoneyFile::instance();

  m_transactionCountMap = file->transactionCountMap();

  m_accountTree->setBaseCurrency(file->baseCurrency());

  bool haveUnusedBudgets = false;

  // create the items
  try {
    const MyMoneySecurity& security = file->baseCurrency();
    m_accountTree->setBaseCurrency(security);

    const MyMoneyAccount& income = file->income();
    QCStringList incSubAcctList = income.accountList();
    m_incomeItem = new KMyMoneyAccountTreeBudgetItem(m_accountTree, income, m_budget, security, i18n("Income"));
    haveUnusedBudgets |= loadSubAccounts(m_incomeItem, incSubAcctList, m_budget);
    m_incomeItem->setSelectable(false);

    const MyMoneyAccount& expense = file->expense();
    QCStringList expSubAcctList = expense.accountList();
    m_expenseItem = new KMyMoneyAccountTreeBudgetItem(m_accountTree, expense, m_budget, security, i18n("Expense"));
    haveUnusedBudgets |= loadSubAccounts(m_expenseItem, expSubAcctList, m_budget);
    m_expenseItem->setSelectable(false);

  } catch(MyMoneyException *e) {
    kdDebug(2) << "Problem in budgetview: " << e->what();
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

  m_updateButton->setEnabled(!(selectedBudget() == m_budget));
  m_resetButton->setEnabled(!(selectedBudget() == m_budget));

  ::timetrace("done load budgets view");
}


bool KBudgetView::loadSubAccounts(KMyMoneyAccountTreeBudgetItem* parent, QCStringList& accountList, const MyMoneyBudget& budget)
{
  MyMoneyFile* file = MyMoneyFile::instance();

  bool unused = false;

  //sort the subaccount list
  //FIXME this is just a hack to order the accounts
  if ( !accountList.isEmpty() ) {
    QMap<QString, MyMoneyAccount> accountMap;
    QValueList<MyMoneyAccount> alist;
    file->accountList ( alist, accountList );
    accountList.clear();
    QValueList<MyMoneyAccount>::const_iterator it_ac;
    for ( it_ac = alist.begin(); it_ac != alist.end(); ++it_ac ) {
      accountMap[(*it_ac).name()] = *it_ac;
    }
    QMap<QString, MyMoneyAccount>::const_iterator it_am;
    for ( it_am = accountMap.begin(); it_am != accountMap.end(); ++it_am ) {
      accountList.prepend((*it_am).id()); //use prepend instead of append otherwise account show up in ascending order
    }
  }

  QCStringList::const_iterator it_a;
  for(it_a = accountList.begin(); it_a != accountList.end(); ++it_a) {
    const MyMoneyAccount& acc = file->account(*it_a);
    QValueList<MyMoneyPrice> prices;
    MyMoneySecurity security = file->baseCurrency();
    try {
      if(acc.isInvest()) {
        security = file->security(acc.currencyId());
        prices += file->price(acc.currencyId(), security.tradingCurrency());
        if(security.tradingCurrency() != file->baseCurrency().id()) {
          MyMoneySecurity sec = file->security(security.tradingCurrency());
          prices += file->price(sec.id(), file->baseCurrency().id());
        }
      } else if(acc.currencyId() != file->baseCurrency().id()) {
        if(acc.currencyId() != file->baseCurrency().id()) {
          security = file->security(acc.currencyId());
          prices += file->price(acc.currencyId(), file->baseCurrency().id());
        }
      }

    } catch(MyMoneyException *e) {
      kdDebug(2) << __PRETTY_FUNCTION__ << " caught exception while adding " << acc.name() << "[" << acc.id() << "]: " << e->what();
      delete e;
    }

    QCStringList subAcctList = acc.accountList();
    KMyMoneyAccountTreeBudgetItem *item = new KMyMoneyAccountTreeBudgetItem(parent, acc, budget, prices, security);
    unused |= loadSubAccounts(item, subAcctList, budget);

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

void KBudgetView::slotSelectBudget(void)
{
  // check if the content of a currently selected payee was modified
  // and ask to store the data
  if (m_updateButton->isEnabled()) {
    if (KMessageBox::questionYesNo(this, QString("<qt>%1</qt>").arg(
      i18n("Do you want to save the changes for <b>%1</b>").arg(m_budget.name())),
      i18n("Save changes")) == KMessageBox::Yes) {
        m_inSelection = true;
        slotUpdateBudget();
        m_inSelection = false;
    }
  }

  KBudgetListItem* item;
  if (m_budget.id().isEmpty()) {
    item = dynamic_cast<KBudgetListItem*>(m_budgetList->firstChild());
    if(item) {
      m_budgetList->blockSignals(true);
      m_budgetList->setSelected(item, true);
      m_budgetList->blockSignals(false);
    }
  }

  m_accountTree->setEnabled(false);
  m_assignmentBox->setEnabled(false);
  m_budget = MyMoneyBudget();

  QListViewItemIterator it_l(m_budgetList, QListViewItemIterator::Selected);
  item = dynamic_cast<KBudgetListItem*>(it_l.current());
  if(item) {
    m_budget = item->budget();
    m_accountTree->setEnabled(true);
  }

  loadAccounts();

  QValueList<MyMoneyBudget> budgetList;
  if(!m_budget.id().isEmpty())
    budgetList << m_budget;
  emit selectObjects(budgetList);
}

const MyMoneyBudget& KBudgetView::selectedBudget(void) const
{
  static MyMoneyBudget nullBudget;

  QListViewItemIterator it_l(m_budgetList, QListViewItemIterator::Selected);
  KBudgetListItem* item = dynamic_cast<KBudgetListItem*>(it_l.current());
  if(item) {
    return item->budget();
  }

  return nullBudget;
}

KMyMoneyAccountTreeBudgetItem* KBudgetView::selectedAccount(void) const
{
  QListViewItemIterator it_l(m_accountTree, QListViewItemIterator::Selected);
  KMyMoneyAccountTreeBudgetItem* item = dynamic_cast<KMyMoneyAccountTreeBudgetItem*>(it_l.current());
  return item;
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
    MyMoneyFileTransaction ft;
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

      MyMoneyBudget b = pBudget->budget();
      b.setName(new_name);
      // don't use pBudget beyond this point as it will change due to call to modifyBudget
      pBudget = 0;

      MyMoneyFile::instance()->modifyBudget(b);

      // the above call to modifyBudget will reload the view so
      // all references and pointers to the view have to be
      // re-established. You cannot use pBudget beyond this point!!!
      ft.commit();

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

void KBudgetView::slotSelectAccount(QListViewItem* item)
{
  if(item->listView() == m_accountTree) {
    m_assignmentBox->setEnabled(false);
    KMyMoneyAccountTreeBudgetItem *account = selectedAccount();
    m_assignmentBox->setEnabled(account != 0);

    if(account) {
      if (m_budget.id().isEmpty() )
        return;

      QCString id = account->id();
      m_leAccounts->setText(MyMoneyFile::instance()->accountToCategory(id));
      m_cbBudgetSubaccounts->setChecked(m_budget.account(id).budgetSubaccounts());
      m_accountTotal->setValue(m_budget.account(id).totalBalance());

      MyMoneyBudget::AccountGroup budgetAccount = m_budget.account( id );
      if ( id != budgetAccount.id() ) {
        budgetAccount.setBudgetLevel(MyMoneyBudget::AccountGroup::eMonthly);
      }
      m_budgetValue->setBudgetValues(m_budget, budgetAccount);
    }
  }
}

void KBudgetView::slotBudgetedAmountChanged(void)
{
  if (m_budget.id().isEmpty())
    return;

  KMyMoneyAccountTreeBudgetItem *account;
  if ((account=selectedAccount()) == NULL)
    return;

  MyMoneyBudget::AccountGroup accountGroup = m_budget.account(account->id());
  accountGroup.setId( account->id() );
  m_budgetValue->budgetValues(m_budget, accountGroup);
  m_budget.setAccount(accountGroup, account->id());

  loadAccounts();
}

void KBudgetView::AccountEnter()
{
  if (m_budget.id().isEmpty())
    return;

  //(ace) kCategoryWidget not currently defined
  KMyMoneyAccountTreeBudgetItem *item = NULL; //dynamic_cast<KMyMoneyAccountTreeBudgetItem*> (m_accountTree->findItem(m_leAccounts->selectedAccountId()));
  if (item)
  {
    m_accountTree->setCurrentItem(item);
    m_accountTree->setOpen(item, true);
  }
}


void KBudgetView::cb_includesSubaccounts_clicked()
{
  if (m_budget.id().isEmpty())
    return;

  if(selectedAccount() != 0) {
    QCString accountID = selectedAccount()->id();
    // now, we get a reference to the accountgroup, to mofify its atribute,
    // and then put the resulting account group instead of the original

    MyMoneyBudget::AccountGroup auxAccount = m_budget.account(accountID);
    auxAccount.setBudgetSubaccounts( m_cbBudgetSubaccounts->isChecked());
    m_budget.setAccount( auxAccount, accountID);

    loadAccounts();
  }
}

void KBudgetView::slotResetBudget(void)
{
  try {
    m_budget = MyMoneyFile::instance()->budget(m_budget.id());
    loadAccounts();
  } catch(MyMoneyException *e) {
    KMessageBox::detailedSorry(0, i18n("Unable to reset budget"),
                               (e->what() + " " + i18n("thrown in") + " " + e->file()+ ":%1").arg(e->line()));
    delete e;
  }
}

void KBudgetView::slotUpdateBudget(void)
{
  MyMoneyFileTransaction ft;
  try {
    MyMoneyFile::instance()->modifyBudget(m_budget);
    ft.commit();
  } catch(MyMoneyException *e) {
    KMessageBox::detailedSorry(0, i18n("Unable to modify budget"),
                               (e->what() + " " + i18n("thrown in") + " " + e->file()+ ":%1").arg(e->line()));
    delete e;
  }
}

#include "kbudgetview.moc"
