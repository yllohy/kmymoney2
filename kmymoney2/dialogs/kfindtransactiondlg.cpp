/***************************************************************************
                          kfindtransactiondlg.cpp
                             -------------------
    copyright            : (C) 2003 by Thomas Baumgart
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
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qtimer.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kglobal.h>
#include <klocale.h>
#if QT_VERSION > 300
#include <kstandarddirs.h>
#else
#include <kstddirs.h>
#endif

#include <kpushbutton.h>
#include <klineedit.h>
#include <klistview.h>
#include <kcombobox.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kfindtransactiondlg.h"
#include "../widgets/kmymoneyregistersearch.h"
#include "../widgets/kmymoneydateinput.h"
#include "../widgets/kmymoneyedit.h"
#include "../widgets/kmymoneylineedit.h"
#include "../mymoney/mymoneyfile.h"
#include "../mymoney/storage/imymoneystorage.h"

KMyMoneyCheckListItem::KMyMoneyCheckListItem(QListView* parent, const QString& txt, Type type, const QCString& id, KFindTransactionDlg* dlg) :
  QCheckListItem(parent, txt, type),
  m_id(id),
  m_dlg(dlg)
{
}

KMyMoneyCheckListItem::KMyMoneyCheckListItem(QListViewItem* parent, const QString& txt, Type type, const QCString& id, KFindTransactionDlg* dlg) :
  QCheckListItem(parent, txt, type),
  m_id(id),
  m_dlg(dlg)
{
}

KMyMoneyCheckListItem::~KMyMoneyCheckListItem()
{
}

void KMyMoneyCheckListItem::stateChange(bool)
{
  m_dlg->slotUpdateSelections();
}

KFindTransactionDlg::KFindTransactionDlg(QWidget *parent, const char *name)
 : KFindTransactionDlgDecl(parent, name, false),
  m_transactionPtr(0)
{
  // hide the transaction register and make sure the dialog is
  // displayed as small as can be. We make sure that the larger
  // version (with the transaction register) will also fit on the screen
  // by moving the dialog by (-45,-45).
  m_register->setParent(this);
  m_registerFrame->hide();
  update();
  resize(minimumSizeHint());
  
  move(x()-45, y()-45);

  setupAccountsPage();
  setupCategoriesPage();
  setupDatePage();
  setupAmountPage();
  setupPayeesPage();
  setupDetailsPage();
      
  // readConfig();
  slotUpdateSelections();
  
  QTimer::singleShot(0, this, SLOT(slotRightSize()));

  // setup the connections
  connect(m_searchButton, SIGNAL(clicked()), this, SLOT(slotSearch()));
  connect(m_resetButton, SIGNAL(clicked()), this, SLOT(slotReset()));
  connect(m_closeButton, SIGNAL(clicked()), this, SLOT(deleteLater()));
  connect(m_textEdit, SIGNAL(textChanged(const QString&)), this, SLOT(slotUpdateSelections()));
}

KFindTransactionDlg::~KFindTransactionDlg()
{
  // writeConfig();
}

void KFindTransactionDlg::slotReset(void)
{
  m_textEdit->setText("");
  m_regExp->setChecked(false);
  m_caseSensitive->setChecked(false);
  
  selectAllItems(m_accountsView, true);

  m_amountEdit->setEnabled(true);
  m_amountFromEdit->setEnabled(false);
  m_amountToEdit->setEnabled(false);
  m_amountEdit->loadText("");
  m_amountFromEdit->loadText("");
  m_amountToEdit->loadText("");
  m_amountButton->setChecked(true);
  m_amountRangeButton->setChecked(false);

  selectAllItems(m_categoriesView, true);

  m_emptyPayeesButton->setChecked(false);
  selectAllItems(m_payeesView, true);

  m_typeBox->setCurrentItem(MyMoneyTransactionFilter::allTypes);
  m_stateBox->setCurrentItem(MyMoneyTransactionFilter::allStates);
  m_nrEdit->setEnabled(true);
  m_nrFromEdit->setEnabled(false);
  m_nrToEdit->setEnabled(false);
  m_nrEdit->setText("");
  m_nrFromEdit->setText("");
  m_nrToEdit->setText("");
  m_nrButton->setChecked(true);
  m_nrRangeButton->setChecked(false);
      
  m_registerFrame->hide();
  
  // the following call implies a call to slotUpdateSelections,
  // that's why we call it last
  m_dateRange->setCurrentItem(allDates);        
  slotDateRangeChanged(allDates);

  QTimer::singleShot(0, this, SLOT(slotRightSize()));
}

void KFindTransactionDlg::slotRightSize(void)
{
  resize(minimumSize());
}

void KFindTransactionDlg::slotUpdateSelections(void)
{
  QString txt = "";

  // Text tab
  if(!m_textEdit->text().isEmpty()) {
    if(!txt.isEmpty())
      txt += ", ";
    txt += i18n("Text");
    m_regExp->setEnabled(QRegExp(m_textEdit->text()).isValid());
  } else
    m_regExp->setEnabled(false);
    
  m_caseSensitive->setEnabled(!m_textEdit->text().isEmpty());
    
  // Account tab
  if(!allItemsSelected(m_accountsView)) {
    if(!txt.isEmpty())
      txt += ", ";
    txt += i18n("Account");
  }

  // Date tab
  if(m_dateRange->currentItem() != 0) {
    if(!txt.isEmpty())
      txt += ", ";
    txt += i18n("Date");
  }

  // Amount tab
  if((m_amountButton->isChecked() && m_amountEdit->isValid())
  || (m_amountRangeButton->isChecked()
      && (m_amountFromEdit->isValid() || m_amountToEdit->isValid()))) {
    if(!txt.isEmpty())
      txt += ", ";
    txt += i18n("Amount");
  }

  // Categories tab
  if(!allItemsSelected(m_categoriesView)) {
    if(!txt.isEmpty())
      txt += ", ";
    txt += i18n("Category");
  }

  // Payees tab
  if(!allItemsSelected(m_payeesView)
  || m_emptyPayeesButton->isChecked()) {
    if(!txt.isEmpty())
      txt += ", ";
    txt += i18n("Payees");
  }
  m_payeesView->setEnabled(!m_emptyPayeesButton->isChecked());
  
  // Details tab
  if(m_typeBox->currentItem() != 0
  || m_stateBox->currentItem() != 0
  || (m_nrButton->isChecked() && m_nrEdit->text().length() != 0)
  || (m_nrRangeButton->isChecked()
     && (m_nrFromEdit->text().length() != 0 || m_nrToEdit->text().length() != 0))) {
    if(!txt.isEmpty())
      txt += ", ";
    txt += i18n("Details");
  }
  
  if(txt.isEmpty()) {
    txt = i18n("(None)");
  }
  m_selectedCriteria->setText(i18n("Current selections: ") + txt);
}

const bool KFindTransactionDlg::allItemsSelected(const QListViewItem *item) const
{
  QListViewItem* it_v;
  
  for(it_v = item->firstChild(); it_v != 0; it_v = it_v->nextSibling()) {
    if(it_v->rtti() == 1) {
      QCheckListItem* it_c = static_cast<QCheckListItem*>(it_v);
      if(!(it_c->isOn() && allItemsSelected(it_v)))
        return false;
    }
  }
  return true;
}

const bool KFindTransactionDlg::allItemsSelected(const QListView* view) const
{
  QListViewItem* it_v;
  
  for(it_v = view->firstChild(); it_v != 0; it_v = it_v->nextSibling()) {
    if(it_v->rtti() == 1) {
      QCheckListItem* it_c = static_cast<QCheckListItem*>(it_v);
      if(it_c->type() == QCheckListItem::CheckBox) {
        if(!(it_c->isOn() && allItemsSelected(it_v)))
          return false;
      } else {
        if(!allItemsSelected(it_v))
          return false;
      }
    }
  }
  return true;
}

void KFindTransactionDlg::loadAccounts(void)
{
  QValueList<MyMoneyAccount> list;
  QValueList<MyMoneyAccount>::Iterator it_l;
    
  KMyMoneyCheckListItem* asset = new KMyMoneyCheckListItem(m_accountsView, i18n("Asset accounts"), QCheckListItem::Controller, "", this);
  asset->setSelectable(false);
  asset->setOpen(true);
  
  KMyMoneyCheckListItem* liability = new KMyMoneyCheckListItem(m_accountsView, i18n("Liability accounts"), QCheckListItem::Controller, "", this);
  liability->setSelectable(false);
  liability->setOpen(true);
  
  list = MyMoneyFile::instance()->accountList();
  // scan all accounts found in the engine
  for(it_l = list.begin(); it_l != list.end(); ++it_l) {
    KMyMoneyCheckListItem* parent;
    
    // don't include expense and income records here
    switch((*it_l).accountType()) {
      case MyMoneyAccount::Expense:
      case MyMoneyAccount::Income:
        break;
        
      default:
        // determine parent
        if(MyMoneyFile::instance()->accountGroup((*it_l).accountType()) == MyMoneyAccount::Asset)
          parent = asset;
        else
          parent = liability;

        KMyMoneyCheckListItem* item = new KMyMoneyCheckListItem(parent, (*it_l).name(), QCheckListItem::CheckBox, (*it_l).id(), this);
        item->setOn(true);
        if((*it_l).accountList().count() > 0) {
          item->setOpen(true);
          loadSubAccounts(item, (*it_l).accountList());
        }
        break;
    }
  }
/*  This code might become important once we reload the lists  
  // check if there are accounts in the listview that are no longer
  // in the engine. When found, erase from view.
  QListViewItem* it_v;
  for(it_v = m_accountsView->firstChild(); it_v != 0; ) {
    try {
      MyMoneyFile::instance()->account(static_cast<KMyMoneyCheckListItem*>(it_v)->accountId());
      it_v = it_v->nextSibling();
    } catch(MyMoneyException *e) {
      delete e;
      QListViewItem* it_d = it_v;
      it_v = it_v->nextSibling();
      delete it_d;
    }
  }
*/  
}

