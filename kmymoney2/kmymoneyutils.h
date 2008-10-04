/***************************************************************************
                          kmymoneyutils.h  -  description
                             -------------------
    begin                : Wed Feb 5 2003
    copyright            : (C) 2000-2003 by Michael Edwardes
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

#ifndef KMYMONEYUTILS_H
#define KMYMONEYUTILS_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qcolor.h>
#include <qfont.h>

// ----------------------------------------------------------------------------
// KDE Headers

#include <kguiitem.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/mymoneyaccount.h>
#include <kmymoney/mymoneysecurity.h>
#include <kmymoney/mymoneyscheduled.h>
#include <kmymoney/mymoneytransaction.h>

/**
  * @author Thomas Baumgart
  */

class KMyMoneyUtils
{
public:
  /**
    * This enum is used to describe the bits of an account type filter mask.
    * Each bit is used to define a specific account class. Multiple classes
    * can be specified by OR'ing multiple entries. The special entry @p last
    * marks the left most bit in the mask and is used by scanners of this
    * bitmask to determine the end of processing.
    */
  enum categoryTypeE {
    none =       0x00,          ///< no account class selected
    liability =  0x01,          ///< liability accounts selected
    asset =      0x02,          ///< asset accounts selected
    expense =    0x04,          ///< expense accounts selected
    income =     0x08,          ///< income accounts selected
    equity =     0x10,          ///< equity accounts selected
    last =       0x20           ///< the leftmost bit in the mask
  };

  enum transactionTypeE {
    /**
      * Unknown transaction type (e.g. used for a transaction with only
      * a single split)
      */
    Unknown,

    /**
      * A 'normal' transaction is one that consists out two splits: one
      * referencing an income/expense account, the other referencing
      * an asset/liability account.
      */
    Normal,

    /**
      * A transfer denotes a transaction consisting of two splits.
      * Both of the splits reference an asset/liability
      * account.
      */
    Transfer,

    /**
      * Whenever a transaction consists of more than 2 splits,
      * it is treated as 'split transaction'.
      */
    SplitTransaction,

    /**
      * This transaction denotes a specific transaction where
      * a loan account is involved. Ususally, a special dialog
      * is used to modify this transaction.
      */
    LoanPayment,

    /**
      * This transaction denotes a specific transaction where
      * an investment is involved. Ususally, a special dialog
      * is used to modify this transaction.
      */
    InvestmentTransaction
  };

  enum EnterScheduleResultCodeE {
    Cancel = 0,    // cancel the operation
    Enter,         // enter the schedule
    Skip,          // skip the schedule
    Ignore         // ignore the schedule
  };

  static const int maxHomePageItems = 5;

  KMyMoneyUtils();
  ~KMyMoneyUtils();

  /**
    * This method is used to convert the internal representation of
    * an account type into a human readable format
    *
    * @param accountType numerical representation of the account type.
    *                    For possible values, see MyMoneyAccount::accountTypeE
    * @return QString representing the human readable form translated according to the language cataglogue
    *
    * @sa MyMoneyAccount::accountTypeToString()
    */
  static const QString accountTypeToString(const MyMoneyAccount::accountTypeE accountType);

  /**
    * This method is used to convert an account type from it's
    * string form to the internal used numeric value.
    *
    * @param type reference to a QString containing the string to convert
    * @return accountTypeE containing the internal used numeric value. For possible
    *         values see MyMoneyAccount::accountTypeE
    */
  static MyMoneyAccount::accountTypeE stringToAccountType(const QString& type);

  /**
    * This method is used to convert a security type from it's
    * string form to the internal used numeric value.
    *
    * @param txt reference to a QString containing the string to convert
    * @return eSECURITYTYPE containing the internal used numeric value. For possible
    *         values see MyMoneySecurity::eSECURITYTYPE
    */
  static MyMoneySecurity::eSECURITYTYPE stringToSecurity(const QString& txt);

