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
#include <kguiitem.h>
#include <kiconloader.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kfindtransactiondlg.h"
#include "../widgets/kmymoneyregistersearch.h"
#include "../widgets/kmymoneydateinput.h"
#include "../widgets/kmymoneyedit.h"
#include "../widgets/kmymoneylineedit.h"
#include "../widgets/kmymoneyaccountselector.h"
#include "../mymoney/mymoneyfile.h"
#include "../mymoney/storage/imymoneystorage.h"

KFindTransactionDlg::KFindTransactionDlg(QWidget *parent, const char *name)
 : KFindTransactionDlgDecl(parent, name, false),
  m_transactionPtr(0)
{
  // hide the transaction register and make sure the dialog is
  // displayed as small as can be. We make sure that the larger
  // version (with the transaction register) will also fit on the screen
  // by moving the dialog by (-45,-45).
  m_register->setParent(this);
  m_register->installEventFilter(this);
  m_registerFrame->hide();
  KFindTransactionDlgDecl::update();
  resize(minimumSizeHint());

  move(x()-45, y()-45);

  setupAccountsPage();
  setupCategoriesPage();
  setupDatePage();
  setupAmountPage();
  setupPayeesPage();
  setupDetailsPage();

  KIconLoader* il = KGlobal::iconLoader();
  KGuiItem resetButtonItem( i18n( "&Reset" ),
                    QIconSet(il->loadIcon("undo", KIcon::Small, KIcon::SizeSmall)),
                    i18n("Reset all settings"),
                    i18n("Use this to reset all settings to the state they were when the dialog was opened."));
  m_resetButton->setGuiItem(resetButtonItem);

  KGuiItem closeButtonItem( i18n( "&Close" ),
                    QIconSet(il->loadIcon("fileclose", KIcon::Small, KIcon::SizeSmall)),
                    i18n("Close the dialog"),
                    i18n("Leave the dialog and return to where you came from."));
  m_closeButton->setGuiItem(closeButtonItem);

  KGuiItem searchButtonItem( i18n( "&Search" ),
                    QIconSet(il->loadIcon("find", KIcon::Small, KIcon::SizeSmall)),
                    i18n("Start the search"),
                    i18n("Takes the current criteria and searches for matching transactions."));
  m_searchButton->setGuiItem(searchButtonItem);

  // readConfig();
  slotUpdateSelections();

  QTimer::singleShot(0, this, SLOT(slotRightSize()));

  // setup the connections
  connect(m_searchButton, SIGNAL(clicked()), this, SLOT(slotSearch()));

  connect(m_resetButton, SIGNAL(clicked()), this, SLOT(slotReset()));
  connect(m_resetButton, SIGNAL(clicked()), m_accountsView, SLOT(slotSelectAllAccounts()));
  connect(m_resetButton, SIGNAL(clicked()), m_categoriesView, SLOT(slotSelectAllAccounts()));

  connect(m_closeButton, SIGNAL(clicked()), this, SLOT(deleteLater()));

  connect(m_textEdit, SIGNAL(textChanged(const QString&)), this, SLOT(slotUpdateSelections()));

  connect(m_register, SIGNAL(clicked(int, int, int, const QPoint&)), this, SLOT(slotRegisterClicked(int, int, int, const QPoint&)));
  connect(m_register, SIGNAL(doubleClicked(int, int, int, const QPoint&)), this, SLOT(slotRegisterDoubleClicked(int, int, int, const QPoint&)));

  connect(m_register, SIGNAL(signalNextTransaction()), this, SLOT(slotNextTransaction()));
  connect(m_register, SIGNAL(signalPreviousTransaction()), this, SLOT(slotPreviousTransaction()));
  connect(m_register, SIGNAL(signalSelectTransaction(const QCString&)), this, SLOT(slotSelectTransaction(const QCString&)));

  // FIXME: Once we have the online help going, we can show the help button
  m_helpButton->hide();

  m_textEdit->setFocus();

  // make sure, we get a note when the engine changes state
  MyMoneyFile::instance()->attach(MyMoneyFile::NotifyClassAnyChange, this);
}

