/***************************************************************************
                          mymoneystoragebintest.h
                          -------------------
    copyright            : (C) 2002 by Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __MYMONEYSTORAGEBINTEST_H__
#define __MYMONEYSTORAGEBINTEST_H__

#include <cppunit/TestCaller.h>
#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>

#include <unistd.h>

#include <qfile.h>
#include <mymoneyseqaccessmgr.h>
#include <mymoneystoragedump.h>

class MyMoneyStorageBinTest : public CppUnit::TestFixture  {
  CPPUNIT_TEST_SUITE(MyMoneyStorageBinTest);
  CPPUNIT_TEST(testEmptyConstructor);
  CPPUNIT_TEST(testReadOldV3MyMoneyFile);
  CPPUNIT_TEST(testReadOldV4MyMoneyFile);
  CPPUNIT_TEST(testReadOldMyMoneyFileEx);
  CPPUNIT_TEST_SUITE_END();

protected:
  MyMoneyStorageBin *m;
public:
  MyMoneyStorageBinTest() {};


void setUp()
{
  m = new MyMoneyStorageBin;
}

void tearDown()
{
  delete m;
}

void testEmptyConstructor()
{
}


void appendV3FileInfo(QDataStream& s, QString version, unsigned long magic)
{
  s << version;
  s << (Q_INT32)magic;
  s << (Q_INT32)0;    // no password
  s << (Q_INT32)0;    // no encryption
  s << QString("");    // empty password
  if(magic == 6) {
    s << QString("old name field");
  }
  s << QString("Thomas Baumgart")
    << QString("Testallee")
    << QString("Testtown")
    << QString("Testcounty")
    << QString("TestZIP")
    << QString("Testphone")
    << QString("testmail");
  s << QDate(1,2,3)   // creation date
    << QDate(4,5,6)   // last access date
    << QDate(7,8,9);  // last modification date

  s << (Q_INT32) 1;    // institution count
  s << QString("I-dummy");

  s << (Q_INT32) 1;    // category count

  s << (Q_INT32) 1;    // expense
  s << QString("Major-category");
  s << (Q_UINT32) 1;    // minor cat count
  s << QString("Minor-category");


  s << (Q_INT32) 1;    // payee count

  s << QString("Payee-name");
  s << QString("Payee-address");
  s << QString("Payee-zip");
  s << QString("Payee-phone");
  s << QString("Payee-mail");

}

void appendV3Institution(QDataStream& s,
       const QString name,
       const QString city,
       const QString street,
       const QString postcode,
       const QString telephone,
       const QString manager,
       const QString sortcode)
{
  s << name << city << street << postcode
    << telephone << manager << sortcode;
}

void appendV3Account(QDataStream& s,
      unsigned long magic,
      const QString& name,
      const QString& desc,
      const QString& number,
      const Q_INT32& type,
      const QDate& open,
      const QDate& reconcile,
      const MyMoneyMoney& balance)
{
  s << name << desc << number << type;
  if(magic == 0x00000007) {
    s << open;
    double amount;
    amount = balance.value() / 100;
    s << amount;
  }
  s << reconcile;
}

void appendV3Transaction(QDataStream& s,
        unsigned int id,
        const QString& number,
        const QString& payee,
        double amount,
        const QDate& date,
        Q_UINT32 method,
        const QString& major,
        const QString& minor,
        const QString& atmBank,
        const QString& fromAccount,
        const QString& toAccount,
        const QString& memo,
        Q_INT32 state)
{
  s << id
    << number
    << payee
    << amount
    << date
    << method
    << major
    << minor
    << atmBank
    << fromAccount
    << toAccount
    << memo
    << state;
}

void createOldFile(const QString version, unsigned long magic) {
  QFile f("old.kmy");
  f.open(IO_WriteOnly);
  CPPUNIT_ASSERT(f.isOpen() == true);
  
  QDataStream s(&f);

  appendV3FileInfo(s, version, magic);

  s << (Q_INT32)1;    // 1 institution

  appendV3Institution(s, "I-name", "I-city", "I-street",
        "I-zip", "I-phone", "I-manager", "I-sort");

  s << (Q_INT32)2;    // two accounts

  appendV3Account(s, magic, "A-Name1", "A-Desc1", "A-Number1",
    (Q_INT32)0,     // checkings account
    QDate(7,8,9),    // opening date
    QDate(4,5,6),    // last reconcile
    MyMoneyMoney(123400));

  s << (Q_INT32)3;    // three transactions

  // Cheque transaction
  appendV3Transaction(s, 0, "Number1", "Payee1",
    1.23, QDate(1,2,3), (Q_UINT32) 0, 
    "Major-category", "Minor-category", "", "A-Name1", "", "Memo1",
    (Q_INT32)0);

  // Deposit transaction (of transfer)
  appendV3Transaction(s, 0, "Number2", "Payee2",
    3.56, QDate(4,5,6), (Q_UINT32) 1,
    "<A-Name2>", "", "", "A-Name1", "", "Memo2",
    (Q_INT32)1);

  // Transfer transaction (with deposit)
  appendV3Transaction(s, 0, "Number3", "Payee3",
    -3.56, QDate(5,6,7), (Q_UINT32) 2,
    "<A-Name2>", "", "", "A-Name1", "", "Memo3",
    (Q_INT32)2);

  appendV3Account(s, magic, "A-Name2", "A-Desc2", "A-Number2",
    (Q_INT32)0,     // checkings account
    QDate(1,2,3),    // opening date
    QDate(7,9,7),    // last reconcile
    MyMoneyMoney(-432100));
    
  s << (Q_INT32)2;    // two transactions

  appendV3Transaction(s, 0, "Number4", "Payee4",
    3.56, QDate(4,5,6), (Q_UINT32) 2,
    "<A-Name1>", "", "", "A-Name2", "", "Memo4",
    (Q_INT32)1);

  appendV3Transaction(s, 0, "Number5", "Payee5",
    3.56, QDate(5,6,7), (Q_UINT32) 1,
    "<A-Name1>", "", "", "A-Name2", "", "Memo5",
    (Q_INT32)2);

  f.close();
}

void testReadOldMyMoneyFile(const QString version, unsigned long magic) {
  MyMoneySeqAccessMgr storage;

  CPPUNIT_ASSERT(storage.institutionCount() == 0);
  CPPUNIT_ASSERT(storage.accountCount() == 4);
  CPPUNIT_ASSERT(storage.transactionCount() == 0);

  createOldFile(version, magic);
  QFile f( "old.kmy" );
  f.open( IO_ReadOnly );
  CPPUNIT_ASSERT(f.isOpen() == true);

  QDataStream s(&f);

  storage.m_dirty = true;

  m->readStream(s, &storage);

  f.close();
  unlink("old.kmy");

  CPPUNIT_ASSERT(storage.dirty() == false);
  CPPUNIT_ASSERT(storage.userName() == "Thomas Baumgart");
  CPPUNIT_ASSERT(storage.userStreet() == "Testallee");
  CPPUNIT_ASSERT(storage.userTown() == "Testtown");
  CPPUNIT_ASSERT(storage.userCounty() == "Testcounty");
  CPPUNIT_ASSERT(storage.userPostcode() == "TestZIP");
  CPPUNIT_ASSERT(storage.userTelephone() == "Testphone");
  CPPUNIT_ASSERT(storage.userEmail() == "testmail");

  CPPUNIT_ASSERT(storage.creationDate() == QDate(1,2,3));
  CPPUNIT_ASSERT(storage.lastModificationDate() == QDate(7,8,9));

  CPPUNIT_ASSERT(storage.institutionCount() == 1);
  // we assume four basic account groups, two old accounts, a major
  // and a minor category. That's eight accounts
  CPPUNIT_ASSERT(storage.accountCount() == 8);
  CPPUNIT_ASSERT(storage.transactionCount() == 3);

  // get account information
  MyMoneyAccount a1, a2;
  try {
    a1 = storage.account("A000003");
    a2 = storage.account("A000004");
  } catch(MyMoneyException *e) {
    cout << e->what() << endl;
    delete e;
  }

  // check account one
  CPPUNIT_ASSERT(a1.name() == "A-Name1");
  CPPUNIT_ASSERT(a1.number() == "A-Number1");
  CPPUNIT_ASSERT(a1.description() == "A-Desc1");

  if(magic == 0x07) {
    CPPUNIT_ASSERT(a1.openingDate() == QDate(7,8,9));
    CPPUNIT_ASSERT(a1.openingBalance() == 123400);
  } else {
    CPPUNIT_ASSERT(a1.openingBalance() == 0);
    CPPUNIT_ASSERT(a1.openingDate() == QDate(1970,1,1));
  }
  CPPUNIT_ASSERT(a1.lastReconciliationDate() == QDate(4,5,6));

  // check account two
  CPPUNIT_ASSERT(a2.name() == "A-Name2");
  CPPUNIT_ASSERT(a2.number() == "A-Number2");
  CPPUNIT_ASSERT(a2.description() == "A-Desc2");

  if(magic == 0x07) {
    CPPUNIT_ASSERT(a2.openingDate() == QDate(1,2,3));
    CPPUNIT_ASSERT(a2.openingBalance() == -432100);
  } else {
    CPPUNIT_ASSERT(a2.openingBalance() == 0);
    CPPUNIT_ASSERT(a2.openingDate() == QDate(1970,1,1));
  }
  CPPUNIT_ASSERT(a2.lastReconciliationDate() == QDate(7,9,7));


  // check transactions in account one
  CPPUNIT_ASSERT(a1.transactionCount() == 3);

  MyMoneyTransaction t;
  MyMoneySplit sp1, sp2;

  // check first transaction
  try {
    t = storage.transaction("A000003", 0);
  } catch(MyMoneyException *e) {
    cout << e->what() << endl;
    delete e;
  }
  CPPUNIT_ASSERT(t.splitCount() == 2);
  CPPUNIT_ASSERT(t.id() == "T000000000000000001");
  CPPUNIT_ASSERT(t.postDate() == QDate(1,2,3));
  // CPPUNIT_ASSERT(t.method() == MyMoneyCheckingTransaction::Cheque);

  sp1 = t.splits()[0];
  sp2 = t.splits()[1];

  CPPUNIT_ASSERT(sp1.accountId() == "A000003");
  CPPUNIT_ASSERT(sp1.value() == -123);
  CPPUNIT_ASSERT(sp1.memo() == "Memo1");
  CPPUNIT_ASSERT(sp1.reconcileFlag() == MyMoneySplit::Cleared);
  CPPUNIT_ASSERT(sp1.reconcileDate() == QDate());

  CPPUNIT_ASSERT(sp2.accountId() == "A000002");
  CPPUNIT_ASSERT(sp2.value() == 123);
  CPPUNIT_ASSERT(sp2.memo() == "Memo1");
  CPPUNIT_ASSERT(sp2.reconcileFlag() == MyMoneySplit::Cleared);
  CPPUNIT_ASSERT(sp2.reconcileDate() == QDate());

  // CPPUNIT_ASSERT(t.payee() == "Payee1");
  // CPPUNIT_ASSERT(t.number() == "Number1");

  // check second transaction
  try {
    t = storage.transaction("A000003", 1);
  } catch(MyMoneyException *e) {
    cout << e->what() << endl;
    delete e;
  }
  CPPUNIT_ASSERT(t.splitCount() == 2);
  CPPUNIT_ASSERT(t.id() == "T000000000000000002");
  CPPUNIT_ASSERT(t.postDate() == QDate(4,5,6));
  // CPPUNIT_ASSERT(t.method() == MyMoneyCheckingTransaction::Cheque);

  sp1 = t.splits()[0];
  sp2 = t.splits()[1];

  CPPUNIT_ASSERT(sp1.accountId() == "A000003");
  CPPUNIT_ASSERT(sp1.value() == 356);
  CPPUNIT_ASSERT(sp1.memo() == "Memo2");
  CPPUNIT_ASSERT(sp1.reconcileFlag() == MyMoneySplit::Reconciled);
  CPPUNIT_ASSERT(sp1.reconcileDate() == QDate(4,5,6));

  CPPUNIT_ASSERT(sp2.accountId() == "A000004");
  CPPUNIT_ASSERT(sp2.value() == -356);
  CPPUNIT_ASSERT(sp2.memo() == "Memo2");
  CPPUNIT_ASSERT(sp2.reconcileFlag() == MyMoneySplit::Reconciled);
  CPPUNIT_ASSERT(sp2.reconcileDate() == QDate(4,5,6));

  // check third transaction
  try {
    t = storage.transaction("A000003", 2);
  } catch(MyMoneyException *e) {
    cout << e->what() << endl;
    delete e;
  }
  CPPUNIT_ASSERT(t.splitCount() == 2);
  CPPUNIT_ASSERT(t.id() == "T000000000000000003");
  CPPUNIT_ASSERT(t.postDate() == QDate(5,6,7));
  // CPPUNIT_ASSERT(t.method() == MyMoneyCheckingTransaction::Cheque);

  sp1 = t.splits()[0];
  sp2 = t.splits()[1];

  CPPUNIT_ASSERT(sp1.accountId() == "A000004");
  CPPUNIT_ASSERT(sp1.value() == 356);
  CPPUNIT_ASSERT(sp1.memo() == "Memo5");
  CPPUNIT_ASSERT(sp1.reconcileFlag() == MyMoneySplit::NotReconciled);
  CPPUNIT_ASSERT(sp1.reconcileDate() == QDate());

  CPPUNIT_ASSERT(sp2.accountId() == "A000003");
  CPPUNIT_ASSERT(sp2.value() == -356);
  CPPUNIT_ASSERT(sp2.memo() == "Memo5");
  CPPUNIT_ASSERT(sp2.reconcileFlag() == MyMoneySplit::NotReconciled);
  CPPUNIT_ASSERT(sp2.reconcileDate() == QDate());

  // check transactions in account two
  CPPUNIT_ASSERT(a2.transactionCount() == 2);

  t = storage.transaction("A000004", 0);
  CPPUNIT_ASSERT(t.splitCount() == 2);
  CPPUNIT_ASSERT(t.id() == "T000000000000000002");
  CPPUNIT_ASSERT(t.postDate() == QDate(4,5,6));

  t = storage.transaction("A000004", 1);
  CPPUNIT_ASSERT(t.splitCount() == 2);
  CPPUNIT_ASSERT(t.id() == "T000000000000000003");
  CPPUNIT_ASSERT(t.postDate() == QDate(5,6,7));

  QFile g( "old.asc" );
  g.open( IO_WriteOnly );
  CPPUNIT_ASSERT(g.isOpen() == true);

  QDataStream st(&g);

  MyMoneyStorageDump dumper;
  dumper.writeStream(st, &storage);
  g.close();  
}

void testReadOldV3MyMoneyFile() {
  testReadOldMyMoneyFile("0.3.3", 0x00000006);
}

void testReadOldV4MyMoneyFile () {
  testReadOldMyMoneyFile("0.4.0", 0x00000007);
}

void testReadNewMyMoneyFile () {
/*
  int rc;
  QFile f( "v4.kmy" );
  f.open( IO_ReadOnly );
  CPPUNIT_ASSERT(f.isOpen() == true);

  QDataStream s(&f);

  rc = m->readStream(s);
  CPPUNIT_ASSERT(rc == 0);
*/
}

void testReadOldMyMoneyFileEx() {
  MyMoneySeqAccessMgr storage;

  CPPUNIT_ASSERT(storage.institutionCount() == 0);
  CPPUNIT_ASSERT(storage.accountCount() == 4);
  CPPUNIT_ASSERT(storage.transactionCount() == 0);

  QFile f( "test.kmy" );
  f.open( IO_ReadOnly );
  if(!f.isOpen())
    return;

  CPPUNIT_ASSERT(f.isOpen() == true);

  QDataStream s(&f);

  m->readStream(s, &storage);

  f.close();

  QFile g( "test.asc" );
  g.open( IO_WriteOnly );
  CPPUNIT_ASSERT(g.isOpen() == true);

  QDataStream st(&g);

  MyMoneyStorageDump dumper;
  dumper.writeStream(st, &storage);
  g.close();
}


/*
 * ideas for more tests
 * --------------------
 * - delete an institution with active accounts and transactions
 *
 * things that need to be added
 * ----------------------------
 * - adding, removing payee
 * - last access date
 */

};

#endif
