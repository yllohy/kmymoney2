/***************************************************************************
                          mymoneyseqaccessmgrtest.cpp
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

#include "mymoneyseqaccessmgrtest.h"
#include <iostream>

MyMoneySeqAccessMgrTest::MyMoneySeqAccessMgrTest()
{
}


void MyMoneySeqAccessMgrTest::setUp()
{
	m = new MyMoneySeqAccessMgr;
}

void MyMoneySeqAccessMgrTest::tearDown()
{
	delete m;
}

void MyMoneySeqAccessMgrTest::testEmptyConstructor()
{
	CPPUNIT_ASSERT(m->m_userName.isEmpty());
	CPPUNIT_ASSERT(m->m_userStreet.isEmpty());
	CPPUNIT_ASSERT(m->m_userTown.isEmpty());
	CPPUNIT_ASSERT(m->m_userCounty.isEmpty());
	CPPUNIT_ASSERT(m->m_userPostcode.isEmpty());
	CPPUNIT_ASSERT(m->m_userTelephone.isEmpty());
	CPPUNIT_ASSERT(m->m_userEmail.isEmpty());
	CPPUNIT_ASSERT(m->m_nextInstitutionID == 0);
	CPPUNIT_ASSERT(m->m_nextAccountID == 0);
	CPPUNIT_ASSERT(m->m_nextTransactionID == 0);
	CPPUNIT_ASSERT(m->m_nextPayeeID == 0);
	CPPUNIT_ASSERT(m->m_nextScheduleID == 0);
	CPPUNIT_ASSERT(m->m_institutionList.count() == 0);
	CPPUNIT_ASSERT(m->m_accountList.count() == 4);
	CPPUNIT_ASSERT(m->m_transactionList.count() == 0);
	CPPUNIT_ASSERT(m->m_transactionKeys.count() == 0);
	CPPUNIT_ASSERT(m->m_payeeList.count() == 0);
	CPPUNIT_ASSERT(m->m_scheduleList.count() == 0);

	CPPUNIT_ASSERT(m->m_dirty == false);
	CPPUNIT_ASSERT(m->m_creationDate == QDate::currentDate());

	CPPUNIT_ASSERT(m->liability().name() == "Liability");
	CPPUNIT_ASSERT(m->asset().name() == "Asset");
	CPPUNIT_ASSERT(m->expense().name() == "Expense");
	CPPUNIT_ASSERT(m->income().name() == "Income");
}

void MyMoneySeqAccessMgrTest::testSetFunctions() {
	
	m->m_dirty = false;
	m->setUserName("Name");
	CPPUNIT_ASSERT(m->dirty() == true);
	m->m_dirty = false;
	m->setUserStreet("Street");
	CPPUNIT_ASSERT(m->dirty() == true);
	m->m_dirty = false;
	m->setUserTown("Town");
	CPPUNIT_ASSERT(m->dirty() == true);
	m->m_dirty = false;
	m->setUserCounty("County");
	CPPUNIT_ASSERT(m->dirty() == true);
	m->m_dirty = false;
	m->setUserPostcode("Postcode");
	CPPUNIT_ASSERT(m->dirty() == true);
	m->m_dirty = false;
	m->setUserTelephone("Telephone");
	CPPUNIT_ASSERT(m->dirty() == true);
	m->m_dirty = false;
	m->setUserEmail("Email");
	CPPUNIT_ASSERT(m->dirty() == true);
	m->m_dirty = false;
	m->setValue("key", "value");
	CPPUNIT_ASSERT(m->dirty() == true);

	CPPUNIT_ASSERT(m->userName() == "Name");
	CPPUNIT_ASSERT(m->userStreet() == "Street");
	CPPUNIT_ASSERT(m->userTown() == "Town");
	CPPUNIT_ASSERT(m->userCounty() == "County");
	CPPUNIT_ASSERT(m->userPostcode() == "Postcode");
	CPPUNIT_ASSERT(m->userTelephone() == "Telephone");
	CPPUNIT_ASSERT(m->userEmail() == "Email");
	CPPUNIT_ASSERT(m->value("key") == "value");

	m->m_dirty = false;
	m->deletePair("key");
	CPPUNIT_ASSERT(m->dirty() == true);
}

void MyMoneySeqAccessMgrTest::testSupportFunctions()
{
	CPPUNIT_ASSERT(m->nextInstitutionID() == "I000001");
	CPPUNIT_ASSERT(m->m_nextInstitutionID == 1);
	CPPUNIT_ASSERT(m->nextAccountID() == "A000001");
	CPPUNIT_ASSERT(m->m_nextAccountID == 1);
	CPPUNIT_ASSERT(m->nextTransactionID() == "T000000000000000001");
	CPPUNIT_ASSERT(m->m_nextTransactionID == 1);
	CPPUNIT_ASSERT(m->nextPayeeID() == "P000001");
	CPPUNIT_ASSERT(m->m_nextPayeeID == 1);
	CPPUNIT_ASSERT(m->nextScheduleID() == "SCH000001");
	CPPUNIT_ASSERT(m->m_nextScheduleID == 1);
}

void MyMoneySeqAccessMgrTest::testIsStandardAccount()
{
	CPPUNIT_ASSERT(m->isStandardAccount(STD_ACC_LIABILITY) == true);
	CPPUNIT_ASSERT(m->isStandardAccount(STD_ACC_ASSET) == true);
	CPPUNIT_ASSERT(m->isStandardAccount(STD_ACC_EXPENSE) == true);
	CPPUNIT_ASSERT(m->isStandardAccount(STD_ACC_INCOME) == true);
	CPPUNIT_ASSERT(m->isStandardAccount("A0002") == false);
}

void MyMoneySeqAccessMgrTest::testNewAccount() {
	MyMoneyAccount a;

	a.setName("AccountName");
	a.setNumber("AccountNumber");

	m->newAccount(a);

	CPPUNIT_ASSERT(m->m_nextAccountID == 1);
	CPPUNIT_ASSERT(m->dirty() == true);
	CPPUNIT_ASSERT(m->m_accountList.count() == 5);
	CPPUNIT_ASSERT(m->m_accountList["A000001"].name() == "AccountName");
}

void MyMoneySeqAccessMgrTest::testAccount() {
	testNewAccount();
	m->m_dirty = false;

	MyMoneyAccount a;

	// make sure that an invalid ID causes an exception
	try {
		a = m->account("Unknown ID");
		CPPUNIT_FAIL("Exception expected");
	} catch (MyMoneyException *e) {
		delete e;
	} 
	CPPUNIT_ASSERT(m->dirty() == false);

	// now make sure, that a real ID works
	try {
		a = m->account("A000001");
		CPPUNIT_ASSERT(a.name() == "AccountName");
		CPPUNIT_ASSERT(a.id() == "A000001");
		CPPUNIT_ASSERT(m->dirty() == true);
	} catch (MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception");
	}
}

void MyMoneySeqAccessMgrTest::testAddNewAccount() {
	testNewAccount();

	MyMoneyAccount a,b;
	b.setName("Account2");
	b.setNumber("Acc2");
	m->newAccount(b);

	m->m_dirty = false;

	CPPUNIT_ASSERT(m->m_nextAccountID == 2);
	CPPUNIT_ASSERT(m->m_accountList.count() == 6);

	// try to add account to undefined account
	try {
		MyMoneyAccount c("UnknownID", b);
		m->addAccount(c, a);
		CPPUNIT_FAIL("Exception expected");
	} catch (MyMoneyException *e) {
		delete e;
	}

	CPPUNIT_ASSERT(m->dirty() == false);
	// now try to add account 1 as sub-account to account 2
	a = m->account("A000001");
	try {
		CPPUNIT_ASSERT(m->m_accountList[STD_ACC_ASSET].accountList().count() == 0);
		m->addAccount(b, a);
		CPPUNIT_ASSERT(m->m_accountList["A000002"].accountList()[0] == "A000001");
		CPPUNIT_ASSERT(m->m_accountList["A000002"].accountList().count() == 1);
		CPPUNIT_ASSERT(m->m_accountList[STD_ACC_ASSET].accountList().count() == 0);
		CPPUNIT_ASSERT(m->dirty() == true);
	} catch (MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception");
	}
}

void MyMoneySeqAccessMgrTest::testAddInstitution() {
	MyMoneyInstitution i;

	i.setName("Inst Name");

	m->addInstitution(i);
	CPPUNIT_ASSERT(m->m_institutionList.count() == 1);
	CPPUNIT_ASSERT(m->m_nextInstitutionID == 1);
	CPPUNIT_ASSERT(m->m_institutionList["I000001"].name() == "Inst Name");
}

void MyMoneySeqAccessMgrTest::testInstitution() {
	testAddInstitution();
	MyMoneyInstitution i;

	m->m_dirty = false;

	// try to find unknown institution
	try {
		i = m->institution("Unknown ID");
		CPPUNIT_FAIL("Exception expected");
	} catch (MyMoneyException *e) {
		delete e;
	}

	CPPUNIT_ASSERT(m->dirty() == false);

	// now try to find real institution
	try {
		i = m->institution("I000001");
		CPPUNIT_ASSERT(i.name() == "Inst Name");
		CPPUNIT_ASSERT(m->dirty() == false);
	} catch (MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception");
	}
}

void MyMoneySeqAccessMgrTest::testAccount2Institution() {
	testAddInstitution();
	testAddNewAccount();

	MyMoneyInstitution i;
	MyMoneyAccount a, b;

	try {
		i = m->institution("I000001");
		a = m->account("A000001");
	} catch (MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception");
	}

	m->m_dirty = false;

	// try to add to a false institution
	MyMoneyInstitution fake("Unknown ID", i);
	try {
		m->addAccount(fake, a);
		CPPUNIT_FAIL("Exception expected");
	} catch (MyMoneyException *e) {
		delete e;
	}

	CPPUNIT_ASSERT(m->dirty() == false);
	// now try to do it with a real institution
	try {
		CPPUNIT_ASSERT(i.accountList().count() == 0);
		m->addAccount(i, a);
		CPPUNIT_ASSERT(m->dirty() == true);
		CPPUNIT_ASSERT(a.institutionId() == i.id());
		b = m->account("A000001");
		CPPUNIT_ASSERT(b.institutionId() == i.id());
		CPPUNIT_ASSERT(i.accountList().count() == 1);
	} catch (MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception");
	}
}

void MyMoneySeqAccessMgrTest::testModifyAccount() {
	testAccount2Institution();

	// test the OK case
	MyMoneyAccount a = m->account("A000001");
	a.setName("New account name");
	m->m_dirty = false;
	try {
		m->modifyAccount(a);
		MyMoneyAccount b = m->account("A000001");
		CPPUNIT_ASSERT(b.parentAccountId() == a.parentAccountId());
		CPPUNIT_ASSERT(b.name() == "New account name");
		CPPUNIT_ASSERT(m->dirty() == true);
	} catch (MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception");
	}

	// modify institution to unknown id
	MyMoneyAccount c("Unknown ID", a);
	m->m_dirty = false;
	try {
		m->modifyAccount(c);
		CPPUNIT_FAIL("Exception expected");
	} catch (MyMoneyException *e) {
		delete e;
	}

	// use different account type
	MyMoneyAccount d;
	d.setAccountType(MyMoneyAccount::CreditCard);
	MyMoneyAccount f("A000001", d);
	try {
		m->modifyAccount(f);
		CPPUNIT_FAIL("Exception expected");
	} catch (MyMoneyException *e) {
		delete e;
	}

	// use different parent
	a.setParentAccountId("A000002");
	try {
		m->modifyAccount(c);
		CPPUNIT_FAIL("Exception expected");
	} catch (MyMoneyException *e) {
		delete e;
	}
}

void MyMoneySeqAccessMgrTest::testModifyInstitution() {
	testAddInstitution();
	MyMoneyInstitution i = m->institution("I000001");

	m->m_dirty = false;
	i.setName("New inst name");
	try {
		m->modifyInstitution(i);
		CPPUNIT_ASSERT(m->dirty() == true);
		i = m->institution("I000001");
		CPPUNIT_ASSERT(i.name() == "New inst name");

	} catch (MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception");
	}
	
	// try to modify an institution that does not exist
	MyMoneyInstitution f("Unknown ID", i);
	try {
		m->modifyInstitution(f);
		CPPUNIT_FAIL("Exception expected");
	} catch (MyMoneyException *e) {
		delete e;
	}
}

void MyMoneySeqAccessMgrTest::testReparentAccount() {
	// this one adds some accounts to the database
	MyMoneyAccount ex1;
	ex1.setAccountType(MyMoneyAccount::Expense);
	MyMoneyAccount ex2;
	ex2.setAccountType(MyMoneyAccount::Expense);
	MyMoneyAccount ex3;
	ex3.setAccountType(MyMoneyAccount::Expense);
	MyMoneyAccount ex4;
	ex4.setAccountType(MyMoneyAccount::Expense);
	MyMoneyAccount in;
	in.setAccountType(MyMoneyAccount::Income);
	MyMoneyAccount ch;
	ch.setAccountType(MyMoneyAccount::Checkings);

	ex1.setName("Sales Tax");
	ex2.setName("Sales Tax 16%");
	ex3.setName("Sales Tax 7%");
	ex4.setName("Grosseries");

	in.setName("Salary");
	ch.setName("My checkings account");

	try {
		m->newAccount(ex1);
		m->newAccount(ex2);
		m->newAccount(ex3);
		m->newAccount(ex4);
		m->newAccount(in);
		m->newAccount(ch);

		CPPUNIT_ASSERT(ex1.id() == "A000001");
		CPPUNIT_ASSERT(ex2.id() == "A000002");
		CPPUNIT_ASSERT(ex3.id() == "A000003");
		CPPUNIT_ASSERT(ex4.id() == "A000004");
		CPPUNIT_ASSERT(in.id() == "A000005");
		CPPUNIT_ASSERT(ch.id() == "A000006");

		MyMoneyAccount parent = m->expense();

		m->addAccount(parent, ex1);
		m->addAccount(ex1, ex2);
		m->addAccount(parent, ex3);
		m->addAccount(parent, ex4);

		parent = m->income();
		m->addAccount(parent, in);

		parent = m->asset();
		m->addAccount(parent, ch);

		CPPUNIT_ASSERT(m->expense().accountCount() == 3);
		CPPUNIT_ASSERT(m->account(ex1.id()).accountCount() == 1);
		CPPUNIT_ASSERT(ex3.parentAccountId() == STD_ACC_EXPENSE);

		m->reparentAccount(ex3, ex1);
		CPPUNIT_ASSERT(m->expense().accountCount() == 2);
		CPPUNIT_ASSERT(m->account(ex1.id()).accountCount() == 2);
		CPPUNIT_ASSERT(ex3.parentAccountId() == ex1.id());
	} catch (MyMoneyException *e) {
		std::cout << std::endl << e->what() << std::endl;
		delete e;
		CPPUNIT_FAIL("Unexpected exception");
	}
}

void MyMoneySeqAccessMgrTest::testAddTransactions() {
	testReparentAccount();

	MyMoneyAccount ch;
	MyMoneyTransaction t1, t2;
	MyMoneySplit s;

	try {
		// I made some money, great
		s.setAccountId("A000006");	// Checkings
		s.setShares(100000);
		s.setValue(100000);
		CPPUNIT_ASSERT(s.id().isEmpty());
		t1.addSplit(s);

		s.setId(QCString());	// enable re-usage of split variable
		s.setAccountId("A000005");	// Salary
		s.setShares(-100000);
		s.setValue(-100000);
		CPPUNIT_ASSERT(s.id().isEmpty());
		t1.addSplit(s);

		t1.setPostDate(QDate(2002,5,10));
	} catch (MyMoneyException *e) {
		unexpectedException(e);
	}

	m->m_dirty = false;
	try {
		m->addTransaction(t1);
		CPPUNIT_ASSERT(m->dirty() == true);
		CPPUNIT_ASSERT(t1.id() == "T000000000000000001");
		CPPUNIT_ASSERT(t1.splitCount() == 2);
		CPPUNIT_ASSERT(m->transactionCount() == 1);
	} catch (MyMoneyException *e) {
		unexpectedException(e);
	}

	try {
		// I spent some money, not so great
		s.setId(QCString());	// enable re-usage of split variable
		s.setAccountId("A000004");	// Grosseries
		s.setShares(10000);
		s.setValue(10000);
		CPPUNIT_ASSERT(s.id().isEmpty());
		t2.addSplit(s);

		s.setId(QCString());	// enable re-usage of split variable
		s.setAccountId("A000002");	// 16% sales tax
		s.setShares(1200);
		s.setValue(1200);
		CPPUNIT_ASSERT(s.id().isEmpty());
		t2.addSplit(s);

		s.setId(QCString());	// enable re-usage of split variable
		s.setAccountId("A000003");	// 7% sales tax
		s.setShares(400);
		s.setValue(400);
		CPPUNIT_ASSERT(s.id().isEmpty());
		t2.addSplit(s);

		s.setId(QCString());	// enable re-usage of split variable
		s.setAccountId("A000006");	// Checkings account
		s.setShares(-11600);
		s.setValue(-11600);
		CPPUNIT_ASSERT(s.id().isEmpty());
		t2.addSplit(s);

		t2.setPostDate(QDate(2002,5,9));
	} catch (MyMoneyException *e) {
		unexpectedException(e);
	}
	m->m_dirty = false;
	try {
		m->addTransaction(t2);
		CPPUNIT_ASSERT(m->dirty() == true);
		CPPUNIT_ASSERT(t2.id() == "T000000000000000002");
		CPPUNIT_ASSERT(t2.splitCount() == 4);
		CPPUNIT_ASSERT(m->transactionCount() == 2);

		QMap<QCString, QCString>::ConstIterator it_k;
		QMap<QCString, MyMoneyTransaction>::ConstIterator it_t;
		it_k = m->m_transactionKeys.begin();
		it_t = m->m_transactionList.begin();

		CPPUNIT_ASSERT((*it_k) == "2002-05-10-T000000000000000001");
		CPPUNIT_ASSERT((*it_t).id() == "T000000000000000002");
		++it_k;
		++it_t;
		CPPUNIT_ASSERT((*it_k) == "2002-05-09-T000000000000000002");
		CPPUNIT_ASSERT((*it_t).id() == "T000000000000000001");
		++it_k;
		++it_t;
		CPPUNIT_ASSERT(it_k == m->m_transactionKeys.end());
		CPPUNIT_ASSERT(it_t == m->m_transactionList.end());

		ch = m->account("A000006");

		// check that the account's transaction list is updated
		QValueList<MyMoneyTransaction> list;
		MyMoneyTransactionFilter filter("A000006");
		list = m->transactionList(filter);
		CPPUNIT_ASSERT(list.size() == 2);

		QValueList<MyMoneyTransaction>::ConstIterator it;
		it = list.begin();
		CPPUNIT_ASSERT((*it).id() == "T000000000000000002");
		++it;
		CPPUNIT_ASSERT((*it).id() == "T000000000000000001");
		++it;
		CPPUNIT_ASSERT(it == list.end());

/* removed with MyMoneyAccount::Transaction
		CPPUNIT_ASSERT(ch.transactionCount() == 2);

		QValueList<MyMoneyAccount::Transaction>::ConstIterator it_l;
		it_l = ch.transactionList().begin();
		CPPUNIT_ASSERT((*it_l).transactionID() == "T000000000000000002");
		CPPUNIT_ASSERT((*it_l).balance() == -11600);
		++it_l;

		CPPUNIT_ASSERT((*it_l).transactionID() == "T000000000000000001");
		CPPUNIT_ASSERT((*it_l).balance() == -11600+100000);

		++it_l;
		CPPUNIT_ASSERT(it_l == ch.transactionList().end());
*/
			
	} catch (MyMoneyException *e) {
		unexpectedException(e);
	}
}

