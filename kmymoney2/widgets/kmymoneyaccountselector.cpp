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

#include "../kmymoneyutils.h"
#include "../mymoney/mymoneyutils.h"
#include "../mymoney/mymoneyfile.h"

#include "kmymoneyaccountselector.h"

kMyMoneyListViewItem::kMyMoneyListViewItem(QListView* parent, const QString& txt, const QCString& id) :
  KListViewItem(parent, txt),
  m_id(id)
{
}

kMyMoneyListViewItem::kMyMoneyListViewItem(QListViewItem* parent, const QString& txt, const QCString& id) :
  KListViewItem(parent, txt),
  m_id(id)
{
}

kMyMoneyListViewItem::~kMyMoneyListViewItem()
{
}

void kMyMoneyListViewItem::paintCell(QPainter *p, const QColorGroup &cg, int column, int width, int alignment)
{
  QColorGroup _cg = cg;
  _cg.setColor(QColorGroup::Base, backgroundColor());

  // make sure to bypass KListViewItem::paintCell() as
  // we don't like it's logic - that's why we do this
  // here ;-)    (ipwizard)
  QListViewItem::paintCell(p, _cg, column, width, alignment);
}

const QColor kMyMoneyListViewItem::backgroundColor()
{
  return isAlternate() ? KMyMoneyUtils::backgroundColour() : KMyMoneyUtils::listColour();
}

kMyMoneyCheckListItem::kMyMoneyCheckListItem(QListView* parent, const QString& txt, const QCString& id, Type type) :
  QCheckListItem(parent, txt, type),
  m_id(id)
{
  setOn(true);
  m_known = false;
}

kMyMoneyCheckListItem::kMyMoneyCheckListItem(QListViewItem* parent, const QString& txt, const QCString& id, Type type) :
  QCheckListItem(parent, txt, type),
  m_id(id)
{
  setOn(true);
  m_known = false;
}

kMyMoneyCheckListItem::~kMyMoneyCheckListItem()
{
}

void kMyMoneyCheckListItem::stateChange(bool state)
{
  emit stateChanged(state);
}

void kMyMoneyCheckListItem::paintCell(QPainter *p, const QColorGroup &cg, int column, int width, int alignment)
{
  QColorGroup _cg = cg;
  _cg.setColor(QColorGroup::Base, backgroundColor());

  // write the groups in bold
  QFont f = p->font();
  f.setBold(!isSelectable());
  p->setFont(f);

  QCheckListItem::paintCell(p, _cg, column, width, alignment);
}

const QColor kMyMoneyCheckListItem::backgroundColor()
{
  return isAlternate() ? KMyMoneyUtils::backgroundColour() : KMyMoneyUtils::listColour();
}

bool kMyMoneyCheckListItem::isAlternate(void)
{
// logic taken from KListViewItem::isAlternate()
  kMyMoneyCheckListItem* above;
  above = dynamic_cast<kMyMoneyCheckListItem*> (itemAbove());
  m_known = above ? above->m_known : true;
  if(m_known) {
    m_odd = above ? !above->m_odd : false;
  } else {
    kMyMoneyCheckListItem* item;
    bool previous = true;
    if(QListViewItem::parent()) {
      item = dynamic_cast<kMyMoneyCheckListItem *>(QListViewItem::parent());
      previous = item->m_odd;
      item = dynamic_cast<kMyMoneyCheckListItem *>(QListViewItem::parent()->firstChild());
    } else {
      item = dynamic_cast<kMyMoneyCheckListItem *>(listView()->firstChild());
    }
    while(item) {
      item->m_odd = previous = !previous;
      item->m_known = true;
      item = dynamic_cast<kMyMoneyCheckListItem *>(item->nextSibling());
    }
  }
  return m_odd;
}

