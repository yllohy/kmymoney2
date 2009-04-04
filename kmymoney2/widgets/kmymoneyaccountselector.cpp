/***************************************************************************
                          kmymoneyaccountselector.cpp  -  description
                             -------------------
    begin                : Thu Sep 18 2003
    copyright            : (C) 2003 by Thomas Baumgart
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
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

#include <qlayout.h>
#include <qheader.h>
#include <qlabel.h>
#include <qtimer.h>
#include <qpainter.h>
#include <qstyle.h>
#include <qrect.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>
#include <kpushbutton.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyaccountselector.h"
#include <kmymoney/mymoneyutils.h>
#include <kmymoney/mymoneyfile.h>
#include <kmymoney/kmymoneylistviewitem.h>
#include <kmymoney/kmymoneychecklistitem.h>

#include "../kmymoneyutils.h"
#include "../kmymoneyglobalsettings.h"

kMyMoneyAccountSelector::kMyMoneyAccountSelector(QWidget *parent, const char *name, QWidget::WFlags flags, const bool createButtons) :
  KMyMoneySelector(parent, name, flags),
  m_allAccountsButton(0),
  m_noAccountButton(0),
  m_incomeCategoriesButton(0),
  m_expenseCategoriesButton(0)
{

  if(createButtons) {
    QVBoxLayout* buttonLayout = new QVBoxLayout( 0, 0, 6, "accountSelectorButtonLayout");

    m_allAccountsButton = new KPushButton( this, "m_allAccountsButton" );
    m_allAccountsButton->setText( i18n( "All" ) );
    buttonLayout->addWidget( m_allAccountsButton );

    m_incomeCategoriesButton = new KPushButton( this, "m_incomeCategoriesButton" );
    m_incomeCategoriesButton->setText( i18n( "Income" ) );
    buttonLayout->addWidget( m_incomeCategoriesButton );

    m_expenseCategoriesButton = new KPushButton( this, "m_expenseCategoriesButton" );
    m_expenseCategoriesButton->setText( i18n( "Expense" ) );
    buttonLayout->addWidget( m_expenseCategoriesButton );

    m_noAccountButton = new KPushButton( this, "m_noAccountButton" );
    m_noAccountButton->setText( i18n( "None" ) );
    buttonLayout->addWidget( m_noAccountButton );

    QSpacerItem* spacer = new QSpacerItem( 0, 67, QSizePolicy::Minimum, QSizePolicy::Expanding );
    buttonLayout->addItem( spacer );
    m_layout->addLayout( buttonLayout );

    connect(m_allAccountsButton, SIGNAL(clicked()), this, SLOT(slotSelectAllAccounts()));
    connect(m_noAccountButton, SIGNAL(clicked()), this, SLOT(slotDeselectAllAccounts()));
    connect(m_incomeCategoriesButton, SIGNAL(clicked()), this, SLOT(slotSelectIncomeCategories()));
    connect(m_expenseCategoriesButton, SIGNAL(clicked()), this, SLOT(slotSelectExpenseCategories()));
  }

  // sort the list of accounts in ascending order
  m_listView->setSorting(0);
}

kMyMoneyAccountSelector::~kMyMoneyAccountSelector()
{
}

void kMyMoneyAccountSelector::removeButtons(void)
{
  delete m_allAccountsButton;
  delete m_incomeCategoriesButton;
  delete m_expenseCategoriesButton;
  delete m_noAccountButton;
}

void kMyMoneyAccountSelector::selectCategories(const bool income, const bool expense)
{
  QListViewItem* it_v;

  for(it_v = m_listView->firstChild(); it_v != 0; it_v = it_v->nextSibling()) {
    if(static_cast<QCheckListItem*>(it_v)->text() == i18n("Income categories"))
      selectAllSubItems(it_v, income);
    else
      selectAllSubItems(it_v, expense);
  }
  emit stateChanged();
}

void kMyMoneyAccountSelector::setSelectionMode(QListView::SelectionMode mode)
{
  m_incomeCategoriesButton->setHidden(mode == QListView::Multi);
  m_expenseCategoriesButton->setHidden(mode == QListView::Multi);
  KMyMoneySelector::setSelectionMode(mode);
}

QStringList kMyMoneyAccountSelector::accountList(const  QValueList<MyMoneyAccount::accountTypeE>& filterList) const
{
  QStringList    list;
  QListViewItemIterator it;
  QListViewItem* it_v;
  QValueList<MyMoneyAccount::accountTypeE>::ConstIterator it_f;

  it = QListViewItemIterator(m_listView, QListViewItemIterator::Selectable);
  while((it_v = it.current()) != 0) {
    {
      if(it_v->rtti() == 1) {
        KMyMoneyCheckListItem* it_c = dynamic_cast<KMyMoneyCheckListItem*>(it_v);
        if(it_c->type() == QCheckListItem::CheckBox) {
          MyMoneyAccount acc = MyMoneyFile::instance()->account(it_c->id());
          it_f = filterList.find(acc.accountType());
          if(filterList.count() == 0 || it_f != filterList.end())
            list << it_c->id();
        }
      } else if(it_v->rtti() == 0) {
        KMyMoneyListViewItem* it_c = dynamic_cast<KMyMoneyListViewItem*>(it_v);
        MyMoneyAccount acc = MyMoneyFile::instance()->account(it_c->id());
        it_f = filterList.find(acc.accountType());
        if(filterList.count() == 0 || it_f != filterList.end())
          list << it_c->id();
      }
    }
    it++;
  }
  return list;
}

bool kMyMoneyAccountSelector::match(const QRegExp& exp, QListViewItem* item) const
{
  if(!item->isSelectable())
    return false;

  KMyMoneyListViewItem* it_l = dynamic_cast<KMyMoneyListViewItem*>(item);
  if(!it_l) {
    KMyMoneyCheckListItem* it_c = dynamic_cast<KMyMoneyCheckListItem*>(item);
    if(!it_c) {
      return KMyMoneySelector::match(exp, item);
    }
    return exp.search(it_c->key(1, true)) != -1;
  }
  return exp.search(it_l->key(1, true)) != -1;
}

bool kMyMoneyAccountSelector::contains(const QString& txt) const
{
  QListViewItemIterator it(m_listView, QListViewItemIterator::Selectable);
  QListViewItem* it_v;

  QString baseName = i18n("Asset") + "|" +
                     i18n("Liability") + "|" +
                     i18n("Income")+ "|" +
                     i18n("Expense")+ "|" +
                     i18n("Equity") + "|" +
                     i18n("Security");

  while((it_v = it.current()) != 0) {
    QRegExp exp(QString("^(?:%1):%2$").arg(baseName).arg(QRegExp::escape(txt)));
    if(it_v->rtti() == 1) {
      KMyMoneyCheckListItem* it_c = dynamic_cast<KMyMoneyCheckListItem*>(it_v);
      if(exp.search(it_c->key(1, true)) != -1) {
        return true;
      }
    } else if(it_v->rtti() == 0) {
      KMyMoneyListViewItem* it_c = dynamic_cast<KMyMoneyListViewItem*>(it_v);
      if(exp.search(it_c->key(1, true)) != -1) {
        return true;
      }
    }
    it++;
  }
  return false;
}

# if 0
void kMyMoneyAccountSelector::update(const QString& /* id */)
{
  QListViewItem* it_v = m_listView->currentItem();
  QString previousHighlighted;
  bool state = false;

  if(m_selMode == QListView::Multi && it_v) {
    if(it_v->rtti() == 1) {
      KMyMoneyCheckListItem* it_c = static_cast<KMyMoneyCheckListItem*>(it_v);
      if(it_c->type() == QCheckListItem::CheckBox) {
        previousHighlighted = it_c->id();
        state = it_c->isOn();
      }
    }
  }

  QStringList list = selectedAccounts();
  QStringList::Iterator it;

  if(!m_typeList.isEmpty())
    loadList(m_typeList);
  else if(!m_baseName.isEmpty()) {
    loadList(m_baseName, m_accountList);
  }

  // because loadList() sets all accounts selected, we have to
  // clear the selection and only turn on those, that were on
  // before the update.
  slotDeselectAllAccounts();
  for(it = list.begin(); it != list.end(); ++it) {
    setSelected(*it, true);
  }

  if(m_selMode == QListView::Multi) {
    // make the previous highlighted item highlighted again
    if(!previousHighlighted.isEmpty()) {
      setSelected(previousHighlighted);
    }
  }
}
#endif

