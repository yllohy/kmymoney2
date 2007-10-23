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

#include <kmymoney/mymoneymoney.h>
#include <kmymoney/mymoneyaccount.h>
#include <kmymoney/mymoneytransaction.h>

class kMyMoneyEdit;
class kMyMoneyLineEdit;

#include "../dialogs/ksplittransactiondlgdecl.h"

/**
  * @author Thomas Baumgart
  */

class KSplitTransactionDlg : public KSplitTransactionDlgDecl
{
  Q_OBJECT

public:
  KSplitTransactionDlg(const MyMoneyTransaction& t,
                       const MyMoneySplit& s,
                       const MyMoneyAccount& acc,
                       const bool amountValid,
                       const bool deposit,
                       const MyMoneyMoney& calculatedValue,
                       const QMap<QCString, MyMoneyMoney>& priceInfo,
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

private:
  /**
    * This method updates the display of the sums below the register
    */
  void updateSums(void);

public slots:
  int exec(void);

protected slots:
  void accept();
  void reject();
  void slotClearAllSplits();
  void slotSetTransaction(const MyMoneyTransaction& t);
  void slotCreateCategory(const QString& txt, QCString& id);

  /// used internally to setup the initial size of all widgets
  void initSize(void);

signals:
  /**
    * This signal is sent out, when a new category needs to be created
    * Depending on the setting of either a payment or deposit, the parent
    * account will be preset to Expense or Income.
    *
    * @param account reference to account info. Will be filled by called slot
    * @param parent reference to parent account
    */
  void createCategory(MyMoneyAccount& account, const MyMoneyAccount& parent);

  /**
    * Signal is emitted, if any of the widgets enters (@a state equals @a true)
    *  or leaves (@a state equals @a false) object creation mode.
    *
    * @param state Enter (@a true) or leave (@a false) object creation
    */
  void objectCreation(bool state);

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
    * This member keeps a copy of the currently selected split
    */
  MyMoneySplit           m_split;

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