kMyMoneyAccountSelector::kMyMoneyAccountSelector(QWidget *parent, const char *name, QWidget::WFlags flags, const bool createButtons) :
  QWidget(parent, name, flags),
  m_allAccountsButton(0),
  m_noAccountButton(0),
  m_incomeCategoriesButton(0),
  m_expenseCategoriesButton(0)
{
  QHBoxLayout*   layout;
  QVBoxLayout*   buttonLayout;

  m_selMode = QListView::Single;

  m_listView = new KListView(this);
  // don't show horizontal scroll bar
  m_listView->setHScrollBarMode(QScrollView::AlwaysOff);

  if(parent) {
    setFocusProxy(parent->focusProxy());
    m_listView->setFocusProxy(parent->focusProxy());
  }

  m_listView->setAllColumnsShowFocus(true);

  layout = new QHBoxLayout( this, 0, 6, "accountSelectorLayout");

  m_listView->addColumn( "Hidden" );
  // m_listView->header()->setClickEnabled( FALSE, m_listView->header()->count() - 1 );
  // m_listView->header()->setResizeEnabled( FALSE, m_listView->header()->count() - 1 );
  m_listView->header()->hide();
  m_listView->header()->setStretchEnabled(true, -1);
  m_listView->header()->adjustHeaderSize();

  layout->addWidget( m_listView );

  if(createButtons) {
    buttonLayout = new QVBoxLayout( 0, 0, 6, "accountSelectorButtonLayout");

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
    layout->addLayout( buttonLayout );
  }

  // force init
  m_selMode = QListView::Multi;
  setSelectionMode(QListView::Single);

  if(createButtons) {
    connect(m_allAccountsButton, SIGNAL(clicked()), this, SLOT(slotSelectAllAccounts()));
    connect(m_noAccountButton, SIGNAL(clicked()), this, SLOT(slotDeselectAllAccounts()));
    connect(m_incomeCategoriesButton, SIGNAL(clicked()), this, SLOT(slotSelectIncomeCategories()));
    connect(m_expenseCategoriesButton, SIGNAL(clicked()), this, SLOT(slotSelectExpenseCategories()));
  }

  connect(m_listView, SIGNAL(rightButtonPressed(QListViewItem* , const QPoint&, int)), this, SLOT(slotListRightMouse(QListViewItem*, const QPoint&, int)));

  MyMoneyFile::instance()->attach(MyMoneyFile::NotifyClassAccountHierarchy, this);
}

kMyMoneyAccountSelector::~kMyMoneyAccountSelector()
{
  MyMoneyFile::instance()->detach(MyMoneyFile::NotifyClassAccountHierarchy, this);
}

void kMyMoneyAccountSelector::setSelectionMode(const QListView::SelectionMode mode)
{
  if(m_selMode != mode) {
    m_selMode = mode;
    m_listView->clear();

    // make sure, it's either Multi or Single
    if(mode != QListView::Multi) {
      m_selMode = QListView::Single;
      connect(m_listView, SIGNAL(selectionChanged(void)), this, SIGNAL(stateChanged(void)));
      connect(m_listView, SIGNAL(executed(QListViewItem*)), this, SLOT(slotItemSelected(QListViewItem*)));

      if(m_allAccountsButton) {
        m_allAccountsButton->hide();
        m_noAccountButton->hide();
        m_incomeCategoriesButton->hide();
        m_expenseCategoriesButton->hide();
      }
    } else {
      disconnect(m_listView, SIGNAL(selectionChanged(void)), this, SIGNAL(stateChanged(void)));
      disconnect(m_listView, SIGNAL(executed(QListViewItem*)), this, SLOT(slotItemSelected(QListViewItem*)));
      if(m_allAccountsButton) {
        m_allAccountsButton->show();
        m_noAccountButton->show();
        m_incomeCategoriesButton->show();
        m_expenseCategoriesButton->show();
      }
    }
  }
  QWidget::update();
}

void kMyMoneyAccountSelector::slotItemSelected(QListViewItem *item)
{
  if(m_selMode == QListView::Single) {
    kMyMoneyListViewItem* l_item = dynamic_cast<kMyMoneyListViewItem*>(item);
    if(l_item && l_item->isSelectable()) {
      emit accountSelected(l_item->id());
    }
  }
}

