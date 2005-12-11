/***************************************************************************
                          mymoneyaccount.h
                          -------------------
    copyright            : (C) 2002 by Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// ----------------------------------------------------------------------------
// QT Includes

#include <qstring.h>
#include <qcstring.h>
#include <qdatetime.h>
#include <qvaluelist.h>
#include <qstringlist.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/mymoneymoney.h>
#include <kmymoney/mymoneyobject.h>
#include <kmymoney/mymoneykeyvaluecontainer.h>
#include <kmymoney/export.h>
#include "mymoneyutils.h"
class MyMoneyTransaction;
class MyMoneyInstitution;
class MyMoneyFile;


/**
  * A representation of an account.
  * This object represents any type of account, those held at an
  * institution as well as the accounts used for double entry
  * accounting.
  *
  * Currently, the following account types are known:
  *
  * @li  UnknownAccountType
  * @li  Checkings
  * @li  Savings
  * @li  Cash
  * @li  CreditCard
  * @li  Loan (collected)
  * @li  CertificateDep
  * @li  Investment
  * @li  MoneyMarket
  * @li  Currency
  * @li  Asset
  * @li  Liability
  * @li  Income
  * @li  Expense
  * @li  Loan (given)
  * @li  Equity
  *
  * @see MyMoneyInstitution
  * @see MyMoneyFile
  *
  * @author Michael Edwardes 2000-2001
  * @author Thomas Baumgart 2002
  *
**/
class KMYMONEY_EXPORT MyMoneyAccount : public MyMoneyObject, public MyMoneyKeyValueContainer
{
public:

/*
  class Transaction
  {
  public:
    Transaction() {};
    Transaction(const QCString& transaction, const MyMoneyMoney& balance) {
      m_transaction = transaction;
      m_balance = balance;
    };
    const QCString& transactionID(void) const { return m_transaction; };
    const MyMoneyMoney& balance(void) const { return m_balance; };

  private:
    QCString m_transaction;
    MyMoneyMoney m_balance;
  };
*/

  /**
    * Account types currently supported.
    */
  typedef enum _accountTypeE {
    UnknownAccountType=0, /**< For error handling */
    Checkings,            /**< Standard checking account */
    Savings,              /**< Typical savings account */
    Cash,                 /**< Denotes a shoe-box or pillowcase stuffed
                               with cash */
    CreditCard,           /**< Credit card accounts */
    Loan,                 /**< Loan and mortgage accounts (liability) */
    CertificateDep,       /**< Certificates of Deposit */
    Investment,           /**< Investment account */
    MoneyMarket,          /**< Money Market Account */
    Asset,                /**< Denotes a generic asset account.*/
    Liability,            /**< Denotes a generic liability account.*/
    Currency,             /**< Denotes a currency trading account. */
    Income,               /**< Denotes an income account */
    Expense,              /**< Denotes an expense account */
    AssetLoan,            /**< Denotes a loan (asset of the owner of this object) */
    Stock,                /**< Denotes an security account as sub-account for an investment */
    Equity,               /**< Denotes an equity account e.g. opening/closeing balance */

    /* insert new account types above this line */
    MaxAccountTypes       /**< Denotes the number of different account types */
  }accountTypeE;

  /**
    * This is the constructor for a new empty account
    */
  MyMoneyAccount();

  /**
    * This is the constructor for a new account known to the current file
    * This is the only constructor that will set the attribute m_openingDate
    * to a correct value.
    *
    * @param id id assigned to the account
    * @param right account definition
    */
  MyMoneyAccount(const QCString& id, const MyMoneyAccount& right);

  /**
    * This is the destructor for any MyMoneyAccount object
    */
  ~MyMoneyAccount();

  /**
    * This operator tests for equality of two MyMoneyAccount objects
    */
  const bool operator == (const MyMoneyAccount &) const;

