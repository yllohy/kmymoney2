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
#include "kinvestmentlistitem.h"
#include "../mymoney/mymoneysecurity.h"
#include "../mymoney/mymoneyaccount.h"
#include "../mymoney/mymoneyobserver.h"
#include "kinvestmentviewdecl.h"

class MyMoneyTransaction;
class MyMoneyInvestTransaction;
class KLedgerView;

typedef QMap<QWidget*, int>::Iterator tabmap_iterator;

/**
  * @author Kevin Tambascio
  */

class KInvestmentView : public kInvestmentViewDecl, MyMoneyObserver
{
  Q_OBJECT

public:
  KInvestmentView(QWidget *parent=0, const char *name=0);
  ~KInvestmentView();

  void update(const QCString& id);

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
    * the account specified by @p id. If @p transactionId is not
    * empty, then the respective transaction will be selected.
    *
    * @param accountId Internal id used for the account to show
    * @param transactionId Internal id used for the transaction to select
    * @param reconciliation if true, the account will be selected in
    *                       reconciliation mode. If false, it will
    *                       be selected in regular ledger mode.
    *
    * @retval true selection of account referenced by @p id succeeded
    * @retval false selection of account failed
    */
  const bool slotSelectAccount(const QCString& accountId, const QCString& transactionId = QCString(), const bool reconciliation = false);

  void show(void);

  void slotNewInvestment(void);

  /**
    * This slot cancels any edit activity in any view. It will
    * be called e.g. before entering the settings dialog.
    */
  void slotCancelEdit(void);

  /**
    * This slot starts the logic to add a price information to the file
    */
  void slotAddPrice(void);

protected:
  /**
    * This method reloads the account selection combo box of the
    * view with all asset and liability accounts from the engine.
    * If the account id of the current account held in @p m_accountId is
    * empty or if the referenced account does not exist in the engine,
    * the first account found in the list will be made the current account.
    */
  void loadAccounts(void);

  /**
    * This methods updates the display of the investment overview.
    */
  void updateDisplay();

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
  void slotTabSelected(QWidget *pWidget);
  void slotRemoveInvestment();

signals:
  void signalViewActivated(void);

  /**
    * This signal is emitted, if an account has been selected
    * which cannot handled by this view.
    */
  void accountSelected(const QCString& accountId, const QCString& transactionId);

private:
  void initSummaryTab(void);
  void initTransactionTab(void);

private:
  KPopupMenu*       m_popMenu;
  KLedgerView*      m_ledgerView;
  MyMoneyAccount    m_account;
  QCString          m_accountId;
};

#endif
