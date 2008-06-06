/***************************************************************************
                          selectedtransaction.h  -  description
                             -------------------
    begin                : Tue Jun 13 2006
    copyright            : (C) 2000-2006 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SELECTEDTRANSACTION_H
#define SELECTEDTRANSACTION_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/mymoneytransaction.h>
#include <kmymoney/mymoneysplit.h>

namespace KMyMoneyRegister {

class SelectedTransaction
{
public:
  SelectedTransaction() {};
  SelectedTransaction(const MyMoneyTransaction& t, const MyMoneySplit& s) : m_transaction(t), m_split(s) {}

  MyMoneyTransaction& transaction(void) { return m_transaction; }
  const MyMoneyTransaction& transaction(void) const { return m_transaction; }
  MyMoneySplit& split(void) { return m_split; }
  const MyMoneySplit& split(void) const { return m_split; }

private:
  MyMoneyTransaction      m_transaction;
  MyMoneySplit            m_split;
};

class Register;

class SelectedTransactions:public QValueList<SelectedTransaction>
{
public:
  SelectedTransactions() {};
  SelectedTransactions(const Register* r);
};

} // namespace

#endif

