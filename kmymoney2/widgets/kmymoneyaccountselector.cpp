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
#include <kmymoney/mymoneyobjectcontainer.h>
#include <kmymoney/kmymoneylistviewitem.h>
#include <kmymoney/kmymoneychecklistitem.h>

#include "../kmymoneyutils.h"
#include "../kmymoneysettings.h"

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
}

kMyMoneyAccountSelector::~kMyMoneyAccountSelector()
{
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

const QCStringList kMyMoneyAccountSelector::accountList(const  QValueList<MyMoneyAccount::accountTypeE>& filterList) const
{
  QCStringList    list;
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
    return exp.search(it_c->key()) != -1;
  }
  return exp.search(it_l->key()) != -1;
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
    QRegExp exp(QString("(?:%1):%2").arg(baseName).arg(QRegExp::escape(txt)));
    if(it_v->rtti() == 1) {
      KMyMoneyCheckListItem* it_c = dynamic_cast<KMyMoneyCheckListItem*>(it_v);
      if(exp.search(it_c->key()) != -1) {
        return true;
      }
    } else if(it_v->rtti() == 0) {
      KMyMoneyListViewItem* it_c = dynamic_cast<KMyMoneyListViewItem*>(it_v);
      if(exp.search(it_c->key()) != -1) {
        return true;
      }
    }
    it++;
  }
  return false;
}

# if 0
void kMyMoneyAccountSelector::update(const QCString& /* id */)
{
  QListViewItem* it_v = m_listView->currentItem();
  QCString previousHighlighted;
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

  QCStringList list = selectedAccounts();
  QCStringList::Iterator it;

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

AccountSet::AccountSet(MyMoneyObjectContainer* objects) :
  m_count(0),
  m_objects(objects)
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
  QCStringList list;
  QCStringList::ConstIterator it_l;
  MyMoneyFile* file = MyMoneyFile::instance();
  int count = 0;
  int typeMask = 0;
  QCString currentId;

  if(selector->selectionMode() == QListView::Single) {
    QCStringList list;
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

  KListView* lv = selector->listView();
  lv->clear();
  m_count = 0;
  QString key;
  QListViewItem* after = 0;

  for(int mask = 0x01; mask != KMyMoneyUtils::last; mask <<= 1) {
    QListViewItem* item = 0;
    if(typeMask & mask & KMyMoneyUtils::asset) {
      ++m_count;
      item = selector->newItem(i18n("Asset accounts"));
      key = i18n("Asset");
      list = file->asset().accountList();
    }

    if(typeMask & mask & KMyMoneyUtils::liability) {
      ++m_count;
      item = selector->newItem(i18n("Liability accounts"));
      key = i18n("Liability");
      list = file->liability().accountList();
    }

    if(typeMask & mask & KMyMoneyUtils::income) {
      ++m_count;
      item = selector->newItem(i18n("Income categories"));
      key = i18n("Income");
      list = file->income().accountList();
      if(selector->selectionMode() == QListView::Multi) {
        selector->m_incomeCategoriesButton->show();
      }
    }

    if(typeMask & mask & KMyMoneyUtils::expense) {
      ++m_count;
      item = selector->newItem(i18n("Expense categories"));
      key = i18n("Expense");
      list = file->expense().accountList();
      if(selector->selectionMode() == QListView::Multi) {
        selector->m_expenseCategoriesButton->show();
      }
    }

    if(typeMask & mask & KMyMoneyUtils::equity) {
      ++m_count;
      item = selector->newItem(i18n("Equity accounts"), after);
      key = i18n("Equity");
      list = file->equity().accountList();
    }

    if(!after)
      after = item;

    if(item != 0) {
      // scan all matching accounts found in the engine
      for(it_l = list.begin(); it_l != list.end(); ++it_l) {
        const MyMoneyAccount& acc = m_objects->account(*it_l);
        ++m_count;
        ++count;
        if(m_typeList.contains(acc.accountType())
        && !acc.isClosed()) {
          QString tmpKey;
          tmpKey = key + ":" + acc.name();
          QListViewItem* subItem = selector->newItem(item, acc.name(), tmpKey, acc.id());
          if(acc.accountList().count() > 0) {
            subItem->setOpen(true);
            count += loadSubAccounts(selector, subItem, tmpKey, acc.accountList());
          }
        }
      }
      item->sortChildItems(0, true);
    }
  }
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

int AccountSet::load(kMyMoneyAccountSelector* selector, const QString& baseName, const QValueList<QCString>& accountIdList, const bool clear)
{
  int count = 0;
  QListViewItem* item = 0;

  m_typeList.clear();
  KListView* lv = selector->listView();
  if(clear) {
    m_count = 0;
    lv->clear();
  }

  item = selector->newItem(baseName);
  ++m_count;

  QValueList<QCString>::ConstIterator it;
  for(it = accountIdList.begin(); it != accountIdList.end(); ++it)   {
    const MyMoneyAccount& acc = m_objects->account(*it);
    if(acc.isClosed())
      continue;
    QString tmpKey;
    tmpKey = baseName + ":" + acc.name();
    selector->newItem(item, acc.name(), tmpKey, acc.id());
    ++m_count;
    ++count;
  }

  if(lv->firstChild()) {
    lv->setCurrentItem(lv->firstChild());
    lv->clearSelection();
  }

  selector->update();
  return count;
}

int AccountSet::loadSubAccounts(kMyMoneyAccountSelector* selector, QListViewItem* parent, const QString& key, const QCStringList& list)
{
  QCStringList::ConstIterator it_l;
  int count = 0;

  for(it_l = list.begin(); it_l != list.end(); ++it_l) {
    const MyMoneyAccount& acc = m_objects->account(*it_l);
    // don't include stock accounts if not in expert mode
    if(acc.accountType() == MyMoneyAccount::Stock
    && !KMyMoneySettings::expertMode())
      continue;

    if(m_typeList.contains(acc.accountType())
    && !acc.isClosed()) {
      QString tmpKey;
      tmpKey = key + ":" + acc.name();
      ++count;
      ++m_count;
      QListViewItem* item = selector->newItem(parent, acc.name(), tmpKey, acc.id());
      if(acc.accountList().count() > 0) {
        item->setOpen(true);
        count += loadSubAccounts(selector, item, tmpKey, acc.accountList());
      }
    }
  }
  return count;
}


#include "kmymoneyaccountselector.moc"
