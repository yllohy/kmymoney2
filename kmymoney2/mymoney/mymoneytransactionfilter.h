/***************************************************************************
                          mymoneytransactionfilter.h  -  description
                             -------------------
    begin                : Fri Aug 22 2003
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

#ifndef MYMONEYTRANSACTIONFILTER_H
#define MYMONEYTRANSACTIONFILTER_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qstring.h>
#include <qcstring.h>
#include <qdatetime.h>
#include <qmap.h>
#include <qasciidict.h>
#include <qintdict.h>
#include <qregexp.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class IMyMoneyStorage;
#include "../mymoney/mymoneytransaction.h"

/**
  * @author Thomas Baumgart
  */

class MyMoneyTransactionFilter
{
public:
  // Make sure to keep the following enum valus in sync with the values
  // used by the GUI (for KMyMoney in kfindtransactiondlgdecl.ui)
  enum typeOptionE {
    allTypes = 0,
    payments,
    deposits,
    transfers,
    // insert new constants above of this line
    typeOptionCount
  };

  // Make sure to keep the following enum valus in sync with the values
  // used by the GUI (for KMyMoney in kfindtransactiondlgdecl.ui)
  enum stateOptionE {
    allStates = 0,
    notReconciled,
    cleared,
    reconciled,
    frozen,
    // insert new constants above of this line
    stateOptionCount
  };

  // Make sure to keep the following enum valus in sync with the values
  // used by the GUI (for KMyMoney in kfindtransactiondlgdecl.ui)
  enum validityOptionE {
    anyValidity = 0,
    valid,
    invalid,
    // insert new constants above of this line
    validityOptionCount
  };


  /**
    * This is the standard constructor for a transaction filter.
    * It creates the object and calls setReportAllSplits() to
    * report all matching splits as separate entries. Use
    * setReportAllSplits() to override this behaviour.
    */
  MyMoneyTransactionFilter();

  /**
    * This is a convenience constructor to allow construction of
    * a simple account filter. It is basically the same as the
    * following:
    *
    * @code
    * :
    *   MyMoneyTransactionFilter filter;
    *   filter.setReportAllSplits(false);
    *   filter.addAccount(id);
    * :
    * @endcode
    *
    * @param id reference to account id
    */
  MyMoneyTransactionFilter(const QCString& id);

  ~MyMoneyTransactionFilter();

  /**
    * This method is used to clear the filter. All settings will be
    * removed.
    */
  void clear(void);

  /**
    * This method is used to set the regular expression filter to the value specified
    * as parameter @p exp. The following text based fields are searched:
    *
    * - Memo
    * - Payee
    * - Category
    *
    * @param exp The regular expression that must be found in a transaction
    *            before it is included in the result set.
    */
  void setTextFilter(const QRegExp& exp);

  /**
    * This method will add the account with id @p id to the list of matching accounts.
    * If the list is empty, any transaction will match.
    *
    * @param id internal ID of the account
    */
  void addAccount(const QCString& id);

  /**
    * This is a convenience method and behaves exactly like the above
    * method but for a list of id's.
    */
  void addAccount(const QCStringList& ids);

  /**
    * This method will add the category with id @p id to the list of matching categories.
    * If the list is empty, only transaction with a single asset/liability account will match.
    *
    * @param id internal ID of the account
    */
  void addCategory(const QCString& id);

  /**
    * This is a convenience method and behaves exactly like the above
    * method but for a list of id's.
    */
  void addCategory(const QCStringList& ids);

  /**
    * This method sets the date filter to match only transactions with posting dates in
    * the date range specified by @p from and @p to. If @p from equal QDate()
    * all transactions with dates prior to @p to match. If @p to equals QDate()
    * all transactions with posting dates past @p from match. If @p from and @p to
    * are equal QDate() the filter is not activated and all transactions match.
    *
    * @param from from date
    * @param to   to date
    */
  void setDateFilter(const QDate& from, const QDate& to);

  /**
    * This method sets the amount filter to match only transactions with
    * an amount in the range specified by @p from and @p to.
    * If a specific amount should be searched, @p from and @p to should be
    * the same value.
    *
    * @param from smallest value to match
    * @param to   largest value to match
    */
  void setAmountFilter(const MyMoneyMoney& from, const MyMoneyMoney& to);

  /**
    * This method will add the payee with id @p id to the list of matching payees.
    * If the list is empty, any transaction will match.
    *
    * @param id internal id of the payee
    */
  void addPayee(const QCString& id);

  /**
    */
  void addType(const int type);

  /**
    */
  void addValidity(const int type);

  /**
    */
  void addState(const int state);

  /**
    * This method sets the number filter to match only transactions with
    * a number in the range specified by @p from and @p to.
    * If a specific number should be searched, @p from and @p to should be
    * the same value.
    *
    * @param from smallest value to match
    * @param to   largest value to match
    *
    * @note @p from and @p to can contain alphanumeric text
    */
  void setNumberFilter(const QString& from, const QString& to);