void KFindTransactionDlg::setupAccountsPage(void)
{
  m_accountsView->setSelectionMode(QListView::Single);
  m_accountsView->header()->hide();
  loadAccounts();

  connect(m_allAccountsButton, SIGNAL(clicked()), this, SLOT(slotSelectAllAccounts()));
  connect(m_noAccountButton, SIGNAL(clicked()), this, SLOT(slotDeselectAllAccounts()));
}

void KFindTransactionDlg::slotSelectAllAccounts(void)
{
  selectAllItems(m_accountsView, true);  
}

void KFindTransactionDlg::slotDeselectAllAccounts(void)
{
  selectAllItems(m_accountsView, false);
}

void KFindTransactionDlg::selectAllItems(QListView* view, const bool state)
{
  QListViewItem* it_v;

  for(it_v = view->firstChild(); it_v != 0; it_v = it_v->nextSibling()) {
    QCheckListItem* it_c = static_cast<QCheckListItem*>(it_v);
    if(it_c->type() == QCheckListItem::CheckBox) {
      it_c->setOn(state);
    }
    selectAllSubItems(it_v, state);
  }  

  slotUpdateSelections();
}

void KFindTransactionDlg::setupCategoriesPage(void)
{
  m_categoriesView->setSelectionMode(QListView::Single);
  m_categoriesView->header()->hide();
  
  loadCategories();

  connect(m_allCategoriesButton, SIGNAL(clicked()), this, SLOT(slotSelectAllCategories()));
  connect(m_noCategoryButton, SIGNAL(clicked()), this, SLOT(slotDeselectAllCategories()));
  connect(m_expenseCategoriesButton, SIGNAL(clicked()), this, SLOT(slotSelectExpenseCategories()));
  connect(m_incomeCategoriesButton, SIGNAL(clicked()), this, SLOT(slotSelectIncomeCategories()));
}

