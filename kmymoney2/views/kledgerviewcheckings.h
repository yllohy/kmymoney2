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
  * @author Thomas Baumgart
  */

/**
  * This class represents the ledger view for checkings accounts.
  * As described with the base class KLedgerView, it consists out
  * of a register, a button line and a form. These are organized as follows:
  *
  * @code
  *
  * +---------------------------------------------------------+
  * | formLayout                                              |
  * |                                                         |
  * | +---------------------------------------------------+   |
  * | | ledgerLayout                                      |   |
  * | |                                                   |   |
  * | | +-----------------------------------------------+ |   |
  * | | | kMyMoneyRegisterCheckings                     | |   |
  * | | |                                               | |   |
  * | | |                                               | |   |
  * | | |                                               | |   |
  * | | |                                               | |   |
  * | | |                                               | |   |
  * | | |                                               | |   |
  * | | |                                               | |   |
  * | | +-----------------------------------------------+ |   |
  * | | +-----------------------------------------------+ |   |
  * | | | Summary Line                                  | |   |
  * | | +-----------------------------------------------+ |   |
  * | | +-----------------------------------------------+ |   |
  * | | | kMyMoneyTransactionForm                       | |   |
  * | | |                                               | |   |
  * | | |                                               | |   |
  * | | |                                               | |   |
  * | | |                                               | |   |
  * | | |                                               | |   |
  * | | +-----------------------------------------------+ |   |
  * | +---------------------------------------------------+   |
  * +---------------------------------------------------------+

  * @endcode
  *
  * The register is provided by kMyMoneyRegisterCheckings. The form
  * is based on a kMyMoneyTransactionForm object. The various parts
  * are created in createRegister(), createSummary() and createForm().
  * The above diagram shows the standard layout.
  *
  * The transaction form itself has the following layout:
  *
  * @code
  *
  * +---------------------------------------------------------+
  * | formLayout                                              |
  * | +-----------------------------------------------------+ +
  * | | tabbar                                              | |
  * | |                                                     | |
  * | +-----------------------------------------------------+ +
  * | +-----------------------------------------------------+ +
  * | | formFrame                                           | |
  * | | +-------------------------------------------------+ | |
  * | | | buttons                                         | | |
  * | | +-------------------------------------------------+ | |
  * | | +-------------------------------------------------+ | |
  * | | | kMyMoneyTransactionFormTable                    | | |
  * | | |                                                 | | |
  * | | |                                                 | | |
  * | | |                                                 | | |
  * | | |                                                 | | |
  * | | +-------------------------------------------------+ | |
  * | +-----------------------------------------------------+ +
  * +---------------------------------------------------------+

  * @endcode
  *
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
  * of the selected of transaction.
  *
  * fillSummary() will be called and should update the summary line.
  */
class KLedgerViewCheckings : public KLedgerView  {
   Q_OBJECT

  friend class kMyMoneyTransactionFormTable;

public:
  KLedgerViewCheckings(QWidget *parent=0, const char *name=0);
  ~KLedgerViewCheckings();

public slots:
  /**
    * refresh the current view
    */
  virtual void refreshView(void);

  void slotActionSelected(int transactionType);

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

  /**
    * Called when the amount field has been changed by the user.
    * m_transaction and m_split will be updated accordingly.
    *
    * @param amount const reference to the amount value
    */
  virtual void slotAmountChanged(const QString& amount);

protected:
  /**
    * This method updates the summary line with the actual values.
    */
  void fillSummary(void);

  /**
    * This method updates the static text areas within the transaction form
    * according to the currently loaded values in m_transaction and m_split.
    */
  void fillFormStatics(void);

  /**
    * This method updates the variable areas within the transaction form
    * according to the currently loaded values in m_transaction and m_split.
    */
  void fillForm(void);

  /**
    * This method returns the index of the tab that is required for
    * the transaction @p t and split @p s.
    *
    * @param t transaction which should be used
    * @param s split which should be used
    */
  int actionTab(const MyMoneyTransaction& t, const MyMoneySplit& s) const;

  void resizeEvent(QResizeEvent*);

  /**
    * This destroys and hides the widgets used to edit a transaction.
    */
  void destroyWidgets(void);

  void updateTabBar(const MyMoneyTransaction& t, const MyMoneySplit& s);

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

  /**
    * This method creates the context menu that is accessible via the
    * Account... button in the transaction form.
    */
  virtual void createAccountMenu(void);

  /**
    * This method enables or disables widgets who's availability depends
    * on a selected account. These are:
    *
    * - the account details button
    * - the account reconcile button
    * - and the one's controlled by KLedgerView::enableWidgets()
    *
    * More widgets can be controlled if you override this method
    * in a derived class.
    *
    * @param enable true enables the widgets, false disables them
    */
  virtual void enableWidgets(const bool enable);

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

  void reloadEditWidgets(const MyMoneyTransaction& t);

  void slotPayeeSelected(void);

private:
  /**
    * This method loads the data of the current transaction into the
    * widgets created with createEditWidgets(). If different widgets are
    * required for in-register and in-form editing, all widgets will be filled.
    */
  void loadEditWidgets(void);

  /**
    * This method arranges the widgets required for in-form editing. It destroys
    * all widgets that have been created specifically for in-register editing.
    * A pointer to the widget that should receive the focus is returned.
    *
    * @return pointer to the widget that should receive focus when editing starts.
    */
  QWidget* arrangeEditWidgetsInForm(void);

  /**
    * This method arranges the widgets required for in-register editing. It destroys
    * all widgets that have been created specifically for in-form editing.
    * A pointer to the widget that should receive the focus is returned.
    *
    * @return pointer to the widget that should receive focus when editing starts.
    */
  QWidget* arrangeEditWidgetsInRegister(void);

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
    * This method is used by the constructor to create the transaction form
    * provided by the view.
    */
  void createForm(void);

  /**
    * This method is used to construct the reconciliation frame shown underneath
    * the summary line during reconciliation.
    */
  void createReconciliationFrame(void);

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

  /**
    * This method returns information if the Nr field is to be
    * displayed and available for the split @p split.
    *
    * @param trans the transaction to be checked
    * @param split the split to be checked
    * @retval true Nr field is required
    * @retval false Nr field is not required
    */
  const bool showNrField(const MyMoneyTransaction& trans, const MyMoneySplit& split) const;

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

protected:
  QTab* m_tabCheck;
  QTab* m_tabDeposit;
  QTab* m_tabTransfer;
  QTab* m_tabWithdrawal;
  QTab* m_tabAtm;

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
  QWidget*        m_summaryLine;

  /**
    * This member keeps a pointer to the reconciliation specific information
    */
  QFrame*         m_reconciliationFrame;

  short           m_actionIdx[5];

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

/**
  * This class implements the summary line widget. It comprises
  * of two labels and a spacer in between.
  *
  * @code
  *
  * +----------------------------------------------------+
  * | Reconcile-Label      <-spacer->      Balance-Label |
  * +----------------------------------------------------+
  *
  * @endcode
  */
class KLedgerViewCheckingsSummaryLine : public QFrame
{
  Q_OBJECT
public:
  KLedgerViewCheckingsSummaryLine(QWidget* parent, const char *name);
  virtual ~KLedgerViewCheckingsSummaryLine() {};

  void setBalance(const QString& txt);
  void setReconciliationDate(const QString& txt);
  void clear(void);

private:
  QLabel*    m_balance;
  QLabel*    m_date;
};


#endif
