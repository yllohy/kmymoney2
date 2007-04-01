/***************************************************************************
                          kfindtransactiondlg.cpp
                             -------------------
    copyright            : (C) 2003, 2007 by Thomas Baumgart
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
#include <qtabwidget.h>
#include <qlayout.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kapplication.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kpushbutton.h>
#include <klineedit.h>
#include <klistview.h>
#include <kcombobox.h>
#include <kstdguiitem.h>
#include <kiconloader.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kfindtransactiondlg.h"

#include <kmymoney/kmymoneydateinput.h>
#include <kmymoney/kmymoneyedit.h>
#include <kmymoney/kmymoneylineedit.h>
#include <kmymoney/kmymoneyaccountselector.h>
#include <kmymoney/mymoneyfile.h>
#include <kmymoney/kmymoneychecklistitem.h>
#include <kmymoney/kmymoneyglobalsettings.h>
#include <kmymoney/register.h>
#include <kmymoney/transaction.h>

#include "ksortoptiondlg.h"

KFindTransactionDlg::KFindTransactionDlg(QWidget *parent, const char *name) :
  KFindTransactionDlgDecl(parent, name, false),
  m_needReload(false)
{
  m_register->installEventFilter(this);
  m_tabWidget->setTabEnabled(m_resultPage, false);

  // 'cause we don't have a separate setupTextPage
  connect(m_textEdit, SIGNAL(textChanged(const QString&)), this, SLOT(slotUpdateSelections()));

  setupAccountsPage();
  setupCategoriesPage();
  setupDatePage();
  setupAmountPage();
  setupPayeesPage();
  setupDetailsPage();

  // We don't need to add the default into the list (see ::slotShowHelp() why)
  // m_helpAnchor[m_textTab] = QString("details.search");
  m_helpAnchor[m_accountTab] = QString("details.search.account");
  m_helpAnchor[m_dateTab] = QString("details.search.date");
  m_helpAnchor[m_amountTab] = QString("details.search.amount");
  m_helpAnchor[m_categoryTab] = QString("details.search.category");
  m_helpAnchor[m_payeeTab] = QString("details.search.payee");
  m_helpAnchor[m_detailsTab] = QString("details.search.details");

  m_closeButton->setGuiItem(KStdGuiItem::close());
  m_helpButton->setGuiItem(KStdGuiItem::help());

#if KDE_IS_VERSION(3,4,0)
  m_searchButton->setGuiItem(KStdGuiItem::find());
  m_resetButton->setGuiItem(KStdGuiItem::reset());
#else
  KIconLoader* il = KGlobal::iconLoader();
  KGuiItem searchButtonItem( i18n( "&Search" ),
                    QIconSet(il->loadIcon("find", KIcon::Small, KIcon::SizeSmall)),
                    i18n("Start the search"),
                    i18n("Takes the current criteria and searches for matching transactions."));
  m_searchButton->setGuiItem(searchButtonItem);

  KGuiItem resetButtonItem( i18n( "&Reset" ),
                            QIconSet(il->loadIcon("undo", KIcon::Small, KIcon::SizeSmall)),
                            i18n("Reset all settings"),
                            i18n("Use this to reset all settings to the state they were when the dialog was opened."));
  m_resetButton->setGuiItem(resetButtonItem);
#endif


  // setup the register
  QValueList<KMyMoneyRegister::Column> cols;
  cols << KMyMoneyRegister::DateColumn;
  cols << KMyMoneyRegister::AccountColumn;
  cols << KMyMoneyRegister::DetailColumn;
  cols << KMyMoneyRegister::ReconcileFlagColumn;
  cols << KMyMoneyRegister::PaymentColumn;
  cols << KMyMoneyRegister::DepositColumn;
  m_register->setupRegister(MyMoneyAccount(), cols);
  m_register->setSelectionMode(QTable::Single);

  connect(m_register, SIGNAL(editTransaction()), this, SLOT(slotSelectTransaction()));
  connect(m_register, SIGNAL(headerClicked()), this, SLOT(slotSortOptions()));

  slotUpdateSelections();

  // setup the connections
  connect(m_searchButton, SIGNAL(clicked()), this, SLOT(slotSearch()));
  connect(m_resetButton, SIGNAL(clicked()), this, SLOT(slotReset()));
  connect(m_resetButton, SIGNAL(clicked()), m_accountsView, SLOT(slotSelectAllAccounts()));
  connect(m_resetButton, SIGNAL(clicked()), m_categoriesView, SLOT(slotSelectAllAccounts()));
  connect(m_closeButton, SIGNAL(clicked()), this, SLOT(deleteLater()));
  connect(m_helpButton, SIGNAL(clicked()), this, SLOT(slotShowHelp()));

  // get signal about engine changes
  connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotRefreshView()));

  m_textEdit->setFocus();
}

void KFindTransactionDlg::slotReset(void)
{
  m_textEdit->setText(QString());
  m_regExp->setChecked(false);
  m_caseSensitive->setChecked(false);

  m_amountEdit->setEnabled(true);
  m_amountFromEdit->setEnabled(false);
  m_amountToEdit->setEnabled(false);
  m_amountEdit->loadText(QString());
  m_amountFromEdit->loadText(QString());
  m_amountToEdit->loadText(QString());
  m_amountButton->setChecked(true);
  m_amountRangeButton->setChecked(false);

  m_emptyPayeesButton->setChecked(false);
  selectAllItems(m_payeesView, true);

  m_typeBox->setCurrentItem(MyMoneyTransactionFilter::allTypes);
  m_stateBox->setCurrentItem(MyMoneyTransactionFilter::allStates);
  m_validityBox->setCurrentItem(MyMoneyTransactionFilter::anyValidity);

  m_nrEdit->setEnabled(true);
  m_nrFromEdit->setEnabled(false);
  m_nrToEdit->setEnabled(false);
  m_nrEdit->setText(QString());
  m_nrFromEdit->setText(QString());
  m_nrToEdit->setText(QString());
  m_nrButton->setChecked(true);
  m_nrRangeButton->setChecked(false);

  m_tabWidget->setTabEnabled(m_resultPage, false);
  m_tabWidget->setCurrentPage(m_tabWidget->indexOf(m_criteriaTab));

  // the following call implies a call to slotUpdateSelections,
  // that's why we call it last
  m_dateRange->setCurrentItem(allDates);
  slotDateRangeChanged(allDates);
}

void KFindTransactionDlg::slotUpdateSelections(void)
{
  QString txt;

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
  if(!m_accountsView->allItemsSelected()) {
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
  if(!m_categoriesView->allItemsSelected()) {
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
  || m_validityBox->currentItem() != 0
  || (m_nrButton->isChecked() && m_nrEdit->text().length() != 0)
  || (m_nrRangeButton->isChecked()
     && (m_nrFromEdit->text().length() != 0 || m_nrToEdit->text().length() != 0))) {
    if(!txt.isEmpty())
      txt += ", ";
    txt += i18n("Details");
  }

  // disable the search button if no selection is made
  m_searchButton->setDisabled(txt.isEmpty());

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

void KFindTransactionDlg::setupAccountsPage(void)
{
  m_accountsView->setSelectionMode(QListView::Multi);
  AccountSet accountSet(&m_objects);
  accountSet.addAccountGroup(MyMoneyAccount::Asset);
  accountSet.addAccountGroup(MyMoneyAccount::Liability);
  accountSet.load(m_accountsView);
  connect(m_accountsView, SIGNAL(stateChanged()), this, SLOT(slotUpdateSelections()));
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

void KFindTransactionDlg::selectItems(QListView* view, const QCStringList& list, const bool state)
{
  QListViewItem* it_v;

  for(it_v = view->firstChild(); it_v != 0; it_v = it_v->nextSibling()) {
    KMyMoneyCheckListItem* it_c = static_cast<KMyMoneyCheckListItem*>(it_v);
    if(it_c->type() == QCheckListItem::CheckBox && list.contains(it_c->id())) {
      it_c->setOn(state);
    }
    selectSubItems(it_v, list, state);
  }

  slotUpdateSelections();
}

void KFindTransactionDlg::setupCategoriesPage(void)
{
  m_categoriesView->setSelectionMode(QListView::Multi);
  AccountSet categorySet(&m_objects);
  categorySet.addAccountGroup(MyMoneyAccount::Income);
  categorySet.addAccountGroup(MyMoneyAccount::Expense);
  categorySet.load(m_categoriesView);
  connect(m_categoriesView, SIGNAL(stateChanged()), this, SLOT(slotUpdateSelections()));
}

void KFindTransactionDlg::selectAllSubItems(QListViewItem* item, const bool state)
{
  QListViewItem* it_v;

  for(it_v = item->firstChild(); it_v != 0; it_v = it_v->nextSibling()) {
    static_cast<QCheckListItem*>(it_v)->setOn(state);
    selectAllSubItems(it_v, state);
  }
}

void KFindTransactionDlg::selectSubItems(QListViewItem* item, const QCStringList& list, const bool state)
{
  QListViewItem* it_v;

  for(it_v = item->firstChild(); it_v != 0; it_v = it_v->nextSibling()) {
    KMyMoneyCheckListItem* it_c = static_cast<KMyMoneyCheckListItem*>(it_v);
    if(list.contains(it_c->id()))
      it_c->setOn(state);
    selectSubItems(it_v, list, state);
  }
}

void KFindTransactionDlg::setupDatePage(void)
{
  MyMoneyTransactionFilter::translateDateRange(allDates,m_startDates[allDates],m_endDates[allDates]);
  MyMoneyTransactionFilter::translateDateRange(untilToday,m_startDates[untilToday],m_endDates[untilToday]);
  MyMoneyTransactionFilter::translateDateRange(currentMonth,m_startDates[currentMonth],m_endDates[currentMonth]);
  MyMoneyTransactionFilter::translateDateRange(currentYear,m_startDates[currentYear],m_endDates[currentYear]);
  MyMoneyTransactionFilter::translateDateRange(monthToDate,m_startDates[monthToDate],m_endDates[monthToDate]);
  MyMoneyTransactionFilter::translateDateRange(yearToDate,m_startDates[yearToDate],m_endDates[yearToDate]);
  MyMoneyTransactionFilter::translateDateRange(yearToMonth,m_startDates[yearToMonth],m_endDates[yearToMonth]);
  MyMoneyTransactionFilter::translateDateRange(lastMonth,m_startDates[lastMonth],m_endDates[lastMonth]);
  MyMoneyTransactionFilter::translateDateRange(lastYear,m_startDates[lastYear],m_endDates[lastYear]);
  MyMoneyTransactionFilter::translateDateRange(last7Days,m_startDates[last7Days],m_endDates[last7Days]);
  MyMoneyTransactionFilter::translateDateRange(last30Days,m_startDates[last30Days],m_endDates[last30Days]);
  MyMoneyTransactionFilter::translateDateRange(last3Months,m_startDates[last3Months],m_endDates[last3Months]);
  MyMoneyTransactionFilter::translateDateRange(last6Months,m_startDates[last6Months],m_endDates[last6Months]);
  MyMoneyTransactionFilter::translateDateRange(last11Months,m_startDates[last11Months],m_endDates[last11Months]);
  MyMoneyTransactionFilter::translateDateRange(last12Months,m_startDates[last12Months],m_endDates[last12Months]);
  MyMoneyTransactionFilter::translateDateRange(next7Days,m_startDates[next7Days],m_endDates[next7Days]);
  MyMoneyTransactionFilter::translateDateRange(next30Days,m_startDates[next30Days],m_endDates[next30Days]);
  MyMoneyTransactionFilter::translateDateRange(next3Months,m_startDates[next3Months],m_endDates[next3Months]);
  MyMoneyTransactionFilter::translateDateRange(next6Months,m_startDates[next6Months],m_endDates[next6Months]);
  MyMoneyTransactionFilter::translateDateRange(next12Months,m_startDates[next12Months],m_endDates[next12Months]);
  MyMoneyTransactionFilter::translateDateRange(userDefined,m_startDates[userDefined],m_endDates[userDefined]);

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
    if(m_fromDate->date() == m_startDates[idx]
    && m_toDate->date() == m_endDates[idx]) {
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
  m_payeesView->setSelectionMode(QListView::Single);
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
    KMyMoneyCheckListItem* item = new KMyMoneyCheckListItem(m_payeesView, (*it_l).name(), QString(), (*it_l).id());
    connect(item, SIGNAL(stateChanged(bool)), this, SLOT(slotUpdateSelections()));
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
  connect(m_validityBox, SIGNAL(activated(int)), this, SLOT(slotUpdateSelections()));

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

void KFindTransactionDlg::scanCheckListItems(const QListViewItem* item, const opTypeE op)
{
  QListViewItem* it_v;

  for(it_v = item->firstChild(); it_v != 0; it_v = it_v->nextSibling()) {
    if(it_v->rtti() == 1) {
      KMyMoneyCheckListItem* it_c = static_cast<KMyMoneyCheckListItem*>(it_v);
      if(it_c->type() == QCheckListItem::CheckBox) {
        if(it_c->isOn())
          addItemToFilter(op, (*it_c).id());
      }
      scanCheckListItems(it_v, op);
    }
  }
}

void KFindTransactionDlg::scanCheckListItems(const QListView* view, const opTypeE op)
{
  QListViewItem* it_v;

  for(it_v = view->firstChild(); it_v != 0; it_v = it_v->nextSibling()) {
    if(it_v->rtti() == 1) {
      KMyMoneyCheckListItem* it_c = static_cast<KMyMoneyCheckListItem*>(it_v);
      if(it_c->type() == QCheckListItem::CheckBox) {
        if(it_c->isOn())
          addItemToFilter(op, (*it_c).id());
      }
      scanCheckListItems(it_v, op);
    }
  }
}

void KFindTransactionDlg::setupFilter(void)
{
  m_filter.clear();

  // Text tab
  if(!m_textEdit->text().isEmpty()) {
    QRegExp exp(m_textEdit->text(), m_caseSensitive->isChecked(), !m_regExp->isChecked());
    m_filter.setTextFilter(exp);
  }

  // Account tab
  if(!m_accountsView->allItemsSelected()) {
    // retrieve a list of selected accounts
    QCStringList list;
    m_accountsView->selectedItems(list);

    // if we're not in expert mode, we need to make sure
    // that all stock accounts for the selected investment
    // account are also selected
    if(!KMyMoneyGlobalSettings::expertMode()) {
      QCStringList missing;
      QCStringList::const_iterator it_a, it_b;
      for(it_a = list.begin(); it_a != list.end(); ++it_a) {
        MyMoneyAccount acc = MyMoneyFile::instance()->account(*it_a);
        if(acc.accountType() == MyMoneyAccount::Investment) {
          for(it_b = acc.accountList().begin(); it_b != acc.accountList().end(); ++it_b) {
            if(!list.contains(*it_b)) {
              missing.append(*it_b);
            }
          }
        }
      }
      list += missing;
    }

    m_filter.addAccount(list);
  }

  // Date tab
  if(m_dateRange->currentItem() != 0) {
    m_filter.setDateFilter(m_fromDate->date(), m_toDate->date());
  }

  // Amount tab
  if((m_amountButton->isChecked() && m_amountEdit->isValid())) {
    m_filter.setAmountFilter(m_amountEdit->value(), m_amountEdit->value());

  } else if((m_amountRangeButton->isChecked()
      && (m_amountFromEdit->isValid() || m_amountToEdit->isValid()))) {

    MyMoneyMoney from(MyMoneyMoney::minValue), to(MyMoneyMoney::maxValue);
    if(m_amountFromEdit->isValid())
      from = m_amountFromEdit->value();
    if(m_amountToEdit->isValid())
      to = m_amountToEdit->value();

    m_filter.setAmountFilter(from, to);
  }

  // Categories tab
  if(!m_categoriesView->allItemsSelected()) {
    m_filter.addCategory(m_categoriesView->selectedItems());
  }

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

  if(m_validityBox->currentItem() != 0)
    m_filter.addValidity(m_validityBox->currentItem());

  if(m_nrButton->isChecked() && !m_nrEdit->text().isEmpty())
    m_filter.setNumberFilter(m_nrEdit->text(), m_nrEdit->text());

  if(m_nrRangeButton->isChecked()
     && (!m_nrFromEdit->text().isEmpty() || !m_nrToEdit->text().isEmpty())) {
    m_filter.setNumberFilter(m_nrFromEdit->text(), m_nrToEdit->text());
  }
}

void KFindTransactionDlg::slotSearch(void)
{
  // setup the filter from the dialog widgets
  setupFilter();

  // filter is setup, now fill the register
  slotRefreshView();

  m_register->setFocus();
}

void KFindTransactionDlg::slotRefreshView(void)
{
  m_needReload = true;
  if(isVisible()) {
    loadView();
    m_needReload = false;
  }
}

void KFindTransactionDlg::show(void)
{
  if(m_needReload) {
    loadView();
    m_needReload = false;
  }
  KFindTransactionDlgDecl::show();
}

void KFindTransactionDlg::loadView(void)
{
  // setup sort order
  m_register->setSortOrder(KMyMoneySettings::sortSearchView());

  // clear out old data
  m_objects.clear();
  m_register->clear();

  // retrieve the list from the engine
  MyMoneyFile::instance()->transactionList(m_transactionList, m_filter);

    // create the elements for the register
  QValueList<QPair<MyMoneyTransaction, MyMoneySplit> >::const_iterator it;
  QMap<QCString, int>uniqueMap;
  MyMoneyMoney deposit, payment;

  int splitCount = 0;
  for(it = m_transactionList.begin(); it != m_transactionList.end(); ++it) {
    const MyMoneySplit& split = (*it).second;
    MyMoneyAccount acc = MyMoneyFile::instance()->account(split.accountId());
    if(acc.isAssetLiability()) {
      ++splitCount;
      uniqueMap[(*it).first.id()]++;

      KMyMoneyRegister::Register::transactionFactory(m_register, &m_objects, (*it).first, (*it).second, uniqueMap[(*it).first.id()]);
      { // debug stuff
        if(split.shares().isNegative()) {
          payment += split.shares().abs();
        } else {
          deposit += split.shares().abs();
        }
      }
    }
  }

    // add the group markers
  m_register->addGroupMarkers();

    // sort the transactions according to the sort setting
  m_register->sortItems();

    // remove trailing and adjacent markers
  m_register->removeUnwantedGroupMarkers();

  // turn on the ledger lens for the register
  m_register->setLedgerLensForced();

  m_register->updateRegister(true);

  m_foundText->setText(i18n(QString("Found %1 matching transactions (D %2 / P %3 = %4)")
                      .arg(splitCount).arg(deposit.formatMoney()).arg(payment.formatMoney()).arg((deposit-payment).formatMoney())));

  m_tabWidget->setTabEnabled(m_resultPage, true);
  m_tabWidget->setCurrentPage(m_tabWidget->indexOf(m_resultPage));

  QTimer::singleShot(10, this, SLOT(slotRightSize()));
}

void KFindTransactionDlg::slotRightSize(void)
{
  m_register->updateContents();
}

void KFindTransactionDlg::resizeEvent(QResizeEvent* ev)
{
  // don't forget the resizer
  KFindTransactionDlgDecl::resizeEvent(ev);

  if(!m_register->isVisible())
    return;

  // resize the register
  int w = m_register->visibleWidth();

  int m_debitWidth = 80;
  int m_creditWidth = 80;

  m_register->adjustColumn(0);
  m_register->adjustColumn(1);
  m_register->adjustColumn(2);

  m_register->setColumnWidth(4, m_debitWidth);
  m_register->setColumnWidth(5, m_creditWidth);

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


void KFindTransactionDlg::slotSelectTransaction(void)
{
  QValueList<KMyMoneyRegister::RegisterItem*> list = m_register->selectedItems();
  if(!list.isEmpty()) {
    KMyMoneyRegister::Transaction* t = dynamic_cast<KMyMoneyRegister::Transaction*>(list[0]);
    if(t) {
      emit transactionSelected(t->split().accountId(), t->transaction().id());
      hide();
    }
  }
}

bool KFindTransactionDlg::eventFilter(QObject* o, QEvent* e)
{
  bool rc = false;

  if(o->isWidgetType()) {
    if(e->type() == QEvent::KeyPress) {
      const QWidget* w = dynamic_cast<const QWidget*>(o);
      QKeyEvent *k = static_cast<QKeyEvent *> (e);
      if(w == m_register) {
        switch(k->key()) {
          default:
            break;

          case Qt::Key_Return:
          case Qt::Key_Enter:
            rc = true;
            slotSelectTransaction();
            break;
        }
      }
    }
  }
  return rc;
}

void KFindTransactionDlg::slotShowHelp(void)
{
  QString anchor = m_helpAnchor[m_criteriaTab->currentPage()];
  if(anchor.isEmpty())
    anchor = QString("details.search");

  kapp->invokeHelp(anchor);
}

void KFindTransactionDlg::slotSortOptions(void)
{
  KSortOptionDlg* dlg = new KSortOptionDlg(this);

  dlg->setSortOption(KMyMoneyGlobalSettings::sortSearchView(), QString());
  dlg->hideDefaultButton();

  if(dlg->exec() == QDialog::Accepted) {
    QString sortOrder = dlg->sortOption();
    if(sortOrder != KMyMoneyGlobalSettings::sortSearchView()) {
      KMyMoneyGlobalSettings::setSortSearchView(sortOrder);
      slotRefreshView();
    }
  }
  delete dlg;
}


// vim:cin:si:ai:et:ts=2:sw=2:


#include "kfindtransactiondlg.moc"
