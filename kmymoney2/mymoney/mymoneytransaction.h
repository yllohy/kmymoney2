/***************************************************************************
                          mymoneytransaction.h
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes
                           (C) 2002 by Thomas Baumgart
    email                : mte@users.sourceforge.net
                           ipwizard@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MYMONEYTRANSACTION_H
#define MYMONEYTRANSACTION_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// ----------------------------------------------------------------------------
// QT Includes

#include <qstring.h>
#include <qdatetime.h>
#include <qptrlist.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyutils.h"
#include "mymoneymoney.h"
#include "mymoneykeyvaluecontainer.h"
#include "mymoneysplit.h"
#include <kmymoney/export.h>

/**
  * This class represents a transaction within the MyMoneyEngine. A transaction
  * contains none, one or more splits of type MyMoneySplit. They are stored in
  * a QValueList<MyMoneySplit> within this object. A transaction containing only
  * a single split with an amount not equal to 0 is an unbalanced transaction. It
  * is tolerated by the engine, but in general not a good idea as it is financially
  * wrong.
  */
class KMYMONEY_EXPORT MyMoneyTransaction : public MyMoneyObject, public MyMoneyKeyValueContainer
{
public:
  MyMoneyTransaction();
  MyMoneyTransaction(const QCString id,
                             const MyMoneyTransaction& transaction);
  /**
    * @param node reference to QDomNode
    * @param forceId see MyMoneyObject(const QDomElement&, const bool)
    */
  MyMoneyTransaction(const QDomElement& node, const bool forceId = true);
  ~MyMoneyTransaction();

public:
  friend QDataStream &operator<<(QDataStream &, MyMoneyTransaction &);
  friend QDataStream &operator>>(QDataStream &, MyMoneyTransaction &);

  // Simple get operations
  const QDate& entryDate(void) const { return m_entryDate; };
  const QDate& postDate(void) const { return m_postDate; };
  const QString& memo(void) const { return m_memo; };
  const QValueList<MyMoneySplit>& splits(void) const { return m_splits; };
  QValueList<MyMoneySplit>& splits(void) { return m_splits; };
  const unsigned int splitCount(void) const { return m_splits.count(); };
  const QCString& commodity(void) const { return m_commodity; };
  const QString& bankID(void) const /*__attribute__ ((deprecated))*/ { return m_bankID; };

  // Simple set operations
  void setPostDate(const QDate& date);
  void setEntryDate(const QDate& date);
  void setMemo(const QString& memo);
  void setCommodity(const QCString& commodityId) { m_commodity = commodityId; };
  void setBankID(const QString& bankID) /*__attribute__ ((deprecated))*/ { m_bankID = bankID; };

  bool operator == (const MyMoneyTransaction&) const;
  inline bool operator != (const MyMoneyTransaction& r) const { return !(*this == r); };
  bool operator< (const MyMoneyTransaction& r) const { return postDate() < r.postDate(); };
  bool operator<= (const MyMoneyTransaction& r) const { return postDate() <= r.postDate(); };
  bool operator> (const MyMoneyTransaction& r) const { return postDate() > r.postDate(); };

  /**
    * This method is used to extract a split for a given accountId
    * from a transaction. A parameter controls, whether the accountId
    * should match or not. In case of 'not match', the first not-matching
    * split is returned.
    *
    * @param accountId the account to look for
    * @param match if true, the account Id must match
    *              if false, the account Id must not match
    *
    * @return reference to split within the transaction is returned
    */
  const MyMoneySplit& splitByAccount(const QCString& accountId, const bool match = true) const;

  /**
    * This method is essentially the same as the previous method, except that
    * takes a list of accounts instead of just one.
    *
    * @param accountIds the list of accounts to look for
    * @param match if true, the account Id must match
    *              if false, the account Id must not match
    *
    * @return reference to split within the transaction is returned
    */
  const MyMoneySplit& splitByAccount(const QCStringList& accountIds, const bool match = true) const;

  /**
    * This method is used to extract a split from a transaction.
    *
    * @param splitId the split to look for
    *
    * @return reference to split within the transaction is returned
    */
  const MyMoneySplit& splitById(const QCString& splitId) const;

