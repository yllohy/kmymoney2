/***************************************************************************
                          mymoneyaccount.h
                             -------------------
    copyright            : (C) 2000-2001 by Michael Edwardes
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

#ifndef MYMONEYACCOUNT_H
#define MYMONEYACCOUNT_H

#include <qstring.h>
#include <qlist.h>
#include <qobject.h>
#include "mymoneytransaction.h"
#include "mymoneymoney.h"
#include "mymoneyscheduled.h"

class MyMoneyBank;

/**
  * A representation of an account typically held at a bank.  This class currently
  * only supports two types of account - current & savings.
  *
  * @see MyMoneyBank
  * @see MyMoneyTransaction
  *
  * @author Michael Edwardes 2000-2001
  * $Id: mymoneyaccount.h,v 1.15 2001/08/20 00:21:51 mte Exp $
  *
  * @short Representation of an account which holds transactions.
**/
class MyMoneyAccount : public QObject {
  Q_OBJECT

public:
  /**
    Account types currently supported.
  **/
  enum accountTypeE {
    Unknown_Account, /**< For error handling */
    Savings, /**< Typical savings account */
    Current /**< Typical current account */
  };

private:
  MyMoneyBank *m_parent;
  // Account details
  QString m_accountName;
  QString m_accountNumber;
  accountTypeE m_accountType;
  unsigned long m_lastId;
  QString m_description;
  QDate m_lastReconcile;
  MyMoneyMoney m_balance;  // Recalculated by balance()
  MyMoneyScheduled m_scheduled;
  QDate m_openingDate;
  MyMoneyMoney m_openingBalance;

  // A list of all the transactions
  QList<MyMoneyTransaction> m_transactions;

  // Object reading/saving code to help in saving/reading of files
  friend QDataStream &operator<<(QDataStream &, const MyMoneyAccount &);
  friend QDataStream &operator>>(QDataStream &, MyMoneyAccount &);

  // Looks through the transaction list for a transaction !
  bool findTransactionPosition(const MyMoneyTransaction& transaction, unsigned int&);

  int convertQIFDate(char* buffer, char* format, int *da, int *mo, int *ye);
  int to_days(char *buffer, int dcount);
  int to_months(char *buffer, int mcount);
  int month_to_no(char *s_number);
  void strupper(char *buffer);
  int to_year(char *buffer, int ycount);
  char *itoa(int num, char *buffer);
  int str_has_alpha(const char *buffer, int len);
  int buffer_contains(const char *buffer, char let);
  int QDateToQIFDate(const QDate date, QString& buffer, const char* format);

public:
  /**
    * The default constructor which loads default values into the attributes.
  **/
  MyMoneyAccount();

  /**
    * The most commonly used constructor.  Initialises all the attributes to the
    * supplied arguments.
    *
    * @param name The account name.
    * @param number The account number.
    * @param type The account type either savings or current at the moment
    * @param description A description of the account. Unlimited in length.
    * @param lastReconcile TODO: remove this param, *why* is it here ?
  **/
  MyMoneyAccount(MyMoneyBank *parent, const QString& name, const QString& number, accountTypeE type,
    const QString& description, const QDate openingDate, const MyMoneyMoney openingBal,
    const QDate& lastReconcile);

  /**
    * Standard destructor.
  **/
  ~MyMoneyAccount();

  /**
    * Simple get operation.
    *
    * @return The name of the account.
  **/
  QString name(void) const { return m_accountName; }

  /**
    * Simple get operation.
    *
    * @return The account number.
  **/
  QString accountNumber(void) { return m_accountNumber; }

  /**
    * Simple get operation.
    *
    * @return The name of the account.
  **/
  QString accountName(void) { return m_accountName; }

  /**
    * Simple get operation.
    *
    * @see accountTypeE
    *
    * @return The account type.
  **/
  accountTypeE accountType(void) { return m_accountType; }

  /**
    * Simple get operation.
    *
    * @return A description of the account.
  **/
  QString description(void) { return m_description; }

  /**
    * Simple get operation.
    *
    * @return The date of the last reconciliaton.
  **/
  QDate lastReconcile(void) { return m_lastReconcile; }

  /**
    * Simple get operation.
    *
    * @see MyMoneyScheduled
    *
    * @return All the scheduled transactions for this account.
  **/
  MyMoneyScheduled scheduled(void) { return m_scheduled; }

  /** */
  QDate openingDate(void) { return m_openingDate; }

  /** */
  MyMoneyMoney openingBalance(void) { return m_openingBalance; }

  /** */
  void setOpeningDate(QDate date);

  /** */
  void setOpeningBalance(MyMoneyMoney money);

  /**
    * Calculates the balance of the account and returns it in a MyMoneyMoney
    * object.
    *
    * @see MyMoneyMoney
    *
    * @return The balance of the account.
  **/
  MyMoneyMoney balance(void) const;

  /**
    * Simple set operation.
    *
    * @param name The new name for the account.
  **/
  void setName(const QString& name);

  /**
    * Simple set operation.
    *
    * @param number The new account number for the account.
  **/
  void setAccountNumber(const QString& number);

  /**
    * Simple set operation.
    * TODO: remove this operation.  Shouldn't be needed because we update m_lastId
    * internally now (in addTransaction).
    *
    * @param id The new last id for the account.
  **/
  void setLastId(const long id);

  /**
    * Simple set operation.
    *
    * @param type The new account type.
    *
    * @see accountTypeE
  **/
  void setAccountType(MyMoneyAccount::accountTypeE type);

