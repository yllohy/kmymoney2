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

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QPopupMenu;

class KComboBox;
class KLedgerView;

/**
  *@author Thomas Baumgart
  */

class KGlobalLedgerView : public QWidget
{
   Q_OBJECT
public:
	KGlobalLedgerView(QWidget *parent=0, const char *name=0);
	~KGlobalLedgerView();

  void reloadView(void);
  void refreshView(void);

  /**
    * This method is used to open the account with the specified id
    * in the ledger view. The respective view for this account type
    * will be selected and the account data loaded.
    * The parameter @p reconciliation determines, if the reconciliation
    * mode is started or not.
    *
    * @param id id of the account in the MyMoneyFile object
    * @param reconciliation if false (default), the standard ledger is
    *                       opened, if true, the reconciliation mode is entered
    * @param forceLoad if set to true, the account is reloaded into the view in any case
    */
  void selectAccount(const QCString& id, const bool reconciliation = false, const bool forceLoad = false);

public slots:
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
  void slotShowTransactionForm(bool show);

  /**
    * This slot cancels any edit session in the ledger views when called.
    */
  void slotCancelEdit(void);

protected:
  KComboBox* accountComboBox;
  QVBoxLayout* Form1Layout;
  QHBoxLayout* Layout2;

  void loadAccounts(void);

protected slots:

  /**
    * This slot is used to select an account by it's @p id.
    *
    * @param id const QCString reference to the account's id
    */
  void slotAccountSelected(const QString& id);

  /**
    * This slot can be used to popup a specific transaction for a
    * specific account. Both entities are defined by the corresponding Id's.
    *
    * @param accountId const QCString reference to the account id
    * @param transactionId const QCString reference to the transaction id
    */
  void slotSelectAccountAndTransaction(const QCString& accountId, const QCString& transactionId);

private:
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

signals:
  void signalViewActivated();

};

#endif