void MyMoneySeqAccessMgrTest::testTransactionCount() {
	testAddTransactions();
	CPPUNIT_ASSERT(m->transactionCount("A000001") == 0);
	CPPUNIT_ASSERT(m->transactionCount("A000002") == 1);
	CPPUNIT_ASSERT(m->transactionCount("A000003") == 1);
	CPPUNIT_ASSERT(m->transactionCount("A000004") == 1);
	CPPUNIT_ASSERT(m->transactionCount("A000005") == 1);
	CPPUNIT_ASSERT(m->transactionCount("A000006") == 2);
}

void MyMoneySeqAccessMgrTest::testBalance() {
	testAddTransactions();

	CPPUNIT_ASSERT(m->balance("A000001") == 0);
	CPPUNIT_ASSERT(m->balance("A000002") == 1200);
	CPPUNIT_ASSERT(m->balance("A000003") == 400);
	CPPUNIT_ASSERT(m->totalBalance("A000001") == 1600);
	CPPUNIT_ASSERT(m->balance("A000006", QDate(2002,5,9)) == -11600);
}

void MyMoneySeqAccessMgrTest::testModifyTransaction() {
	testAddTransactions();

	MyMoneyTransaction t = m->transaction("T000000000000000002");
	MyMoneySplit s;
	MyMoneyAccount ch;

	// just modify simple stuff (splits)
	CPPUNIT_ASSERT(t.splitCount() == 4);

	s = t.splits()[0];
	s.setShares(11000);
	s.setValue(11000);
	t.modifySplit(s);

	CPPUNIT_ASSERT(t.splitCount() == 4);
	s = t.splits()[3];
	s.setShares(-12600);
	s.setValue(-12600);
	t.modifySplit(s);

	try {
		CPPUNIT_ASSERT(m->balance("A000004") == 10000);
		CPPUNIT_ASSERT(m->balance("A000006") == 100000-11600);
		CPPUNIT_ASSERT(m->totalBalance("A000001") == 1600);
		m->modifyTransaction(t);
		CPPUNIT_ASSERT(m->balance("A000004") == 11000);
		CPPUNIT_ASSERT(m->balance("A000006") == 100000-12600);
		CPPUNIT_ASSERT(m->totalBalance("A000001") == 1600);
	} catch (MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception");
	}

	// now modify the date
	t.setPostDate(QDate(2002,5,11));
	try {
		m->modifyTransaction(t);
		CPPUNIT_ASSERT(m->balance("A000004") == 11000);
		CPPUNIT_ASSERT(m->balance("A000006") == 100000-12600);
		CPPUNIT_ASSERT(m->totalBalance("A000001") == 1600);

		QMap<QCString, QCString>::ConstIterator it_k;
		QMap<QCString, MyMoneyTransaction>::ConstIterator it_t;
		it_k = m->m_transactionKeys.begin();
		it_t = m->m_transactionList.begin();
		CPPUNIT_ASSERT((*it_k) == "2002-05-10-T000000000000000001");
		CPPUNIT_ASSERT((*it_t).id() == "T000000000000000001");
		++it_k;
		++it_t;
		CPPUNIT_ASSERT((*it_k) == "2002-05-11-T000000000000000002");
		CPPUNIT_ASSERT((*it_t).id() == "T000000000000000002");
		++it_k;
		++it_t;
		CPPUNIT_ASSERT(it_k == m->m_transactionKeys.end());
		CPPUNIT_ASSERT(it_t == m->m_transactionList.end());

		ch = m->account("A000006");

		// check that the account's transaction list is updated
		QValueList<MyMoneyTransaction> list;
		MyMoneyTransactionFilter filter("A000006");
		list = m->transactionList(filter);
		CPPUNIT_ASSERT(list.size() == 2);

		QValueList<MyMoneyTransaction>::ConstIterator it;
		it = list.begin();
		CPPUNIT_ASSERT((*it).id() == "T000000000000000001");
		++it;
		CPPUNIT_ASSERT((*it).id() == "T000000000000000002");
		++it;
		CPPUNIT_ASSERT(it == list.end());

/* removed with MyMoneyAccount::Transaction
		// CPPUNIT_ASSERT(ch.transactionCount() == 2);

		QValueList<MyMoneyAccount::Transaction>::ConstIterator it_l;
		it_l = ch.transactionList().begin();
		CPPUNIT_ASSERT((*it_l).transactionID() == "T000000000000000001");
		CPPUNIT_ASSERT((*it_l).balance() == 100000);
		++it_l;

		CPPUNIT_ASSERT((*it_l).transactionID() == "T000000000000000002");
		CPPUNIT_ASSERT((*it_l).balance() == -12600+100000);

		++it_l;
		CPPUNIT_ASSERT(it_l == ch.transactionList().end());
*/
	} catch (MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception");
	}
}