  /**
    * Simple set operation.
    *
    * @param description The new description for the account.
  **/
  void setDescription(const QString& description);

  /**
    * Simple set operation.
    *
    * TODO: maybe do this automatically somehow ??
    *
    * @param date The last time this account was reconciled.
  **/
  void setLastReconcile(const QDate& date);

  /**
    * Finds a transaction in the list that matches the argument.
    *
    * @param transaction The transaction to look for.
    *
    * @return The found transaction or 0 if not found.
  **/
  MyMoneyTransaction* transaction(const MyMoneyTransaction& transaction);
  
  /**
    * Gets the first transaction in the list.
    * Typically used in for statements such as:
    * for (transaction=transactionFirst(); transaction; transaction=transactionNext()) {
    *  ...
    * }
    *
    * @return The first transaction in the list or 0 if no transactions exist.
    *
    * @see transactionNext
    * @see transactionLast
  **/
  MyMoneyTransaction* transactionFirst(void);
  
  /**
    * Gets the next transaction in the list.  You must have called transactionFirst to get the
    * next !
    * Typically used in for statements such as:
    * for (transaction=transactionFirst(); transaction; transaction=transactionNext()) {
    *  ...
    * }
    *
    * @return The next transaction in the list or 0 if at end or no transactions.
    *
    * @see transactionFirst
    * @see transactionLast
  **/
  MyMoneyTransaction* transactionNext(void);
  
  /**
    * Gets the last transaction in the list.
    *
    * @return The last transaction in the list or 0 if no transactions.
    *
    * @see transactionFirst
    * @see transactionNext
  **/
  MyMoneyTransaction* transactionLast(void);
  
  /**
    * Get a transaction from a specific index in the list.
    * TODO: Why do we have this, I can't remember putting it in ??  Isn't it
    * a bit error prone.  It doesn't even use the argument it just gets
    * the last indexed transaction ?? - Michael 15-March-2001.
    *
    * @param index The index into the list (Not Used??)
    *
    * @return The current transaction
  **/
//  MyMoneyTransaction* transactionAt(int index);
  
  /**
    * Retrieve the number of transactions held in this account.
    *
    * @return The total number of transactions.
  **/
  unsigned int transactionCount(void) const;

  /**
    * Retrieve the number of transactions held in this account between the
    * specified dates.
    *
    * @return The total number of transactions between the two dates.
  **/
  unsigned int MyMoneyAccount::transactionCount(const QDate start, const QDate end);

  /**
    * Removes a specific transaction from the list.
    *
    * @param transaction The transaction to remove.
    *
    * @return Whether the remove was successful.
  **/
  bool removeTransaction(const MyMoneyTransaction& transaction);

  /** I don't want any indexed methods */
//  bool removeCurrentTransaction(unsigned int index);

  /**
    * Adds a transaction to the list.
    *
    * @param methodType The transaction method.
    * @param number The transaction number.
    * @param memo A memo for the transaction.
    * @param amount A MyMoneyMoney object containing the transaction amount.
    * @param date The date on which the transaction occurred.
    * @param categoryMajor The major category string.  TODO: use an index instead.
    * @param categoryMinor The minor category string.  TODO: use an index instead.
    * @param atmName Future expansion.
    * @param payee The payees name.
    * @param accountFrom  For a transfer transaction this is the account where the money come from.
    * TODO: remove this and accountTo and only use method calls from elsewhere e.g transferFunds(Account1, Account2).
    * @param accountTo For a transfer transaction this is the account where the money goes to.
    * TODO: see above.
    * @param state Whether or not the transaction has been reconciled.  TODO: remove this it's not needed because
    * the default of UnReconciled is always used.
    *
    * @return TRUE if the transaction has been added.
    *
    * @see MyMoneyTransaction
  **/
  bool addTransaction(MyMoneyTransaction::transactionMethod methodType, const QString& number, const QString& memo,
    const MyMoneyMoney& amount, const QDate& date, const QString& categoryMajor, const QString& categoryMinor,
    const QString& atmName, const QString& payee, const QString& accountFrom, const QString& accountTo,
    MyMoneyTransaction::stateE state);

  /**
    * Remove all the transaction from the list.
  **/
  void clear(void);

  /**
    * Equality operator.
    *
    * @param right The account to compare to.
    *
    * @return TRUE if the accounts are the same.
  **/
  bool operator == (const MyMoneyAccount& right);

  /**
    * Copy constructor.
    *
    * @param right The account to copy.
  **/

  MyMoneyAccount(const MyMoneyAccount& right);
  /**
    * Assignment operator.
    *
    * @param right The account to copy.
  **/
  MyMoneyAccount& operator = (const MyMoneyAccount& right);

  /** */
  bool readAllData(int version, QDataStream& stream);
  /** No descriptions */
  QList<MyMoneyTransaction> * getTransactionList();

  MyMoneyBank *bank(void) { return m_parent; }

  /** No descriptions */
  bool readQIFFile(const QString& name, const QString& dateFormat, int& transCount, int& catCount);

  /** No descriptions */
  bool writeQIFFile(const QString& name, const QString& dateFormat, bool expCat,bool expAcct,
                    QDate startDate, QDate endDate, int& transCount, int& catCount);

  bool validateQIFDateFormat(const char *buffer, const char *format, int& result, bool checkBuffer=true);
  const char *getQIFDateFormatErrorString(int res);

signals:
  void signalProgressCount(int);
  void signalProgress(int);

};

#endif
