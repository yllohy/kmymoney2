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

void MyMoneyStorageDump::readStream(QDataStream& s, IMyMoneySerialize* storage)
{
  qDebug("Reading not supported by MyMoneyStorageDump!!");
}

void MyMoneyStorageDump::writeStream(QDataStream& _s, IMyMoneySerialize* _storage)
{
  QTextStream s(_s.device());
  IMyMoneyStorage* storage = dynamic_cast<IMyMoneyStorage *> (_storage);

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
  QValueList<MyMoneyAccount>::ConstIterator it_a;
  for(it_a = list_a.begin(); it_a != list_a.end(); ++it_a) {
    s << "  ID = " << (*it_a).id() << "\n";
    s << "  Name = " << (*it_a).name() << "\n";
    s << "  Parent = " << (*it_a).parentAccountId();
    MyMoneyAccount parent = storage->account((*it_a).parentAccountId());
    s << " (" << parent.name() << ")" << "\n";
    // s << "  Balance = " << (*it_a).balance().formatMoney() << "\n";
    s << "\n";
  }
  s << "\n";

  s << "Transactions" << "\n";
  s << "------------" << "\n";

  QValueList<MyMoneyTransaction> list_t = storage->transactionList();
  QValueList<MyMoneyTransaction>::ConstIterator it_t;
  for(it_t = list_t.begin(); it_t != list_t.end(); ++it_t) {
    s << "  ID = " << (*it_t).id() << "\n";
    s << "  Postdate = " << (*it_t).postDate().toString() << "\n";
    s << "  Splits\n";
    s << "  ------\n";
    QValueList<MyMoneySplit>::ConstIterator it_s;
    for(it_s = (*it_t).splits().begin(); it_s != (*it_t).splits().end(); ++it_s) {
      s << "   ID = " << (*it_s).id() << "\n";
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

