/***************************************************************************
                          imymoneyserialize.h  -  description
                             -------------------
    begin                : Fri May 10 2002
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

#ifndef IMYMONEYSERIALIZE_H
#define IMYMONEYSERIALIZE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qstring.h>
#include <qvaluelist.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "../mymoneyutils.h"
#include "../mymoneyinstitution.h"
#include "../mymoneyaccount.h"
#include "../mymoneytransaction.h"
#include "../mymoneypayee.h"
#include "../mymoneyscheduled.h"
#include "../mymoneytransactionfilter.h"
#include "../mymoneyequity.h"
#include "../mymoneycurrency.h"
#include "../mymoneyreport.h"

/**
  * @author Thomas Baumgart
  */

/**
  * This class represents the interface to serialize a MyMoneyStorage object
  */
class IMyMoneySerialize {
public:
  IMyMoneySerialize();
  virtual ~IMyMoneySerialize();

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
  virtual void setCreationDate(const QDate& val) = 0;

  /**
    * This method is used to set the last modification date of
    * the storage object. It also clears the dirty flag and should
    * therefor be called as last operation when loading from a
    * file.
    *
    * @param val QDate of last modification
    */
  virtual void setLastModificationDate(const QDate& val) = 0;

  /**
    * This method returns a list of the accounts
    * inside a MyMoneyStorage object
    *
    * @return QMap<QString,MyMoneyAccount> with accounts
    */
  virtual const QValueList<MyMoneyAccount> accountList(void) const = 0;

  /**
    * This method returns a list of the institutions
    * inside a MyMoneyStorage object
    *
    * @return QMap containing the institution information
    */
  virtual const QValueList<MyMoneyInstitution> institutionList(void) const = 0;

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
    * @param account QCString reference to account id. If account is empty
    +                all transactions (the journal) is returned. If account
    *                is not empty it returns the set of transactions
    *                that have splits in this account.
    *
    * @return set of transactions in form of a QValueList<MyMoneyTransaction>
    */
  // virtual const QValueList<MyMoneyTransaction> transactionList(const QCString& account = "") const = 0;

  /**
    * This method returns a list of the payees
    * inside a MyMoneyStorage object
    *
    * @return QValueList<MyMoneyPayee> containing the payee information
    */
  virtual const QValueList<MyMoneyPayee> payeeList(void) const = 0;

  /**
    * This method returns a list of the scheduled transactions
    * inside a MyMoneyStorage object. In order to retrieve a complete
    * list of the transactions, all arguments should be used with their
    * default arguments.
    */
  virtual const QValueList<MyMoneySchedule> scheduleList(const QCString& = QCString(),
                                     const MyMoneySchedule::typeE = MyMoneySchedule::TYPE_ANY,
                                     const MyMoneySchedule::occurenceE = MyMoneySchedule::OCCUR_ANY,
                                     const MyMoneySchedule::paymentTypeE = MyMoneySchedule::STYPE_ANY,
                                     const QDate& = QDate(),
                                     const QDate& = QDate(),
                                     const bool = false) const = 0;

   /**
    * This method returns a list of equity objects that the engine has
    * knowledge of.
    */
  virtual const QValueList<MyMoneyEquity> equityList(void) const = 0;

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
    *
    * @deprecated This method is only provided as long as we provide
    *             the version 0.4 binary reader. As soon as we deprecate
    *             this compatability mode this method will disappear from
    *             this interface!
    */
  virtual void addAccount(MyMoneyAccount& parent, MyMoneyAccount& account) = 0;

  /**
    * This method is used to create a new payee
    *
    * An exception will be thrown upon error conditions
    *
    * @param payee MyMoneyPayee reference to payee information
    *
    * @deprecated This method is only provided as long as we provide
    *             the version 0.4 binary reader. As soon as we deprecate
    *             this compatability mode this method will disappear from
    *             this interface!
    *
    */
  virtual void addPayee(MyMoneyPayee& payee) = 0;

  /**
    * This method is used to add an account to an institution. The account
    * data will be updated to contain the correct id of the referenced institution
    * and the account will be added to the list of accounts held at the
    * referenced institution. The objects passed as arguments will be updated
    * accordingly.
    *
    * An exception will be thrown upon error conditions.
    *
    * @param institution MyMoneyInstitution the account should be added to
    * @param account MyMoneyAccount to be added
    *
    * @deprecated This method is only provided as long as we provide
    *             the version 0.4 binary reader. As soon as we deprecate
    *             this compatability mode this method will disappear from
    *             this interface!
    */
  virtual void addAccount(MyMoneyInstitution& institution, MyMoneyAccount& account) = 0;