  /**
    * This converts the account type into one of the four
    * major account types liability, asset, expense or income.
    *
    * The current assignment is as follows:
    *
    * - Asset
    *   - Asset
    *   - Checkings
    *   - Savings
    *   - Cash
    *   - Currency
    *   - Investment
    *   - MoneyMarket
    *   - CertificateDep
    *
    * - Liability
    *   - Liability
    *   - CreditCard
    *   - Loan
    *
    * - Income
    *   - Income
    *
    * - Expense
    *   - Expense
    *
    * @param type actual account type
    * @return accountTypeE of major account type
    */
  static const MyMoneyAccount::accountTypeE accountGroup(MyMoneyAccount::accountTypeE type);

  const MyMoneyAccount::accountTypeE accountGroup(void) const;

  /**
    * This method returns the id of the MyMoneyInstitution object this account
    * belongs to.
    * @return id of MyMoneyInstitution object. QString() if it is
    *         an internal account
    * @see setInstitution
    */
  const QCString institutionId(void) const { return m_institution; }

  /**
    * This method returns the name of the account
    * @return name of account
    * @see setName()
    */
  const QString name(void) const { return m_name; }

  /**
    * This method returns the number of the account at the institution
    * @return number of account at the institution
    * @see setNumber
    */
  const QString number(void) const { return m_number; }

  /**
    * This method returns the descriptive text of the account.
    * @return description of account
    * @see setDescription
    */
  const QString description(void) const { return m_description; }

  /**
    * This method returns the opening date of this account
    * @return date of opening of this account as const QDate value
    * @see setOpeningDate()
    */
  const QDate openingDate(void) const { return m_openingDate; }

  /**
    * This method is used to return the opening balance of an account
    *
    * @return MyMoneyMoney value of opening balance
    */
  __attribute__ ((deprecated)) const MyMoneyMoney openingBalance(void) const { return m_openingBalance; };

  /**
    * This method returns the date of the last reconciliation of this account
    * @return date of last reconciliation as const QDate value
    * @see setLastReconciliationDate
    */
  const QDate lastReconciliationDate(void) const { return m_lastReconciliationDate; }

  /**
    * This method returns the date the account was last modified
    * @return date of last modification as const QDate value
    * @see setLastModified
    */
  const QDate lastModified(void) const { return m_lastModified; }

  /**
    * This method is used to get the number of transactions
    * in this account.
    *
    * @return number of transactions in the account
    */
/*
  const int transactionCount(void) const { return m_transactionList.count(); }
*/

  /**
    * This method returns the current total balance of this account's
    * transactions. Sub-Accounts are not included.
    * @return const reference to MyMoneyMoney balance of split values
    * @see balanceTotal()
    */
/*
  const MyMoneyMoney balance(void) const;
*/

  /**
    * This method is used to return the ID of the parent account
    * @return QCString with the ID of the parent of this account
    */
  const QCString parentAccountId(void) const { return m_parentAccount; };

  /**
    * This method is used to return the transaction id and balance
    * at the given index. If the index is invalid or larger than
    * the count of transactions in the list, an exception will be
    * thrown.
    *
    * @param idx index of transaction in list
    * @return id and balance of transaction
    */
/*
  const MyMoneyAccount::Transaction& transaction(const int idx) const;
*/
  /**
    * This method returns the list of transactions and balances
    * @return const QValueList<MyMoneyAccount::Transaction>& transactionList
    */
/*
  const QValueList<MyMoneyAccount::Transaction>& transactionList(void) const;
*/

  /**
    * This method is used by the mymoney engine to add a transaction
    * of the journal to this account
    * @param transaction reference to MyMoneyAccount::Transaction
    */
/*
  void addTransaction(const MyMoneyAccount::Transaction& val);
*/

  /**
    * This method is used to clear the list of transactions
    */
/*
  void clearTransactions(void);
*/

  /**
    * This method returns the list of the account id's of
    * subordinate accounts
    * @return QStringList account ids
    */
  const QCStringList& accountList(void) const { return m_accountList; };