void KFindTransactionDlg::loadSubAccounts(QListViewItem* parent, const QCStringList& list)
{
  QCStringList::ConstIterator it_l;
  MyMoneyFile* file = MyMoneyFile::instance();
  
  for(it_l = list.begin(); it_l != list.end(); ++it_l) {
    MyMoneyAccount acc = file->account(*it_l);
    KMyMoneyCheckListItem* item = new KMyMoneyCheckListItem(parent, acc.name(), QCheckListItem::CheckBox, acc.id(), this);
    item->setOn(true);
    if(acc.accountList().count() > 0) {
      item->setOpen(true);
      loadSubAccounts(item, acc.accountList());
    }
  }
}

void KFindTransactionDlg::loadCategories(void)
{
  QCStringList list;
  QCStringList::ConstIterator it_l;
  MyMoneyFile* file = MyMoneyFile::instance();
  
  KMyMoneyCheckListItem* expenses = new KMyMoneyCheckListItem(m_categoriesView, i18n("Expense categories"), QCheckListItem::Controller, "", this);
  expenses->setSelectable(false);
  expenses->setOpen(true);
  
  list = file->expense().accountList();
  // scan all expense accounts found in the engine
  for(it_l = list.begin(); it_l != list.end(); ++it_l) {
    MyMoneyAccount acc = file->account(*it_l);
    KMyMoneyCheckListItem* item = new KMyMoneyCheckListItem(expenses, acc.name(), QCheckListItem::CheckBox, acc.id(), this);
    item->setOn(true);
    if(acc.accountList().count() > 0) {
      item->setOpen(true);
      loadSubAccounts(item, acc.accountList());
    }
  }

  KMyMoneyCheckListItem* income = new KMyMoneyCheckListItem(m_categoriesView, i18n("Income categories"), QCheckListItem::Controller, "", this);
  income->setSelectable(false);
  income->setOpen(true);
  
  list = file->income().accountList();
  // scan all income accounts found in the engine
  for(it_l = list.begin(); it_l != list.end(); ++it_l) {
    MyMoneyAccount acc = file->account(*it_l);
    KMyMoneyCheckListItem* item = new KMyMoneyCheckListItem(income, acc.name(), QCheckListItem::CheckBox, acc.id(), this);
    item->setOn(true);
    if(acc.accountList().count() > 0) {
      item->setOpen(true);
      loadSubAccounts(item, acc.accountList());
    }
  }
}