QListViewItem* kMyMoneyAccountSelector::newEntryFactory(QListViewItem* parent, const QString& name, const QCString& id)
{
  QListViewItem* p;

  if(m_selMode == QListView::Multi) {
    kMyMoneyCheckListItem* q = new kMyMoneyCheckListItem(parent, name, id);
    connect(q, SIGNAL(stateChanged(bool)), this, SIGNAL(stateChanged(void)));
    p = static_cast<QListViewItem*> (q);

  } else {
    kMyMoneyListViewItem* q = new kMyMoneyListViewItem(parent, name, id);
    p = static_cast<QListViewItem*> (q);
  }

  return p;
}

void kMyMoneyAccountSelector::protectAccount(const QCString& accId, const bool protect)
{
  QListViewItemIterator it(m_listView, QListViewItemIterator::Selectable);
  QListViewItem* it_v;
  kMyMoneyListViewItem* it_l;
  kMyMoneyCheckListItem* it_c;

  // scan items
  while((it_v = it.current()) != 0) {
    it_l = dynamic_cast<kMyMoneyListViewItem*>(it_v);
    if(it_l) {
      if(it_l->id() == accId) {
        it_l->setSelectable(!protect);
        break;
      }
    } else {
      it_c = dynamic_cast<kMyMoneyCheckListItem*>(it_v);
      if(it_c) {
        if(it_c->id() == accId) {
          it_c->setSelectable(!protect);
          break;
        }
      }
    }
    ++it;
  }
}

void kMyMoneyAccountSelector::setOptimizedWidth(void)
{
  QListViewItemIterator it(m_listView, QListViewItemIterator::Selectable);
  QListViewItem* it_v;
  kMyMoneyListViewItem* it_l;
  kMyMoneyCheckListItem* it_c;

  // scan items
  int w = 0;
  QFontMetrics fm( KGlobalSettings::generalFont());
  while((it_v = it.current()) != 0) {
    it_l = dynamic_cast<kMyMoneyListViewItem*>(it_v);
    int nw = 0;
    if(it_l) {
      nw = it_l->width(fm, m_listView, 0);
    } else {
      it_c = dynamic_cast<kMyMoneyCheckListItem*>(it_v);
      if(it_c) {
        nw = it_c->width(fm, m_listView, 0);
      }
    }
    if(nw > w)
      w = nw;
    ++it;
  }
  m_listView->setMinimumWidth(w+30);
  m_listView->setMaximumWidth(w+30);
  m_listView->setColumnWidth(0, w+28);
}

const int kMyMoneyAccountSelector::loadList(KMyMoneyUtils::categoryTypeE typeMask)
{
  QValueList<int> typeList;

  if(typeMask & KMyMoneyUtils::asset) {
    typeList << MyMoneyAccount::Checkings;
    typeList << MyMoneyAccount::Savings;
    typeList << MyMoneyAccount::Cash;
    typeList << MyMoneyAccount::AssetLoan;
    typeList << MyMoneyAccount::CertificateDep;
    typeList << MyMoneyAccount::Investment;
    typeList << MyMoneyAccount::Stock;
    typeList << MyMoneyAccount::MoneyMarket;
    typeList << MyMoneyAccount::Asset;
    typeList << MyMoneyAccount::Currency;
  }
  if(typeMask & KMyMoneyUtils::liability) {
    typeList << MyMoneyAccount::CreditCard;
    typeList << MyMoneyAccount::Loan;
    typeList << MyMoneyAccount::Liability;
  }
  if(typeMask & KMyMoneyUtils::income) {
    typeList << MyMoneyAccount::Income;
  }
  if(typeMask & KMyMoneyUtils::expense) {
    typeList << MyMoneyAccount::Expense;
  }

  return loadList(typeList);
}