  /**
    * This method is used to extract a split for a given payeeId
    * from a transaction.
    *
    * @param payeeId the payee to look for
    *
    * @return reference to split within the transaction is returned
    */
  const MyMoneySplit& splitByPayee(const QCString& payeeId) const;

  /**
    * This method is used to check if the given account is used
    * in any of the splits of this transation
    *
    * @param id account id that should be checked for usage
    */
  const bool accountReferenced(const QCString& id) const;

  /**
    * This method is used to add a split to the transaction. The split
    * will be assigned an id. The id member must be empty.
    *
    * @param split reference to the split that should be added
    *
    */
  void addSplit(MyMoneySplit& split);

  /**
    * This method is used to modify a split in a transaction
    */
  void modifySplit(MyMoneySplit& split);

  /**
    * This method is used to remove a split from a transaction
    */
  void removeSplit(const MyMoneySplit& split);

  /**
    * This method is used to remove all splits from a transaction
    */
  void removeSplits(void);

  /**
    * This method is used to return the sum of all splits of this transaction
    *
    * @return MyMoneyMoney value of sum of all splits
    */
  const MyMoneyMoney splitSum(void) const;

  /**
    * This method returns information if the transaction
    * contains information of a loan payment or not.
    * Loan payment transactions have at least one
    * split that is identified with a MyMoneySplit::action() of type
    * MyMoneySplit::ActionAmortization.
    *
    * @retval false transaction is no loan payment transaction
    * @retval true  transaction is a loan payment transaction
    *
    * @note Upon internal failures, the return value @p false will be used.
    */
  const bool isLoanPayment(void) const;

  /**
    * This method is used to check if two transactions are identical.
    * Identical transactions have:
    *
    * - the same number of splits
    * - reference the same accounts
    * - have the same values in the splits
    * - have a postDate wihtin 3 days
    *
    * @param transaction reference to the transaction to be checked
    *                    against this transaction
    * @retval true transactions are identical
    * @retval false transactions are not identical
    */
  const bool isDuplicate(const MyMoneyTransaction& transaction) const;

  /**
    * This static method returns the id which will be assigned to the
    * first split added to a transaction. This ID can be used to figure
    * out the split that references the account through which a transaction
    * was entered.
    *
    * @return QCString with ID of the first split of transactions
    */
  static const QCString firstSplitID(void);

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
  virtual bool hasReferenceTo(const QCString& id) const;

  /**
    * Checks whether any split contains an autocalc split.
    *
    * @retval true at least one split has an autocalc value
    * @retval false all splits have fixed values
    */
  bool hasAutoCalcSplit(void) const;

private:
  static const int SPLIT_ID_SIZE = 4;

  /**
    * This member contains the date when the transaction was entered
    * into the engine
    */
  QDate m_entryDate;

  /**
    * This member contains the date the transaction was posted
    */
  QDate m_postDate;

  /**
    * This member keeps the memo text associated with this transaction
    */
  QString m_memo;

  /**
    * This member contains the splits for this transaction
    */
  QValueList<MyMoneySplit> m_splits;

  /**
    * This member keeps the unique numbers of splits within this
    * transaction. Upon creation of a MyMoneyTransaction object this
    * value will be set to 1.
    */
  unsigned int m_nextSplitID;

  /**
    * This member keeps the base commodity (e.g. currency) for this transaction
    */
  QCString  m_commodity;

  /**
    * This member keeps the bank's unique ID for the transaction, so we can
    * avoid duplicates.  This is only used for electronic statement downloads.
    *
    * Note this is now deprecated!  Bank ID's should be set on splits, not transactions.
    */
  QString m_bankID;

private:
  /**
    * This method returns the next id to be used for a split
    */
  const QCString nextSplitID(void);

  /**
    * This module implements an algorithm used by P.J. Weinberger
    * for fast hashing. Source: COMPILERS by Alfred V. Aho,
    * pages 435-437.
    *
    * It converts the string passed in @p txt into a non-unique
    * unsigned long integer value.
    *
    * @param txt the text to be hashed
    * @return non-unique hash value of the text @p txt
    */
  const unsigned long hash(const QString& txt) const;
};
#endif