AccountSet::AccountSet() :
  m_count(0),
  m_file(MyMoneyFile::instance()),
  m_favorites(0),
  m_hideClosedAccounts(true)
{
}

void AccountSet::addAccountGroup(MyMoneyAccount::accountTypeE group)
{
  if(group == MyMoneyAccount::Asset) {
    m_typeList << MyMoneyAccount::Checkings;
    m_typeList << MyMoneyAccount::Savings;
    m_typeList << MyMoneyAccount::Cash;
    m_typeList << MyMoneyAccount::AssetLoan;
    m_typeList << MyMoneyAccount::CertificateDep;
    m_typeList << MyMoneyAccount::Investment;
    m_typeList << MyMoneyAccount::Stock;
    m_typeList << MyMoneyAccount::MoneyMarket;
    m_typeList << MyMoneyAccount::Asset;
    m_typeList << MyMoneyAccount::Currency;

  } else if(group == MyMoneyAccount::Liability) {
    m_typeList << MyMoneyAccount::CreditCard;
    m_typeList << MyMoneyAccount::Loan;
    m_typeList << MyMoneyAccount::Liability;

  } else if(group == MyMoneyAccount::Income) {
    m_typeList << MyMoneyAccount::Income;

  } else if(group == MyMoneyAccount::Expense) {
    m_typeList << MyMoneyAccount::Expense;

  } else if(group == MyMoneyAccount::Equity) {
    m_typeList << MyMoneyAccount::Equity;
  }
}

