/***************************************************************************
                          imymoneystoragedump.cpp  -  description
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
    if((*it_a).parentAccountId() != "") {
      MyMoneyAccount parent = storage->account((*it_a).parentAccountId());
      s << " (" << parent.name() << ")";
    } else {
      s << "n/a";
    }
    s << "\n";
    s << "  Balance = " << (*it_a).openingBalance().formatMoney() << "\n";
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
    s << "  ID = " << (*it_t).id() << "\n";
    s << "  Postdate  = " << (*it_t).postDate().toString() << "\n";
    s << "  EntryDate = " << (*it_t).entryDate().toString() << "\n";
    s << "  KVP: " << "\n";
    QMap<QCString, QString>kvp = (*it_t).pairs();
    QMap<QCString, QString>::Iterator it;
    for(it = kvp.begin(); it != kvp.end(); ++it) {
      s << "    '" << it.key() << "' = '" << it.data() << "'\n";
    }
    s << "  Splits\n";
    s << "  ------\n";
    QValueList<MyMoneySplit>::ConstIterator it_s;
    for(it_s = (*it_t).splits().begin(); it_s != (*it_t).splits().end(); ++it_s) {
      s << "   ID = " << (*it_s).id() << "\n";
      s << "    Payee = " << (*it_s).payeeId();
      if((*it_s).payeeId() != "") {
        MyMoneyPayee p = storage->payee((*it_s).payeeId());
        s << " (" << p.name() << ")" << "\n";
      } else
        s << " ()\n";
      s << "    Account = " << (*it_s).accountId();
      MyMoneyAccount acc = storage->account((*it_s).accountId());
      s << " (" << acc.name() << ")" << "\n";
      s << "    Memo = " << (*it_s).memo() << "\n";
      s << "    Value = " << (*it_s).value().formatMoney() << "\n";
      s << "    Action = '" << (*it_s).action() << "'\n";
      s << "    Nr = '" << (*it_s).number() << "'\n";
      s << "\n";
    }
    s << "\n";
  }
  s << "\n";

}

