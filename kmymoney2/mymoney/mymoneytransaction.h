/***************************************************************************
                          mymoneytransaction.h
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes
    email                : mte@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MYMONEYTRANSACTION_H
#define MYMONEYTRANSACTION_H

#include <qstring.h>
#include <qdatetime.h>
#include "mymoneymoney.h"

// This class represents a Transaction in an Account
class MyMoneyTransaction {
public:
  enum transactionType { Debit, Credit };
  enum transactionMethod { Cheque, Deposit, Transfer, Withdrawal, ATM };
  enum stateE { Cleared, Reconciled, Unreconciled };

private:
  // The 'fields'
  unsigned long m_id;
  QString m_number;
  QString m_memo;
  MyMoneyMoney m_amount;
  QDate m_date;
  transactionMethod m_method;
  QString m_categoryMajor;
  QString m_categoryMinor;
  QString m_atmBankName;
  QString m_payee;
  QString m_accountFrom;
  QString m_accountTo;
  stateE m_state;
  unsigned int m_index;

  friend QDataStream &operator<<(QDataStream &, const MyMoneyTransaction &);
  friend QDataStream &operator>>(QDataStream &, MyMoneyTransaction &);

public:
  MyMoneyTransaction();
  MyMoneyTransaction(const long id, transactionMethod methodType, const QString& number, const QString& memo,
                     const MyMoneyMoney& amount, const QDate& date, const QString& categoryMajor, const QString& categoryMinor, const QString& atmName,
                     const QString& fromTo, const QString& bankFrom, const QString& bankTo, stateE state);
  ~MyMoneyTransaction();

  // Simple get operations
  QString number(void) const { return m_number; }
  QString memo(void) const { return m_memo; }
  MyMoneyMoney amount(void) const { return m_amount; }
  QDate date(void) const { return m_date; }
  long id(void) const { return m_id; }
  transactionType type(void) const;
  transactionMethod method(void) const { return m_method; }
  QString categoryMajor(void) const { return m_categoryMajor; }
  QString categoryMinor(void) const { return m_categoryMinor; }
  QString atmBankName(void) const { return m_atmBankName; }
  QString payee(void) const { return m_payee; }
  QString accountFrom(void) const { return m_accountFrom; }
  QString accountTo(void) const { return m_accountTo; }
  stateE state(void) const { return m_state; }

  // Simple set operations
//  void setId(const long id) { m_id = id; }
  void setNumber(const QString& val) { m_number = val; }
  void setMemo(const QString& val) { m_memo = val; }
  void setAmount(const MyMoneyMoney& val) { m_amount = val; }
  void setDate(const QDate& date) { m_date = date; }
  void setMethod(const transactionMethod method) { m_method = method; }
  void setCategoryMajor(const QString& major) { m_categoryMajor = major; }
  void setCategoryMinor(const QString& minor) { m_categoryMinor = minor; }
  void setAtmBankName(const QString& val) { m_atmBankName = val; }
  void setPayee(const QString& fromTo) { m_payee = fromTo; }
  void setAccountFrom(const QString& bankFrom) { m_accountFrom = bankFrom; }
  void setAccountTo(const QString& bankTo) { m_accountTo = bankTo; }
  void setState(const stateE state) { m_state = state; }

  void setIndex(const unsigned int index) { m_index = index; }
  unsigned int index(void) { return m_index; }

  bool operator == (const MyMoneyTransaction&);
  // Copy constructors
  MyMoneyTransaction(const MyMoneyTransaction&);
  MyMoneyTransaction& operator = (const MyMoneyTransaction&);

  bool readAllData(int version, QDataStream& stream);
};

#endif
