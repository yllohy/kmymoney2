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
#include <kstandarddirs.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/mymoneyfile.h>
#include <kmymoney/mymoneyforecast.h>
#include <kmymoney/kmymoneyglobalsettings.h>
#include <kmymoney/investtransactioneditor.h>

#include "kmymoneyutils.h"

KMyMoneyUtils::KMyMoneyUtils()
{
}

KMyMoneyUtils::~KMyMoneyUtils()
{
}

const QString KMyMoneyUtils::accountTypeToString(const MyMoneyAccount::accountTypeE accountType)
{
  return MyMoneyAccount::accountTypeToString(accountType);
}

MyMoneyAccount::accountTypeE KMyMoneyUtils::stringToAccountType(const QString& type)
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

MyMoneySecurity::eSECURITYTYPE KMyMoneyUtils::stringToSecurity(const QString& txt)
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
  return i18n(MyMoneySecurity::securityTypeToString(securityType));
}

const QString KMyMoneyUtils::occurenceToString(const MyMoneySchedule::occurenceE occurence)
{
  return i18n(MyMoneySchedule::occurenceToString(occurence));
}

const QString KMyMoneyUtils::paymentMethodToString(MyMoneySchedule::paymentTypeE paymentType)
{
  return i18n(MyMoneySchedule::paymentMethodToString(paymentType));
}

const QString KMyMoneyUtils::weekendOptionToString(MyMoneySchedule::weekendOptionE weekendOption)
{
  return i18n(MyMoneySchedule::weekendOptionToString(weekendOption));
}