const int kMyMoneyAccountSelector::loadList(QValueList<int> typeList)
{
  QCStringList list;
  QCStringList::ConstIterator it_l;
  MyMoneyFile* file = MyMoneyFile::instance();
  int count = 0;
  int typeMask = 0;
  m_baseName = QString();
  QCString currentId;

  if(m_selMode == QListView::Single)
    currentId = selectedAccounts().first();

  if((typeList.contains(MyMoneyAccount::Checkings)
    + typeList.contains(MyMoneyAccount::Savings)
    + typeList.contains(MyMoneyAccount::Cash)
    + typeList.contains(MyMoneyAccount::AssetLoan)
    + typeList.contains(MyMoneyAccount::CertificateDep)
    + typeList.contains(MyMoneyAccount::Investment)
    + typeList.contains(MyMoneyAccount::Stock)
    + typeList.contains(MyMoneyAccount::MoneyMarket)
    + typeList.contains(MyMoneyAccount::Asset)
    + typeList.contains(MyMoneyAccount::Currency)) > 0)
    typeMask |= KMyMoneyUtils::asset;

  if((typeList.contains(MyMoneyAccount::CreditCard)
    + typeList.contains(MyMoneyAccount::Loan)
    + typeList.contains(MyMoneyAccount::Liability)) > 0)
    typeMask |= KMyMoneyUtils::liability;

  if((typeList.contains(MyMoneyAccount::Income)) > 0)
    typeMask |= KMyMoneyUtils::income;

  if((typeList.contains(MyMoneyAccount::Expense)) > 0)
    typeMask |= KMyMoneyUtils::expense;

  m_typeList = typeList;
  m_listView->clear();

  if(m_selMode == QListView::Multi) {
    m_incomeCategoriesButton->hide();
    m_expenseCategoriesButton->hide();
  }

  for(int mask = 0x01; mask != KMyMoneyUtils::last; mask <<= 1) {
    kMyMoneyCheckListItem* item = 0;
    if(typeMask & mask & KMyMoneyUtils::asset) {
      item = new kMyMoneyCheckListItem(m_listView, i18n("Asset accounts"), QCString(), QCheckListItem::Controller);
      list = file->asset().accountList();
    }

    if(typeMask & mask & KMyMoneyUtils::liability) {
      item = new kMyMoneyCheckListItem(m_listView, i18n("Liability accounts"), QCString(), QCheckListItem::Controller);
      list = file->liability().accountList();
    }

    if(typeMask & mask & KMyMoneyUtils::income) {
      item = new kMyMoneyCheckListItem(m_listView, i18n("Income categories"), QCString(), QCheckListItem::Controller);
      list = file->income().accountList();
      if(m_selMode == QListView::Multi) {
        m_incomeCategoriesButton->show();
      }
    }

    if(typeMask & mask & KMyMoneyUtils::expense) {
      item = new kMyMoneyCheckListItem(m_listView, i18n("Expense categories"), QCString(), QCheckListItem::Controller);
      list = file->expense().accountList();
      if(m_selMode == QListView::Multi) {
        m_expenseCategoriesButton->show();
      }
    }

    if(item != 0) {
      item->setSelectable(false);
      item->setOpen(true);
      // scan all matching accounts found in the engine
      for(it_l = list.begin(); it_l != list.end(); ++it_l) {
        ++count;
        MyMoneyAccount acc = file->account(*it_l);
        if(m_typeList.contains(acc.accountType())) {
          QListViewItem* subItem = newEntryFactory(item, acc.name(), acc.id());
          if(acc.accountList().count() > 0) {
            subItem->setOpen(true);
            count += loadSubAccounts(subItem, acc.accountList());
          }
        }
      }
    }
  }
  if(m_listView->firstChild()) {
    if(currentId.isEmpty()) {
      m_listView->setCurrentItem(m_listView->firstChild());
      m_listView->clearSelection();
    } else {
      setSelected(currentId);
    }
  }
  QWidget::update();
  return count;
}

const int kMyMoneyAccountSelector::loadList(const QString& baseName, const QValueList<QCString>& accountIdList, const bool clear)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  int count = 0;
  kMyMoneyCheckListItem* item = 0;
  m_typeList.clear();
  m_baseName = baseName;
  m_accountList = accountIdList;

  if(clear)
    m_listView->clear();

  item = new kMyMoneyCheckListItem(m_listView, baseName, QCString(), QCheckListItem::Controller);
  item->setSelectable(false);
  item->setOpen(true);

  QValueList<QCString>::ConstIterator it;
  for(it = accountIdList.begin(); it != accountIdList.end(); ++it)   {
    ++count;
    MyMoneyAccount acc = file->account(*it);
    newEntryFactory(item, acc.name(), acc.id());
  }

  if(m_listView->firstChild()) {
    m_listView->setCurrentItem(m_listView->firstChild());
    m_listView->clearSelection();
  }

  QWidget::update();
  return count;
}

