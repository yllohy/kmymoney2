/***************************************************************************
                          mymoneyscheduled.h
                             -------------------
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

#ifndef MYMONEYSCHEDULED_H
#define MYMONEYSCHEDULED_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qstringlist.h>
#include <qmap.h>
#include <qdatetime.h>


// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneytransaction.h"

/**
  *@author Michael Edwardes
  */

/**
  * This class represents a schedule. (A series of bills, deposits or
  * transfers).
  *
  * @short A class to represent a schedule.
  * @see MyMoneyScheduled
  */
class MyMoneySchedule {
public:
  /**
    * This enum is used to describe all the possible schedule frequencies.
    * The special entry, OCCUR_ANY, is used to combine all the other types.
    */
  enum occurenceE { OCCUR_ONCE=0, OCCUR_DAILY, OCCUR_WEEKLY, OCCUR_FORTNIGHTLY,
                    OCCUR_EVERYOTHERWEEK, OCCUR_MONTHLY, OCCUR_EVERYFOURWEEKS,
                    OCCUR_EVERYOTHERMONTH, OCCUR_EVERYTHREEMONTHS,
                    OCCUR_TWICEYEARLY, OCCUR_EVERYOTHERYEAR, OCCUR_QUARTERLY,
                    OCCUR_EVERYFOURMONTHS, OCCUR_YEARLY,
                    OCCUR_ANY };

  /**
    * This enum is used to describe the schedule type.
    */
  enum typeE {  TYPE_BILL=15, TYPE_DEPOSIT, TYPE_TRANSFER,
                TYPE_ANY };

  /**
    * This enum is used to describe the schedule's payment type.
    */
  enum paymentTypeE { STYPE_DIRECTDEBIT=19, STYPE_DIRECTDEPOSIT,
                      STYPE_MANUALDEPOSIT, STYPE_OTHER,
                      STYPE_WRITECHEQUE,
                      STYPE_ANY };

  /**
    * Standard constructor
    */
  MyMoneySchedule() {
    // Set up the default values
    m_occurence = OCCUR_ANY;
    m_type = TYPE_ANY;
    m_paymentType = STYPE_ANY;
    m_fixed = false;
    m_willEnd = false;
    m_transactionsRemaining = 0;
    m_autoEnter = false;
    m_startDate = QDate(1900, 1, 1);
    m_lastPayment = QDate(1900, 1, 1);
  }

  /**
    * Constructor for initialising the object.
    *
    * Please note that the optional fields are not set and the transaction
    * MUST be set before it can be used.
    */
  MyMoneySchedule(const QString& name, typeE type, occurenceE occurence, paymentTypeE paymentType,
        QDate startDate, bool willEnd, bool fixed, bool autoEnter) {
    // Set up the default values
    m_occurence = occurence;
    m_type = type;
    m_paymentType = paymentType;
    m_fixed = fixed;
    m_willEnd = willEnd;
    m_transactionsRemaining = 0;
    m_autoEnter = autoEnter;
    m_startDate = startDate;
    m_lastPayment = m_startDate;
  }

  /**
    * Standard destructor
    */
  ~MyMoneySchedule() {
  }

  /**
    * Simple get method that returns the occurence frequency.
    *
    * @return occurenceE The instance frequency.
    */
  occurenceE occurence(void) const { return m_occurence; }

  /**
    * Simple get method that returns the schedule type.
    *
    * @return typeE The instance type.
    */
  typeE type(void) const { return m_type; }

  /**
    * Simple get method that returns the schedule startDate.
    *
    * @return QDate The instance startDate.
    */
  QDate startDate(void) const { return m_startDate; }

  /**
    * Simple get method that returns the schedule paymentType.
    *
    * @return paymentTypeE The instance paymentType.
    */
  paymentTypeE paymentType(void) const { return m_paymentType; }

  /**
    * Simple get method that returns true if the schedule is fixed.
    *
    * @return bool To indicate whether the instance is fixed.
    */
  bool isFixed(void) const { return m_fixed; }

  /**
    * Simple get method that returns true if the schedule will end
    * at some time.
    *
    * @return bool Indicates whether the instance will end.
    */
  bool willEnd(void) const { return m_willEnd; }

  /**
    * Simple get method that returns the number of transactions remaining.
    *
    * @return int The number of transactions remaining for the instance.
    */
  int transactionsRemaining(void) const { return m_transactionsRemaining; }

  /**
    * Simple get method that returns the schedule end date.
    *
    * @return QDate The end date for the instance.
    */
  QDate endDate(void) const { return m_endDate; }

  /**
    * Simple get method that returns true if the transaction should be
    * automatically entered into the register.
    *
    * @return bool Indicates whether the instance will be automatically entered.
    */
  bool autoEnter(void) const { return m_autoEnter; }

