/***************************************************************************
                          mymoneymoney.h
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

#ifndef MYMONEYMONEY_H
#define MYMONEYMONEY_H

#include <qdatastream.h>

// This class represents a money type.
class MyMoneyMoney { // In the future it could do conversion rates etc
private:
  double m_amount;
	
  friend QDataStream &operator<<(QDataStream &, const MyMoneyMoney &);
  friend QDataStream &operator>>(QDataStream &, MyMoneyMoney &);

public:
	MyMoneyMoney();
	MyMoneyMoney(const double amount);
	MyMoneyMoney(const QString& amount);
	~MyMoneyMoney();
	
  void setAmount(const double amount) const { ((MyMoneyMoney*)this)->m_amount = amount; }
  double amount(void) const { return m_amount; }
  bool isZero(void);

  // Copy constructors
  MyMoneyMoney(const MyMoneyMoney&);
  MyMoneyMoney& operator = (const MyMoneyMoney&);
  MyMoneyMoney& operator = (const double&);
  MyMoneyMoney& operator = (const QString&);

  // Some convenience operators
  MyMoneyMoney operator + (const MyMoneyMoney&);
  MyMoneyMoney operator - (const MyMoneyMoney&);
  void operator += (const MyMoneyMoney&);
  void operator -= (const MyMoneyMoney&);
  bool operator == (const MyMoneyMoney&);
  bool operator == (const double&);
  bool operator != (const MyMoneyMoney&);
  bool operator < (const MyMoneyMoney&);
  bool operator > (const MyMoneyMoney&);
  bool operator <= (const MyMoneyMoney&);
  bool operator >= (const MyMoneyMoney&);
};

#endif
