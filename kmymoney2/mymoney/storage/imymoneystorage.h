/***************************************************************************
                          imymoneystorage.h  -  description
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

#ifndef IMYMONEYSTORAGE_H
#define IMYMONEYSTORAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qstring.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "../mymoneyutils.h"
#include "../mymoneyinstitution.h"
#include "../mymoneyaccount.h"
#include "../mymoneytransaction.h"
#include "../mymoneypayee.h"
#include "../mymoneyscheduled.h"
#include "../mymoneyobserver.h"
#include "../mymoneytransactionfilter.h"
#include "../mymoneyequity.h"
#include "../mymoneycurrency.h"

/**
  * @author Thomas Baumgart
  */

/**
  * The IMyMoneyStorage class describes the interface between the MyMoneyFile class
  * and the real storage manager.
  *
  * @see MyMoneySeqAccessMgr
  */
class IMyMoneyStorage {
public: 
  IMyMoneyStorage();
  virtual ~IMyMoneyStorage();

  // general get functions
  virtual const QString& userName(void) const = 0;
  virtual const QString& userStreet(void) const = 0;
  virtual const QString& userTown(void) const = 0;
  virtual const QString& userCounty(void) const = 0;
  virtual const QString& userPostcode(void) const = 0;
  virtual const QString& userTelephone(void) const = 0;
  virtual const QString& userEmail(void) const = 0;
  virtual const QDate& creationDate(void) const = 0;
  virtual const QDate& lastModificationDate(void) const = 0;

  // general set functions
  virtual void setUserName(const QString& val) = 0;
  virtual void setUserStreet(const QString& val) = 0;
  virtual void setUserTown(const QString& val) = 0;
  virtual void setUserCounty(const QString& val) = 0;
  virtual void setUserPostcode(const QString& val) = 0;
  virtual void setUserTelephone(const QString& val) = 0;
  virtual void setUserEmail(const QString& val) = 0;

  // methods provided by MyMoneyKeyValueContainer
  virtual void setValue(const QCString& key, const QString& key) = 0;
  virtual const QString value(const QCString& key) const = 0;
  virtual void deletePair(const QCString& key) = 0;

  /**
    * This method is used to duplicate an IMyMoneyStorage object and return
    * a pointer to the newly created copy. The caller of this method is the
    * new owner of the object and must destroy it.
    */
  virtual IMyMoneyStorage* const duplicate(void) = 0;

  /**
    * This method is used to create a new account
    *
    * An exception will be thrown upon error conditions.
    *
    * @param account MyMoneyAccount filled with data
    */
  virtual void newAccount(MyMoneyAccount& account) = 0;

  /**
    * This method is used to add one account as sub-ordinate to another
    * (parent) account. The objects that are passed will be modified
    * accordingly.
    *
    * An exception will be thrown upon error conditions.
    *
    * @param parent parent account the account should be added to
    * @param account the account to be added
    */
  virtual void addAccount(MyMoneyAccount& parent, MyMoneyAccount& account) = 0;

  /**
    * This method is used to create a new payee
    *
    * An exception will be thrown upon error conditions
    *
    * @param payee MyMoneyPayee reference to payee information
    */
  virtual void addPayee(MyMoneyPayee& payee) = 0;

  /**
    * This method is used to retrieve information about a payee
    * An exception will be thrown upon error conditions.
    *
    * @param id QCString reference to id of payee
    *
    * @return MyMoneyPayee object of payee
    */
  virtual const MyMoneyPayee payee(const QCString& id) const = 0;

  /**
    * This method is used to retrieve the id to a corresponding
    * name of a payee/receiver.
    * An exception will be thrown upon error conditions.
    *
    * @param payee QString reference to name of payee
    *
    * @return MyMoneyPayee object of payee
    */
  virtual const MyMoneyPayee payeeByName(const QString& payee) const = 0;

  /**
    * This method is used to modify an existing payee
    *
    * An exception will be thrown upon error conditions
    *
    * @param payee MyMoneyPayee reference to payee information
    */
  virtual void modifyPayee(const MyMoneyPayee& payee) = 0;

  /**
    * This method is used to remove an existing payee
    *
    * An exception will be thrown upon error conditions
    *
    * @param payee MyMoneyPayee reference to payee information
    */
  virtual void removePayee(const MyMoneyPayee& payee) = 0;

  /**
    * This method returns a list of the payees
    * inside a MyMoneyStorage object
    *
    * @return QValueList<MyMoneyPayee> containing the payee information
    */
  virtual const QValueList<MyMoneyPayee> payeeList(void) const = 0;

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
  virtual void addAccount(MyMoneyInstitution& institution, MyMoneyAccount& account) = 0;

  /**
    * Returns the account addressed by it's id.
    *
    * An exception will be thrown upon error conditions.
    *
    * @param id id of the account to locate.
    * @return reference to MyMoneyAccount object. An exception is thrown
    *         if the id is unknown
    */
  virtual const MyMoneyAccount& account(const QCString id) const = 0;

  /**
    * This method is used to check whether a given
    * account id references one of the standard accounts or not.
    *
    * An exception will be thrown upon error conditions.
    *
    * @param id account id
    * @return true if account-id is one of the standards, false otherwise
    */
  virtual const bool isStandardAccount(const QCString& id) const = 0;

  /**
    * This method is used to set the name for the specified standard account
    * within the storage area. An exception will be thrown, if an error
    * occurs
    *
    * @param id QCString reference to one of the standard accounts.
    * @param name QString reference to the name to be set
    *
    */
  virtual void setAccountName(const QCString& id, const QString& name) = 0;

  /**
    * Adds an institution to the storage. A
    * respective institution-ID will be generated within this record.
    * The ID is stored as QString in the object passed as argument.
    *
    * An exception will be thrown upon error conditions.
    *
    * @param institution The complete institution information in a
    *        MyMoneyInstitution object
    */
  virtual void addInstitution(MyMoneyInstitution& institution) = 0;

  /**
    * Adds a transaction to the file-global transaction pool. A respective
    * transaction-ID will be generated within this record. The ID is stored
    * QString with the object.
    *
    * An exception will be thrown upon error conditions.
    *
    * @param transaction reference to the transaction
    * @param skipAccountUpdate if set, the transaction lists of the accounts
    *        referenced in the splits are not updated. This is used for
    *        bulk loading a lot of transactions but not during normal operation
    */
  virtual void addTransaction(MyMoneyTransaction& transaction, const bool skipAccountUpdate = false) = 0;

  /**
    * This method is used to determince, if the account with the
    * given ID is referenced by any split in m_transactionList.
    *
    * An exception will be thrown upon error conditions.
    *
    * @param id id of the account to be checked for
    * @return true if account is referenced, false otherwise
    */
  virtual const bool hasActiveSplits(const QCString& id) const = 0;

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
  virtual const MyMoneyMoney balance(const QCString& id, const QDate& date)= 0;

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
  virtual const MyMoneyMoney totalBalance(const QCString& id, const QDate& date) = 0;

  /**
    * Returns the institution of a given ID
    *
    * @param id id of the institution to locate
    * @return MyMoneyInstitution object filled with data. If the institution
    *         could not be found, an exception will be thrown
    */
  virtual const MyMoneyInstitution& institution(const QCString& id) const = 0;

  /**
    * This method returns an indicator if the storage object has been
    * changed after it has last been saved to permanent storage.
    *
    * @return true if changed, false if not
    */
  virtual bool dirty(void) const = 0;

  /**
    * This method can be used by an external object to force the
    * storage object to be dirty. This is used e.g. when an upload
    * to an external destination failed but the previous storage
    * to a local disk was ok.
    */
  virtual void setDirty(void) = 0;
  
  /**
    * This method returns the number of accounts currently known to this storage
    * in the range 0..MAXUINT
    *
    * @return number of accounts currently known inside a MyMoneyFile object
    */
  virtual const unsigned int accountCount(void) const = 0;

  /**
    * This method returns a list of the institutions
    * inside a MyMoneyStorage object
    *
    * @return QValueList<MyMoneyInstitution> containing the
    *         institution information
    */
  virtual const QValueList<MyMoneyInstitution> institutionList(void) const = 0;

  /**
    * Modifies an already existing account in the file global account pool.
    *
    * An exception will be thrown upon error conditions.
    *
    * @param account reference to the new account information
    * @param skipCheck allows to skip the builtin consistency checks
    */
  virtual void modifyAccount(const MyMoneyAccount& account, const bool skipCheck = false) = 0;

  /**
    * Modifies an already existing institution in the file global
    * institution pool.
    *
    * An exception will be thrown upon error conditions.
    *
    * @param institution The complete new institution information
    */
  virtual void modifyInstitution(const MyMoneyInstitution& institution) = 0;

  /**
    * This method is used to update a specific transaction in the
    * transaction pool of the MyMoneyFile object
    *
    * An exception will be thrown upon error conditions.
    *
    * @param transaction reference to transaction to be changed
    */
  virtual void modifyTransaction(const MyMoneyTransaction& transaction) = 0;

  /**
    * This method re-parents an existing account
    *
    * An exception will be thrown upon error conditions.
    *
    * @param account MyMoneyAccount reference to account to be re-parented
    * @param parent  MyMoneyAccount reference to new parent account
    */
  virtual void reparentAccount(MyMoneyAccount &account, MyMoneyAccount& parent) = 0;

