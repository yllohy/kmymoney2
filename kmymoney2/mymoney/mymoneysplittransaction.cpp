/***************************************************************************
                          mymoneysplittransaction.cpp  -  description
                             -------------------
    begin                : Wed Jan 9 2002
    copyright            : (C) 2000-2002 by Michael Edwardes
    email                : mte@users.sourceforge.net
                             Javier Campos Morales <javi_c@users.sourceforge.net>
                             Felix Rodriguez <frodriguez@users.sourceforge.net>
                             John C <thetacoturtle@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "mymoneysplittransaction.h"
#include "mymoneytransaction.h"

MyMoneySplitTransaction::MyMoneySplitTransaction()
{
  m_parent = 0;
}

MyMoneySplitTransaction::MyMoneySplitTransaction(const MyMoneySplitTransaction& right)
  : MyMoneyTransactionBase(right)
{
  m_parent = 0;
}


MyMoneySplitTransaction::MyMoneySplitTransaction(MyMoneyTransaction* parent)
{
  m_parent = parent;
}

MyMoneySplitTransaction::~MyMoneySplitTransaction()
{
}

void MyMoneySplitTransaction::setParent(MyMoneyTransaction* parent)
{
  m_parent = parent;
}

void MyMoneySplitTransaction::setDirty(const bool flag)
{
  if (m_parent)
    m_parent->setDirty(flag);
}
