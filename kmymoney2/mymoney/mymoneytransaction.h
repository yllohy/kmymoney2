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
#include "mymoneyfile.h"
#include "mymoneysplit.h"

class MyMoneyFile;

class MyMoneyTransaction {
public:
	MyMoneyTransaction();
  // MyMoneyTransaction(MyMoneyFile *file, const QString id,
  MyMoneyTransaction(const QString id,
                             const MyMoneyTransaction& transaction);
	~MyMoneyTransaction();

public:
  friend QDataStream &operator<<(QDataStream &, MyMoneyTransaction &);
  friend QDataStream &operator>>(QDataStream &, MyMoneyTransaction &);

  // Simple get operations
  const QDate entryDate(void) const { return m_entryDate; };
  const QDate postDate(void) const { return m_postDate; };
  const QString id(void) const { return m_id; };
  const QString memo(void) const { return m_memo; };
  const QValueList<MyMoneySplit> splits(void) const { return m_splits; };
  const unsigned int splitCount(void) const { return m_splits.count(); };

  // Simple set operations
  void setPostDate(const QDate& date);
  void setMemo(const QString& memo);

  bool operator == (const MyMoneyTransaction&) const;

  /**
    * This method is used to check if the given account is used
    * in any of the splits of this transation
    *
    * @param id reference to id
    */
  const bool accountReferenced(const QString& id) const;

  /// Returns a pointer to the file this transaction belongs to
  /// @return pointer to MyMoneyFile
  MyMoneyFile *file(void) const { return m_file; }

  /**
    * This method is used to add a split to the transaction
    */
  void addSplit(MyMoneySplit split);

  /**
    * This method is used to modify a split in a transaction
    */
  void modifySplit(MyMoneySplit& split);

  /**
    * This method is used to remove a split from a transaction
    */
  void removeSplit(const MyMoneySplit& split);

private:
  static const int SPLIT_ID_SIZE = 4;

  /**
    * This member points back to the file that this transaction belongs to
    */
  MyMoneyFile *m_file;

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
  QString m_id;

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
    * transaction
    */
  unsigned int m_nextSplitID;

private:
  const QString nextSplitID(void);

  // friend QDataStream &operator<<(QDataStream &, const MyMoneyTransaction &);
  // friend QDataStream &operator>>(QDataStream &, MyMoneyTransaction &);

};
#endif
