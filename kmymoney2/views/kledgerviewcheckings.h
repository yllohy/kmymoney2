/***************************************************************************
                          kledgerviewcheckings.h  -  description
                             -------------------
    begin                : Sun Jul 14 2002
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

#ifndef KLEDGERVIEWCHECKINGS_H
#define KLEDGERVIEWCHECKINGS_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qwidget.h>
#include <qtabbar.h>
class QLabel;
class QHBoxLayout;
class QCheckBox;

// ----------------------------------------------------------------------------
// KDE Includes

class KPushButton;

// ----------------------------------------------------------------------------
// Project Includes

#include "kledgerview.h"
class kMyMoneyTransactionFormTable;

/**
  *@author Thomas Baumgart
  */

/**
  * This class represents the ledger view for checkings accounts.
  * As described with the base class KLedgerView, it consists out
  * of a register, a button line and a form. These are organized as follows:
  *
  * @code
  *
  * +------------------------------------------------------------------------------+
  * | formLayout                                                                   |
  * |                                                                              |
  * | +---------------------------------------------------+   +------------------+ |
  * | | ledgerLayout                                      |   | buttonLayout     | |
  * | |                                                   |   |                  | |
  * | | +-----------------------------------------------+ |   | +--------------+ | |
  * | | | kMyMoneyRegisterCheckings                     | |   | | KPushButton  | | |
  * | | |                                               | |   | +--------------+ | |
  * | | |                                               | |   |                  | |
  * | | |                                               | |   | +--------------+ | |
  * | | |                                               | |   | | KPushButton  | | |
  * | | |                                               | |   | +--------------+ | |
  * | | |                                               | |   |                  | |
  * | | |                                               | |   |                  | |
  * | | +-----------------------------------------------+ |   |       -----      | |
  * | | +-----------------------------------------------+ |   |         |        | |
  * | | | Summary Line                                  | |   |         |        | |
  * | | +-----------------------------------------------+ |   |       -----      | |
  * | | +-----------------------------------------------+ |   |   QSpacerItem    | |
  * | | | kMyMoneyTransactionForm                       | |   |                  | |
  * | | |                                               | |   |                  | |
  * | | |                                               | |   |                  | |
  * | | |                                               | |   |                  | |
  * | | |                                               | |   |                  | |
  * | | |                                               | |   |                  | |
  * | | +-----------------------------------------------+ |   |                  | |
  * | +---------------------------------------------------+   +------------------+ |
  * +------------------------------------------------------------------------------+

  * @endcode
  *
  * The register is provided by kMyMoneyRegisterCheckings. The form
  * is based on a kMyMoneyTransactionForm object. The various parts
  * are created in createRegister(), createSummary() and createForm().
  * The above diagram shows the standard layout. The buttonLayout is
  * actually contained as one page in the QWidgetStack m_infoStack.
  * The widgets therein are created in the createInfoStack() method which
  * is called by the constructor. A second page exists, which is used by
  * the reconciliation code.
  *
  * The tabbar on top of the actual form is part of the kMyMoneyTransactionForm
  * widget and shows the possible transaction types. It
  * is loaded in the constructor of this class.
  *
  * The form is QTable-based and will be created with 4 rows and 5 columns
  * in the constructor of this class. Except for
  * the category input field, fields in col 1 also span col 2. The
  * category field provides a button in col 2 to enter the splits dialog.
  *
  * The edit widgets are created within showWidgets(). This method also
  * attaches the widgets to the table's cells using QTable::setCellWidget().
  * It also maintains the tab order. It uses the private methods
  * loadEditWidgets(), arrangeEditWidgetsInForm() and arrangeEditWidgetsInRegister()
  * to load the data of the current selected transaction into the widgets and
  * to place the widgets appropriately.
  *
  * hideWidgets() removes all edit widgets from the form table and returns
  * to the read-only form view.
  *
  * fillForm() fills the data provided by the current selected transaction
  * into the read-only form. The layout of the form depends on the type
  * the selected of transaction.
  *
  * fillSummary() will be called and should update the summary line.
  */
