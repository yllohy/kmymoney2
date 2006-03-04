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
#include "../widgets/kmymoneyaccounttreebudget.h"

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

  MyMoneyBudget& budget(void) { return m_budget; };

private:
  MyMoneyBudget  m_budget;
};

/**
  * @author Darren Gould
  */

/**
  * This class represents an item in the BudgetAmount list view.
  */
class KBudgetAmountListItem : public KListViewItem
{
public:
  /**
    * Constructor to be used to construct a BudgetAmount entry object.
    *
    * @param parent pointer to the KListView object this entry should be
    *               added to.
    * @param amount const reference to MyMoneyMoney for which
    *               the KListView entry is constructed
    * @param date   const reference to QDate for the budgeted item
    */
  KBudgetAmountListItem(KListView *parent, const MyMoneyMoney& amount, const QDate &date);

  /**
    * Constructor to be used to construct a BudgetAmount entry object.
    *
    * @param parent pointer to the KListView object this entry should be
    *               added to.
    * @param amount const reference to MyMoneyMoney for which
    *               the KListView entry is constructed
    * @param date   const reference to QDate for the budgeted item
    * @param label  const reference to QString for a label for the item
    */
  KBudgetAmountListItem(KListView *parent, const MyMoneyMoney& amount, const QDate &date, const QString &label);

  ~KBudgetAmountListItem();

  /**
    * This method is re-implemented from QListViewItem::paintCell().
    * Besides the standard implementation, the QPainter is set
    * according to the applications settings.
    */
  void paintCell(QPainter *p, const QColorGroup & cg, int column, int width, int align);
  void setAmount( const QString &newamount);

  MyMoneyMoney& amount(void) { return m_amount; };
  QDate&        date(void)   { return m_date; };

private:
  MyMoneyMoney  m_amount;
  QDate         m_date;
  QString       m_label;
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
  void slotSelectBudget(void);
  void slotStartRename(void);
  void slotBudgetApply(void);

protected:
  void resizeEvent(QResizeEvent*);
  void loadAccounts(void);
  bool loadSubAccounts(KMyMoneyAccountTreeBudgetItem* parent, const QCStringList& accountList, const MyMoneyBudget& budget);
  void loadBudget(void);
  void ensureBudgetVisible(const QCString& id);
  bool selectedBudget(MyMoneyBudget& budget) const;

protected slots:

  /**
    * This slot is called when the name of a budget is changed inside
    * the budget list view and only a single budget is selected.
    *
    * @param p The listviewitem containing the budget name
    * @param col The column where the name is located
    * @param txt The text of the new name
    */
  void slotRenameBudget(QListViewItem *p, int col, const QString& txt);

  /**
    * This slot is called when the name of a budget is changed inside
    * the budget list view and only a single budget is selected.
    */
  void slotBudgetedAmount(QListViewItem *p, int col, const QString& newamount);

  /**
    * This slot is called when the year of a budget is changed inside
    * the budget list view and only a single budget is selected.
    */
  void slotSelectYear(int iYear);

  /**
    * This slot is called when the time span of a budget is changed
    */
  void slotSelectTimeSpan(int iTimeSpan);


  /**
    * Select the object pointed to by @p i. This slot emits selectObject signals
    * with an emtpy MyMoneyAccount and an empty MyMoneyInstitution object
    * to deselect current selections. If @p i points to a KMyMoneyAccountTreeItem
    * object, it emits selectObject() for this item.
    *
    * @param i pointer to QListViewItem of object to be selected
    */
  void slotSelectObject(QListViewItem *i);

private slots:
  void slotRearrange(void);

  /**
    * This slot receives the signal from the listview control that an item was right-clicked,
    * If @p item points to a real payee item, emits openContextMenu().
    *
    * @param i the item on which the cursor resides
    */
  void slotOpenContextMenu(QListViewItem* i);

signals:
  void signalViewActivated();
  /**
    * This signal serves as proxy for KMyMoneyBudgetList::selectObject()
    */
  void openContextMenu(const MyMoneyObject& obj);
  void selectObjects(const QValueList<MyMoneyBudget>& budget);

private:
  typedef enum {
    eNone=-1,
    eYearly=0,
    eMonthly=1,
    eMonthByMonth=2
  } eTimePeriodColumn;

  /**
    * This member holds the state of the toggle switch used
    * to suppress updates due to MyMoney engine data changes
    */
  bool m_suspendUpdate;
  QMap<QCString, MyMoneyAccount>      m_accountMap;

  QMap<QCString, MyMoneySecurity>     m_securityMap;
  QMap<QCString, unsigned long>       m_transactionCountMap;
  QStringList                         m_yearList;

  KMyMoneyAccountTreeBudgetItem*            m_incomeItem;
  KMyMoneyAccountTreeBudgetItem*            m_expenseItem;
  KMyMoneyAccountTreeBudgetItem*            m_currentAccountItem;

  /// set if a view needs to be reloaded during show()
  bool                                m_needReload;

  static const int m_iBudgetYearsAhead;
  static const int m_iBudgetYearsBack;
};

#endif