  /**
    * Simple get method that returns the transaction data for the schedule.
    *
    * @return MyMoneyTransaction The transaction data for the instance.
    */
  MyMoneyTransaction transaction(void) const { return m_transaction; }

  /**
    * Simple method that returns the schedule id.
    *
    * @return QString The instances id.
    */
  QString id(void) const { return m_id; }

  /**
    * Simple method that returns the schedule last payment.
    *
    * @retun QDate The last payment for the instance.
    */
  QDate lastPayment(void) const { return m_lastPayment; }

  /**
    * Simple method that sets the frequency for the schedule.
    *
    * @param occ The new occurence (frequency).
    * @return none
    */
  void setOccurence(occurenceE occ)
    { m_occurence = occ; }

  /**
    * Simple method that sets the type for the schedule.
    *
    * @param type The new type.
    * @return none
    */
  void setType(typeE type)
    { m_type = type; }

  /**
    * Simple method that sets the start date for the schedule.
    *
    * @param date The new start date.
    * @return none
    */
  void setStartDate(const QDate& date)
    { m_startDate = date; }

  /**
    * Simple method that sets the payment type for the schedule.
    *
    * @param type The new payment type.
    * @return none
    */
  void setPaymentType(paymentTypeE type)
    { m_paymentType = type; }

  /**
    * Simple method to set whether the schedule is fixed or not.
    *
    * @param fixed boolean to indicate whether the instance is fixed.
    * @return none
    */
  void setFixed(bool fixed)
    { m_fixed = fixed; }

  /**
    * Simple method that sets the transaction for the schedule.
    *
    * @param transaction The new transaction.
    * @return none
    */
  void setTransaction(const MyMoneyTransaction& transaction)
    { m_transaction = transaction; }

  /**
    * Simple method to set whether the schedule will end.
    *
    * @param willEnd boolean to indicate whether the instance will end at some time.
    * @return none
    */
  void setWillEnd(bool willEnd)
    { m_willEnd = willEnd; }

  /**
    * Simple method that sets the number of transactions remaining for the schedule.
    *
    * @param type The new type.
    * @return none
    */
  void setTransactionsRemaining(int remaining)
    { m_transactionsRemaining = remaining; }

  /**
    * Simple set method to set the end date for the schedule.
    *
    * @param date The new end date.
    * @return none
    */
  void setEndDate(const QDate& date)
    { m_endDate = date; }

  /**
    * Simple set method to set whether this transaction should be automatically entered into the register.
    *
    * @param autoenter boolean to indicate whether we need to automatically enter the transaction.
    * @return none
    */
  void setAutoEnter(bool autoenter)
    { m_autoEnter = autoenter; }

  /**
    * Simple set method to set the schedule's id.  DO NOT USE, EVER!
    *
    * @param id The instances id.
    * @return none
    */
  void setId(const QString& id)
    { m_id = id; }

  /**
    * Simple set method to set the schedule's last payment.
    *
    * @param date The instances last payment date.
    * @return none
    */
  void setLastPayment(const QDate& date)
    { m_lastPayment = date; }

  /**
    * Validates the schedule instance.
    *
    * Makes sure the paymentType matches the type and that the required fields have been set.
    *
    * @return Boolean True if the instance is ok.
    */
  bool validate(bool id_check=true) const;

  /**
    * Calculates the date of the next payment.
    *
    * The date 1/1/1900 is returned if an error occurs.
    *
    * @return QDate The date the next payment is due
    */
  QDate nextPayment(void) const;

  /**
    * Calculates the dates of the payment over a certain period of time.
    *
    * An empty list is returned for no payments or error.
    *
    * @param startDate The start date for the range calculations
    * @param endDate The end date for the range calculations.
    * @return QList<Qdate> The dates on which the payments are due.
    */
  QValueList<QDate> paymentDates(const QDate& startDate, const QDate& endDate) const;

  /**
    * Helper method to convert the frequency enum.
    *
    * @param none.
    * @return The textual description of the frequency.
  **/
  QString occurenceToString(void) const;

  /**
    * Helper method to convert the payment type enum.
    *
    * @param none.
    * @return The textual description of the payment type.
  **/
  QString paymentMethodToString(void) const;

  /**
    * Returns the instances name
    *
    * @param none.
    * @return The name
  **/
  QString name(void) const { return m_name; }

  /**
    * Changes the instance name
    *
    * @param nm The new name
    * @return none
  **/
  void setName(const QString& nm) { m_name = nm; }    

  /**
    * Helper method to convert the type enum.
    *
    * @param none.
    * @return The textual description of the type.
  **/
  QString typeToString(void) const;

private:
  /// Its occurence
  occurenceE m_occurence;