int kMyMoneyAccountSelector::loadSubAccounts(QListViewItem* parent, const QCStringList& list)
{
  QCStringList::ConstIterator it_l;
  MyMoneyFile* file = MyMoneyFile::instance();
  int count = 0;

  for(it_l = list.begin(); it_l != list.end(); ++it_l) {
    ++count;
    MyMoneyAccount acc = file->account(*it_l);
    if(m_typeList.contains(acc.accountType())) {
      QListViewItem* item = newEntryFactory(parent, acc.name(), acc.id());
      if(acc.accountList().count() > 0) {
        item->setOpen(true);
        count += loadSubAccounts(item, acc.accountList());
      }
    }
  }
  return count;
}

const bool kMyMoneyAccountSelector::allAccountsSelected(void) const
{
  QListViewItem* it_v;

  if(m_selMode == QListView::Single)
    return false;

  for(it_v = m_listView->firstChild(); it_v != 0; it_v = it_v->nextSibling()) {
    if(it_v->rtti() == 1) {
      QCheckListItem* it_c = static_cast<QCheckListItem*>(it_v);
      if(it_c->type() == QCheckListItem::CheckBox) {
        if(!(it_c->isOn() && allAccountsSelected(it_v)))
          return false;
      } else {
        if(!allAccountsSelected(it_v))
          return false;
      }
    }
  }
  return true;
}

bool kMyMoneyAccountSelector::allAccountsSelected(const QListViewItem *item) const
{
  QListViewItem* it_v;

  for(it_v = item->firstChild(); it_v != 0; it_v = it_v->nextSibling()) {
    if(it_v->rtti() == 1) {
      QCheckListItem* it_c = static_cast<QCheckListItem*>(it_v);
      if(!(it_c->isOn() && allAccountsSelected(it_v)))
        return false;
    }
  }
  return true;
}


void kMyMoneyAccountSelector::selectAllAccounts(const bool state)
{
  QListViewItem* it_v;

  for(it_v = m_listView->firstChild(); it_v != 0; it_v = it_v->nextSibling()) {
    if(it_v->rtti() == 1) {
      QCheckListItem* it_c = static_cast<QCheckListItem*>(it_v);
      if(it_c->type() == QCheckListItem::CheckBox) {
        it_c->setOn(state);
      }
      selectAllSubAccounts(it_v, state);
    }
  }
  emit stateChanged();
}

void kMyMoneyAccountSelector::selectAccounts(const QCStringList& accountlist, const bool state)
{
  QListViewItem* it_v;

  for(it_v = m_listView->firstChild(); it_v != 0; it_v = it_v->nextSibling()) {
    if(it_v->rtti() == 1) {
      // TOM, why were you using static_cast for these? (ace)
      kMyMoneyCheckListItem* it_c = dynamic_cast<kMyMoneyCheckListItem*>(it_v);
      if(it_c->type() == QCheckListItem::CheckBox && accountlist.contains(it_c->id())) {
        it_c->setOn(state);
      }
      selectSubAccounts(it_v, accountlist, state);
    }
  }
  emit stateChanged();
}

void kMyMoneyAccountSelector::selectSubAccounts(QListViewItem* item, const QCStringList& accountlist, const bool state)
{
  QListViewItem* it_v;

  for(it_v = item->firstChild(); it_v != 0; it_v = it_v->nextSibling()) {
    if(it_v->rtti() == 1) {
      // TOM, why were you using static_cast for these? (ace)
      kMyMoneyCheckListItem* it_c = dynamic_cast<kMyMoneyCheckListItem*>(it_v);
      if(it_c->type() == QCheckListItem::CheckBox && accountlist.contains(it_c->id())) {
        it_c->setOn(state);
      }
      selectSubAccounts(it_v, accountlist, state);
    }
  }
}

