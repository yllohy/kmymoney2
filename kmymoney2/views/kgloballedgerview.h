/***************************************************************************
                          kgloballedgerview.h  -  description
                             -------------------
    begin                : Sat Jul 13 2002
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

#ifndef KACCOUNTVIEW_H
#define KACCOUNTVIEW_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qwidget.h>
#include <qwidgetstack.h>
#include <qstring.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "../mymoney/mymoneyutils.h"
#include "../mymoney/mymoneyaccount.h"
#include "../mymoney/mymoneyobserver.h"

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QPopupMenu;

class kMyMoneyAccountCombo;
class KLedgerView;

/**
  *@author Thomas Baumgart
  */

class KGlobalLedgerView : public QWidget, MyMoneyObserver
{
   Q_OBJECT
public:
  KGlobalLedgerView(QWidget *parent=0, const char *name=0);
  ~KGlobalLedgerView();

  // void refreshView(void);

  /**
    * utility method to suspend/activate updates of the MyMoney engine on
    * all views. This is used to speed up operations with lot's of updates
    * of engine data in a short time (e.g. importing data, creating a
    * new file).
    *
    * @param suspend Suspend updates or not. Possible values are
    *
    * @li true updates are suspended
    * @li false updates will be performed immediately
    */
  void suspendUpdate(const bool suspend);

  /**
    */
  void update(const QCString& id);

public slots:
  // void reloadView(void);

  /**
    * This slot calls the hide() slot of all known specific ledger views
    */
  void hide(void);

  void show(void);

  /**
    * Called when the user changes the visibility
    * setting of the transaction form
    *
    * @param show if true, the transaction form will be shown
    */
  // void slotShowTransactionForm(bool show);

  /**
    * This slot cancels any edit session in the ledger views when called.
    */
  void slotCancelEdit(void);

  /**
    * This slot can be used to popup a specific transaction for a
    * specific account. Both entities are defined by the corresponding Id's.
    *
    * @param accountId const QCString reference to the account id
    * @param transactionId const QCString reference to the transaction id
    */
  void slotSelectAccountAndTransaction(const QCString& accountId, const QCString& transactionId);

  /**
    * This slot is used to select an account by it's @p id.
    *
    * @param id const QCString reference to the account's id
    *
    * @note The account will be selected in standard mode (not in reconciliation mode)
    */
  // void slotAccountSelected(const QString& id);

  /**
    * This slot is used to select an account by it's @p id.
    *
    * @param id const QCString reference to the account's id
    * @param reconciliation if false (default), the standard ledger is
    *                       opened, if true, the reconciliation mode is entered
    */
  // void slotAccountSelected(const QString& id, const bool reconciliation);

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
    * the account specified by @p id in a specific mode.
    *
    * @param id Internal id used for the account to show
    * @param reconciliation if true, the account will be selected in
    *                       reconciliation mode. If false, it will
    *                       be selected in regular ledger mode.
    *
    * @retval true selection of account referenced by @p id succeeded
    * @retval false selection of account failed
    */
  const bool slotSelectAccount(const QCString& id, const bool reconciliation);

  /**
    * This slot is used to select the correct ledger view type for
    * the account specified by @p id. It is essentially the same
    * as calling slotSelectAccount(id, false);
    *
    * @param id Internal id used for the account to show
    *
    * @retval true selection of account referenced by @p id succeeded
    * @retval false selection of account failed
    */
  const bool slotSelectAccount(const QCString& id);

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
    * This method is used to open the account with the specified id
    * in the ledger view. The respective view for this account type
    * will be selected and the account data loaded.
    * If a transaction ID is given in @p transaction, the specified
    * transaction will be selected and shown. If transaction is empty,
    * then the last transaction will be selected.
    * The parameter @p reconciliation determines, if the reconciliation
    * mode is started or not.
    *
    * @param id id of the account in the MyMoneyFile object
    * @param transaction ID of the transaction to be selected
    * @param reconciliation if false (default), the standard ledger is
    *                       opened, if true, the reconciliation mode is entered
    * @param forceLoad if set to true, the account is reloaded into the view in any case
    */
  // void selectAccount(const QCString& id, const QCString& transaction = "", const bool reconciliation = false, const bool forceLoad = false);

protected slots:

protected:


private:
  kMyMoneyAccountCombo* m_accountComboBox;

  /**
    * This member holds the id of the currently selected account
    */
  QCString m_accountId;

  /**
    * m_specificView[] keeps pointers to the specific views for the
    * different accounts.
    * m_accountStack is the widget stack for them
    * m_currentView points to the current active view
    */
  KLedgerView* m_specificView[MyMoneyAccount::MaxAccountTypes];
  QWidgetStack* m_accountStack;
  KLedgerView* m_currentView;
  QVBoxLayout* m_formLayout;

signals:
  /**
    * This signal is emitted whenever this view is activated.
    */
  void signalViewActivated(void);

  /**
    * This signal is emitted whenever the ledger views are required to
    * cancel any pending edit operation.
    */
  void cancelEdit();

  /**
    */
  void payeeSelected(const QCString& payeeId, const QCString& accountId, const QCString& transactionId);

  /**
    * This signal is emitted, if an account has been selected
    * which cannot handled by this view.
    */
  void accountSelected(const QCString& accountId, const QCString& transactionId);

};

#endif
