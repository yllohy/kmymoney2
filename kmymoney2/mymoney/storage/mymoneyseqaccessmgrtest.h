/***************************************************************************
                          mymoneyseqaccessmgrtest.h
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

#ifndef __MYMONEYSEQACCESSMGRTEST_H__
#define __MYMONEYSEQACCESSMGRTEST_H__

#include <cppunit/TestCaller.h>
#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>

class MyMoneySeqAccessMgrTest : public CppUnit::TestFixture  {
	CPPUNIT_TEST_SUITE(MyMoneySeqAccessMgrTest);
	CPPUNIT_TEST(testEmptyConstructor);
	CPPUNIT_TEST(testSetFunctions);
	CPPUNIT_TEST(testIsStandardAccount);
	CPPUNIT_TEST(testNewAccount);
	CPPUNIT_TEST(testAddNewAccount);
	CPPUNIT_TEST(testReparentAccount);
	CPPUNIT_TEST(testAddInstitution);
	CPPUNIT_TEST(testInstitution);
	CPPUNIT_TEST(testAccount2Institution);
	CPPUNIT_TEST(testModifyAccount);
	CPPUNIT_TEST(testModifyInstitution);
	CPPUNIT_TEST(testAddTransactions);
	CPPUNIT_TEST(testBalance);
	CPPUNIT_TEST(testModifyTransaction);
	CPPUNIT_TEST(testRemoveUnusedAccount);
	CPPUNIT_TEST(testRemoveUsedAccount);
	CPPUNIT_TEST(testRemoveInstitution);
	CPPUNIT_TEST(testRemoveTransaction);
	CPPUNIT_TEST(testTransactionList);
	CPPUNIT_TEST_SUITE_END();

protected:
	MyMoneySeqAccessMgr *m;
public:
	MyMoneySeqAccessMgrTest() {};


void setUp()
{
	m = new MyMoneySeqAccessMgr;
}

void tearDown()
{
	delete m;
}

void testEmptyConstructor()
{
	CPPUNIT_ASSERT(m->m_userName == "");
	CPPUNIT_ASSERT(m->m_userStreet == "");
	CPPUNIT_ASSERT(m->m_userTown == "");
	CPPUNIT_ASSERT(m->m_userCounty == "");
	CPPUNIT_ASSERT(m->m_userPostcode == "");
	CPPUNIT_ASSERT(m->m_userTelephone == "");
	CPPUNIT_ASSERT(m->m_userEmail == "");
	CPPUNIT_ASSERT(m->m_nextInstitutionID == 0);
	CPPUNIT_ASSERT(m->m_nextAccountID == 0);
	CPPUNIT_ASSERT(m->m_nextTransactionID == 0);
	CPPUNIT_ASSERT(m->m_institutionList.count() == 0);
	CPPUNIT_ASSERT(m->m_accountList.count() == 4);
	CPPUNIT_ASSERT(m->m_transactionList.count() == 0);
	CPPUNIT_ASSERT(m->m_transactionKeys.count() == 0);
	CPPUNIT_ASSERT(m->m_dirty == false);
	CPPUNIT_ASSERT(m->m_creationDate == QDate::currentDate());
}

void testSetFunctions() {
	
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

	CPPUNIT_ASSERT(m->userName() == "Name");
	CPPUNIT_ASSERT(m->userStreet() == "Street");
	CPPUNIT_ASSERT(m->userTown() == "Town");
	CPPUNIT_ASSERT(m->userCounty() == "County");
	CPPUNIT_ASSERT(m->userPostcode() == "Postcode");
	CPPUNIT_ASSERT(m->userTelephone() == "Telephone");
	CPPUNIT_ASSERT(m->userEmail() == "Email");
}

void testIsStandardAccount()
{
	CPPUNIT_ASSERT(m->isStandardAccount(STD_ACC_LIABILITY) == true);
	CPPUNIT_ASSERT(m->isStandardAccount(STD_ACC_ASSET) == true);
	CPPUNIT_ASSERT(m->isStandardAccount(STD_ACC_EXPENSE) == true);
	CPPUNIT_ASSERT(m->isStandardAccount(STD_ACC_INCOME) == true);
	CPPUNIT_ASSERT(m->isStandardAccount("A0002") == false);
}

void testNewAccount() {
	MyMoneyAccount a;

	a.setName("AccountName");
	a.setNumber("AccountNumber");

	m->newAccount(a);

	CPPUNIT_ASSERT(m->m_nextAccountID == 1);
	CPPUNIT_ASSERT(m->dirty() == true);
	CPPUNIT_ASSERT(m->m_accountList.count() == 5);
	CPPUNIT_ASSERT(m->m_accountList["A000001"].name() == "AccountName");
}

void testAccount() {
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

void testAddNewAccount() {
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

void testAddInstitution() {
	MyMoneyInstitution i;

	i.setName("Inst Name");

	m->addInstitution(i);
	CPPUNIT_ASSERT(m->m_institutionList.count() == 1);
	CPPUNIT_ASSERT(m->m_nextInstitutionID == 1);
	CPPUNIT_ASSERT(m->m_institutionList["I000001"].name() == "Inst Name");
}

void testInstitution() {
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

void testAccount2Institution() {
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

void testModifyAccount() {
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

void testModifyInstitution() {
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

void testReparentAccount() {
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
		cout << endl << e->what() << endl;
		delete e;
		CPPUNIT_FAIL("Unexpected exception");
	}
}

void testAddTransactions() {
	testReparentAccount();

	MyMoneyAccount ch;
	MyMoneyTransaction t1, t2;
	MyMoneySplit s;

	// I made some money, great
	s.setAccountId("A000006");	// Checkings
	s.setShares(100000);
	s.setValue(100000);
	t1.addSplit(s);

	s.setAccountId("A000005");	// Salary
	s.setShares(-100000);
	s.setValue(-100000);
	t1.addSplit(s);

	t1.setPostDate(QDate(2002,5,10));
	m->m_dirty = false;
	try {
		m->addTransaction(t1);
		CPPUNIT_ASSERT(m->dirty() == true);
		CPPUNIT_ASSERT(t1.id() == "T000000000000000001");
		CPPUNIT_ASSERT(t1.splitCount() == 2);
		CPPUNIT_ASSERT(m->transactionCount() == 1);
	} catch (MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception");
	}

	// I spent some money, not so great
	s.setAccountId("A000004");	// Grosseries
	s.setShares(10000);
	s.setValue(10000);
	t2.addSplit(s);

	s.setAccountId("A000002");	// 16% sales tax
	s.setShares(1200);
	s.setValue(1200);
	t2.addSplit(s);

	s.setAccountId("A000003");	// 7% sales tax
	s.setShares(400);
	s.setValue(400);
	t2.addSplit(s);

	s.setAccountId("A000006");	// Checkings account
	s.setShares(-11600);
	s.setValue(-11600);
	t2.addSplit(s);

	t2.setPostDate(QDate(2002,5,9));
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
			
	} catch (MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception");
	}
}

void testBalance() {
	testAddTransactions();

	CPPUNIT_ASSERT(m->balance("A000001") == 0);
	CPPUNIT_ASSERT(m->balance("A000002") == 1200);
	CPPUNIT_ASSERT(m->balance("A000003") == 400);
	CPPUNIT_ASSERT(m->totalBalance("A000001") == 1600);
}

void testModifyTransaction() {
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
		CPPUNIT_ASSERT(ch.transactionCount() == 2);

		QValueList<MyMoneyAccount::Transaction>::ConstIterator it_l;
		it_l = ch.transactionList().begin();
		CPPUNIT_ASSERT((*it_l).transactionID() == "T000000000000000001");
		CPPUNIT_ASSERT((*it_l).balance() == 100000);
		++it_l;

		CPPUNIT_ASSERT((*it_l).transactionID() == "T000000000000000002");
		CPPUNIT_ASSERT((*it_l).balance() == -12600+100000);

		++it_l;
		CPPUNIT_ASSERT(it_l == ch.transactionList().end());
	} catch (MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception");
	}
}


void testRemoveUnusedAccount() {
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

void testRemoveUsedAccount() {
	testAddTransactions();

	MyMoneyAccount a = m->account("A000006");

	try {
		m->removeAccount(a);
		CPPUNIT_FAIL("Exception expected");
	} catch (MyMoneyException *e) {
		delete e;
	}
}

void testRemoveInstitution() {
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
		CPPUNIT_ASSERT(a.institutionId() == "");
		CPPUNIT_ASSERT(m->institutionCount() == 0);
	} catch (MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception");
	}
}

void testRemoveTransaction() {
	testAddTransactions();

	MyMoneyTransaction t = m->transaction("T000000000000000002");

	m->m_dirty = false;
	try {
		m->removeTransaction(t);
		CPPUNIT_ASSERT(m->dirty() == true);
		CPPUNIT_ASSERT(m->transactionCount() == 1);
		CPPUNIT_ASSERT(m->account("A000006").transactionCount() == 1);

	} catch (MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception");
	}
}

void testTransactionList() {
	testAddTransactions();

	QValueList<MyMoneyTransaction> list;

	list = m->transactionList("A000006");
	CPPUNIT_ASSERT(list.count() == 2);
	CPPUNIT_ASSERT((*(list.at(0))).id() == "T000000000000000002");
	CPPUNIT_ASSERT((*(list.at(1))).id() == "T000000000000000001");

	list = m->transactionList("A000003");
	CPPUNIT_ASSERT(list.count() == 1);
	CPPUNIT_ASSERT((*(list.at(0))).id() == "T000000000000000002");

	list = m->transactionList("");
	CPPUNIT_ASSERT(list.count() == 2);
	CPPUNIT_ASSERT((*(list.at(0))).id() == "T000000000000000002");
	CPPUNIT_ASSERT((*(list.at(1))).id() == "T000000000000000001");
}

};

#endif