void KFindTransactionDlg::slotSelectAllCategories(void)
{
  selectAllCategories(true, true);
}

void KFindTransactionDlg::slotDeselectAllCategories(void)
{
  selectAllCategories(false, false);
}

void KFindTransactionDlg::slotSelectIncomeCategories(void)
{
  selectAllCategories(true, false);
}

void KFindTransactionDlg::slotSelectExpenseCategories(void)
{
  selectAllCategories(false, true);
}

void KFindTransactionDlg::selectAllSubItems(QListViewItem* item, const bool state)
{
  QListViewItem* it_v;

  for(it_v = item->firstChild(); it_v != 0; it_v = it_v->nextSibling()) {
    static_cast<QCheckListItem*>(it_v)->setOn(state);
    selectAllSubItems(it_v, state);
  }
}

void KFindTransactionDlg::selectAllCategories(const bool income, const bool expense)
{
  QListViewItem* it_v;
  
  for(it_v = m_categoriesView->firstChild(); it_v != 0; it_v = it_v->nextSibling()) {
    if(static_cast<QCheckListItem*>(it_v)->text() == i18n("Income categories"))
      selectAllSubItems(it_v, income);
    else
      selectAllSubItems(it_v, expense);
  }
  
  slotUpdateSelections();
}

void KFindTransactionDlg::setupDatePage(void)
{
  int yr, mon, day;
  yr = QDate::currentDate().year();
  mon = QDate::currentDate().month();
  day = QDate::currentDate().day();

  m_startDates[allDates] = QDate();
  m_startDates[untilToday] = QDate();
  m_startDates[currentMonth] = QDate(yr,mon,1);
  m_startDates[currentYear] = QDate(yr,1,1);
  m_startDates[monthToDate] = QDate(yr,mon,1);
  m_startDates[yearToDate] = QDate(yr,1,1);
  m_startDates[lastMonth] = QDate(yr,mon,1).addMonths(-1);
  m_startDates[lastYear] = QDate(yr,1,1).addYears(-1);
  m_startDates[last30Days] = QDate::currentDate().addDays(-30);
  m_startDates[last3Months] = QDate::currentDate().addMonths(-3);
  m_startDates[last6Months] = QDate::currentDate().addMonths(-6);
  m_startDates[last12Months] = QDate::currentDate().addMonths(-12);
  m_startDates[next30Days] = QDate::currentDate();
  m_startDates[next3Months] = QDate::currentDate();
  m_startDates[next6Months] = QDate::currentDate();
  m_startDates[next12Months] = QDate::currentDate();
  m_startDates[userDefined] = QDate();
  
  m_endDates[allDates] = QDate();  
  m_endDates[untilToday] = QDate::currentDate();
  m_endDates[currentMonth] = QDate(yr,mon,1).addMonths(1).addDays(-1);
  m_endDates[currentYear] = QDate(yr,12,31);
  m_endDates[monthToDate] = QDate::currentDate();
  m_endDates[yearToDate] = QDate::currentDate();
  m_endDates[lastMonth] = QDate(yr,mon,1).addDays(-1);
  m_endDates[lastYear] = QDate(yr,12,31).addYears(-1);
  m_endDates[last30Days] = QDate::currentDate();
  m_endDates[last3Months] = QDate::currentDate();
  m_endDates[last6Months] = QDate::currentDate();
  m_endDates[last12Months] = QDate::currentDate();
  m_endDates[next30Days] = QDate::currentDate().addDays(30);
  m_endDates[next3Months] = QDate::currentDate().addMonths(3);
  m_endDates[next6Months] = QDate::currentDate().addMonths(6);
  m_endDates[next12Months] = QDate::currentDate().addMonths(12);
  m_endDates[userDefined] = QDate();

  connect(m_dateRange, SIGNAL(activated(int)), this, SLOT(slotDateRangeChanged(int)));
  connect(m_fromDate, SIGNAL(dateChanged(const QDate&)), this, SLOT(slotDateChanged()));
  connect(m_toDate, SIGNAL(dateChanged(const QDate&)), this, SLOT(slotDateChanged()));
  
  slotDateRangeChanged(allDates);
}

