/***************************************************************************
                             kaccountssview.h
                             -------------------
    copyright            : (C) 2005 by Thomas Baumgart
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

#ifndef KACCOUNTSSVIEW_H
#define KACCOUNTSSVIEW_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <kiconview.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/mymoneyaccount.h>
#include <kmymoney/kmymoneyaccounttree.h>
#include <kmymoney/mymoneyutils.h>

#include "../views/kaccountsviewdecl.h"

/**
  * @author Thomas Baumgart
  */

/**
  * This class represents an item in the account icon view. It is used
  * by the KAccountsView to select between the accounts using icons.
  */
class KMyMoneyAccountIconItem : public KIconViewItem
{
public:
  /**
    * Constructor to be used to construct an account icon object.
    *
    * @param parent pointer to the KIconView object this entry should be
    *               added to.
    * @param account const reference to MyMoneyAccount for which
    *               the KIconView entry is constructed
    */
  KMyMoneyAccountIconItem(QIconView *parent, const MyMoneyAccount& account);
  ~KMyMoneyAccountIconItem();

  const MyMoneyObject& itemObject(void) const { return m_account; };

private:
  MyMoneyAccount        m_account;
};




/**
  * This class implements the accounts hierarchical and iconic 'view'.
  */
class KAccountsView : public KAccountsViewDecl
{
  Q_OBJECT
private:

public:
  KAccountsView(QWidget *parent=0, const char *name=0);
  virtual ~KAccountsView();

public slots:
  void slotLoadAccounts(void);

  /**
    * Override the base class behaviour to include all updates that
    * happened in the meantime.
    */
  void show(void);

  /**
    * update the account objects if their icon position has changed since
    * the last time.
    *
    * @param action must be KMyMoneyView::preSave, otherwise this slot is a NOP.
    */
  void slotUpdateIconPos(unsigned int action);

protected:
  typedef enum {
    ListView = 0,
    IconView,
    // insert new values above this line
    MaxViewTabs
  } AccountsViewTab;

  /**
    * This method loads the accounts for the respective tab.
    *
    * @param tab which tab should be loaded
    */
  void loadAccounts(AccountsViewTab tab);
  void loadListView(void);
  void loadIconView(void);

  bool loadSubAccounts(KMyMoneyAccountTreeItem* parent, const QCStringList& accountList);

  void viewChanged(void);

  /**
    * This method returns a pointer to the currently selected
    * account icon or 0 if no icon is selected.
    */
  KMyMoneyAccountIconItem* selectedIcon(void) const;

  QPoint point(const QString& str) const;
  QString point(const QPoint& val) const;

protected slots:
  void slotUpdateNetWorth(void);
  void slotTabChanged(QWidget*);
  void slotSelectIcon(QIconViewItem* item);
  void slotOpenContext(QIconViewItem* item);
  void slotOpenObject(QIconViewItem* item);

signals:
  /**
    * This signal serves as proxy for KMyMoneyAccountTree::selectObject()
    *
    * @param obj const reference to object
    */
  void selectObject(const MyMoneyObject& obj);

  /**
    * This signal serves as proxy for
    * KMyMoneyAccountTree::openContextMenu(const MyMoneyObject&)
    *
    * @param obj const reference to object
    */
  void openContextMenu(const MyMoneyObject& obj);

  /**
    * This signal will be emitted when the left mouse button is double
    * clicked (actually the KDE executed setting is used) on an object.
    *
    * @param obj const reference to object
    */
  void openObject(const MyMoneyObject& obj);

  /**
    * This signal is emitted, when the user selected to reparent the
    * account @p acc to be a subordinate account of @p parent.
    *
    * @param acc const reference to account to be reparented
    * @param parent const reference to new parent account
    */
  void reparent(const MyMoneyAccount& acc, const MyMoneyAccount& parent);

private:
  QMap<QString, MyMoneyAccount>       m_accountMap;
  QMap<QCString, MyMoneySecurity>     m_securityMap;
  QMap<QCString, unsigned long>       m_transactionCountMap;

  KMyMoneyAccountTreeItem*            m_assetItem;
  KMyMoneyAccountTreeItem*            m_liabilityItem;

  /// set if a view needs to be reloaded during show()
  bool                                m_needReload[MaxViewTabs];
  bool                                m_haveUnusedCategories;
};

#endif
