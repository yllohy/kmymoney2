/***************************************************************************
                          kbanksview.h
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes
    email                : mte@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KBANKSVIEW_H
#define KBANKSVIEW_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qwidget.h>
#include <qlabel.h>
#include <qwidget.h>
#include <qevent.h>
#include <qsize.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>
#include <klistview.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "../mymoney/mymoneyobserver.h"
#include "../mymoney/mymoneyaccount.h"
#include "../views/kbankviewdecl.h"
#include "../views/kbanklistitem.h"

/**
  *@author Michael Edwardes, Thomas Baumgart
  */

/**
  * This class handles the bank and account hierarchical 'view'.
  * It handles the resize event, the totals widgets
  * and the KAccountListView itself
  */
class KAccountsView : public KBankViewDecl, MyMoneyObserver  {
   Q_OBJECT
private:
  bool m_bSelectedAccount;
  QCString m_selectedAccount;
  bool m_suspendUpdate;
  // bool m_bSignals;
  bool m_bViewNormalAccountsView;
  bool m_hideCategory;
  QCString m_selectedInstitution;
  bool m_bSelectedInstitution;

  void refresh(const QCString& selectAccount);
  void refreshNetWorth(void);
  const bool showSubAccounts(const QCStringList& accounts, KAccountListItem *parentItem, const bool used);

  QMap<QCString, MyMoneyAccount> m_accountMap;
  QMap<QCString, unsigned long> m_transactionCountMap;

public:
  KAccountsView(QWidget *parent=0, const char *name=0);
  ~KAccountsView();
  QCString currentAccount(bool&);
  QCString currentInstitution(bool&);
  void clear(void);
  void show();

  /**
    * This method is called by the MyMoneyFile object, whenever the
    * account hierarchy changes within the MyMoneyFile engine.
    *
    * @param id reference to QCString of the id
    */
  void update(const QCString& id);

  /**
    * This method is used to suppress updates for specific times
    * (e.g. during creation of a new MyMoneyFile object when the
    * default accounts are loaded). The behaviour of update() is
    * controlled with the parameter @p suspend.
    *
    * @param suspend Suspend updates or not. Possible values are
    *
    * @li true updates are suspended
    * @li false updates will be performed immediately
    *
    * When a true/false transition of the parameter between
    * calls to this method is detected,
    * the view will be refreshed once automatically.
    */
  void suspendUpdate(const bool suspend);

public slots:
  void slotEditClicked(void);
  void slotDeleteClicked(void);
  void slotRefreshView(void);
  void slotReloadView(void) { slotRefreshView(); };

protected:
  void resizeEvent(QResizeEvent*);

  // void fillTransactionCountMap(void);
  void fillAccountMap(void);

protected slots:

  /**
    * This slot receives the signal from the listview control that an item was double-clicked,
    * if this item was a bank account, try to show the list of transactions for that account.
    */
  void slotListDoubleClicked(QListViewItem* pItem, const QPoint& pos, int c);

  /**
    * This slot receives the signal from the iconview control that an item was double-clicked,
    * if this item was a bank account, try to show the list of transactions for that account.
    */
  void slotIconDoubleClicked(QIconViewItem* pItem);

  /**
    * This slot receives the signal from the listview control that an item was right-clicked,
    * Pass this signal along to the main view to display the RMB menu.
    */
  void slotListRightMouse(QListViewItem* item, const QPoint& point, int);

  /**
    * This slot receives the signal from the iconview control that an item was right-clicked,
    * Pass this signal along to the main view to display the RMB menu.
    */
  void slotIconRightMouse(QIconViewItem* item, const QPoint& point);

  void slotSelectionChanged(QListViewItem *item);

  void slotViewSelected(QWidget* view);

private:
  /**
    * This method returns an icon according to the account type
    * passed in the argument @p type.
    *
    * @param type account type as defined in MyMoneyAccount::accountTypeE
    */
  const QPixmap accountImage(const MyMoneyAccount::accountTypeE type) const;

signals:
  /**
    * This signal will be emitted when the left mouse button is double

    * clicked on an asset or liability account. It is not emitted for
    * expense, income and any of the standard accounts.
    */
  void accountDoubleClick();

  void signalViewActivated();

  void accountRightMouseClick();

  void bankRightMouseClick();

  void rightMouseClick();

};

#endif
