/***************************************************************************
                          mymoneymoney.h
                             -------------------
    copyright            : (C) 2000-2002 by Michael Edwardes
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
#ifndef _MYMONEYMONEY_H
#define _MYMONEYMONEY_H

#include <stdlib.h>

#ifndef HAVE_ATOLL
#  ifdef HAVE_STRTOLL
#    define atoll(a) strtoll(a, 0, 10)
#  endif
#endif

// So we can save this object
#include <qstring.h>
#include <qdatastream.h>

typedef long long signed64;
typedef unsigned long long unsigned64;

class MyMoneyMoney
{
private:
  signed64 m_64Value;

  friend QDataStream &operator<<(QDataStream &, const MyMoneyMoney &);
  friend QDataStream &operator>>(QDataStream &, MyMoneyMoney &);

public:
  // construction
  MyMoneyMoney();
  MyMoneyMoney( const QString& pszAmountInPence );
  MyMoneyMoney( signed64 AmountInPence );
  MyMoneyMoney( long ldAmountInPence );
  MyMoneyMoney( int iAmountInPence );
  MyMoneyMoney( double dAmountInPence );

  // copy constructor
  MyMoneyMoney( const MyMoneyMoney& AmountInPence );

  signed64 value(void) const { return m_64Value; };
  const QString formatMoney(/*QString locale="C", bool addPrefixPostfix=false*/void) const;

  // assignment
  const MyMoneyMoney& operator=( const MyMoneyMoney& AmountInPence );
  const MyMoneyMoney& operator=( const QString& pszAmountInPence );
  const MyMoneyMoney& operator=( signed64 AmountInPence );
  const MyMoneyMoney& operator=( long ldAmountInPence );
  const MyMoneyMoney& operator=( int iAmountInPence );

  // comparison
  bool operator==( const MyMoneyMoney& AmountInPence ) const;
  bool operator!=( const MyMoneyMoney& AmountInPence ) const;
  bool operator<( const MyMoneyMoney& AmountInPence ) const;
  bool operator>( const MyMoneyMoney& AmountInPence ) const;
  bool operator<=( const MyMoneyMoney& AmountInPence ) const;
  bool operator>=( const MyMoneyMoney& AmountInPence ) const;

  bool operator==( const QString& pszAmountInPence ) const;
  bool operator!=( const QString& pszAmountInPence ) const;
  bool operator<( const QString& pszAmountInPence ) const;
  bool operator>( const QString& pszAmountInPence ) const;
  bool operator<=( const QString& pszAmountInPence ) const;
  bool operator>=( const QString& pszAmountInPence ) const;

  bool operator==( signed64 AmountInPence ) const;
  bool operator!=( signed64 AmountInPence ) const;
  bool operator<( signed64 AmountInPence ) const;
  bool operator>( signed64 AmountInPence ) const;
  bool operator<=( signed64 AmountInPence ) const;
  bool operator>=( signed64 AmountInPence ) const;

  bool operator==( long ldAmountInPence ) const;
  bool operator!=( long ldAmountInPence ) const;
  bool operator<( long ldAmountInPence ) const;
  bool operator>( long ldAmountInPence ) const;
  bool operator<=( long ldAmountInPence ) const;
  bool operator>=( long ldAmountInPence ) const;

  bool operator==( int iAmountInPence ) const;
  bool operator!=( int iAmountInPence ) const;
  bool operator<( int iAmountInPence ) const;
  bool operator>( int iAmountInPence ) const;
  bool operator<=( int iAmountInPence ) const;
  bool operator>=( int iAmountInPence ) const;

  // calculation
  MyMoneyMoney operator+( const MyMoneyMoney&	AmountInPence ) const;
  MyMoneyMoney operator+( const QString& pszAmountInPence ) const;
  MyMoneyMoney operator+( signed64 AmountInPence ) const;
  MyMoneyMoney operator+( long ldAmountInPence ) const;
  MyMoneyMoney operator+( int iAmountInPence ) const;

  MyMoneyMoney operator-( const MyMoneyMoney&	AmountInPence ) const;
  MyMoneyMoney operator-( const QString& pszAmountInPence ) const;
  MyMoneyMoney operator-( signed64 AmountInPence ) const;
  MyMoneyMoney operator-( long ldAmountInPence ) const;
  MyMoneyMoney operator-( int iAmountInPence ) const;
  MyMoneyMoney operator-( ) const;

  MyMoneyMoney operator*( MyMoneyMoney&	AmountInPence ) const;
  MyMoneyMoney operator*( signed64 AmountInPence ) const;
  MyMoneyMoney operator*( long ldAmountInPence ) const;
  MyMoneyMoney operator*( int iAmountInPence ) const;

  // unary operators
  MyMoneyMoney& operator+= ( const MyMoneyMoney&	AmountInPence );
  MyMoneyMoney& operator+=( const QString& pszAmountInPence );
  MyMoneyMoney& operator+=( signed64 AmountInPence );
  MyMoneyMoney& operator+=( long ldAmountInPence );
  MyMoneyMoney& operator+=( int iAmountInPence );

  MyMoneyMoney& operator-= ( const MyMoneyMoney&	AmountInPence );
  MyMoneyMoney& operator-=( const QString& pszAmountInPence );
  MyMoneyMoney& operator-=( signed64 AmountInPence );
  MyMoneyMoney& operator-=( long ldAmountInPence );
  MyMoneyMoney& operator-=( int iAmountInPence );

  // increment and decrement
  MyMoneyMoney& operator++();
  MyMoneyMoney  operator++( int );
  MyMoneyMoney& operator--();
  MyMoneyMoney  operator--( int );
};

