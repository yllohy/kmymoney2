/***************************************************************************
                          kledgerviewloan.h  -  description
                             -------------------
    begin                : Sat Sep 13 2003
    copyright            : (C) 2003 by Thomas Baumgart
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

#ifndef KLEDGERVIEWLOAN_H
#define KLEDGERVIEWLOAN_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qwidget.h>
class QLabel;
class QHBoxLayout;

// ----------------------------------------------------------------------------
// KDE Includes

class KPushButton;

// ----------------------------------------------------------------------------
// Project Includes

#include <kledgerview.h>
class kMyMoneyTransactionFormTable;

/**
  * @author Thomas Baumgart
  */

class KLedgerViewLoan : public KLedgerView  {
   Q_OBJECT
public:
  KLedgerViewLoan(QWidget *parent=0, const char *name=0);
  ~KLedgerViewLoan();

  /// This has to be included for internal reasons, no API change
  bool eventFilter(QObject* o, QEvent* e);

public slots:
  /**
    * refresh the current view
    */
  virtual void refreshView(void);

  /**
    *
    */
  virtual void slotRegisterClicked(int row, int col, int button, const QPoint &mousePos);

  void slotRegisterDoubleClicked(int row, int col, int button, const QPoint &mousePos);

  /**
    * Calling this slot starts the reconciliation of a loan account.
    * A wizard is used to collect the necessary statement data from the
    * user and when the user accepts the data, then the matching
    * transactions of this account will be reconciled.
    * Any active editing of a transaction is cancelled.
    */
  virtual void slotReconciliation(void);

protected:
  void fillSummary(void);

  void fillForm(void);

  void fillFormStatics(void);

  void resizeEvent(QResizeEvent*);

  /**
    * This destroys and hides the widgets used to edit a transaction.
    */
  void destroyWidgets(void);

  virtual void updateTabBar(const MyMoneyTransaction& t, const MyMoneySplit& s, const bool enableAll = false);

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
    * Calling this slot opens the edit loan account wizard for the current
    * selected account.
    */
  virtual void slotLoanAccountDetail(void);

  /**
    * Calling this slot opens the account edit dialog for the current
    * selected account.
    */
  virtual void slotAccountDetail(void);

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
  /**
    * This member keeps a pointer to the summary line
    * which is located underneath the register. The
    * widget itself is created in createSummary()
    */
  QWidget*        m_summaryLine;
};

#endif
