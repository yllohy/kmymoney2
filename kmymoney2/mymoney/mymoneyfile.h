/***************************************************************************
                          mymoneyfile.h
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

#ifndef MYMONEYFILE_H
#define MYMONEYFILE_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// ----------------------------------------------------------------------------
// QT Includes

#include <qstring.h>
#include <qmap.h>
#include <qvaluelist.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/imymoneystorage.h>
#include <kmymoney/mymoneyexception.h>
#include <kmymoney/mymoneyutils.h>
#include <kmymoney/mymoneyinstitution.h>
#include <kmymoney/mymoneyaccount.h>
#include <kmymoney/mymoneytransaction.h>
#include <kmymoney/mymoneypayee.h>
#include <kmymoney/mymoneyobserver.h>
#include <kmymoney/mymoneysubject.h>
#include <kmymoney/mymoneykeyvaluecontainer.h>
#include <kmymoney/mymoneysecurity.h>
#include <kmymoney/mymoneyprice.h>
#include <kmymoney/mymoneyreport.h>
#include <kmymoney/mymoneyscheduled.h>
#include <kmymoney/export.h>

/**
  * @author Thomas Baumgart, Michael Edwardes, Kevin Tambascio
  */

class IMyMoneyStorage;
class MyMoneyTransactionFilter;

/**
  * This class represents the interface to the MyMoney engine.
  * For historical reasons it is still called MyMoneyFile.
  * It is implemented using the singleton pattern and thus only
  * exists once for each running instance of an application.
  *
  * The instance of the MyMoneyFile object is accessed as follows:
  *
  * @code
  * MyMoneyFile *file = MyMoneyFile::instance();
  * file->anyMemberFunction();
  * @endcode
  *
  * The first line of the above code creates a unique MyMoneyFile
  * object if it is called for the first time ever. All subsequent
  * calls to this functions return a pointer to the object created
  * during the first call.
  *
  * As the MyMoneyFile object represents the business logic, a storage
  * manager must be attached to it. This mechanism allows to use different
  * access methods to store the objects. The interface to access such an
  * storage manager is defined in the class IMyMoneyStorage. The methods
  * attachStorage() and detachStorage() are used to attach/detach a
  * storage manager object. The following code can be used to create a
  * functional MyMoneyFile instance:
  *
  * @code
  * IMyMoneyStorage *storage = ....
  * MyMoneyFile *file = MyMoneyFile::instance();
  * file->attachStorage(storage);
  * @endcode
  *
  * The methods addAccount(), modifyAccount() and removeAccount() implement the
  * general account maintenance functions. The method reparentAccount() is
  * available to move an account from one superordinate account to another.
  * account() and accountList() are used to retrieve a single instance or a
  * QValueList of MyMoneyAccount objects.
  *
  * The methods addInstitution(), modifyInstitution() and removeInstitution()
  * implement the general institution maintenance functions. institution() and
  * institutionList() are used to retrieve a single instance or a
  * QValueList of MyMoneyInstitution objects.
  *
  * The methods addPayee(), modifyPayee() and removePayee()
  * implement the general institution maintenance functions.
  * payee() and payeeList() are used to retrieve a single instance or a
  * QValueList of MyMoneyPayee objects.
  *
  * The methods addTransaction(), modifyTransaction() and removeTransaction()
  * implement the general transaction maintenance functions.
  * transaction() and transactionList() are used to retrieve
  * a single instance or a QValueList of MyMoneyTransaction objects.
  *
  * The methods addSecurity(), modifySecurity() and removeSecurity()
  * implement the general access to equities held in the engine.
  *
  * The methods addCurrency(), modifyCurrency() and removeCurrency()
  * implement the general access to multiple currencies held in the engine.
  * The methods baseCurrency() and setBaseCurrency() allow to retrieve/set
  * the currency selected by the user as base currency. If a currency
  * reference is emtpy, it will usually be interpreted as baseCurrency().
  *
  * The methods liability(), asset(), expense() and income() are used to
  * retrieve the four standard accounts. isStandardAccount() checks if a
  * given accountId references one of the or not. setAccountName() is used
  * to specify a name for the standard accounts from the GUI.
  *
  * The methods attach() and detach() provide the necessary functions
  * for an external observer to be attached and detached to and from
  * an object of the engine.
  *
  * For abritrary values that have to be stored with the storage object
  * but are of importance to the application only, the object is derived
  * for MyMoneyKeyValueContainer which provides a container to store
  * these values indexed by an alphanumeric key.
  *
  * @exception MyMoneyException is thrown whenever an error occurs
  * while the engine code is running. The MyMoneyException:: object
  * describes the problem.
  */
class KMYMONEY_EXPORT MyMoneyFile
{
public:

  class MyMoneyFileSubject : public MyMoneySubject
  {
  public:
    MyMoneyFileSubject() {};
    ~MyMoneyFileSubject() {};
  };


  class MyMoneyNotifier
  {
  public:
    MyMoneyNotifier(MyMoneyFile* file) { m_file = file; m_file->clearNotification(); };
    ~MyMoneyNotifier() { m_file->notify(); };
  private:
    MyMoneyFile* m_file;
  };

  friend class MyMoneyNotifier;

  /**
    * This is the function to access the MyMoneyFile object.
    * It returns a pointer to the single instance of the object.
    * If no instance exists, it will be created.
    */
  static MyMoneyFile* const instance();

  /**
    * This is the destructor for any MyMoneyFile object
    */
  ~MyMoneyFile();

  /**
    * @deprecated This is a convenience constructor. Do not use it anymore.
    * It will be deprecated in a future version of the engine.
    *
    * @param storage pointer to object that implements the IMyMoneyStorage
    *                interface.
    */
  MyMoneyFile(IMyMoneyStorage *storage);

  // general get functions
  const MyMoneyPayee& user(void) const;

  // general set functions
  void setUser(const MyMoneyPayee& user);

  /**
    * This method can be used to turn on/off the notifications
    * of the engine. During bulk-updates it's sometimes useful
    * to turn off notifications for an improved performance.
    *
    * Engine notifications that are generated while the notifications
    * are suspended will be postponed until the notifications are
    * re-enabled.
    *
    * @param state if @p true, no notifications will be send out,
    *              if @p false, notifications will be send out.
    */
  void suspendNotify(const bool state);

  /**
    * This method is used to attach a storage to the MyMoneyFile object
    * Without an attached storage object, the MyMoneyFile object is
    * of no use. In case of an error condition, an exception is thrown.
    *
    * @param storage pointer to object that implements the IMyMoneyStorage
    *                interface.
    */
  void attachStorage(IMyMoneyStorage* const storage);

  /**
    * This method is used to detach a previously attached storage
    * object from the MyMoneyFile object. In case of an error
    * condition an exception will be thrown.
    *
    * @param storage pointer to object that implements the IMyMoneyStorage
    *                interface.
    */
  void detachStorage(IMyMoneyStorage* const storage = 0);

  /**
    * This method returns whether a storage is currently attached to
    * the engine or not.
    *
    * @return true if storage object is attached, false otherwise
    */
  bool storageAttached(void) const { return m_storage != 0; };

  /**
    * This method returns a pointer to the storage object
    *
    * @return const pointer to the current attached storage object.
    *         If no object is attached, returns 0.
    */
  IMyMoneyStorage* const storage(void) const { return m_storage; };

  /**
    * This method is used to return the standard liability account
    * @return MyMoneyAccount liability account(group)
    */
  const MyMoneyAccount& liability(void) const;

  /**
    * This method is used to return the standard asset account
    * @return MyMoneyAccount asset account(group)
    */
  const MyMoneyAccount& asset(void) const;

  /**
    * This method is used to return the standard expense account
    * @return MyMoneyAccount expense account(group)
    */
  const MyMoneyAccount& expense(void) const;

  /**
    * This method is used to return the standard income account
    * @return MyMoneyAccount income account(group)
    */
  const MyMoneyAccount& income(void) const;

  /**
    * This method is used to return the standard equity account
    * @return MyMoneyAccount equity account(group)
    */
  const MyMoneyAccount& equity(void) const;

  /**
    * This method returns the account information for the opening
    * balances account for the given @p security. If the respective
    * account does not exist, it will be created. The name is constructed
    * using MyMoneyFile::OpeningBalancesPrefix and appending " (xxx)" in
    * case the @p security is not the baseCurrency(). The account created
    * will be a sub-account of the standard equity account provided by equity().
    *
    * @param security Security for which the account is searched
    *
    * @return The opening balance account
    *
    * @note No notifications will be sent!
    */
  const MyMoneyAccount openingBalanceAccount(const MyMoneySecurity& security);

  /**
    * This method is essentially the same as the above, except it works on
    * const objects.  If there is no opening balance account, this method
    * WILL NOT create one.  Instead it will thrown an exception.
    *
    * @param security Security for which the account is searched
    *
    * @return The opening balance account
    *
    * @note No notifications will be sent!
    */
  const MyMoneyAccount openingBalanceAccount(const MyMoneySecurity& security) const;