void MyMoneySeqAccessMgrTest::testRemoveUnusedAccount() {
	testAccount2Institution();

	MyMoneyAccount a = m->account("A000001");
	MyMoneyInstitution i = m->institution("I000001");

	m->m_dirty = false;
	// make sure, we cannot remove the standard account groups
	try {
		m->removeAccount(m->liability());
		CPPUNIT_FAIL("Exception expected");
	} catch (MyMoneyException *e) {
		delete e;
	}

	try {
		m->removeAccount(m->asset());
		CPPUNIT_FAIL("Exception expected");
	} catch (MyMoneyException *e) {
		delete e;
	}

	try {
		m->removeAccount(m->expense());
		CPPUNIT_FAIL("Exception expected");
	} catch (MyMoneyException *e) {
		delete e;
	}

	try {
		m->removeAccount(m->income());
		CPPUNIT_FAIL("Exception expected");
	} catch (MyMoneyException *e) {
		delete e;
	}

	// now really remove an account
	try {
		CPPUNIT_ASSERT(i.accountCount() == 1);
		CPPUNIT_ASSERT(m->accountCount() == 6);

		m->removeAccount(a);
		CPPUNIT_ASSERT(m->accountCount() == 5);
		CPPUNIT_ASSERT(m->dirty() == true);
		i = m->institution("I000001");
		CPPUNIT_ASSERT(i.accountCount() == 0);
	} catch (MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception");
	}
}

void MyMoneySeqAccessMgrTest::testRemoveUsedAccount() {
	testAddTransactions();

	MyMoneyAccount a = m->account("A000006");

	try {
		m->removeAccount(a);
		CPPUNIT_FAIL("Exception expected");
	} catch (MyMoneyException *e) {
		delete e;
	}
}

void MyMoneySeqAccessMgrTest::testRemoveInstitution() {
	testModifyInstitution();
	testReparentAccount();

	MyMoneyInstitution i;
	MyMoneyAccount a;

	// assign the checkings account to the institution
	try {
		i = m->institution("I000001");
		a = m->account("A000006");
		m->addAccount(i, a);
		CPPUNIT_ASSERT(i.accountCount() == 1);
	} catch (MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception");
	}

	m->m_dirty = false;
	// now remove the institution and see if the account survived ;-)
	try {
		m->removeInstitution(i);
		a = m->account("A000006");
		CPPUNIT_ASSERT(m->dirty() == true);
		CPPUNIT_ASSERT(a.institutionId().isEmpty());
		CPPUNIT_ASSERT(m->institutionCount() == 0);
	} catch (MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception");
	}
}

void MyMoneySeqAccessMgrTest::testRemoveTransaction() {
	testAddTransactions();

	MyMoneyTransaction t = m->transaction("T000000000000000002");

	m->m_dirty = false;
	try {
		m->removeTransaction(t);
		CPPUNIT_ASSERT(m->dirty() == true);
		CPPUNIT_ASSERT(m->transactionCount() == 1);
/* removed with MyMoneyAccount::Transaction
		CPPUNIT_ASSERT(m->account("A000006").transactionCount() == 1);
*/
	} catch (MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception");
	}
}

