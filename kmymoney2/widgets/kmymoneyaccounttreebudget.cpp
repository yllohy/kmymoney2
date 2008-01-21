/***************************************************************************
                         kmymoneyaccounttreebudget.cpp  -  description
                            -------------------
   begin                : Tue Feb 21 2006
   copyright            : (C) 2005 by Darren Gould
   email                : Darren Gould <darren_gould@gmx.de>
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

#include <qpoint.h>
#include <qevent.h>
#include <qdragobject.h>
#include <qtimer.h>
#include <qcursor.h>
#include <qheader.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qstyle.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kmessagebox.h>
#include <klocale.h>
#include <kglobal.h>
#include <kstandarddirs.h>

// ----------------------------------------------------------------------------
// Project Includes
#include <kmymoneyaccounttreebudget.h>

KMyMoneyAccountTreeBudget::KMyMoneyAccountTreeBudget(QWidget* parent, const char* name) :
  KMyMoneyAccountTree::KMyMoneyAccountTree(parent, name)
{
}

KMyMoneyAccountTreeBudgetItem* KMyMoneyAccountTreeBudget::selectedItem(void) const
{
  return dynamic_cast<KMyMoneyAccountTreeBudgetItem *>(KMyMoneyAccountTree::selectedItem());
}

void KMyMoneyAccountTreeBudget::slotSelectObject(const QListViewItem* i)
{
  emit selectObject(MyMoneyInstitution());
  emit selectObject(MyMoneyAccount());

  const KMyMoneyAccountTreeItem* item = dynamic_cast<const KMyMoneyAccountTreeItem*>(i);
  if(item) {
    emit KMyMoneyAccountTree::openObject(item->itemObject());
  }
}


KMyMoneyAccountTreeBudgetItem* KMyMoneyAccountTreeBudget::findItem(const QCString& id)
{
  // tried to use a  QListViewItemIterator  but that does not fit
  // with the constness of this method. Arghhh.

  QListViewItem* p = firstChild();
  while(p) {
    // item found, check for the id
    KMyMoneyAccountTreeBudgetItem* item = dynamic_cast<KMyMoneyAccountTreeBudgetItem*>(p);
    if(item && item->id() == id)
      break;

    // item did not match, search the next one
    QListViewItem* next = p->firstChild();
    if(!next) {
      while((next = p->nextSibling()) == 0) {
        p = p->parent();
        if(!p)
          break;
      }
    }
    p = next;
  }

  return dynamic_cast<KMyMoneyAccountTreeBudgetItem*>(p);
}


/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

KMyMoneyAccountTreeBudgetItem::KMyMoneyAccountTreeBudgetItem(KListView *parent, const MyMoneyAccount& account, const MyMoneyBudget &budget, const MyMoneySecurity& security, const QString& name) :
  KMyMoneyAccountTreeItem(parent, account, security, name),
  m_budget(budget)
{
  updateAccount(account, true);
}

KMyMoneyAccountTreeBudgetItem::KMyMoneyAccountTreeBudgetItem(KMyMoneyAccountTreeBudgetItem *parent, const MyMoneyAccount& account, const MyMoneyBudget& budget, const QValueList<MyMoneyPrice>& price, const MyMoneySecurity& security) :
  KMyMoneyAccountTreeItem(parent, account, price, security),
  m_budget(budget)
{
  updateAccount(account, true);
}


KMyMoneyAccountTreeBudgetItem::~KMyMoneyAccountTreeBudgetItem()
{
}

void KMyMoneyAccountTreeBudgetItem::setBudget(const MyMoneyBudget& budget)
{
  m_budget = budget;
  updateAccount(m_account);
}

void KMyMoneyAccountTreeBudgetItem::updateAccount(const MyMoneyAccount& account, bool forceTotalUpdate)
{
  // make sure it's for the same object
  if(account.id() != m_account.id())
    return;

  QString icon;
  switch (m_account.accountGroup())
  {
    case MyMoneyAccount::Income:
      icon = "account-types_income";
      break;
    case MyMoneyAccount::Expense:
      icon = "account-types_expense";
      break;
    case MyMoneyAccount::Liability:
      icon = "account-types_liability";
      break;
    case MyMoneyAccount::Asset:
      icon = "account-types_asset";
      break;
    default:
      icon = "account";
  }
  if(m_account.isClosed()) {
    QPixmap pic = QPixmap(KGlobal::dirs()->findResource("appdata",QString( "icons/hicolor/22x22/actions/%1.png").arg(icon)));
    QPixmap closed = QPixmap(KGlobal::dirs()->findResource("appdata",QString( "icons/hicolor/22x22/actions/account-types_closed.png")));
    bitBlt(&pic, 0, 0, &closed, 0, 0, closed.width(), closed.height(), Qt::CopyROP, false);
    setPixmap(0, pic);
  } else
    setPixmap(0, QPixmap(KGlobal::dirs()->findResource("appdata",QString( "icons/hicolor/22x22/actions/%1.png").arg(icon))));

  setText(KMyMoneyAccountTree::NameColumn, account.name());

  // make sure we have the right parent object
  // for the extended features
  KMyMoneyAccountTree* lv = listView();
  if(!lv)
    return;

  MyMoneyMoney oldValue = m_value;
  m_account = account;
  // find out if the account is budgeted
  MyMoneyBudget::AccountGroup budgetAccount = m_budget.account( account.id() );
  m_balance = MyMoneyMoney();
  if ( budgetAccount.id() == account.id() )
    m_balance = budgetAccount.balance();

  switch(budgetAccount.budgetLevel()) {
    case MyMoneyBudget::AccountGroup::eMonthly:
      m_balance = m_balance * 12;
      break;

    default:
      break;
  }

  // calculate the new value by running down the price list
  m_value = m_balance;

  QValueList<MyMoneyPrice>::const_iterator it_p;
  QCString security = m_security.id();
  for(it_p = m_price.begin(); it_p != m_price.end(); ++it_p) {
    m_value = m_value * (MyMoneyMoney(1,1) / (*it_p).rate(security));
    if((*it_p).from() == security)
      security = (*it_p).to();
    else
      security = (*it_p).from();
  }

  // check if we need to update the display of values
  if(parent() && (isOpen() || m_account.accountList().count() == 0)) {
    if(m_security.id() != listView()->baseCurrency().id())
    {
      QString strAmount = m_balance.formatMoney(m_security.tradingSymbol(), MyMoneyMoney::denomToPrec(m_security.smallestAccountFraction())) + "  ";
      setText(KMyMoneyAccountTree::BalanceColumn, strAmount);
    }
    setText(KMyMoneyAccountTree::ValueColumn, m_value.formatMoney(listView()->baseCurrency().tradingSymbol(),   MyMoneyMoney::denomToPrec(listView()->baseCurrency().smallestAccountFraction())) + "  ");
  }

  // check if we need to tell upstream account objects in the tree
  // that the value has changed
  if(oldValue != m_value || forceTotalUpdate) {
    adjustTotalValue(m_value - oldValue);
    lv->emitValueChanged();
  }
}

#include "kmymoneyaccounttreebudget.moc"