  /**
    * Create an opening balance transaction for the account @p acc
    * with a value of @p balance. If the corresponding opening balance account
    * for the account's currency does not exist it will be created. If it exists
    * and it's opening date is later than the opening date of @p acc,
    * the opening date of the opening balances account will be adjusted to the
    * one of @p acc.
    *
    * @param acc reference to account for which the opening balance transaction
    *            should be created
    * @param balance reference to the value of the opening balance transaction
    */
  void createOpeningBalanceTransaction(const MyMoneyAccount& acc, const MyMoneyMoney& balance);

  /**
    * Retrieve the opening balance transaction for the account @p acc.
    * If there is no opening balance transaction, QCString() will be returned.
    *
    * @param acc reference to account for which the opening balance transaction
    *            should be retrieved
    * @return QCString id for the transaction, or QCString() if no transaction exists
    */
  QCString openingBalanceTransaction(const MyMoneyAccount& acc) const;

  /**
    * This method returns an indicator if the MyMoneyFile object has been
    * changed after it has last been saved to permanent storage.
    *
    * @return true if changed, false if not
    */
  bool dirty(void) const;

  /**
    * This method is used to force the attached storage object to
    * be dirty. This is used by the application to re-set the dirty
    * flag after a failed upload to a server when the save operation
    * to a local temp file was OK.
    */
  void setDirty(void) const;

  /**
    * Adds an institution to the file-global institution pool. A
    * respective institution-ID will be generated for this object.
    * The ID is stored as QString in the object passed as argument.
    *
    * An exception will be thrown upon error conditions.
    *
    * @param institution The complete institution information in a
    *        MyMoneyInstitution object
    */
  void addInstitution(MyMoneyInstitution& institution);

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
    * Deletes an existing institution from the file global institution pool
    * Also modifies the accounts that reference this institution as
    * their institution.
    *
    * An exception will be thrown upon error conditions.
    *
    * @param institution institution to be deleted.
    */
  void removeInstitution(const MyMoneyInstitution& institution);

  /**
    * Adds an account to the file-global institution pool. A respective
    * account-ID will be generated within this record. The modified
    * members of account will be updated.
    *
    * A few parameters of the account to be added are checked against
    * the following conditions. If they do not match, an exception is
    * thrown.
    *
    * An account must match the following conditions:
    *
    *   a) the account must have a name with length > 0
    *   b) the account must not have an id assigned
    *   c) the transaction list must be empty
    *   d) the account must not have any sub-ordinate accounts
    *   e) the account must have no parent account
    *   f) the account must not have reference to a MyMoneyFile object
    *
    * An exception will be thrown upon error conditions.
    *
    * @param account The complete account information in a MyMoneyAccount object
    * @param parent  The complete account information of the parent account
    */
  void addAccount(MyMoneyAccount& account, MyMoneyAccount& parent);

  /**
    * Modifies an already existing account in the file global account pool.
    *
    * An exception will be thrown upon error conditions.
    *
    * @param account reference to the new account information
    */
  void modifyAccount(const MyMoneyAccount& account);

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
    * This converts the account type into one of the four
    * major account types liability, asset, expense or income
    * @param type actual account type
    * @return accountTypeE of major account type
    */
  const MyMoneyAccount::accountTypeE accountGroup(MyMoneyAccount::accountTypeE type) const;

  /**
    * moves splits from one account to another
    *
    * @param oldAccount id of the current account
    * @param newAccount if of the new account
    *
    * @return the number of modified splits
    */
  const unsigned int moveSplits(const QCString& oldAccount, const QCString& newAccount);

  /**
    * This method is used to determince, if the account with the
    * given ID is referenced by any split in m_transactionList.
    *
    * @param id id of the account to be checked for
    * @return true if account is referenced, false otherwise
    */
  const bool hasActiveSplits(const QCString& id) const;

  /**
    * This method is used to check whether a given
    * account id references one of the standard accounts or not.
    *
    * An exception will be thrown upon error conditions.
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
    * @param id QCString reference to one of the standard accounts.
    * @param name QString reference to the name to be set
    *
    */
  void setAccountName(const QCString& id, const QString& name) const;

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
    * Adds a transaction to the file-global transaction pool. A respective
    * transaction-ID will be generated for this object. The ID is stored
    * as QCString in the object passed as argument.
    * Splits must reference valid accounts and valid payees. The payee
    * id can be empty.
    *
    * An exception will be thrown upon error conditions.
    *
    * @param transaction reference to the transaction
    */
  void addTransaction(MyMoneyTransaction& transaction);

  /**
    * This method is used to update a specific transaction in the
    * transaction pool of the MyMoneyFile object.
    * Splits must reference valid accounts and valid payees. The payee
    * id can be empty.
    *
    * An exception will be thrown upon error conditions.
    *
    * @param transaction reference to transaction to be changed
    */
  void modifyTransaction(const MyMoneyTransaction& transaction);

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
  QValueList<MyMoneyTransaction> transactionList(MyMoneyTransactionFilter& filter) const;

  void transactionList(QValueList<MyMoneyTransaction>& list, MyMoneyTransactionFilter& filter) const;

  /**
    * This method is used to remove a transaction from the transaction
    * pool (journal).
    *
    * @param transaction const reference to transaction to be deleted
    */
  void removeTransaction(const MyMoneyTransaction& transaction);

  /**
    * This method is used to return the actual balance of an account
    * without it's sub-ordinate accounts. If a @p date is presented,
    * the balance at the beginning of this date (not including any
    * transaction on this date) is returned. Otherwise all recorded
    * transactions are included in the balance.
    *
    * @param id id of the account in question
    * @param date return balance for specific date (default = QDate())
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
    * @param date return balance for specific date (default = QDate())
    * @return balance of the account as MyMoneyMoney object
    */
  const MyMoneyMoney totalBalance(const QCString& id, const QDate& date = QDate()) const;

  /**
    * This method is used to return the actual value of an account
    * including it's sub-ordinate accounts. A value of an account
    * is calculated by multiplying the balance with the currencies
    * price for today. The price for accounts held in the baseCurrency()
    * is 1. The same applies for prices that are not available (no entry
    * in price list for the current date).
    *
    * See totalValueValid(const QCString&) for information about the
    * validity of the returned value.
    *
    * @param id account id of the account in question
    * @param date return value for specific date (default = QDate())
    * @return value of the account as MyMoneyMoney object
    */
  const MyMoneyMoney totalValue(const QCString& id, const QDate& date = QDate()) const;

  /**
    * This method is used to return the actual value of an account
    * excluding it's sub-ordinate accounts. A value of an account
    * is calculated by multiplying the balance with the security's
    * price for today. If the security's trading currency differs
    * from baseCurrency() then the result of this multiplication
    * is multiplied again by the trading currency's price for today.
    * If a @p date is presented,
    * the value at the beginning of this date (not including any
    * transaction on this date) is returned. Otherwise all recorded
    * transactions are included in the value.
    *
    * The price for accounts held in the baseCurrency()
    * is 1. The same applies for prices that are not available (no entry
    * in price list for the current date).
    *
    * See accountValueValid(const QCString&) for information about the
    * validity of the returned value.
    *
    * @param id account id of the account in question
    * @param date return value for specific date (default = QDate())
    * @return value of the account as MyMoneyMoney object
    */
  const MyMoneyMoney accountValue(const QCString& id, const QDate& date = QDate()) const;

  /**
    * This method returns if the value returned by totalValue(const QCString&)
    * is valid or not. The value is considered valid, if all conversion rates (prices)
    * are available to convert the account's balance to the base currency.
    * The same applies for any of the subaccounts of @p id.
    *
    * If for any of the accounts a conversion rate for the current date cannot
    * be found, false is returned. If for all such currencies a conversion rate
    * is available, true is returned.
    *
    * @param id account id of the account in question
    * @retval false value returned by totalValue(const QCString&) is not
    *               completely correct (missing conversion rates are assumed to be 1)
    * @retval true value returned by totalValue(const QCString&) is correct
    */
  const bool totalValueValid(const QCString& id) const;

  /**
    * This method returns if the value returned by accountValue(const QCString&)
    * is valid or not. The value is considered valid, if a conversion rate (price)
    * is available for the currency used for the account referenced by @p id.
    *
    * @param id account id of the account in question
    * @retval false value returned by accountValue(const QCString&) is not correct
    * @retval true value returned by accountValue(const QCString&) is correct
    */
  const bool accountValueValid(const QCString& id) const;

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

  /**
    * This method returns a QMap filled with the number of transactions
    * per account. The account id serves as index into the map. If one
    * needs to have all transactionCounts() for many accounts, this method
    * is faster than calling transactionCount(const QCString& account) many
    * times.
    *
    * @return QMap with numbers of transactions per account
    */
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
    * Returns the institution of a given ID
    *
    * @param id id of the institution to locate
    * @return MyMoneyInstitution object filled with data. If the institution
    *         could not be found, an exception will be thrown
    */
  const MyMoneyInstitution& institution(const QCString& id) const;

  /**
    * This method returns a list of the institutions
    * inside a MyMoneyFile object
    *
    * @return QValueList containing the institution objects
    */
  const QValueList<MyMoneyInstitution> institutionList(void) const;

  /**
    * Returns the account addressed by it's id.
    *
    * @param id id of the account to locate.
    * @return MyMoneyAccount object carrying the @p id. An exception is thrown
    *         if the id is unknown
    */
  const MyMoneyAccount& account(const QCString& id) const;

  /**
    * This method returns a list of accounts inside a MyMoneyFile object.
    * An optional parameter is a list of id's. If this list is emtpy (the default)
    * the returned list contains all accounts, otherwise only those referenced
    * in the id-list.
    *
    * @param idlist QCStringList of account ids of those accounts that
    *        should be returned. If this list is empty, all accounts
    *        currently known will be returned.
    *
    * @param recursive if @p true, then recurse in all found accounts. The default is @p false
    *
    * @return QValueList containing the account objects
    */
  const QValueList<MyMoneyAccount> accountList(const QCStringList& idlist = QCStringList(), const bool recursive = false) const;

  /**
    * This method is used to convert an account id to a string representation
    * of the names which can be used as a category description. If the account
    * is part of a hierarchy, the category name will be the concatenation of
    * the single account names seperated by MyMoneyAccount::AccountSeperator.
    *
    * @param accountId const QCString reference of the account's id
    *
    * @return QString of the constructed name.
    */
  const QString accountToCategory(const QCString& accountId) const;

  /**
    * This method is used to convert a string representing a category to
    * an account id. A category can be the concatenation of multiple accounts
    * representing a hierarchy of accounts. They have to be seperated by
    * MyMoneyAccount::AccountSeperator.
    *
    * @param category const reference to QString containing the category
    *
    * @return QCString of the corresponding account. If account was not found
    *         the return value will be an empty string.
    */
  const QCString categoryToAccount(const QString& category) const;

  /**
    * This method is used to convert a string representing an asset or
    * liability account to an account id. An account name can be the
    * concatenation of multiple accounts representing a hierarchy of
    * accounts. They have to be seperated by MyMoneyAccount::AccountSeperator.
    *
    * @param name const reference to QString containing the account name
    *
    * @return QCString of the corresponding account. If account was not found
    *         the return value will be an empty string.
    */
  const QCString nameToAccount(const QString& name) const;

  /**
    * This method is used to extract the parent part of an account hierarchy
    * name who's parts are seperated by MyMoneyAccount::AccountSeperator.
    *
    * @param name full account name
    * @return parent name (full account name excluding the last part)
    */
  const QString parentName(const QString& name) const;

  /**
    * This method is used to attach an observer to a subject
    * represented by it's id. Whenever the object represented
    * by the id changes it's state, the observers method
    * ::update(const QCString& id) is called with id set to
    * the value passed as argument to attach. This allows an
    * object of the GUI to observe multiple objects with the
    * ability to know which object changed. If it observes
    * multiple objects, it's update(const QCString& id) method
    * is called multiple times if more than one object is changed
    * during a single operation.
    *
    * @param id reference to id of the subject from which
    *           notifications should be activated
    * @param observer pointer to object observing the subject
    *
    * @see detach
    */
  void attach(const QCString& id, MyMoneyObserver* observer);

  /**
    * This method is used to detach an observer from a subject
    * represented by it's id. If an object observes more than one
    * subject, it must call this method multiple times.
    *
    * @param id reference to id of the subject from which
    *           notifications were activated
    * @param observer pointer to object observing the subject
    *
    * @see attach
    */
  void detach(const QCString& id, MyMoneyObserver* observer);

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
  const MyMoneyPayee& payee(const QCString& id) const;

  /**
    * This method is used to retrieve the id to a corresponding
    * name of a payee/receiver.
    * An exception will be thrown upon error conditions.
    *
    * @param payee QString reference to name of payee
    *
    * @return MyMoneyPayee object of payee
    */
  const MyMoneyPayee& payeeByName(const QString& payee) const;

  /**
    * This method is used to modify an existing payee
    *
    * An exception will be thrown upon error conditions
    *
    * @param payee MyMoneyPayee reference to payee information
    */
  void modifyPayee(const MyMoneyPayee& payee);

  /**
    * This method is used to remove an existing payee.
    * An error condition occurs, if the payee is still referenced
    * by a split.
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
    * This method is used to extract a value from the storage's
    * KeyValueContainer. For details see MyMoneyKeyValueContainer::value().
    *
    * @param key const reference to QCString containing the key
    * @return QString containing the value
    */
  const QString value(const QCString& key) const;

  /**
    * This method is used to set a value in the storage's
    * KeyValueContainer. For details see MyMoneyKeyValueContainer::setValue().
    *
    * @param key const reference to QCString containing the key
    * @param val const reference to QString containing the value
    *
    * @note Keys starting with the leadin @p kmm- are reserved for internal use
    *       by the MyMoneyFile object.
    */
  void setValue(const QCString& key, const QString& val);

  /**
    * This method is used to delete a key-value-pair from the
    * storage's KeyValueContainer identified by the parameter
    * @p key. For details see MyMoneyKeyValueContainer::deletePair().
    *
    * @param key const reference to QCString containing the key
    */
  void deletePair(const QCString& key);

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

  const QStringList consistencyCheck(void);

  /**
    * MyMoneyFile::NotifyClassAccount
    * is a special id that will be notified whenever any account is changed.
    * The typical usage is in the account tree, that must be notified about
    * all changes of any account.
    *
    * @code
    * MyAccountTreeClass tree;  // MyAccountTreeClass is derived from MyMoneyObserver
    * MyMoneyFile *file = MyMoneyFile::instance();
    *
    * file->attach(MyMoneyFile::NotifyClassAccount, &tree);
    * // from now on, the tree.update() method will be called, whenever an
    * // account in the storage area is changed.
    *
    * file->detach(MyMoneyFile::NotifyClassAccount, &tree);
    * // the change notification will not be performed anymore.
    * @endcode
    */
  static const QCString NotifyClassAccount;

  /**
    * MyMoneyFile::NotifyClassAccount
    * is a special id that will be notified whenever the accounthierarchy is
    * changed.
    * The typical usage is in the account tree, that must be reconstructed
    * upon this type of change.
    */
  static const QCString NotifyClassAccountHierarchy;

  /**
    * MyMoneyFile::NotifyClassPayee
    * is a special id that will be notified whenever any account is add,
    * changed or removed from the engine
    */
  static const QCString NotifyClassPayee;

  /**
    * MyMoneyFile::NotifyClassPayeeSet
    * is a special id that will be notified whenever any account is add
    * or removed from the engine
    */
  static const QCString NotifyClassPayeeSet;

  /**
    * MyMoneyFile::NotifyClassPayee
    * is a special id that will be notified whenever any account is changed
    */
  static const QCString NotifyClassInstitution;

  /**
    * MyMoneyFile::NotifyClassSchedule
    * is a special id that will be notified whenever any schedule is changed
    */
  static const QCString NotifyClassSchedule;

  /**
    * MyMoneyFile::NotifyClassAnyChange
    * is a special id that will be notified whenever any object of the engine changes
    */
  static const QCString NotifyClassAnyChange;

  /**
    * MyMoneyFile::NotifyClassCurrency
    * is a special id that will be notified whenever any currency is changed
    */
  static const QCString NotifyClassCurrency;

  /**
    * MyMoneyFile::NotifyClassSecurity
    * is a special id that will be notified whenever any security is changed
    */
  static const QCString NotifyClassSecurity;

  /**
    * MyMoneyFile::NotifyClassPrice
    * is a special id that will be notified whenever a price is changed
    */
  static const QCString NotifyClassPrice;

  /**
    * MyMoneyFile::NotifyClassReport
    * is a special id that will be notified whenever any report is changed
    */
  static const QCString NotifyClassReport;

  /**
    * MyMoneyFile::OpeningBalancesPrefix is a special string used
    * to generate the name for opening balances accounts. See openingBalanceAccount()
    * for details.
    */
  static const QString OpeningBalancesPrefix;

  /**
    * createCategory creates a category from a text name.
    *
    * The whole account hierarchy is created if it doesnt
    * already exist.  e.g if name = Bills:Credit Card and
    * base = expense(), Bills will first be checked to see if
    * it exists and created if not.  Credit Card will then
    * be created with Bills as it's parent.  The Credit Card account
    * will have it's id returned.
    *
    * @param base The base account (expense or income)
    * @param name The category to create
    *
    * @return The category account id or empty on error.
    *
    * @exception An exception will be thrown, if @p base is not equal
    *            expense() or income().
    **/
  QCString createCategory(const MyMoneyAccount& base, const QString& name);

  QValueList<MyMoneySchedule> scheduleListEx( int scheduleTypes,
                                              int scheduleOcurrences,
                                              int schedulePaymentTypes,
                                              QDate startDate,
                                              const QCStringList& accounts=QCStringList()) const;

  /**
    * This method is used to add a new security object to the engine.
    * The ID of the object is the trading symbol, so there is no need for an additional
    * ID since the symbol is guaranteed to be unique.
    *
    * An exception will be thrown upon erronous situations.
    *
    * @param security reference to the MyMoneySecurity object
    */
  void addSecurity(MyMoneySecurity& security);

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
    * This method is used to retrieve a single MyMoneySecurity object.
    * The id of the object must be supplied in the parameter @p id.
    * If no security with the given id is found, then a corresponding
    * currency is searched. If @p id is empty, the baseCurrency() is returned.
    *
    * An exception will be thrown upon erronous situations.
    *
    * @param id QCString containing the id of the MyMoneySecurity object
    * @return MyMoneySecurity object
    */
  const MyMoneySecurity security(const QCString& id) const;

  /**
    * This method is used to retrieve a list of all MyMoneySecurity objects.
    */
  const QValueList<MyMoneySecurity> securityList(void) const;

  /**
    * This method is used to add a new currency object to the engine.
    * The ID of the object is the trading symbol, so there is no need for an additional
    * ID since the symbol is guaranteed to be unique.
    *
    * An exception will be thrown upon erronous situations.
    *
    * @param currency reference to the MyMoneySecurity object
    */
  void addCurrency(const MyMoneySecurity& currency);

  /**
    * This method is used to modify an existing MyMoneySecurity
    * object.
    *
    * An exception will be thrown upon erronous situations.
    *
    * @param currency reference to the MyMoneySecurity object
    */
  void modifyCurrency(const MyMoneySecurity& currency);

  /**
    * This method is used to remove an existing MyMoneySecurity object
    * from the engine.
    *
    * An exception will be thrown upon erronous situations.
    *
    * @param currency reference to the MyMoneySecurity object
    */
  void removeCurrency(const MyMoneySecurity& currency);

  /**
    * This method is used to retrieve a single MyMoneySchedule object.
    * The id of the object must be supplied in the parameter @p id.
    * If @p id is empty, this method returns baseCurrency().
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
    * @return QValueList of all MyMoneySecurity objects.
    */
  const QValueList<MyMoneySecurity> currencyList(void) const;

  /**
    * This method retrieves a MyMoneySecurity object representing
    * the selected base currency. If the base currency is not
    * selected (e.g. due to a previous call to setBaseCurrency())
    * a standard MyMoneySecurity object will be returned. See
    * MyMoneySecurity() for details.
    *
    * An exception will be thrown upon erronous situations.
    *
    * @return MyMoneySecurity describing base currency
    */
  const MyMoneySecurity baseCurrency(void) const;

  /**
    * This method allows to select the base currency. It does
    * not perform any changes to the data in the engine. It merely
    * stores a reference to the base currency. The currency
    * passed as argument must exist in the engine.
    *
    * An exception will be thrown upon erronous situations.
    *
    * @param currency
    */
  void setBaseCurrency(const MyMoneySecurity& currency);

  /**
    * This method adds/replaces a price to/from the price list
    */
  void addPrice(const MyMoneyPrice& price);

  /**
    * This method removes a price from the price list
    */
  void removePrice(const MyMoneyPrice& price);

  /**
    * This method is used to retrieve a price for a specific security
    * on a specific date. If there is no price for this date, the last
    * known price for this currency is used. If no price information
    * is available, 1.0 will be returned as price.
    *
    * @param fromId the id of the currency in question
    * @param toId the id of the currency to convert to (if emtpy, baseCurrency)
    * @param date the date for which the price should be returned (default = today)
    * @param exactDate if true, entry for date must exist, if false any price information
    *                  with a date less or equal to @p date will be returned
    *
    * @return price found as MyMoneyPrice object
    * @note This throws an exception when the base currency is not set and toId is empty
    */
  const MyMoneyPrice price(const QCString& fromId, const QCString& toId = QCString(), const QDate& date = QDate::currentDate(), const bool exactDate = false) const;

  /**
    * This method returns a list of all prices.
    *
    * @return MyMoneyPriceList of all MyMoneyPrice objects.
    */
  const MyMoneyPriceList priceList(void) const;

  /**
    * This method allows to interrogate the engine, if a known account
    * with id @p id has a subaccount with the name @p name.
    *
    * @param id id of the account to look at
    * @param name account name that needs to be searched force
    * @retval true account with name @p name found as subaccounts
    * @retval false no subaccount present with that name
    */
  const bool hasAccount(const QCString& id, const QString& name) const;

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
    * Adds a report to the file-global institution pool. A
    * respective report-ID will be generated for this object.
    * The ID is stored as QString in the object passed as argument.
    *
    * An exception will be thrown upon error conditions.
    *
    * @param report The complete report information in a
    *        MyMoneyReport object
    */
  void addReport( MyMoneyReport& report );

  /**
    * Modifies an already existing report in the file global
    * report pool.
    *
    * An exception will be thrown upon error conditions.
    *
    * @param report The complete new report information
    */
  void modifyReport( const MyMoneyReport& report );

  /**
    * This method returns the number of reports currently known to file
    * in the range 0..MAXUINT
    *
    * @return number of reports known to file
    */
  unsigned countReports( void ) const;

  /**
    * This method is used to retrieve a single MyMoneyReport object.
    * The id of the object must be supplied in the parameter @p id.
    *
    * An exception will be thrown upon erronous situations.
    *
    * @param id QCString containing the id of the MyMoneyReport object
    * @return MyMoneyReport object
    */
  MyMoneyReport report( const QCString& id ) const;

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
    * This method checks, if the given @p object is referenced
    * by another engine object.
    *
    * @param obj const reference to object to be checked
    *
    * @retval false @p object is not referenced
    * @retval true @p institution is referenced
    */
  bool isReferenced(const MyMoneyObject& obj) const;

  /**
    * This method checks if the given check no &p no is used in
    * a transaction referencing account &p accId. If  @p accId is empty,
    * @p false is returned.
    *
    * @param accId id of account to checked
    * @param no check number to be verified if used or not
    * @retval false @p no is not in use
    * @retval true @p no is already assigned
    */
  const bool checkNoUsed(const QCString& accId, const QString& no) const;

  /**
    * This method returns the highest assigned check no for
    * account @p accId.
    *
    * @param accId id of account to be scanned
    * @return highest check no. used
    */
  QString highestCheckNo(const QCString& accId) const;

  /**
    * Clear all internal caches (used internally for performance measurements)
    */
  void clearCache(void);

protected:
  /**
    * This is the constructor for a new empty file description
    */
  MyMoneyFile();

private:
  const QCString locateSubAccount(const MyMoneyAccount& base, const QString& category) const;

  void ensureDefaultCurrency(MyMoneyAccount& acc) const;

  void warningMissingRate(const QCString& fromId, const QCString& toId) const;

  /**
    * This method creates an opening balances account. The name is constructed
    * using MyMoneyFile::OpeningBalancesPrefix and appending " (xxx)" in
    * case the @p security is not the baseCurrency(). The account created
    * will be a sub-account of the standard equity account provided by equity().
    *
    * @param security Security for which the account is searched
    */
  const MyMoneyAccount createOpeningBalanceAccount(const MyMoneySecurity& security);

  const MyMoneyAccount openingBalanceAccount_internal(const MyMoneySecurity& security) const;

private:
  /**
    * This method is used to add an id to the list of objects
    * to be notified. If id is empty, then nothing is added to the list.
    *
    * @param id id of object to be notified
    * @see attach, detach
    */
  void addNotification(const QCString& id);

  /**
    * This method is used to clear the notification list
    */
  void clearNotification(void);

  /**
    * This method is used to notify the objects observing
    * a specific id
    */
  void notify(const QCString& id);

  /**
    * This method is used to notify all objects observing
    * any object contained in m_notificationList.
    */
  void notify(void);

  /**
    * This method is used to add the account with id 'id'
    * and all it's parent account's ids to the notification list.
    *
    * @param id account's id
    */
  void notifyAccountTree(const QCString& id);

  /**
    * This method checks if a storage object is attached and
    * throws and exception if not.
    */
  inline void checkStorage(void) const {
    if(m_storage == 0)
      throw new MYMONEYEXCEPTION("No storage object attached to MyMoneyFile");
  }

private:
  /**
    * This member points to the storage strategy
    */
  IMyMoneyStorage *m_storage;

  /**
    * This member variable keeps a set of all subjects known to
    * the object (accounts, institutions, etc)
    */
  QMap<QCString, MyMoneyFileSubject> m_subjects;

  /**
    * This member keeps a list of ids to notify after an
    * operation is completed.
    */
  QMap<QCString, bool> m_notificationList;

  /**
    */
  bool m_suspendNotify;

  static MyMoneyFile* _instance;
};
#endif

