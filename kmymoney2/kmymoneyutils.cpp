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

#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kconfig.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyutils.h"
#include "mymoney/mymoneyfile.h"

QColor KMyMoneyUtils::_backgroundColour;
QColor KMyMoneyUtils::_listColour;
QColor KMyMoneyUtils::_gridColour;

QFont  KMyMoneyUtils::_cellFont;
QFont  KMyMoneyUtils::_headerFont;

bool   KMyMoneyUtils::_expertMode;

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
      returnString = i18n("Checking");
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
    case MyMoneyAccount::AssetLoan:
      returnString = i18n("Investment Loan");
      break;
    case MyMoneyAccount::Stock:
      returnString = i18n("Stock");
      break;
    case MyMoneyAccount::Equity:
      returnString = i18n("Equity");
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

  if(tmp == i18n("Checking").lower())
    rc = MyMoneyAccount::Checkings;
  else if(tmp == i18n("Savings").lower())
    rc = MyMoneyAccount::Savings;
  else if(tmp == i18n("Credit Card").lower())
    rc = MyMoneyAccount::CreditCard;
  else if(tmp == i18n("Cash").lower())
    rc = MyMoneyAccount::Cash;
  else if(tmp == i18n("Loan").lower())
    rc = MyMoneyAccount::Loan;
  else if(tmp == i18n("Certificate of Deposit").lower())
    rc = MyMoneyAccount::CertificateDep;
  else if(tmp == i18n("Investment").lower())
    rc = MyMoneyAccount::Investment;
  else if(tmp == i18n("Money Market").lower())
    rc = MyMoneyAccount::MoneyMarket;
  else if(tmp == i18n("Asset").lower())
    rc = MyMoneyAccount::Asset;
  else if(tmp == i18n("Liability").lower())
    rc = MyMoneyAccount::Liability;
  else if(tmp == i18n("Currency").lower())
    rc = MyMoneyAccount::Currency;
  else if(tmp == i18n("Income").lower())
    rc = MyMoneyAccount::Income;
  else if(tmp == i18n("Expense").lower())
    rc = MyMoneyAccount::Expense;
  else if(tmp == i18n("Investment Loan").lower())
    rc = MyMoneyAccount::AssetLoan;
  else if(tmp == i18n("Stock").lower())
    rc = MyMoneyAccount::Stock;
  else if(tmp == i18n("Equity").lower())
    rc = MyMoneyAccount::Equity;

  return rc;
}

const MyMoneySecurity::eSECURITYTYPE KMyMoneyUtils::stringToSecurity(const QString& txt)
{
  MyMoneySecurity::eSECURITYTYPE rc = MyMoneySecurity::SECURITY_NONE;
  QString tmp = txt.lower();

  if(tmp == i18n("Stock").lower())
    rc = MyMoneySecurity::SECURITY_STOCK;
  else if(tmp == i18n("Mutual Fund").lower())
    rc = MyMoneySecurity::SECURITY_MUTUALFUND;
  else if(tmp == i18n("Bond").lower())
    rc = MyMoneySecurity::SECURITY_BOND;
  else if(tmp == i18n("Currency").lower())
    rc = MyMoneySecurity::SECURITY_CURRENCY;

  return rc;
}

const QString KMyMoneyUtils::securityTypeToString(const MyMoneySecurity::eSECURITYTYPE securityType)
{
  QString returnString;

  switch (securityType)
  {
  case MyMoneySecurity::SECURITY_STOCK:
    returnString = i18n("Stock");
    break;
  case MyMoneySecurity::SECURITY_MUTUALFUND:
    returnString = i18n("Mutual Fund");
    break;
  case MyMoneySecurity::SECURITY_BOND:
    returnString = i18n("Bond");
    break;
  case MyMoneySecurity::SECURITY_CURRENCY:
    returnString = i18n("Currency");
    break;
  case MyMoneySecurity::SECURITY_NONE:
    returnString = i18n("None");
    break;
  default:
    returnString = i18n("Unknown");
  }

  return returnString;
}

