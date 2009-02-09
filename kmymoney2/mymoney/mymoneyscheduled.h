/***************************************************************************
                          mymoneyscheduled.h
                             -------------------
    copyright            : (C) 2000-2002 by Michael Edwardes
                           (C) 2007 by Thomas Baumgart
    email                : mte@users.sourceforge.net
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// ----------------------------------------------------------------------------
// QT Includes

#include <qstringlist.h>
#include <qmap.h>
#include <qdatetime.h>


// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneytransaction.h"
#include "mymoneyaccount.h"
#include <kmymoney/export.h>
#include <kmymoney/mymoneyobject.h>

/**
  * @author Michael Edwardes
  */

/**
  * This class represents a schedule. (A series of bills, deposits or
  * transfers).
  *
  * @short A class to represent a schedule.
  * @see MyMoneyScheduled
  */
class KMYMONEY_EXPORT MyMoneySchedule : public MyMoneyObject
{
public:
  /**
    * This enum is used to describe all the possible schedule frequencies.
    * The special entry, OCCUR_ANY, is used to combine all the other types.
    */
  enum occurenceE { OCCUR_ANY=0, OCCUR_ONCE=1, OCCUR_DAILY=2, OCCUR_WEEKLY=4, OCCUR_FORTNIGHTLY=8,
                    OCCUR_EVERYOTHERWEEK=16,
                    OCCUR_EVERYHALFMONTH=18,
                    OCCUR_EVERYTHREEWEEKS=20,
                    OCCUR_EVERYTHIRTYDAYS=30,
                    OCCUR_MONTHLY=32, OCCUR_EVERYFOURWEEKS=64,
                    OCCUR_EVERYEIGHTWEEKS=126,
                    OCCUR_EVERYOTHERMONTH=128, OCCUR_EVERYTHREEMONTHS=256,
                    OCCUR_TWICEYEARLY=1024, OCCUR_EVERYOTHERYEAR=2048, OCCUR_QUARTERLY=4096,
                    OCCUR_EVERYFOURMONTHS=8192, OCCUR_YEARLY=16384
                     };

  /**
    * This enum is used to describe the schedule type.
    */
  enum typeE {  TYPE_ANY=0, TYPE_BILL=1, TYPE_DEPOSIT=2, TYPE_TRANSFER=4, TYPE_LOANPAYMENT=5 };

  /**
    * This enum is used to describe the schedule's payment type.
    */
  enum paymentTypeE { STYPE_ANY=0, STYPE_DIRECTDEBIT=1, STYPE_DIRECTDEPOSIT=2,
                      STYPE_MANUALDEPOSIT=4, STYPE_OTHER=8,
                      STYPE_WRITECHEQUE=16,
                      STYPE_STANDINGORDER=32,
                      STYPE_BANKTRANSFER=64 };

  /**
    * This enum is used by the auto-commit functionality.
    *
    * Depending upon the value of m_weekdayOption the schedule can
    * be entered on a different date
  **/
  enum weekendOptionE { MoveFriday=0, MoveMonday=1, MoveNothing=2 };

  /**
    * Standard constructor
    */
  MyMoneySchedule();

  /**
    * Constructor for initialising the object.
    *
    * Please note that the optional fields are not set and the transaction
    * MUST be set before it can be used.
    *
    * @a startDate is not used anymore and internally set to QDate()
    */
  MyMoneySchedule(const QString& name, typeE type, occurenceE occurence, int occurenceMultiplier,
          paymentTypeE paymentType, const QDate& startDate, const QDate& endDate, bool fixed, bool autoEnter);

  MyMoneySchedule(const QDomElement& node);

  MyMoneySchedule(const QString& id, const MyMoneySchedule& right);

  /**
    * Standard destructor
    */
  ~MyMoneySchedule() {}

  /**
    * Simple get method that returns the occurence frequency.
    *
    * @return occurenceE The instance frequency.
    */
  occurenceE occurence(void) const { return m_occurence; }

  /**
    * Simple get method that returns the occurence period
    * multiplier and occurence
    *
    * @return occurenceE The instance period 
    *
    */
  occurenceE occurencePeriod(void) const { return m_occurence; }

  /**
    * Simple get method that returns the occurence period multiplier.
    *
    * @return int The frequency multiplier
    */
  int occurenceMultiplier(void) const { return m_occurenceMultiplier; }

