/***************************************************************************
                          mymoneyaccount.h
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes
    email                : Michael.Edwardes@students.dmu.ac.uk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MYMONEYACCOUNT_H
#define MYMONEYACCOUNT_H

#include <qstring.h>
#include <qlist.h>
#include "mymoneytransaction.h"
#include "mymoneymoney.h"
#include "mymoneyscheduled.h"

// This class represents an Account held at a Bank
class MyMoneyAccount {
public:
  enum accountTypeE { Unknown_Account, Savings, Current };

private:
  // Account details
  QString m_accountName;
  QString m_accountNumber;
  accountTypeE m_accountType;
  unsigned long m_lastId;
  QString m_description;
  QDate m_lastReconcile;
  MyMoneyMoney m_balance;  // Recalculated by balance()
  MyMoneyScheduled m_scheduled;

  // A list of all the transactions
  QList<MyMoneyTransaction> m_transactions;

  // Object reading/saving code to help in saving/reading of files
  friend QDataStream &operator<<(QDataStream &, const MyMoneyAccount &);
  friend QDataStream &operator>>(QDataStream &, MyMoneyAccount &);

  bool findTransactionPosition(const MyMoneyTransaction& transaction, unsigned int&);

public:
	MyMoneyAccount();
  MyMoneyAccount(const QString& name, const QString& number, accountTypeE type, const QString& description, const QDate& lastReconcile);
	~MyMoneyAccount();

  // Simple get operations
  QString name(void) const { return m_accountName; }
  QString accountNumber(void) { return m_accountNumber; }
  QString accountName(void) { return m_accountName; }
  accountTypeE accountType(void) { return m_accountType; }
  QString description(void) { return m_description; }
  QDate lastReconcile(void) { return m_lastReconcile; }
  MyMoneyScheduled scheduled(void) { return m_scheduled; }

  MyMoneyMoney balance(void) const;

  // Simple set operations
  void setName(const QString& name) { m_accountName = name; }
  void setAccountNumber(const QString& number) { m_accountNumber = number; }
  void setLastId(const long id) { m_lastId = id; }
  void setAccountType(MyMoneyAccount::accountTypeE type) { m_accountType = type; }
  void setDescription(const QString& description) { m_description = description; }
  void setLastReconcile(const QDate& date) { m_lastReconcile = date; }

  // Operations on transactions
  MyMoneyTransaction* transaction(const MyMoneyTransaction& transaction);
	MyMoneyTransaction* transactionFirst(void);
	MyMoneyTransaction* transactionNext(void);
	MyMoneyTransaction* transactionLast(void);
	MyMoneyTransaction* transactionAt(int index);
  unsigned int transactionCount(void) const;
  bool removeTransaction(const MyMoneyTransaction& transaction);
  bool addTransaction(MyMoneyTransaction::transactionMethod methodType, const QString& number, const QString& memo,
                     const MyMoneyMoney& amount, const QDate& date, const QString& categoryMajor, const QString& categoryMinor, const QString& atmName,
                     const QString& fromTo, const QString& bankFrom, const QString& bankTo, MyMoneyTransaction::stateE state);

  void clear(void);

  bool operator == (const MyMoneyAccount&);
  // Copy constructors
  MyMoneyAccount(const MyMoneyAccount&);
  MyMoneyAccount& operator = (const MyMoneyAccount&);
};

#endif
