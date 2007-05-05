/***************************************************************************
                          mymoneystoragedump.cpp  -  description
                             -------------------
    begin                : Sun May 5 2002
    copyright            : (C) 2000-2002 by Michael Edwardes
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

#include <qstring.h>
#include <qdatetime.h>
#include <qvaluelist.h>
#include <qstringlist.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneystoragedump.h"
#include "imymoneystorage.h"
#include "../mymoneyaccount.h"
#include "../mymoneysecurity.h"
#include "../mymoneyprice.h"

MyMoneyStorageDump::MyMoneyStorageDump()
{
}

MyMoneyStorageDump::~MyMoneyStorageDump()
{
}

void MyMoneyStorageDump::readStream(QDataStream& /* s */, IMyMoneySerialize* /* storage */)
{
  qDebug("Reading not supported by MyMoneyStorageDump!!");
}

void MyMoneyStorageDump::writeStream(QDataStream& _s, IMyMoneySerialize* _storage)
{
  QTextStream s(_s.device());
  IMyMoneyStorage* storage = dynamic_cast<IMyMoneyStorage *> (_storage);
  MyMoneyPayee user = storage->user();

  s << "File-Info\n";
  s << "---------\n";
  s << "user name = " << user.name() << "\n";
  s << "user street = " << user.address() << "\n";
  s << "user city = " << user.city() << "\n";
  s << "user city = " << user.state() << "\n";
  s << "user zip = " << user.postcode() << "\n";
  s << "user telephone = " << user.telephone() << "\n";
  s << "user e-mail = " << user.email() << "\n";
  s << "creation date = " << storage->creationDate().toString(Qt::ISODate) << "\n";
  s << "last modification date = " << storage->lastModificationDate().toString(Qt::ISODate) << "\n";
  s << "base currency = " << storage->value("kmm-baseCurrency") << "\n";
  s << "\n";

  s << "Internal-Info\n";
  s << "-------------\n";
  s << "accounts = " << _storage->accountList().count() <<", next id = " << _storage->accountId() << "\n";
  MyMoneyTransactionFilter filter;
  filter.setReportAllSplits(false);
  QValueList<MyMoneyTransaction> list_t;
  storage->transactionList(list_t, filter);
  QValueList<MyMoneyTransaction>::ConstIterator it_t;
  s << "transactions = " << list_t.count() << ", next id = " << _storage->transactionId() << "\n";
  QMap<int,int> xferCount;
  for(it_t = list_t.begin(); it_t != list_t.end(); ++it_t) {
    QValueList<MyMoneySplit>::ConstIterator it_s;
    int accountCount = 0;
    for(it_s = (*it_t).splits().begin(); it_s != (*it_t).splits().end(); ++it_s) {
      MyMoneyAccount acc = storage->account((*it_s).accountId());
      if(acc.accountGroup() != MyMoneyAccount::Expense
      && acc.accountGroup() != MyMoneyAccount::Income)
        accountCount++;
    }
    if(accountCount > 1)
      xferCount[accountCount] = xferCount[accountCount] + 1;
  }
  QMap<int,int>::ConstIterator it_cnt;
  for(it_cnt = xferCount.begin(); it_cnt != xferCount.end(); ++it_cnt) {
    s << "               " << *it_cnt << " of them references " << it_cnt.key() << " accounts\n";
  }

  s << "payees = " << _storage->payeeList().count() << ", next id = " << _storage->payeeId() << "\n";
  s << "institutions = " << _storage->institutionList().count() << ", next id = " << _storage->institutionId() << "\n";
  s << "schedules = " << _storage->scheduleList().count() << ", next id = " << _storage->scheduleId() << "\n";
  s << "\n";

  s << "Institutions\n";
  s << "------------\n";

  QValueList<MyMoneyInstitution> list_i = storage->institutionList();
  QValueList<MyMoneyInstitution>::ConstIterator it_i;
  for(it_i = list_i.begin(); it_i != list_i.end(); ++it_i) {
    s << "  ID = " << (*it_i).id() << "\n";
    s << "  Name = " << (*it_i).name() << "\n";
    s << "\n";
  }
  s << "\n";

  s << "Payees" << "\n";
  s << "------" << "\n";

  QValueList<MyMoneyPayee> list_p = storage->payeeList();
  QValueList<MyMoneyPayee>::ConstIterator it_p;
  for(it_p = list_p.begin(); it_p != list_p.end(); ++it_p) {
    s << "  ID = " << (*it_p).id() << "\n";
    s << "  Name = " << (*it_p).name() << "\n";
    s << "  Address = " << (*it_p).address() << "\n";
    s << "  City = " << (*it_p).city() << "\n";
    s << "  State = " << (*it_p).state() << "\n";
    s << "  Zip = " << (*it_p).postcode() << "\n";
    s << "  E-Mail = " << (*it_p).email() << "\n";
    s << "  Telephone = " << (*it_p).telephone() << "\n";
    s << "  Reference = " << (*it_p).reference() << "\n";
    s << "\n";
  }
  s << "\n";


  s << "Accounts" << "\n";
  s << "--------" << "\n";

  QValueList<MyMoneyAccount> list_a = storage->accountList();
  list_a.push_front(storage->equity());
  list_a.push_front(storage->expense());
  list_a.push_front(storage->income());
  list_a.push_front(storage->liability());
  list_a.push_front(storage->asset());
  QValueList<MyMoneyAccount>::ConstIterator it_a;
  for(it_a = list_a.begin(); it_a != list_a.end(); ++it_a) {
    s << "  ID = " << (*it_a).id() << "\n";
    s << "  Name = " << (*it_a).name() << "\n";
    s << "  Number = " << (*it_a).number() << "\n";
    s << "  Description = " << (*it_a).description() << "\n";
    s << "  Type = " << (*it_a).accountType() << "\n";
    if((*it_a).currencyId().isEmpty()) {
      s << "  Currency = unknown\n";
    } else {
      if((*it_a).accountType() == MyMoneyAccount::Stock) {
        s << "  Equity = " << storage->security((*it_a).currencyId()).name() << "\n";
      } else {
        s << "  Currency = " << storage->currency((*it_a).currencyId()).name() << "\n";
      }
    }
    s << "  Parent = " << (*it_a).parentAccountId();
    if(!(*it_a).parentAccountId().isEmpty()) {
      MyMoneyAccount parent = storage->account((*it_a).parentAccountId());
      s << " (" << parent.name() << ")";
    } else {
      s << "n/a";
    }
    s << "\n";

    s << "  Institution = " << (*it_a).institutionId();
    if(!(*it_a).institutionId().isEmpty()) {
      MyMoneyInstitution inst = storage->institution((*it_a).institutionId());
      s << " (" << inst.name() << ")";
    } else {
      s << "n/a";
    }
    s << "\n";

    s << "  Opening data = " << (*it_a).openingDate().toString(Qt::ISODate) << "\n";
    s << "  Last modified = " << (*it_a).lastModified().toString(Qt::ISODate) << "\n";
    s << "  Last reconciled = " << (*it_a).lastReconciliationDate().toString(Qt::ISODate) << "\n";
    s << "  Balance = " << (*it_a).balance().formatMoney() << "\n";

    s << "  KVP: " << "\n";
    QMap<QCString, QString>kvp = (*it_a).pairs();
    QMap<QCString, QString>::Iterator it;
    for(it = kvp.begin(); it != kvp.end(); ++it) {
      s << "    '" << it.key() << "' = '" << it.data() << "'\n";
    }

    QCStringList list_s = (*it_a).accountList();
    QCStringList::ConstIterator it_s;
    if(list_s.count() > 0) {
      s << "  Children =" << "\n";
    }
    for(it_s = list_s.begin(); it_s != list_s.end(); ++it_s) {
      MyMoneyAccount child = storage->account(*it_s);
      s << "    " << *it_s << " (" << child.name() << ")\n";
    }
    s << "\n";
  }
  s << "\n";

#if 0
  s << "Currencies" << "\n";
  s << "----------" << "\n";

  QValueList<MyMoneyCurrency> list_c = storage->currencyList();
  QValueList<MyMoneyCurrency>::ConstIterator it_c;
  for(it_c = list_c.begin(); it_c != list_c.end(); ++it_c) {
    s << "  Name = " << (*it_c).name() << "\n";
    s << "    ID = " << (*it_c).id() << "\n";
    s << "    Symbol = " << (*it_c).tradingSymbol() << "\n";
    s << "    Parts/Unit = " << (*it_c).partsPerUnit() << "\n";
    s << "    smallest cash fraction = " << (*it_c).smallestCashFraction() << "\n";
    s << "    smallest account fraction = " << (*it_c).smallestAccountFraction() << "\n";
    dumpPriceHistory(s, (*it_c).priceHistory());
    s << "\n";
  }
  s << "\n";
#endif

  s << "Securities" << "\n";
  s << "----------" << "\n";

  QValueList<MyMoneySecurity> list_e = storage->securityList();
  QValueList<MyMoneySecurity>::ConstIterator it_e;
  for(it_e = list_e.begin(); it_e != list_e.end(); ++it_e) {
    s << "  Name = " << (*it_e).name() << "\n";
    s << "    ID = " << (*it_e).id() << "\n";
    s << "    Market   = " << (*it_e).tradingMarket() << "\n";
    s << "    Symbol   = " << (*it_e).tradingSymbol() << "\n";
    s << "    Currency = " << (*it_e).tradingCurrency() << " (";
    if((*it_e).tradingCurrency().isEmpty()) {
      s << "unknown";
    } else {
      MyMoneySecurity tradingCurrency = storage->currency((*it_e).tradingCurrency());
      if(!tradingCurrency.isCurrency()) {
        s << "invalid currency: ";
      }
      s << tradingCurrency.name();
    }
    s << ")\n";

    s << "    Type = " << securityTypeToString((*it_e).securityType()) << "\n";
    s << "    smallest account fraction = " << (*it_e).smallestAccountFraction() << "\n";

    s << "    KVP: " << "\n";
    QMap<QCString, QString>kvp = (*it_e).pairs();
    QMap<QCString, QString>::Iterator it;
    for(it = kvp.begin(); it != kvp.end(); ++it) {
      s << "      '" << it.key() << "' = '" << it.data() << "'\n";
    }
    s << "\n";
  }
  s << "\n";

  s << "Prices" << "\n";
  s << "--------" << "\n";

  MyMoneyPriceList list_pr = _storage->priceList();
  MyMoneyPriceList::ConstIterator it_pr;
  for(it_pr = list_pr.begin(); it_pr != list_pr.end(); ++it_pr) {
    s << "  From = " << it_pr.key().first << "\n";
    s << "    To = " << it_pr.key().second << "\n";
    MyMoneyPriceEntries::ConstIterator it_pre;
    for(it_pre = (*it_pr).begin(); it_pre != (*it_pr).end(); ++it_pre) {
      s << "      Date = " << (*it_pre).date().toString() << "\n";
      s << "        Price = " << (*it_pre).rate().formatMoney("", 8) << "\n";
      s << "        Source = " << (*it_pre).source() << "\n";
      s << "        From = " << (*it_pre).from() << "\n";
      s << "        To   = " << (*it_pre).to() << "\n";
    }
    s << "\n";
  }
  s << "\n";

  s << "Transactions" << "\n";
  s << "------------" << "\n";

  for(it_t = list_t.begin(); it_t != list_t.end(); ++it_t) {
    dumpTransaction(s, storage, *it_t);
  }
  s << "\n";


  s << "Schedules" << "\n";
  s << "---------" << "\n";

  QValueList<MyMoneySchedule> list_s = storage->scheduleList();
  QValueList<MyMoneySchedule>::ConstIterator it_s;
  for(it_s = list_s.begin(); it_s != list_s.end(); ++it_s) {
    s << "  ID = " << (*it_s).id() << "\n";
    s << "  Name = " << (*it_s).name() << "\n";
    s << "  Startdate = " << (*it_s).startDate().toString(Qt::ISODate) << "\n";
    if((*it_s).willEnd())
      s << "  Enddate   = " << (*it_s).endDate().toString(Qt::ISODate) << "\n";
    else
      s << "  Enddate   = not specified\n";
    s << "  Occurence = " << occurenceToString((*it_s).occurence()) << "\n";
    s << "  Type = " << scheduleTypeToString((*it_s).type()) << "\n";
    s << "  Paymenttype = " << paymentMethodToString((*it_s).paymentType()) << "\n";
    s << "  Fixed = " << (*it_s).isFixed() << "\n";
    s << "  AutoEnter = " << (*it_s).autoEnter() << "\n";

    if((*it_s).lastPayment().isValid())
      s << "  Last payment = " << (*it_s).lastPayment().toString(Qt::ISODate) << "\n";
    else
      s << "  Last payment = not defined" << "\n";
    if((*it_s).isFinished())
      s << "  Next payment = payment finished" << "\n";
    else {
      s << "  Next payment = " << (*it_s).nextPayment().toString(Qt::ISODate) << "\n";
      if((*it_s).isOverdue())
      s << "               = overdue!" << "\n";
    }

    QValueList<QDate> list_d;
    QValueList<QDate>::ConstIterator it_d;

    list_d = (*it_s).recordedPayments();
    if(list_d.count() > 0) {
      s << "  Recorded payments" << "\n";
      for(it_d = list_d.begin(); it_d != list_d.end(); ++it_d) {
        s << "    " << (*it_d).toString(Qt::ISODate) << "\n";
      }
    }
    s << "  TRANSACTION\n";
    dumpTransaction(s, storage, (*it_s).transaction());
  }
  s << "\n";
}

