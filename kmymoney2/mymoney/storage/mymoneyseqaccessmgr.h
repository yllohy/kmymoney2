/***************************************************************************
                          imymoneystoragestream.h  -  description
                             -------------------
    begin                : Sun May 5 2002
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

#ifndef MYMONEYSEQACCESSMGR_H
#define MYMONEYSEQACCESSMGR_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "imymoneystorage.h"
#include "imymoneyserialize.h"

/**
  *@author Thomas Baumgart
  */

class MyMoneyTransactionFilter;

/**
  * This member represents an item in the balance cache. The balance cache
  * is used for fast processing of the balance of an account. Several
  * of these objects are held by the MyMoneySeqAccessMgr() object in a map
  * with the account Id as key. If such a cache item is present in the map,
  * the contained balance of it will be used as current balance for this
  * account. If the balance is changed by any operation, the
  * MyMoneyBalanceCacheItem for the modified account will be removed from
  * the map and the next time the balance for this account is requested,
  * it has to be recalculated. After recalculation, a new MyMoneyBalanceCacheItem
  * will be created containing the new balance value.
  *
  * @see MyMoneySeqAccessMgr::balance() and
  *      MyMoneySeqAccessMgr::invalidateBalanceCache() for a usage example
  */
class MyMoneyBalanceCacheItem {
public:
  MyMoneyBalanceCacheItem() { valid = false; };
  MyMoneyBalanceCacheItem(const MyMoneyMoney& val) { balance = val; valid = true; };

  bool          valid;
  MyMoneyMoney  balance;
};

/**
  * The MyMoneySeqAccessMgr class represents the storage engine for sequential
  * files. The actual file type and it's internal storage format (e.g. binary
  * or XML) is not important and handled through the IMyMoneySerialize() interface.
  *
  * The MyMoneySeqAccessMgr must be loaded by an application using the
  * IMyMoneySerialize() interface and can then be accessed through the
  * IMyMoneyStorage() interface. All data is loaded into memory, modified
  * and kept there. It is the subject of an outside object to store the
  * modified data in a persistant storage area using the IMyMoneySerialize()
  * interface. As indication, if data has been changed, the retrun value
  * of the method dirty() can be used.
  */