void KFindTransactionDlg::slotDateRangeChanged(int idx)
{
  switch(idx) {
    case allDates:
    case userDefined:
      m_fromDate->loadDate(QDate());
      m_toDate->loadDate(QDate());
      break;
    default:
      m_fromDate->loadDate(m_startDates[idx]);
      m_toDate->loadDate(m_endDates[idx]);
      break;
  }
  slotUpdateSelections();
}

void KFindTransactionDlg::slotDateChanged(void)
{
  int idx;
  for(idx = untilToday; idx < userDefined; ++idx) {
    if(m_fromDate->getQDate() == m_startDates[idx]
    && m_toDate->getQDate() == m_endDates[idx]) {
      break;
    }
  }
  m_dateRange->setCurrentItem(idx);
  slotUpdateSelections();
}

void KFindTransactionDlg::setupAmountPage(void)
{
  connect(m_amountButton, SIGNAL(clicked()), this, SLOT(slotAmountSelected()));
  connect(m_amountRangeButton, SIGNAL(clicked()), this, SLOT(slotAmountRangeSelected()));
  
  connect(m_amountEdit, SIGNAL(textChanged(const QString&)), this, SLOT(slotUpdateSelections()));
  connect(m_amountFromEdit, SIGNAL(textChanged(const QString&)), this, SLOT(slotUpdateSelections()));
  connect(m_amountToEdit, SIGNAL(textChanged(const QString&)), this, SLOT(slotUpdateSelections()));

  m_amountButton->setChecked(true);
  slotAmountSelected();  
}

