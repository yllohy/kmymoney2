/***************************************************************************
                          kmymoneyaccountselector.h  -  description
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

#ifndef KMYMONEYACCOUNTSELECTOR_H
#define KMYMONEYACCOUNTSELECTOR_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qwidget.h>
#include <qlistview.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klistview.h>
class KPushButton;

// ----------------------------------------------------------------------------
// Project Includes

#include "../kmymoneyutils.h"

/**
  * @author Thomas Baumgart
  */

/**
  * This class implements a derived version of a QCheckListItem that
  * allows the storage of an account id with the object and emits
  * a signal upon state change.
  */
class kMyMoneyCheckListItem : public QObject, public QCheckListItem
{
  Q_OBJECT
public:
  kMyMoneyCheckListItem(QListView *parent, const QString& txt, const QCString& id, Type type = QCheckListItem::CheckBox);
  kMyMoneyCheckListItem(QListViewItem *parent, const QString& txt, const QCString& id, Type type = QCheckListItem::CheckBox);
  ~kMyMoneyCheckListItem();
  const QCString& accountId(void) const { return m_id; };

signals:
  void stateChanged(bool);
  
protected:
  virtual void stateChange(bool);

private:
  QCString             m_id;
};

/**
  * This class implements a derived version of a QListViewItem that
  * allows the storage of an account id with the object
  */
class kMyMoneyListViewItem : public QObject, public KListViewItem
{
  Q_OBJECT
public:
  kMyMoneyListViewItem(QListView *parent, const QString& txt, const QCString& id);
  kMyMoneyListViewItem(QListViewItem *parent, const QString& txt, const QCString& id);
  ~kMyMoneyListViewItem();
  const QCString& accountId(void) const { return m_id; };

private:
  QCString             m_id;
};

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
class kMyMoneyAccountSelector : public QWidget
{
  Q_OBJECT
public: 
  kMyMoneyAccountSelector(QWidget *parent=0, const char *name=0);
  ~kMyMoneyAccountSelector();

  /**
    * This method sets the mode of operation of this widget.
    * Supported values are @p QListView::Single and @p QListView::Multi.
    *
    * @param mode @p QListView::Single selects single selection mode and
    *             @p QListView::Multi selects multi selection mode
    *
    * @note This method must be called prior to loadList(). When the
    *       widget is created, it defaults to QListView::Single.
    *       
    */
  void setSelectionMode(const QListView::SelectionMode mode);

  /**
    * This method loads the set of accounts into the widget
    * as defined by the parameter @p typeMask. @p typeMask is
    * a bit mask. The following definitions can be passed:
    *
    * - KMyMoneyUtils::asset
    * - KMyMoneyUtils::liability
    * - KMyMoneyUtils::income
    * - KMyMoneyUtils::expense
    *
    * If multiple sets should be displayed, the above values
    * can be logically 'or'-ed.
    *
    * @param typeMask bitmask defining which types of accounts
    *                 should be loaded into the widget
    *
    */
  void loadList(KMyMoneyUtils::categoryTypeE typeMask);

  /**
    * This method returns the list of selected account id's. If
    * no account is selected, the list is empty.
    *
    * @return list of selected accounts
    */
  const QCStringList selectedAccounts(void) const;

  /**
    * This method returns an information if all items
    * currently shown are selected or not.
    *
    * @retval true All accounts shown are selected
    * @retval false Not all accounts are selected
    *
    * @note If the selection mode is set to Single, this
    *       method always returns false.
    */
  const bool allAccountsSelected(void) const;

  /**
    * This method sets the current selected account.
    */
  void setSelected(const QCString& id);

public slots:
  /**
    * This slot selects all items that are currently in
    * the account list of the widget.
    */
  void slotSelectAllAccounts(void) { selectAllAccounts(true); };

  /**
    * This slot deselects all items that are currently in
    * the account list of the widget.
    */
  void slotDeselectAllAccounts(void) { selectAllAccounts(false); };

signals:
  void stateChanged(void);
  
protected:
  /**
    * Helper method for setSelected() to traverse the tree.
    */
  void setSelected(QListViewItem *item, const QCString& id);
  
  /**
    * Helper method for selectedAccounts() to traverse the tree.
    */
  void selectedAccounts(QCStringList& list, QListViewItem* item) const;
  
  /**
    * Helper method for allAccountsSelected() to traverse the tree.
    */
  const bool allAccountsSelected(const QListViewItem *item) const;
  
  /**
    * This method creates a new selectable object depending on the
    * selection mode. This is either a KListViewItem for single
    * selection mode or a KMyMoneyCheckListItem for multi selection mode
    *
    * @return pointer to newly created object
    */
  QListViewItem* newEntryFactory(QListViewItem* parent, const QString& name, const QCString& id);

  /**
    * This method loads the list of subaccounts as found in the
    * @p list and attaches them to the parent widget passed as @p parent.
    *
    * @param parent pointer to parent widget
    * @param list QCStringList containing the ids of all subaccounts to load
    */
  void loadSubAccounts(QListViewItem* parent, const QCStringList& list);

  /**
    * This method selects/deselects all items that
    * are currently in the account list according
    * to the parameter @p state.
    *
    * @param state select items if @p true, deselect otherwise
    */
  void selectAllAccounts(const bool state);

  /**
    * This is a helper method for selectAllItems().
    */
  void selectAllSubAccounts(QListViewItem* item, const bool state);

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
  
private:
  QListView::SelectionMode  m_selMode;
  KListView*                m_listView;
  
  KPushButton*              m_allAccountsButton;
  KPushButton*              m_noAccountButton;
  KPushButton*              m_incomeCategoriesButton;
  KPushButton*              m_expenseCategoriesButton;
};

#endif