  /**
    * This method is used to remove a transaction from the transaction
    * pool (journal).
    *
    * An exception will be thrown upon error conditions.
    *
    * @param transaction const reference to transaction to be deleted
    */
  virtual void removeTransaction(const MyMoneyTransaction& transaction) = 0;

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
  virtual const unsigned int transactionCount(const QCString& account = QCString()) const = 0;

  /**
    * This method returns a QMap filled with the number of transactions
    * per account. The account id serves as index into the map. If one
    * needs to have all transactionCounts() for many accounts, this method
    * is faster than calling transactionCount(const QCString& account) many
    * times.
    *
    * @return QMap with numbers of transactions per account
    */
  virtual const QMap<QCString, unsigned long> transactionCountMap(void) const = 0;

  /**
    * This method is used to pull a list of transactions from the file
    * global transaction pool. It returns all those transactions
    * that match the filter passed as argument. If the filter is empty,
    * the whole journal will be returned.
    *
    * @param filter MyMoneyTransactionFilter object with the match criteria
    *
    * @return set of transactions in form of a QValueList<MyMoneyTransaction>
    */
  virtual const QValueList<MyMoneyTransaction> transactionList(MyMoneyTransactionFilter& filter) const = 0;

  /**
    * This method is used to pull a list of transactions from the file
    * global transaction pool. It returns either the whole journal or
    * the set of transaction referenced by a specific account depending
    * on the argument given.
    *
    * @param account QCString reference to account id. If account is empty all
    +                transactions (the journal) is returned. If account
    *                is not empty it returns the set of transactions
    *                that have splits in this account.
    *
    * @return set of transactions in form of a QValueList<MyMoneyTransaction>
    */
  // virtual const QValueList<MyMoneyTransaction> transactionList(const QCString& account = "") const = 0;

  /**
    * Deletes an existing account from the file global account pool
    * This method only allows to remove accounts that are not
    * referenced by any split. Use moveSplits() to move splits
    * to another account. An exception is thrown in case of a
    * problem.
    *
    * @param account reference to the account to be deleted.
    */
  virtual void removeAccount(const MyMoneyAccount& account) = 0;

  /**
    * Deletes an existing institution from the file global institution pool
    * Also modifies the accounts that reference this institution as
    * their institution.
    *
    * An exception will be thrown upon error conditions.
    *
    * @param institution institution to be deleted.
    */
  virtual void removeInstitution(const MyMoneyInstitution& institution) = 0;

  /**
    * This method is used to extract a transaction from the file global
    * transaction pool through an id. In case of an invalid id, an
    * exception will be thrown.
    *
    * @param id id of transaction as QString.
    * @return reference to the requested transaction
    */
  virtual const MyMoneyTransaction& transaction(const QCString& id) const = 0;

  /**
    * This method is used to extract a transaction from the file global
    * transaction pool through an index into an account.
    *
    * @param account id of the account as QString
    * @param idx number of transaction in this account
    * @return reference to MyMoneyTransaction object
    */
  virtual const MyMoneyTransaction& transaction(const QCString& account, const int idx) const = 0;

  /**
    * This method returns the number of institutions currently known to file
    * in the range 0..MAXUINT
    *
    * @return number of institutions known to file
    */
  virtual const unsigned int institutionCount(void) const = 0;

  /**
    * This method returns a list of the accounts
    * inside a MyMoneyStorage object
    *
    * @return QValueList<MyMoneyAccount> with accounts
    */
  virtual const QValueList<MyMoneyAccount> accountList(void) const = 0;

  /**
    * This method is used to return the standard liability account
    * @return MyMoneyAccount liability account(group)
    */
  virtual const MyMoneyAccount liability(void) const = 0;

  /**
    * This method is used to return the standard asset account
    * @return MyMoneyAccount asset account(group)
    */
  virtual const MyMoneyAccount asset(void) const = 0;

  /**
    * This method is used to return the standard expense account
    * @return MyMoneyAccount expense account(group)
    */
  virtual const MyMoneyAccount expense(void) const = 0;

  /**
    * This method is used to return the standard income account
    * @return MyMoneyAccount income account(group)
    */
  virtual const MyMoneyAccount income(void) const = 0;

  /**
    * This method is used to add a new equity object to the engine.
    * The ID of the object is the trading symbol, so there is no need for an additional
    * ID since the symbol is guaranteed to be unique.
    *
    * An exception will be thrown upon erronous situations.
    *
    * @param equity reference to the MyMoneyEquity object
    */
  virtual void addEquity(MyMoneyEquity& equity) = 0;

  /**
    * This method is used to modify an existing MyMoneySchedule
    * object.
    *
    * An exception will be thrown upon erronous situations.
    *
    * @param equity reference to the MyMoneyEquity object to be updated
    */
  virtual void modifyEquity(const MyMoneyEquity& equity) = 0;