  /**
    * Simple get method that returns the schedule type.
    *
    * @return typeE The instance type.
    */
  typeE type(void) const { return m_type; }

  /**
    * Simple get method that returns the schedule startDate. If
    * the schedule has been executed once, the date of the first
    * execution is returned. Otherwise, the next due date is
    * returned.
    *
    * @return reference to QDate containing the start date.
    */
  const QDate& startDate(void) const;

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
  bool willEnd(void) const { return m_endDate.isValid(); }

  /**
    * Simple get method that returns the number of transactions remaining.
    *
    * @return int The number of transactions remaining for the instance.
    */
  int transactionsRemaining(void) const;

  /**
    * Simple get method that returns the schedule end date.
    *
    * @return QDate The end date for the instance.
    */
  const QDate& endDate(void) const { return m_endDate; }

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
  const MyMoneyTransaction& transaction(void) const { return m_transaction; }

  /**
    * Simple method that returns the schedules last payment. If the
    * schedule has never been executed, QDate() will be returned.
    *
    * @return QDate The last payment for the schedule.
    */
  const QDate& lastPayment(void) const { return m_lastPayment; }

  /**
    * Simple method that returns the next due date for the schedule.
    *
    * @return reference to QDate containing the next due date.
    *
    * @note The date returned can represent a value that is past
    *       a possible end of the schedule. Make sure to consider
    *       the return value of isFinished() when using the value returned.
    */
  const QDate& nextDueDate(void) const;

  /**
    * This method adjusts returns the next due date adjusted
    * according to the rules specified by the schedule's weekend option.
    *
    * @return QDate containing the adjusted next due date. If the
    *         schedule is finished (@sa isFinished()) then the method
    *         returns an invalid QDate.
    *
    * @sa weekendOption()
    */
  QDate adjustedNextDueDate(void) const;

  /**
    * Get the weekendOption that determines how the schedule check code
    * will enter transactions that occur on a weekend.
    *
    * This not used by MyMoneySchedule but by the support code.
  **/
  weekendOptionE weekendOption(void) const { return m_weekendOption; }

  /**
    * Simple method that sets the frequency for the schedule.
    *
    * @param occ The new occurence (frequency).
    * @return none
    */
  void setOccurence(occurenceE occ);

  /**
    * Simple method that sets the schedule period 
    *
    * @param occ The new occurence period (frequency)
    * @return none
    */
  void setOccurencePeriod(occurenceE occ);

  /**
    * Simple method that sets the frequency multiplier for the schedule.
    *
    * @param occmultiplier The new occurence (frequency) multiplier.
    * @return none
    */
  void setOccurenceMultiplier(int occmultiplier);

  /**
    * Simple method that sets the type for the schedule.
    *
    * @param type The new type.
    * @return none
    */
  void setType(typeE type);

  /**
    * Simple method that sets the start date for the schedule.
    *
    * @param date The new start date.
    * @return none
    */
  void setStartDate(const QDate& date);

  /**
    * Simple method that sets the payment type for the schedule.
    *
    * @param type The new payment type.
    * @return none
    */
  void setPaymentType(paymentTypeE type);

  /**
    * Simple method to set whether the schedule is fixed or not.
    *
    * @param fixed boolean to indicate whether the instance is fixed.
    * @return none
    */
  void setFixed(bool fixed);

  /**
    * Simple method that sets the transaction for the schedule.
    * The transaction must have a valid postDate set, otherwise
    * it will not be accepted.
    *
    * @param transaction The new transaction.
    * @return none
    */
  void setTransaction(const MyMoneyTransaction& transaction);

  /**
    * Simple set method to set the end date for the schedule.
    *
    * @param date The new end date.
    * @return none
    */
  void setEndDate(const QDate& date);

  /**
    * Simple set method to set whether this transaction should be automatically
    * entered into the journal whenever it is due.
    *
    * @param autoenter boolean to indicate whether we need to automatically
    *                  enter the transaction.
    * @return none
    */
  void setAutoEnter(bool autoenter);

  /**
    * Simple set method to set the schedule's next payment date.
    *
    * @param date The next payment date.
    * @return none
    */
  void setNextDueDate(const QDate& date);

