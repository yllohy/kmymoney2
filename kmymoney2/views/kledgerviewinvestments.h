/***************************************************************************
                          kledgerviewinvestments.h  -  description
                             -------------------
    begin                : Sun Mar 7 2004
    copyright            : (C) 2000-2004 by Michael Edwardes
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

#ifndef KLEDGERVIEWINVESTMENTS_H
#define KLEDGERVIEWINVESTMENTS_H

// ----------------------------------------------------------------------------
// QT Includes
#include <qlayout.h>
#include <qtabbar.h>
#include <qtabwidget.h>
#include <qsignalmapper.h>

class QLabel;

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "../mymoney/mymoneysecurity.h"
#include "../widgets/kmymoneyaccountcombo.h"
#include "kledgerview.h"

class kMyMoneyEdit;
class kMyMoneyEquity;

/**
  *@author Kevin Tambascio
  */

class KLedgerViewInvestments : public KLedgerView
{
  Q_OBJECT
public:

  KLedgerViewInvestments(QWidget *parent = NULL, const char *name = NULL);
  ~KLedgerViewInvestments();

  const investTransactionTypeE transactionType(const MyMoneyTransaction& t, const MyMoneySplit& split) const;

  /**
    * This method returns a pointer to the transaction data
    * in the ledger of this account. The transaction is identified
    * by the parameter @p idx. If in inline editing mode and idx
    * points to the current transaction, then 0 is returned.
    * This functions should only be used by kMyMoneyRegister and
    * derivatives. Usual code is better off using KLedgerView::transaction().
    *
    * @param idx index into ledger starting at 0
    * @return pointer to MyMoneyTransaction object representing the
    *         selected transaction. If idx is out of bounds,
    *         0 will be returned.
    */
  KMyMoneyTransaction* transaction(const int idx) const;

  /// This has to be included for internal reasons, no API change
  bool eventFilter(QObject* o, QEvent* e);

protected:
  enum ChangedFieldE {
    None = 0,
    Shares,
    Price,
    Fees,
    Total
  };

public slots:
  void slotRegisterDoubleClicked(int row, int col, int button, const QPoint &mousePos);

  virtual void refreshView(const bool transactionFormVisible);

protected slots:
  /**
    * Calling this slot opens the account edit dialog for the current
    * selected account.
    */
  virtual void slotAccountDetail(void);
  virtual void slotTypeSelected(int type);
  virtual void slotReconciliation(void);
  virtual void slotNew();
  virtual void slotEndEdit();
  virtual void slotSecurityChanged(const QCString& id);
  const bool slotDataChanged(int field);

protected:
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
    */
  virtual void createEditWidgets();

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
    * This method updates the variable areas within the transaction form
    * according to the currently loaded values in m_transaction and m_split.
    */
  virtual void fillForm();

  /**
    * This method updates the static text areas within the transaction form
    * according to the currently loaded values in m_transaction and m_split.
    */
  virtual void fillFormStatics(void);

  /**
    * This method updates the summary line with the actual values.
    */
  virtual void fillSummary();

  /**
    * This destroys and hides the widgets used to edit a transaction.
    */
  virtual void destroyWidgets();

  virtual void reloadEditWidgets(const MyMoneyTransaction& t);
  virtual void updateTabBar(const MyMoneyTransaction& t, const MyMoneySplit& s);

  void resizeEvent(QResizeEvent*);

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

  void updateValues(int field);

private:
  /**
    * This method loads the data of the current transaction into the
    * widgets created with createEditWidgets(). If different widgets are
    * required for in-register and in-form editing, all widgets will be filled.
    */
  void loadEditWidgets(void);

  void updateEditWidgets(void);

  /**
    * This method scans the splits of the given transaction and copies the
    * splits to m_feeSplit, m_accountSplit, m_interestSplit and m_split.
    *
    * @param t const reference to MyMoneyTransaction object
    */
  void preloadInvestmentSplits(const MyMoneyTransaction& t);

  void preloadEditType(void);

  /**
    * This method is used by the constructor to create the necessary widgets
    * for the register of the view and set it up.
    */
  void createRegister(void);

  /**
    * This method is used by the constructor to create the transaction form
    * provided by the view.
    */
  void createForm(void);

  /**
    * This method is used by the constructor to create the summary line underneath
    * the register widget in the view.
    */
  void createSummary(void);

protected:
  int actionTab(const MyMoneyTransaction& t, const MyMoneySplit& split) const;

protected:

private:
  /**
    * This member keeps a pointer to the summary line
    * which is located underneath the register. The
    * widget itself is created in createSummary()
    */
  QLabel          *m_summaryLine;

  QHBoxLayout*    m_summaryLayout;

  QLabel*         m_lastReconciledLabel;

  QCString        m_action;

  investTransactionTypeE   m_transactionType;

  // the edit widgets
  QGuardedPtr<kMyMoneyEdit>         m_editShares;
  QGuardedPtr<kMyMoneyEdit>         m_editPPS;
  QGuardedPtr<kMyMoneyEdit>         m_editTotalAmount;
  QGuardedPtr<kMyMoneyEdit>         m_editFees;
  QGuardedPtr<kMyMoneyAccountCombo> m_editStockAccount;
  QGuardedPtr<kMyMoneyAccountCombo> m_editCashAccount;
  QGuardedPtr<kMyMoneyCategory>     m_editFeeCategory;

  // The stock split is kept in m_split which comes with KLedgerView
  MyMoneySplit    m_accountSplit;
  MyMoneySplit    m_feeSplit;
  MyMoneySplit    m_interestSplit;
  MyMoneySecurity m_security;

  QSignalMapper   m_editMapper;
};

#endif
