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
#include <qwidgetlist.h>
#include <qstring.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/mymoneyutils.h>
#include <kmymoney/mymoneyaccount.h>
#include <kmymoney/kmymoneyview.h>
#include <kmymoney/register.h>
#include <kmymoney/transactionform.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QPopupMenu;
class QFrame;
class QLabel;

class KMyMoneyAccountCombo;
class KLedgerView;
class KToolBar;
class KToolBarButton;
class MyMoneyReport;
class MyMoneyObjectContainer;
class TransactionEditor;

class KGlobalLedgerViewPrivate;

/**
  * helper class implementing an event filter to detect mouse button press
  * events on widgets outside a given set of widgets. This is used internally
  * to detect when to leave the edit mode.
  */
class MousePressFilter : public QObject
{
  Q_OBJECT
public:
  MousePressFilter(QWidget* parent = 0, const char* name = 0);

  /**
    * Add widget @p w to the list of possible parent objects. See eventFilter() how
    * they will be used.
    */
  void addWidget(QWidget* w);

public slots:
  /**
    * This slot allows to activate/deactive the filter. By default the
    * filter is active.
    *
    * @param state Allows to activate (@a true) or deactivate (@a false) the filter
    */
  void setFilterActive(bool state = true);

  /**
    * This slot allows to activate/deactive the filter. By default the
    * filter is active.
    *
    * @param state Allows to deactivate (@a true) or activate (@a false) the filter
    */
  void setFilterDeactive(bool state = false) { setFilterActive(!state); }

protected:
  /**
    * This method checks if the widget @p child is a child of
    * the widget @p parent and returns either @a true or @a false.
    *
    * @param child pointer to child widget
    * @param parent pointer to parent widget
    * @retval true @p child points to widget which has @p parent as parent or grand-parent
    * @retval false @p child points to a widget which is not related to @p parent
    */
  bool isChildOf(QWidget* child, QWidget* parent);

  /**
    * Reimplemented from base class. Sends out the mousePressedOnExternalWidget() signal
    * if object @p o points to an object which is not a child widget of any added previously
    * using the addWidget() method. The signal is sent out only once for each event @p e.
    *
    * @param o pointer to QObject
    * @param e pointer to QEvent
    * @return always returns @a false
    */
  bool eventFilter(QObject* o, QEvent* e);

signals:
  void mousePressedOnExternalWidget(void);

private:
  QValueList<QWidget*> m_parents;
  QEvent*              m_lastMousePressEvent;
  bool                 m_filterActive;
};

/**
  * @author Thomas Baumgart
  */
class KGlobalLedgerView : public KMyMoneyViewBase
{
  Q_OBJECT
public:
  KGlobalLedgerView(QWidget *parent=0, const char *name=0);
  ~KGlobalLedgerView();

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
    * This method returns the id of the currently selected account
    * or QCString() if none is selected.
    */
  const QCString accountId(void) const { return m_account.id(); }

  /**
    * This method allows to see investment transactions within
    * the stack (useful for the import verification).
    *
    * @note should be removed when we have a better import transaction
    *       matcher
    * @deprecated
    */
  void loadInvestmentView(void);

  TransactionEditor* startEdit(const QValueList<KMyMoneyRegister::SelectedTransaction>& list);

  /**
    * Method to prepare the ledger view to create a new transaction.
    * Returns if successful or not.
    *
    * retval true Emtpy transaction selected.
    * retval false Not successful (e.g. already editing a transaction)
    */
  bool selectEmptyTransaction(void);

  /**
    * Switch to reconciliation mode for account with id @a accountId.
    * If @a accountId is QCString() (the default), reconciliation mode
    * is turned off.
    *
    * @param accountId id of account for which reconciliation mode is activated.
    *                  Defaults is QCString().
    */
  void setReconciliationAccount(const QCString& accountId = QCString(), const MyMoneyMoney& endingBalance = MyMoneyMoney());

public slots:
  void show(void);

  /**
    * This slot cancels any edit session in the ledger views when called.
    */
  void slotCancelEdit(void);

  /**
    * This method loads the view with data from the MyMoney engine.
    */
  void slotLoadView(void);

  /**
    * This slot is used to select the correct ledger view type for
    * the account specified by @p id in a specific mode.
    *
    * @param accountId Internal id used for the account to show
    * @param transactionId Internal id used for the transaction to select.
    *                      Default is QCString() which will select the last
    *                      transaction in the ledger if not the same account
    *
    * @retval true selection of account referenced by @p id succeeded
    * @retval false selection of account failed
    */
  const bool slotSelectAccount(const QCString& accountId, const QCString& transactionId = QCString());

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
    * This method connects the common signals that are needed for all views
    *
    * @param view pointer to view to be connected
    */
  void setupConnections(KLedgerView* view);

  /**
    * This method clears the register, form, transaction list and object container. See @sa m_register,
    * @sa m_transactionList, @sa m_objects
    */
  void clear(void);

  void loadView(void);

  void resizeEvent(QResizeEvent*);

  /**
    * This method creates group marker items and adds them to the register
    */
  void addGroupMarkers(void);

  /**
    * This method removes all group marker items from the register
    */
  void removeGroupMarkers(void);

  void selectTransaction(const QCString& id);

  /**
    * This method handles the focus of the keyboard. When in edit mode
    * (m_inEditMode is true) the keyboard focus is handled
    * according to the widgets that are referenced in m_tabOrderWidgets.
    * If not in edit mode, the base class functionality is provided.
    *
    * @param next true if forward-tab, false if backward-tab was
    *             pressed by the user
    */
  bool focusNextPrevChild(bool next);

  bool eventFilter(QObject* o, QEvent* e);

  /**
    * Returns @a true if setReconciliationAccount() has been called for
    * the current loaded account.
    *
    * @retval true current account is in reconciliation mode
    * @retval false current account is not in reconciliation mode
    */
  bool isReconciliationAccount(void) const;

  /**
    * Updates the values on the summary line beneath the register with
    * the given values. The contents shown differs between reconciliation
    * mode and normal mode.
    *
    * @param actBalance value to be used as actual balance
    * @param clearedBalance value to be used as cleared balance
    */
  void updateSummaryLine(const MyMoneyMoney& actBalance, const MyMoneyMoney& clearedBalance);

protected slots:
  void slotLeaveEditMode(const QValueList<KMyMoneyRegister::SelectedTransaction>& list);
  void slotNewTransaction(void);

  /**
    * Sets the contentsPos of the register to d->m_startPoint or makes
    * the focus item visible if d->m_startPoint equals QPoint(-1, -1).
    */
  void slotUpdateViewPos(void);
  void slotSortOptions(void);

protected:
  /**
    * This member keeps the date that was used as the last posting date.
    * It will be updated whenever the user modifies the post date
    * and is used to preset the posting date when new transactions are created.
    * This member is initialised to the current date when the program is started.
    */
  static QDate         m_lastPostDate;

private:
  KGlobalLedgerViewPrivate*     d;

  // frames
  QFrame*                       m_toolbarFrame;
  QFrame*                       m_registerFrame;
  QFrame*                       m_buttonFrame;
  QFrame*                       m_formFrame;
  QFrame*                       m_summaryFrame;

  // widgets
  KMyMoneyAccountCombo*         m_accountComboBox;
  KMyMoneyRegister::Register*   m_register;
  KToolBar*                     m_toolbar;
  KToolBar*                     m_buttonbar;

  /**
    * This member holds the currently selected account
    */
  MyMoneyAccount m_account;

  /**
    * This member holds the transaction list
    */
  QValueList<QPair<MyMoneyTransaction, MyMoneySplit> >  m_transactionList;

  MyMoneyObjectContainer*         m_objects;

  QLabel*                         m_leftSummaryLabel;
  QLabel*                         m_centerSummaryLabel;
  QLabel*                         m_rightSummaryLabel;

  KMyMoneyTransactionForm::TransactionForm* m_form;

  bool                            m_needReload;
  bool                            m_newAccountLoaded;
  bool                            m_inEditMode;

  QWidgetList                     m_tabOrderWidgets;

signals:
  void accountSelected(const MyMoneyObject&);
  void transactionsSelected(const QValueList<KMyMoneyRegister::SelectedTransaction>&);
  void newTransaction(void);
  void startEdit(void);
  void cancelEdit(void);
  void endEdit(void);
  void cancelOrEndEdit(void);

  /**
    * This signal is emitted, when a new report has been generated.  A
    * 'generated' report is halfway between a default report and a custom
    * report.  It's created by the system in response to the user's
    * request, and it's usually filtered to be a little more specific
    * than the usual default reports.
    *
    * The proper behaviour when getting this signal is to switch to the
    * reports view and display the report.  But it should NOT be added
    * to the data file, unless the user customizes it further.  That's
    * because the user can always come back to the ledger UI to generate
    * the report again.
    *
    * @param report reference to MyMoneyReport object that contains the report
    *     details
    */
  void reportGenerated(const MyMoneyReport& report);

  void openContextMenu(void);
};

#endif
// vim:cin:si:ai:et:ts=2:sw=2:
