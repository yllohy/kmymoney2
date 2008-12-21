/***************************************************************************
                          reportaccount.h
                             -------------------
    begin                : Sat May 22 2004
    copyright            : (C) 2004-2005 by Ace Jones
    email                : <ace.j@hotpop.com>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef REPORTACCOUNT_H
#define REPORTACCOUNT_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qstringlist.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes
#include "../mymoney/mymoneyaccount.h"

namespace reports {

/**
  * This is a MyMoneyAccount as viewed from the reporting engine.
  *
  * All reporting methods should use ReportAccount INSTEAD OF
  * MyMoneyAccount at all times.
  *
  * The primary functionality this provides is a full chain of account
  * hierarchy that is easy to traverse.  It's needed because the PivotTable
  * grid needs to store and sort by the full account hierarchy, while still
  * having access to the account itself for currency conversion.
  *
  * In addition, several other convenience functions are provided that may
  * be worth moving into MyMoneyAccount at some point.
  *
  * @author Ace Jones
  *
  * @short
**/
class ReportAccount: public MyMoneyAccount
{
private:
  QStringList m_nameHierarchy;

public:
  /**
    * Default constructor
    *
    * Needed to allow this object to be stored in a QMap.
    */
  ReportAccount( void );

  /**
    * Copy constructor
    *
    * Needed to allow this object to be stored in a QMap.
    */
  ReportAccount( const ReportAccount& );

  /**
    * Regular constructor
    *
    * @param accountid Account which this account descriptor should be based off of
    */
  ReportAccount( const QString& accountid );

  /**
    * Regular constructor
    *
    * @param accountid Account which this account descriptor should be based off of
    */
  ReportAccount( const MyMoneyAccount& accountid );

  /**
    * @param right The object to compare against
    * @return bool True if this account's fully-qualified hierarchy name
    * is less than that of the given qccount
    */
  bool operator<( const ReportAccount& right ) const;

  /**
    * Returns the price of this account's underlying currency on the indicated date,
    * translated into the account's deep currency
    *
    * There are three different currencies in play with a single Account:
    *   - The underlying currency: What currency the account itself is denominated in
    *   - The deep currency: The underlying currency's own underlying currency.  This
    *      is only a factor if the underlying currency of this account IS NOT a
    *      currency itself, but is some other kind of security.  In that case, the
    *      underlying security has its own currency.  The deep currency is the
    *      currency of the underlying security.  On the other hand, if the account
    *      has a currency itself, then the deep currency == the underlying currency,
    *      and this function will return 1.0.
    *   - The base currency: The base currency of the user's overall file
    *
    * @param date The date in question
    * @return MyMoneyMoney The value of the account's currency on that date
    */
  MyMoneyMoney deepCurrencyPrice( const QDate& date ) const;

  /**
    * Returns the price of this account's deep currency on the indicated date,
    * translated into the base currency
    *
    * @param date The date in question
    * @return MyMoneyMoney The value of the account's currency on that date
    */
  MyMoneyMoney baseCurrencyPrice( const QDate& date ) const;

  /**
    * Fetch the trading symbol of this account's deep currency
    *
    * @return  The account's currency trading currency object
    */
  MyMoneySecurity currency( void ) const;

  /**
    * Determine if this account's deep currency is different from the file's
    * base currency
    *
    * @return bool True if this account is in a foreign currency
    */
  bool isForeignCurrency( void ) const;

  /**
    * The name of only this account.  No matter how deep the hierarchy, this
    * method only returns the last name in the list, which is the engine name]
    * of this account.
    *
    * @return QString The account's name
    */
  QString name( void ) const;

  /**
    * The entire hierarchy of this account descriptor
    * This is similiar to debugName(), however debugName() is not guaranteed
    * to always look pretty, while fullName() is.  So if the user is ever
    * going to see the results, use fullName().
    *
    * @return QString The account's full hierarchy
    */
  QString fullName( void ) const;

  /**
    * The entire hierarchy of this account descriptor, suitable for displaying
    * in debugging output
    *
    * @return QString The account's full hierarchy (suitable for debugging)
    */
  QString debugName( void ) const;

  /**
    * Whether this account is a 'top level' parent account.  This means that
    * it's parent is an account class, like asset, liability, expense or income
    *
    * @return bool True if this account is a top level parent account
    */
  /*inline*/ bool isTopLevel( void ) const;

  /**
    * Returns the name of the top level parent account
    *
    * (See isTopLevel for a definition of 'top level parent')
    *
    * @return QString The name of the top level parent account
    */
  /*inline*/ QString topParentName( void ) const;

  /**
    * Returns a report account containing the top parent account
    *
    * @return ReportAccount The account of the top parent
    */
  ReportAccount topParent( void ) const;

  /**
    * Returns a report account containing the immediate parent account
    *
    * @return ReportAccount The account of the immediate parent
    */
  ReportAccount parent( void ) const;

  /**
    * Returns the number of accounts in this account's hierarchy.  If this is a
    * Top Category, it returns 1.  If it's parent is a Top Category, returns 2,
    * etc.
    *
    * @return unsigned Hierarchy depth
    */
  unsigned hierarchyDepth( void ) const;

  /**
    * Returns whether this account is a liquid asset
    *
    */
  bool isLiquidAsset( void ) const;

  /**
   * Returns whether this account is a liquid liability
   *
   */
  bool isLiquidLiability( void ) const;

protected:
  /**
    * Calculates the full account hierarchy of this account
    */
  void calculateAccountHierarchy( void );

};

} // end namespace reports

#endif // REPORTACCOUNT_H