  /**
    * This method returns the number of entries in the m_accountList
    * @return number of entries in the accountList
    */
  const int accountCount(void) const { return m_accountList.count(); };

  /**
    * This method is used to add an account id as sub-ordinate account
    * @param account const QString reference to account ID
    */
  void addAccountId(const QCString& account);

  /**
    * This method is used to remove an account from the list of
    * sub-ordinate accounts.
    * @param account const QString reference to account ID to be removed
    */
  void removeAccountId(const QCString& account);

  /**
    * This method is used to modify the date of the last
    * modification access.
    * @param date date of last modification
    * @see lastModified
    */
  void setLastModified(const QDate& date);

  /**
    * This method is used to set the name of the account
    * @param name name of the account
    * @see name
    */
  void setName(const QString& name);

  /**
    * This method is used to set the number of the account at the institution
    * @param number number of the account
    * @see number
    */
  void setNumber(const QString& number);

  /**
    * This method is used to set the descriptive text of the account
    *
    * @param desc text that serves as description
    * @see setDescription
    */
  void setDescription(const QString& desc);

  /**
    * This method is used to set the id of the institution this account
    * belongs to.
    *
    * @param id id of the institution this account belongs to
    * @see institution
    */
  void setInstitutionId(const QCString& id);

  /**
    * This method is used to set the opening date information of an
    * account.
    *
    * @param date QDate of opening date
    * @see openingDate
    */
  void setOpeningDate(const QDate& date);

  /**
    * This method is used to set the opening balance information of an
    * account.
    *
    * @param balance MyMoneyMoney value of opening balance
    *
    * @see openingBalance
    */
  void setOpeningBalance(const MyMoneyMoney& balance) __attribute__ ((deprecated));

  /**
    * This method is used to set the date of the last reconciliation
    * of an account.
    * @param date QDate of last reconciliation
    * @see lastReconciliationDate
    */
  void setLastReconciliationDate(const QDate& date);

  /**
    * This method is used to change the account type
    *
    * @param type account type
    */
  void setAccountType(const accountTypeE type);

  /**
    * This method returns the account type based on the string that is read from
    * the XML file.
    *
    * @param QCString reference to type from XML file.
    */
  // void setAccountTypeByName(const QCString& strType);

  /**
    * This method is used to set a new parent account id
    * @param parent QString reference to new parent account
    */
  void setParentAccountId(const QCString& parent);

#if 0
  /**
    * This method is used to set a new account id
    * @param id QString reference to new id.
    */
  void setAccountId(const QCString& id);
#endif

  /**
    * This method is used to update m_lastModified to the current date
    */
  void touch(void) { setLastModified(QDate::currentDate()); }

  /**
    * This method returns the type of the account.
    */
  const accountTypeE accountType(void) const {return m_accountType; };

  /**
    * This method retrieves the id of the currency used with this account.
    * If the return value is empty, the base currency should be used.
    *
    * @return id of currency
    */
  const QCString currencyId(void) const { return m_currencyId; };

  /**
    * This method sets the id of the currency used with this account.
    *
    * @param id ID of currency to be associated with this account.
    */
  void setCurrencyId(const QCString& id);

  void writeXML(QDomDocument& document, QDomElement& parent) const;

  void readXML(const QDomElement& node);

  /**
    * This method checks if a reference to the given object exists. It returns,
    * a @p true if the object is referencing the one requested by the
    * parameter @p id. If it does not, this method returns @p false.
    *
    * @param id id of the object to be checked for references
    * @retval true This object references object with id @p id.
    * @retval false This object does not reference the object with id @p id.
    */
  virtual bool hasReferenceTo(const QCString& id) const;

private:
  /**
    * This member variable identifies the type of account
    */
  accountTypeE m_accountType;

  /**
    * This member variable keeps the ID of the MyMoneyInstitution object
    * that this object belongs to.
    */
  QCString m_institution;

#if 0
  /**
    * This member variable keeps the ID of the institution under which it
    * is known inside the MyMoneyFile.
    */
  QCString  m_id;
#endif

