/***************************************************************************
                          mymoneytransactiontest.h
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

#ifndef __MYMONEYTRANSACTIONTEST_H__
#define __MYMONEYTRANSACTIONTEST_H__

#include <cppunit/TestCaller.h>
#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>

class MyMoneyTransactionTest : public CppUnit::TestFixture  {
        CPPUNIT_TEST_SUITE(MyMoneyTransactionTest);
	CPPUNIT_TEST(testEmptyConstructor);
	CPPUNIT_TEST(testSetFunctions);
	CPPUNIT_TEST(testConstructor);
	CPPUNIT_TEST(testCopyConstructor);
	CPPUNIT_TEST(testAssignmentConstructor);
	CPPUNIT_TEST(testAddSplits);
	CPPUNIT_TEST(testModifySplits);
	CPPUNIT_TEST(testDeleteSplits);
	CPPUNIT_TEST(testEquality);
	CPPUNIT_TEST(testInequality);
        CPPUNIT_TEST_SUITE_END();

protected:
	MyMoneyTransaction *m;

public:
	MyMoneyTransactionTest () {};


void setUp () {
	m = new MyMoneyTransaction();
}

void tearDown () {
	delete m;
}

void testEmptyConstructor() {
	CPPUNIT_ASSERT(m->id() == "");
	CPPUNIT_ASSERT(m->file() == 0);
	CPPUNIT_ASSERT(m->entryDate() == QDate());
	CPPUNIT_ASSERT(m->memo() == "");
	CPPUNIT_ASSERT(m->splits().count() == 0);
}

void testSetFunctions() {
	m->setMemo("Memo");
	m->setPostDate(QDate(1,2,3));

	CPPUNIT_ASSERT(m->postDate() == QDate(1,2,3));
	CPPUNIT_ASSERT(m->memo() == "Memo");
}

void testConstructor() {
	testSetFunctions();
	MyMoneyTransaction a("ID", *m);

	CPPUNIT_ASSERT(a.id() == "ID");
	CPPUNIT_ASSERT(a.entryDate() == QDate::currentDate());
	CPPUNIT_ASSERT(a.memo() == "Memo");
	CPPUNIT_ASSERT(a.postDate() == QDate(1,2,3));
}

void testCopyConstructor() {
	testConstructor();
	MyMoneyTransaction a("ID", *m);

	MyMoneyTransaction n(a);

	CPPUNIT_ASSERT(n.id() == "ID");
	CPPUNIT_ASSERT(n.entryDate() == QDate::currentDate());
	CPPUNIT_ASSERT(n.memo() == "Memo");
	CPPUNIT_ASSERT(n.postDate() == QDate(1,2,3));
	
}

void testAssignmentConstructor() {
	testConstructor();
	MyMoneyTransaction a("ID", *m);

	MyMoneyTransaction n;

	n = a;

	CPPUNIT_ASSERT(n.id() == "ID");
	CPPUNIT_ASSERT(n.entryDate() == QDate::currentDate());
	CPPUNIT_ASSERT(n.memo() == "Memo");
	CPPUNIT_ASSERT(n.postDate() == QDate(1,2,3));
}

void testEquality() {
	testConstructor();

	MyMoneyTransaction n(*m);

	CPPUNIT_ASSERT(n == *m);
}

void testInequality() {
	testConstructor();

	MyMoneyTransaction n(*m);

	n.setPostDate(QDate(1,1,1));
	CPPUNIT_ASSERT(!(n == *m));
}

void testAddSplits() {
	MyMoneySplit split1, split2;
	split1.setAccount("A000001");
	split2.setAccount("A000002");

	try {
		CPPUNIT_ASSERT(m->accountReferenced("A000001") == false);
		CPPUNIT_ASSERT(m->accountReferenced("A000002") == false);
		m->addSplit(split1);
		m->addSplit(split2);
		CPPUNIT_ASSERT(m->splitCount() == 2);
		CPPUNIT_ASSERT(m->splits()[0].account() == "A000001");
		CPPUNIT_ASSERT(m->splits()[1].account() == "A000002");
		CPPUNIT_ASSERT(m->accountReferenced("A000001") == true);
		CPPUNIT_ASSERT(m->accountReferenced("A000002") == true);
		CPPUNIT_ASSERT(m->splits()[0].id() == "S0001");
		CPPUNIT_ASSERT(m->splits()[1].id() == "S0002");

	} catch(MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception!");
	}
}

void testModifySplits() {
	testAddSplits();
	MyMoneySplit split;

	split = m->splits()[0];
	split.setAccount("A000003");
	split.setID("S00000000");

	// this one should fail, because the ID is invalid
	try {
		m->modifySplit(split);
		CPPUNIT_FAIL("Exception expected");
	} catch(MyMoneyException *e) {
		delete e;
	}

	// set id to correct value, and check that it worked
	split.setID("S0001");
	try {
		m->modifySplit(split);
                CPPUNIT_ASSERT(m->splitCount() == 2);
                CPPUNIT_ASSERT(m->splits()[0].account() == "A000003");
                CPPUNIT_ASSERT(m->splits()[1].account() == "A000002");
                CPPUNIT_ASSERT(m->accountReferenced("A000001") == false);
                CPPUNIT_ASSERT(m->accountReferenced("A000002") == true);
                CPPUNIT_ASSERT(m->splits()[0].id() == "S0001");
                CPPUNIT_ASSERT(m->splits()[1].id() == "S0002");

		CPPUNIT_ASSERT(split.id() == "S0001");
		CPPUNIT_ASSERT(split.account() == "A000003");

	} catch(MyMoneyException *e) {
		CPPUNIT_FAIL("Unexpected exception!");
		delete e;
	}
}

void testDeleteSplits() {
	testAddSplits();
	MyMoneySplit split;

	split.setID("S00000000");
	// this one should fail, because the ID is invalid
	try {
		m->modifySplit(split);
		CPPUNIT_FAIL("Exception expected");
	} catch(MyMoneyException *e) {
		delete e;
	}

	// set id to correct value, and check that it worked
	split.setID("S0001");
	try {
		m->removeSplit(split);
                CPPUNIT_ASSERT(m->splitCount() == 1);
                CPPUNIT_ASSERT(m->splits()[0].account() == "A000002");
                CPPUNIT_ASSERT(m->accountReferenced("A000001") == false);
                CPPUNIT_ASSERT(m->accountReferenced("A000002") == true);
                CPPUNIT_ASSERT(m->splits()[0].id() == "S0002");

	} catch(MyMoneyException *e) {
		CPPUNIT_FAIL("Unexpected exception!");
		delete e;
	}
}

};

#endif
