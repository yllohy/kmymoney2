/***************************************************************************
                          mymoneytransactionbase.cpp  -  description
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

#include "mymoneytransactionbase.h"

MyMoneyTransactionBase::MyMoneyTransactionBase()
{
}

MyMoneyTransactionBase::~MyMoneyTransactionBase()
{
}

MyMoneyTransactionBase::MyMoneyTransactionBase(const MyMoneyTransactionBase& right)
{
  m_memo = right.m_memo;
  m_amount = right.m_amount;
  m_categoryMajor = right.m_categoryMajor;
  m_categoryMinor = right.m_categoryMinor;
}

MyMoneyTransactionBase::MyMoneyTransactionBase(const QString& memo, const MyMoneyMoney& amount, const QString& categoryMajor, const QString& categoryMinor)
{
  m_memo = memo;
  m_amount = amount;
  m_categoryMajor = categoryMajor;
  m_categoryMinor = categoryMinor;
}

MyMoneyTransactionBase& MyMoneyTransactionBase::operator = (const MyMoneyTransactionBase& right)
{
  qDebug("MyMoneyTransactionBase = operator");
  m_memo = right.m_memo;
  m_amount = right.m_amount;
  m_categoryMajor = right.m_categoryMajor;
  m_categoryMinor = right.m_categoryMinor;
  return *this;
}

void MyMoneyTransactionBase::setMemo(const QString& val)
{
  m_memo = val;
  setDirty(true);
}

void MyMoneyTransactionBase::setAmount(const MyMoneyMoney& val)
{
  m_amount = val;
  setDirty(true);
}

void MyMoneyTransactionBase::setCategoryMajor(const QString& major)
{
  m_categoryMajor = major;
  setDirty(true);
}

void MyMoneyTransactionBase::setCategoryMinor(const QString& minor)
{
  m_categoryMinor = minor;
  setDirty(true);
}