  /**
    * This method is used to remove an existing MyMoneyEquity object
    * from the engine.
    *
    * An exception will be thrown upon erronous situations.
    *
    * @param equity reference to the MyMoneyEquity object to be removed
    */
  virtual void removeEquity(const MyMoneyEquity& equity) = 0;

  /**
    * This method is used to retrieve a single MyMoneyEquity object.
    * The id of the object must be supplied in the parameter @p id.
    *
    * An exception will be thrown upon erronous situations.
    *
    * @param id QCString containing the id of the MyMoneyEquity object
    * @return MyMoneyEquity object
    */
  virtual const MyMoneyEquity equity(const QCString& id) const = 0;

  /**
    * This method returns a list of the equity objects
    * inside a MyMoneyStorage object
    *
    * @return QValueList<MyMoneyEquity> containing objects
    */
  virtual const QValueList<MyMoneyEquity> equityList(void) const = 0;

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
  virtual void addSchedule(MyMoneySchedule& sched) = 0;

  /**
    * This method is used to modify an existing MyMoneySchedule
    * object. Therefor, the id attribute of the object must be set.
    *
    * An exception will be thrown upon erronous situations.
    *
    * @param sched const reference to the MyMoneySchedule object to be updated
    */
  virtual void modifySchedule(const MyMoneySchedule& sched) = 0;

  /**
    * This method is used to remove an existing MyMoneySchedule object
    * from the engine. The id attribute of the object must be set.
    *
    * An exception will be thrown upon erronous situations.
    *
    * @param sched const reference to the MyMoneySchedule object to be updated
    */
  virtual void removeSchedule(const MyMoneySchedule& sched) = 0;

  /**
    * This method is used to retrieve a single MyMoneySchedule object.
    * The id of the object must be supplied in the parameter @p id.
    *
    * An exception will be thrown upon erronous situations.
    *
    * @param id QCString containing the id of the MyMoneySchedule object
    * @return MyMoneySchedule object
    */
  virtual const MyMoneySchedule schedule(const QCString& id) const = 0;

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
    * @param occurence only schedules of occurence type @p occurance are searched for.
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
  virtual const QValueList<MyMoneySchedule> scheduleList(const QCString& accountId = QCString(),
                                     const MyMoneySchedule::typeE type = MyMoneySchedule::TYPE_ANY,
                                     const MyMoneySchedule::occurenceE occurence = MyMoneySchedule::OCCUR_ANY,
                                     const MyMoneySchedule::paymentTypeE paymentType = MyMoneySchedule::STYPE_ANY,
                                     const QDate& startDate = QDate(),
                                     const QDate& endDate = QDate(),
                                     const bool overdue = false) const = 0;

  virtual QValueList<MyMoneySchedule> scheduleListEx( int scheduleTypes,
                                              int scheduleOcurrences,
                                              int schedulePaymentTypes,
                                              QDate startDate,
                                              const QCStringList& accounts=QCStringList()) const = 0;

  /**
    * This method is used to add a new currency object to the engine.
    * The ID of the object is the trading symbol, so there is no need for an additional
    * ID since the symbol is guaranteed to be unique.
    *
    * An exception will be thrown upon erronous situations.
    *
    * @param currency reference to the MyMoneyCurrency object
    */
  virtual void addCurrency(const MyMoneyCurrency& currency) = 0;

  /**
    * This method is used to modify an existing MyMoneyCurrency
    * object.
    *
    * An exception will be thrown upon erronous situations.
    *
    * @param currency reference to the MyMoneyCurrency object
    */
  virtual void modifyCurrency(const MyMoneyCurrency& currency) = 0;

  /**
    * This method is used to remove an existing MyMoneyCurrency object
    * from the engine.
    *
    * An exception will be thrown upon erronous situations.
    *
    * @param currency reference to the MyMoneyCurrency object
    */
  virtual void removeCurrency(const MyMoneyCurrency& currency) = 0;

  /**
    * This method is used to retrieve a single MyMoneySchedule object.
    * The id of the object must be supplied in the parameter @p id.
    *
    * An exception will be thrown upon erronous situations.
    *
    * @param id QCString containing the id of the MyMoneySchedule object
    * @return MyMoneySchedule object
    */
  virtual const MyMoneyCurrency currency(const QCString& id) const = 0;

  /**
    * This method is used to retrieve the list of all currencies
    * known to the engine.
    *
    * An exception will be thrown upon erronous situations.
    *
    * @return QValueList of all MyMoneyCurrency objects.
    */
  virtual const QValueList<MyMoneyCurrency> currencyList(void) const = 0;
};

#endif
