/***************************************************************************
                          mymoneyqifwriter.cpp  -  description
                             -------------------
    begin                : Sun Jan 5 2003
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
// QT Headers

#include <qfile.h>
#include <qtextstream.h>

// ----------------------------------------------------------------------------
// KDE Headers

#include <klocale.h>
#include <kmessagebox.h>

// ----------------------------------------------------------------------------
// Project Headers

#include "mymoneyqifwriter.h"
#include "../mymoney/mymoneyfile.h"

MyMoneyQifWriter::MyMoneyQifWriter()
{
}

MyMoneyQifWriter::~MyMoneyQifWriter()
{
}

void MyMoneyQifWriter::write(const QString& filename, const QString& profile,
             const QCString& accountId, const bool accountData,
             const bool categoryData,
             const QDate& startDate, const QDate& endDate)
{
  m_qifProfile.loadProfile("Profile-" + profile);

  QFile qifFile(filename);
  if(qifFile.open(IO_WriteOnly)) {
    QTextStream s(&qifFile);

    try {
      if(categoryData) {
        writeCategoryEntries(s);
      }

      if(accountData) {
        writeAccountEntry(s, accountId, startDate, endDate);
      }
      emit signalProgress(-1, -1);

    } catch(MyMoneyException *e) {
      QString errMsg = i18n("Unexpected exception '%1' thrown in %2, line %3 "
                            "caught in MyMoneyQifWriter::write()")
                       .arg(e->what()).arg(e->file()).arg(e->line());

      KMessageBox::error(0, errMsg);
      delete e;
    }

    qifFile.close();
  } else {
    KMessageBox::error(0, i18n("Unable to open file '%1' for writing").arg(filename));
  }
}

void MyMoneyQifWriter::writeAccountEntry(QTextStream &s, const QCString& accountId, const QDate& startDate, const QDate& endDate)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  MyMoneyAccount account;

  account = file->account(accountId);
  MyMoneyTransactionFilter filter(accountId);
  QValueList<MyMoneyTransaction> list = file->transactionList(filter);

  s << "!Type:" << m_qifProfile.profileType() << endl;
  s << "D" << m_qifProfile.date(account.openingDate()) << endl;
  s << "T" << m_qifProfile.value('T', account.openingBalance()) << endl;
  s << "CX" << endl;
  s << "P" << m_qifProfile.openingBalanceText() << endl;
  s << "L";
  if(m_qifProfile.accountDelimiter()[0])
    s << m_qifProfile.accountDelimiter()[0];
  s << account.name();
  if(m_qifProfile.accountDelimiter()[1])
    s << m_qifProfile.accountDelimiter()[1];
  s << endl;
  s << "^" << endl;

  QValueList<MyMoneyTransaction>::ConstIterator it;
  signalProgress(0, list.count());
  int count = 0;
  for(it = list.begin(); it != list.end(); ++it) {
    if((*it).postDate() >= startDate && (*it).postDate() <= endDate) {
      writeTransactionEntry(s, *it, accountId);
    }
    signalProgress(++count, 0);
  }
}

void MyMoneyQifWriter::writeCategoryEntries(QTextStream &s)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  MyMoneyAccount income;
  MyMoneyAccount expense;

  income = file->income();
  expense = file->expense();

  s << "!Type:Cat" << endl;
  QCStringList list = income.accountList() + expense.accountList();
  emit signalProgress(0, list.count());
  QCStringList::Iterator it;
  int count = 0;
  for(it = list.begin(); it != list.end(); ++it) {
    writeCategoryEntry(s, *it, "");
    emit signalProgress(++count, 0);
  }
}

void MyMoneyQifWriter::writeCategoryEntry(QTextStream &s, const QCString& accountId, const QString& leadIn)
{
  MyMoneyAccount acc = MyMoneyFile::instance()->account(accountId);
  QString name = acc.name();

  s << "N" << leadIn << name << endl;
  s << (MyMoneyAccount::accountGroup(acc.accountType()) == MyMoneyAccount::Expense ? "E" : "I") << endl;
  s << "^" << endl;

  QCStringList list = acc.accountList();
  QCStringList::Iterator it;
  name += ":";
  for(it = list.begin(); it != list.end(); ++it) {
    writeCategoryEntry(s, *it, name);
  }
}

void MyMoneyQifWriter::writeTransactionEntry(QTextStream &s, const MyMoneyTransaction& t, const QCString& accountId)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  MyMoneySplit split = t.splitByAccount(accountId);

  s << "D" << m_qifProfile.date(t.postDate()) << endl;

  switch(split.reconcileFlag()) {
    case MyMoneySplit::Cleared:
      s << "C*" << endl;
      break;

    case MyMoneySplit::Reconciled:
    case MyMoneySplit::Frozen:
      s << "CX" << endl;
      break;

    default:
      break;
  }

  if(split.memo().length() > 0)
    s << "M" << split.memo() << endl;

  s << "T" << m_qifProfile.value('T', split.value()) << endl;

  if(split.number().length() > 0)
    s << "N" << split.number() << endl;

  if(!split.payeeId().isEmpty()) {
    MyMoneyPayee payee = file->payee(split.payeeId());
    s << "P" << payee.name() << endl;
  }

  MyMoneySplit sp = t.splitByAccount(accountId, false);
  MyMoneyAccount acc = file->account(sp.accountId());
  if(split.action() == MyMoneySplit::ActionTransfer) {
    s << "L" << m_qifProfile.accountDelimiter()[0]
             << acc.name()
             << m_qifProfile.accountDelimiter()[1] << endl;
  } else {
    s << "L" << file->accountToCategory(sp.accountId()) << endl;
    if(t.splitCount() > 2) {
      QValueList<MyMoneySplit> list = t.splits();
      QValueList<MyMoneySplit>::ConstIterator it;
      for(it = list.begin(); it != list.end(); ++it) {
        if(!((*it) == split)) {
          writeSplitEntry(s, *it);
        }
      }
    }
  }
  s << "^" << endl;
}

void MyMoneyQifWriter::writeSplitEntry(QTextStream& s, const MyMoneySplit& split)
{
  s << "S" << MyMoneyFile::instance()->accountToCategory(split.accountId()) << endl;
  if(split.memo().length() > 0)
    s << "E" << split.memo() << endl;

  s << "$" << m_qifProfile.value('$', -split.value()) << endl;
}