void kMyMoneyAccountSelector::selectAllSubAccounts(QListViewItem* item, const bool state)
{
  QListViewItem* it_v;

  for(it_v = item->firstChild(); it_v != 0; it_v = it_v->nextSibling()) {
    if(it_v->rtti() == 1) {
      QCheckListItem* it_c = static_cast<QCheckListItem*>(it_v);
      if(it_c->type() == QCheckListItem::CheckBox) {
        it_c->setOn(state);
      }
      selectAllSubAccounts(it_v, state);
    }
  }
}

void kMyMoneyAccountSelector::selectCategories(const bool income, const bool expense)
{
  QListViewItem* it_v;

  for(it_v = m_listView->firstChild(); it_v != 0; it_v = it_v->nextSibling()) {
    if(static_cast<QCheckListItem*>(it_v)->text() == i18n("Income categories"))
      selectAllSubAccounts(it_v, income);
    else
      selectAllSubAccounts(it_v, expense);
  }
  emit stateChanged();
}

const QCStringList kMyMoneyAccountSelector::selectedAccounts(void) const
{
  QListViewItem*  it_v;
  QCStringList    list;

  if(m_selMode == QListView::Single) {
    kMyMoneyListViewItem* it_c = static_cast<kMyMoneyListViewItem*>(m_listView->selectedItem());
    if(it_c != 0)
      list << it_c->id();

  } else {
    for(it_v = m_listView->firstChild(); it_v != 0; it_v = it_v->nextSibling()) {
      if(it_v->rtti() == 1) {
        kMyMoneyCheckListItem* it_c = static_cast<kMyMoneyCheckListItem*>(it_v);
        if(it_c->type() == QCheckListItem::CheckBox) {
          if(it_c->isOn())
            list << (*it_c).id();
        }
        selectedAccounts(list, it_v);
      }
    }
  }
  return list;
}

