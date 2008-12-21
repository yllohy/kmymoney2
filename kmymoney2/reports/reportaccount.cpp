/***************************************************************************
                          reportaccount.cpp
                             -------------------
    begin                : Mon May 17 2004
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

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes
// This is just needed for i18n().  Once I figure out how to handle i18n
// without using this macro directly, I'll be freed of KDE dependency.  This
// is a minor problem because we use these terms when rendering to HTML,
// and a more major problem because we need it to translate account types
// (e.g. MyMoneyAccount::Checkings) into their text representation.  We also
// use that text representation in the core data structure of the report. (Ace)

#include <klocale.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "../mymoney/mymoneyfile.h"
#include "../mymoney/mymoneysecurity.h"
#include "reportdebug.h"
#include "reportaccount.h"

namespace reports {

ReportAccount::ReportAccount( void )
{
}

ReportAccount::ReportAccount( const ReportAccount& copy ):
  MyMoneyAccount( copy ), m_nameHierarchy( copy.m_nameHierarchy )
{
  // NOTE: I implemented the copy constructor solely for debugging reasons

  DEBUG_ENTER(__PRETTY_FUNCTION__);
}

ReportAccount::ReportAccount( const QString& accountid ):
  MyMoneyAccount( MyMoneyFile::instance()->account(accountid) )
{
  DEBUG_ENTER(__PRETTY_FUNCTION__);
  DEBUG_OUTPUT(QString("Account %1").arg(accountid));
  calculateAccountHierarchy();
}

ReportAccount::ReportAccount( const MyMoneyAccount& account ):
  MyMoneyAccount( account )
{
  DEBUG_ENTER(__PRETTY_FUNCTION__);
  DEBUG_OUTPUT(QString("Account %1").arg(account.id()));
  calculateAccountHierarchy();
}

void ReportAccount::calculateAccountHierarchy( void )
{
  DEBUG_ENTER(__PRETTY_FUNCTION__);

  MyMoneyFile* file = MyMoneyFile::instance();
  QString resultid = id();
  QString parentid = parentAccountId();

#ifdef DEBUG_HIDE_SENSITIVE
  m_nameHierarchy.prepend(file->account(resultid).id());
#else
  m_nameHierarchy.prepend(file->account(resultid).name());
#endif
  while (!file->isStandardAccount(parentid))
  {
    // take on the identity of our parent
    resultid = parentid;

    // and try again
    parentid = file->account(resultid).parentAccountId();
#ifdef DEBUG_HIDE_SENSITIVE
    m_nameHierarchy.prepend(file->account(resultid).id());
#else
    m_nameHierarchy.prepend(file->account(resultid).name());
#endif
  }
}

MyMoneyMoney ReportAccount::deepCurrencyPrice( const QDate& date ) const
{
  DEBUG_ENTER(__PRETTY_FUNCTION__);

  MyMoneyMoney result(1, 1);
  MyMoneyFile* file = MyMoneyFile::instance();

  MyMoneySecurity undersecurity = file->security( currencyId() );
  if ( ! undersecurity.isCurrency() )
  {
    MyMoneyPrice price = file->price(undersecurity.id(),undersecurity.tradingCurrency(),date);
    if ( price.isValid() )
    {
      result = price.rate(undersecurity.tradingCurrency());

      DEBUG_OUTPUT(QString("Converting under %1 to deep %2, price on %3 is %4")
        .arg(undersecurity.name())
        .arg(file->security(undersecurity.tradingCurrency()).name())
        .arg(date.toString())
        .arg(result.toDouble()));
    }
    else
    {
      DEBUG_OUTPUT(QString("No price to convert under %1 to deep %2 on %3")
        .arg(undersecurity.name())
        .arg(file->security(undersecurity.tradingCurrency()).name())
        .arg(date.toString()));
    }
  }

  return result;
}

MyMoneyMoney ReportAccount::baseCurrencyPrice( const QDate& date ) const
{
  // Note that whether or not the user chooses to convert to base currency, all the values
  // for a given account/category are converted to the currency for THAT account/category
  // The "Convert to base currency" tells the report to convert from the account/category
  // currency to the file's base currency.
  //
  // An example where this matters is if Category 'C' and account 'U' are in USD, but
  // Account 'J' is in JPY.  Say there are two transactions, one is US$100 from U to C,
  // the other is JPY10,000 from J to C.  Given a JPY price of USD$0.01, this means
  // C will show a balance of $200 NO MATTER WHAT the user chooses for 'convert to base
  // currency.  This confused me for a while, which is why I wrote this comment.
  //    --acejones

  DEBUG_ENTER(__PRETTY_FUNCTION__);

  MyMoneyMoney result(1, 1);
  MyMoneyFile* file = MyMoneyFile::instance();

  if(isForeignCurrency())
  {
    MyMoneyPrice price = file->price(currency().id(), file->baseCurrency().id(), date);
    if(price.isValid())
    {
      result = price.rate(file->baseCurrency().id());

      DEBUG_OUTPUT(QString("Converting deep %1 to base %2, price on %3 is %4")
        .arg(file->currency(currency().id()).name())
        .arg(file->baseCurrency().name())
        .arg(date.toString())
        .arg(result.toDouble()));
    }
    else
    {
      DEBUG_OUTPUT(QString("No price to convert deep %1 to base %2 on %3")
        .arg(file->currency(currency().id()).name())
        .arg(file->baseCurrency().name())
        .arg(date.toString()));
    }
  }

  return result;
}

/**
  * Fetch the trading currency of this account's currency
  *
  * @return The account's currency trading currency
  */
MyMoneySecurity ReportAccount::currency( void ) const
{
  MyMoneyFile* file = MyMoneyFile::instance();

  // First, get the deep currency
  MyMoneySecurity deepcurrency = file->security( currencyId() );
  if ( ! deepcurrency.isCurrency() )
    deepcurrency = file->security( deepcurrency.tradingCurrency() );

  // Return the deep currency's ID
  return deepcurrency;
}

/**
  * Determine if this account's deep currency is different from the file's
  * base currency
  *
  * @return bool True if this account is in a foreign currency
  */
bool ReportAccount::isForeignCurrency( void ) const
{
  return ( currency().id() != MyMoneyFile::instance()->baseCurrency().id() );
}

bool ReportAccount::operator<(const ReportAccount& second) const
{
//   DEBUG_ENTER(__PRETTY_FUNCTION__);

  bool result = false;
  bool haveresult = false;
  QStringList::const_iterator it_first = m_nameHierarchy.begin();
  QStringList::const_iterator it_second = second.m_nameHierarchy.begin();
  while ( it_first != m_nameHierarchy.end() )
  {
    // The first string is longer than the second, but otherwise identical
    if ( it_second == second.m_nameHierarchy.end() )
    {
      result = false;
      haveresult = true;
      break;
    }

    if ( (*it_first) < (*it_second) )
    {
      result = true;
      haveresult = true;
      break;
    }
    else if ( (*it_first) > (*it_second) )
    {
      result = false;
      haveresult = true;
      break;
    }

    ++it_first;
    ++it_second;
  }

  // The second string is longer than the first, but otherwise identical
  if ( !haveresult && ( it_second != second.m_nameHierarchy.end() ) )
    result = true;

//   DEBUG_OUTPUT(QString("%1 < %2 is %3").arg(debugName(),second.debugName()).arg(result));
  return result;
}

/**
  * The name of only this account.  No matter how deep the hierarchy, this
  * method only returns the last name in the list, which is the engine name]
  * of this account.
  *
  * @return QString The account's name
  */
QString ReportAccount::name( void ) const
{
  return m_nameHierarchy.back();
}

// MyMoneyAccount:fullHierarchyDebug()
QString ReportAccount::debugName( void ) const
{
  return m_nameHierarchy.join("|");
}

// MyMoneyAccount:fullHierarchy()
QString ReportAccount::fullName( void ) const
{
  return m_nameHierarchy.join(": ");
}

// MyMoneyAccount:isTopCategory()
bool ReportAccount::isTopLevel( void ) const
{
  return ( m_nameHierarchy.size() == 1 );
}

// MyMoneyAccount:hierarchyDepth()
unsigned ReportAccount::hierarchyDepth( void ) const
{
  return ( m_nameHierarchy.size() );
}

ReportAccount ReportAccount::parent( void ) const
{
  return ReportAccount( parentAccountId() );
}

ReportAccount ReportAccount::topParent( void ) const
{
  DEBUG_ENTER(__PRETTY_FUNCTION__);

  MyMoneyFile* file = MyMoneyFile::instance();
  QString resultid = id();
  QString parentid = parentAccountId();

  while (!file->isStandardAccount(parentid))
  {
    // take on the identity of our parent
    resultid = parentid;

    // and try again
    parentid = file->account(resultid).parentAccountId();
  }

  return ReportAccount( resultid );
}

QString ReportAccount::topParentName( void ) const
{
  return m_nameHierarchy.first();
}

bool ReportAccount::isLiquidAsset( void ) const
{
  return accountType() == MyMoneyAccount::Cash ||
      accountType() == MyMoneyAccount::Checkings ||
      accountType() == MyMoneyAccount::Savings;
}


bool ReportAccount::isLiquidLiability( void ) const
{
  return accountType() == MyMoneyAccount::CreditCard;

}




}  // end namespace reports
