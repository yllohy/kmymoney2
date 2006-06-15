/***************************************************************************
                         kmymoneyaccounttreebudget.h  -  description
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

#ifndef KMYMONEYACCOUNTTREEBUDGET_H
#define KMYMONEYACCOUNTTREEBUDGET_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qtimer.h>
class QDragObject;

// ----------------------------------------------------------------------------
// KDE Includes

#include <klistview.h>

// ----------------------------------------------------------------------------
// Project Includes
#include "kmymoneyaccounttree.h"
#include "../../kmymoney/mymoneybudget.h"

class KMyMoneyAccountTreeBudgetItem;

class KMyMoneyAccountTreeBudget : public KMyMoneyAccountTree
{
  Q_OBJECT
public:
  KMyMoneyAccountTreeBudget(QWidget* parent = 0, const char *name = 0);
  virtual ~KMyMoneyAccountTreeBudget() {};

  /**
    * overridden from base class implementation to return a pointer
    * to a KMyMoneyAccountTreeItem.
    *
    * @return pointer to currently selected item
    */
  KMyMoneyAccountTreeBudgetItem* selectedItem(void) const;
  KMyMoneyAccountTreeBudgetItem* findItem(const QCString& id);

public slots:
  void slotSelectObject(const QListViewItem* i);

};

class KMyMoneyAccountTreeBudgetItem : public KMyMoneyAccountTreeItem
{
public:

  /**
    * Constructor to be used to construct an account
    * entry object for a budget.
    *
    * @param parent pointer to the parent KAccountListView object this entry should be
    *               added to.
    * @param account const reference to MyMoneyAccount for which
    *               the KListView entry is constructed
    * @param budget const reference to the budget to
    * which the account belongs
    * @param price price to be used to calculate value (defaults to 1)
    *              This is used for accounts denominated in foreign currencies or stocks
    * @param security const reference to the security used to show the value. Usually
    *                 one should pass MyMoneyFile::baseCurrency() here.
    */
  KMyMoneyAccountTreeBudgetItem(KMyMoneyAccountTreeBudgetItem *parent, const MyMoneyAccount& account, const MyMoneyBudget& budget, const QValueList<MyMoneyPrice>& price = QValueList<MyMoneyPrice>(), const MyMoneySecurity& security = MyMoneySecurity());

  /**
    * Constructor to be used to construct an account
    * entry object for a budget.
    *
    * @param parent pointer to the parent KAccountListView object this entry should be
    *               added to.
    * @param account const reference to MyMoneyAccount for which
    *               the KListView entry is constructed
    * @param budget const reference to the budget to
    * which the account belongs
    * @param security const reference to the security used to show the value. Usually
    *                 one should pass MyMoneyFile::baseCurrency() here.
    * @param name name of the account to be used instead of the one stored with @p account
    *               If empty, the one stored with @p account will be used. Default: empty
    */
   KMyMoneyAccountTreeBudgetItem(KListView *parent, const MyMoneyAccount& account, const MyMoneyBudget &budget, const MyMoneySecurity& security = MyMoneySecurity(), const QString& name = QString());

  ~KMyMoneyAccountTreeBudgetItem();

  /**
    * This method is loads new information into the item and updates the fields
    *
    * @param account the account data for the object to be updated
    * @param forceTotalUpdate set to true to force update of total values
    *                         (used in constructor, should not be necessary to
    *                          be set by application code)
    *
    * @note if account.id() is not equal to the current account id
    *       then this method returns immediately
    */
  void updateAccount(const MyMoneyAccount& account, bool forceTotalUpdate = false);

  MyMoneyMoney balance(void);
  QString tradingSymbol();

private:
  MyMoneyBudget m_budget;

  MyMoneyMoney                      m_balance;
  MyMoneyMoney                      m_value;
  QValueList<MyMoneyPrice>          m_price;
  MyMoneySecurity                   m_security;
  MyMoneyMoney                      m_totalValue;
  MyMoneyMoney                      m_displayFactor;

  MyMoneyAccount                    m_account;
  MyMoneyInstitution                m_institution;

  KMyMoneyAccountTreeItemType       m_type;
};

#endif