KFindTransactionDlg::~KFindTransactionDlg()
{
  // detach ourself from the engine
  MyMoneyFile::instance()->detach(MyMoneyFile::NotifyClassAnyChange, this);
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
  if(!m_accountsView->allAccountsSelected()) {
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
  if(!m_categoriesView->allAccountsSelected()) {
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
  m_accountsView->loadList(static_cast<KMyMoneyUtils::categoryTypeE>
                           (KMyMoneyUtils::asset | KMyMoneyUtils::liability));
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
    kMyMoneyCheckListItem* it_c = static_cast<kMyMoneyCheckListItem*>(it_v);
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
  m_categoriesView->loadList(static_cast<KMyMoneyUtils::categoryTypeE>
                           (KMyMoneyUtils::income | KMyMoneyUtils::expense));
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
    kMyMoneyCheckListItem* it_c = static_cast<kMyMoneyCheckListItem*>(it_v);
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
  MyMoneyTransactionFilter::translateDateRange(lastMonth,m_startDates[lastMonth],m_endDates[lastMonth]);
  MyMoneyTransactionFilter::translateDateRange(lastYear,m_startDates[lastYear],m_endDates[lastYear]);
  MyMoneyTransactionFilter::translateDateRange(last30Days,m_startDates[last30Days],m_endDates[last30Days]);
  MyMoneyTransactionFilter::translateDateRange(last3Months,m_startDates[last3Months],m_endDates[last3Months]);
  MyMoneyTransactionFilter::translateDateRange(last6Months,m_startDates[last6Months],m_endDates[last6Months]);
  MyMoneyTransactionFilter::translateDateRange(last12Months,m_startDates[last12Months],m_endDates[last12Months]);
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
    kMyMoneyCheckListItem* item = new kMyMoneyCheckListItem(m_payeesView, (*it_l).name(), (*it_l).id());
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
      kMyMoneyCheckListItem* it_c = static_cast<kMyMoneyCheckListItem*>(it_v);
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
      kMyMoneyCheckListItem* it_c = static_cast<kMyMoneyCheckListItem*>(it_v);
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
  // TOM: Why was this commented out? (Ace)
  if(!m_accountsView->allAccountsSelected()) {
    m_filter.addAccount(m_accountsView->selectedAccounts());
  }

  // Date tab
  if(m_dateRange->currentItem() != 0) {
    m_filter.setDateFilter(m_fromDate->getQDate(), m_toDate->getQDate());
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

  if(!m_categoriesView->allAccountsSelected()) {
    m_filter.addCategory(m_categoriesView->selectedAccounts());
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
      MyMoneyAccount acc = MyMoneyFile::instance()->account(m_filter.matchingSplits()[ofs].accountId());
      if(acc.accountGroup() == MyMoneyAccount::Asset
      || acc.accountGroup() == MyMoneyAccount::Liability)
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

bool KFindTransactionDlg::focusNextPrevChild(bool next)
{
  return KFindTransactionDlgDecl::focusNextPrevChild(next);
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


void KFindTransactionDlg::slotRegisterClicked(int row, int /* col */, int /* button */, const QPoint& /* mousePos */)
{
  if(m_register->setCurrentTransactionRow(row) == true) {
    m_register->ensureTransactionVisible();
    m_register->repaintContents(false);
  }
}

void KFindTransactionDlg::slotRegisterDoubleClicked(int row,
                                                    int /* col */,
                                                    int /* button */,
                                                    const QPoint & /* mousePos */)
{
  if(m_register->setCurrentTransactionRow(row) == true) {
    m_register->ensureTransactionVisible();
    m_register->repaintContents(false);
  }
  slotSelectTransaction();
}

void KFindTransactionDlg::slotSelectTransaction(void)
{
  KMyMoneyTransaction *k = transaction(m_register->currentTransactionIndex());
  if(k != 0) {
    emit transactionSelected(k->splitById(k->splitId()).accountId(), k->id());
    hide();
  }
}

void KFindTransactionDlg::slotNextTransaction(void)
{
  if(static_cast<unsigned> (m_register->currentTransactionIndex() + 1) <= m_transactionPtrVector.count()) {
    m_register->setCurrentTransactionIndex(m_register->currentTransactionIndex()+1);
    m_register->ensureTransactionVisible();
    m_register->repaintContents(false);
  }
}

void KFindTransactionDlg::slotPreviousTransaction(void)
{
  if(m_register->currentTransactionIndex() > 0) {
    m_register->setCurrentTransactionIndex(m_register->currentTransactionIndex()-1);
    m_register->ensureTransactionVisible();
    m_register->repaintContents(false);
  }
}

void KFindTransactionDlg::slotSelectTransaction(const QCString& transactionId)
{
  int   idx = -1;

  if(!transactionId.isEmpty()) {
    for(unsigned i = 0; i < m_transactionPtrVector.count(); ++i) {
      if(m_transactionPtrVector[i]->id() == transactionId) {
        idx = i;
        break;
      }
    }
  } else {
    if(m_transactionPtrVector.count() > 0) {
      idx = m_transactionPtrVector.count()-1;
    }
  }

  if(idx != -1) {
    // qDebug("KLedgerView::selectTransaction index is %d", idx);
    m_transactionPtr = m_transactionPtrVector[idx];
    m_register->setCurrentTransactionIndex(idx);
    m_register->ensureTransactionVisible();
    m_register->repaintContents(false);
  }
}

void KFindTransactionDlg::update(const QCString& /* id */)
{
  // only calculate the new list if it is currently visible
  if(m_registerFrame->isVisible()) {
    KMyMoneyTransaction *k = transaction(m_register->currentTransactionIndex());
    QCString id = k->id();

    slotRefreshView();

    slotSelectTransaction(id);
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

const MyMoneyMoney KFindTransactionDlg::balance(const int /* idx */) const
{
  return 0;
}

// vim:cin:si:ai:et:ts=2:sw=2:
