/***************************************************************************
                          kmymoneyutils.cpp  -  description
                             -------------------
    begin                : Wed Feb 5 2003
    copyright            : (C) 2000-2003 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
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
// KDE Headers

#include "klocale.h"

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyutils.h"

KMyMoneyUtils::KMyMoneyUtils()
{
}

KMyMoneyUtils::~KMyMoneyUtils()
{
}

const QString KMyMoneyUtils::accountTypeToString(const MyMoneyAccount::accountTypeE accountType)
{
  QString returnString;

  switch (accountType)
  {
    case MyMoneyAccount::Checkings:
      returnString = i18n("Checkings");
      break;
    case MyMoneyAccount::Savings:
      returnString = i18n("Savings");
      break;
    case MyMoneyAccount::CreditCard:
      returnString = i18n("Credit Card");
      break;
    case MyMoneyAccount::Cash:
      returnString = i18n("Cash");
      break;
    case MyMoneyAccount::Loan:
      returnString = i18n("Loan");
      break;
    case MyMoneyAccount::CertificateDep:
      returnString = i18n("Certificate of Deposit");
      break;
    case MyMoneyAccount::Investment:
      returnString = i18n("Investment");
      break;
    case MyMoneyAccount::MoneyMarket:
      returnString = i18n("Money Market");
      break;
    case MyMoneyAccount::Asset:
      returnString = i18n("Asset");
      break;
    case MyMoneyAccount::Liability:
      returnString = i18n("Liability");
      break;
    case MyMoneyAccount::Currency:
      returnString = i18n("Currency");
      break;
    case MyMoneyAccount::Income:
      returnString = i18n("Income");
      break;
    case MyMoneyAccount::Expense:
      returnString = i18n("Expense");
      break;
    default:
      returnString = i18n("Unknown");
  }

  return returnString;
}

const MyMoneyAccount::accountTypeE KMyMoneyUtils::stringToAccountType(const QString& type)
{
  MyMoneyAccount::accountTypeE rc = MyMoneyAccount::UnknownAccountType;
  QString tmp = type.lower();
  
  if(tmp == i18n("checkings"))
    rc = MyMoneyAccount::Checkings;
  else if(tmp == i18n("savings"))
    rc = MyMoneyAccount::Savings;
  else if(tmp == i18n("credit card"))
    rc = MyMoneyAccount::CreditCard;
  else if(tmp == i18n("cash"))
    rc = MyMoneyAccount::Cash;
  else if(tmp == i18n("loan"))
    rc = MyMoneyAccount::Loan;
  else if(tmp == i18n("certificate of deposit"))
    rc = MyMoneyAccount::CertificateDep;
  else if(tmp == i18n("investment"))
    rc = MyMoneyAccount::Investment;
  else if(tmp == i18n("money market"))
    rc = MyMoneyAccount::MoneyMarket;
  else if(tmp == i18n("asset"))
    rc = MyMoneyAccount::Asset;
  else if(tmp == i18n("liability"))
    rc = MyMoneyAccount::Liability;
  else if(tmp == i18n("currency"))
    rc = MyMoneyAccount::Currency;
  else if(tmp == i18n("income"))
    rc = MyMoneyAccount::Income;
  else if(tmp == i18n("expense"))
    rc = MyMoneyAccount::Expense;

  return rc;
}
