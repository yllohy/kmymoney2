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

  s << "File-Info\n";
  s << "---------\n";
  s << "username = " << storage->userName() << "\n";
  s << "usercity = " << storage->userTown() << "\n";

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
    s << "\n";
  }
  s << "\n";


  s << "Accounts" << "\n";
  s << "--------" << "\n";

  QValueList<MyMoneyAccount> list_a = storage->accountList();
  list_a.push_front(storage->expense());
  list_a.push_front(storage->income());
  list_a.push_front(storage->liability());
  list_a.push_front(storage->asset());
  QValueList<MyMoneyAccount>::ConstIterator it_a;
  for(it_a = list_a.begin(); it_a != list_a.end(); ++it_a) {
    s << "  ID = " << (*it_a).id() << "\n";
    s << "  Name = " << (*it_a).name() << "\n";
    s << "  Type = " << (*it_a).accountType() << "\n";
    s << "  Parent = " << (*it_a).parentAccountId();
    if(!(*it_a).parentAccountId().isEmpty()) {
      MyMoneyAccount parent = storage->account((*it_a).parentAccountId());
      s << " (" << parent.name() << ")";
    } else {
      s << "n/a";
    }
    s << "\n";
    s << "  Balance = " << (*it_a).openingBalance().formatMoney() << "\n";
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

  s << "Transactions" << "\n";
  s << "------------" << "\n";

  MyMoneyTransactionFilter filter;
  QValueList<MyMoneyTransaction> list_t = storage->transactionList(filter);
  QValueList<MyMoneyTransaction>::ConstIterator it_t;
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
    s << "  Startdate = " << (*it_s).startDate().toString() << "\n";
    if((*it_s).willEnd())
      s << "  Enddate   = " << (*it_s).endDate().toString() << "\n";
    else
      s << "  Enddate   = not specified\n";
    s << "  Occurence = " << occurenceToString((*it_s).occurence()) << "\n";
    s << "  Type = " << scheduleTypeToString((*it_s).type()) << "\n";
    s << "  Paymenttype = " << paymentMethodToString((*it_s).paymentType()) << "\n";
    s << "  Fixed = " << (*it_s).isFixed() << "\n";
    s << "  AutoEnter = " << (*it_s).autoEnter() << "\n";

    if((*it_s).lastPayment().isValid())
      s << "  Last payment = " << (*it_s).lastPayment().toString() << "\n";
    else
      s << "  Last payment = not defined" << "\n";
    if((*it_s).isFinished())
      s << "  Next payment = payment finished" << "\n";
    else {
      s << "  Next payment = " << (*it_s).nextPayment().toString() << "\n";
      if((*it_s).isOverdue())
      s << "               = overdue!" << "\n";
    }  
    s << "  TRANSACTION\n";
    dumpTransaction(s, storage, (*it_s).transaction());    
  }
  s << "\n";
}

void MyMoneyStorageDump::dumpTransaction(QTextStream& s, IMyMoneyStorage* storage, const MyMoneyTransaction& it_t)
{
  s << "  ID = " << it_t.id() << "\n";
  s << "  Postdate  = " << it_t.postDate().toString() << "\n";
  s << "  EntryDate = " << it_t.entryDate().toString() << "\n";
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
    s << "    Payee = " << (*it_s).payeeId();
    if(!(*it_s).payeeId().isEmpty()) {
      MyMoneyPayee p = storage->payee((*it_s).payeeId());
      s << " (" << p.name() << ")" << "\n";
    } else
      s << " ()\n";
    s << "    Account = " << (*it_s).accountId();
    MyMoneyAccount acc = storage->account((*it_s).accountId());
    s << " (" << acc.name() << ")" << "\n";
    s << "    Memo = " << (*it_s).memo() << "\n";
    if((*it_s).value() == MyMoneyMoney::minValue + 1)
      s << "    Value = will be calculated" << "\n";
    else
      s << "    Value = " << (*it_s).value().formatMoney() << "\n";
    s << "    Action = '" << (*it_s).action() << "'\n";
    s << "    Nr = '" << (*it_s).number() << "'\n";
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
      text = i18n("Write cheque");
      break;
    case MyMoneySchedule::STYPE_ANY:
      text = i18n("Any (Error)");
      break;
  }
  return text;
}