void KFindTransactionDlg::slotAmountSelected(void)
{
  m_amountEdit->setEnabled(true);
  m_amountFromEdit->setEnabled(false);
  m_amountToEdit->setEnabled(false);
  slotUpdateSelections();
}

void KFindTransactionDlg::slotAmountRangeSelected(void)
{
  m_amountEdit->setEnabled(false);
  m_amountFromEdit->setEnabled(true);
  m_amountToEdit->setEnabled(true);
  slotUpdateSelections();
}

void KFindTransactionDlg::setupPayeesPage(void)
{
  m_accountsView->setSelectionMode(QListView::Single);
  m_payeesView->header()->hide();

  loadPayees();  
  m_emptyPayeesButton->setChecked(false);
  
  connect(m_allPayeesButton, SIGNAL(clicked()), this, SLOT(slotSelectAllPayees()));
  connect(m_clearPayeesButton, SIGNAL(clicked()), this, SLOT(slotDeselectAllPayees()));
  connect(m_emptyPayeesButton, SIGNAL(stateChanged(int)), this, SLOT(slotUpdateSelections()));
}

void KFindTransactionDlg::loadPayees(void)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  QValueList<MyMoneyPayee> list;
  QValueList<MyMoneyPayee>::Iterator it_l;

  list = file->payeeList();
  // load view
  for(it_l = list.begin(); it_l != list.end(); ++it_l) {
    KMyMoneyCheckListItem* item = new KMyMoneyCheckListItem(m_payeesView, (*it_l).name(), QCheckListItem::CheckBox, (*it_l).id(), this);
    item->setOn(true);
  }
}
void KFindTransactionDlg::slotSelectAllPayees(void)
{
  selectAllItems(m_payeesView, true);
}

void KFindTransactionDlg::slotDeselectAllPayees(void)
{
  selectAllItems(m_payeesView, false);
}

void KFindTransactionDlg::setupDetailsPage(void)
{
  connect(m_typeBox, SIGNAL(activated(int)), this, SLOT(slotUpdateSelections()));
  connect(m_stateBox, SIGNAL(activated(int)), this, SLOT(slotUpdateSelections()));
  
  connect(m_nrButton, SIGNAL(clicked()), this, SLOT(slotNrSelected()));
  connect(m_nrRangeButton, SIGNAL(clicked()), this, SLOT(slotNrRangeSelected()));
  connect(m_nrEdit, SIGNAL(textChanged(const QString&)), this, SLOT(slotUpdateSelections()));
  connect(m_nrFromEdit, SIGNAL(textChanged(const QString&)), this, SLOT(slotUpdateSelections()));
  connect(m_nrToEdit, SIGNAL(textChanged(const QString&)), this, SLOT(slotUpdateSelections()));

  m_nrButton->setChecked(true);
  slotNrSelected();
}

void KFindTransactionDlg::slotNrSelected(void)
{
  m_nrEdit->setEnabled(true);
  m_nrFromEdit->setEnabled(false);
  m_nrToEdit->setEnabled(false);
  slotUpdateSelections();
}

void KFindTransactionDlg::slotNrRangeSelected(void)
{
  m_nrEdit->setEnabled(false);
  m_nrFromEdit->setEnabled(true);
  m_nrToEdit->setEnabled(true);
  slotUpdateSelections();
}

void KFindTransactionDlg::addItemToFilter(const opTypeE op, const QCString& id)
{
  switch(op) {
    case addAccountToFilter:
      m_filter.addAccount(id);
      break;
    case addCategoryToFilter:
      m_filter.addCategory(id);
      break;
    case addPayeeToFilter:
      m_filter.addPayee(id);
      break;
  }
}

