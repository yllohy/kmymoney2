/***************************************************************************
                          mymoneytransaction.cpp
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes,
                               2002 by Thomas Baumgart
    email                : mte@users.sourceforge.net,
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

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneytransaction.h"

MyMoneyTransaction::MyMoneyTransaction()
{
  m_nextSplitID = 1;
  m_id =
  m_memo = "";
  m_entryDate = QDate();
  m_postDate = QDate();
}

// MyMoneyTransaction::MyMoneyTransaction(MyMoneyFile *file,
MyMoneyTransaction::MyMoneyTransaction(const QCString id,
                                       const MyMoneyTransaction& transaction)
{
  *this = transaction;
  m_id = id;
  if(m_entryDate == QDate())
    m_entryDate = QDate::currentDate();
}

MyMoneyTransaction::~MyMoneyTransaction()
{
}

bool MyMoneyTransaction::operator == (const MyMoneyTransaction& right) const
{
  return ((m_id == right.m_id) &&
      (m_memo == right.m_memo) &&
      (m_splits == right.m_splits) &&
      (m_entryDate == right.m_entryDate) &&
      (m_postDate == right.m_postDate) );
}

const bool MyMoneyTransaction::accountReferenced(const QCString& id) const
{
  QValueList<MyMoneySplit>::ConstIterator it;

  for(it = m_splits.begin(); it != m_splits.end(); ++it) {
    if((*it).accountId() == id)
      return true;
  }
  return false;
}

void MyMoneyTransaction::addSplit(MyMoneySplit& split)
{
  if(split.id() != "")
    throw new MYMONEYEXCEPTION("Cannot add split with assigned id (" + split.id() + ")");

  split.setId(nextSplitID());
  m_splits.append(split);
}

void MyMoneyTransaction::modifySplit(MyMoneySplit& split)
{
  QValueList<MyMoneySplit>::Iterator it;

  for(it = m_splits.begin(); it != m_splits.end(); ++it) {
    if(split.id() == (*it).id()) {
      *it = split;
      break;
    }
  }
  if(it == m_splits.end())
    throw new MYMONEYEXCEPTION("Invalid split id '" + split.id() + "'");

  split = *it;
}

void MyMoneyTransaction::removeSplit(const MyMoneySplit& split)
{
  QValueList<MyMoneySplit>::Iterator it;

  for(it = m_splits.begin(); it != m_splits.end(); ++it) {
    if(split.id() == (*it).id()) {
      m_splits.remove(it);
      break;
    }
  }
  if(it == m_splits.end())
    throw new MYMONEYEXCEPTION("Invalid split id '" + split.id() + "'");
}

void MyMoneyTransaction::removeSplits(void)
{
  m_splits.clear();
}

const MyMoneySplit& MyMoneyTransaction::splitByPayee(const QCString& payeeId) const
{
  QValueList<MyMoneySplit>::ConstIterator it;

  for(it = m_splits.begin(); it != m_splits.end(); ++it) {
    if((*it).payeeId() == payeeId)
      return *it;
  }
  throw new MYMONEYEXCEPTION(QString("Split not found for payee") + QString(payeeId));
}

const MyMoneySplit& MyMoneyTransaction::splitByAccount(const QCString& accountId, const bool match) const
{
  QValueList<MyMoneySplit>::ConstIterator it;

  for(it = m_splits.begin(); it != m_splits.end(); ++it) {
    if(match == true && (*it).accountId() == accountId)
      return *it;
    if(match == false && (*it).accountId() != accountId)
      return *it;
  }
  throw new MYMONEYEXCEPTION(QString("Split not found for account") + QString(accountId));
}

const MyMoneySplit& MyMoneyTransaction::splitById(const QCString& splitId) const
{
  QValueList<MyMoneySplit>::ConstIterator it;

  for(it = m_splits.begin(); it != m_splits.end(); ++it) {
    if((*it).id() == splitId)
      return *it;
  }
  throw new MYMONEYEXCEPTION(QString("Split not found for id ") + QString(splitId));
}

const QCString MyMoneyTransaction::nextSplitID()
{
  QCString id;
  id = "S" + id.setNum(m_nextSplitID++).rightJustify(SPLIT_ID_SIZE, '0');
  return id;

}

const MyMoneyMoney MyMoneyTransaction::splitSum(void) const
{
  MyMoneyMoney result(0);
  QValueList<MyMoneySplit>::ConstIterator it;

  for(it = m_splits.begin(); it != m_splits.end(); ++it) {
    result += (*it).value();
  }
  return result;
}

void MyMoneyTransaction::setPostDate(const QDate& date) { m_postDate = date; }
void MyMoneyTransaction::setEntryDate(const QDate& date) { m_entryDate = date; }
void MyMoneyTransaction::setMemo(const QString& memo) { m_memo = memo; }

const bool MyMoneyTransaction::isLoanPayment(void) const
{
  try {
    QValueList<MyMoneySplit>::ConstIterator it;

    for(it = m_splits.begin(); it != m_splits.end(); ++it) {
      if((*it).isAmortizationSplit())
        return true;
    }
  } catch (MyMoneyException *e) {
    delete e;
  }
  return false;
}
