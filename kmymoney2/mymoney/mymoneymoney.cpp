/***************************************************************************
                          mymoneymoney.cpp
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes
    email                : mte@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "mymoneymoney.h"

MyMoneyMoney::MyMoneyMoney()
{
  m_amount=0.0;
}

MyMoneyMoney::MyMoneyMoney(const double amount)
{
  m_amount = amount;
}

MyMoneyMoney::~MyMoneyMoney()
{
}

MyMoneyMoney::MyMoneyMoney(const MyMoneyMoney& right)
{
  m_amount = right.m_amount;
}

MyMoneyMoney& MyMoneyMoney::operator = (const MyMoneyMoney& right)
{
  m_amount = right.m_amount;
  return *this;
}

MyMoneyMoney& MyMoneyMoney::operator = (const double& right)
{
  m_amount = right;
  return *this;
}

MyMoneyMoney MyMoneyMoney::operator + (const MyMoneyMoney& right)
{
  MyMoneyMoney money;
  money.m_amount = m_amount + right.m_amount;
  return money;
}

MyMoneyMoney MyMoneyMoney::operator - (const MyMoneyMoney& right)
{
  MyMoneyMoney money;
  money.m_amount = m_amount - right.m_amount;
  return money;
}

void MyMoneyMoney::operator += (const MyMoneyMoney& right)
{
  m_amount += right.m_amount;
}

void MyMoneyMoney::operator -= (const MyMoneyMoney& right)
{
  m_amount -= right.m_amount;
}

bool MyMoneyMoney::operator == (const MyMoneyMoney& right)
{
  return m_amount == right.m_amount;
}

bool MyMoneyMoney::operator == (const double& right)
{
  return m_amount == right;
}

bool MyMoneyMoney::operator != (const MyMoneyMoney& right)
{
  return m_amount != right.m_amount;
}

bool MyMoneyMoney::isZero(void)
{
  return (m_amount==0.0)?true:false;
}

bool MyMoneyMoney::operator < (const MyMoneyMoney& right)
{
  return m_amount < right.m_amount;
}

bool MyMoneyMoney::operator > (const MyMoneyMoney& right)
{
  return m_amount > right.m_amount;
}

bool MyMoneyMoney::operator <= (const MyMoneyMoney& right)
{
  return m_amount <= right.m_amount;
}

bool MyMoneyMoney::operator >= (const MyMoneyMoney& right)
{
  return m_amount >= right.m_amount;
}

QDataStream &operator<<(QDataStream &s, const MyMoneyMoney &money)
{
  return s << money.m_amount;
}

QDataStream &operator>>(QDataStream &s, MyMoneyMoney &money)
{
  return s >> money.m_amount;
}