  /**
    * This method is used to convert the internal representation of
    * an security type into a human readable format
    *
    * @param securityType enumerated representation of the security type.
    *                     For possible values, see MyMoneySecurity::eSECURITYTYPE
    * @return QString representing the human readable form translated according to the language cataglogue
    *
    * @sa MyMoneySecurity::securityTypeToString()
    */
  static const QString securityTypeToString(const MyMoneySecurity::eSECURITYTYPE securityType);

  /**
    * This method is used to convert the occurence type from it's
    * internal representation into a human readable format.
    *
    * @param occurence numerical representation of the MyMoneySchedule
    *                  occurence type
    *
    * @return QString representing the human readable format translated according to the language cataglogue
    *
    * @sa MyMoneySchedule::occurenceToString()
    *
    * @deprecated Use i18n(MyMoneySchedule::occurenceToString(occurence)) instead
    */
  static const QString occurenceToString(const MyMoneySchedule::occurenceE occurence);

  /**
    * This method is used to convert the occurence type from the
    * human readable form into it's internal representation.
    *
    * @param text reference to QString representing the human readable format
    * @return numerical representation of the occurence
    */
  static MyMoneySchedule::occurenceE stringToOccurence(const QString& text);

  /**
    * This method is used to convert the payment type from it's
    * internal representation into a human readable format.
    *
    * @param paymentType numerical representation of the MyMoneySchedule
    *                  payment type
    *
    * @return QString representing the human readable format translated according to the language cataglogue
    *
    * @sa MyMoneySchedule::paymentMethodToString()
    */
  static const QString paymentMethodToString(MyMoneySchedule::paymentTypeE paymentType);

  /**
    * This method is used to convert the schedule weekend option from it's
    * internal representation into a human readable format.
    *
    * @param weekendOption numerical representation of the MyMoneySchedule
    *                  weekend option
    *
    * @return QString representing the human readable format translated according to the language cataglogue
    *
    * @sa MyMoneySchedule::weekendOptionToString()
    */
  static const QString weekendOptionToString(MyMoneySchedule::weekendOptionE weekendOption);

  /**
    * This method is used to convert the schedule type from it's
    * internal representation into a human readable format.
    *
    * @param type numerical representation of the MyMoneySchedule
    *                  schedule type
    *
    * @return QString representing the human readable format translated according to the language cataglogue
    *
    * @sa MyMoneySchedule::scheduleTypeToString()
    */
  static const QString scheduleTypeToString(MyMoneySchedule::typeE type);

  /**
    * This method is used to convert a numeric index of an item
    * represented on the home page into it's string form.
    *
    * @param idx numeric index of item
    *
    * @return QString with text of this item
    */
  static const QString homePageItemToString(const int idx);

  /**
    * This method is used to convert the name of a home page item
    * to it's internal numerical representation
    *
    * @param txt QString reference of the items name
    *
    * @retval 0 @p txt is unknown
    * @retval >0 numeric value for @p txt
    */
  static int stringToHomePageItem(const QString& txt);

  /**
    * Retrieve a KDE KGuiItem for the new schedule button.
    *
    * @return The KGuiItem that can be used to display the icon and text
    */
  static KGuiItem scheduleNewGuiItem(void);

  /**
    * Retrieve a KDE KGuiItem for the account filter button
    *
    * @return The KGuiItem that can be used to display the icon and text
    */
  static KGuiItem accountsFilterGuiItem(void);

  /**
    * This method adds the file extension passed as argument @p extension
    * to the end of the file name passed as argument @p name if it is not present.
    * If @p name contains an extension it will be removed.
    *
    * @param name filename to be checked
    * @param extension extension to be added (w/o the dot)
    *
    * @retval true if @p name was changed
    * @retval false if @p name remained unchanged
    */
  static bool appendCorrectFileExt(QString& name, const QString& extension);

  static QPixmap billScheduleIcon(int size);
  static QPixmap depositScheduleIcon(int size);
  static QPixmap transferScheduleIcon(int size);
  static QPixmap scheduleIcon(int size);

  /**
    * This method is used to convert a MyMoneySchedule occurence period into
    * the frequency used in the MyMoneyFinancialCalculator.
    *
    * @param occurence MyMoneySchedule::occurenceE type occurence of a payment
    * @return int reflecting the payment frequency in days
    */
  static int occurenceToFrequency(const MyMoneySchedule::occurenceE occurence);

