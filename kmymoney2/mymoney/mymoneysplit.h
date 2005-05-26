/***************************************************************************
                          mymoneysplit.h  -  description
                             -------------------
    begin                : Sun Apr 28 2002
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

#ifndef MYMONEYSPLIT_H
#define MYMONEYSPLIT_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// ----------------------------------------------------------------------------
// QT Includes

#include <qstring.h>
#include <qdatetime.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyexception.h"
#include "mymoneyutils.h"
#include "mymoneymoney.h"
#include <kmymoney/export.h>

/**
  * @author Thomas Baumgart
  */

/**
  * This class represents a split of a transaction.
  */
class KMYMONEY_EXPORT MyMoneySplit {
public:
  /**
    * This enum defines the possible reconciliation states a split
    * can be in. Possible values are as follows:
    *
    * @li NotReconciled
    * @li Cleared
    * @li Reconciled
    * @li Frozen
    *
    * Whenever a new split is created, it has the status NotReconciled. It
    * can be set to cleared when the transaction has been performed. Once the
    * account is reconciled, cleared splits will be set to Reconciled. The
    * state Frozen will be used, when the concept of books is introduced into
    * the engine and a split must not be changed anymore.
    */
  enum reconcileFlagE {
    NotReconciled = 0,
    Cleared,
    Reconciled,
    Frozen
  };

  MyMoneySplit();
  ~MyMoneySplit();

  bool operator == (const MyMoneySplit&) const;

  const MyMoneyMoney shares(void) const { return m_shares; };
  const MyMoneyMoney value(void) const { return m_value; };
  const MyMoneyMoney value(const QCString& transactionCurrencyId, const QCString& splitCurrencyId) const;
  const QCString accountId(void) const { return m_account; };
  const QString memo(void) const { return m_memo; };
  const reconcileFlagE reconcileFlag(void) const { return m_reconcileFlag; };
  const QDate reconcileDate(void) const { return m_reconcileDate; };
  const QCString id(void) const { return m_id; };
  const QCString payeeId(void) const { return m_payee; };
  const QCString action(void) const { return m_action; };
  const QString number(void) const { return m_number; };
  const bool isAmortizationSplit(void) const { return m_action == ActionAmortization; };

  void setShares(const MyMoneyMoney& shares);
  void setValue(const MyMoneyMoney& value);

  /**
    * This method is used to set either the shares or the value depending on
    * the currencies assigned to the split/account and the transaction.
    *
    * If @p transactionCurrencyId equals @p splitCurrencyId this method
    * calls setValue(MyMoneyMoney) otherwise setShares(MyMoneyMoney).
    *
    * @param value the value to be assiged
    * @param transactionCurrencyId the id of the currency assigned to the transaction
    * @param splitCurrencyId the id of the currency assigned to the split (i.e. the
    *                        the id of the currency assigned to the account that is
    *                        referenced by the split)
    */
  void setValue(const MyMoneyMoney& value, const QCString& transactionCurrencyId, const QCString& splitCurrencyId);

  void setAccountId(const QCString& account);
  void setMemo(const QString& memo);
  void setReconcileFlag(const reconcileFlagE flag);
  void setReconcileDate(const QDate date);
  void setId(const QCString& id);
  void setPayeeId(const QCString& payee);
  void setAction(const QCString& action);
  void setNumber(const QString& number);

  static const char ActionCheck[];
  static const char ActionDeposit[];
  static const char ActionTransfer[];
  static const char ActionWithdrawal[];
  static const char ActionATM[];

  static const char ActionAmortization[];
  static const char ActionInterest[];

  static const char ActionBuyShares[];  // negative amount is sellShares
  static const char ActionDividend[];
  static const char ActionReinvestDividend[];
  static const char ActionYield[];
  static const char ActionAddShares[];  // negative amount is removeShares

private:
  /**
    * This member contains the ID of the transaction
    */
  QCString      m_id;

  /**
    * This member contains the ID of the payee
    */
  QCString      m_payee;

  /**
    * This member contains the ID of the account
    */
  QCString      m_account;

  /**
    */
  MyMoneyMoney  m_shares;

  /**
    */
  MyMoneyMoney  m_value;

  QString       m_memo;

  /**
    * This member contains information about the reconciliation
    * state of the split. Possible values are
    *
    * @li NotReconciled
    * @li Cleared
    * @li Reconciled
    * @li Frozen
    *
    */
  reconcileFlagE m_reconcileFlag;

  /**
    * In case the reconciliation flag is set to Reconciled or Frozen
    * this member contains the date of the reconciliation.
    */
  QDate         m_reconcileDate;

  /**
    * The m_action member is an arbitrary string, but is intended to
    * be conveniently limited to a menu of selections such as
    * "Buy", "Sell", "Interest", etc.
    */
  QCString      m_action;

  /**
    * The m_number member is used to store a reference number to
    * the split supplied by the user (e.g. check number, etc.).
    */
  QString       m_number;
};

#endif