  /**
    * This member variable keeps the name of the account
    * It is solely for documentation purposes and it's contents is not
    * used otherwise by the mymoney-engine.
    */
  QString m_name;

  /**
    * This member variable keeps the account number at the institution
    * It is solely for documentation purposes and it's contents is not
    * used otherwise by the mymoney-engine.
    */
  QString m_number;

  /**
    * This member variable is a description of the account.
    * It is solely for documentation purposes and it's contents is not
    * used otherwise by the mymoney-engine.
    */
  QString m_description;

  /**
    * This member variable keeps the date when the account
    * was last modified.
    */
  QDate m_lastModified;

  /**
    * This member variable keeps the date when the
    * account was created as an object in a MyMoneyFile
    */
  QDate m_openingDate;

  /**
    * This member holds the opening balance of this account
    */
  MyMoneyMoney m_openingBalance;

  /**
    * This member variable keeps the date of the last
    * reconciliation of this account
    */
  QDate m_lastReconciliationDate;

  /**
    * This member variable keeps a sorted list of transactions keys
    * of those transactions that have a split that references
    * this account and the balance of the account after processing of
    * the corresponding split.
    */
/*
  QValueList<MyMoneyAccount::Transaction> m_transactionList;
*/

  /**
    * This member holds the ID's of all sub-ordinate accounts
    */
  QCStringList m_accountList;

  /**
    * This member contains the ID of the parent account
    */
  QCString m_parentAccount;

  /**
    * This member contains the ID of the currency associated with this account
    */
  QCString m_currencyId;
};

/**
  * This class is a convenience class to access data for loan accounts.
  * It does contain the same member variables as a MyMoneyAccount object,
  * but serves a set of getter/setter methods to ease the access to
  * laon relevant data stored in the key value container of the MyMoneyAccount
  * object.
  */
class KMYMONEY_EXPORT MyMoneyAccountLoan : public MyMoneyAccount
{
public:
  enum interestDueE {
    paymentDue = 0,
    paymentReceived
  };

  MyMoneyAccountLoan() {};
  MyMoneyAccountLoan(const MyMoneyAccount&);
  ~MyMoneyAccountLoan() {};

  const MyMoneyMoney loanAmount(void) const;
  void setLoanAmount(const MyMoneyMoney& amount);
  const MyMoneyMoney interestRate(const QDate& date) const;
  void setInterestRate(const QDate& date, const MyMoneyMoney& rate);
  const interestDueE interestCalculation(void) const;
  void setInterestCalculation(const interestDueE onReception);
  const QDate nextInterestChange(void) const;
  void setNextInterestChange(const QDate& date);
  const QCString schedule(void) const;
  void setSchedule(const QCString& sched);
  const bool fixedInterestRate(void) const;
  void setFixedInterestRate(const bool fixed);
  const MyMoneyMoney finalPayment(void) const;
  void setFinalPayment(const MyMoneyMoney& finalPayment);
  const unsigned int term(void) const;
  void setTerm(const unsigned int payments);
  const int interestChangeFrequency(int* unit = 0) const;
  void setInterestChangeFrequency(const int amount, const int unit);
  const MyMoneyMoney periodicPayment(void) const;
  void setPeriodicPayment(const MyMoneyMoney& payment);
  const QCString payee(void) const;
  void setPayee(const QCString& payee);
  const QCString interestAccountId(void) const;
  void setInterestAccountId(const QCString& id);

  /**
    * This method checks if a reference to the given object exists. It returns,
    * a @p true if the object is referencing the one requested by the
    * parameter @p id. If it does not, this method returns @p false.
    *
    * @param id id of the object to be checked for references
    * @retval true This object references object with id @p id.
    * @retval false This object does not reference the object with id @p id.
    */
  virtual bool hasReferenceTo(const QCString& id) const;

};

#endif


