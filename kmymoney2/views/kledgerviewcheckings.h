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

  friend kMyMoneyTransactionFormTable;

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

  void showWidgets(void);
  void hideWidgets(void);

  virtual bool focusNextPrevChild(bool next);

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
