/***************************************************************************
                          ksplittransactiondlg.h  -  description
                             -------------------
    begin                : Thu Jan 10 2002
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

#ifndef KSPLITTRANSACTIONDLG_H
#define KSPLITTRANSACTIONDLG_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <kpopupmenu.h>
#include <kiconloader.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "../mymoney/mymoneymoney.h"
#include "../mymoney/mymoneyaccount.h"
#include "../mymoney/mymoneytransaction.h"
class kMyMoneyCategory;
class kMyMoneyEdit;
class kMyMoneyLineEdit;

#include "../dialogs/ksplittransactiondlgdecl.h"

/**
  * @author Thomas Baumgart
  * @todo Add account (hierarchy) upon new category
  */

class KSplitTransactionDlg : public kSplitTransactionDlgDecl  {
  Q_OBJECT

public:
  KSplitTransactionDlg(const MyMoneyTransaction& t,
                       const MyMoneyAccount& acc,
                       const bool amountValid,
                       const bool deposit,
                       const MyMoneyMoney& calculatedValue = 0,
                       QWidget* parent = 0, const char* name = 0);

  virtual ~KSplitTransactionDlg();

  /**
    * Using this method, an external object can retrieve the result
    * of the dialog.
    *
    * @return MyMoneyTransaction based on the transaction passes during
    *         the construction of this object and modified using the
    *         dialog.
    */
  const MyMoneyTransaction& transaction(void) const { return m_transaction; };

private:
  /**
    * This method updates the display of the sums below the register
    */
  void updateSums(void);

  /**
    * This method calculates the difference between the split that references
    * the account passed as argument to the constructor of this object and
    * all the other splits shown in the register of this dialog.
    *
    * @return difference as MyMoneyMoney object
    */
  MyMoneyMoney diffAmount(void);

  /**
    * This method calculates the sum of the splits shown in the register
    * of this dialog.
    *
    * @return sum of splits as MyMoneyMoney object
    */
  MyMoneyMoney splitsValue(void);

protected slots:
  void accept();
  void reject();
  void slotClearAllSplits();
  void slotSetTransaction(const MyMoneyTransaction& t);

private slots:
  /// used internally to setup the initial size of all widgets
  void initSize(void);

private:
  /**
    * This member keeps a copy of the current selected transaction
    */
  MyMoneyTransaction     m_transaction;

  /**
    * This member keeps a copy of the currently selected account
    */
  MyMoneyAccount         m_account;

  /**
    * flag that shows that the amount specified in the constructor
    * should be used as fix value (true) or if it can be changed (false)
    */
  bool                   m_amountValid;

  /**
    * This member keeps track if the current transaction is of type
    * deposit (true) or withdrawal (false).
    */
  bool                   m_isDeposit;

  /**
    * This member keeps the amount that will be assigned to all the
    * splits that are marked 'will be calculated'.
    */
  MyMoneyMoney           m_calculatedValue;
};

#endif