void KFindTransactionDlg::scanCheckListItems(QListViewItem* item, const opTypeE op)
{
  QListViewItem* it_v;

  for(it_v = item->firstChild(); it_v != 0; it_v = it_v->nextSibling()) {
    if(it_v->rtti() == 1) {
      KMyMoneyCheckListItem* it_c = static_cast<KMyMoneyCheckListItem*>(it_v);
      if(it_c->type() == QCheckListItem::CheckBox) {
        if(it_c->isOn())
          addItemToFilter(op, (*it_c).accountId());
      }
      scanCheckListItems(it_v, op);
    }
  }
}

void KFindTransactionDlg::scanCheckListItems(QListView* view, const opTypeE op)
{
  QListViewItem* it_v;

  for(it_v = view->firstChild(); it_v != 0; it_v = it_v->nextSibling()) {
    if(it_v->rtti() == 1) {
      KMyMoneyCheckListItem* it_c = static_cast<KMyMoneyCheckListItem*>(it_v);
      if(it_c->type() == QCheckListItem::CheckBox) {
        if(it_c->isOn())
          addItemToFilter(op, (*it_c).accountId());
      }
      scanCheckListItems(it_v, op);
    }
  }
}

void KFindTransactionDlg::slotSearch(void)
{
  // setup the filter from the dialog widgets
  m_filter.clear();
  
  // Text tab
  if(!m_textEdit->text().isEmpty()) {
    QRegExp exp(m_textEdit->text(), m_caseSensitive->isChecked(), !m_regExp->isChecked());
    m_filter.setTextFilter(exp);
  }
  
  // Account tab
  if(!allItemsSelected(m_accountsView)) {
    scanCheckListItems(m_accountsView, addAccountToFilter);
  }

  // Date tab
  if(m_dateRange->currentItem() != 0) {
    m_filter.setDateFilter(m_fromDate->getQDate(), m_toDate->getQDate());
  }
  
  // Amount tab
  if((m_amountButton->isChecked() && m_amountEdit->isValid())) {
    m_filter.setAmountFilter(m_amountEdit->getMoneyValue(), m_amountEdit->getMoneyValue());
    
  } else if((m_amountRangeButton->isChecked()
      && (m_amountFromEdit->isValid() || m_amountToEdit->isValid()))) {
    
    MyMoneyMoney from(MyMoneyMoney::minValue), to(MyMoneyMoney::maxValue);
    if(m_amountFromEdit->isValid())
      from = m_amountFromEdit->getMoneyValue();
    if(m_amountToEdit->isValid())
      to = m_amountToEdit->getMoneyValue();
      
    m_filter.setAmountFilter(from, to);
  }
  
  // Categories tab
  scanCheckListItems(m_accountsView, addCategoryToFilter);
  
  // Payees tab
  if(m_emptyPayeesButton->isChecked()) {
    m_filter.addPayee(QCString());
    
  } else if(!allItemsSelected(m_payeesView)) {
    scanCheckListItems(m_payeesView, addPayeeToFilter);
  }
  
  // Details tab
  if(m_typeBox->currentItem() != 0)
    m_filter.addType(m_typeBox->currentItem());

  if(m_stateBox->currentItem() != 0)
    m_filter.addState(m_stateBox->currentItem());

  if(m_nrButton->isChecked() && !m_nrEdit->text().isEmpty())
    m_filter.setNumberFilter(m_nrEdit->text(), m_nrEdit->text());

  if(m_nrRangeButton->isChecked()
     && (!m_nrFromEdit->text().isEmpty() || !m_nrToEdit->text().isEmpty())) {
    m_filter.setNumberFilter(m_nrFromEdit->text(), m_nrToEdit->text());
  }


  // filter is setup, now fill the register
  slotRefreshView();
}

