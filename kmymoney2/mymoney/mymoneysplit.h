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

// ----------------------------------------------------------------------------
// QT Includes

#include <qstring.h>
#include <qdatetime.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyexception.h"
#include "mymoneyutils.h"
#include "mymoneymoney.h"

/**
  * @author Thomas Baumgart
  *
  * @todo add a type field (e.g. transfer etc)
  */

class MyMoneySplit {
public: 
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
  const QCString accountId(void) const { return m_account; };
  const QString memo(void) const { return m_memo; };
  const reconcileFlagE reconcileFlag(void) const { return m_reconcileFlag; };
  const QDate reconcileDate(void) const { return m_reconcileDate; };
  const QCString id(void) const { return m_id; };
  const QCString payeeId(void) const { return m_payee; };

  void setShares(const MyMoneyMoney& shares);
  void setValue(const MyMoneyMoney& value);
  void setAccountId(const QCString& account);
  void setMemo(const QString& memo);
  void setReconcileFlag(const reconcileFlagE flag);
  void setReconcileDate(const QDate date);
  void setId(const QCString& id);
  void setPayeeId(const QCString& payee);

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
    *
    */
  reconcileFlagE m_reconcileFlag;

  /**
    * In case the reconciliation flag is set to Reconciled
    * this member contains the date of the reconciliation.
    */
  QDate         m_reconcileDate;
};

#endif
