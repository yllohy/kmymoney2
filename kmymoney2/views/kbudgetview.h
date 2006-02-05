/***************************************************************************
                          kbudgetview.h
                          -------------
    begin                : Thu Jan 24 2002
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

#ifndef KBUDGETVIEW_H
#define KBUDGETVIEW_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qwidget.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klistview.h>
#include <kpopupmenu.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kbudgetviewdecl.h"
#include "kledgerview.h"
#include "../mymoney/mymoneyobserver.h"
#include "../mymoney/mymoneybudget.h"
#include "../mymoney/mymoneysecurity.h"
#include "../widgets/kmymoneyaccounttree.h"

/**
  * @author Darren Gould
  */

/**
  * This class represents an item in the budgets list view.
  */
class KBudgetListItem : public KListViewItem
{
public:
  /**
    * Constructor to be used to construct a budget entry object.
    *
    * @param parent pointer to the KListView object this entry should be
    *               added to.
    * @param budget const reference to MyMoneyBudget for which
    *               the KListView entry is constructed
    */
  KBudgetListItem(KListView *parent, const MyMoneyBudget& budget);
  ~KBudgetListItem();

  /**
    * This method is re-implemented from QListViewItem::paintCell().
    * Besides the standard implementation, the QPainter is set
    * according to the applications settings.
    */
  void paintCell(QPainter *p, const QColorGroup & cg, int column, int width, int align);

  const MyMoneyBudget& budget(void) const { return m_budget; };

private:
  MyMoneyBudget  m_budget;
};



/**
  *@author Darren Gould
  */

class KBudgetView : public KBudgetViewDecl
{
   Q_OBJECT
public:
  KBudgetView(QWidget *parent=0, const char *name=0);
  ~KBudgetView();
  void show();
  /**
    * This method is used to suppress updates for specific times
    * (e.g. during creation of a new MyMoneyFile object when the
    * default accounts are loaded). The behaviour of update() is
    * controlled with the parameter.
    *
    * @param suspend Suspend updates or not. Possible values are
    *
    * @li true updates are suspended
    * @li false updates will be performed immediately
    *
    * When a true/false transition of the parameter between
    * calls to this method is detected,
    * refresh() will be invoked once automatically.
    */
  void suspendUpdate(const bool suspend);

public slots:
  void slotReloadView(void);
  void slotRefreshView(void);

protected:
  void resizeEvent(QResizeEvent*);
  void loadAccounts(void);
  bool loadSubAccounts(KMyMoneyAccountTreeItem* parent, const QCStringList& accountList);
  void loadBudget(void);
  void ensureBudgetVisible(const QCString& id);

protected slots:

  /**
    * This slot is called when the name of a budget is changed inside
    * the budget list view and only a single budget is selected.
    */
  void slotRenameBudget(QListViewItem *p, int col, const QString& txt);

private slots:
  void rearrange(void);

  /**
    * This slot receives the signal from the listview control that an item was right-clicked,
    * If @p item points to a real payee item, emits openContextMenu().
    *
    * @param item the item on which the cursor resides
    */
  void slotOpenContextMenu(QListViewItem* i);

signals:
  void signalViewActivated();
  /**
    * This signal serves as proxy for KMyMoneyAccountTree::selectObject()
    */
  void selectObject(const MyMoneyObject&);
  void openContextMenu(const MyMoneyObject& obj);

private:
  /**
    * This member holds the state of the toggle switch used
    * to suppress updates due to MyMoney engine data changes
    */
  bool m_suspendUpdate;
  QMap<QCString, MyMoneyAccount>      m_accountMap;

  QMap<QCString, MyMoneySecurity>     m_securityMap;
  QMap<QCString, unsigned long>       m_transactionCountMap;

  KMyMoneyAccountTreeItem*            m_incomeItem;
  KMyMoneyAccountTreeItem*            m_expenseItem;

  /// set if a view needs to be reloaded during show()
  bool                                m_needReload;

  MyMoneyBudget m_budget;
};

#endif
