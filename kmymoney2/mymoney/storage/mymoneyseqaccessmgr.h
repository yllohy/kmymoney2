/***************************************************************************
                          mymoneyseqaccessmgr.h  -  description
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
#include "mymoneymap.h"

/**
  * @author Thomas Baumgart
  */

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

  const bool operator == (const MyMoneyBalanceCacheItem& right) const;
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

  MyMoneySeqAccessMgr();
  ~MyMoneySeqAccessMgr();

  // general get functions
  const MyMoneyPayee user(void) const { return m_user; };
  const QDate creationDate(void) const { return m_creationDate; };
  const QDate lastModificationDate(void) const { return m_lastModificationDate; };
  const unsigned int currentFixVersion(void) const { return m_currentFixVersion; };
  const unsigned int fileFixVersion(void) const { return m_fileFixVersion; };


  // general set functions
  void setUser(const MyMoneyPayee& user) { m_user = user;
        touch(); };
  void setCreationDate(const QDate& val) { m_creationDate = val; touch(); };
  void setLastModificationDate(const QDate& val) { m_lastModificationDate = val; m_dirty = false; };
  void setFileFixVersion(const unsigned int v) { m_fileFixVersion = v; };
  /**
    * This method is used to get a SQL reader for subsequent database access
    */
  MyMoneyStorageSql *connectToDatabase (const KURL& url);
   /**
   * This method is used when a database file is open, and the data is to
   * be saved in a different file or format. It will ensure that all data
   * from the database is available in memory to enable it to be written.
   */
  virtual void fillStorage() {  };

  /**
    * This method is used to duplicate the MyMoneySeqAccessMgr object and return
    * a pointer to the newly created copy. The caller of this method is the
    * new owner of the object and must destroy it.
    */
  MyMoneySeqAccessMgr * const duplicate(void);

  /**
    * Returns the account addressed by it's id.
    *
    * @param id id of the account to locate.
    * @return reference to MyMoneyAccount object. An exception is thrown
    *         if the id is unknown
    */
  const MyMoneyAccount account(const QCString& id) const;

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
    *           @li STD_ACC_EQUITY
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
  void addAccount(MyMoneyAccount& account);

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
    * @return MyMoneyPayee reference to object of payee
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
    * @param skipCheck if @p true, skips the built in consistency check for
    *                  the object to be updated. Do not set this parameter
    *                  to true. This is only used for the MyMoneyFile::consistencyCheck()
    *                  procedure to be able to reload accounts. The default
    *                  setting of this parameter is @p false.
    */
  void modifyAccount(const MyMoneyAccount& account, const bool skipCheck = false);

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
  const MyMoneyTransaction transaction(const QCString& id) const;

  /**
    * This method is used to extract a transaction from the file global
    * transaction pool through an index into an account.
    *
    * @param account id of the account as QString
    * @param idx number of transaction in this account
    * @return reference to MyMoneyTransaction object
    */
  const MyMoneyTransaction transaction(const QCString& account, const int idx) const;

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
    * without it's sub-ordinate accounts. If a @p date is presented,
    * the balance at the beginning of this date (not including any
    * transaction on this date) is returned. Otherwise all recorded
    * transactions are included in the balance.
    *
    * @param id id of the account in question
    * @param date return balance for specific date
    * @return balance of the account as MyMoneyMoney object
    */
  const MyMoneyMoney balance(const QCString& id, const QDate& date = QDate()) const;

  /**
    * This method is used to return the actual balance of an account
    * including it's sub-ordinate accounts. If a @p date is presented,
    * the balance at the beginning of this date (not including any
    * transaction on this date) is returned. Otherwise all recorded
    * transactions are included in the balance.
    *
    * @param id id of the account in question
    * @param date return balance for specific date
    * @return balance of the account as MyMoneyMoney object
    */
  const MyMoneyMoney totalBalance(const QCString& id, const QDate& date = QDate()) const;

  /**
    * Returns the institution of a given ID
    *
    * @param id id of the institution to locate
    * @return MyMoneyInstitution object filled with data. If the institution
    *         could not be found, an exception will be thrown
    */
  const MyMoneyInstitution institution(const QCString& id) const;

  /**
    * This method returns an indicator if the storage object has been
    * changed after it has last been saved to permanent storage.
    *
    * @return true if changed, false if not
    */
  bool dirty(void) const { return m_dirty; }

  /**
    * This method can be used by an external object to force the
    * storage object to be dirty. This is used e.g. when an upload
    * to an external destination failed but the previous storage
    * to a local disk was ok.
    */
  void setDirty(void) { m_dirty = true; };

  /**
    * This method returns a list of the institutions
    * inside a MyMoneyFile object
    *
    * @return QMap containing the institution information
    */
  const QValueList<MyMoneyInstitution> institutionList(void) const;

  /**
    * This method returns a list of accounts inside the storage object.
    *
    * @param list reference to QValueList receiving the account objects
    *
    * @note The standard accounts will not be returned
    */
  void accountList(QValueList<MyMoneyAccount>& list) const;

  /**
    * This method is used to pull a list of transactions from the file
    * global transaction pool. It returns all those transactions
    * that match the filter passed as argument. If the filter is empty,
    * the whole journal will be returned.
    * The list returned is sorted according to the transactions posting date.
    * If more than one transaction exists for the same date, the order among
    * them is undefined.
    *
    * The @p list will be cleared by this method.
    *
    * @param list reference to list
    * @param filter MyMoneyTransactionFilter object with the match criteria
    *
    * @return set of transactions in form of a QValueList<MyMoneyTransaction>
    */
  void transactionList(QValueList<MyMoneyTransaction>& list, MyMoneyTransactionFilter& filter) const;

  /**
    * This method is used to pull a list of transactions from the file
    * global transaction pool. It returns all those transactions
    * that match the filter passed as argument. If the filter is empty,
    * the whole journal will be returned.
    * The list returned is sorted according to the transactions posting date.
    * If more than one transaction exists for the same date, the order among
    * them is undefined.
    *
    * The @p list will be cleared by this method.
    *
    * @param list reference to list
    * @param filter MyMoneyTransactionFilter object with the match criteria
    *
    * @return set of transactions in form of a QValueList<QPair<MyMoneyTransaction,MyMoneySplit> >
    */
  void transactionList(QValueList< QPair<MyMoneyTransaction, MyMoneySplit> >& list, MyMoneyTransactionFilter& filter) const;

  /**
    * Compatibility interface for the previous method.
    */
  const QValueList<MyMoneyTransaction> transactionList(MyMoneyTransactionFilter& filter) const;

  /**
    * This method returns whether a given transaction is already in memory, to avoid
    * reloading it from the database
    */
  bool isDuplicateTransaction(const QCString& id) const { return m_transactionKeys.contains(id); }

  /**
    * This method returns the number of transactions currently known to file
    * in the range 0..MAXUINT
    *
    * @param account QCString reference to account id. If account is empty
    +                all transactions (the journal) will be counted. If account
    *                is not empty it returns the number of transactions
    *                that have splits in this account.
    *
    * @return number of transactions in journal/account
    */
  const unsigned int transactionCount(const QCString& account = QCString()) const;

  const QMap<QCString, unsigned long> transactionCountMap(void) const;

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
    * This method is used to return the standard equity account
    * @return MyMoneyAccount equity account(group)
    */
  const MyMoneyAccount equity(void) const { return account(STD_ACC_EQUITY); };

  virtual void loadAccounts(const QMap<QCString, MyMoneyAccount>& acc);
  virtual void loadTransactions(const QMap<QCString, MyMoneyTransaction>& map);
  virtual void loadInstitutions(const QMap<QCString, MyMoneyInstitution>& map);
  virtual void loadPayees(const QMap<QCString, MyMoneyPayee>& map);
  virtual void loadSchedules(const QMap<QCString, MyMoneySchedule>& map);
  virtual void loadSecurities(const QMap<QCString, MyMoneySecurity>& map);
  virtual void loadCurrencies(const QMap<QCString, MyMoneySecurity>& map);

  virtual void loadAccountId(const unsigned long id);
  virtual void loadTransactionId(const unsigned long id);
  virtual void loadPayeeId(const unsigned long id);
  virtual void loadInstitutionId(const unsigned long id);
  virtual void loadScheduleId(const unsigned long id);
  virtual void loadSecurityId(const unsigned long id);
  virtual void loadReportId(const unsigned long id);
  virtual void loadBudgetId(const unsigned long id);

  virtual const unsigned long accountId(void) const { return m_nextAccountID; };
  virtual const unsigned long transactionId(void) const { return m_nextTransactionID; };
  virtual const unsigned long payeeId(void) const { return m_nextPayeeID; };
  virtual const unsigned long institutionId(void) const { return m_nextInstitutionID; };
  virtual const unsigned long scheduleId(void) const { return m_nextScheduleID; };
  virtual const unsigned long securityId(void) const { return m_nextSecurityID; };
  virtual const unsigned long reportId(void) const { return m_nextReportID; };
  virtual const unsigned long budgetId(void) const { return m_nextBudgetID; };


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
  const QMap<QCString, QString> pairs(void) const;

  // documented in IMyMoneySerialize base class
  void setPairs(const QMap<QCString, QString>& list);

  /**
    * This method is used to add a scheduled transaction to the engine.
    * It must be sure, that the id of the object is not filled. When the
    * method returns to the caller, the id will be filled with the
    * newly created object id value.
    *
    * An exception will be thrown upon erronous situations.
    *
    * @param sched reference to the MyMoneySchedule object
    */
  void addSchedule(MyMoneySchedule& sched);

  /**
    * This method is used to modify an existing MyMoneySchedule
    * object. Therefor, the id attribute of the object must be set.
    *
    * An exception will be thrown upon erronous situations.
    *
    * @param sched const reference to the MyMoneySchedule object to be updated
    */
  void modifySchedule(const MyMoneySchedule& sched);

  /**
    * This method is used to remove an existing MyMoneySchedule object
    * from the engine. The id attribute of the object must be set.
    *
    * An exception will be thrown upon erronous situations.
    *
    * @param sched const reference to the MyMoneySchedule object to be updated
    */
  void removeSchedule(const MyMoneySchedule& sched);

  /**
    * This method is used to retrieve a single MyMoneySchedule object.
    * The id of the object must be supplied in the parameter @p id.
    *
    * An exception will be thrown upon erronous situations.
    *
    * @param id QCString containing the id of the MyMoneySchedule object
    * @return MyMoneySchedule object
    */
  const MyMoneySchedule schedule(const QCString& id) const;

  /**
    * This method is used to create a new security object.  The ID will be created
    * automatically. The object passed with the parameter @p security is modified
    * to contain the assigned id.
    *
    * An exception will be thrown upon error conditions.
    *
    * @param security MyMoneySecurity filled with data
    */
  virtual void addSecurity(MyMoneySecurity& security);

  /**
    * This method is used to modify an existing MyMoneySchedule
    * object.
    *
    * An exception will be thrown upon erronous situations.
    *
    * @param security reference to the MyMoneySecurity object to be updated
    */
  void modifySecurity(const MyMoneySecurity& security);

  /**
    * This method is used to remove an existing MyMoneySecurity object
    * from the engine.
    *
    * An exception will be thrown upon erronous situations.
    *
    * @param security reference to the MyMoneySecurity object to be removed
    */
  void removeSecurity(const MyMoneySecurity& security);

  /**
    * This method is used to retrieve a single MyMoneySchedule object.
    * The id of the object must be supplied in the parameter @p id.
    *
    * An exception will be thrown upon erronous situations.
    *
    * @param id QCString containing the id of the MyMoneySchedule object
    * @return MyMoneySchedule object
    */
  const MyMoneySecurity security(const QCString& id) const;


  /**
    * This method returns a list of security objects that the engine has
    * knowledge of.
    */
  const QValueList<MyMoneySecurity> securityList(void) const;

  /**
    * This method is used to add a new currency object to the engine.
    * The ID of the object is the trading symbol, so there is no need for an additional
    * ID since the symbol is guaranteed to be unique.
    *
    * An exception will be thrown upon erronous situations.
    *
    * @param currency reference to the MyMoneyCurrency object
    */
  void addCurrency(const MyMoneySecurity& currency);

  /**
    * This method is used to modify an existing MyMoneyCurrency
    * object.
    *
    * An exception will be thrown upon erronous situations.
    *
    * @param currency reference to the MyMoneyCurrency object
    */
  void modifyCurrency(const MyMoneySecurity& currency);

  /**
    * This method is used to remove an existing MyMoneyCurrency object
    * from the engine.
    *
    * An exception will be thrown upon erronous situations.
    *
    * @param currency reference to the MyMoneyCurrency object
    */
  void removeCurrency(const MyMoneySecurity& currency);

  /**
    * This method is used to retrieve a single MyMoneySchedule object.
    * The id of the object must be supplied in the parameter @p id.
    *
    * An exception will be thrown upon erronous situations.
    *
    * @param id QCString containing the id of the MyMoneySchedule object
    * @return MyMoneySchedule object
    */
  const MyMoneySecurity currency(const QCString& id) const;

  /**
    * This method is used to retrieve the list of all currencies
    * known to the engine.
    *
    * An exception will be thrown upon erronous situations.
    *
    * @return QValueList of all MyMoneyCurrency objects.
    */
  const QValueList<MyMoneySecurity> currencyList(void) const;

  /**
    * This method is used to extract a list of scheduled transactions
    * according to the filter criteria passed as arguments.
    *
    * @param accountId only search for scheduled transactions that reference
    *                  accound @p accountId. If accountId is the empty string,
    *                  this filter is off. Default is @p QCString().
    * @param type      only schedules of type @p type are searched for.
    *                  See MyMoneySchedule::typeE for details.
    *                  Default is MyMoneySchedule::TYPE_ANY
    * @param occurence only schedules of occurence type @p occurence are searched for.
    *                  See MyMoneySchedule::occurenceE for details.
    *                  Default is MyMoneySchedule::OCCUR_ANY
    * @param paymentType only schedules of payment method @p paymentType
    *                  are searched for.
    *                  See MyMoneySchedule::paymentTypeE for details.
    *                  Default is MyMoneySchedule::STYPE_ANY
    * @param startDate only schedules with payment dates after @p startDate
    *                  are searched for. Default is all dates (QDate()).
    * @param endDate   only schedules with payment dates ending prior to @p endDate
    *                  are searched for. Default is all dates (QDate()).
    * @param overdue   if true, only those schedules that are overdue are
    *                  searched for. Default is false (all schedules will be returned).
    *
    * @return const QValueList<MyMoneySchedule> list of schedule objects.
    */
  const QValueList<MyMoneySchedule> scheduleList(const QCString& accountId = QCString(),
                                     const MyMoneySchedule::typeE type = MyMoneySchedule::TYPE_ANY,
                                     const MyMoneySchedule::occurenceE occurence = MyMoneySchedule::OCCUR_ANY,
                                     const MyMoneySchedule::paymentTypeE paymentType = MyMoneySchedule::STYPE_ANY,
                                     const QDate& startDate = QDate(),
                                     const QDate& endDate = QDate(),
                                     const bool overdue = false) const;

  const QValueList<MyMoneySchedule> scheduleListEx( int scheduleTypes,
                                              int scheduleOcurrences,
                                              int schedulePaymentTypes,
                                              QDate startDate,
                                              const QCStringList& accounts=QCStringList()) const;

  /**
    * This method is used to retrieve the list of all reports
    * known to the engine.
    *
    * An exception will be thrown upon erronous situations.
    *
    * @return QValueList of all MyMoneyReport objects.
    */
  const QValueList<MyMoneyReport> reportList( void ) const;

  /**
    * This method is used to add a new report to the engine.
    * It must be sure, that the id of the object is not filled. When the
    * method returns to the caller, the id will be filled with the
    * newly created object id value.
    *
    * An exception will be thrown upon erronous situations.
    *
    * @param report reference to the MyMoneyReport object
    */
  void addReport( MyMoneyReport& report );

  /**
    * This method is used to load a set of reports into the engine.  This is
    * used when loading from storage, and an ID is already known.
    *
    * An exception will be thrown upon erronous situations.
    *
    * @param reports reference to the map of MyMoneyReport objects
    */
  void loadReports( const QMap<QCString, MyMoneyReport>& reports );

  /**
    * This method is used to modify an existing MyMoneyReport
    * object. Therefor, the id attribute of the object must be set.
    *
    * An exception will be thrown upon erronous situations.
    *
    * @param report const reference to the MyMoneyReport object to be updated
    */
  void modifyReport( const MyMoneyReport& report );

  /**
    * This method returns the number of reports currently known to file
    * in the range 0..MAXUINT
    *
    * @return number of reports known to file
    */
  unsigned countReports(void) const;

  /**
    * This method is used to retrieve a single MyMoneyReport object.
    * The id of the object must be supplied in the parameter @p id.
    *
    * An exception will be thrown upon erronous situations.
    *
    * @param id QCString containing the id of the MyMoneyReport object
    * @return MyMoneyReport object
    */
  const MyMoneyReport report( const QCString& id ) const;

  /**
    * This method is used to remove an existing MyMoneyReport object
    * from the engine. The id attribute of the object must be set.
    *
    * An exception will be thrown upon erronous situations.
    *
    * @param report const reference to the MyMoneyReport object to be updated
    */
  void removeReport(const MyMoneyReport& report);

  /**
    * This method is used to retrieve the list of all budgets
    * known to the engine.
    *
    * An exception will be thrown upon erronous situations.
    *
    * @return QValueList of all MyMoneyBudget objects.
    */
  const QValueList<MyMoneyBudget> budgetList( void ) const;

  /**
    * This method is used to add a new budget to the engine.
    * It must be sure, that the id of the object is not filled. When the
    * method returns to the caller, the id will be filled with the
    * newly created object id value.
    *
    * An exception will be thrown upon erronous situations.
    *
    * @param budget reference to the MyMoneyBudget object
    */
  void addBudget( MyMoneyBudget& budget );

  /**
    * This method is used to load a set of budgets into the engine.  This is
    * used when loading from storage, and an ID is already known.
    *
    * An exception will be thrown upon erronous situations.
    *
    * @param budgets reference to the map of MyMoneyBudget object
    */
  void loadBudgets(const QMap<QCString, MyMoneyBudget>& budgets);

  /**
    * This method is used to retrieve the id to a corresponding
    * name of a budget
    * An exception will be thrown upon error conditions.
    *
    * @param budget QString reference to name of budget
    *
    * @return MyMoneyBudget reference to object of budget
    */
  const MyMoneyBudget budgetByName(const QString& budget) const;

  /**
    * This method is used to modify an existing MyMoneyBudget
    * object. Therefore, the id attribute of the object must be set.
    *
    * An exception will be thrown upon erronous situations.
    *
    * @param budget const reference to the MyMoneyBudget object to be updated
    */
  void modifyBudget( const MyMoneyBudget& budget );

  /**
    * This method returns the number of budgets currently known to file
    * in the range 0..MAXUINT
    *
    * @return number of budgets known to file
    */
  unsigned countBudgets(void) const;

  /**
    * This method is used to retrieve a single MyMoneyBudget object.
    * The id of the object must be supplied in the parameter @p id.
    *
    * An exception will be thrown upon erronous situations.
    *
    * @param id QCString containing the id of the MyMoneyBudget object
    * @return MyMoneyBudget object
    */
  MyMoneyBudget budget( const QCString& id ) const;

  /**
    * This method is used to remove an existing MyMoneyBudget object
    * from the engine. The id attribute of the object must be set.
    *
    * An exception will be thrown upon erronous situations.
    *
    * @param budget const reference to the MyMoneyBudget object to be updated
    */
  void removeBudget(const MyMoneyBudget& budget);


  /**
    * This method adds/replaces a price to/from the price list
    */
  void addPrice(const MyMoneyPrice& price);

  /**
    * This method removes a price from the price list
    */
  void removePrice(const MyMoneyPrice& price);

  /**
    * This method retrieves a price from the price list.
    * If @p date is inValid, QDate::currentDate() is assumed.
    */
  const MyMoneyPrice price(const QCString& fromId, const QCString& toId, const QDate& date, const bool exactDate) const;

  /**
    * This method returns a list of all price entries.
    */
  const MyMoneyPriceList priceList(void) const;

  /**
    * Clear all internal caches (used internally for performance measurements)
    */
  void clearCache(void);

  /**
    * This method checks, if the given @p object is referenced
    * by another engine object.
    *
    * @param obj const reference to object to be checked
    * @param skipCheck QBitArray with ReferenceCheckBits set for which
    *                  the check should be skipped
    *
    * @retval false @p object is not referenced
    * @retval true @p institution is referenced
    */
  bool isReferenced(const MyMoneyObject& obj, const MyMoneyFileBitArray& skipCheck = MyMoneyFileBitArray()) const;

  /**
    * This method recalculates the balances of all accounts
    * based on the transactions stored in the engine.
    */
  void rebuildAccountBalances(void);

  virtual void startTransaction(void);
  virtual void commitTransaction(void);
  virtual void rollbackTransaction(void);