const QCStringList kMyMoneyAccountSelector::accountList(const  QValueList<MyMoneyAccount::accountTypeE>& filterList) const
{
  QCStringList    list;
  QListViewItemIterator it;
  QListViewItem* it_v;
  QValueList<MyMoneyAccount::accountTypeE>::ConstIterator it_f;

#if QT_VERSION >= 0x030201
  it = QListViewItemIterator(m_listView, QListViewItemIterator::Selectable);
  while((it_v = it.current()) != 0) {
    {
#else
  it = QListViewItemIterator(m_listView);
  while((it_v = it.current()) != 0) {
    if(it_v->isSelectable()) {
#endif
      if(it_v->rtti() == 1) {
        kMyMoneyCheckListItem* it_c = static_cast<kMyMoneyCheckListItem*>(it_v);
        if(it_c->type() == QCheckListItem::CheckBox) {
          MyMoneyAccount acc = MyMoneyFile::instance()->account(it_c->id());
          it_f = filterList.find(acc.accountType());
          if(filterList.count() == 0 || it_f != filterList.end())
            list << it_c->id();
        }
      } else if(it_v->rtti() == 0) {
        kMyMoneyListViewItem* it_c = static_cast<kMyMoneyListViewItem*>(it_v);
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

void kMyMoneyAccountSelector::selectedAccounts(QCStringList& list, QListViewItem* item) const
{
  QListViewItem* it_v;

  for(it_v = item->firstChild(); it_v != 0; it_v = it_v->nextSibling()) {
    if(it_v->rtti() == 1) {
      kMyMoneyCheckListItem* it_c = static_cast<kMyMoneyCheckListItem*>(it_v);
      if(it_c->type() == QCheckListItem::CheckBox) {
        if(it_c->isOn())
          list << (*it_c).id();
        selectedAccounts(list, it_v);
      }
    }
  }
}

void kMyMoneyAccountSelector::setSelected(const QCString& id, const bool state)
{
  QListViewItem* it_v;

  if(id.isEmpty() && m_selMode == QListView::Single) {
    QListViewItem* item = m_listView->selectedItem();
    if(item)
      m_listView->setSelected(item, false);
    return;
  }

  for(it_v = m_listView->firstChild(); it_v != 0; it_v = it_v->nextSibling()) {
    if(it_v->rtti() == 1) {
      kMyMoneyCheckListItem* it_c = dynamic_cast<kMyMoneyCheckListItem*>(it_v);
      Q_CHECK_PTR(it_c);
      if(it_c->type() == QCheckListItem::CheckBox) {
        if(it_c->id() == id) {
          it_c->setOn(state);
          m_listView->setSelected(it_v, true);
          ensureItemVisible(it_v);
          return;
        }
      }
    } else if(it_v->rtti() == 0) {
      kMyMoneyListViewItem* it_c = dynamic_cast<kMyMoneyListViewItem*>(it_v);
      Q_CHECK_PTR(it_c);
      if(it_c->id() == id) {
        m_listView->setSelected(it_v, true);
        ensureItemVisible(it_v);
        return;
      }
    }
    setSelected(it_v, id, state);
  }
}

void kMyMoneyAccountSelector::setSelected(QListViewItem* item, const QCString& id, const bool state)
{
  QListViewItem* it_v;

  for(it_v = item->firstChild(); it_v != 0; it_v = it_v->nextSibling()) {
    if(it_v->rtti() == 1) {
      kMyMoneyCheckListItem* it_c = dynamic_cast<kMyMoneyCheckListItem*>(it_v);
      Q_CHECK_PTR(it_c);
      if(it_c->id() == id) {
        it_c->setOn(state);
        m_listView->setSelected(it_v, true);
        ensureItemVisible(it_v);
        return;
      }
    } else if(it_v->rtti() == 0) {
      kMyMoneyListViewItem* it_c = dynamic_cast<kMyMoneyListViewItem*>(it_v);
      Q_CHECK_PTR(it_c);
      if(it_c->id() == id) {
        m_listView->setSelected(it_v, true);
        ensureItemVisible(it_v);
        return;
      }
    }
    setSelected(it_v, id, state);
  }
}


void kMyMoneyAccountSelector::ensureItemVisible(const QListViewItem *it_v)
{
  // for some reason, I could only use the ensureItemVisible() method
  // of QListView successfully, after the widget was drawn on the screen.
  // If called before it had no effect (if the item was not visible).
  //
  // The solution was to store the item we wanted to see in a local var
  // and call QListView::ensureItemVisible() about 10ms later in
  // the slot slotShowSelected.  (ipwizard, 12/29/2003)
  m_visibleItem = it_v;
  QTimer::singleShot(10, this, SLOT(slotShowSelected()));
}

void kMyMoneyAccountSelector::slotShowSelected(void)
{
  m_listView->ensureItemVisible(m_visibleItem);
}

void kMyMoneyAccountSelector::update(const QCString& /* id */)
{
  QListViewItem* it_v = m_listView->currentItem();
  QCString previousHighlighted;
  bool state = false;

  if(m_selMode == QListView::Multi && it_v) {
    if(it_v->rtti() == 1) {
      kMyMoneyCheckListItem* it_c = static_cast<kMyMoneyCheckListItem*>(it_v);
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

int kMyMoneyAccountSelector::slotMakeCompletion(const QString& txt)
{
#if QT_VERSION >= 0x030201
  QListViewItemIterator it(m_listView, QListViewItemIterator::Selectable);
#else
  QListViewItemIterator it(m_listView);
#endif

  QListViewItem* it_v;

  // The logic used here seems to be awkward. The problem is, that
  // QListViewItem::setVisible works recursively on all it's children
  // and grand-children.
  //
  // The way out of this is as follows: Make all items visible.
  // Then go through the list again and perform the checks.
  // If an item does not have any children (last leaf in the tree view)
  // perform the check. Then check recursively on the parent of this
  // leaf that it has no visible children. If that is the case, make the
  // parent invisible and continue this check with it's parent.
  while((it_v = it.current()) != 0) {
    it_v->setVisible(true);
    ++it;
  }

  if(!txt.isEmpty()) {
#if QT_VERSION >= 0x030201
    it = QListViewItemIterator(m_listView, QListViewItemIterator::Selectable);
    while((it_v = it.current()) != 0) {
#else
    it = QListViewItemIterator(m_listView);
    while((it_v = it.current()) != 0)
     if(!it_v->isSelectable())
       ++it;
     else {
#endif
      if(it_v->firstChild() == 0) {
        if(it_v->text(0).contains(txt, false) == 0) {
          // this is a node which does not contain the
          // text and does not have children. So we can
          // safely hide it. Then we check, if the parent
          // has more children which are still visible. If
          // none are found, the parent node is hidden also. We
          // continue until the top of the tree or until we
          // find a node that still has visible children.
          bool hide = true;
          while(hide) {
            it_v->setVisible(false);
            it_v = it_v->parent();
            if(it_v && it_v->isSelectable()) {
              hide = (it_v->text(0).contains(txt, false) == 0);
              QListViewItem* child = it_v->firstChild();
              for(; child && hide; child = child->nextSibling()) {
                if(child->isVisible())
                  hide = false;
              }
            } else
              hide = false;
          }
        }
        ++it;

      } else if(it_v->text(0).contains(txt, false) != 0) {
        // a node with children contains the text. We want
        // to display all child nodes in this case, so we need
        // to advance the iterator to the next sibling of the
        // current node. This could well be the sibling of a
        // parent or grandparent node.
        QListViewItem* curr = it_v;
        QListViewItem* item;
        while((item = curr->nextSibling()) == 0) {
          curr = curr->parent();
          if(curr == 0)
            break;
        }
        do {
          ++it;
        } while(it.current() && it.current() != item);

      } else {
        // It's a node with children that does not match. We don't
        // change it's status here.
        ++it;
      }
    }
  }

  // Get the number of visible nodes for the return code
  int cnt = 0;

#if QT_VERSION >= 0x030201
  it = QListViewItemIterator(m_listView, QListViewItemIterator::Selectable | QListViewItemIterator::Visible);
  while((it_v = it.current()) != 0) {
    cnt++;
    it++;
  }
#else
  it = QListViewItemIterator(m_listView);
  while((it_v = it.current()) != 0) {
    if(it_v->isSelectable() && it_v->isVisible())
      cnt++;
    it++;
  }
#endif
  return cnt;
}

void kMyMoneyAccountSelector::slotListRightMouse(QListViewItem* it_v, const QPoint& pos, int /* col */)
{
  if(it_v->rtti() == 1) {
    kMyMoneyCheckListItem* it_c = static_cast<kMyMoneyCheckListItem*>(it_v);
    if(it_c->type() == QCheckListItem::CheckBox) {
      // the following is copied from QCheckListItem::activate() et al
      int boxsize = m_listView->style().pixelMetric(QStyle::PM_CheckListButtonSize, m_listView);
      int align = m_listView->columnAlignment( 0 );
      int marg = m_listView->itemMargin();
      int y = 0;

      if ( align & AlignVCenter )
        y = ( ( height() - boxsize ) / 2 ) + marg;
      else
        y = (m_listView->fontMetrics().height() + 2 + marg - boxsize) / 2;

      QRect r( 0, y, boxsize-3, boxsize-3 );
      // columns might have been swapped
      r.moveBy( m_listView->header()->sectionPos( 0 ), 0 );

      QPoint topLeft = m_listView->itemRect(it_v).topLeft(); //### inefficient?
      QPoint p = m_listView->mapFromGlobal( pos ) - topLeft;

      int xdepth = m_listView->treeStepSize() * (it_v->depth() + (m_listView->rootIsDecorated() ? 1 : 0))
                   + m_listView->itemMargin();
      xdepth += m_listView->header()->sectionPos( m_listView->header()->mapToSection( 0 ) );
      p.rx() -= xdepth;
      // copy ends around here

      if ( r.contains( p ) ) {
        // we get down here, if we have a right click onto the checkbox
        selectAllSubAccounts(it_c, it_c->isOn());
      }
    }
  }
}

#include "kmymoneyaccountselector.moc"