void AccountSet::addAccountType(MyMoneyAccount::accountTypeE type)
{
    m_typeList << type;
}

void AccountSet::removeAccountType(MyMoneyAccount::accountTypeE type)
{
  QValueList<MyMoneyAccount::accountTypeE>::iterator it;
  it = m_typeList.find(type);
  if(it != m_typeList.end()) {
    m_typeList.remove(it);
  }
}

void AccountSet::clear(void)
{
  m_typeList.clear();
}

int AccountSet::load(kMyMoneyAccountSelector* selector)
{
  QStringList list;
  QStringList::ConstIterator it_l;
  int count = 0;
  int typeMask = 0;
  QString currentId;

  if(selector->selectionMode() == QListView::Single) {
    QStringList list;
    selector->selectedItems(list);
    if(list.count() > 0)
      currentId = list.first();
  }
  if((m_typeList.contains(MyMoneyAccount::Checkings)
    + m_typeList.contains(MyMoneyAccount::Savings)
    + m_typeList.contains(MyMoneyAccount::Cash)
    + m_typeList.contains(MyMoneyAccount::AssetLoan)
    + m_typeList.contains(MyMoneyAccount::CertificateDep)
    + m_typeList.contains(MyMoneyAccount::Investment)
    + m_typeList.contains(MyMoneyAccount::Stock)
    + m_typeList.contains(MyMoneyAccount::MoneyMarket)
    + m_typeList.contains(MyMoneyAccount::Asset)
    + m_typeList.contains(MyMoneyAccount::Currency)) > 0)
    typeMask |= KMyMoneyUtils::asset;

  if((m_typeList.contains(MyMoneyAccount::CreditCard)
    + m_typeList.contains(MyMoneyAccount::Loan)
    + m_typeList.contains(MyMoneyAccount::Liability)) > 0)
    typeMask |= KMyMoneyUtils::liability;

  if((m_typeList.contains(MyMoneyAccount::Income)) > 0)
    typeMask |= KMyMoneyUtils::income;

  if((m_typeList.contains(MyMoneyAccount::Expense)) > 0)
    typeMask |= KMyMoneyUtils::expense;

  if((m_typeList.contains(MyMoneyAccount::Equity)) > 0)
    typeMask |= KMyMoneyUtils::equity;

  selector->clear();
  KListView* lv = selector->listView();
  m_count = 0;
  QString key;
  QListViewItem* after = 0;

  // create the favorite section first and sort it to the beginning
  key = QString("A%1").arg(i18n("Favorites"));
  m_favorites = selector->newItem(i18n("Favorites"), key);

  for(int mask = 0x01; mask != KMyMoneyUtils::last; mask <<= 1) {
    QListViewItem* item = 0;
    if((typeMask & mask & KMyMoneyUtils::asset) != 0) {
      ++m_count;
      key = QString("B%1").arg(i18n("Asset"));
      item = selector->newItem(i18n("Asset accounts"), key);
      list = m_file->asset().accountList();
    }

    if((typeMask & mask & KMyMoneyUtils::liability) != 0) {
      ++m_count;
      key = QString("C%1").arg(i18n("Liability"));
      item = selector->newItem(i18n("Liability accounts"), key);
      list = m_file->liability().accountList();
    }

    if((typeMask & mask & KMyMoneyUtils::income) != 0) {
      ++m_count;
      key = QString("D%1").arg(i18n("Income"));
      item = selector->newItem(i18n("Income categories"), key);
      list = m_file->income().accountList();
      if(selector->selectionMode() == QListView::Multi) {
        selector->m_incomeCategoriesButton->show();
      }
    }

    if((typeMask & mask & KMyMoneyUtils::expense) != 0) {
      ++m_count;
      key = QString("E%1").arg(i18n("Expense"));
      item = selector->newItem(i18n("Expense categories"), key);
      list = m_file->expense().accountList();
      if(selector->selectionMode() == QListView::Multi) {
        selector->m_expenseCategoriesButton->show();
      }
    }

    if((typeMask & mask & KMyMoneyUtils::equity) != 0) {
      ++m_count;
      key = QString("F%1").arg(i18n("Equity"));
      item = selector->newItem(i18n("Equity accounts"), key);
      list = m_file->equity().accountList();
    }

    if(!after)
      after = item;

    if(item != 0) {
      // scan all matching accounts found in the engine
      for(it_l = list.begin(); it_l != list.end(); ++it_l) {
        const MyMoneyAccount& acc = m_file->account(*it_l);
        ++m_count;
        ++count;
        //this will include an account if it matches the account type and
        //if it is still open or it has been set to show closed accounts
        if(includeAccount(acc)
        && (!isHidingClosedAccounts() || !acc.isClosed()) ) {
          QString tmpKey;
          tmpKey = key + MyMoneyFile::AccountSeperator + acc.name();
          QListViewItem* subItem = selector->newItem(item, acc.name(), tmpKey, acc.id());
          if(acc.value("PreferredAccount") == "Yes"
             && m_typeList.contains(acc.accountType())) {
            selector->newItem(m_favorites, acc.name(), tmpKey, acc.id());
          }
          if(acc.accountList().count() > 0) {
            subItem->setOpen(true);
            count += loadSubAccounts(selector, subItem, tmpKey, acc.accountList());
          }

          //disable the item if it has been added only because a subaccount matches the type
          if( !m_typeList.contains(acc.accountType()) ) {
            subItem->setEnabled(false);
          }
        }
      }
      item->sortChildItems(0, true);
    }
  }

  // if we don't have a favorite account or the selector is for multi-mode
  // we get rid of the favorite entry and subentries.
  if(m_favorites->childCount() == 0 || selector->selectionMode() == QListView::Multi) {
    delete m_favorites;
    m_favorites = 0;
  }

  // sort the list
  selector->listView()->sort();

  if(lv->firstChild()) {
    if(currentId.isEmpty()) {
      lv->setCurrentItem(lv->firstChild());
      lv->clearSelection();
    } else {
      selector->setSelected(currentId);
    }
  }
  selector->update();
  return count;
}