  /**
    * This method is used to check a specific transaction against the filter.
    * The transaction will match the whole filter, if all specified filters
    * match. If the filter is cleared using the clear() method, any transaciton
    * matches.
    *
    * @param transaction A transaction
    * @param storage pointer to object of IMyMoneyStorage class
    *
    * @retval true The transaction matches the filter set
    * @retval false The transaction does not match at least of of
    *               the filters in the filter set
    */
  const bool match(const MyMoneyTransaction& transaction, const IMyMoneyStorage* const storage);

  /**
    * This method is used to switch the amount of splits reported
    * by matchingSplits(). If the argument @p report is @p true (the default
    * if no argument specified) then matchingSplits() will return all
    * matching splits of the transaction. If @p report is set to @p false,
    * then only the very first matching split will be returned by
    * matchingSplits().
    *
    * @param report controls the behaviour of matchingsSplits() as explained above.
    */
  void setReportAllSplits(const bool report = true);

  void setConsiderCategory(const bool check = true);

  /**
    * This method returns the id of the matching splits for the filter.
    * If m_reportAllSplits is set to false, then only the very first
    * split will be returned. Use setReportAllSplits() to change the
    * behaviour.
    *
    * @return reference to QCString object containing the id of the
    *         matching split. If multiple split match, only the first
    *         one will be returned.
    *
    * @note an empty id will be returned, if the filter only required
    *       to check the data contained in the MyMoneyTransaction
    *       object (e.g. posting-date, state, etc.).
    *
    * @note The constructors set m_reportAllSplits differently. Please
    *       see the documentation of the constructors MyMoneyTransactionFilter()
    *       and MyMoneyTransactionFilter(const QCString&) for details.
    */
  const QValueList<MyMoneySplit> matchingSplits(void) const;

  /**
    * This method returns the from date set in the filter. If
    * no value has been set up for this filter, then QDate() is
    * returned.
    *
    * @return returns m_fromDate
    */
  const QDate fromDate(void) const { return m_fromDate; };

  /**
    * This method returns the to date set in the filter. If
    * no value has been set up for this filter, then QDate() is
    * returned.
    *
    * @return returns m_toDate
    */
  const QDate toDate(void) const { return m_toDate; };

  /**
    * This method is used to return information about the
    * presence of a specific category in the category filter.
    * The category in question is included in the filter set,
    * if it has been set or no category filter is set.
    *
    * @param cat id of category in question
    * @return true if category is in filter set, false otherwise
    */
  const bool includesCategory( const QCString& cat ) const;

  /**
    * This method is used to return information about the
    * presence of a specific account in the account filter.
    * The account in question is included in the filter set,
    * if it has been set or no account filter is set.
    *
    * @param cat id of account in question
    * @return true if account is in filter set, false otherwise
    */
  const bool includesAccount( const QCString& acc ) const;

private:
  /**
    * This is a conversion tool from MyMoneySplit::reconcileFlagE
    * to MyMoneyTransactionFilter::stateE types
    *
    * @param split reference to split in question
    *
    * @return converted reconcile flag of the split passed as parameter
    */
  const int splitState(const MyMoneySplit& split) const;

  /**
    * This is a conversion tool from MyMoneySplit::action
    * to MyMoneyTransactionFilter::typeE types
    *
    * @param storage pointer to object of IMyMoneyStorage class
    * @param transaction reference to transaction
    * @param split reference to split in question
    *
    * @return converted action of the split passed as parameter
    */
  const int splitType(const IMyMoneyStorage* const storage, const MyMoneyTransaction& t, const MyMoneySplit& split) const;

  /**
    * This method checks if a transaction is valid or not. A transaction
    * is considered valid, if the sum of all splits is zero, invalid otherwise.
    *
    * @param transaction reference to transaction to be checked
    * @retval valid transaction is valid
    * @retval invalid transaction is invalid
    */
  const validityOptionE validTransaction(const MyMoneyTransaction& transaction) const;

private:
  union FilterSet {
    unsigned  allFilter;
    struct {
      unsigned textFilter       : 1;
      unsigned accountFilter    : 1;
      unsigned payeeFilter      : 1;
      unsigned categoryFilter   : 1;
      unsigned nrFilter         : 1;
      unsigned dateFilter       : 1;
      unsigned amountFilter     : 1;
      unsigned typeFilter       : 1;
      unsigned stateFilter      : 1;
      unsigned validityFilter   : 1;
    } singleFilter;
  }                   m_filterSet;
  bool                m_reportAllSplits;
  bool                m_considerCategory;

  QRegExp             m_text;
  QAsciiDict<char>    m_accounts;
  QAsciiDict<char>    m_payees;
  QAsciiDict<char>    m_categories;
  QIntDict<char>      m_states;
  QIntDict<char>      m_types;
  QIntDict<char>      m_validity;
  QString             m_fromNr, m_toNr;
  QDate               m_fromDate, m_toDate;
  MyMoneyMoney        m_fromAmount, m_toAmount;
  QValueList<MyMoneySplit> m_matchingSplits;
};

#endif
