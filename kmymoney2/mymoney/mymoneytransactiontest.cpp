/***************************************************************************
                          mymoneytransactiontest.cpp
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

#include "mymoneytransactiontest.h"

MyMoneyTransactionTest::MyMoneyTransactionTest ()
{
}


void MyMoneyTransactionTest::setUp () {
	m = new MyMoneyTransaction();
}

void MyMoneyTransactionTest::tearDown () {
	delete m;
}

void MyMoneyTransactionTest::testEmptyConstructor() {
	CPPUNIT_ASSERT(m->id() == "");
	CPPUNIT_ASSERT(m->entryDate() == QDate());
	CPPUNIT_ASSERT(m->memo() == "");
	CPPUNIT_ASSERT(m->splits().count() == 0);
}

void MyMoneyTransactionTest::testSetFunctions() {
	m->setMemo("Memo");
	m->setPostDate(QDate(1,2,3));

	CPPUNIT_ASSERT(m->postDate() == QDate(1,2,3));
	CPPUNIT_ASSERT(m->memo() == "Memo");
}

void MyMoneyTransactionTest::testConstructor() {
	testSetFunctions();
	MyMoneyTransaction a("ID", *m);

	CPPUNIT_ASSERT(a.id() == "ID");
	CPPUNIT_ASSERT(a.entryDate() == QDate::currentDate());
	CPPUNIT_ASSERT(a.memo() == "Memo");
	CPPUNIT_ASSERT(a.postDate() == QDate(1,2,3));
}

void MyMoneyTransactionTest::testCopyConstructor() {
	testConstructor();
	MyMoneyTransaction a("ID", *m);

	MyMoneyTransaction n(a);

	CPPUNIT_ASSERT(n.id() == "ID");
	CPPUNIT_ASSERT(n.entryDate() == QDate::currentDate());
	CPPUNIT_ASSERT(n.memo() == "Memo");
	CPPUNIT_ASSERT(n.postDate() == QDate(1,2,3));
	
}

void MyMoneyTransactionTest::testAssignmentConstructor() {
	testConstructor();
	MyMoneyTransaction a("ID", *m);

	MyMoneyTransaction n;

	n = a;

	CPPUNIT_ASSERT(n.id() == "ID");
	CPPUNIT_ASSERT(n.entryDate() == QDate::currentDate());
	CPPUNIT_ASSERT(n.memo() == "Memo");
	CPPUNIT_ASSERT(n.postDate() == QDate(1,2,3));
}

void MyMoneyTransactionTest::testEquality() {
	testConstructor();

	MyMoneyTransaction n(*m);

	CPPUNIT_ASSERT(n == *m);
}

void MyMoneyTransactionTest::testInequality() {
	testConstructor();

	MyMoneyTransaction n(*m);

	n.setPostDate(QDate(1,1,1));
	CPPUNIT_ASSERT(!(n == *m));
}

void MyMoneyTransactionTest::testAddSplits() {
	MyMoneySplit split1, split2;
	split1.setAccountId("A000001");
	split2.setAccountId("A000002");

	try {
		CPPUNIT_ASSERT(m->accountReferenced("A000001") == false);
		CPPUNIT_ASSERT(m->accountReferenced("A000002") == false);
		m->addSplit(split1);
		m->addSplit(split2);
		CPPUNIT_ASSERT(m->splitCount() == 2);
		CPPUNIT_ASSERT(m->splits()[0].accountId() == "A000001");
		CPPUNIT_ASSERT(m->splits()[1].accountId() == "A000002");
		CPPUNIT_ASSERT(m->accountReferenced("A000001") == true);
		CPPUNIT_ASSERT(m->accountReferenced("A000002") == true);
		CPPUNIT_ASSERT(m->splits()[0].id() == "S0001");
		CPPUNIT_ASSERT(m->splits()[1].id() == "S0002");
		CPPUNIT_ASSERT(split1.id() == "S0001");
		CPPUNIT_ASSERT(split2.id() == "S0002");

	} catch(MyMoneyException *e) {
		unexpectedException(e);
	}

	// try to add split with assigned ID
	try {
		m->addSplit(split1);
		CPPUNIT_FAIL("Exception expected!");

	} catch (MyMoneyException *e) {
		delete e;
	}
}

void MyMoneyTransactionTest::testModifySplits() {
	testAddSplits();
	MyMoneySplit split;

	split = m->splits()[0];
	split.setAccountId("A000003");
	split.setId("S00000000");

	// this one should fail, because the ID is invalid
	try {
		m->modifySplit(split);
		CPPUNIT_FAIL("Exception expected");
	} catch(MyMoneyException *e) {
		delete e;
	}

	// set id to correct value, and check that it worked
	split.setId("S0001");
	try {
		m->modifySplit(split);
                CPPUNIT_ASSERT(m->splitCount() == 2);
                CPPUNIT_ASSERT(m->splits()[0].accountId() == "A000003");
                CPPUNIT_ASSERT(m->splits()[1].accountId() == "A000002");
                CPPUNIT_ASSERT(m->accountReferenced("A000001") == false);
                CPPUNIT_ASSERT(m->accountReferenced("A000002") == true);
                CPPUNIT_ASSERT(m->splits()[0].id() == "S0001");
                CPPUNIT_ASSERT(m->splits()[1].id() == "S0002");

		CPPUNIT_ASSERT(split.id() == "S0001");
		CPPUNIT_ASSERT(split.accountId() == "A000003");

	} catch(MyMoneyException *e) {
		CPPUNIT_FAIL("Unexpected exception!");
		delete e;
	}
}

void MyMoneyTransactionTest::testDeleteSplits() {
	testAddSplits();
	MyMoneySplit split;

	split.setId("S00000000");
	// this one should fail, because the ID is invalid
	try {
		m->modifySplit(split);
		CPPUNIT_FAIL("Exception expected");
	} catch(MyMoneyException *e) {
		delete e;
	}

	// set id to correct value, and check that it worked
	split.setId("S0001");
	try {
		m->removeSplit(split);
                CPPUNIT_ASSERT(m->splitCount() == 1);
                CPPUNIT_ASSERT(m->splits()[0].accountId() == "A000002");
                CPPUNIT_ASSERT(m->accountReferenced("A000001") == false);
                CPPUNIT_ASSERT(m->accountReferenced("A000002") == true);
                CPPUNIT_ASSERT(m->splits()[0].id() == "S0002");

	} catch(MyMoneyException *e) {
		CPPUNIT_FAIL("Unexpected exception!");
		delete e;
	}
}

void MyMoneyTransactionTest::testDeleteAllSplits() {
	testAddSplits();

	try {
		m->removeSplits();
		CPPUNIT_ASSERT(m->splitCount() == 0);
	} catch(MyMoneyException *e) {
		CPPUNIT_FAIL("Unexpected exception!");
		delete e;
	}
}

void MyMoneyTransactionTest::testExtractSplit() {
	testAddSplits();
	MyMoneySplit split;

	// this one should fail, as the account is not referenced by
	// any split in the transaction
	try {
		split = m->splitByAccount("A000003");
		CPPUNIT_FAIL("Exception expected");
	} catch(MyMoneyException *e) {
		delete e;
	}

	// this one should be found
	try {
		split = m->splitByAccount("A000002");
		CPPUNIT_ASSERT(split.id() == "S0002");

	} catch(MyMoneyException *e) {
		CPPUNIT_FAIL("Unexpected exception!");
		delete e;
	}

	// this one should be found also
	try {
		split = m->splitByAccount("A000002", false);
		CPPUNIT_ASSERT(split.id() == "S0001");
	} catch(MyMoneyException *e) {
		CPPUNIT_FAIL("Unexpected exception!");
		delete e;
	}
}

void MyMoneyTransactionTest::testSplitSum() {
	CPPUNIT_ASSERT(m->splitSum() == 0);

	testAddSplits();

	MyMoneySplit s1, s2;

	s1 = m->splits()[0];
	s1.setValue(0);
	s2 = m->splits()[1];
	s2.setValue(0);

	m->modifySplit(s1);
	m->modifySplit(s2);
	CPPUNIT_ASSERT(m->splitSum() == 0);

	s1.setValue(1234);
	m->modifySplit(s1);
	CPPUNIT_ASSERT(m->splitSum() == 1234);

	s2.setValue(-1234);
	m->modifySplit(s2);
	CPPUNIT_ASSERT(m->splitSum() == 0);

	s1.setValue(5678);
	m->modifySplit(s1);
	CPPUNIT_ASSERT(m->splitSum() == 4444);
}
