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
#include <qbitarray.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/mymoneyutils.h>
#include <kmymoney/mymoneyinstitution.h>
#include <kmymoney/mymoneyaccount.h>
#include <kmymoney/mymoneytransaction.h>
#include <kmymoney/mymoneypayee.h>
#include <kmymoney/mymoneyscheduled.h>
#include <kmymoney/mymoneyobserver.h>
#include <kmymoney/mymoneytransactionfilter.h>
#include <kmymoney/mymoneysecurity.h>
#include <kmymoney/mymoneyprice.h>
#include <kmymoney/mymoneyreport.h>
#include <kmymoney/mymoneybudget.h>

/**
  * @author Thomas Baumgart
  *
  * A simple replacement for QBitArray that does not bark if testBit()
  * is called with an index out of bounds. It silently returns false
  * in that case, otherwise calls the base classes implementation.
  */
class MyMoneyFileBitArray : public QBitArray
{
public:
  MyMoneyFileBitArray() : QBitArray() {}
  MyMoneyFileBitArray(int size) : QBitArray(size) {}
  bool testBit(uint index) const;
  bool operator[](int index) const { return testBit(index); }
  bool at(uint index) const { return testBit(index); }
};


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

  typedef enum {
    RefCheckAccount = 0,
    RefCheckInstitution,
    RefCheckPayee,
    RefCheckTransaction,
    RefCheckReport,
    RefCheckBudget,
    RefCheckSchedule,
    RefCheckSecurity,
    RefCheckCurrency,
    RefCheckPrice,
    // insert new entries above this line
    MaxRefCheckBits
  } ReferenceCheckBits;

  IMyMoneyStorage();
  virtual ~IMyMoneyStorage();

  // general get functions
  virtual const MyMoneyPayee& user(void) const = 0;
  virtual const QDate& creationDate(void) const = 0;
  virtual const QDate& lastModificationDate(void) const = 0;
  virtual const unsigned int currentFixVersion(void) const = 0;
  virtual const unsigned int fileFixVersion(void) const = 0;

  // general set functions
  virtual void setUser(const MyMoneyPayee& user) = 0;
  virtual void setFileFixVersion(const unsigned int v) = 0;

  // methods provided by MyMoneyKeyValueContainer
  virtual void setValue(const QCString& key, const QString& value) = 0;
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
  virtual void addAccount(MyMoneyAccount& account) = 0;

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
  virtual const MyMoneyPayee& payee(const QCString& id) const = 0;

  /**
    * This method is used to retrieve the id to a corresponding
    * name of a payee/receiver.
    * An exception will be thrown upon error conditions.
    *
    * @param payee QString reference to name of payee
    *
    * @return MyMoneyPayee object of payee
    */
  virtual const MyMoneyPayee& payeeByName(const QString& payee) const = 0;

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
    * Returns the account addressed by it's id.
    *
    * An exception will be thrown upon error conditions.
    *
    * @param id id of the account to locate.
    * @return reference to MyMoneyAccount object. An exception is thrown
    *         if the id is unknown
    */
  virtual const MyMoneyAccount& account(const QCString& id) const = 0;

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
    * The list returned is sorted according to the transactions posting date.
    * If more than one transaction exists for the same date, the order among
    * them is undefined.
    *
    * @param filter MyMoneyTransactionFilter object with the match criteria
    *
    * @return set of transactions in form of a QValueList<MyMoneyTransaction>
    */
  virtual QValueList<MyMoneyTransaction> transactionList(MyMoneyTransactionFilter& filter) const = 0;

  virtual void transactionList(QValueList<MyMoneyTransaction>& list, MyMoneyTransactionFilter& filter) const = 0;

  virtual void transactionList(QValueList<QPair<MyMoneyTransaction, MyMoneySplit> >& list, MyMoneyTransactionFilter& filter) const = 0;

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
    * This method returns a list of accounts inside the storage object.
    *
    * @param list reference to QValueList receiving the account objects
    *
    * @note The standard accounts will not be returned
    */
  virtual void accountList(QValueList<MyMoneyAccount>& list) = 0;

  /**
    * This method is used to return the standard liability account
    * @return MyMoneyAccount liability account(group)
    */
  virtual const MyMoneyAccount& liability(void) const = 0;

  /**
    * This method is used to return the standard asset account
    * @return MyMoneyAccount asset account(group)
    */
  virtual const MyMoneyAccount& asset(void) const = 0;

  /**
    * This method is used to return the standard expense account
    * @return MyMoneyAccount expense account(group)
    */
  virtual const MyMoneyAccount& expense(void) const = 0;

  /**
    * This method is used to return the standard income account
    * @return MyMoneyAccount income account(group)
    */
  virtual const MyMoneyAccount& income(void) const = 0;

  /**
    * This method is used to return the standard equity account
    * @return MyMoneyAccount equity account(group)
    */
  virtual const MyMoneyAccount& equity(void) const = 0;

  /**
    * This method is used to create a new security object.  The ID will be created
    * automatically. The object passed with the parameter @p security is modified
    * to contain the assigned id.
    *
    * An exception will be thrown upon error conditions.
    *
    * @param security MyMoneySecurity filled with data
    */
  virtual void addSecurity(MyMoneySecurity& security) = 0;

  /**
    * This method is used to modify an existing MyMoneySecurity
    * object.
    *
    * An exception will be thrown upon erronous situations.
    *
    * @param security reference to the MyMoneySecurity object to be updated
    */
  virtual void modifySecurity(const MyMoneySecurity& security) = 0;

  /**
    * This method is used to remove an existing MyMoneySecurity object
    * from the engine.
    *
    * An exception will be thrown upon erronous situations.
    *
    * @param security reference to the MyMoneySecurity object to be removed
    */
  virtual void removeSecurity(const MyMoneySecurity& security) = 0;

  /**
    * This method is used to retrieve a single MyMoneySecurity object.
    * The id of the object must be supplied in the parameter @p id.
    *
    * An exception will be thrown upon erronous situations.
    *
    * @param id QCString containing the id of the MyMoneySecurity object
    * @return MyMoneySecurity object
    */
  virtual const MyMoneySecurity security(const QCString& id) const = 0;

  /**
    * This method returns a list of the security objects
    * inside a MyMoneyStorage object
    *
    * @return QValueList<MyMoneySecurity> containing objects
    */
  virtual const QValueList<MyMoneySecurity> securityList(void) const = 0;

  virtual void addPrice(const MyMoneyPrice& price) = 0;
  virtual void removePrice(const MyMoneyPrice& price) = 0;
  virtual MyMoneyPrice price(const QCString& fromId, const QCString& toId, const QDate& date, const bool exactDate) const = 0;

  /**
    * This method returns a list of all prices.
    *
    * @return MyMoneyPriceList of all MyMoneyPrice objects.
    */
  virtual const MyMoneyPriceList& priceList(void) const = 0;

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
    * @param currency reference to the MyMoneySecurity object
    */
  virtual void addCurrency(const MyMoneySecurity& currency) = 0;

  /**
    * This method is used to modify an existing MyMoneySecurity
    * object.
    *
    * An exception will be thrown upon erronous situations.
    *
    * @param currency reference to the MyMoneyCurrency object
    */
  virtual void modifyCurrency(const MyMoneySecurity& currency) = 0;

  /**
    * This method is used to remove an existing MyMoneySecurity object
    * from the engine.
    *
    * An exception will be thrown upon erronous situations.
    *
    * @param currency reference to the MyMoneySecurity object
    */
  virtual void removeCurrency(const MyMoneySecurity& currency) = 0;

  /**
    * This method is used to retrieve a single MyMoneySecurity object.
    * The id of the object must be supplied in the parameter @p id.
    *
    * An exception will be thrown upon erronous situations.
    *
    * @param id QCString containing the id of the MyMoneySecurity object
    * @return MyMoneyCurrency object
    */
  virtual const MyMoneySecurity& currency(const QCString& id) const = 0;

  /**
    * This method is used to retrieve the list of all currencies
    * known to the engine.
    *
    * An exception will be thrown upon erronous situations.
    *
    * @return QValueList of all MyMoneySecurity objects representing a currency.
    */
  virtual const QValueList<MyMoneySecurity> currencyList(void) const = 0;

  /**
    * This method is used to retrieve the list of all reports
    * known to the engine.
    *
    * An exception will be thrown upon erronous situations.
    *
    * @return QValueList of all MyMoneyReport objects.
    */
  virtual const QValueList<MyMoneyReport> reportList( void ) const = 0;

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
  virtual void addReport( MyMoneyReport& report ) = 0;

  /**
    * This method is used to modify an existing MyMoneyReport
    * object. Therefor, the id attribute of the object must be set.
    *
    * An exception will be thrown upon erronous situations.
    *
    * @param report const reference to the MyMoneyReport object to be updated
    */
  virtual void modifyReport( const MyMoneyReport& report ) = 0;

  /**
    * This method returns the number of reports currently known to file
    * in the range 0..MAXUINT
    *
    * @return number of reports known to file
    */
  virtual unsigned countReports( void ) const = 0;

  /**
    * This method is used to retrieve a single MyMoneyReport object.
    * The id of the object must be supplied in the parameter @p id.
    *
    * An exception will be thrown upon erronous situations.
    *
    * @param id QCString containing the id of the MyMoneyReport object
    * @return MyMoneyReport object
    */
  virtual MyMoneyReport report( const QCString& id ) const = 0;

  /**
    * This method is used to remove an existing MyMoneyReport object
    * from the engine. The id attribute of the object must be set.
    *
    * An exception will be thrown upon erronous situations.
    *
    * @param report const reference to the MyMoneyReport object to be updated
    */
  virtual void removeReport(const MyMoneyReport& report) = 0;

  /**
    * This method is used to retrieve the list of all budgets
    * known to the engine.
    *
    * An exception will be thrown upon erronous situations.
    *
    * @return QValueList of all MyMoneyBudget objects.
    */
  virtual const QValueList<MyMoneyBudget> budgetList( void ) const = 0;

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
  virtual void addBudget( MyMoneyBudget& budget ) = 0;

  /**
    * This method is used to retrieve the id to a corresponding
    * name of a budget
    * An exception will be thrown upon error conditions.
    *
    * @param budget QString reference to name of budget
    *
    * @return MyMoneyBudget object of budget
    */
  virtual const MyMoneyBudget& budgetByName(const QString& budget) const = 0;

  /**
    * This method is used to modify an existing MyMoneyBudget
    * object. Therefor, the id attribute of the object must be set.
    *
    * An exception will be thrown upon erronous situations.
    *
    * @param budget const reference to the MyMoneyBudget object to be updated
    */
  virtual void modifyBudget( const MyMoneyBudget& budget ) = 0;

  /**
    * This method returns the number of budgets currently known to file
    * in the range 0..MAXUINT
    *
    * @return number of budgets known to file
    */
  virtual unsigned countBudgets( void ) const = 0;

  /**
    * This method is used to retrieve a single MyMoneyBudget object.
    * The id of the object must be supplied in the parameter @p id.
    *
    * An exception will be thrown upon erronous situations.
    *
    * @param id QCString containing the id of the MyMoneyBudget object
    * @return MyMoneyBudget object
    */
  virtual MyMoneyBudget budget( const QCString& id ) const = 0;

  /**
    * This method is used to remove an existing MyMoneyBudget object
    * from the engine. The id attribute of the object must be set.
    *
    * An exception will be thrown upon erronous situations.
    *
    * @param budget const reference to the MyMoneyBudget object to be updated
    */
  virtual void removeBudget(const MyMoneyBudget& budget) = 0;



  /**
    * Clear all internal caches (used internally for performance measurements)
    */
  virtual void clearCache(void) = 0;

  /**
    * This method checks, if the given @p object is referenced
    * by another engine object.
    *
    * @param obj const reference to object to be checked
    * @param skipCheck MyMoneyFileBitArray with ReferenceCheckBits set for which
    *                  the check should be skipped
    *
    * @retval false @p object is not referenced
    * @retval true @p institution is referenced
    */
  virtual bool isReferenced(const MyMoneyObject& obj, const MyMoneyFileBitArray& skipCheck = MyMoneyFileBitArray()) const = 0;

  /**
    * This method is provided to allow closing of the database before logoff
    */
  virtual void close(void) = 0;

  /**
    * These methods have to be provided to allow transaction safe data handling.
    */
  virtual void startTransaction(void) = 0;
  virtual void commitTransaction(void) = 0;
  virtual void rollbackTransaction(void) = 0;
};

#endif