void KFindTransactionDlg::slotRefreshView(void)
{
  try {
    QValueList<MyMoneyTransaction> list = MyMoneyFile::instance()->transactionList(m_filter);
    QValueList<MyMoneyTransaction>::ConstIterator it;
    
    m_transactionList.clear();
    QCString lastId;
    int ofs = 0;
    
    for(it = list.begin(); it != list.end(); ++it) {
      KMyMoneyTransaction k(*it);
      m_filter.match(*it, MyMoneyFile::instance()->storage());
      if(lastId != (*it).id()) {
        ofs = 0;
        lastId = (*it).id();
      } else
        ofs++;
        
      k.setSplitId(m_filter.matchingSplits()[ofs].id());
      m_transactionList.append(k);
    }
  } catch(MyMoneyException *e) {
    delete e;
    return;
  }

  QValueList<KMyMoneyTransaction>::ConstIterator it_t;

  // setup the pointer vector
  m_transactionPtrVector.clear();
  m_transactionPtrVector.resize(m_transactionList.size());

  int i;
  for(i = 0, it_t = m_transactionList.begin(); it_t != m_transactionList.end(); ++it_t) {
    m_transactionPtrVector.insert(i, &(*it_t));
    ++i;
  }

  // sort the transactions
  m_transactionPtrVector.sort();

  bool dateMarkPlaced = false;
  m_register->setCurrentDateIndex();    // turn off date mark

  try {
    // the trick is to go backwards ;-)

    while(--i >= 0) {
      if(m_transactionPtrVector.sortType() == KTransactionPtrVector::SortPostDate) {
        if(m_transactionPtrVector[i]->postDate() > QDate::currentDate()) {
          m_register->setCurrentDateIndex(i+1);

        } else if(dateMarkPlaced == false) {
          m_register->setCurrentDateIndex(i+1);
          dateMarkPlaced = true;
        }
      }
    }
  } catch(MyMoneyException *e) {
    qWarning("Unexpected exception in KFindTransactionDlg::refreshView");
    delete e;
    return;
  }
  m_foundText->setText(i18n(QString("Found %1 matching transactions")
                      .arg(m_transactionPtrVector.count())));
  m_register->setTransactionCount(m_transactionPtrVector.count());
  m_register->setCurrentTransactionIndex(0);
  m_registerFrame->show();
}

KMyMoneyTransaction* KFindTransactionDlg::transaction(const int idx) const
{
  if(idx >= 0 && static_cast<unsigned> (idx) < m_transactionPtrVector.count())
    return m_transactionPtrVector[idx];
  return 0;
}

const QCString KFindTransactionDlg::accountId(const MyMoneyTransaction * const transaction, int match) const
{
  QValueList<MyMoneySplit>::ConstIterator it;
  MyMoneyFile* file = MyMoneyFile::instance();
  MyMoneyTransactionFilter filter = m_filter;
  
  if(filter.match(*transaction, file->storage())) {
    for(it = filter.matchingSplits().begin(); it != filter.matchingSplits().end(); ++it) {
      MyMoneyAccount acc = file->account((*it).accountId());
      switch(acc.accountGroup()) {
        case MyMoneyAccount::Income:
        case MyMoneyAccount::Expense:
          break;
        default:
          if(!match--)
            return (*it).accountId();
          break;
      }
    }
  }
  qFatal("KFindTransactionDlg::accountId(): No asset/liability account for transaction. This usually crashes");
  return QCString();  
}

void KFindTransactionDlg::resizeEvent(QResizeEvent* /* ev */)
{
  if(!m_register->isVisible())
    return;
    
  // resize the register
  int w = m_register->visibleWidth();

  int m_debitWidth = 80;
  int m_creditWidth = 80;

  m_register->setColumnWidth(0, 80);

  // Resize the date field to the size required by the input widget
  kMyMoneyDateInput* datefield = new kMyMoneyDateInput();
  datefield->setFont(m_register->cellFont());
  m_register->setColumnWidth(1, datefield->minimumSizeHint().width());
  delete datefield;

  m_register->setColumnWidth(5, m_debitWidth);
  m_register->setColumnWidth(6, m_creditWidth);

  for(int i = 0; i < m_register->numCols(); ++i) {
    switch(i) {
      default:
        w -= m_register->columnWidth(i);
        break;
      case 3:     // skip the one, we want to set
        break;
    }
  }
  m_register->setColumnWidth(3, w);
}
