/***************************************************************************
                          mymoneytransactionbase.h  -  description
                             -------------------
    begin                : Thu Jan 10 2002
    copyright            : (C) 2000-2002 by Thomas Baumgart
    email                : Thomas Bamgart <ipwizard@users.sourceforge.net>
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Michael Edwardes <mte@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MYMONEYTRANSACTIONBASE_H
#define MYMONEYTRANSACTIONBASE_H

#include <qstring.h>
#include "mymoneymoney.h"

/**
  * The representation of the members that are common to a standard
  * transaction as well as a splitted part of a transaction.
  *
  * @see MyMoneyTransaction
  * @see MyMoneySplitTransaction
  *
  * @author Thomas Baumgart
  * @version $Id: mymoneytransactionbase.h,v 1.3 2002/01/24 16:54:36 ipwizard Exp $
  *
  * @short Representation of the common values of transactions.
**/

class MyMoneyTransactionBase {
public:
  virtual ~MyMoneyTransactionBase();

  /// @return The text of the memo field
  ///
  /// @sa setMemo
  QString memo(void) const { return m_memo; }

  /// @return The amount of this transaction
  ///
  /// @see setAmount
  MyMoneyMoney amount(void) const { return m_amount; }

  /// @return The text of major category field
  ///
  /// @see setCategoryMajor
  QString categoryMajor(void) const { return m_categoryMajor; }

  /// @return The text of minor category field
  ///
  /// @see setCategoryMinor
  QString categoryMinor(void) const { return m_categoryMinor; }

  /// Set the text of the memo field
  ///
  /// @param val Text to be put into the memo field
  ///
  /// @see memo
  void setMemo(const QString& val);

  /// Set the amount of this transaction
  ///
  /// @param val MyMoneyMoney value to be put in the amount field
  ///
  /// @see amount
  void setAmount(const MyMoneyMoney& val);

  /// Set the text of major category field
  ///
  /// @param major Text to be put into the major category field
  ///
  /// @see categoryMajor
  void setCategoryMajor(const QString& major);

  /// Set the text of minor category field
  ///
  /// @param major Text to be put into the minor category field
  ///
  /// @see categoryMinor
  void setCategoryMinor(const QString& minor);

  /// Set the parent's dirty flag
  ///
  /// @param flag Set the parent's dirty flag to this value. Can be true or false.
  ///
  /// @see MyMoneyTransaction
  /// @see MyMoneySplitTransaction
  virtual void setDirty(const bool flag) = 0;

  /// Check whether this instance is a splitted or non-splitted transaction.
  /// This mehtod must be overriden by derived classes)
  ///
  /// @return true or false
  ///
  /// @see MyMoneyTransaction::isSplit
  /// @see MyMoneySplitTransaction::isSplit
  virtual bool isSplit(void) = 0;

protected:
  MyMoneyTransactionBase(const MyMoneyTransactionBase& right);
  MyMoneyTransactionBase();
  MyMoneyTransactionBase(const QString& memo, const MyMoneyMoney& amount,
                         const QString& categoryMajor, const QString& categoryMinor);
  /// common initialization for copy and assignment constructor
  /// @param right reference to the right side of the assignment
  void init(const MyMoneyTransactionBase& right);

private:
  MyMoneyTransactionBase& operator = (const MyMoneyTransactionBase& right);

protected:
  QString m_memo;               ///< contains the value of the memo field
  MyMoneyMoney m_amount;        ///< contains the value of the amount field
  QString m_categoryMajor;      ///< contains the value of the major category field
  QString m_categoryMinor;      ///< contains the value of the minor category field
};

#endif