const QString KMyMoneyUtils::scheduleTypeToString(MyMoneySchedule::typeE type)
{
  return i18n(MyMoneySchedule::scheduleTypeToString(type));
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

const char* homePageItems[] = {
  I18N_NOOP("Payments"),
  I18N_NOOP("Preferred accounts"),
  I18N_NOOP("Payment accounts"),
  I18N_NOOP("Favorite reports"),
  I18N_NOOP("Forecast (schedule)"),
  I18N_NOOP("Networth forecast"),
  I18N_NOOP("Forecast (history)"),
  I18N_NOOP("Assets and Liabilities"),
  I18N_NOOP("Budget"),
  I18N_NOOP("CashFlow"),
  // insert new items above this comment
  0
};

const QString KMyMoneyUtils::homePageItemToString(const int idx)
{
  QString rc;
  if(abs(idx) > 0 && abs(idx) < static_cast<int>(sizeof(homePageItems)/sizeof(homePageItems[0]))) {
    rc = i18n(homePageItems[abs(idx-1)]);
  }
  return rc;
}

int KMyMoneyUtils::stringToHomePageItem(const QString& txt)
{
  int idx = 0;
  for(idx = 0; homePageItems[idx] != 0; ++idx) {
    if(txt == i18n(homePageItems[idx]))
      return idx+1;
  }
  return 0;
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

void KMyMoneyUtils::checkConstants(void)
{
  Q_ASSERT(static_cast<int>(KLocale::ParensAround) == static_cast<int>(MyMoneyMoney::ParensAround));
  Q_ASSERT(static_cast<int>(KLocale::BeforeQuantityMoney) == static_cast<int>(MyMoneyMoney::BeforeQuantityMoney));
  Q_ASSERT(static_cast<int>(KLocale::AfterQuantityMoney) == static_cast<int>(MyMoneyMoney::AfterQuantityMoney));
  Q_ASSERT(static_cast<int>(KLocale::BeforeMoney) == static_cast<int>(MyMoneyMoney::BeforeMoney));
  Q_ASSERT(static_cast<int>(KLocale::AfterMoney) == static_cast<int>(MyMoneyMoney::AfterMoney));
}

QString KMyMoneyUtils::variableCSS(void)
{
  QColor tcolor = KGlobalSettings::textColor();

  QString css;
  css += "<style type=\"text/css\">\n<!--\n";
  css += QString(".row-even, .item0 { background-color: %1; color: %2 }\n")
    .arg((KMyMoneyGlobalSettings::listBGColor()).name()).arg(tcolor.name());
  css += QString(".row-odd, .item1  { background-color: %1; color: %2 }\n")
    .arg((KMyMoneyGlobalSettings::listColor()).name()).arg(tcolor.name());
  css += "-->\n</style>\n";
  return css;
}

QString KMyMoneyUtils::findResource(const char* type, const QString& filename)
{
  QString language = KGlobal::locale()->language();
  QString country = KGlobal::locale()->country();
  QString rc, mask;

  // check that the placeholder is present
  if(!filename.find("%1")) {
    qWarning("%%1 not found in '%s'", filename.latin1());
    return filename;
  }

  // search the given resource
  mask = filename.arg("_%1.%2");
  rc = KGlobal::dirs()->findResource(type, mask.arg(country).arg(language));
  if(rc.isEmpty()) {
    mask = filename.arg("_%1");
    rc = KGlobal::dirs()->findResource(type, mask.arg(language));
  }
  if(rc.isEmpty()) {
    // qDebug(QString("html/home_%1.html not found").arg(country).latin1());
    rc = KGlobal::dirs()->findResource(type, mask.arg(country));
  }
  if(rc.isEmpty()) {
    rc = KGlobal::dirs()->findResource(type, filename.arg(""));
  }

  if(rc.isEmpty()) {
    qWarning("No resource found for (%s,%s)", type, filename.latin1());
  }
  return rc;
}

const MyMoneySplit KMyMoneyUtils::stockSplit(const MyMoneyTransaction& t)
{
  QValueList<MyMoneySplit>::ConstIterator it_s;
  MyMoneySplit investmentAccountSplit;
  for(it_s = t.splits().begin(); it_s != t.splits().end(); ++it_s) {
    if(!(*it_s).accountId().isEmpty()) {
      MyMoneyAccount acc = MyMoneyFile::instance()->account((*it_s).accountId());
      if(acc.isInvest()) {
        return *it_s;
      }
      // if we have a reference to an investment account, we remember it here
      if(acc.accountType() == MyMoneyAccount::Investment)
        investmentAccountSplit = *it_s;
    }
  }
  // if we haven't found a stock split, we see if we've seen
  // an investment account on the way. If so, we return it.
  if(!investmentAccountSplit.id().isEmpty())
    return investmentAccountSplit;

  // if none was found, we return an empty split.
  return MyMoneySplit();
}

KMyMoneyUtils::transactionTypeE KMyMoneyUtils::transactionType(const MyMoneyTransaction& t)
{
  if(!stockSplit(t).id().isEmpty())
    return InvestmentTransaction;

  if(t.splitCount() < 2) {
    return Unknown;
  } else if(t.splitCount() > 2) {
    // FIXME check for loan transaction here
    return SplitTransaction;
  }
  QString ida, idb;
  ida = t.splits()[0].accountId();
  idb = t.splits()[1].accountId();
  if(ida.isEmpty() || idb.isEmpty())
    return Unknown;

  MyMoneyAccount a, b;
  a = MyMoneyFile::instance()->account(ida);
  b = MyMoneyFile::instance()->account(idb);
  if((a.accountGroup() == MyMoneyAccount::Asset
   || a.accountGroup() == MyMoneyAccount::Liability)
  && (b.accountGroup() == MyMoneyAccount::Asset
   || b.accountGroup() == MyMoneyAccount::Liability))
    return Transfer;
  return Normal;
}

void KMyMoneyUtils::calculateAutoLoan(const MyMoneySchedule& schedule, MyMoneyTransaction& transaction, const QMap<QString, MyMoneyMoney>& balances)
{
  try {
    MyMoneyForecast::calculateAutoLoan(schedule, transaction, balances);
  } catch (MyMoneyException* e) {
    KMessageBox::detailedError(0, i18n("Unable to load schedule details"), e->what());
    delete e;
  }
}

QString KMyMoneyUtils::nextCheckNumber(const MyMoneyAccount& acc)
{
  // determine next check number
  QString number;
  QRegExp exp(QString("(.*\\D)?(\\d+)(\\D.*)?"));
  if(exp.search(acc.value("lastNumberUsed")) != -1) {
    number = QString("%1%2%3").arg(exp.cap(1)).arg(exp.cap(2).toULongLong() + 1).arg(exp.cap(3));
  } else {
    number = "1";
  }
  return number;
}

QString KMyMoneyUtils::reconcileStateToString(MyMoneySplit::reconcileFlagE flag, bool text)
{
  QString txt;
  if(text) {
    switch(flag) {
      case MyMoneySplit::NotReconciled:
        txt = i18n("Reconcile state 'Not reconciled'", "Not reconciled");
        break;
      case MyMoneySplit::Cleared:
        txt = i18n("Reconcile state 'Cleared'", "Cleared");
        break;
      case MyMoneySplit::Reconciled:
        txt = i18n("Reconcile state 'Reconciled'", "Reconciled");
        break;
      case MyMoneySplit::Frozen:
        txt = i18n("Reconcile state 'Frozen'", "Frozen");
        break;
      default:
        txt = i18n("Unknown");
        break;
    }
  } else {
    switch(flag) {
      case MyMoneySplit::NotReconciled:
        break;
      case MyMoneySplit::Cleared:
        txt = i18n("Reconcile flag C", "C");
        break;
      case MyMoneySplit::Reconciled:
        txt = i18n("Reconcile flag R", "R");
        break;
      case MyMoneySplit::Frozen:
        txt = i18n("Reconcile flag F", "F");
        break;
      default:
        txt = i18n("Flag for unknown reconciliation state", "?");
        break;
    }
  }
  return txt;
}

MyMoneyTransaction KMyMoneyUtils::scheduledTransaction(const MyMoneySchedule& schedule)
{
  MyMoneyTransaction t = schedule.transaction();

  try {
    if (schedule.type() == MyMoneySchedule::TYPE_LOANPAYMENT) {
      calculateAutoLoan(schedule, t, QMap<QString, MyMoneyMoney>());
    }
  } catch (MyMoneyException* e) {
    qDebug("Unable to load schedule details for '%s' during transaction match: %s", schedule.name().data(), e->what().data());
    delete e;
  }

  t.clearId();
  t.setEntryDate(QDate());
  return t;
}

void KMyMoneyUtils::previouslyUsedCategories(const QString& investmentAccount, QString& feesId, QString& interestId)
{
  feesId = interestId = QString();
  MyMoneyFile* file = MyMoneyFile::instance();
  try {
    MyMoneyAccount acc = file->account(investmentAccount);
    MyMoneyTransactionFilter filter(investmentAccount);
    filter.setReportAllSplits(false);
    // since we assume an investment account here, we need to collect the stock accounts as well
    filter.addAccount(acc.accountList());
    QValueList< QPair<MyMoneyTransaction, MyMoneySplit> > list;
    file->transactionList(list, filter);
    QValueList< QPair<MyMoneyTransaction, MyMoneySplit> >::const_iterator it_t;
    for(it_t = list.begin(); it_t != list.end(); ++it_t) {
      const MyMoneyTransaction& t = (*it_t).first;
      const MyMoneySplit&s = (*it_t).second;
      MyMoneySplit assetAccountSplit;
      QValueList<MyMoneySplit> feeSplits;
      QValueList<MyMoneySplit> interestSplits;
      MyMoneySecurity security;
      MyMoneySecurity currency;
      MyMoneySplit::investTransactionTypeE transactionType;
      InvestTransactionEditor::dissectTransaction(t, s, assetAccountSplit, feeSplits, interestSplits, security, currency, transactionType);
      if(feeSplits.count() == 1) {
        feesId = feeSplits.first().accountId();
      }
      if(interestSplits.count() == 1) {
        interestId = interestSplits.first().accountId();
      }
    }
  } catch(MyMoneyException *e) {
    delete e;
  }

}


