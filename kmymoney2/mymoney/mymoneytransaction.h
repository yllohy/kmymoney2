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

// ----------------------------------------------------------------------------
// QT Includes

#include <qstring.h>
#include <qdatetime.h>
#include <qlist.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyutils.h"
#include "mymoneymoney.h"
#include "mymoneykeyvaluecontainer.h"
#include "mymoneysplit.h"

/**
  * This class represents a transaction within the MyMoneyEngine. A transaction
  * contains none, one or more splits of type MyMoneySplit. They are stored in
  * a QValueList<MyMoneySplit> within this object. A transaction containing only
  * a single split with an amount not equal to 0 is an unbalanced transaction. It
  * is tolerated by the engine, but in general not a good idea as it is financially
  * wrong.
  */
class MyMoneyTransaction : public MyMoneyKeyValueContainer
{
public:
  MyMoneyTransaction();
  MyMoneyTransaction(const QCString id,
                             const MyMoneyTransaction& transaction);
  ~MyMoneyTransaction();

public:
  friend QDataStream &operator<<(QDataStream &, MyMoneyTransaction &);
  friend QDataStream &operator>>(QDataStream &, MyMoneyTransaction &);

  // Simple get operations
  const QDate entryDate(void) const { return m_entryDate; };
  const QDate postDate(void) const { return m_postDate; };
  const QCString id(void) const { return m_id; };
  const QString memo(void) const { return m_memo; };
  const QValueList<MyMoneySplit> splits(void) const { return m_splits; };
  const unsigned int splitCount(void) const { return m_splits.count(); };

  // Simple set operations
  void setPostDate(const QDate& date);
  void setEntryDate(const QDate& date);
  void setMemo(const QString& memo);

  bool operator == (const MyMoneyTransaction&) const;

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
  const MyMoneySplit& split(const QCString& accountId, const bool match = true) const;

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
    * @param id reference to id
    */
  const bool accountReferenced(const QCString& id) const;

  /**
    * This method is used to add a split to the transaction. The split
    * will be assigned an id. The id member must be empty.
    *
    * param @split reference to the split that should be added
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
    * This member contains the transaction id
    */
  QCString m_id;

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

private:
  /**
    * This method returns the next id to be used for a split
    */
  const QCString nextSplitID(void);

  // friend QDataStream &operator<<(QDataStream &, const MyMoneyTransaction &);
  // friend QDataStream &operator>>(QDataStream &, MyMoneyTransaction &);

};
#endif
