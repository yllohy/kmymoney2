/***************************************************************************
                          kinvestmentview.h  -  description
                             -------------------
    begin                : Tue Jan 29 2002
    copyright            : (C) 2000-2002 by Michael Edwardes
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

#ifndef KINVESTMENTVIEW_H
#define KINVESTMENTVIEW_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qptrlist.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kpopupmenu.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "../mymoney/mymoneyequity.h"
#include "../mymoney/mymoneyaccount.h"
#include "kinvestmentviewdecl.h"

class MyMoneyTransaction;
class MyMoneyInvestTransaction;
class KLedgerView;

//indexes for the various columns on the summary view
#define COLUMN_NAME_INDEX       0
#define COLUMN_SYMBOL_INDEX     1
#define COLUMN_QUANTITY_INDEX   2
#define COLUMN_CURRPRICE_INDEX  3
#define COLUMN_COSTBASIS_INDEX  4
#define COLUMN_RAWGAIN_INDEX    5
#define COLUMN_1WEEKGAIN_INDEX  6
#define COLUMN_4WEEKGAIN_INDEX  7
#define COLUMN_3MONGAIN_INDEX   8
#define COLUMN_YTDGAIN_INDEX    9



/**
  * @author Kevin Tambascio
  */

class KInvestmentView : public kInvestmentViewDecl
{
  Q_OBJECT
  
public:
  typedef enum {
    VIEW_SUMMARY = 0,
    VIEW_INVESTMENT
  } eViewType;

  KInvestmentView(QWidget *parent=0, const char *name=0);
  ~KInvestmentView();

  /** No descriptions */
  bool init(const MyMoneyAccount& account);

  /** No descriptions */
  void updateDisplay();

  /** No description */
  void displayNewEquity(MyMoneyEquity *pEntry);

  /** No description */
  void addEquityEntry(MyMoneyEquity *pEntry);

public slots:
  /**
    * This slot is used to reload all data from the MyMoneyFile engine.
    * All existing data in the view will be discarded.
    * Call this e.g. if a new file has been loaded.
    */
  void slotReloadView(void);

  /**
    * This slot is used to refresh the view with data from the MyMoneyFile
    * engine. All data in the view (e.g. current account) will be kept if
    * still available. Call this, e.g. if the global view settings have
    * been changed.
    */
  void slotRefreshView(void);

  /**
    * This slot is used to select the correct ledger view type for
    * the account specified by @p id.
    *
    * @param id Internal id used for the account to show
    * @param reconciliation if true, the account will be selected in
    *                       reconciliation mode. If false, it will
    *                       be selected in regular ledger mode.
    *
    * @retval true selection of account referenced by @p id succeeded
    * @retval false selection of account failed
    */
  const bool slotSelectAccount(const QCString& id, const bool reconciliation = false);

  /**
    * This is an overloaded version of the above method.
    *
    * Using this method one can select an account by it's name. The name
    * must match an asset or liability account name.
    *
    * @param accountName name of an existing account
    *
    * @retval true selection of account referenced by @p id succeeded
    * @retval false selection of account failed
    */
  const bool slotSelectAccount(const QString& accountName);

protected:
  /**
    * This method reloads the account selection combo box of the
    * view with all asset and liability accounts from the engine.
    * If the account id of the current account held in @p m_accountId is
    * empty or if the referenced account does not exist in the engine,
    * the first account found in the list will be made the current account.
    */
  void loadAccounts(void);

protected slots:

  /**
    * This slot receives the signal from the listview control that an item was double-clicked,
    */
  void slotItemDoubleClicked(QListViewItem* pItem, const QPoint& pos, int c);

  /**
    * This slot receives the signal from the listview control that an item was right-clicked,
    */
  void slotListRightMouse(QListViewItem* item, const QPoint& point, int);
  //void slotNewInvestment();
  void slotEditInvestment();
  void slotUpdatePrice();
  void slotViewChanged(int);

private:
  void initSummaryTab(void);
  void initTransactionTab(void);

private:
  KPopupMenu*       m_popMenu;
  KLedgerView*      m_ledgerView;
  MyMoneyAccount    m_account;
};

#endif