class MyMoneySeqAccessMgr : public IMyMoneyStorage, public IMyMoneySerialize,
                            public MyMoneyKeyValueContainer
{
public:

  // definitions for the ID's of the standard accounts
#define STD_ACC_LIABILITY "AStd::Liability"
#define STD_ACC_ASSET     "AStd::Asset"
#define STD_ACC_EXPENSE   "AStd::Expense"
#define STD_ACC_INCOME    "AStd::Income"

	MyMoneySeqAccessMgr();
	~MyMoneySeqAccessMgr();

  // general get functions
  const QString& userName(void) const { return m_userName; };
  const QString& userStreet(void) const { return m_userStreet; };
  const QString& userTown(void) const { return m_userTown; };
  const QString& userCounty(void) const { return m_userCounty; };
  const QString& userPostcode(void) const { return m_userPostcode; };
  const QString& userTelephone(void) const { return m_userTelephone; };
  const QString& userEmail(void) const { return m_userEmail; };
  const QDate& creationDate(void) const { return m_creationDate; };
  const QDate& lastModificationDate(void) const { return m_lastModificationDate; };

  // general set functions
  void setUserName(const QString& val) { m_userName = val; touch(); };
  void setUserStreet(const QString& val) { m_userStreet = val; touch(); };
  void setUserTown(const QString& val) { m_userTown = val; touch(); };
  void setUserCounty(const QString& val) { m_userCounty = val; touch(); };
  void setUserPostcode(const QString& val) { m_userPostcode = val; touch(); };
  void setUserTelephone(const QString& val) { m_userTelephone = val; touch(); };
  void setUserEmail(const QString& val) { m_userEmail = val; touch(); };
  void setCreationDate(const QDate& val) { m_creationDate = val; touch(); };
  void setLastModificationDate(const QDate& val) { m_lastModificationDate = val; m_dirty = false; };

  /**
    * Returns the account addressed by it's id.
    *
    * @param id id of the account to locate.
    * @return reference to MyMoneyAccount object. An exception is thrown
    *         if the id is unknown
    */
  const MyMoneyAccount& account(const QCString id) const;

  /**
    * This method is used to check whether a given
    * account id references one of the standard accounts or not.
    *
    * @param id account id
    * @return true if account-id is one of the standards, false otherwise
    */
  const bool isStandardAccount(const QCString& id) const;

  /**
    * This method is used to set the name for the specified standard account
    * within the storage area. An exception will be thrown, if an error
    * occurs
    *
    * @param id QCString reference to one of the standard accounts. Possible
    *           values are:
    *
    *           @li STD_ACC_LIABILITY
    *           @li STD_ACC_ASSET
    *           @li STD_ACC_EXPENSE
    *           @li STD_ACC_INCOME
    *
    * @param name QString reference to the name to be set
    *
    */
  void setAccountName(const QCString& id, const QString& name);

  /**
    * This method is used to create a new account
    *
    * An exception will be thrown upon error conditions.
    *
    * @param account MyMoneyAccount filled with data
    */
  void newAccount(MyMoneyAccount& account);

  /**
    * This method is used to create a new payee
    *
    * An exception will be thrown upon error conditions
    *
    * @param payee MyMoneyPayee reference to payee information
    */
  void addPayee(MyMoneyPayee& payee);

  /**
    * This method is used to retrieve information about a payee
    * An exception will be thrown upon error conditions.
    *
    * @param id QCString reference to id of payee
    *
    * @return MyMoneyPayee object of payee
    */
  const MyMoneyPayee payee(const QCString& id) const;

  /**
    * This method is used to retrieve the id to a corresponding
    * name of a payee/receiver.
    * An exception will be thrown upon error conditions.
    *
    * @param payee QString reference to name of payee
    *
    * @return MyMoneyPayee object of payee
    */
  const MyMoneyPayee payeeByName(const QString& payee) const;

  /**
    * This method is used to modify an existing payee
    *
    * An exception will be thrown upon error conditions
    *
    * @param payee MyMoneyPayee reference to payee information
    */
  void modifyPayee(const MyMoneyPayee& payee);

  /**
    * This method is used to remove an existing payee
    *
    * An exception will be thrown upon error conditions
    *
    * @param payee MyMoneyPayee reference to payee information
    */
  void removePayee(const MyMoneyPayee& payee);

  /**
    * This method returns a list of the payees
    * inside a MyMoneyStorage object
    *
    * @return QValueList<MyMoneyPayee> containing the payee information
    */
  const QValueList<MyMoneyPayee> payeeList(void) const;

  /**
    * This method is used to add one account as sub-ordinate to another
    * (parent) account. The objects passed as arguments will be modified
    * accordingly.
    *
    * @param parent parent account the account should be added to
    * @param account the account to be added
    */
  void addAccount(MyMoneyAccount& parent, MyMoneyAccount& account);

  /**
    * This method is used to add an account to an institution. The account
    * data will be updated to contain the correct id of the referenced institution
    * and the account will be added to the list of accounts held at the
    * referenced institution. The objects passed as arguments will be updated
    * accordingly.
    *
    * An exception will be thrown upon error conditions.
    *
    * @param institution MyMoneyInsitution the account should be added to
    * @param account MyMoneyAccount to be added
    */
  void addAccount(MyMoneyInstitution& institution, MyMoneyAccount& account);

  /**
    * Adds an institution to the storage. A
    * respective institution-ID will be generated within this record.
    * The ID is stored as QString in the object passed as argument.
    * An exception will be thrown upon error conditions.
    *
    * @param institution The complete institution information in a
    *        MyMoneyInstitution object
    */
  void addInstitution(MyMoneyInstitution& institution);

  /**
    * Adds a transaction to the file-global transaction pool. A respective
    * transaction-ID will be generated within this record. The ID is stored
    * as QString in the transaction object. The accounts of the referenced splits
    * will be updated to have a reference to the transaction just added.
    *
    * @param transaction reference to the transaction
    * @param skipAccountUpdate if set, the transaction lists of the accounts
    *        referenced in the splits are not updated. This is used for
    *        bulk loading a lot of transactions but not during normal operation
    */
  void addTransaction(MyMoneyTransaction& transaction, const bool skipAccountUpdate = false);

  /**
    * Modifies an already existing account in the file global account pool.
    *
    * An exception will be thrown upon error conditions.
    *
    * @param account reference to the new account information
    */
  void modifyAccount(const MyMoneyAccount& account);

  /**
    * Modifies an already existing institution in the file global
    * institution pool.
    *
    * An exception will be thrown upon error conditions.
    *
    * @param institution The complete new institution information
    */
  void modifyInstitution(const MyMoneyInstitution& institution);

  /**
    * This method is used to update a specific transaction in the
    * transaction pool of the MyMoneyFile object
    *
    * An exception will be thrown upon error conditions.
    *
    * @param transaction reference to transaction to be changed
    */
  void modifyTransaction(const MyMoneyTransaction& transaction);

  /**
    * This method re-parents an existing account
    *
    * An exception will be thrown upon error conditions.
    *
    * @param account MyMoneyAccount reference to account to be re-parented
    * @param parent  MyMoneyAccount reference to new parent account
    */
  void reparentAccount(MyMoneyAccount &account, MyMoneyAccount& parent);

  /**
    * This method is used to remove a transaction from the transaction
    * pool (journal).
    *
    * @param transaction const reference to transaction to be deleted
    */
  void removeTransaction(const MyMoneyTransaction& transaction);

  /**
    * Deletes an existing account from the file global account pool
    * This method only allows to remove accounts that are not
    * referenced by any split. Use moveSplits() to move splits
    * to another account. An exception is thrown in case of a
    * problem.
    *
    * @param account reference to the account to be deleted.
    */
  void removeAccount(const MyMoneyAccount& account);

  /**
    * Deletes an existing institution from the file global institution pool
    * Also modifies the accounts that reference this institution as
    * their institution.
    *
    * @param institution institution to be deleted.
    */
  void removeInstitution(const MyMoneyInstitution& institution);

  /**
    * This method is used to extract a transaction from the file global
    * transaction pool through an id. In case of an invalid id, an
    * exception will be thrown.
    *
    * @param id id of transaction as QString.
    * @return reference to the requested transaction
    */
  const MyMoneyTransaction& transaction(const QCString& id) const;

  /**
    * This method is used to extract a transaction from the file global
    * transaction pool through an index into an account.
    *
    * @param account id of the account as QString
    * @param idx number of transaction in this account
    * @return reference to MyMoneyTransaction object
    */
  const MyMoneyTransaction& transaction(const QCString& account, const int idx) const;

  /**
    * This method is used to determince, if the account with the
    * given ID is referenced by any split in m_transactionList.
    *
    * @param id id of the account to be checked for
    * @return true if account is referenced, false otherwise
    */
  const bool hasActiveSplits(const QCString& id) const;

  /**
    * This method is used to return the actual balance of an account
    * without it's sub-ordinate accounts
    *
    * @param account id of the account in question
    * @return balance of the account as MyMoneyMoney object
    */
  const MyMoneyMoney balance(const QCString& id);

  /**
    * This method is used to return the actual balance of an account
    * including it's sub-ordinate accounts
    *
    * @param account id of the account in question
    * @return balance of the account as MyMoneyMoney object
    */
  const MyMoneyMoney totalBalance(const QCString& id);

  /**
    * Returns the institution of a given ID
    *
    * @param id id of the institution to locate
    * @return MyMoneyInstitution object filled with data. If the institution
    *         could not be found, an exception will be thrown
    */
  const MyMoneyInstitution& institution(const QCString& id) const;

  /**
    * This method returns an indicator if the storage object has been
    * changed after it has last been saved to permanent storage.
    *
    * @return true if changed, false if not
    */
  bool dirty(void) const { return m_dirty; }

  /**
    * This method returns a list of the institutions
    * inside a MyMoneyFile object
    *
    * @return QMap containing the institution information
    */
  const QValueList<MyMoneyInstitution> institutionList(void) const;

  /**
    * This method returns a list of the accounts
    * inside a MyMoneyFile object
    *
    * @return QMap<QString,MyMoneyAccount> with accounts
    */
  const QValueList<MyMoneyAccount> accountList(void) const;

  /**
    * This method is used to pull a list of transactions from the file
    * global transaction pool. It returns either the whole journal or
    * the set of transaction referenced by a specific account depending
    * on the argument given.
    *
    * @param account QCString reference to account id. If account equals ""
    +                all transactions (the journal) is returned. If account
    *                is not equal to "" it returns the set of transactions
    *                that have splits in this account.
    *
    * @return set of transactions in form of a QValueList<MyMoneyTransaction>
    */
  const QValueList<MyMoneyTransaction> transactionList(const QCString& account = "") const;

  /**
    * This method returns the number of transactions currently known to file
    * in the range 0..MAXUINT
    *
    * @param account QCString reference to account id. If account equals ""
    +                all transactions (the journal) will be counted. If account
    *                is not equal to "" it returns the number of transactions
    *                that have splits in this account.
    *
    * @return number of transactions in journal/account
    */
  const unsigned int transactionCount(const QCString& account = "") const;

  /**
    * This method returns the number of institutions currently known to file
    * in the range 0..MAXUINT
    *
    * @return number of institutions known to file
    */
  const unsigned int institutionCount(void) const;

  /**
    * This method returns the number of accounts currently known to file
    * in the range 0..MAXUINT
    *
    * @return number of accounts currently known inside a MyMoneyFile object
    */
  const unsigned int accountCount(void) const;

  /**
    * This method is used to return the standard liability account
    * @return MyMoneyAccount liability account(group)
    */
  const MyMoneyAccount liability(void) const { return account(STD_ACC_LIABILITY); };

  /**
    * This method is used to return the standard asset account
    * @return MyMoneyAccount asset account(group)
    */
  const MyMoneyAccount asset(void) const { return account(STD_ACC_ASSET); };

  /**
    * This method is used to return the standard expense account
    * @return MyMoneyAccount expense account(group)
    */
  const MyMoneyAccount expense(void) const { return account(STD_ACC_EXPENSE); };

  /**
    * This method is used to return the standard income account
    * @return MyMoneyAccount income account(group)
    */
  const MyMoneyAccount income(void) const { return account(STD_ACC_INCOME); };

  /**
    * This method reconstructs the transaction list of all accounts
    * in the m_accountList.
    *
    * @see refreshAccountTransactionList, addTransaction
    */
  void refreshAllAccountTransactionLists(void);

  virtual void loadAccount(const MyMoneyAccount& acc);
  virtual void loadTransaction(const MyMoneyTransaction& tr);
  virtual void loadInstitution(const MyMoneyInstitution& inst);
  virtual void loadPayee(const MyMoneyPayee& payee);

  virtual void loadAccountId(const unsigned long id);
  virtual void loadTransactionId(const unsigned long id);
  virtual void loadPayeeId(const unsigned long id);
  virtual void loadInstitutionId(const unsigned long id);

  virtual const unsigned long accountId(void) { return m_nextAccountID; };
  virtual const unsigned long transactionId(void) { return m_nextTransactionID; };
  virtual const unsigned long payeeId(void) { return m_nextPayeeID; };
  virtual const unsigned long institutionId(void) { return m_nextInstitutionID; };

  /**
    * This method is used to extract a value from 
    * KeyValueContainer. For details see MyMoneyKeyValueContainer::value().
    *
    * @param key const reference to QCString containing the key
    * @return QString containing the value
    */
  const QString value(const QCString& key) const;

  /**
    * This method is used to set a value in the 
    * KeyValueContainer. For details see MyMoneyKeyValueContainer::setValue().
    *
    * @param key const reference to QCString containing the key
    * @param val const reference to QString containing the value
    */
  void setValue(const QCString& key, const QString& val);

  /**
    * This method is used to delete a key-value-pair from the
    * KeyValueContainer identified by the parameter
    * @p key. For details see MyMoneyKeyValueContainer::deletePair().
    *
    * @param key const reference to QCString containing the key
    */
  void deletePair(const QCString& key);

  // documented in IMyMoneySerialize base class
  QMap<QCString, QString> pairs(void) const;

  // documented in IMyMoneySerialize base class
  void setPairs(const QMap<QCString, QString>& list);

private:

  static const int INSTITUTION_ID_SIZE = 6;
  static const int ACCOUNT_ID_SIZE = 6;
  static const int TRANSACTION_ID_SIZE = 18;
  static const int PAYEE_ID_SIZE = 6;

  static const int YEAR_SIZE = 4;
  static const int MONTH_SIZE = 2;
  static const int DAY_SIZE = 2;

  /**
    * This method is used to generate the sort-key for a transaction
    *
    * @param transaction reference to the transaction
    * @return key as QString
    */
  const QCString transactionKey(const MyMoneyTransaction& transaction) const;

  /**
    * This method is used to set the dirty flag and update the
    * date of the last modification.
    */
  void touch(void);

  /**
    * This method is used to invalidate the cached balance for
    * the selected account and all it's parents.
    *
    * @param account id of the account in question
    */
  void invalidateBalanceCache(const QCString& id);

  /**
    * This method reconstructs the transaction list of an account
    * in the m_accountList.
    *
    * @param acc iterator to the m_accountList
    */
  void refreshAccountTransactionList(QMap<QCString, MyMoneyAccount>::Iterator acc) const;

  /**
    * This member variable keeps the name of the User
    * @see userName(), setUserName()
    */
  QString m_userName;

  /**
    * This member variable keeps the street of the users address
    * @see userStreet(), setUserStreet()
    */
  QString m_userStreet;

  /**
    * This member variable keeps the name of the town the user resides in
    * @see userTown(), setUserTown()
    */
  QString m_userTown;

  /**
    * This member variable keeps the name of the county for the users address
    * @see userCounty(), setUserCounty()
    */
  QString m_userCounty;

  /**
    * This member variable keeps the zip-code of the users address
    * @see userPostcode(), setUserPostcode()
    */
  QString m_userPostcode;

  /**
    * This member variable keeps the telephone number of the user
    * @see userTelephone(), setUserTelephone()
    */
  QString m_userTelephone;

  /**
    * This member variable keeps the users e-mail address
    * @see userEmail(), setUserEmail()
    */
  QString m_userEmail;

  /**
    * The member variable m_nextInstitutionID keeps the number that will be
    * assigned to the next institution created. It is maintained by
    * nextInstitutionID().
    */
  unsigned long m_nextInstitutionID;

  /**
    * The member variable m_nextAccountID keeps the number that will be
    * assigned to the next institution created. It is maintained by
    * nextAccountID().
    */
  unsigned long m_nextAccountID;

  /**
    * The member variable m_nextTransactionID keeps the number that will be
    * assigned to the next transaction created. It is maintained by
    * nextTransactionID().
    */
  unsigned long m_nextTransactionID;

  /**
    * The member variable m_nextPayeeID keeps the number that will be
    * assigned to the next payee created. It is maintained by
    * nextPayeeID()
    */
  unsigned long m_nextPayeeID;

  /**
    * The member variable m_institutionList is the container for the
    * institutions known within this file.
    */
  QMap<QCString, MyMoneyInstitution> m_institutionList;

  /**
    * The member variable m_accountList is the container for the accounts
    * known within this file.
    */
  QMap<QCString, MyMoneyAccount> m_accountList;

  /**
    * The member variable m_balanceCache is the container for the
    * accounts actual balance
    */
  QMap<QCString, MyMoneyBalanceCacheItem> m_balanceCache;

  /**
    * The member variable m_transactionList is the container for all
    * transactions within this file.
    * @see m_transactionKeys
    */
  QMap<QCString, MyMoneyTransaction> m_transactionList;

  /**
    * The member variable m_transactionKeys is used to convert
    * transaction id's into the corresponding key used in m_transactionList.
    * @see m_transactionList;
    */
  QMap<QCString, QCString> m_transactionKeys;

  /**
    * A list containing all the payees that have been used
    */
  QMap<QCString, MyMoneyPayee> m_payeeList;

  /**
    * This member signals if the file has been modified or not
    */
  bool  m_dirty;

  /**
    * This member variable keeps the creation date of this MyMoneySeqAccessMgr
    * object. It is set during the constructor and can only be modified using
    * the stream read operator.
    */
  QDate m_creationDate;

  /**
    * This member variable keeps the date of the last modification of
    * the MyMoneySeqAccessMgr object.
    */
  QDate m_lastModificationDate;

  /**
    * This method is used to get the next valid ID for a institution
    * @return id for a institution
    */
  const QCString nextInstitutionID(void);

  /**
    * This method is used to get the next valid ID for an account
    * @return id for an account
    */
  const QCString nextAccountID(void);

  /**
    * This method is used to get the next valid ID for a transaction
    * @return id for a transaction
    */
  const QCString nextTransactionID(void);

  /**
    * This method is used to get the next valid ID for a payee
    * @return id for a transaction
    */
  const QCString nextPayeeID(void);

  /**
    * This method re-parents an existing account
    *
    * An exception will be thrown upon error conditions.
    *
    * @param account MyMoneyAccount reference to account to be re-parented
    * @param parent  MyMoneyAccount reference to new parent account
    * @param sendNotification if true, notifications with the ids
    *                of all modified objects are send
    */
  void reparentAccount(MyMoneyAccount &account, MyMoneyAccount& parent, const bool sendNotification);
};

#endif