  /**
    * Simple set method to set the schedule's last payment. If
    * this method is called for the first time on the object,
    * the @a m_startDate member will be set to @a date as well.
    *
    * This method should be called whenever a schedule is entered or skipped.
    *
    * @param date The last payment date.
    * @return none
    */
  void setLastPayment(const QDate& date);

  /**
    * Set the weekendOption that determines how the schedule check code
    * will enter transactions that occur on a weekend. The following values
    * are valid:
    *
    * - MoveNothing: don't modify date
    * - MoveFriday: modify the date to the previous friday
    * - MoveMonday: modify the date to the following monday
    *
    * If an invalid option is given, the option is set to MoveNothing.
    *
    * @param option See list in description
    * @return none
    *
    * @note This not used by MyMoneySchedule but by the support code.
    **/
  void setWeekendOption(const weekendOptionE option);

  /**
    * Validates the schedule instance.
    *
    * Makes sure the paymentType matches the type and that the required
    * fields have been set.
    *
    * @param id_check if @p true, the method will check for an empty id.
    *                 if @p false, this check is skipped. Default is @p true.
    *
    * @return If this method returns, all checks are passed. Otherwise,
    *         it will throw a MyMoneyException object.
    *
    * @exception MyMoneyException with detailed error information is thrown
    *            in case of failure of any check.
    */
  void validate(bool id_check=true) const;

  /**
    * Calculates the date of the next payment.
    *
    * @param refDate the reference date from which the next payment
    *                date will be calculated (defaults to current date)
    *
    * @return QDate The date the next payment is due. This date is
    *         always past @a refDate.  In case of an error or
    *         if there is no more payment then an empty/invalid QDate()
    *         will be returned.
    */
  QDate nextPayment(const QDate& refDate = QDate::currentDate()) const;

  /**
    * Calculates the dates of the payment over a certain period of time.
    *
    * An empty list is returned for no payments or error.
    *
    * @param startDate The start date for the range calculations
    * @param endDate The end date for the range calculations.
    * @return QValueList<QDate> The dates on which the payments are due.
    */
  QValueList<QDate> paymentDates(const QDate& startDate, const QDate& endDate) const;

  /**
    * Returns the instances name
    *
    * @return The name
    */
  const QString& name(void) const { return m_name; }

  /**
    * Changes the instance name
    *
    * @param nm The new name
    * @return none
    */
  void setName(const QString& nm);

  bool operator ==(const MyMoneySchedule& right) const;
  bool operator !=(const MyMoneySchedule& right) const { return ! operator==(right); }

  bool operator <(const MyMoneySchedule& right) const;

  MyMoneyAccount account(int cnt = 1) const;
  MyMoneyAccount transferAccount(void) const { return account(2); };
  QDate dateAfter(int transactions) const;

  bool isOverdue() const;
  bool isFinished() const;
  bool hasRecordedPayment(const QDate&) const;
  void recordPayment(const QDate&);
  QValueList<QDate> recordedPayments(void) const { return m_recordedPayments; }

  void writeXML(QDomDocument& document, QDomElement& parent) const;

  /**
    * This method checks if a reference to the given object exists. It returns,
    * a @p true if the object is referencing the one requested by the
    * parameter @p id. If it does not, this method returns @p false.
    *
    * @param id id of the object to be checked for references
    * @retval true This object references object with id @p id.
    * @retval false This object does not reference the object with id @p id.
    */
  virtual bool hasReferenceTo(const QString& id) const;

  /**
   * Returns the human-readable format of Schedule's occurence
   *
   * @return QString representing the human readable format
   */
  QString occurenceToString() const;

  /**
   * This method is used to convert the occurence type from it's
   * internal representation into a human readable format.
   *
   * @param type numerical representation of the MyMoneySchedule
   *                  occurence type
   *
   * @return QString representing the human readable format
   */
  static QString occurenceToString(occurenceE type);

  /**
   * This method is used to convert a multiplier and base occurence type
   * from it's internal representation into a human readable format.
   * When multiplier * occurence is equivalent to a simple occurence
   * the method returns the same as occurenceToString of the simple occurence
   *
   * @param mult occurence multiplier
   * @param type occurence period 
   *
   * @return QString representing the human readable format
   */
  static QString occurenceToString(int mult, occurenceE type);