class KLedgerViewCheckings : public KLedgerView  {
   Q_OBJECT

  friend class kMyMoneyTransactionFormTable;

public: 
	KLedgerViewCheckings(QWidget *parent=0, const char *name=0);
	~KLedgerViewCheckings();

  void show();

public slots:
  /**
    * refresh the current view
    */
  virtual void refreshView(void);

  void slotTypeSelected(int transactionType);

  /**
    *
    */
  virtual void slotRegisterClicked(int row, int col, int button, const QPoint &mousePos);

  void slotRegisterDoubleClicked(int row, int col, int button, const QPoint &mousePos);

  /**
    * Calling this slot enters reconciliation mode. For checkings accounts
    * this means, that a dialog is opened where the previous endinge balance, the
    * current ending balance and the issue date of the statement are collected
    * from the user. Any active editing of a transaction is cancelled.
    */
  virtual void slotReconciliation(void);

protected:
  void fillSummary(void);

  void fillForm(void);

  void resizeEvent(QResizeEvent*);

  /**
    * This method creates, loads, arranges and shows the widgets required
    * to edit a transaction. See createEditWidgets(), loadEditWidgets(),
    * arrangeEditWidgetsInForm() and arrangeEditWidgetsInRegister() for details.
    */
  void showWidgets(void);

  /**
    * This destroys and hides the widgets used to edit a transaction.
    */
  void hideWidgets(void);

  /**
    * This method is called to determine the next widget that receives focus
    * upon a Tab or Back-Tab event.
    * The parameter @p next defines the search direction.
    *
    * @param next if true, searches forward, if false searches backward
    * @return true if widget could be found, false otherwise.
    */
  virtual bool focusNextPrevChild(bool next);

  /**
    * This method creates all widgets that allow a view to edit
    * a transaction. All signal/slot connections of the created
    * widgets will be setup also in this method.
    *
    * If different widgets are required for in-register
    * and in-form editing, both will be created. They can be destroyed
    * later on. See arrangeEditWidgetsInForm() or arrangeEditWidgetsInRegister().
    * This method will be called by showWidgets().
    */
  virtual void createEditWidgets(void);

  /**
    * This method creates the context menu that is accessible via the
    * right mouse button while pointing on a transaction in the register
    */
  virtual void createContextMenu(void);

  /**
    * This method creates the context menu that is accessible via the
    * More... button in the transaction form.
    */
  virtual void createMoreMenu(void);

protected slots:
  /**
    * Calling this slot opens the account edit dialog for the current
    * selected account.
    */
  virtual void slotAccountDetail(void);

  /**
    * Calling this slot toggles the clear flag of the split of the
    * current selected transaction. The display will be updated.
    */
  virtual void slotToggleClearFlag(void);

  /**
    * Calling this slot postpones the current reconciliation operation.
    * Data entered by the user is saved in the accounts key-value-container
    */
  virtual void slotPostponeReconciliation(void);

  /**
    * Calling this slot ends the current reconciliation operation.
    * Data entered by the user is saved in the account. All transactions
    * that are marked cleared will be marked reconciled. The visual appearance
    * reverts to normal transaction edit mode.
    */
  virtual void slotEndReconciliation(void);

  /**
    * Calling this slot opens the split dialog with the current transaction
    * loaded.
    */
  virtual void slotOpenSplitDialog(void);

  /**
    * Calling this slot starts editing and opens the split dialog
    */
  virtual void slotStartEditSplit(void);

  /**
    * Calling this slot displays the More menu.
    */
  virtual void slotMorePressed(void);

  void reloadEditWidgets(const MyMoneyTransaction& t);

  void slotPayeeSelected(void);

private:
  /**
    * This method loads the data of the current transaction into the
    * widgets created with createEditWidgets(). If different widgets are
    * required for in-register and in-form editing, all widgets will be filled.
    * This method also analyses the data of the transaction and determines
    * the transaction type which is returned in the parameter @p transType.
    *
    * @param transType reference to transaction type. The method will set this
    *                  value upon return to the caller.
    * @return The return value is passed in the variable referenced by @p transType
    */
  void loadEditWidgets(int& transType);

  /**
    * This method arranges the widgets required for in-form editing in the
    * form according to the transaction type passed by @p transType. It destroys
    * all widgets that have been created specifically for in-register editing.
    * Depending on the transaction type, the @p focusWidget will be selected.
    *
    * @param focusWidget reference to pointer which will point to the widget
    *                    that should receive focus when editing starts.
    * @param transType type of transaction as determined by loadEditWidgets()
    * @return The return value is passed in the variable referenced by @p focusWidget.
    */
  void arrangeEditWidgetsInForm(QWidget*& focusWidget, const int transType);

  /**
    * This method arranges the widgets required for in-register editing in the
    * register according to the transaction type passed by @p transType. It destroys
    * all widgets that have been created specifically for in-form editing.
    * Depending on the transaction type, the @p focusWidget will be selected.
    *
    * @param focusWidget reference to pointer which will point to the widget
    *                    that should receive focus when editing starts.
    * @param transType type of transaction as determined by loadEditWidgets()
    * @return The return value is passed in the variable referenced by @p focusWidget.
    */
  void arrangeEditWidgetsInRegister(QWidget*& focusWidget, const int transType);

  /**
    * This method is used by the constructor to create the necessary widgets
    * for the register of the view and set it up.
    */
  void createRegister(void);

  /**
    * This method is used by the constructor to create the summary line underneath
    * the register widget in the view.
    */
  void createSummary(void);

  /**
    * This method is used by the constructor to create the info stack on
    * the right of the register widget. The stack widget itself is created
    * by the base class member of this function.
    */
  void createInfoStack(void);

  /**
    * This method is used by the constructor to create the transaction form
    * provided by the view.
    */
  void createForm(void);

  /**
    * This method updates the respective data shown on the right of the
    * register during reconciliation.
    */
  void fillReconcileData(void);

  /**
    * This method updates the visual elements from reconciliation mode
    * to edit transaction mode. Called by slotPostponeReconciliation()
    * and slotEndReconciliation().
    */
  void endReconciliation(void);

private slots:
  /**
    * This method enables and disables the options available for
    * the selected transaction (e.g. context menu options). It
    * is connected to the context menu's aboutToShow() signal.
    */
  void slotConfigureContextMenu(void);

  /**
    * This method enables and disables the options available for
    * the selected transaction (e.g. more menu options). It
    * is connected to the more menu's aboutToShow() signal.
    */
  void slotConfigureMoreMenu(void);

signals:
protected:
  QTab* m_tabCheck;
  QTab* m_tabDeposit;
  QTab* m_tabTransfer;
  QTab* m_tabWithdrawal;
  QTab* m_tabAtm;

  KPushButton*  m_detailsButton;
  KPushButton*  m_reconcileButton;

  /**
    * This attribute stores the current selected transaction type
    * which is used for new transactions.
    */
  QCString m_action;

  /**
    * This member keeps a pointer to the summary line
    * which is located underneath the register. The
    * widget itself is created in createSummary()
    */
  QLabel          *m_summaryLine;

  QHBoxLayout*    m_summaryLayout;

private:

  // The following attributes are exclusively used for reconciliation
  MyMoneyMoney    m_prevBalance;
  MyMoneyMoney    m_endingBalance;
  QDate           m_endingDate;

  QLabel*         m_clearedLabel;
  QLabel*         m_statementLabel;
  QLabel*         m_differenceLabel;

  QCheckBox*      m_transactionCheckBox;
};

#endif