  /**
    * Adds an institution to the storage. A
    * respective institution-ID will be generated within this record.
    * The ID is stored as QString in the object passed as argument.
    *
    * An exception will be thrown upon error conditions.
    *
    * @param institution The complete institution information in a
    *        MyMoneyInstitution object
    *
    * @deprecated This method is only provided as long as we provide
    *             the version 0.4 binary reader. As soon as we deprecate
    *             this compatability mode this method will disappear from
    *             this interface!
    */
  virtual void addInstitution(MyMoneyInstitution& institution) = 0;

  /**
    * Adds a transaction to the file-global transaction pool. A respective
    * transaction-ID will be generated within this record. The ID is stored
    * as QString with the object.
    *
    * An exception will be thrown upon error conditions.
    *
    * @param transaction reference to the transaction
    * @param skipAccountUpdate if set, the transaction lists of the accounts
    *        referenced in the splits are not updated. This is used for
    *        bulk loading a lot of transactions but not during normal operation.
    *        Refreshing the account's transaction list can be done using
    *        refreshAllAccountTransactionList().
    *
    * @deprecated This method is only provided as long as we provide
    *             the version 0.4 binary reader. As soon as we deprecate
    *             this compatability mode this method will disappear from
    *             this interface!
    */
  virtual void addTransaction(MyMoneyTransaction& transaction, const bool skipAccountUpdate = false) = 0;

  virtual void loadAccount(const MyMoneyAccount& acc) = 0;
  virtual void loadTransaction(const MyMoneyTransaction& tr) = 0;
  virtual void loadInstitution(const MyMoneyInstitution& inst) = 0;
  virtual void loadPayee(const MyMoneyPayee& payee) = 0;
  virtual void loadSchedule(const MyMoneySchedule& sched) = 0;
  virtual void loadEquity(const MyMoneyEquity& equity) = 0;
  virtual void loadCurrency(const MyMoneyCurrency& currency) = 0;
  virtual void loadReport( const MyMoneyReport& report ) = 0;

  virtual const unsigned long accountId(void) = 0;
  virtual const unsigned long transactionId(void) = 0;
  virtual const unsigned long payeeId(void) = 0;
  virtual const unsigned long institutionId(void) = 0;
  virtual const unsigned long scheduleId(void) = 0;
  virtual const unsigned long equityId(void) = 0;
  virtual const unsigned long reportId(void) = 0;

  virtual void loadAccountId(const unsigned long id) = 0;
  virtual void loadTransactionId(const unsigned long id) = 0;
  virtual void loadPayeeId(const unsigned long id) = 0;
  virtual void loadInstitutionId(const unsigned long id) = 0;
  virtual void loadScheduleId(const unsigned long id) = 0;
  virtual void loadEquityId(const unsigned long id) = 0;
  virtual void loadReportId(const unsigned long id) = 0;

  /**
    * This method is used to retrieve the whole set of key/value pairs
    * from the container. It is meant to be used for permanent storage
    * functionality. See MyMoneyKeyValueContainer::pairs() for details.
    *
    * @return QMap<QCString, QString> containing all key/value pairs of
    *         this container.
    */
  virtual QMap<QCString, QString> pairs(void) const = 0;

  /**
    * This method is used to initially store a set of key/value pairs
    * in the container. It is meant to be used for loading functionality
    * from permanent storage. See MyMoneyKeyValueContainer::setPairs()
    * for details
    *
    * @param list const QMap<QCString, QString> containing the set of
    *             key/value pairs to be loaded into the container.
    *
    * @note All existing key/value pairs in the container will be deleted.
    */
  virtual void setPairs(const QMap<QCString, QString>& list) = 0;

  virtual QValueList<MyMoneySchedule> scheduleListEx( int scheduleTypes,
                                              int scheduleOcurrences,
                                              int schedulePaymentTypes,
                                              QDate startDate,
                                              const QCStringList& accounts=QCStringList()) const = 0;

  /**
    * This method is used to retrieve the list of all currencies
    * known to the engine.
    *
    * An exception will be thrown upon erronous situations.
    *
    * @return QValueList of all MyMoneyCurrency objects.
    */
  virtual const QValueList<MyMoneyCurrency> currencyList(void) const = 0;

  /**
    * This method is used to retrieve the list of all reports
    * known to the engine.
    *
    * An exception will be thrown upon erronous situations.
    *
    * @return QValueList of all MyMoneyReport objects.
    */
  virtual const QValueList<MyMoneyReport> reportList( void ) const = 0;
  
};

#endif