int AccountSet::load(kMyMoneyAccountSelector* selector, const QString& baseName, const QValueList<QString>& accountIdList, const bool clear)
{
  int count = 0;
  QListViewItem* item = 0;

  m_typeList.clear();
  if(clear) {
    m_count = 0;
    selector->clear();
  }

  item = selector->newItem(baseName);
  ++m_count;

  QValueList<QString>::ConstIterator it;
  for(it = accountIdList.begin(); it != accountIdList.end(); ++it)   {
    const MyMoneyAccount& acc = m_file->account(*it);
    if(acc.isClosed())
      continue;
    QString tmpKey;
    // the first character must be preset. Since we don't know any sort order here, we just use A
    tmpKey = QString("A%1%2%3").arg(baseName, MyMoneyFile::AccountSeperator, acc.name());
    selector->newItem(item, acc.name(), tmpKey, acc.id());
    ++m_count;
    ++count;
  }

  KListView* lv = selector->listView();
  if(lv->firstChild()) {
    lv->setCurrentItem(lv->firstChild());
    lv->clearSelection();
  }

  selector->update();
  return count;
}

int AccountSet::loadSubAccounts(kMyMoneyAccountSelector* selector, QListViewItem* parent, const QString& key, const QStringList& list)
{
  QStringList::ConstIterator it_l;
  int count = 0;

  for(it_l = list.begin(); it_l != list.end(); ++it_l) {
    const MyMoneyAccount& acc = m_file->account(*it_l);
    // don't include stock accounts if not in expert mode
    if(acc.isInvest() && !KMyMoneyGlobalSettings::expertMode())
      continue;

    if(includeAccount(acc)
    && !acc.isClosed()) {
      QString tmpKey;
      tmpKey = key + MyMoneyFile::AccountSeperator + acc.name();
      ++count;
      ++m_count;
      QListViewItem* item = selector->newItem(parent, acc.name(), tmpKey, acc.id());
      if(acc.value("PreferredAccount") == "Yes"
         && m_typeList.contains(acc.accountType())) {
        selector->newItem(m_favorites, acc.name(), tmpKey, acc.id());
      }
      if(acc.accountList().count() > 0) {
        item->setOpen(true);
        count += loadSubAccounts(selector, item, tmpKey, acc.accountList());
      }

      //disable the item if it has been added only because a subaccount matches the type
      if( !m_typeList.contains(acc.accountType()) ) {
        item->setEnabled(false);
      }
    }
  }
  return count;
}

bool AccountSet::includeAccount(const MyMoneyAccount& acc)
{
  if( m_typeList.contains(acc.accountType()) )
    return true;

  QStringList accounts = acc.accountList();

  if(accounts.size() > 0) {
    QStringList::ConstIterator it_acc;
    for(it_acc = accounts.begin(); it_acc != accounts.end(); ++it_acc) {
      MyMoneyAccount account = m_file->account(*it_acc);
      if( includeAccount(account) )
        return true;
    }
  }
  return false;
}


#include "kmymoneyaccountselector.moc"
