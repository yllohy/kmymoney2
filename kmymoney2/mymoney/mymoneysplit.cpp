/***************************************************************************
                          mymoneysplit.cpp  -  description
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

#include "mymoneysplit.h"

MyMoneySplit::MyMoneySplit()
{
  m_reconcileFlag = NotReconciled;
  m_account =
  m_memo =
  m_payee =
  m_id = "";
}

MyMoneySplit::~MyMoneySplit()
{
}

bool MyMoneySplit::operator == (const MyMoneySplit& right) const
{
  return
    m_id == right.m_id &&
    m_account == right.m_account &&
    m_payee == right.m_payee &&
    m_memo == right.m_memo &&
    m_reconcileDate == right.m_reconcileDate &&
    m_reconcileFlag == right.m_reconcileFlag &&
    m_shares == right.m_shares &&
    m_value == right.m_value;
}

void MyMoneySplit::setAccountId(const QCString& account)
{
  m_account = account;
}

void MyMoneySplit::setMemo(const QString& memo)
{
  m_memo = memo;
}

void MyMoneySplit::setReconcileDate(const QDate date)
{
  m_reconcileDate = date;
}

void MyMoneySplit::setReconcileFlag(const reconcileFlagE flag)
{
  m_reconcileFlag = flag;
}

void MyMoneySplit::setShares(const MyMoneyMoney& shares)
{
  m_shares = shares;
}

void MyMoneySplit::setValue(const MyMoneyMoney& value)
{
  m_value = value;
}

void MyMoneySplit::setId(const QCString& id)
{
  m_id = id;
}

void MyMoneySplit::setPayeeId(const QCString& payee)
{
  m_payee = payee;
}