protected:
  void removeReferences(const QCString& id);

private:

  static const int INSTITUTION_ID_SIZE = 6;
  static const int ACCOUNT_ID_SIZE = 6;
  static const int TRANSACTION_ID_SIZE = 18;
  static const int PAYEE_ID_SIZE = 6;
  static const int SCHEDULE_ID_SIZE = 6;
  static const int SECURITY_ID_SIZE = 6;
  static const int REPORT_ID_SIZE = 6;
  static const int BUDGET_ID_SIZE = 6;

  /**
    * This method is used to set the dirty flag and update the
    * date of the last modification.
    */
  void touch(void);

  /**
    * This method is used to invalidate the cached balance for
    * the selected account and all it's parents.
    *
    * @param id id of the account in question
    */
  void invalidateBalanceCache(const QCString& id);

  /**
    * This member variable keeps the User information.
    * @see setUser()
    */
  MyMoneyPayee m_user;

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
    * The member variable m_nextScheduleID keeps the number that will be
    * assigned to the next schedule created. It is maintained by
    * nextScheduleID()
    */
  unsigned long m_nextScheduleID;

  /**
    * The member variable m_nextSecurityID keeps the number that will be
    * assigned to the next security object created.  It is maintained by
    * nextSecurityID()
    */
  unsigned long m_nextSecurityID;

  unsigned long m_nextReportID;

  /**
    * The member variable m_nextBudgetID keeps the number that will be
    * assigned to the next budget object created.  It is maintained by
    * nextBudgetID()
    */
  unsigned long m_nextBudgetID;

  /**
    * The member variable m_institutionList is the container for the
    * institutions known within this file.
    */
  MyMoneyMap<QCString, MyMoneyInstitution> m_institutionList;

  /**
    * The member variable m_accountList is the container for the accounts
    * known within this file.
    */
  MyMoneyMap<QCString, MyMoneyAccount> m_accountList;

  /**
    * The member variable m_balanceCache is the container for the
    * accounts actual balance
    */
  mutable QMap<QCString, MyMoneyBalanceCacheItem> m_balanceCache;

  /**
    * This member keeps the date for which the m_balanceCache member
    * is valid. In case the whole cache is invalid it is set to
    * QDate().
    */
  mutable QDate          m_balanceCacheDate;

  /**
    * The member variable m_transactionList is the container for all
    * transactions within this file.
    * @see m_transactionKeys
    */
  MyMoneyMap<QCString, MyMoneyTransaction> m_transactionList;

  /**
    * The member variable m_transactionKeys is used to convert
    * transaction id's into the corresponding key used in m_transactionList.
    * @see m_transactionList;
    */
  MyMoneyMap<QCString, QCString> m_transactionKeys;

  /**
    * A list containing all the payees that have been used
    */
  MyMoneyMap<QCString, MyMoneyPayee> m_payeeList;

  /**
    * A list containing all the scheduled transactions
    */
  MyMoneyMap<QCString, MyMoneySchedule> m_scheduleList;

  /**
    * A list containing all the security information objects.  Each object
    * can represent a stock, bond, or mutual fund.  It contains a price
    * history that a user can add entries to.  The price history will be used
    * to determine the cost basis for sales, as well as the source of
    * information for reports in a security account.
    */
  MyMoneyMap<QCString, MyMoneySecurity> m_securitiesList;

  /**
    * A list containing all the currency information objects.
    */
  MyMoneyMap<QCString, MyMoneySecurity> m_currencyList;

  MyMoneyMap<QCString, MyMoneyReport> m_reportList;

  /**
    * A list containing all the budget information objects.
    */
  MyMoneyMap<QCString, MyMoneyBudget> m_budgetList;

  MyMoneyPriceList              m_priceList;

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
    * This member variable contains the current fix level of application
    * data files. (see kmymoneyview.cpp)
    */
  unsigned int m_currentFixVersion;
  /**
   * This member variable contains the current fix level of the
   *  presently open data file. (see kmymoneyview.cpp)
   */
  unsigned int m_fileFixVersion;
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
    * @return id for a payee
    */
  const QCString nextPayeeID(void);

  /**
    * This method is used to get the next valid ID for a scheduled transaction
    * @return id for a scheduled transaction
    */
  const QCString nextScheduleID(void);

  /**
    * This method is used to get the next valid ID for an security object.
    * @return id for an security object
    */
  const QCString nextSecurityID(void);

  const QCString nextReportID(void);

  /**
    * This method is used to get the next valid ID for a budget object.
    * @return id for an budget object
    */
  const QCString nextBudgetID(void);


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
  /**
   * This method will close a database and log the use roff
   */
  void close(void) { };

  /**
    * This member variable is set when all transactions have been read from the database.
    * This is would be probably the case when doing, for e.g., a full report,
    * or after some types of transaction search which cannot be easily implemented in SQL
    */
  bool m_transactionListFull;
};
#endif
