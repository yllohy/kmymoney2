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

// ----------------------------------------------------------------------------
// KDE Includes

class KPushButton;

// ----------------------------------------------------------------------------
// Project Includes

#include "kledgerview.h"
class kMyMoneyTransactionFormTable;

/**
  *@author Thomas Baumgart
  *
  * @todo in-register editing of transactions in KLedgerViewCheckings
  */

/**
  * This class represents the ledger view for checkings accounts.
  * As described with the base class KLedgerView, it consists out
  * of a register, a button line and a form.
  * The register is provided by kMyMoneyRegisterCheckings. The form
  * is maintained within this class, even though most of the members
  * required are provided by KLedgerView.
  *
  * The tabbar on top of the form shows the possible transaction types. It
  * is also loaded in the constructor of this class.
  *
  * The form is QTable-based and will be created with 4 rows and 5 columns
  * in the constructor of this class. Except for
  * the category input field, fields in col 1 also span col 2. The
  * category field provides a button in col 2 to enter the splits dialog.
  *
  * The edit widgets are created within showWidgets(). This method also
  * attaches the widgets to the table's cells using QTable::setCellWidget().
  * It also maintains the tab order.
  *
  * hideWidgets() removes all edit widgets from the form table and returns
  * to the read-only form view.
  *
  * fillForm() fills the data provided by the current selected transaction
  * into the read-only form. The layout depends on the type of transaction.
  */
class KLedgerViewCheckings : public KLedgerView  {
   Q_OBJECT

  friend class kMyMoneyTransactionFormTable;

public: 
	KLedgerViewCheckings(QWidget *parent=0, const char *name=0);
	~KLedgerViewCheckings();

  void show();

  void fillForm(void);

public slots:
  /**
    * refresh the current view
    */
  virtual void refreshView(void);

  void slotTypeSelected(int transactionType);

  void slotRegisterDoubleClicked(int row, int col, int button, const QPoint &mousePos);

protected:
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

private:
  /**
    * This method creates all widgets that allow a view to edit
    * a transaction. If different widgets are required for in-register
    * and in-form editing, both will be created. They can be destroyed
    * later on. See arrangeEditWidgetsInForm() or arrangeEditWidgetsInRegister().
    */
  void createEditWidgets(void);

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

private:
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
};

#endif