const MyMoneySchedule::occurenceE KMyMoneyUtils::stringToOccurence(const QString& text)
{
  MyMoneySchedule::occurenceE occurence = MyMoneySchedule::OCCUR_ANY;
  QString tmp = text.lower();

  if(tmp == i18n("Once").lower())
    occurence = MyMoneySchedule::OCCUR_ONCE;
  else if(tmp == i18n("Daily").lower())
    occurence = MyMoneySchedule::OCCUR_DAILY;
  else if(tmp == i18n("Weekly").lower())
    occurence = MyMoneySchedule::OCCUR_WEEKLY;
  else if(tmp == i18n("Fortnightly").lower())
    occurence = MyMoneySchedule::OCCUR_FORTNIGHTLY;
  else if(tmp == i18n("Every other week").lower())
    occurence = MyMoneySchedule::OCCUR_EVERYOTHERWEEK;
  else if(tmp == i18n("Every four week").lower())
    occurence = MyMoneySchedule::OCCUR_EVERYFOURWEEKS;
  else if(tmp == i18n("Monthly").lower())
    occurence = MyMoneySchedule::OCCUR_MONTHLY;
  else if(tmp == i18n("Every two months").lower())
    occurence = MyMoneySchedule::OCCUR_EVERYOTHERMONTH;
  else if(tmp == i18n("Every three months").lower())
    occurence = MyMoneySchedule::OCCUR_EVERYTHREEMONTHS;
  else if(tmp == i18n("Quarterly").lower())
    occurence = MyMoneySchedule::OCCUR_QUARTERLY;
  else if(tmp == i18n("Every four months").lower())
    occurence = MyMoneySchedule::OCCUR_EVERYFOURMONTHS;
  else if(tmp == i18n("Twice yearly").lower())
    occurence = MyMoneySchedule::OCCUR_TWICEYEARLY;
  else if(tmp == i18n("Yearly").lower())
    occurence = MyMoneySchedule::OCCUR_YEARLY;
  else if(tmp == i18n("Every other year").lower())
    occurence = MyMoneySchedule::OCCUR_EVERYOTHERYEAR;

  return occurence;
}

const QString KMyMoneyUtils::occurenceToString(const MyMoneySchedule::occurenceE occurence)
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
      text = i18n("Every two months");
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
    case MyMoneySchedule::TYPE_LOANPAYMENT:
      text = i18n("Loan payment");
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
    case 4:
      rc = i18n("Favorite reports");
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
  else if(txt == i18n("Favorite reports"))
    idx = 4;
  return idx;
}

void KMyMoneyUtils::addDefaultHomePageItems(QStringList& list)
{
  for(int i = 1; i <= KMyMoneyUtils::maxHomePageItems; ++i) {
    if(list.find(QString::number(i)) != list.end()
    || list.find(QString::number(-i)) != list.end())
      continue;
    list.append(QString::number(i));
  }
}

bool KMyMoneyUtils::appendCorrectFileExt(QString& str, const QString& strExtToUse)
{
  bool rc = false;

  if(!str.isEmpty()) {
    //find last . delminator
    int nLoc = str.findRev('.');
    if(nLoc != -1) {
      QString strExt, strTemp;
      strTemp = str.left(nLoc + 1);
      strExt = str.right(str.length() - (nLoc + 1));
      if(strExt.find(strExtToUse, 0, FALSE) == -1) {
        // if the extension given contains a period, we remove our's
        if(strExtToUse.find('.') != -1)
          strTemp = strTemp.left(strTemp.length()-1);
        //append extension to make complete file name
        strTemp.append(strExtToUse);
        str = strTemp;
        rc = true;
      }
    } else {
      str.append(".");
      str.append(strExtToUse);
      rc = true;
    }
  }
  return rc;
}

