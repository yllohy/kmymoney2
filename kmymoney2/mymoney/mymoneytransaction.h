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
#include <qlist.h>
#include "mymoneytransactionbase.h"
#include "mymoneysplittransaction.h"
#include "mymoneymoney.h"

class MyMoneyAccount;

/// This class represents a Transaction in an Account
class MyMoneyTransaction : public MyMoneyTransactionBase {
public:
  enum transactionType { Debit, Credit };
  enum transactionMethod { Cheque, Deposit, Transfer, Withdrawal, ATM };
  enum stateE { Cleared, Reconciled, Unreconciled };

private:
	MyMoneyAccount *m_parent;

  // The 'fields'
  unsigned long m_id;
  QString m_number;
  QDate m_date;
  transactionMethod m_method;
  QString m_atmBankName;
  QString m_payee;
  QString m_accountFrom;
  QString m_accountTo;
  stateE m_state;
  unsigned int m_index;

private:
  /// common initialization for copy and assignment constructor
  /// @param right reference to the right side of the assignment
  void init(MyMoneyTransaction& right);

  friend QDataStream &operator<<(QDataStream &, const MyMoneyTransaction &);
  friend QDataStream &operator>>(QDataStream &, MyMoneyTransaction &);

public:   // FIXME: for testing I made this public, must be private.
  /// List of splits
  QList<MyMoneySplitTransaction> m_splitList;


public:
  MyMoneyTransaction();
  MyMoneyTransaction(MyMoneyAccount *parent, const long id, transactionMethod methodType, const QString& number, const QString& memo,
                     const MyMoneyMoney& amount, const QDate& date, const QString& categoryMajor, const QString& categoryMinor, const QString& atmName,
                     const QString& fromTo, const QString& bankFrom, const QString& bankTo, stateE state);
  ~MyMoneyTransaction();

  // Simple get operations
  QString number(void) const { return m_number; }
  QDate date(void) const { return m_date; }
  long id(void) const { return m_id; }
  transactionType type(void) const;
  transactionMethod method(void) const { return m_method; }
  QString atmBankName(void) const { return m_atmBankName; }
  QString payee(void) const { return m_payee; }
  QString accountFrom(void) const { return m_accountFrom; }
  QString accountTo(void) const { return m_accountTo; }
  stateE state(void) const { return m_state; }
  //const QList<MyMoneySplitTransaction> splitList(void) { return m_splitList; }

  /// retrieve a pointer to the first split transaction
  /// @return pointer to first MyMoneySplitTransaction
  MyMoneySplitTransaction* const firstSplit(void);

  /// retrieve a pointer to the next split transaction
  /// @return pointer to next MyMoneySplitTransaction
  MyMoneySplitTransaction* const nextSplit(void);

  // Simple set operations
  void setNumber(const QString& val);
  void setDate(const QDate& date);
  void setMethod(const transactionMethod method);
  void setAtmBankName(const QString& val);
  void setPayee(const QString& fromTo);
  void setAccountFrom(const QString& bankFrom);
  void setAccountTo(const QString& bankTo);
  void setState(const stateE state);

  /// Set the parent's dirty flag
  /// @param flag Set the parent's dirty flag to this value. Can be true or false.
  /// @see MyMoneyTransaction
  /// @see MyMoneySplitTransaction
  void setDirty(const bool flag);

  /// clear the split transaction list
  void clearSplitList(void);

  /// add a split transaction to the list
  void addSplit(MyMoneySplitTransaction* const split);

  void setIndex(const unsigned int index);
  unsigned int index(void) { return m_index; }

  bool operator == (const MyMoneyTransaction&);

  // Copy constructors
  MyMoneyTransaction(const MyMoneyTransaction&);
  MyMoneyTransaction& operator = (const MyMoneyTransaction&);

  bool readAllData(int version, QDataStream& stream);

  /// Returns false for MyMoneyTransactions
  ///
  /// @return always false
  virtual bool isSplit(void) { return false; };

  /// Returns a pointer to the account this transaction belongs to
  /// @return pointer to account
  MyMoneyAccount *account(void) { return m_parent; }

  static transactionMethod stringToMethod(const char *method);

};

#endif
