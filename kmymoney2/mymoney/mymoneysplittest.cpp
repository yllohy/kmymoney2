/***************************************************************************
                          mymoneysplittest.cpp
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

#include "mymoneysplittest.h"

MyMoneySplitTest::MyMoneySplitTest()
{
}


void MyMoneySplitTest::setUp () {
	m = new MyMoneySplit();
}

void MyMoneySplitTest::tearDown () {
	delete m;
}

void MyMoneySplitTest::testEmptyConstructor() {
	CPPUNIT_ASSERT(m->accountId() == "");
	CPPUNIT_ASSERT(m->id() == "");
	CPPUNIT_ASSERT(m->memo() == "");
	CPPUNIT_ASSERT(m->action() == "");
	CPPUNIT_ASSERT(m->shares() == 0);
	CPPUNIT_ASSERT(m->value() == 0);
	CPPUNIT_ASSERT(m->reconcileFlag() == MyMoneySplit::NotReconciled);
	CPPUNIT_ASSERT(m->reconcileDate() == QDate());
}

void MyMoneySplitTest::testSetFunctions() {
	m->setAccountId("Account");
	m->setMemo("Memo");
	m->setReconcileDate(QDate(1,2,3));
	m->setReconcileFlag(MyMoneySplit::Cleared);
	m->setShares(1234);
	m->setValue(3456);
	m->setId("MyID");
	m->setPayeeId("Payee");
	m->setAction("Action");

	CPPUNIT_ASSERT(m->accountId() == "Account");
	CPPUNIT_ASSERT(m->memo() == "Memo");
	CPPUNIT_ASSERT(m->reconcileDate() == QDate(1,2,3));
	CPPUNIT_ASSERT(m->reconcileFlag() == MyMoneySplit::Cleared);
	CPPUNIT_ASSERT(m->shares() == 1234);
	CPPUNIT_ASSERT(m->value() == 3456);
	CPPUNIT_ASSERT(m->id() == "MyID");
	CPPUNIT_ASSERT(m->payeeId() == "Payee");
	CPPUNIT_ASSERT(m->action() == "Action");
}


void MyMoneySplitTest::testCopyConstructor() {
	testSetFunctions();

	MyMoneySplit n(*m);

	CPPUNIT_ASSERT(n.accountId() == "Account");
	CPPUNIT_ASSERT(n.memo() == "Memo");
	CPPUNIT_ASSERT(n.reconcileDate() == QDate(1,2,3));
	CPPUNIT_ASSERT(n.reconcileFlag() == MyMoneySplit::Cleared);
	CPPUNIT_ASSERT(n.shares() == 1234);
	CPPUNIT_ASSERT(n.value() == 3456);
	CPPUNIT_ASSERT(n.id() == "MyID");
	CPPUNIT_ASSERT(n.payeeId() == "Payee");
	CPPUNIT_ASSERT(n.action() == "Action");
}

void MyMoneySplitTest::testAssignmentConstructor() {
	testSetFunctions();

	MyMoneySplit n;

	n = *m;

	CPPUNIT_ASSERT(n.accountId() == "Account");
	CPPUNIT_ASSERT(n.memo() == "Memo");
	CPPUNIT_ASSERT(n.reconcileDate() == QDate(1,2,3));
	CPPUNIT_ASSERT(n.reconcileFlag() == MyMoneySplit::Cleared);
	CPPUNIT_ASSERT(n.shares() == 1234);
	CPPUNIT_ASSERT(n.value() == 3456);
	CPPUNIT_ASSERT(n.id() == "MyID");
	CPPUNIT_ASSERT(n.payeeId() == "Payee");
	CPPUNIT_ASSERT(n.action() == "Action");
}

void MyMoneySplitTest::testEquality() {
	testSetFunctions();

	MyMoneySplit n(*m);

	CPPUNIT_ASSERT(n == *m);
}

void MyMoneySplitTest::testInequality() {
	testSetFunctions();

	MyMoneySplit n(*m);

	n.setShares(3456);
	CPPUNIT_ASSERT(!(n == *m));

	n = *m;
	n.setId("Not My ID");
	CPPUNIT_ASSERT(!(n == *m));

	n = *m;
	n.setPayeeId("No payee");
	CPPUNIT_ASSERT(!(n == *m));

	n = *m;
	n.setAction("No action");
	CPPUNIT_ASSERT(!(n == *m));

	n = *m;
	n.setNumber("No number");
	CPPUNIT_ASSERT(!(n == *m));

	n = *m;
	n.setAccountId("No account");
	CPPUNIT_ASSERT(!(n == *m));

	n = *m;
	n.setMemo("No memo");
	CPPUNIT_ASSERT(!(n == *m));

	n = *m;
	n.setReconcileDate(QDate(3,4,5));
	CPPUNIT_ASSERT(!(n == *m));

	n = *m;
	n.setReconcileFlag(MyMoneySplit::Frozen);
	CPPUNIT_ASSERT(!(n == *m));

	n = *m;
	n.setShares(4567);
	CPPUNIT_ASSERT(!(n == *m));

	n = *m;
	n.setValue(9876);
	CPPUNIT_ASSERT(!(n == *m));
}


void MyMoneySplitTest::testAmortization() {
	CPPUNIT_ASSERT(m->isAmortizationSplit() == false);
	testSetFunctions();
	CPPUNIT_ASSERT(m->isAmortizationSplit() == false);
	m->setAction(MyMoneySplit::ActionAmortization);
	CPPUNIT_ASSERT(m->isAmortizationSplit() == true);
}