  /// Its type
  typeE m_type;

  /// The date the schedule commences
  QDate m_startDate;

  /// The payment type
  paymentTypeE m_paymentType;

  /// Can the amount vary
  bool m_fixed;

  /// The, possibly estimated, amount plus all other relevant details
  MyMoneyTransaction m_transaction;

  /// Will the schedule end at some time
  bool m_willEnd;

  /// How many transactions remaining if the schedule does end at a fixed date
  int m_transactionsRemaining;

  /// The last transaction date if the schedule does end at a fixed date
  QDate m_endDate;

  /// Enter the transaction into the register automatically
  bool m_autoEnter;

  /// The internal id.
  QString m_id;

  /// Internal date used for calculations
  QDate m_lastPayment;

  /// The name
  QString m_name;
};


/**
  * This class represents a set of schedules.
  *
  * Each schedule is given a unique id and the set
  * can be queried for the common operations, (overdue etc).
  *
  * The schedules are mapped to the accounts they belong to and all member
  * functions expect a valid account id.
  *
  * Implemeted in a singleton pattern.  Use MyMoneyScheduled::instance() to get
  * the instance.
  *
  * @short A class to represent a set of schedules.
  * @see MyMoneySchedule
  */
  class MyMoneyScheduled {
public:

  /**
    * Get the instance.
    */
  static MyMoneyScheduled* instance();
  
  /**
    * Standard destructor.
    */
  ~MyMoneyScheduled();

  /**
    * Adds a schedule to the collection.
    *
    * Must be a valid schedule otherwise an exception is thrown and an empty
    * id is returned.
    *
    * The account referenced must exist or an exception will be thrown.
    *
    * @param accountId The account reference the schedule is for.
    * @param schedule The new schedule to insert.
    * @return QString The system generated id for the schedule if successful.
    *
    * @see MyMoneySchedule
    * @see MyMoneyException
    */
  QString addSchedule(const QCString& accountId, const MyMoneySchedule& schedule);

  /**
    * Remove a schedule from the collection.
    *
    * An exception is thrown if no schedule in the collection matches the
    * supplied parameter.
    *
    * The account reference supplied must exist or an exception will
    * be thrown.
    *
    * @param accountId The account reference the schedule is for.
    * @param scheduleId The schedule id to be removed.
    * @return none
    *
    * @see MyMoneySchedule
    * @see MyMoneyException
    */
  void removeSchedule(const QCString& accountId, const QString& scheduleId);

  /**
    * Replace a schedule in the collection.
    *
    * An exception is thrown if no schedule in the collection matches
    * the supplied id.
    *
    * The account reference supplied must exist or an exception will
    * be thrown.
    *
    * @param accountId The account reference the schedule is for.
    * @param scheduleId The scheduleId to be replaced.
    * @param schedule The new schedule data.
    * @return QString The schedule id if successful, empty string otherwise.
    *
    * @see MyMoneySchedule
    * @see MyMoneyException
    */
  QString replaceSchedule(const QCString& accountId, const QString& scheduleId, const MyMoneySchedule& schedule);

  /**
    * Retrieve a specific schedule from the collection by its id.
    *
    * Throws an exception if no matcing schedule id is found.
    *
    * The account reference supplied must exist or an exception will
    * be thrown.
    *
    * @param accountId The account reference the schedule is for.
    * @param scheduleId The schedule id to be retrieved.
    * @return MyMoneySchedule
    */
  MyMoneySchedule getSchedule(const QCString& accountId, const QString& scheduleId);
    
 
  /**
    * An overloaded method to retrieve all schedules conforming to the supplied parameters.
    *
    * If no parameters are supplied the whole collection is returned as a list.
    *
    * The account reference supplied must exist or an exception will
    * be thrown.
    *
    * Default sort order is by date.
    *
    * @param accountId The account reference the schedule is for.
    * @param type Restrict the schedule types returned to type.  Default is any type.
    * @param paymentType Restrict the schedule payment types to paymentType. Default
    * is any payment type.
    * @param occurence Restrict the schedule frequencyies returned to occurence.
    * Default is any frequency type.
    *
    * @return QList<QString> A sorted list of schedule id's that match the supplied
    * parameters.
    *
    * @see MyMoneySchedule
    *
    * @todo Implement sorting and add other sort types.
    */
  QStringList getScheduled(const QCString& accountId, const MyMoneySchedule::typeE type=MyMoneySchedule::TYPE_ANY,
    const MyMoneySchedule::paymentTypeE paymentType=MyMoneySchedule::STYPE_ANY,
    const MyMoneySchedule::occurenceE occurence=MyMoneySchedule::OCCUR_ANY);
    
  /**
    * An overloaded method to retrieve all schedules conforming to the supplied parameters.
    *
    * If no parameters are supplied the whole collection is returned as a list.
    *
    * The account reference supplied must exist or an exception will
    * be thrown.
    *
    * Default sort order is by date.
    *
    * @param accountId The account reference the schedule is for.
    * @param startDate Restrict the search between the start and end dates.
    * @param endDate Restrict the search between the start and end dates.
    * @param type Restrict the schedule types returned to type.  Default is any type.
    * @param paymentType Restrict the schedule payment types to paymentType. Default
    * is any payment type.
    * @param occurence Restrict the schedule frequencyies returned to occurence.
    * Default is any frequency type.
    *
    * @return QList<QString> A sorted list of schedule id's that match the supplied
    * parameters.
    *
    * @see MyMoneySchedule
    *
    * @todo Implement sorting and add other sort types.
    */
  QStringList getScheduled(const QCString& accountId, const QDate& startDate, const QDate& endDate,
    const MyMoneySchedule::typeE type=MyMoneySchedule::TYPE_ANY,
    const MyMoneySchedule::paymentTypeE paymentType=MyMoneySchedule::STYPE_ANY,
    const MyMoneySchedule::occurenceE occurence=MyMoneySchedule::OCCUR_ANY);

  /**
    * A method to retrieve all schedules that are overdue and conform to the supplied
    * parameters.
    *
    * The account reference supplied must exist or an exception will
    * be thrown.
    *
    * If no parameters are supplied the whole collection is returned as
    * a list.
    *
    * Default sort order is by date.
    *
    * @param accountId The account reference the schedule is for.
    * @param type Restrict the schedule types returned to type.  Default is any type.
    * @param paymentType Restrict the schedule payment types to paymentType. Default
    * is any payment type.
    * @param occurence Restrict the schedule frequencyies returned to occurence.
    * Default is any frequency type.
    *
    * @return QList<QString> A sorted list of schedule id's that match the supplied
    * parameters.
    *
    * @see MyMoneySchedule
    *
    * @todo Implement sorting and add other sort types.
    */
  QStringList getOverdue(const QCString& accountId, const MyMoneySchedule::typeE type=MyMoneySchedule::TYPE_ANY,
    const MyMoneySchedule::paymentTypeE paymentType=MyMoneySchedule::STYPE_ANY,
    const MyMoneySchedule::occurenceE occurence=MyMoneySchedule::OCCUR_ANY);
      
  /**
    * Convenience function to see if any schedules are overdue.
    *
    * Can restrict the check by using the parameters.
    *
    * The account reference supplied must exist or an exception will
    * be thrown.
    *
    * @param accountId The account reference the schedule is for.
    * @param type Restrict the schedule types returned to type.  Default is any type.
    * @param paymentType Restrict the schedule payment types to paymentType. Default
    * is any payment type.
    *
    * @param boolean True if any overdue schedules exist for the supplied parameters.
    *
    * @see MyMoneySchedule
    */
  bool anyOverdue(const QCString& accountId, const MyMoneySchedule::typeE type=MyMoneySchedule::TYPE_ANY,
    const MyMoneySchedule::occurenceE occurence=MyMoneySchedule::OCCUR_ANY,
    const MyMoneySchedule::paymentTypeE paymentType=MyMoneySchedule::STYPE_ANY);
    
  /**
    * Convenience function to see if any schedules exist.
    *
    * The account reference supplied must exist or an exception will
    * be thrown.
    *
    * Can restrict the check by using the parameters.
    *
    * @param accountId The account reference the schedule is for.
    * @param type Restrict the schedule types returned to type.  Default is any type.
    * @param paymentType Restrict the schedule payment types to paymentType. Default
    * is any payment type.
    *
    * @param boolean True if any schedules exist for the supplied parameters.
    *
    * @see MyMoneySchedule
    */
  bool anyScheduled(const QCString& accountId, const MyMoneySchedule::typeE type=MyMoneySchedule::TYPE_ANY,
    const MyMoneySchedule::occurenceE occurence=MyMoneySchedule::OCCUR_ANY,
    const MyMoneySchedule::paymentTypeE paymentType=MyMoneySchedule::STYPE_ANY);

  /**
    * Returns the number of schedules scheduled for the account.
    *
    *
    * @param accountId The account reference the schedule is for.
    * @return int The number of schedules.
  **/
  unsigned int count(const QCString& accountId);

private:
  /// The instance
  static MyMoneyScheduled *m_instance;

  /// The account to schedule dictionary
  QMap<QCString, QMap<QString, MyMoneySchedule> > m_accountsScheduled;

  /// The next id available for the schedule, used for all.
  unsigned int m_nextId;

  /**
    * Standard constructor.
    */
  MyMoneyScheduled();
};

#endif
