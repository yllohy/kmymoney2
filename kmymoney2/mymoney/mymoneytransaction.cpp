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
	m_file = 0;
  m_nextSplitID = 0;
  m_id =
  m_memo = "";
  m_entryDate = QDate();
  m_postDate = QDate();
}

// MyMoneyTransaction::MyMoneyTransaction(MyMoneyFile *file,
MyMoneyTransaction::MyMoneyTransaction(const QString id,
                                       const MyMoneyTransaction& transaction)
{
  *this = transaction;
//	m_file = file;
  m_id = id;
  m_entryDate = QDate::currentDate();
}

MyMoneyTransaction::~MyMoneyTransaction()
{
}

bool MyMoneyTransaction::operator == (const MyMoneyTransaction& right) const
{
  return ((m_id == right.m_id) &&
      (file() == right.file()) &&
      (m_memo == right.m_memo) &&
      (m_splits == right.m_splits) &&
      (m_entryDate == right.m_entryDate) &&
      (m_postDate == right.m_postDate) );
}

const bool MyMoneyTransaction::accountReferenced(const QString& id) const
{
  QValueList<MyMoneySplit>::ConstIterator it;

  for(it = m_splits.begin(); it != m_splits.end(); ++it) {
    if((*it).accountId() == id)
      return true;
  }
  return false;
}

void MyMoneyTransaction::addSplit(MyMoneySplit split)
{
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
    throw new MYMONEYEXCEPTION("Invalid split id");

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
    throw new MYMONEYEXCEPTION("Invalid split id");
}

const QString MyMoneyTransaction::nextSplitID()
{
  QString id = "S";
  id += QString::number(++m_nextSplitID).rightJustify(SPLIT_ID_SIZE, '0');
  return id;

}

#if 0
QDataStream &operator<<(QDataStream &s, const MyMoneyTransaction &trans)
{
  return s << (Q_UINT32)trans.m_id
    << trans.m_number
    << trans.m_payee
    << trans.m_amount
    << trans.m_date
    << (Q_UINT32)trans.m_method
    << trans.m_categoryMajor
    << trans.m_categoryMinor
    << trans.m_atmBankName
    << trans.m_accountFrom
    << trans.m_accountTo
    << trans.m_memo
    << (Q_INT32)trans.m_state;
  // no need to save m_index as its just an internal counter
}

QDataStream &operator>>(QDataStream &s, MyMoneyTransaction &trans)
{
  s >> (Q_UINT32 &)trans.m_id
    >> trans.m_number
    >> trans.m_payee
    >> trans.m_amount
    >> trans.m_date
    >> (Q_UINT32 &)trans.m_method
    >> trans.m_categoryMajor
    >> trans.m_categoryMinor
    >> trans.m_atmBankName
    >> trans.m_accountFrom
    >> trans.m_accountTo
    >> trans.m_memo
    >> (Q_INT32 &)trans.m_state;
    return s;
}

bool MyMoneyTransaction::readAllData(int version, QDataStream& stream)
{
  // ignore version for now.
  stream >> (Q_UINT32 &)m_id
    >> m_number
    >> m_payee
    >> m_amount
    >> m_date
    >> (Q_UINT32 &)m_method
    >> m_categoryMajor
    >> m_categoryMinor
    >> m_atmBankName
    >> m_accountFrom
    >> m_accountTo
    >> m_memo
    >> (Q_INT32 &)m_state;
    return true;
}
#endif

void MyMoneyTransaction::setPostDate(const QDate& date) { m_postDate = date; }
void MyMoneyTransaction::setMemo(const QString& memo) { m_memo = memo; }

#if 0
MyMoneyTransaction::transactionMethod MyMoneyTransaction::stringToMethod(const char *method)
{
  if ((strcmp(method, "Deposit"))==0)
    return Deposit;
  else if ((strcmp(method, "Cheque"))==0)
    return Cheque;
  else if ((strcmp(method, "Transfer"))==0)
    return Transfer;
  else if ((strcmp(method, "Withdrawal"))==0)
    return Withdrawal;
  else if ((strcmp(method, "ATM"))==0)
    return ATM;
  qDebug("Invalid transaction method '%s'. Use ATM instead.", method);
  return ATM;
}

#endif
