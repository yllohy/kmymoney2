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

// ----------------------------------------------------------------------------
// QT Includes

#include <qstring.h>
#include <qmap.h>
#include <qvaluelist.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "storage/imymoneystorage.h"
#include "mymoneyexception.h"
#include "mymoneyutils.h"
#include "mymoneyinstitution.h"
#include "mymoneyaccount.h"
#include "mymoneytransaction.h"
#include "mymoneypayee.h"
#include "mymoneyobserver.h"
#include "mymoneysubject.h"
#include "mymoneykeyvaluecontainer.h"

/**
  * @author Thomas Baumgart, Michael Edwardes
  */

class IMyMoneyStorage;
class MyMoneyTransactionFilter;

/**.
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
  * @exceptions MyMoneyException is thrown whenever an error occurs
  * while the engine code is running. The MyMoneyException:: object
  * describes the problem.
  */
class MyMoneyFile
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
  const QString userName(void) const;
  const QString userStreet(void) const;
  const QString userTown(void) const;
  const QString userCounty(void) const;
  const QString userPostcode(void) const;
  const QString userTelephone(void) const;
  const QString userEmail(void) const;

  // general set functions
  void setUserName(const QString& val);
  void setUserStreet(const QString& val);
  void setUserTown(const QString& val);
  void setUserCounty(const QString& val);
  void setUserPostcode(const QString& val);
  void setUserTelephone(const QString& val);
  void setUserEmail(const QString& val);

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
    * This method is used to return the standard liability account
    * @return MyMoneyAccount liability account(group)
    */
  const MyMoneyAccount liability(void) const;

  /**
    * This method is used to return the standard asset account
    * @return MyMoneyAccount asset account(group)
    */
  const MyMoneyAccount asset(void) const;

  /**
    * This method is used to return the standard expense account
    * @return MyMoneyAccount expense account(group)
    */
  const MyMoneyAccount expense(void) const;

  /**
    * This method is used to return the standard income account
    * @return MyMoneyAccount income account(group)
    */
  const MyMoneyAccount income(void) const;

  /**
    * This method returns an indicator if the MyMoneyFile object has been
    * changed after it has last been saved to permanent storage.
    *
    * @return true if changed, false if not
    */
  bool dirty(void) const;

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
    * global transaction pool. It returns either the whole journal or
    * the set of transaction referenced by a specific account depending
    * on the argument given.
    *
    * @param account QCString reference to account id. If account equals ""
    *                all transactions (the journal) is returned. If account
    *                is not equal to "" it returns the set of transactions
    *                that have splits in this account.
    *
    * @return set of transactions in form of a QValueList<MyMoneyTransaction>
    */
  const QValueList<MyMoneyTransaction> transactionList(const QCString& account = "") const;

  /**
    * This method is used to remove a transaction from the transaction
    * pool (journal).
    *
    * @param transaction const reference to transaction to be deleted
    */
  void removeTransaction(const MyMoneyTransaction& transaction);

  /**
    * This method is used to return the actual balance of an account
    * without it's sub-ordinate accounts
    *
    * @param account id of the account in question
    * @return balance of the account as MyMoneyMoney object
    */
  const MyMoneyMoney balance(const QCString& id) const;

  /**
    * This method is used to return the actual balance of an account
    * including it's sub-ordinate accounts
    *
    * @param account id of the account in question
    * @return balance of the account as MyMoneyMoney object
    */
  const MyMoneyMoney totalBalance(const QCString& id) const;

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
    * @return QValueList containing the institution information
    */
  const QValueList<MyMoneyInstitution> institutionList(void) const;

  /**
    * Returns the account addressed by it's id.
    *
    * @param id id of the account to locate.
    * @return reference to MyMoneyAccount object. An exception is thrown
    *         if the id is unknown
    */
  const MyMoneyAccount& account(const QCString& id) const;

  /**
    * This method returns a list of the accounts
    * inside a MyMoneyFile object
    *
    * @return QMap<QString,MyMoneyAccount> with accounts
    */
  const QValueList<MyMoneyAccount> accountList(void) const;

  /**
    * This method is used to convert an account id to a string representation
    * of the names which can be used as a category description. If the account
    * is part of a hierarchy, the category name will be the concatenation of
    * the single account names seperated by MyMoneyAccount::AccountSeperator.
    *
    * @param id const QCString reference of the account's id
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

public:
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
    * is a special id that will be notified whenever any account is changed
    */
  static const QCString NotifyClassPayee;

  /**
    * MyMoneyFile::NotifyClassPayee
    * is a special id that will be notified whenever any account is changed
    */
  static const QCString NotifyClassInstitution;


protected:
  /**
    * This is the constructor for a new empty file description
    */
  MyMoneyFile();

private:
  const QCString locateSubAccount(const MyMoneyAccount& base, const QString& category) const;


private:
  /**
    * This method is used to add an id to the list of objects
    * to be notified. If id == "", then nothing is added to the list.
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
  void notify(const QCString& id) const;

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

  static MyMoneyFile* _instance;
};
#endif