void MyMoneySeqAccessMgrTest::testTransactionList() {
	testAddTransactions();

	QValueList<MyMoneyTransaction> list;
	MyMoneyTransactionFilter filter("A000006");
	list = m->transactionList(filter);
	CPPUNIT_ASSERT(list.count() == 2);
	CPPUNIT_ASSERT((*(list.at(0))).id() == "T000000000000000002");
	CPPUNIT_ASSERT((*(list.at(1))).id() == "T000000000000000001");

	filter.clear();
	filter.addAccount("A000003");
	list = m->transactionList(filter);
	CPPUNIT_ASSERT(list.count() == 1);
	CPPUNIT_ASSERT((*(list.at(0))).id() == "T000000000000000002");

	filter.clear();
	list = m->transactionList(filter);
	CPPUNIT_ASSERT(list.count() == 2);
	CPPUNIT_ASSERT((*(list.at(0))).id() == "T000000000000000002");
	CPPUNIT_ASSERT((*(list.at(1))).id() == "T000000000000000001");
}

void MyMoneySeqAccessMgrTest::testAddPayee() {
	MyMoneyPayee p;

	p.setName("THB");
	m->m_dirty = false;
	try {
		CPPUNIT_ASSERT(m->m_nextPayeeID == 0);
		m->addPayee(p);
		CPPUNIT_ASSERT(m->dirty() == true);
		CPPUNIT_ASSERT(m->m_nextPayeeID == 1);
	} catch (MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception");
	}

}

void MyMoneySeqAccessMgrTest::testSetAccountName() {
	try {
		m->setAccountName(STD_ACC_LIABILITY, "Verbindlichkeiten");
	} catch (MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception");
	}
	try {
		m->setAccountName(STD_ACC_ASSET, "Verm�gen");
	} catch (MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception");
	}
	try {
		m->setAccountName(STD_ACC_EXPENSE, "Ausgaben");
	} catch (MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception");
	}
	try {
		m->setAccountName(STD_ACC_INCOME, "Einnahmen");
	} catch (MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception");
	}

	CPPUNIT_ASSERT(m->liability().name() == "Verbindlichkeiten");
	CPPUNIT_ASSERT(m->asset().name() == "Verm�gen");
	CPPUNIT_ASSERT(m->expense().name() == "Ausgaben");
	CPPUNIT_ASSERT(m->income().name() == "Einnahmen");

	try {
		m->setAccountName("A000001", "New account name");
		CPPUNIT_FAIL("Exception expected");
	} catch (MyMoneyException *e) {
		delete e;
	}
}

void MyMoneySeqAccessMgrTest::testModifyPayee() {
	MyMoneyPayee p;

	testAddPayee();

	p = m->payee("P000001");
	p.setName("New name");
	m->m_dirty = false;
	try {
		m->modifyPayee(p);
		p = m->payee("P000001");
		CPPUNIT_ASSERT(p.name() == "New name");
		CPPUNIT_ASSERT(m->dirty() == true);
	} catch (MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception");
	}
}

void MyMoneySeqAccessMgrTest::testRemovePayee() {
	testAddPayee();
	m->m_dirty = false;

	// check that we can remove an unreferenced payee
	MyMoneyPayee p = m->payee("P000001");
	try {
		CPPUNIT_ASSERT(m->m_payeeList.count() == 1);
		m->removePayee(p);
		CPPUNIT_ASSERT(m->m_payeeList.count() == 0);
		CPPUNIT_ASSERT(m->dirty() == true);
	} catch (MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception");
	}

	// add transaction
	testAddTransactions();

	MyMoneyTransaction tr = m->transaction("T000000000000000001");
	MyMoneySplit sp;
	sp = tr.splits()[0];
	sp.setPayeeId("P000001");
	tr.modifySplit(sp);

	// check that we cannot add a transaction referencing
	// an unknown payee
	try {
		m->modifyTransaction(tr);
		CPPUNIT_FAIL("Expected exception");
	} catch (MyMoneyException *e) {
		delete e;
	} 

	m->m_nextPayeeID = 0;		// reset here, so that the
					// testAddPayee will not fail
	testAddPayee();

	// check that it works when the payee exists
	try {
		m->modifyTransaction(tr);
	} catch (MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception");
	}

	m->m_dirty = false;

	// now check, that we cannot remove the payee
	try {
		m->removePayee(p);
		CPPUNIT_FAIL("Expected exception");
	} catch (MyMoneyException *e) {
		delete e;
	} 
	CPPUNIT_ASSERT(m->m_payeeList.count() == 1);
}


void MyMoneySeqAccessMgrTest::testRemoveAccountFromTree() {
	MyMoneyAccount a, b, c;
	a.setName("Acc A");
	b.setName("Acc B");
	c.setName("Acc C");

	// build a tree A -> B -> C, remove B and see if A -> C
	// remains in the storag manager

	try {
		m->newAccount(a);
		m->newAccount(b);
		m->newAccount(c);
		m->reparentAccount(b, a);
		m->reparentAccount(c, b);

		CPPUNIT_ASSERT(a.accountList().count() == 1);
		CPPUNIT_ASSERT(m->account(a.accountList()[0]).name() == "Acc B");

		CPPUNIT_ASSERT(b.accountList().count() == 1);
		CPPUNIT_ASSERT(m->account(b.accountList()[0]).name() == "Acc C");

		CPPUNIT_ASSERT(c.accountList().count() == 0);

		m->removeAccount(b);

		// reload account info from titutionIDtorage
		a = m->account(a.id());
		c = m->account(c.id());

		try {
			b = m->account(b.id());
			CPPUNIT_FAIL("Exception expected");
		} catch (MyMoneyException *e) {
			delete e;
		}
		CPPUNIT_ASSERT(a.accountList().count() == 1);
		CPPUNIT_ASSERT(m->account(a.accountList()[0]).name() == "Acc C");

		CPPUNIT_ASSERT(c.accountList().count() == 0);

	} catch (MyMoneyException *e) {
		unexpectedException(e);
	}
}

void MyMoneySeqAccessMgrTest::testPayeeName() {
	testAddPayee();

	MyMoneyPayee p;
	QString name("THB");

	// OK case
	try {
		p = m->payeeByName(name);
		CPPUNIT_ASSERT(p.name() == "THB");
		CPPUNIT_ASSERT(p.id() == "P000001");
	} catch (MyMoneyException *e) {
		unexpectedException(e);
	}

	// Not OK case
	name = "Thb";
	try {
		p = m->payeeByName(name);
		CPPUNIT_FAIL("Exception expected");
	} catch (MyMoneyException *e) {
		delete e;
	}
}

void MyMoneySeqAccessMgrTest::testAssignment() {
	testAddTransactions();

	m->setUserName("Thomas");

	MyMoneySeqAccessMgr test = *m;
	testEquality(&test);
}

void MyMoneySeqAccessMgrTest::testEquality(const MyMoneySeqAccessMgr *t)
{
	CPPUNIT_ASSERT( m->m_userName == t->m_userName);
	CPPUNIT_ASSERT( m->m_userStreet == t->m_userStreet);
	CPPUNIT_ASSERT( m->m_userTown == t->m_userTown);
	CPPUNIT_ASSERT( m->m_userCounty == t->m_userCounty);
	CPPUNIT_ASSERT( m->m_userPostcode == t->m_userPostcode);
	CPPUNIT_ASSERT( m->m_userTelephone == t->m_userTelephone);
	CPPUNIT_ASSERT( m->m_userEmail == t->m_userEmail);
	CPPUNIT_ASSERT( m->m_nextInstitutionID == t->m_nextInstitutionID);
	CPPUNIT_ASSERT( m->m_nextAccountID == t->m_nextAccountID);
	CPPUNIT_ASSERT( m->m_nextTransactionID == t->m_nextTransactionID);
	CPPUNIT_ASSERT( m->m_nextPayeeID == t->m_nextPayeeID);
	CPPUNIT_ASSERT( m->m_nextScheduleID == t->m_nextScheduleID);
	CPPUNIT_ASSERT( m->m_dirty == t->m_dirty);
	CPPUNIT_ASSERT( m->m_creationDate == t->m_creationDate);
	CPPUNIT_ASSERT( m->m_lastModificationDate == t->m_lastModificationDate);

	/*
	 * make sure, that the keys and values are the same
	 * on the left and the right side
	 */
	CPPUNIT_ASSERT(m->m_payeeList.keys() == t->m_payeeList.keys());
	CPPUNIT_ASSERT(m->m_payeeList.values() == t->m_payeeList.values());
	CPPUNIT_ASSERT(m->m_transactionKeys.keys() == t->m_transactionKeys.keys()); 
	CPPUNIT_ASSERT(m->m_transactionKeys.values() == t->m_transactionKeys.values()); 
	CPPUNIT_ASSERT(m->m_institutionList.keys() == t->m_institutionList.keys()); 
	CPPUNIT_ASSERT(m->m_institutionList.values() == t->m_institutionList.values()); 
	CPPUNIT_ASSERT(m->m_accountList.keys() == t->m_accountList.keys()); 
	CPPUNIT_ASSERT(m->m_accountList.values() == t->m_accountList.values()); 
	CPPUNIT_ASSERT(m->m_transactionList.keys() == t->m_transactionList.keys()); 
	CPPUNIT_ASSERT(m->m_transactionList.values() == t->m_transactionList.values()); 
	CPPUNIT_ASSERT(m->m_balanceCache.keys() == t->m_balanceCache.keys()); 
	CPPUNIT_ASSERT(m->m_balanceCache.values() == t->m_balanceCache.values()); 

//	CPPUNIT_ASSERT(m->m_scheduleList.keys() == t->m_scheduleList.keys());
//	CPPUNIT_ASSERT(m->m_scheduleList.values() == t->m_scheduleList.values());
}

void MyMoneySeqAccessMgrTest::testDuplicate() {
	const MyMoneySeqAccessMgr* t;

	testModifyTransaction();

	t = m->duplicate();
	testEquality(t);
	delete t;
}

void MyMoneySeqAccessMgrTest::testAddSchedule() {
	/* Note addSchedule() now calls validate as it should
	 * so we need an account id.  Later this will
	 * be checked to make sure its a valid account id.  The
	 * tests currently fail because no splits are defined
	 * for the schedules transaction.
	*/


	try {
	CPPUNIT_ASSERT(m->m_scheduleList.count() == 0);
	MyMoneyTransaction t1;
	MyMoneySplit s1, s2;
	s1.setAccountId("A000001");
	t1.addSplit(s1);
	s2.setAccountId("A000002");
	t1.addSplit(s2);
	MyMoneySchedule schedule("Sched-Name",
				 MyMoneySchedule::TYPE_DEPOSIT,
				 MyMoneySchedule::OCCUR_DAILY,
				 MyMoneySchedule::STYPE_MANUALDEPOSIT,
				 QDate(2003,7,10),
				 QDate(),
				 true,
				 false);
	schedule.setTransaction(t1);

		m->addSchedule(schedule);

		CPPUNIT_ASSERT(m->m_scheduleList.count() == 1);
		CPPUNIT_ASSERT(schedule.id() == "SCH000001");
		CPPUNIT_ASSERT(m->m_scheduleList["SCH000001"].id() == "SCH000001");
	} catch(MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception");
	}

	try {
		MyMoneySchedule schedule("Sched-Name",
					 MyMoneySchedule::TYPE_DEPOSIT,
					 MyMoneySchedule::OCCUR_DAILY,
					 MyMoneySchedule::STYPE_MANUALDEPOSIT,
					 QDate(2003,7,10),
					 QDate(),
					 true,
					 false);
		m->addSchedule(schedule);
		CPPUNIT_FAIL("Exception expected");
	} catch(MyMoneyException *e) {
		delete e;
	}
}

