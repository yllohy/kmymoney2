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
  const QString account(void) const { return m_account; };
  const QString memo(void) const { return m_memo; };
  const reconcileFlagE reconcileFlag(void) const { return m_reconcileFlag; };
  const QDate reconcileDate(void) const { return m_reconcileDate; };
  const QString id(void) const { return m_id; };

  void setShares(const MyMoneyMoney& shares);
  void setValue(const MyMoneyMoney& value);
  void setAccount(const QString& account);
  void setMemo(const QString& memo);
  void setReconcileFlag(const reconcileFlagE flag);
  void setReconcileDate(const QDate date);
  void setID(const QString& id);

private:
  QString       m_id;
  MyMoneyMoney  m_shares;
  MyMoneyMoney  m_value;
  QString       m_account;
  QString       m_memo;
  reconcileFlagE m_reconcileFlag;
  QDate         m_reconcileDate;
};

#endif