  /**
   * This method is used to convert the payment type from it's
   * internal representation into a human readable format.
   *
   * @param paymentType numerical representation of the MyMoneySchedule
   *                  payment type
   *
   * @return QString representing the human readable format
   */
  static QString paymentMethodToString(MyMoneySchedule::paymentTypeE paymentType);

  /**
   * This method is used to convert the schedule weekend option from it's
   * internal representation into a human readable format.
   *
   * @param weekendOption numerical representation of the MyMoneySchedule
   *                  weekend option
   *
   * @return QString representing the human readable format
   */
  static QString weekendOptionToString(MyMoneySchedule::weekendOptionE weekendOption);

  /**
   * This method is used to convert the schedule type from it's
   * internal representation into a human readable format.
   *
   * @param type numerical representation of the MyMoneySchedule
   *                  schedule type
   *
   * @return QString representing the human readable format
   */
  static QString scheduleTypeToString(MyMoneySchedule::typeE type);

  int variation(void) const;
  void setVariation(int var);

 /**
  *
  * Convert an occurence to the maximum number of events possible during a single
  * calendar year.
  * A fortnight is treated as 15 days.
  *
  * @param occurence  The occurence
  *
  * @return int  Number of days between events
  */
  static int eventsPerYear(MyMoneySchedule::occurenceE occurence);

 /**
  *
  * Convert an occurence to the number of days between events
  * Treats a month as 30 days.
  * Treats a fortnight as 15 days.
  *
  * @param occurence  The occurence
  *
  * @return int  Number of days between events
  */
  static int daysBetweenEvents(MyMoneySchedule::occurenceE occurence);

  /**
    * Helper method to convert simple occurence to compound occurence + multiplier
    *
    * @param multiplier Returned by reference.  Adjusted multiplier
    * @param occurence Returned by reference.  Occurence type
    */
  static void simpleToCompoundOccurence(int& multiplier,occurenceE& occurence);

  /**
    * Helper method to convert compound occurence + multiplier to simple occurence
    *
    * @param multiplier Returned by reference.  Adjusted multiplier
    * @param occurence Returned by reference.  Occurence type
    */
  static void compoundToSimpleOccurence(int& multiplier,occurenceE& occurence);

private:
  /**
    * This method forces the day of the passed @p date to
    * be the day of the start date of this schedule kept
    * in m_startDate. It is internally used when calculating
    * the payment dates over several periods.
    *
    * @param date reference to QDate object to be checked and adjusted
    */
  void fixDate(QDate& date) const;

  /**
    * Simple method that sets the transaction for the schedule.
    * The transaction must have a valid postDate set, otherwise
    * it will not be accepted. This test is bypassed, if @a noDateCheck
    * is set to true
    *
    * @param transaction The new transaction.
    * @param noDateCheck if @a true, the date check is bypassed
    * @return none
    */
  void setTransaction(const MyMoneyTransaction& transaction, bool noDateCheck);

  /**
    * This method adds a Half Month to the given Date.
    * This is used for OCCUR_EVERYHALFMONTH occurences.
    * The addition uses the following rules:
    * Day 1-13: add 15 days
    * Day 14: add 15 days (except February: the last day of the month)
    * Day 15: last day of the month
    * Day 16-29 (not last day in February): subtract 15 days and add 1 month
    * 30 and last day: 15th of next month
    *
    * This calculation pairs days 1 to 12 with 16 to 27.
    * Day 15 is paired with the last day of every month.
    * Repeated addition has issues in the following cases:
    * - Days 13 to 14 are paired with 28 to 29 until addition hits the last day of February
    *   after which the (15,last) pair will be used.
    * - Addition from Day 30 leads immediately to the (15th,last) day pair.
    *
    * @param date The date
    */
  QDate addHalfMonth( QDate date ) const;

private:
  /// Its occurence
  occurenceE m_occurence;

  /// Its occurence multiplier
  int m_occurenceMultiplier;

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

  /// The last transaction date if the schedule does end at a fixed date
  QDate m_endDate;

  /// Enter the transaction into the register automatically
  bool m_autoEnter;

  /// Internal date used for calculations
  QDate m_lastPayment;

  /// The name
  QString m_name;

  /// The recorded payments
  QValueList<QDate> m_recordedPayments;

  /// The weekday option
  weekendOptionE m_weekendOption;
};
#endif