  /**
    * Check that internal MyMoney engine constants use the same
    * values as the KDE constants.
    */
  static void checkConstants(void);

  static QString variableCSS(void);

  /**
    * This method searches a KDE specific resource and applies country and
    * language settings during the search. Therefore, the parameter @p filename must contain
    * the characters '%1' which gets replaced with the language/country values.
    *
    * The search is performed in the following order (stopped immediately if a file was found):
    * - @c \%1 is replaced with <tt>_\<country\>.\<language\></tt>
    * - @c \%1 is replaced with <tt>_\<language\></tt>
    * - @c \%1 is replaced with <tt>_\<country\></tt>
    * - @c \%1 is replaced with the empty string
    *
    * @c \<country\> and @c \<language\> denote the respective KDE settings.
    *
    * Example: The KDE settings for country is Spain (es) and language is set
    * to Galician (gl). The code for looking up a file looks like this:
    *
    * @code
    *
    *  :
    *  QString fname = KMyMoneyUtils::findResource("appdata", "html/home%1.html")
    *  :
    *
    * @endcode
    *
    * The method calls KStandardDirs::findResource() with the following values for the
    * parameter @p filename:
    *
    * - <tt>html/home_es.gl.html</tt>
    * - <tt>html/home_gl.html</tt>
    * - <tt>html/home_es.html</tt>
    * - <tt>html/home.html</tt>
    *
    * @note See KStandardDirs::findResource() for details on the parameters
    */
  static QString findResource(const char* type, const QString& filename);

  /**
    * This method returns the split referencing a stock account if
    * one exists in the transaction passed as @p t. If none is present
    * in @p t, an empty MyMoneySplit() object will be returned.
    *
    * @param t transaction to be checked for a stock account
    * @return MyMoneySplit object referencing a stock account or an
    *         empty MyMoneySplit object.
    */
  static const MyMoneySplit stockSplit(const MyMoneyTransaction& t);

  /**
    * This method analyses the splits of a transaction and returns
    * the type of transaction. Possible values are defined by the
    * KMyMoneyUtils::transactionTypeE enum.
    *
    * @param t const reference to the transaction
    *
    * @return KMyMoneyUtils::transactionTypeE value of the action
    */
  static transactionTypeE transactionType(const MyMoneyTransaction& t);

  /**
    * This method modifies a scheduled loan transaction such that all
    * references to automatic calculated values are resolved to actual values.
    *
    * @param schedule const reference to the schedule the transaction is based on
    * @param transaction reference to the transaction to be checked and modified
    * @param balances QMap of (account-id,balance) pairs to be used as current balance
    *                 for the calculation of interest. If map is empty, the engine
    *                 will be interrogated for current balances.
    */
  static void calculateAutoLoan(const MyMoneySchedule& schedule, MyMoneyTransaction& transaction, const QMap<QCString, MyMoneyMoney>& balances);

  /**
    * Return next check number for account @a acc.
    */
  static QString nextCheckNumber(const MyMoneyAccount& acc);

  /**
    * Returns the text representing the reconcile flag. If @a text is @p true
    * then the full text will be returned otherwise a short form (usually one character).
    */
  static QString reconcileStateToString(MyMoneySplit::reconcileFlagE flag, bool text = false);

  /**
   * Returns the transaction for @a schedule. In case of a loan payment the
   * transaction will be modified by calculateAutoLoan().
   * The ID of the transaction as well as the entryDate will be reset.
   *
   * @returns adjusted transaction
   */
  static MyMoneyTransaction scheduledTransaction(const MyMoneySchedule& schedule);

  /**
   * This method tries to figure out the category to be used for fees and interest
   * from previous transactions in the given @a investmentAccount and returns the
   * ids of those categories in @a feesId and @a interestId. The last used category
   * will be returned.
   */
  static void previouslyUsedCategories(const QCString& investmentAccount, QCString& feesId, QCString& interestId);

};

#endif
