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

/**
  *@author Thomas Baumgart
  */

class IMyMoneyStorage;

/// This class represents the interface to all data kept in a KMyMoney file
class MyMoneyFile
{
public:
  /**
    * This is the constructor for a new empty file description
    */
  MyMoneyFile(IMyMoneyStorage *storage);

  /**
    * This is the destructor for any MyMoneyFile object
    */
  ~MyMoneyFile();

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
  const unsigned int moveSplits(const QString& oldAccount, const QString& newAccount);

  /**
    * This method is used to determince, if the account with the
    * given ID is referenced by any split in m_transactionList.
    *
    * @param id id of the account to be checked for
    * @return true if account is referenced, false otherwise
    */
  const bool hasActiveSplits(const QString& id) const;

  /**
    * This method is used to check whether a given
    * account id references one of the standard accounts or not.
    *
    * An exception will be thrown upon error conditions.
    *
    * @param id account id
    * @return true if account-id is one of the standards, false otherwise
    */
  const bool isStandardAccount(const QString& id) const;

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
    * as QString in the object passed as argument.
    *
    * An exception will be thrown upon error conditions.
    *
    * @param transaction reference to the transaction
    */
  void addTransaction(MyMoneyTransaction& transaction);

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
    * This method is used to extract a transaction from the file global
    * transaction pool through an id. In case of an invalid id, an
    * exception will be thrown.
    *
    * @param id id of transaction as QString.
    * @return reference to the requested transaction
    */
  const MyMoneyTransaction& transaction(const QString& id) const;

  /**
    * This method is used to extract a transaction from the file global
    * transaction pool through an index into an account.
    *
    * @param account id of the account as QString
    * @param idx number of transaction in this account
    * @return reference to MyMoneyTransaction object
    */
  const MyMoneyTransaction& transaction(const QString& account, const int idx) const;

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
  const MyMoneyMoney balance(const QString& id) const;

  /**
    * This method is used to return the actual balance of an account
    * including it's sub-ordinate accounts
    *
    * @param account id of the account in question
    * @return balance of the account as MyMoneyMoney object
    */
  const MyMoneyMoney totalBalance(const QString& id) const;

  /**
    * This method returns the number of transactions currently known to file
    * in the range 0..MAXUINT
    *
    * @return number of transactions in journal
    */
  const unsigned int transactionCount(void) const;

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
  const MyMoneyInstitution& institution(const QString& id) const;

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
  const MyMoneyAccount& account(const QString& id) const;

  /**
    * This method returns a list of the accounts
    * inside a MyMoneyFile object
    *
    * @return QMap<QString,MyMoneyAccount> with accounts
    */
  const QValueList<MyMoneyAccount> accountList(void) const;

  /**
    * The following error codes are defined during reading
    * a MyMoneyFile from a QDataStream.
    */
  enum readStreamErrorType {
    OK = 0,
    UNKNOWN_FILE_TYPE,
    UNKNOWN_FILE_FORMAT
  };

  /**
    * This method is used to read the data from a QDataStream.
    * It is the responsability of the caller to open and close
    * the stream. Different versions of the file layout are
    * handled by this routine.
    *
    * @param s QDataStream to read from
    * @return return code
    */
  const int readStream(QDataStream& s);

  /**
    * This method is used to write all data found in the MyMoneyFile
    * object to a QDataStream.
    * It is the responsability of the caller to open and close
    * the stream.
    *
    * @param s QDataStream to read from
    * @return return code
    */
  const int writeStream(QDataStream& s);

  // Payee operations
  void addPayee(const QString& newPayee, const QString& address=QString::null, const QString& postcode=QString::null, const QString& telephone=QString::null, const QString& email=QString::null);
  void removePayee(const QString name);

private:
  IMyMoneyStorage *m_storage;



};
#endif

