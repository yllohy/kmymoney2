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
#include <kglobal.h>
#include <kiconloader.h>

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

const QString KMyMoneyUtils::occurenceToString(MyMoneySchedule::occurenceE occurence)
{
  QString text;

  switch (occurence)
  {
    case MyMoneySchedule::OCCUR_ONCE:
      text = i18n("Once");
      break;
    case MyMoneySchedule::OCCUR_DAILY:
      text = i18n("Daily");
      break;
    case MyMoneySchedule::OCCUR_WEEKLY:
      text = i18n("Weekly");
      break;
    case MyMoneySchedule::OCCUR_FORTNIGHTLY:
      text = i18n("Fortnightly");
      break;
    case MyMoneySchedule::OCCUR_EVERYOTHERWEEK:
      text = i18n("Every other week");
      break;
    case MyMoneySchedule::OCCUR_EVERYFOURWEEKS:
      text = i18n("Every four weeks");
      break;
    case MyMoneySchedule::OCCUR_MONTHLY:
      text = i18n("Monthly");
      break;
    case MyMoneySchedule::OCCUR_EVERYOTHERMONTH:
      text = i18n("Every other month");
      break;
    case MyMoneySchedule::OCCUR_EVERYTHREEMONTHS:
      text = i18n("Every three months");
      break;
    case MyMoneySchedule::OCCUR_QUARTERLY:
      text = i18n("Quarterly");
      break;
    case MyMoneySchedule::OCCUR_EVERYFOURMONTHS:
      text = i18n("Every four months");
      break;
    case MyMoneySchedule::OCCUR_TWICEYEARLY:
      text = i18n("Twice yearly");
      break;
    case MyMoneySchedule::OCCUR_YEARLY:
      text = i18n("Yearly");
      break;
    case MyMoneySchedule::OCCUR_EVERYOTHERYEAR:
      text = i18n("Every other year");
      break;
    case MyMoneySchedule::OCCUR_ANY:
      text = i18n("Any (Error)");
      break;
  }
  return text;
}

const QString KMyMoneyUtils::paymentMethodToString(MyMoneySchedule::paymentTypeE paymentType)
{
  QString text;

  switch (paymentType)
  {
    case MyMoneySchedule::STYPE_DIRECTDEBIT:
      text = i18n("Direct debit");
      break;
    case MyMoneySchedule::STYPE_DIRECTDEPOSIT:
      text = i18n("Direct deposit");
      break;
    case MyMoneySchedule::STYPE_MANUALDEPOSIT:
      text = i18n("Manual deposit");
      break;
    case MyMoneySchedule::STYPE_OTHER:
      text = i18n("Other");
      break;
    case MyMoneySchedule::STYPE_WRITECHEQUE:
      text = i18n("Write cheque");
      break;
    case MyMoneySchedule::STYPE_ANY:
      text = i18n("Any (Error)");
      break;
  }
  return text;
}

const QString KMyMoneyUtils::scheduleTypeToString(MyMoneySchedule::typeE type)
{
  QString text;
  
  switch (type)
  {
    case MyMoneySchedule::TYPE_BILL:
      text = i18n("Bill");
      break;
    case MyMoneySchedule::TYPE_DEPOSIT:
      text = i18n("Deposit");
      break;
    case MyMoneySchedule::TYPE_TRANSFER:
      text = i18n("Transfer");
      break;
    case MyMoneySchedule::TYPE_ANY:
    default:
      text = i18n("Unknown");
  }
  return text;
}

KGuiItem KMyMoneyUtils::splitGuiItem(void)
{
  KIconLoader *ic = KGlobal::iconLoader();
  
  KGuiItem splitGuiItem(  i18n("&Split"),
                          QIconSet(ic->loadIcon("split", KIcon::User, KIcon::SizeSmall)),
                          i18n("Split the amount into different categories."),
                          i18n("Split the amount into different categories."));

  return splitGuiItem;
}

KGuiItem KMyMoneyUtils::scheduleNewGuiItem(void)
{
  KIconLoader *ic = KGlobal::iconLoader();

  KGuiItem splitGuiItem(  i18n("&New Schedule..."),
                          QIconSet(ic->loadIcon("filenew", KIcon::Small, KIcon::SizeSmall)),
                          i18n("Create a new schedule."),
                          i18n("Use this to create a new schedule."));

  return splitGuiItem;
}

KGuiItem KMyMoneyUtils::accountsFilterGuiItem(void)
{
  KIconLoader *ic = KGlobal::iconLoader();

  KGuiItem splitGuiItem(  i18n("&Filter"),
                          QIconSet(ic->loadIcon("filter", KIcon::Small, KIcon::SizeSmall)),
                          i18n("Filter out accounts"),
                          i18n("Use this to filter out accounts"));

  return splitGuiItem;
}
  
QPixmap KMyMoneyUtils::billScheduleIcon(int size)
{
  KIconLoader *ic = KGlobal::iconLoader();
  return ic->loadIcon("billschedule", KIcon::User, size);
}

QPixmap KMyMoneyUtils::depositScheduleIcon(int size)
{
  KIconLoader *ic = KGlobal::iconLoader();
  return ic->loadIcon("depositschedule", KIcon::User, size);
}

QPixmap KMyMoneyUtils::transferScheduleIcon(int size)
{
  KIconLoader *ic = KGlobal::iconLoader();
  return ic->loadIcon("transferschedule", KIcon::User, size);
}

QPixmap KMyMoneyUtils::scheduleIcon(int size)
{
  KIconLoader *ic = KGlobal::iconLoader();
  return ic->loadIcon("schedule", KIcon::User, size);
}

const QString KMyMoneyUtils::homePageItemToString(const int idx)
{
  QString rc;
  
  switch(abs(idx)) {
    case 1:
      rc = i18n("Payments");
      break;
    case 2:
      rc = i18n("Preferred accounts");
      break;
    case 3:
      rc = i18n("Payment accounts");
      break;
    default:
      rc = "";
      break;
  }
  return rc;
}

const int KMyMoneyUtils::stringToHomePageItem(const QString& txt)
{
  int idx = 0;
  if(txt == i18n("Payments"))
    idx = 1;
  else if(txt == i18n("Preferred accounts"))
    idx = 2;
  else if(txt == i18n("Payment accounts"))
    idx = 3;
  return idx;
}