void MyMoneyStorageDump::dumpTransaction(QTextStream& s, IMyMoneyStorage* storage, const MyMoneyTransaction& it_t)
{
  s << "  ID = " << it_t.id() << "\n";
  s << "  Postdate  = " << it_t.postDate().toString(Qt::ISODate) << "\n";
  s << "  EntryDate = " << it_t.entryDate().toString(Qt::ISODate) << "\n";
  s << "  Commodity = [" << it_t.commodity() << "]\n";
  s << "  Memo = " << it_t.memo() << "\n";
  s << "  BankID = " << it_t.bankID() << "\n";
  s << "  KVP: " << "\n";
  QMap<QCString, QString>kvp = it_t.pairs();
  QMap<QCString, QString>::Iterator it;
  for(it = kvp.begin(); it != kvp.end(); ++it) {
    s << "    '" << it.key() << "' = '" << it.data() << "'\n";
  }
  s << "  Splits\n";
  s << "  ------\n";
  QValueList<MyMoneySplit>::ConstIterator it_s;
  for(it_s = it_t.splits().begin(); it_s != it_t.splits().end(); ++it_s) {
    s << "   ID = " << (*it_s).id() << "\n";
    s << "    Transaction = " << (*it_s).transactionId() << "\n";
    s << "    Payee = " << (*it_s).payeeId();
    if(!(*it_s).payeeId().isEmpty()) {
      MyMoneyPayee p = storage->payee((*it_s).payeeId());
      s << " (" << p.name() << ")" << "\n";
    } else
      s << " ()\n";
    s << "    Account = " << (*it_s).accountId();
    MyMoneyAccount acc;
    try {
      acc = storage->account((*it_s).accountId());
      s << " (" << acc.name() << ") [" << acc.currencyId() << "]\n";
    } catch (MyMoneyException *e) {
      s << " (---) [---]\n";
      delete e;
    }
    s << "    Memo = " << (*it_s).memo() << "\n";
    if((*it_s).value() == MyMoneyMoney::autoCalc)
      s << "    Value = will be calculated" << "\n";
    else
      s << "    Value = " << (*it_s).value().formatMoney()
                          << " (" << (*it_s).value().toString() << ")\n";
    s << "    Shares = " <<  (*it_s).shares().formatMoney()
                         << " (" << (*it_s).shares().toString() << ")\n";
    s << "    Action = '" << (*it_s).action() << "'\n";
    s << "    Nr = '" << (*it_s).number() << "'\n";
    s << "    ReconcileFlag = '" << reconcileToString((*it_s).reconcileFlag()) << "'\n";
    if((*it_s).reconcileFlag() != MyMoneySplit::NotReconciled) {
      s << "    ReconcileDate = " << (*it_s).reconcileDate().toString(Qt::ISODate) << "\n";
    }
    s << "\n";
  }
  s << "\n";
}