void MyMoneySeqAccessMgrTest::testSchedule() {
	testAddSchedule();
	MyMoneySchedule sched;

	sched = m->schedule("SCH000001");
	CPPUNIT_ASSERT(sched.name() == "Sched-Name");
	CPPUNIT_ASSERT(sched.id() == "SCH000001");

	try {
		m->schedule("SCH000002");
		CPPUNIT_FAIL("Exception expected");
	} catch(MyMoneyException *e) {
		delete e;
	}
}

void MyMoneySeqAccessMgrTest::testModifySchedule() {
	testAddSchedule();
	MyMoneySchedule sched;

	sched = m->schedule("SCH000001");
	sched.setId("SCH000002");
	try {
		m->modifySchedule(sched);
		CPPUNIT_FAIL("Exception expected");
	} catch(MyMoneyException *e) {
		delete e;
	}

	sched = m->schedule("SCH000001");
	sched.setName("New Sched-Name");
	try {
		m->modifySchedule(sched);
		CPPUNIT_ASSERT(m->m_scheduleList.count() == 1);
		CPPUNIT_ASSERT(m->m_scheduleList["SCH000001"].name() == "New Sched-Name");
		
	} catch(MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception");
	}
	
}

void MyMoneySeqAccessMgrTest::testRemoveSchedule() {
	testAddSchedule();
	MyMoneySchedule sched;

	sched = m->schedule("SCH000001");
	sched.setId("SCH000002");
	try {
		m->removeSchedule(sched);
		CPPUNIT_FAIL("Exception expected");
	} catch(MyMoneyException *e) {
		delete e;
	}

	sched = m->schedule("SCH000001");
	try {
		m->removeSchedule(sched);
		CPPUNIT_ASSERT(m->m_scheduleList.count() == 0);
		
	} catch(MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception");
	}
}

void MyMoneySeqAccessMgrTest::testScheduleList() {
	QDate	testDate = QDate::currentDate();
	QDate	notOverdue = testDate.addDays(2);
	QDate	overdue = testDate.addDays(-2);


	MyMoneyTransaction t1;
	MyMoneySplit s1, s2;
	s1.setAccountId("A000001");
	t1.addSplit(s1);
	s2.setAccountId("A000002");
	t1.addSplit(s2);
	MyMoneySchedule schedule1("Schedule 1",
				 MyMoneySchedule::TYPE_BILL,
				 MyMoneySchedule::OCCUR_ONCE,
				 MyMoneySchedule::STYPE_DIRECTDEBIT,
				 notOverdue,
				 QDate(),
				 false,
				 false);
	schedule1.setTransaction(t1);
	schedule1.setLastPayment(notOverdue);

	MyMoneyTransaction t2;
	MyMoneySplit s3, s4;
	s3.setAccountId("A000001");
	t2.addSplit(s3);
	s4.setAccountId("A000003");
	t2.addSplit(s4);
	MyMoneySchedule schedule2("Schedule 2",
				 MyMoneySchedule::TYPE_DEPOSIT,
				 MyMoneySchedule::OCCUR_DAILY,
				 MyMoneySchedule::STYPE_DIRECTDEPOSIT,
				 notOverdue.addDays(1),
				 QDate(),
				 false,
				 false);
	schedule2.setTransaction(t2);
	schedule2.setLastPayment(notOverdue.addDays(1));

	MyMoneyTransaction t3;
	MyMoneySplit s5, s6;
	s5.setAccountId("A000005");
	t3.addSplit(s5);
	s6.setAccountId("A000006");
	t3.addSplit(s6);
	MyMoneySchedule schedule3("Schedule 3",
				 MyMoneySchedule::TYPE_TRANSFER,
				 MyMoneySchedule::OCCUR_WEEKLY,
				 MyMoneySchedule::STYPE_OTHER,
				 notOverdue.addDays(2),
				 QDate(),
				 false,
				 false);
	schedule3.setTransaction(t3);
	schedule3.setLastPayment(notOverdue.addDays(2));

	MyMoneyTransaction t4;
	MyMoneySplit s7, s8;
	s7.setAccountId("A000005");
	t4.addSplit(s7);
	s8.setAccountId("A000006");
	t4.addSplit(s8);
	MyMoneySchedule schedule4("Schedule 4",
				 MyMoneySchedule::TYPE_BILL,
				 MyMoneySchedule::OCCUR_WEEKLY,
				 MyMoneySchedule::STYPE_WRITECHEQUE,
				 overdue.addDays(-7),
				 notOverdue.addDays(31),
				 false,
				 false);
	schedule4.setTransaction(t4);

	try {
		m->addSchedule(schedule1);
		m->addSchedule(schedule2);
		m->addSchedule(schedule3);
		m->addSchedule(schedule4);
	} catch(MyMoneyException *e) {
		qDebug("Error: %s", e->what().latin1());
		delete e;
		CPPUNIT_FAIL("Unexpected exception");
	}

	QValueList<MyMoneySchedule> list;

	// no filter
	list = m->scheduleList();
	CPPUNIT_ASSERT(list.count() == 4);

	// filter by type
	list = m->scheduleList("", MyMoneySchedule::TYPE_BILL);
	CPPUNIT_ASSERT(list.count() == 2);
	CPPUNIT_ASSERT(list[0].name() == "Schedule 1");
	CPPUNIT_ASSERT(list[1].name() == "Schedule 4");

	// filter by occurence
	list = m->scheduleList("", MyMoneySchedule::TYPE_ANY,
				MyMoneySchedule::OCCUR_DAILY);
	CPPUNIT_ASSERT(list.count() == 1);
	CPPUNIT_ASSERT(list[0].name() == "Schedule 2");

	// filter by payment type
	list = m->scheduleList("", MyMoneySchedule::TYPE_ANY,
				MyMoneySchedule::OCCUR_ANY,
				MyMoneySchedule::STYPE_DIRECTDEPOSIT);
	CPPUNIT_ASSERT(list.count() == 1);
	CPPUNIT_ASSERT(list[0].name() == "Schedule 2");

	// filter by account
	list = m->scheduleList("A01");
	CPPUNIT_ASSERT(list.count() == 0);
	list = m->scheduleList("A000001");
	CPPUNIT_ASSERT(list.count() == 2);
	list = m->scheduleList("A000002");
	CPPUNIT_ASSERT(list.count() == 1);

	// filter by start date
	list = m->scheduleList("", MyMoneySchedule::TYPE_ANY,
				MyMoneySchedule::OCCUR_ANY,
				MyMoneySchedule::STYPE_ANY,
				notOverdue.addDays(31));
	CPPUNIT_ASSERT(list.count() == 3);
	CPPUNIT_ASSERT(list[0].name() == "Schedule 2");
	CPPUNIT_ASSERT(list[1].name() == "Schedule 3");
	CPPUNIT_ASSERT(list[2].name() == "Schedule 4");

	// filter by end date
	list = m->scheduleList("", MyMoneySchedule::TYPE_ANY,
				MyMoneySchedule::OCCUR_ANY,
				MyMoneySchedule::STYPE_ANY,
				QDate(),
				notOverdue.addDays(1));
	CPPUNIT_ASSERT(list.count() == 3);
	CPPUNIT_ASSERT(list[0].name() == "Schedule 1");
	CPPUNIT_ASSERT(list[1].name() == "Schedule 2");
	CPPUNIT_ASSERT(list[2].name() == "Schedule 4");

	// filter by start and end date
	list = m->scheduleList("", MyMoneySchedule::TYPE_ANY,
				MyMoneySchedule::OCCUR_ANY,
				MyMoneySchedule::STYPE_ANY,
				notOverdue.addDays(-1),
				notOverdue.addDays(1));
	CPPUNIT_ASSERT(list.count() == 2);
	CPPUNIT_ASSERT(list[0].name() == "Schedule 1");
	CPPUNIT_ASSERT(list[1].name() == "Schedule 2");

	// filter by overdue status
	list = m->scheduleList("", MyMoneySchedule::TYPE_ANY,
				MyMoneySchedule::OCCUR_ANY,
				MyMoneySchedule::STYPE_ANY,
				QDate(),
				QDate(),
				true);
	CPPUNIT_ASSERT(list.count() == 1);
	CPPUNIT_ASSERT(list[0].name() == "Schedule 4");
}

void MyMoneySeqAccessMgrTest::testAddCurrency()
{
	MyMoneyCurrency curr("EUR", "Euro", "?", 100, 100);
	CPPUNIT_ASSERT(m->m_currencyList.count() == 0);
	m->m_dirty = false;
	try {
		m->addCurrency(curr);
		CPPUNIT_ASSERT(m->m_currencyList.count() == 1);
		CPPUNIT_ASSERT(m->m_currencyList["EUR"].name() == "Euro");
		CPPUNIT_ASSERT(m->dirty() == true);
	} catch(MyMoneyException *e) {
                delete e;
                CPPUNIT_FAIL("Unexpected exception");
	}

	m->m_dirty = false;
	try {
		m->addCurrency(curr);
                CPPUNIT_FAIL("Expected exception missing");
	} catch(MyMoneyException *e) {
		CPPUNIT_ASSERT(m->dirty() == false);
                delete e;
	}
}

void MyMoneySeqAccessMgrTest::testModifyCurrency()
{
	MyMoneyCurrency curr("EUR", "Euro", "?", 100, 100);
	testAddCurrency();
	m->m_dirty = false;
	curr.setName("EURO");
	try {
		m->modifyCurrency(curr);
		CPPUNIT_ASSERT(m->m_currencyList.count() == 1);
		CPPUNIT_ASSERT(m->m_currencyList["EUR"].name() == "EURO");
		CPPUNIT_ASSERT(m->dirty() == true);
	} catch(MyMoneyException *e) {
                delete e;
                CPPUNIT_FAIL("Unexpected exception");
	}

	m->m_dirty = false;

	MyMoneyCurrency unknownCurr("DEM", "Deutsche Mark", "DM", 100, 100);
	try {
		m->modifyCurrency(unknownCurr);
                CPPUNIT_FAIL("Expected exception missing");
	} catch(MyMoneyException *e) {
		CPPUNIT_ASSERT(m->dirty() == false);
                delete e;
	}
}

void MyMoneySeqAccessMgrTest::testRemoveCurrency()
{
	MyMoneyCurrency curr("EUR", "Euro", "?", 100, 100);
	testAddCurrency();
	m->m_dirty = false;
	try {
		m->removeCurrency(curr);
		CPPUNIT_ASSERT(m->m_currencyList.count() == 0);
		CPPUNIT_ASSERT(m->dirty() == true);
	} catch(MyMoneyException *e) {
                delete e;
                CPPUNIT_FAIL("Unexpected exception");
	}

	m->m_dirty = false;

	MyMoneyCurrency unknownCurr("DEM", "Deutsche Mark", "DM", 100, 100);
	try {
		m->removeCurrency(unknownCurr);
                CPPUNIT_FAIL("Expected exception missing");
	} catch(MyMoneyException *e) {
		CPPUNIT_ASSERT(m->dirty() == false);
                delete e;
	}
}

void MyMoneySeqAccessMgrTest::testCurrency()
{
	MyMoneyCurrency curr("EUR", "Euro", "?", 100, 100);
	MyMoneyCurrency newCurr;
	testAddCurrency();
	m->m_dirty = false;
	try {
		newCurr = m->currency("EUR");
		CPPUNIT_ASSERT(m->dirty() == false);
		CPPUNIT_ASSERT(newCurr.id() == curr.id());
		CPPUNIT_ASSERT(newCurr.name() == curr.name());
	} catch(MyMoneyException *e) {
                delete e;
                CPPUNIT_FAIL("Unexpected exception");
	}

	try {
		m->currency("DEM");
                CPPUNIT_FAIL("Expected exception missing");
	} catch(MyMoneyException *e) {
		CPPUNIT_ASSERT(m->dirty() == false);
                delete e;
	}
}

void MyMoneySeqAccessMgrTest::testCurrencyList()
{
	CPPUNIT_ASSERT(m->currencyList().count() == 0);

	testAddCurrency();
	CPPUNIT_ASSERT(m->currencyList().count() == 1);

	MyMoneyCurrency unknownCurr("DEM", "Deutsche Mark", "DM", 100, 100);
	try {
		m->addCurrency(unknownCurr);
		m->m_dirty = false;
		CPPUNIT_ASSERT(m->m_currencyList.count() == 2);
		CPPUNIT_ASSERT(m->currencyList().count() == 2);
		CPPUNIT_ASSERT(m->dirty() == false);
	} catch(MyMoneyException *e) {
                delete e;
                CPPUNIT_FAIL("Unexpected exception");
	}
}
