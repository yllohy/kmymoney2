/***************************************************************************
                          mymoneyaccounttest.cpp
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

#include "mymoneyaccounttest.h"

MyMoneyAccountTest::MyMoneyAccountTest()
{
}


void MyMoneyAccountTest::setUp () {
}

void MyMoneyAccountTest::tearDown () {
}

void MyMoneyAccountTest::testEmptyConstructor() {
	MyMoneyAccount a;

	CPPUNIT_ASSERT(a.id() == "");
	CPPUNIT_ASSERT(a.file() == 0);
	CPPUNIT_ASSERT(a.name() == "");
	CPPUNIT_ASSERT(a.accountType() == MyMoneyAccount::UnknownAccountType);
	CPPUNIT_ASSERT(a.openingDate() == QDate());
	CPPUNIT_ASSERT(a.openingBalance() == 0);
	CPPUNIT_ASSERT(a.lastModified() == QDate());
	CPPUNIT_ASSERT(a.lastReconciliationDate() == QDate());
	CPPUNIT_ASSERT(a.transactionList().count() == 0);
	CPPUNIT_ASSERT(a.accountList().count() == 0);
}

void MyMoneyAccountTest::testConstructor() {
	QCString id = "A000001";
	QString institutionid = "B000001";
	QCString parent = "Parent";
	MyMoneyAccount r;
	r.setAccountType(MyMoneyAccount::Asset);
	r.setOpeningDate(QDate::currentDate());
	r.setLastModified(QDate::currentDate());
	r.setDescription("Desc");
	r.setNumber("465500");
	r.setParentAccountId(parent);
	r.setOpeningBalance(1);

	MyMoneyAccount a(id, r);

	CPPUNIT_ASSERT(a.id() == id);
	CPPUNIT_ASSERT(a.institutionId() == "");
	CPPUNIT_ASSERT(a.accountType() == MyMoneyAccount::Asset);
	CPPUNIT_ASSERT(a.openingDate() == QDate::currentDate());
	CPPUNIT_ASSERT(a.lastModified() == QDate::currentDate());
	CPPUNIT_ASSERT(a.number() == "465500");
	CPPUNIT_ASSERT(a.description() == "Desc");
	CPPUNIT_ASSERT(a.transactionList().count() == 0);
	CPPUNIT_ASSERT(a.accountList().count() == 0);
	CPPUNIT_ASSERT(a.parentAccountId() == "Parent");
	CPPUNIT_ASSERT(a.openingBalance() == 1);
}

void MyMoneyAccountTest::testSetFunctions() {
	MyMoneyAccount a;

	QDate today(QDate::currentDate());
	CPPUNIT_ASSERT(a.name() == "");
	CPPUNIT_ASSERT(a.lastModified() == QDate());
	CPPUNIT_ASSERT(a.description() == "");

	a.setName("Account");
	a.setInstitutionId("Institution1");
	a.setLastModified(today);
	a.setDescription("Desc");
	a.setNumber("123456");
	a.setOpeningBalance(2);
	a.setAccountType(MyMoneyAccount::MoneyMarket);

	CPPUNIT_ASSERT(a.name() == "Account");
	CPPUNIT_ASSERT(a.institutionId() == "Institution1");
	CPPUNIT_ASSERT(a.lastModified() == today);
	CPPUNIT_ASSERT(a.description() == "Desc");
	CPPUNIT_ASSERT(a.number() == "123456");
	CPPUNIT_ASSERT(a.openingBalance() == 2);
	CPPUNIT_ASSERT(a.transactionList().count() == 0);
	CPPUNIT_ASSERT(a.accountType() == MyMoneyAccount::MoneyMarket);
}

void MyMoneyAccountTest::testCopyConstructor() {
	MyMoneyAccount::Transaction ta("Trans1", 1000);
	QCString id = "A000001";
	QCString institutionid = "B000001";
	QCString parent = "ParentAccount";
	MyMoneyAccount r;
	r.setAccountType(MyMoneyAccount::Expense);
	r.setOpeningDate(QDate::currentDate());
	r.setLastModified(QDate::currentDate());
	r.setName("Account");
	r.setInstitutionId("Inst1");
	r.setDescription("Desc1");
	r.setNumber("Number");
	r.setParentAccountId(parent);
	r.setOpeningBalance(3);

	MyMoneyAccount a(id, r);
	a.setInstitutionId(institutionid);
	a.addTransaction(ta);

	MyMoneyAccount b(a);

	CPPUNIT_ASSERT(b.name() == "Account");
	CPPUNIT_ASSERT(b.institutionId() == institutionid);
	CPPUNIT_ASSERT(b.accountType() == MyMoneyAccount::Expense);
	CPPUNIT_ASSERT(b.lastModified() == QDate::currentDate());
	CPPUNIT_ASSERT(b.openingDate() == QDate::currentDate());
	CPPUNIT_ASSERT(b.description() == "Desc1");
	CPPUNIT_ASSERT(b.number() == "Number");
	CPPUNIT_ASSERT(b.transactionList().count() == 1);
	CPPUNIT_ASSERT(b.parentAccountId() == "ParentAccount");
	CPPUNIT_ASSERT(b.openingBalance() == 3);

	MyMoneyAccount::Transaction tc = b.transaction(0);
	CPPUNIT_ASSERT(tc.transactionID() == "Trans1");
	CPPUNIT_ASSERT(tc.balance() == 1000);

#if 0 // make that a QList<MyMoneyTransaction> interface
	for(int i=-1; i < 2; ++i) {
		CPPUNIT_ASSERT(b.transactionID(i) == "");
	}
#endif
}

void MyMoneyAccountTest::testAssignmentConstructor() {
	MyMoneyAccount a;
	a.setAccountType(MyMoneyAccount::Checkings);
	a.setName("Account");
	a.setInstitutionId("Inst1");
	a.setDescription("Bla");
	a.setNumber("assigned Number");

	MyMoneyAccount b;

	b.setLastModified(QDate::currentDate());

	b = a;

	CPPUNIT_ASSERT(b.name() == "Account");
	CPPUNIT_ASSERT(b.institutionId() == "Inst1");
	CPPUNIT_ASSERT(b.accountType() == MyMoneyAccount::Checkings);
	CPPUNIT_ASSERT(b.lastModified() == QDate());
	CPPUNIT_ASSERT(b.openingDate() == a.openingDate());
	CPPUNIT_ASSERT(b.description() == "Bla");
	CPPUNIT_ASSERT(b.number() == "assigned Number");
	CPPUNIT_ASSERT(b.transactionList().count() == 0);

#if 0 // make the a QList<MyMoneyTransaction> interface
	for(int i=-1; i < 2; ++i) {
		CPPUNIT_ASSERT(b.transactionID(i) == "");
	}
#endif
}

void MyMoneyAccountTest::testTransactionList() {
	MyMoneyAccount a;
	a.setAccountType(MyMoneyAccount::Checkings);
	MyMoneyAccount::Transaction ta("Trans1", 1000);
	MyMoneyAccount::Transaction tb("Trans2", 2000);

	CPPUNIT_ASSERT(a.transactionList().count() == 0);
	a.addTransaction(ta);
	CPPUNIT_ASSERT(a.transactionList().count() == 1);
	a.addTransaction(tb);
	CPPUNIT_ASSERT(a.transactionList().count() == 2);

	a.clearTransactions();
	CPPUNIT_ASSERT(a.transactionList().count() == 0);
}

void MyMoneyAccountTest::testTransactionRetrieval() {
	MyMoneyAccount a;
	a.setAccountType(MyMoneyAccount::Checkings);
	MyMoneyAccount::Transaction ta("Trans1", 1000);
	MyMoneyAccount::Transaction tb("Trans2", 2000);
	MyMoneyAccount::Transaction tc;

	a.addTransaction(ta);
	a.addTransaction(tb);

	// Check access to transactions with numeric index

	try {
		tc = a.transaction(0);
		CPPUNIT_ASSERT(tc.transactionID() == "Trans1");
		CPPUNIT_ASSERT(tc.balance() == 1000);
		tc = a.transaction(1);
		CPPUNIT_ASSERT(tc.transactionID() == "Trans2");
		CPPUNIT_ASSERT(tc.balance() == 3000);
	} catch(MyMoneyException *e) {
		CPPUNIT_FAIL("Retrieving existing transaction must not throw exception!");
	}

	try {
		tc = a.transaction(2);
		CPPUNIT_FAIL("Retrieving unknown transaction must throw exception!");
	} catch(MyMoneyException *e) {
		delete e;
	}
	try {
		tc = a.transaction(-1);
		CPPUNIT_FAIL("Retrieving unknown transaction must throw exception!");
	} catch(MyMoneyException *e) {
		delete e;
	}
}

void MyMoneyAccountTest::testBalance() {
	MyMoneyAccount a;
	a.setAccountType(MyMoneyAccount::Checkings);
	MyMoneyAccount::Transaction ta("Trans1", 1000);
	MyMoneyAccount::Transaction tb("Trans2", 2000);
	MyMoneyAccount::Transaction tc("Trans3", -4000);

	CPPUNIT_ASSERT(a.balance() == 0);

	a.addTransaction(ta);
	CPPUNIT_ASSERT(a.balance() == 1000);

	a.addTransaction(tb);
	CPPUNIT_ASSERT(a.balance() == 3000);

	a.addTransaction(tc);
	CPPUNIT_ASSERT(a.balance() == -1000);

	a.setOpeningBalance(1000);
	CPPUNIT_ASSERT(a.balance() == 0);
}

void MyMoneyAccountTest::testSubAccounts()
{
	MyMoneyAccount a;
	a.setAccountType(MyMoneyAccount::Checkings);
	
	a.addAccountId("Subaccount1");
	CPPUNIT_ASSERT(a.accountList().count() == 1);
	a.addAccountId("Subaccount1");
	CPPUNIT_ASSERT(a.accountList().count() == 1);
	a.addAccountId("Subaccount2");
	CPPUNIT_ASSERT(a.accountList().count() == 2);

}

