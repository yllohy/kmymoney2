/***************************************************************************
                             kmymoneyaccountselector.h
                             -------------------
    begin                : Thu Sep 18 2003
    copyright            : (C) 2003 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KMYMONEYACCOUNTSELECTOR_H
#define KMYMONEYACCOUNTSELECTOR_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

class KPushButton;

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/kmymoneyselector.h>
#include <kmymoney/kmymoneyutils.h>
#include <kmymoney/mymoneyaccount.h>

class kMyMoneyAccountCompletion;
class AccountSet;
class MyMoneyFile;

/**
  * This class implements an account/category selector. It is based
  * on a tree view. Using this widget, one can select one or multiple
  * accounts depending on the mode of operation and the set of accounts
  * selected to be displayed. (see setSelectionMode()
  * and loadList() about the specifics of configuration).
  *
  * - Single selection mode\n
  *   In this mode the widget allows to select a single entry out of
  *   the set of displayed accounts.
  *
  * - Multi selection mode\n
  *   In this mode, the widget allows to select one or more entries
  *   out of the set of displayed accounts. Selection is performed
  *   by marking the account in the view.
  */
class kMyMoneyAccountSelector : public KMyMoneySelector
{
  Q_OBJECT
public:
  friend class AccountSet;

  kMyMoneyAccountSelector(QWidget *parent=0, const char *name=0, QWidget::WFlags flags = 0, const bool createButtons = true);
  virtual ~kMyMoneyAccountSelector();

  /**
    * This method returns a list of account ids of those accounts
    * currently loaded into the widget. It is possible to select
    * a list of specific account types only. In this case, pass
    * a list of account types as parameter @p list.
    *
    * @param list QValueList of account types to be returned. If this
    *             list is empty (the default), then the ids of all accounts
    *             will be returned.
    * @return QStringList of account ids
    */
  QStringList accountList(const QValueList<MyMoneyAccount::accountTypeE>& list = QValueList<MyMoneyAccount::accountTypeE>()) const;

  void setSelectionMode(QListView::SelectionMode mode);

  /**
    * This method checks if a given @a item matches the given regular expression @a exp.
    *
    * @param exp const reference to a regular expression object
    * @param item pointer to QListViewItem
    *
    * @retval true item matches
    * @retval false item does not match
    */
  virtual bool match(const QRegExp& exp, QListViewItem* item) const;

  /**
    * This method returns, if any of the items in the selector contains
    * the text @a txt.
    *
    * @param txt const reference to string to be looked for
    * @retval true exact match found
    * @retval false no match found
    */
  virtual bool contains(const QString& txt) const;

  /**
    * This method removes all the buttons of the widget
    */
  void removeButtons(void);

public slots:
  /**
    * This slot selects all items that are currently in
    * the account list of the widget.
    */
  void slotSelectAllAccounts(void) { selectAllItems(true); };

  /**
    * This slot deselects all items that are currently in
    * the account list of the widget.
    */
  void slotDeselectAllAccounts(void) { selectAllItems(false); };

protected:
  /**
    * This method loads the list of subaccounts as found in the
    * @p list and attaches them to the parent widget passed as @p parent.
    *
    * @param parent pointer to parent widget
    * @param list QStringList containing the ids of all subaccounts to load
    * @return This method returns the number of accounts loaded into the list
    */
  int loadSubAccounts(QListViewItem* parent, const QStringList& list);

  /**
    * This is a helper method for selectAllIncomeCategories()
    * and selectAllExpenseCategories().
    */
  void selectCategories(const bool income, const bool expense);

protected slots:
  /**
    * This slot selects all income categories
    */
  void slotSelectIncomeCategories(void) { selectCategories(true, false); };

  /**
    * This slot selects all expense categories
    */
  void slotSelectExpenseCategories(void) { selectCategories(false, true); };

protected:
  KPushButton*              m_allAccountsButton;
  KPushButton*              m_noAccountButton;
  KPushButton*              m_incomeCategoriesButton;
  KPushButton*              m_expenseCategoriesButton;
  QValueList<int>           m_typeList;
  QStringList               m_accountList;
};


class AccountSet
{
public:
  AccountSet();

  void addAccountType(MyMoneyAccount::accountTypeE type);
  void addAccountGroup(MyMoneyAccount::accountTypeE type);
  void removeAccountType(MyMoneyAccount::accountTypeE type);

  void clear(void);

  int load(kMyMoneyAccountSelector* selector);
  int load(kMyMoneyAccountSelector* selector, const QString& baseName, const QValueList<QString>& accountIdList, const bool clear = false);

  int count(void) const { return m_count; }

  void setHideClosedAccounts (bool _bool) { m_hideClosedAccounts = _bool; }
  bool isHidingClosedAccounts (void) { return m_hideClosedAccounts; }

protected:
  int loadSubAccounts(kMyMoneyAccountSelector* selector, QListViewItem* parent, const QString& key, const QStringList& list);

private:
  int                                      m_count;
  MyMoneyFile*                             m_file;
  QValueList<MyMoneyAccount::accountTypeE> m_typeList;
  QListViewItem*                           m_favorites;
  bool                                     m_hideClosedAccounts;
};
#endif