//=============================================================================
//
//  Inline functions
//
//=============================================================================

////////////////////////////////////////////////////////////////////////////////
//      Name: MyMoneyMoney
//   Purpose: Constructor - constructs object set to 0.
//   Returns: None
//    Throws: Nothing.
// Arguments: pszAmountInPence - NULL terminated string containing amount in pence
//
////////////////////////////////////////////////////////////////////////////////
inline MyMoneyMoney::MyMoneyMoney()
{
  m_64Value = 0;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: MyMoneyMoney
//   Purpose: Constructor - constructs object from an amount in a NULL terminated
//            string
//   Returns: None
//    Throws: Nothing.
// Arguments: pszAmountInPence - NULL terminated string containing amount in pence
//
////////////////////////////////////////////////////////////////////////////////
inline MyMoneyMoney::MyMoneyMoney(const QString& pszAmountInPence)
{
  m_64Value = atoll( pszAmountInPence );
}

////////////////////////////////////////////////////////////////////////////////
//      Name: MyMoneyMoney
//   Purpose: Constructor - constructs object from an amount in a signed64 value 
//   Returns: None
//    Throws: Nothing.
// Arguments: AmountInPence - signed 64 object containing amount in pence
//
////////////////////////////////////////////////////////////////////////////////
inline MyMoneyMoney::MyMoneyMoney(signed64 AmountInPence)
{
  m_64Value = AmountInPence;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: MyMoneyMoney
//   Purpose: Constructor - constructs object from an amount in a double value
//   Returns: None
//    Throws: Nothing.
// Arguments: dAmountInPence - double object containing amount in pence
//
////////////////////////////////////////////////////////////////////////////////
inline MyMoneyMoney::MyMoneyMoney(const double dAmountInPence)
{
  double adj = dAmountInPence < 0 ? -0.5 : 0.5;
  m_64Value = static_cast<signed64> (dAmountInPence * 100 + adj);
}

////////////////////////////////////////////////////////////////////////////////
//      Name: MyMoneyMoney
//   Purpose: Constructor - constructs object from an amount in a long value
//   Returns: None
//    Throws: Nothing.
// Arguments: ldAmountInPence - long object containing amount in pence
//
////////////////////////////////////////////////////////////////////////////////
inline MyMoneyMoney::MyMoneyMoney(long ldAmountInPence)
{
  m_64Value = static_cast<signed64>(ldAmountInPence);
}

////////////////////////////////////////////////////////////////////////////////
//      Name: MyMoneyMoney
//   Purpose: Constructor - constructs object from an amount in a integer value 
//   Returns: None
//    Throws: Nothing.
// Arguments: iAmountInPence - integer object containing amount in pence
//
////////////////////////////////////////////////////////////////////////////////
inline MyMoneyMoney::MyMoneyMoney(int iAmountInPence)
{
  m_64Value = static_cast<signed64>(iAmountInPence);
}

////////////////////////////////////////////////////////////////////////////////
//      Name: MyMoneyMoney
//   Purpose: Copy Constructor - constructs object from another MyMoneyMoney object
//   Returns: None
//    Throws: Nothing.
// Arguments: AmountInPence - MyMoneyMoney object to be copied
//
////////////////////////////////////////////////////////////////////////////////
inline MyMoneyMoney::MyMoneyMoney(const MyMoneyMoney& AmountInPence)
{
  m_64Value = AmountInPence.m_64Value;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator=
//   Purpose: Assignment operator - modifies object from input MyMoneyMoney object
//   Returns: Const reference to the object
//    Throws: Nothing.
// Arguments: AmountInPence - MyMoneyMoney object to be modified from
//
////////////////////////////////////////////////////////////////////////////////
inline const MyMoneyMoney& MyMoneyMoney::operator=(const MyMoneyMoney& AmountInPence)
{
  m_64Value = AmountInPence.m_64Value;
  return *this;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator=
//   Purpose: Assignment operator - modifies object from input NULL terminated
//            string
//   Returns: Const reference to the object
//    Throws: Nothing.
// Arguments: pszAmountInPence - NULL terminated string that contains amount
//            in pence.
//
////////////////////////////////////////////////////////////////////////////////
inline const MyMoneyMoney& MyMoneyMoney::operator=(const QString& pszAmountInPence)
{
  *this = MyMoneyMoney( pszAmountInPence );
  return *this;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator=
//   Purpose: Assignment operator - modifies object from input signed 64 object
//   Returns: Const reference to the object
//    Throws: Nothing.
// Arguments: AmountInPence - signed 64 object to be modified from
//
////////////////////////////////////////////////////////////////////////////////
inline const MyMoneyMoney& MyMoneyMoney::operator=(signed64 AmountInPence)
{
  m_64Value = AmountInPence;
  return *this;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator=
//   Purpose: Assignment operator - modifies object from input long object
//   Returns: Const reference to the object
//    Throws: Nothing.
// Arguments: ldAmountInPence - long object to be modified from
//
////////////////////////////////////////////////////////////////////////////////
inline const MyMoneyMoney& MyMoneyMoney::operator=(long ldAmountInPence)
{
  m_64Value = static_cast<signed64>(ldAmountInPence);
  return *this;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator=
//   Purpose: Assignment operator - modifies object from input integer object
//   Returns: Const reference to the object
//    Throws: Nothing.
// Arguments: AmountInPence - integer object to be modified from
//
////////////////////////////////////////////////////////////////////////////////
inline const MyMoneyMoney& MyMoneyMoney::operator=(int iAmountInPence)
{
  m_64Value = static_cast<signed64>(iAmountInPence);
  return *this;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator==
//   Purpose: Compare equal operator - compares object with input MyMoneyMoney object
//   Returns: true if equal, otherwise false
//    Throws: Nothing.
// Arguments: AmountInPence - MyMoneyMoney object to be compared with
//
////////////////////////////////////////////////////////////////////////////////
inline bool MyMoneyMoney::operator==(const MyMoneyMoney& AmountInPence) const
{ 
  return ( m_64Value == AmountInPence.m_64Value ) ;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator!=
//   Purpose: Compare not equal operator - compares object with input MyMoneyMoney object
//   Returns: true if not equal, otherwise false
//    Throws: Nothing.
// Arguments: AmountInPence - MyMoneyMoney object to be compared with
//
////////////////////////////////////////////////////////////////////////////////
inline bool MyMoneyMoney::operator!=(const MyMoneyMoney& AmountInPence) const
{ 
  return ( m_64Value != AmountInPence.m_64Value ) ;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator<
//   Purpose: Compare less than operator - compares object with input MyMoneyMoney object
//   Returns: true if object less than input amount
//    Throws: Nothing.
// Arguments: AmountInPence - MyMoneyMoney object to be compared with
//
////////////////////////////////////////////////////////////////////////////////
inline bool MyMoneyMoney::operator<(	const MyMoneyMoney& AmountInPence) const
{ 
  return ( m_64Value < AmountInPence.m_64Value ) ;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator>
//   Purpose: Compare greater than operator - compares object with input MyMoneyMoney
//            object
//   Returns: true if object greater than input amount
//    Throws: Nothing.
// Arguments: AmountInPence - MyMoneyMoney object to be compared with
//
////////////////////////////////////////////////////////////////////////////////
inline bool MyMoneyMoney::operator>(	const MyMoneyMoney& AmountInPence) const
{ 
  return ( m_64Value > AmountInPence.m_64Value ) ;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator<=
//   Purpose: Compare less than equal to operator - compares object with input 
//            MyMoneyMoney object
//   Returns: true if object less than or equal to input amount
//    Throws: Nothing.
// Arguments: AmountInPence - MyMoneyMoney object to be compared with
//
////////////////////////////////////////////////////////////////////////////////
inline bool MyMoneyMoney::operator<=(const MyMoneyMoney& AmountInPence) const
{ 
  return ( m_64Value <= AmountInPence.m_64Value ) ;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator>=
//   Purpose: Compare greater than equal to operator - compares object with input 
//            MyMoneyMoney object
//   Returns: true if object greater than or equal to input amount
//    Throws: Nothing.
// Arguments: AmountInPence - MyMoneyMoney object to be compared with
//
////////////////////////////////////////////////////////////////////////////////
inline bool MyMoneyMoney::operator>=(const MyMoneyMoney& AmountInPence) const
{ 
  return ( m_64Value >= AmountInPence.m_64Value ) ;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator==
//   Purpose: Compare equal operator - compares object with input amount in a 
//            NULL terminated string
//   Returns: true if equal, otherwise false
//    Throws: Nothing.
// Arguments: pszAmountInPence - NULL terminated string that contains amount in 
//            pence
//
////////////////////////////////////////////////////////////////////////////////
inline bool MyMoneyMoney::operator==(const QString& pszAmountInPence) const
{ 
  MyMoneyMoney AmountInPence( pszAmountInPence );
  return ( m_64Value == AmountInPence.m_64Value ) ;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator!=
//   Purpose: Compare not equal operator - compares object with input amount in
//            a NULL terminated string
//   Returns: true if not equal, otherwise false
//    Throws: Nothing.
// Arguments: pszAmountInPence - NULL terminated string that contains amount in
//            pence
//
////////////////////////////////////////////////////////////////////////////////
inline bool MyMoneyMoney::operator!=(const QString& pszAmountInPence) const
{ 
  MyMoneyMoney AmountInPence( pszAmountInPence );
  return ( m_64Value != AmountInPence.m_64Value ) ;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator<
//   Purpose: Compare less than operator - compares object with input amount in
//            a NULL terminated string
//   Returns: true if object less than input amount
//    Throws: Nothing.
// Arguments: pszAmountInPence - NULL terminated string that contains amount in
//            pence
////////////////////////////////////////////////////////////////////////////////
inline bool MyMoneyMoney::operator<(	const QString& pszAmountInPence) const
{ 
  MyMoneyMoney AmountInPence( pszAmountInPence );
  return ( m_64Value < AmountInPence.m_64Value ) ;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator>
//   Purpose: Compare greater than operator - compares object with input amount in
//            a NULL terminated string
//   Returns: true if object greater than input amount
//    Throws: Nothing.
// Arguments: AmountInPence - NULL terminated string that contains the amount in
//            pence
//
////////////////////////////////////////////////////////////////////////////////
inline bool MyMoneyMoney::operator>(	const QString& pszAmountInPence) const
{ 
  MyMoneyMoney AmountInPence( pszAmountInPence );
  return ( m_64Value > AmountInPence.m_64Value ) ;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator<=
//   Purpose: Compare less than equal to operator - compares object with input 
//            amount in a NULL terminated string
//   Returns: true if object less than or equal to input amount
//    Throws: Nothing.
// Arguments: pszAmountInPence - NULL terminated string that contains amount in
//            pence
////////////////////////////////////////////////////////////////////////////////
inline bool MyMoneyMoney::operator<=(const QString& pszAmountInPence) const
{ 
  MyMoneyMoney AmountInPence( pszAmountInPence );
  return ( m_64Value <= AmountInPence.m_64Value ) ;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator>=
//   Purpose: Compare greater than equal to operator - compares object with input 
//            amount in a NULL terminated string
//   Returns: true if object greater than or equal to input amount
//    Throws: Nothing.
// Arguments: AmountInPence - MyMoneyMoney object to be compared with
//
////////////////////////////////////////////////////////////////////////////////
inline bool MyMoneyMoney::operator>=(const QString& pszAmountInPence) const
{ 
  MyMoneyMoney AmountInPence( pszAmountInPence );
  return ( m_64Value >= AmountInPence.m_64Value ) ;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator==
//   Purpose: Compare equal operator - compares object with input signed64 amount
//   Returns: true if equal, otherwise false
//    Throws: Nothing.
// Arguments: AmountInPence - signed64 object to be compared with
//
////////////////////////////////////////////////////////////////////////////////
inline bool MyMoneyMoney::operator==(signed64 AmountInPence) const
{ 
  return ( m_64Value == AmountInPence ) ;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator!=
//   Purpose: Compare not equal operator - compares object with input signed64
//            object
//   Returns: true if not equal, otherwise false
//    Throws: Nothing.
// Arguments: AmountInPence - signed64 object to be compared with
//
////////////////////////////////////////////////////////////////////////////////
inline bool MyMoneyMoney::operator!=(signed64 AmountInPence) const
{ 
  return ( m_64Value != AmountInPence ) ;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator<
//   Purpose: Compare less than operator - compares object with input signed64
//            object
//   Returns: true if object less than input amount
//    Throws: Nothing.
// Arguments: AmountInPence - signed64 object to be compared with
//
////////////////////////////////////////////////////////////////////////////////
inline bool MyMoneyMoney::operator<(	signed64 AmountInPence) const
{ 
  return ( m_64Value < AmountInPence ) ;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator>
//   Purpose: Compare greater than operator - compares object with input signed64 
//            object
//   Returns: true if object greater than input amount
//    Throws: Nothing.
// Arguments: AmountInPence - signed64 object to be compared with
//
////////////////////////////////////////////////////////////////////////////////
inline bool MyMoneyMoney::operator>(	signed64 AmountInPence) const
{ 
  return ( m_64Value > AmountInPence ) ;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator<=
//   Purpose: Compare less than equal to operator - compares object with input 
//            signed64 object
//   Returns: true if object less than or equal to input amount
//    Throws: Nothing.
// Arguments: AmountInPence - signed64 object to be compared with
//
////////////////////////////////////////////////////////////////////////////////
inline bool MyMoneyMoney::operator<=(signed64 AmountInPence) const
{ 
  return ( m_64Value <= AmountInPence ) ;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator>=
//   Purpose: Compare greater than equal to operator - compares object with input 
//            signed64 object
//   Returns: true if object greater than or equal to input amount
//    Throws: Nothing.
// Arguments: AmountInPence - signed64 object to be compared with
//
////////////////////////////////////////////////////////////////////////////////
inline bool MyMoneyMoney::operator>=(signed64 AmountInPence) const
{ 
  return ( m_64Value >= AmountInPence ) ;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator==
//   Purpose: Compare equal operator - compares object with input integer object
//   Returns: true if equal, otherwise false
//    Throws: Nothing.
// Arguments: AmountInPence - integer object to be compared with
//
////////////////////////////////////////////////////////////////////////////////
inline bool MyMoneyMoney::operator==(int iAmountInPence) const
{ 
  return ( m_64Value == static_cast<signed64>(iAmountInPence) ) ;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator!=
//   Purpose: Compare not equal operator - compares object with input integer object
//   Returns: true if not equal, otherwise false
//    Throws: Nothing.
// Arguments: AmountInPence - integer object to be compared with
//
////////////////////////////////////////////////////////////////////////////////
inline bool MyMoneyMoney::operator!=(int iAmountInPence) const
{ 
  return ( m_64Value != static_cast<signed64>(iAmountInPence) ) ;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator<
//   Purpose: Compare less than operator - compares object with input integer
//            object
//   Returns: true if object less than input amount
//    Throws: Nothing.
// Arguments: AmountInPence - integer object to be compared with
//
////////////////////////////////////////////////////////////////////////////////
inline bool MyMoneyMoney::operator<(	int iAmountInPence) const
{ 
  return ( m_64Value < static_cast<signed64>(iAmountInPence) ) ;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator>
//   Purpose: Compare greater than operator - compares object with input integer 
//            object
//   Returns: true if object greater than input amount
//    Throws: Nothing.
// Arguments: AmountInPence - integer object to be compared with
//
////////////////////////////////////////////////////////////////////////////////
inline bool MyMoneyMoney::operator>(	int iAmountInPence) const
{ 
  return ( m_64Value > static_cast<signed64>(iAmountInPence) ) ;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator<=
//   Purpose: Compare less than equal to operator - compares object with input 
//            integer object
//   Returns: true if object less than or equal to input amount
//    Throws: Nothing.
// Arguments: AmountInPence - integer object to be compared with
//
////////////////////////////////////////////////////////////////////////////////
inline bool MyMoneyMoney::operator<=(int iAmountInPence) const
{ 
  return ( m_64Value <= static_cast<signed64>(iAmountInPence) ) ;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator>=
//   Purpose: Compare greater than equal to operator - compares object with input 
//            integer object
//   Returns: true if object greater than or equal to input amount
//    Throws: Nothing.
// Arguments: AmountInPence - integer object to be compared with
//
////////////////////////////////////////////////////////////////////////////////
inline bool MyMoneyMoney::operator>=(int iAmountInPence) const
{ 
  return ( m_64Value >= static_cast<signed64>(iAmountInPence) ) ;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator==
//   Purpose: Compare equal operator - compares object with input long object
//   Returns: true if equal, otherwise false
//    Throws: Nothing.
// Arguments: AmountInPence - long object to be compared with
//
////////////////////////////////////////////////////////////////////////////////
inline bool MyMoneyMoney::operator==(long ldAmountInPence) const
{ 
  return ( m_64Value == static_cast<signed64>(ldAmountInPence) ) ;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator!=
//   Purpose: Compare not equal operator - compares object with input long object
//   Returns: true if not equal, otherwise false
//    Throws: Nothing.
// Arguments: AmountInPence - long object to be compared with
//
////////////////////////////////////////////////////////////////////////////////
inline bool MyMoneyMoney::operator!=(long ldAmountInPence) const
{ 
  return ( m_64Value != static_cast<signed64>(ldAmountInPence) ) ;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator<
//   Purpose: Compare less than operator - compares object with input long
//            object
//   Returns: true if object less than input amount
//    Throws: Nothing.
// Arguments: AmountInPence - long object to be compared with
//
////////////////////////////////////////////////////////////////////////////////
inline bool MyMoneyMoney::operator<(	long ldAmountInPence) const
{ 
  return ( m_64Value < static_cast<signed64>(ldAmountInPence) ) ;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator>
//   Purpose: Compare greater than operator - compares object with input long 
//            object
//   Returns: true if object greater than input amount
//    Throws: Nothing.
// Arguments: AmountInPence - long object to be compared with
//
////////////////////////////////////////////////////////////////////////////////
inline bool MyMoneyMoney::operator>(	long ldAmountInPence) const
{ 
  return ( m_64Value > static_cast<signed64>(ldAmountInPence) ) ;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator<=
//   Purpose: Compare less than equal to operator - compares object with input 
//            long object
//   Returns: true if object less than or equal to input amount
//    Throws: Nothing.
// Arguments: AmountInPence - long object to be compared with
//
////////////////////////////////////////////////////////////////////////////////
inline bool MyMoneyMoney::operator<=(long ldAmountInPence) const
{ 
  return ( m_64Value <= static_cast<signed64>(ldAmountInPence) ) ;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator>=
//   Purpose: Compare greater than equal to operator - compares object with input 
//            long object
//   Returns: true if object greater than or equal to input amount
//    Throws: Nothing.
// Arguments: AmountInPence - long object to be compared with
//
////////////////////////////////////////////////////////////////////////////////
inline bool MyMoneyMoney::operator>=(long ldAmountInPence) const
{ 
  return ( m_64Value >= static_cast<signed64>(ldAmountInPence) ) ;
}
////////////////////////////////////////////////////////////////////////////////
//      Name: operator+
//   Purpose: Addition operator - adds the input amount to the object
//   Returns: The current object
//    Throws: Nothing.
// Arguments: AmountInPence - MyMoneyMoney object to be added
//
////////////////////////////////////////////////////////////////////////////////
inline MyMoneyMoney MyMoneyMoney::operator+(const MyMoneyMoney& AmountInPence) const
{
  MyMoneyMoney result(*this);
  result.m_64Value += AmountInPence.m_64Value;
  return result;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator+
//   Purpose: Addition operator - adds the input amount to the object
//   Returns: The current object
//    Throws: Nothing.
// Arguments: AmountInPence - NULL terminated string that contains an amount in
//            pence to be added
//
////////////////////////////////////////////////////////////////////////////////
inline MyMoneyMoney MyMoneyMoney::operator+(const QString& pszAmountInPence) const
{
  MyMoneyMoney result(*this);
  result += MyMoneyMoney( pszAmountInPence );
  return result;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator+
//   Purpose: Addition operator - adds the input amount to the object
//   Returns: The current object
//    Throws: Nothing.
// Arguments: AmountInPence - signed64 object to be added
//
////////////////////////////////////////////////////////////////////////////////
inline MyMoneyMoney MyMoneyMoney::operator+(signed64 AmountInPence) const
{
  MyMoneyMoney result(*this);
  result.m_64Value += AmountInPence ;
  return result;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator+
//   Purpose: Addition operator - adds the input amount to the object
//   Returns: The current object
//    Throws: Nothing.
// Arguments: AmountInPence - long object to be added
//
////////////////////////////////////////////////////////////////////////////////
inline MyMoneyMoney MyMoneyMoney::operator+(long ldAmountInPence) const
{
  MyMoneyMoney result(*this);
  result.m_64Value += static_cast<signed64>(ldAmountInPence);
  return result;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator+
//   Purpose: Addition operator - adds the input amount to the object
//   Returns: The current object
//    Throws: Nothing.
// Arguments: AmountInPence - int object to be added
//
////////////////////////////////////////////////////////////////////////////////
inline MyMoneyMoney MyMoneyMoney::operator+(int iAmountInPence) const
{
  MyMoneyMoney result(*this);
  result.m_64Value += static_cast<signed64>(iAmountInPence);
  return result;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator-
//   Purpose: Addition operator - subtracts the input amount from the object
//   Returns: The current object
//    Throws: Nothing.
// Arguments: AmountInPence - MyMoneyMoney object to be subtracted
//
////////////////////////////////////////////////////////////////////////////////
inline MyMoneyMoney MyMoneyMoney::operator-(const MyMoneyMoney& AmountInPence) const
{
  MyMoneyMoney result(*this);
  result.m_64Value -= AmountInPence.m_64Value;
  return result;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator-
//   Purpose: Unary operator - returns the negative value from the object
//   Returns: The current object
//    Throws: Nothing.
// Arguments: None
//
////////////////////////////////////////////////////////////////////////////////
inline MyMoneyMoney MyMoneyMoney::operator-() const
{
  MyMoneyMoney result(*this);
  result.m_64Value = -result.m_64Value;
  return result;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator-
//   Purpose: Addition operator - subtracts the input amount from the object
//   Returns: The current object
//    Throws: Nothing.
// Arguments: AmountInPence - NULL terminated string that contains an amount in
//            pence to be subtracted
//
////////////////////////////////////////////////////////////////////////////////
inline MyMoneyMoney MyMoneyMoney::operator-(const QString& pszAmountInPence) const
{
  MyMoneyMoney result(*this);
  result -= MyMoneyMoney( pszAmountInPence );
  return result;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator-
//   Purpose: Addition operator - subtracts the input amount from the object
//   Returns: The current object
//    Throws: Nothing.
// Arguments: AmountInPence - signed64 object to be subtracted
//
////////////////////////////////////////////////////////////////////////////////
inline MyMoneyMoney MyMoneyMoney::operator-(signed64 AmountInPence) const
{
  MyMoneyMoney result(*this);
  result.m_64Value -= AmountInPence ;
  return result;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator-
//   Purpose: Addition operator - subtracts the input amount from the object
//   Returns: The current object
//    Throws: Nothing.
// Arguments: AmountInPence - long object to be subtracted
//
////////////////////////////////////////////////////////////////////////////////
inline MyMoneyMoney MyMoneyMoney::operator-(long ldAmountInPence) const
{
  MyMoneyMoney result(*this);
  result.m_64Value -= static_cast<signed64>(ldAmountInPence);
  return result;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator-
//   Purpose: Addition operator - subtracts the input amount from the object
//   Returns: The current object
//    Throws: Nothing.
// Arguments: AmountInPence - integer object to be subtracted
//
////////////////////////////////////////////////////////////////////////////////
inline MyMoneyMoney MyMoneyMoney::operator-(int iAmountInPence) const
{
  MyMoneyMoney result(*this);
  result.m_64Value -= static_cast<signed64>(iAmountInPence);
  return result;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator*
//   Purpose: Multiplication operator - multiplies the input amount to the object
//   Returns: The current object
//    Throws: Nothing.
// Arguments: AmountInPence - signed64 object to be multiplied
//
////////////////////////////////////////////////////////////////////////////////
inline MyMoneyMoney MyMoneyMoney::operator*(MyMoneyMoney& AmountInPence ) const
{
  MyMoneyMoney result(*this);
  result.m_64Value *= AmountInPence.value();
  return result;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator*
//   Purpose: Multiplication operator - multiplies the input amount to the object
//   Returns: The current object
//    Throws: Nothing.
// Arguments: AmountInPence - signed64 object to be multiplied
//
////////////////////////////////////////////////////////////////////////////////
inline MyMoneyMoney MyMoneyMoney::operator*(signed64 AmountInPence ) const
{
  MyMoneyMoney result(*this);
  result.m_64Value *= AmountInPence;
  return result;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator*
//   Purpose: Multiplication operator - multiplies the input amount to the object
//   Returns: The current object
//    Throws: Nothing.
// Arguments: AmountInPence - long object to be multiplied
//
////////////////////////////////////////////////////////////////////////////////
inline MyMoneyMoney MyMoneyMoney::operator*(long ldAmountInPence ) const
{
  MyMoneyMoney result(*this);
  result.m_64Value *= static_cast<signed64>(ldAmountInPence);
  return result;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator*
//   Purpose: Multiplication operator - multiplies the input amount to the object
//   Returns: The current object
//    Throws: Nothing.
// Arguments: AmountInPence - long object to be multiplied
//
////////////////////////////////////////////////////////////////////////////////
inline MyMoneyMoney MyMoneyMoney::operator*(int iAmountInPence ) const
{
  MyMoneyMoney result(*this);
  result.m_64Value *= static_cast<signed64>(iAmountInPence);
  return result;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator+=
//   Purpose: Addition operator - adds the input amount to the object together
//   Returns: Reference to the current object
//    Throws: Nothing.
// Arguments: AmountInPence - MyMoneyMoney object to be added
//
////////////////////////////////////////////////////////////////////////////////
inline MyMoneyMoney& MyMoneyMoney::operator+=(const MyMoneyMoney& AmountInPence)
{
  m_64Value += AmountInPence.m_64Value;
  return *this;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator+=
//   Purpose: Addition operator - adds the input amount to the object together
//   Returns: Reference to the current object
//    Throws: Nothing.
// Arguments: AmountInPence - NULL terminated string that contains an amount in
//            pence
//
////////////////////////////////////////////////////////////////////////////////
inline MyMoneyMoney& MyMoneyMoney::operator+=(const QString& pszAmountInPence)
{
  *this += MyMoneyMoney( pszAmountInPence );
  return *this;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator+=
//   Purpose: Addition operator - adds the input amount to the object together
//   Returns: Reference to the current object
//    Throws: Nothing.
// Arguments: AmountInPence - signed64 object to be added
//
////////////////////////////////////////////////////////////////////////////////
inline MyMoneyMoney& MyMoneyMoney::operator+=(signed64 AmountInPence)
{
  m_64Value += AmountInPence;
  return *this;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator+=
//   Purpose: Addition operator - adds the input amount to the object together
//   Returns: Reference to the current object
//    Throws: Nothing.
// Arguments: AmountInPence - long object to be added
//
////////////////////////////////////////////////////////////////////////////////
inline MyMoneyMoney& MyMoneyMoney::operator+=(long ldAmountInPence)
{
  m_64Value += static_cast<signed64>(ldAmountInPence);
  return *this;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator+=
//   Purpose: Addition operator - adds the input amount to the object together
//   Returns: Reference to the current object
//    Throws: Nothing.
// Arguments: AmountInPence - integer object to be added
//
////////////////////////////////////////////////////////////////////////////////
inline MyMoneyMoney& MyMoneyMoney::operator+=(int iAmountInPence)
{
  m_64Value += static_cast<signed64>(iAmountInPence);
  return *this;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator-=
//   Purpose: Subtraction operator - subtracts the input amount from the object
//   Returns: Reference to the current object
//    Throws: Nothing.
// Arguments: AmountInPence - MyMoneyMoney object to be subtracted
//
////////////////////////////////////////////////////////////////////////////////
inline MyMoneyMoney& MyMoneyMoney::operator-=(const MyMoneyMoney& AmountInPence)
{
  m_64Value -= AmountInPence.m_64Value;
  return *this;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator-=
//   Purpose: Subtraction operator - subtracts the input amount from the object
//   Returns: The current object
//    Throws: Nothing.
// Arguments: AmountInPence - NULL terminated string that contains an amount in
//            pence
//
////////////////////////////////////////////////////////////////////////////////
inline MyMoneyMoney& MyMoneyMoney::operator-=(const QString& pszAmountInPence)
{
  *this -= MyMoneyMoney( pszAmountInPence );
  return *this;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator-=
//   Purpose: Subtraction operator - subtracts the input amount from the object
//   Returns: Reference to the current object
//    Throws: Nothing.
// Arguments: AmountInPence - signed64 object to be subtracted
//
////////////////////////////////////////////////////////////////////////////////
inline MyMoneyMoney& MyMoneyMoney::operator-=(signed64 AmountInPence)
{
  m_64Value -= AmountInPence;
  return *this;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator-=
//   Purpose: Subtraction operator - subtracts the input amount from the object
//   Returns: Reference to the current object
//    Throws: Nothing.
// Arguments: AmountInPence - long object to be subtracted
//
////////////////////////////////////////////////////////////////////////////////
inline MyMoneyMoney& MyMoneyMoney::operator-=(long ldAmountInPence)
{
  m_64Value -= static_cast<signed64>(ldAmountInPence);
  return *this;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator-=
//   Purpose: Subtraction operator - subtracts the input amount from the object
//   Returns: Reference to the current object
//    Throws: Nothing.
// Arguments: AmountInPence - integer object to be subtracted
//
////////////////////////////////////////////////////////////////////////////////
inline MyMoneyMoney& MyMoneyMoney::operator-=(int iAmountInPence)
{
  m_64Value -= static_cast<signed64>(iAmountInPence);
  return *this;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator++
//   Purpose: Pre-increment operator - adds 1 (pence) to the current object which
//            is then returned
//   Returns: Reference to the current object
//    Throws: Nothing.
// Arguments: None
//
////////////////////////////////////////////////////////////////////////////////
inline MyMoneyMoney& MyMoneyMoney::operator++()
{
  m_64Value += 1;
  return *this;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator++
//   Purpose: Post-increment operator - adds 1 (pence) to the current object before
//            returning object before addition
//   Returns: State of object before addition
//    Throws: Nothing.
// Arguments: int (dummy argument)
//
////////////////////////////////////////////////////////////////////////////////
inline MyMoneyMoney MyMoneyMoney::operator++(int)
{
  MyMoneyMoney current( *this );
  m_64Value += 1;
  return current;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator--
//   Purpose: Pre-deccrement operator - subtracts 1 (pence) from the current object 
//            which is then returned
//   Returns: Reference to the current object
//    Throws: Nothing.
// Arguments: None
//
////////////////////////////////////////////////////////////////////////////////
inline MyMoneyMoney& MyMoneyMoney::operator--()
{
  m_64Value -= 1;
  return *this;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator--
//   Purpose: Post-increment operator - subtracts 1 (pence) from the current object
//            before returning object before addition
//   Returns: State of object before subtraction
//    Throws: Nothing.
// Arguments: int (dummy argument)
//
////////////////////////////////////////////////////////////////////////////////
inline MyMoneyMoney MyMoneyMoney::operator--(int)
{
  MyMoneyMoney current( *this );
  m_64Value -= 1;
  return current;
}

#endif