// the below code fragments are taken from KMyMoneyUtils. In order
// to keep them compatible with the source, we redefine i18n() here
// so that no translation will be necessary
#define i18n(a) QString(a)

const QString MyMoneyStorageDump::occurenceToString(const MyMoneySchedule::occurenceE occurence)
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

const QString MyMoneyStorageDump::scheduleTypeToString(MyMoneySchedule::typeE type)
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

const QString MyMoneyStorageDump::paymentMethodToString(MyMoneySchedule::paymentTypeE paymentType)
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
      text = i18n("Write check");
      break;
    case MyMoneySchedule::STYPE_ANY:
      text = i18n("Any (Error)");
      break;
  }
  return text;
}

const QString MyMoneyStorageDump::reconcileToString(MyMoneySplit::reconcileFlagE flag) const
{
  QString rc;

  switch(flag) {
    case MyMoneySplit::NotReconciled:
      rc = i18n("not reconciled");
      break;
    case MyMoneySplit::Cleared:
      rc = i18n("cleared");
      break;
    case MyMoneySplit::Reconciled:
      rc = i18n("reconciled");
      break;
    case MyMoneySplit::Frozen:
      rc = i18n("frozen");
      break;
    default:
      rc = i18n("unknown");
      break;
  }
  return rc;
}

const QString MyMoneyStorageDump::securityTypeToString(const MyMoneySecurity::eSECURITYTYPE securityType)
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
#if 0
void MyMoneyStorageDump::dumpPriceHistory(QTextStream& s, const equity_price_history history)
{
  if(history.count() != 0) {
    s << "    Price History:\n";

    equity_price_history::const_iterator it_price = history.begin();
    while ( it_price != history.end() )
    {
      s << "      " << it_price.key().toString() << ": " << it_price.data().toDouble() << "\n";
      it_price++;
    }
  }
}
#endif