int KMyMoneyUtils::occurenceToFrequency(const MyMoneySchedule::occurenceE occurence)
{
  int rc = 0;

  switch(occurence) {
    case MyMoneySchedule::OCCUR_DAILY:
      rc = 365;
      break;
    case MyMoneySchedule::OCCUR_WEEKLY:
      rc = 52;
      break;
    case MyMoneySchedule::OCCUR_FORTNIGHTLY:
      rc = 24;
      break;
    case MyMoneySchedule::OCCUR_EVERYOTHERWEEK:
      rc = 26;
      break;
    case MyMoneySchedule::OCCUR_EVERYFOURWEEKS:
      rc = 13;
      break;
    case MyMoneySchedule::OCCUR_MONTHLY:
      rc = 12;
      break;
    case MyMoneySchedule::OCCUR_EVERYOTHERMONTH:
      rc = 6;
      break;
    case MyMoneySchedule::OCCUR_QUARTERLY:
      rc = 4;
      break;
    case MyMoneySchedule::OCCUR_EVERYFOURMONTHS:
      rc = 3;
      break;
    case MyMoneySchedule::OCCUR_TWICEYEARLY:
      rc = 2;
      break;
    case MyMoneySchedule::OCCUR_YEARLY:
      rc = 1;
      break;
    default:
      qWarning("Occurence not supported by financial calculator");
  }

  return rc;
}

void KMyMoneyUtils::newPayee(QWidget* parent, kMyMoneyPayee* payeeEdit, const QString& payeeName)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  MyMoneyPayee payee;

  if(!payeeEdit)
    return;

  // Ask the user if that is what he intended to do?
  QString msg = i18n("Do you want to add '%1' as payee/receiver ?").arg(payeeName);

  if(KMessageBox::questionYesNo(parent, msg, i18n("New payee/receiver"),KStdGuiItem::yes(),KStdGuiItem::no(),"NewPayee") == KMessageBox::Yes) {
    // for now, we just add the payee to the pool. In the future,
    // we could open a dialog and ask for all the other attributes
    // of the payee.
    payee.setName(payeeName);

    try {
      file->addPayee(payee);
      payeeEdit->loadList();

    } catch(MyMoneyException *e) {
      KMessageBox::detailedSorry(0, i18n("Unable to add payee/receiver"),
        (e->what() + " " + i18n("thrown in") + " " + e->file()+ ":%1").arg(e->line()));
      delete e;

      payeeEdit->resetText();
    }
  } else
    payeeEdit->resetText();
}

const QColor KMyMoneyUtils::defaultBackgroundColour(void)
{
  return QColor(255,255,204);
}

const QColor KMyMoneyUtils::defaultListColour(void)
{
  return QColor(255,255,238);
}

const QColor KMyMoneyUtils::defaultGridColour(void)
{
  return QColor(154,154,154);
}

void KMyMoneyUtils::updateSettings(void)
{
  QColor c;
  QFont  f;
  KConfig *config = KGlobal::config();
  config->setGroup("List Options");

  c = defaultBackgroundColour();
  _backgroundColour = config->readColorEntry("listBGColor", &c);

  c = defaultListColour();
  _listColour = config->readColorEntry("listColor", &c);

  c = defaultGridColour();
  _gridColour = config->readColorEntry("listGridColor", &c);

  f = KGlobalSettings::generalFont();
  if(config->readBoolEntry("useSystemFont", true) == false)
    _cellFont = config->readFontEntry("listCellFont", &f);
  else
    _cellFont = f;

  f = KGlobalSettings::generalFont();
  f.setBold(true);
  if(config->readBoolEntry("useSystemFont", true) == false)
    _headerFont = config->readFontEntry("listHeaderFont", &f);
  else
    _headerFont = f;

  config->setGroup("General Options");
  _expertMode = config->readBoolEntry("ExpertMode", false);
}

void KMyMoneyUtils::checkConstants(void)
{
  Q_ASSERT(static_cast<int>(KLocale::ParensAround) == static_cast<int>(MyMoneyMoney::ParensAround));
  Q_ASSERT(static_cast<int>(KLocale::BeforeQuantityMoney) == static_cast<int>(MyMoneyMoney::BeforeQuantityMoney));
  Q_ASSERT(static_cast<int>(KLocale::AfterQuantityMoney) == static_cast<int>(MyMoneyMoney::AfterQuantityMoney));
  Q_ASSERT(static_cast<int>(KLocale::BeforeMoney) == static_cast<int>(MyMoneyMoney::BeforeMoney));
  Q_ASSERT(static_cast<int>(KLocale::AfterMoney) == static_cast<int>(MyMoneyMoney::AfterMoney));
}
