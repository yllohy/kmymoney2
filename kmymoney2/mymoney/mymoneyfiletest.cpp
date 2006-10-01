/***************************************************************************
                          mymoneyfiletest.cpp
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

#include "mymoneyfiletest.h"
#include <iostream>

#include <memory>
#include <unistd.h>
#include <qfile.h>
#include <qdatastream.h>

MyMoneyFileTest:: MyMoneyFileTest () {};


void MyMoneyFileTest::setUp () {
	observer = new TestObserverSet;
	hierarchyObserver = new TestObserverSet;
	storage = new MyMoneySeqAccessMgr;
	m = MyMoneyFile::instance();
	m->attachStorage(storage);
}

void MyMoneyFileTest::tearDown () {
	m->detachStorage(storage);
	delete m;
	delete storage;
	delete observer;
	delete hierarchyObserver;
}

void MyMoneyFileTest::testEmptyConstructor() {
	MyMoneyPayee user = m->user();

	CPPUNIT_ASSERT(user.name().isEmpty());
	CPPUNIT_ASSERT(user.address().isEmpty());
	CPPUNIT_ASSERT(user.city().isEmpty());
	CPPUNIT_ASSERT(user.state().isEmpty());
	CPPUNIT_ASSERT(user.postcode().isEmpty());
	CPPUNIT_ASSERT(user.telephone().isEmpty());
	CPPUNIT_ASSERT(user.email().isEmpty());

	CPPUNIT_ASSERT(m->institutionCount() == 0);
	CPPUNIT_ASSERT(m->dirty() == false);
	CPPUNIT_ASSERT(m->accountCount() == 5);
}

void MyMoneyFileTest::testAddOneInstitution() {
	MyMoneyInstitution institution;

	institution.setName("institution1");
	institution.setTown("town");
	institution.setStreet("street");
	institution.setPostcode("postcode");
	institution.setTelephone("telephone");
	institution.setManager("manager");
	institution.setSortcode("sortcode");

	// MyMoneyInstitution institution_file("", institution);
	MyMoneyInstitution institution_id("I000002", institution);
	MyMoneyInstitution institution_noname(institution);
	institution_noname.setName(QString());

	QString id;

	CPPUNIT_ASSERT(m->institutionCount() == 0);
	storage->m_dirty = false;

	try {
		m->addInstitution(institution);
		CPPUNIT_ASSERT(institution.id() == "I000001");
		CPPUNIT_ASSERT(m->institutionCount() == 1);
		CPPUNIT_ASSERT(m->dirty() == true);
	} catch(MyMoneyException *e) {
		CPPUNIT_FAIL("Unexpected exception");
		delete e;
	}

	try {
		m->addInstitution(institution_id);
		CPPUNIT_FAIL("Missing expected exception");
	} catch(MyMoneyException *e) {
		CPPUNIT_ASSERT(m->institutionCount() == 1);
		delete e;
	}

	try {
		m->addInstitution(institution_noname);
		CPPUNIT_FAIL("Missing expected exception");
	} catch(MyMoneyException *e) {
		CPPUNIT_ASSERT(m->institutionCount() == 1);
		delete e;
	}
}

void MyMoneyFileTest::testAddTwoInstitutions() {
	testAddOneInstitution();
	MyMoneyInstitution institution;
	institution.setName("institution2");
	institution.setTown("town");
	institution.setStreet("street");
	institution.setPostcode("postcode");
	institution.setTelephone("telephone");
	institution.setManager("manager");
	institution.setSortcode("sortcode");

	QString id;

	storage->m_dirty = false;
	try {
		m->addInstitution(institution);

		CPPUNIT_ASSERT(institution.id() == "I000002");
		CPPUNIT_ASSERT(m->institutionCount() == 2);
		CPPUNIT_ASSERT(m->dirty() == true);
	} catch(MyMoneyException *e) {
		CPPUNIT_FAIL("Unexpected exception");
		delete e;
	}

	storage->m_dirty = false;

	try {
		institution = m->institution("I000001");
		CPPUNIT_ASSERT(institution.id() == "I000001");
		CPPUNIT_ASSERT(m->institutionCount() == 2);
		CPPUNIT_ASSERT(m->dirty() == false);

		institution = m->institution("I000002");
		CPPUNIT_ASSERT(institution.id() == "I000002");
		CPPUNIT_ASSERT(m->institutionCount() == 2);
		CPPUNIT_ASSERT(m->dirty() == false);
	} catch (MyMoneyException *e) {
		CPPUNIT_FAIL("Unexpected exception");
		delete e;
	}
}

void MyMoneyFileTest::testRemoveInstitution() {
	testAddTwoInstitutions();

	MyMoneyInstitution i;

	CPPUNIT_ASSERT(m->institutionCount() == 2);

	i = m->institution("I000001");
	CPPUNIT_ASSERT(i.id() == "I000001");
	CPPUNIT_ASSERT(i.accountCount() == 0);

	storage->m_dirty = false;

	try {
		m->removeInstitution(i);
		CPPUNIT_ASSERT(m->institutionCount() == 1);
		CPPUNIT_ASSERT(m->dirty() == true);
	} catch (MyMoneyException *e) {
		CPPUNIT_FAIL("Unexpected exception");
		delete e;
	}

	storage->m_dirty = false;

	try {
		m->institution("I000001");
		CPPUNIT_FAIL("Missing expected exception");
	} catch(MyMoneyException *e) {
		CPPUNIT_ASSERT(m->institutionCount() == 1);
		CPPUNIT_ASSERT(m->dirty() == false);
		delete e;
	}

	try {
		m->removeInstitution(i);
		CPPUNIT_FAIL("Missing expected exception");
	} catch(MyMoneyException *e) {
		CPPUNIT_ASSERT(m->institutionCount() == 1);
		CPPUNIT_ASSERT(m->dirty() == false);
		delete e;
	}
}

void MyMoneyFileTest::testInstitutionRetrieval () {

	testAddOneInstitution();

	storage->m_dirty = false;
	
	MyMoneyInstitution institution;

	CPPUNIT_ASSERT(m->institutionCount() == 1);

	try {
		institution = m->institution("I000001");
		CPPUNIT_ASSERT(institution.id() == "I000001");
		CPPUNIT_ASSERT(m->institutionCount() == 1);
	} catch (MyMoneyException *e) {
		CPPUNIT_FAIL("Unexpected exception");
		delete e;
	}

	try {
		institution = m->institution("I000002");
		CPPUNIT_FAIL("Missing expected exception");
	} catch(MyMoneyException *e) {
		CPPUNIT_ASSERT(m->institutionCount() == 1);
		delete e;
	}

	CPPUNIT_ASSERT(m->dirty() == false);
}

void MyMoneyFileTest::testInstitutionListRetrieval () {
	QValueList<MyMoneyInstitution> list;

	storage->m_dirty = false;
	list = m->institutionList();
	CPPUNIT_ASSERT(m->dirty() == false);
	CPPUNIT_ASSERT(list.count() == 0);

	testAddTwoInstitutions();

	storage->m_dirty = false;
	list = m->institutionList();
	CPPUNIT_ASSERT(m->dirty() == false);
	CPPUNIT_ASSERT(list.count() == 2);

	QValueList<MyMoneyInstitution>::ConstIterator it;
	it = list.begin();
	
	CPPUNIT_ASSERT((*it).name() == "institution1");
	++it;
	CPPUNIT_ASSERT((*it).name() == "institution2");
	++it;
	CPPUNIT_ASSERT(it == list.end());
}

void MyMoneyFileTest::testInstitutionModify() {
	testAddTwoInstitutions();
	MyMoneyInstitution institution;

	institution = m->institution("I000001");
        institution.setStreet("new street");
        institution.setTown("new town");
        institution.setPostcode("new postcode");
        institution.setTelephone("new telephone");
        institution.setManager("new manager");
        institution.setName("new name");
        institution.setSortcode("new sortcode");

	storage->m_dirty = false;

	try {
		m->modifyInstitution(institution);
		CPPUNIT_ASSERT(institution.id() == "I000001");
		CPPUNIT_ASSERT(m->institutionCount() == 2);
		CPPUNIT_ASSERT(m->dirty() == true);
	} catch(MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception");
	}

	MyMoneyInstitution newInstitution;
	newInstitution = m->institution("I000001");

        CPPUNIT_ASSERT(newInstitution.id() == "I000001");
        CPPUNIT_ASSERT(newInstitution.street() == "new street");
        CPPUNIT_ASSERT(newInstitution.town() == "new town");
        CPPUNIT_ASSERT(newInstitution.postcode() == "new postcode");
        CPPUNIT_ASSERT(newInstitution.telephone() == "new telephone");
        CPPUNIT_ASSERT(newInstitution.manager() == "new manager");
        CPPUNIT_ASSERT(newInstitution.name() == "new name");
        CPPUNIT_ASSERT(newInstitution.sortcode() == "new sortcode");

	storage->m_dirty = false;

	MyMoneyInstitution failInstitution2("I000003", newInstitution);
	try {
		m->modifyInstitution(failInstitution2);
		CPPUNIT_FAIL("Exception expected");
	} catch(MyMoneyException *e) {
		delete e;
		CPPUNIT_ASSERT(failInstitution2.id() == "I000003");
		CPPUNIT_ASSERT(m->institutionCount() == 2);
		CPPUNIT_ASSERT(m->dirty() == false);
	}
}

void MyMoneyFileTest::testSetFunctions() {
	MyMoneyPayee user = m->user();

	CPPUNIT_ASSERT(user.name().isEmpty());
	CPPUNIT_ASSERT(user.address().isEmpty());
	CPPUNIT_ASSERT(user.city().isEmpty());
	CPPUNIT_ASSERT(user.state().isEmpty());
	CPPUNIT_ASSERT(user.postcode().isEmpty());
	CPPUNIT_ASSERT(user.telephone().isEmpty());
	CPPUNIT_ASSERT(user.email().isEmpty());
	
	storage->m_dirty = false;
	user.setName("Name");
	m->setUser(user);
	CPPUNIT_ASSERT(m->dirty() == true);
	storage->m_dirty = false;
	user.setAddress("Street");
	m->setUser(user);
	CPPUNIT_ASSERT(m->dirty() == true);
	storage->m_dirty = false;
	user.setCity("Town");
	m->setUser(user);
	CPPUNIT_ASSERT(m->dirty() == true);
	storage->m_dirty = false;
	user.setState("County");
	m->setUser(user);
	CPPUNIT_ASSERT(m->dirty() == true);
	storage->m_dirty = false;
	user.setPostcode("Postcode");
	m->setUser(user);
	CPPUNIT_ASSERT(m->dirty() == true);
	storage->m_dirty = false;
	user.setTelephone("Telephone");
	m->setUser(user);
	CPPUNIT_ASSERT(m->dirty() == true);
	storage->m_dirty = false;
	user.setEmail("Email");
	m->setUser(user);
	CPPUNIT_ASSERT(m->dirty() == true);
	storage->m_dirty = false;

	user = m->user();
	CPPUNIT_ASSERT(user.name() == "Name");
	CPPUNIT_ASSERT(user.address() == "Street");
	CPPUNIT_ASSERT(user.city() == "Town");
	CPPUNIT_ASSERT(user.state() == "County");
	CPPUNIT_ASSERT(user.postcode() == "Postcode");
	CPPUNIT_ASSERT(user.telephone() == "Telephone");
	CPPUNIT_ASSERT(user.email() == "Email");
}

void MyMoneyFileTest::testAddAccounts() {
	testAddTwoInstitutions();
	MyMoneyAccount  a, b, c;
	a.setAccountType(MyMoneyAccount::Checkings);
	b.setAccountType(MyMoneyAccount::Checkings);

	MyMoneyInstitution institution;

	storage->m_dirty = false;

	CPPUNIT_ASSERT(m->accountCount() == 5);

	institution = m->institution("I000001");
	CPPUNIT_ASSERT(institution.id() == "I000001");

	m->attach("I000001", observer);
	m->attach("I000002", observer);
	m->attach(MyMoneyFile::NotifyClassAccount, observer);
	m->attach(MyMoneyFile::NotifyClassAccountHierarchy, hierarchyObserver);

	a.setName("Account1");
	a.setInstitutionId(institution.id());

	try {
		MyMoneyAccount parent = m->asset();
		observer->reset();
		hierarchyObserver->reset();
		m->addAccount(a, parent);
		CPPUNIT_ASSERT(m->accountCount() == 6);
		CPPUNIT_ASSERT(a.parentAccountId() == "AStd::Asset");
		CPPUNIT_ASSERT(a.id() == "A000001");
		CPPUNIT_ASSERT(a.institutionId() == "I000001");
		CPPUNIT_ASSERT(m->dirty() == true);
		CPPUNIT_ASSERT(m->asset().accountList().count() == 1);
		CPPUNIT_ASSERT(m->asset().accountList()[0] == "A000001");

		institution = m->institution("I000001");
		CPPUNIT_ASSERT(institution.accountCount() == 1);
		CPPUNIT_ASSERT(institution.accountList()[0] == "A000001");

		CPPUNIT_ASSERT(observer->updated().count() == 2);
		CPPUNIT_ASSERT(observer->updated().contains("I000001") == 1);
		CPPUNIT_ASSERT(observer->updated().contains(MyMoneyFile::NotifyClassAccount) == 1);

		CPPUNIT_ASSERT(hierarchyObserver->updated().count() == 1);
		CPPUNIT_ASSERT(hierarchyObserver->updated().contains(MyMoneyFile::NotifyClassAccountHierarchy) == 1);
		observer->reset();
		hierarchyObserver->reset();
	} catch(MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception!");
	}

	// try to add this account again, should not work
	try {
		MyMoneyAccount parent = m->asset();
		m->addAccount(a, parent);
		CPPUNIT_FAIL("Expecting exception!");
	} catch(MyMoneyException *e) {
		delete e;
	}

	CPPUNIT_ASSERT(observer->updated().count() == 0);

	// check that we can modify the local object and
	// reload it from the file
	a.setName("AccountX");
	a = m->account("A000001");
	CPPUNIT_ASSERT(a.name() == "Account1");

	storage->m_dirty = false;

	// check if we can get the same info to a different object
	c = m->account("A000001");
	CPPUNIT_ASSERT(c.accountType() == MyMoneyAccount::Checkings);
	CPPUNIT_ASSERT(c.id() == "A000001");
	CPPUNIT_ASSERT(c.name() == "Account1");
	CPPUNIT_ASSERT(c.institutionId() == "I000001");

	CPPUNIT_ASSERT(m->dirty() == false);
	CPPUNIT_ASSERT(observer->updated().count() == 0);

	// add a second account
	institution = m->institution("I000002");
	b.setName("Account2");
	b.setInstitutionId(institution.id());
	try {
		MyMoneyAccount parent = m->asset();
		m->addAccount(b, parent);
		CPPUNIT_ASSERT(m->dirty() == true);
		CPPUNIT_ASSERT(b.id() == "A000002");
		CPPUNIT_ASSERT(b.parentAccountId() == "AStd::Asset");
		CPPUNIT_ASSERT(m->accountCount() == 7);

		institution = m->institution("I000001");
		CPPUNIT_ASSERT(institution.accountCount() == 1);
		CPPUNIT_ASSERT(institution.accountList()[0] == "A000001");

		institution = m->institution("I000002");
		CPPUNIT_ASSERT(institution.accountCount() == 1);
		CPPUNIT_ASSERT(institution.accountList()[0] == "A000002");

		CPPUNIT_ASSERT(m->asset().accountList().count() == 2);
		CPPUNIT_ASSERT(m->asset().accountList()[0] == "A000001");
		CPPUNIT_ASSERT(m->asset().accountList()[1] == "A000002");

		CPPUNIT_ASSERT(observer->updated().count() == 2);
		CPPUNIT_ASSERT(observer->updated().contains("I000002") == 1);
		CPPUNIT_ASSERT(observer->updated().contains(MyMoneyFile::NotifyClassAccount) == 1);
		observer->reset();
	} catch(MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception!");
	}

	MyMoneyAccount p;

	p = m->account("A000002");
	CPPUNIT_ASSERT(p.accountType() == MyMoneyAccount::Checkings);
	CPPUNIT_ASSERT(p.id() == "A000002");
	CPPUNIT_ASSERT(p.name() == "Account2");
	CPPUNIT_ASSERT(p.institutionId() == "I000002");
}

void MyMoneyFileTest::testModifyAccount() {
	testAddAccounts();
	storage->m_dirty = false;

	observer->reset();
	CPPUNIT_ASSERT(observer->updated().count() == 0);

	MyMoneyAccount p = m->account("A000001");
	MyMoneyInstitution institution;

	CPPUNIT_ASSERT(p.accountType() == MyMoneyAccount::Checkings);
	CPPUNIT_ASSERT(p.name() == "Account1");

	p.setName("New account name");
	try {
		m->modifyAccount(p);

		CPPUNIT_ASSERT(m->dirty() == true);
		CPPUNIT_ASSERT(m->accountCount() == 7);
		CPPUNIT_ASSERT(p.accountType() == MyMoneyAccount::Checkings);
		CPPUNIT_ASSERT(p.name() == "New account name");

		CPPUNIT_ASSERT(observer->updated().count() == 2);
		CPPUNIT_ASSERT(observer->updated().contains(MyMoneyFile::NotifyClassAccount) == 1);
		observer->reset();
	} catch(MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception!");
	}
	storage->m_dirty = false;

	// try to move account to new institution
	p.setInstitutionId("I000002");
	try {
		m->modifyAccount(p);

		CPPUNIT_ASSERT(m->dirty() == true);
		CPPUNIT_ASSERT(m->accountCount() == 7);
		CPPUNIT_ASSERT(p.accountType() == MyMoneyAccount::Checkings);
		CPPUNIT_ASSERT(p.name() == "New account name");
		CPPUNIT_ASSERT(p.institutionId() == "I000002");

		institution = m->institution("I000001");
		CPPUNIT_ASSERT(institution.accountCount() == 0);

		institution = m->institution("I000002");
		CPPUNIT_ASSERT(institution.accountCount() == 2);
		CPPUNIT_ASSERT(institution.accountList()[0] == "A000002");
		CPPUNIT_ASSERT(institution.accountList()[1] == "A000001");

		CPPUNIT_ASSERT(observer->updated().count() == 3);
		CPPUNIT_ASSERT(observer->updated().contains(MyMoneyFile::NotifyClassAccount) == 1);
		observer->reset();
	} catch(MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception!");
	}
	storage->m_dirty = false;

	// try to fool engine a bit
	p.setParentAccountId("A000001");
	try {
		m->modifyAccount(p);
		CPPUNIT_FAIL("Expecting exception!");
	} catch(MyMoneyException *e) {
		delete e;
	}
}

void MyMoneyFileTest::testReparentAccount() {
	testAddAccounts();
	storage->m_dirty = false;

	observer->reset();
	CPPUNIT_ASSERT(observer->updated().count() == 0);

	hierarchyObserver->reset();
	CPPUNIT_ASSERT(hierarchyObserver->updated().count() == 0);

	MyMoneyAccount p = m->account("A000001");
	MyMoneyAccount q = m->account("A000002");
	MyMoneyAccount o = m->account(p.parentAccountId());

	// make A000001 a child of A000002
	try {
		CPPUNIT_ASSERT(p.parentAccountId() != q.id());
		CPPUNIT_ASSERT(o.accountCount() == 2);
		CPPUNIT_ASSERT(q.accountCount() == 0);
		m->reparentAccount(p, q);
		CPPUNIT_ASSERT(m->dirty() == true);
		CPPUNIT_ASSERT(p.parentAccountId() == q.id());
		CPPUNIT_ASSERT(q.accountCount() == 1);
		CPPUNIT_ASSERT(q.id() == "A000002");
		CPPUNIT_ASSERT(p.id() == "A000001");
		CPPUNIT_ASSERT(q.accountList()[0] == p.id());
		
		CPPUNIT_ASSERT(observer->updated().count() == 3);
		CPPUNIT_ASSERT(observer->updated().contains(MyMoneyFile::NotifyClassAccount) == 1);

		CPPUNIT_ASSERT(hierarchyObserver->updated().count() == 1);
		CPPUNIT_ASSERT(hierarchyObserver->updated().contains(MyMoneyFile::NotifyClassAccountHierarchy) == 1);
		observer->reset();
		hierarchyObserver->reset();

		o = m->account(o.id());
		CPPUNIT_ASSERT(o.accountCount() == 1);
	} catch(MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception!");
	}
}

void MyMoneyFileTest::testRemoveStdAccount(const MyMoneyAccount& acc) {
	QString txt("Exception expected while removing account ");
	txt += acc.id();
	try {
		m->removeAccount(acc);
		CPPUNIT_FAIL(txt.latin1());
	} catch(MyMoneyException *e) {
		delete e;
	}
}

void MyMoneyFileTest::testRemoveAccount() {
	MyMoneyInstitution institution;

	testAddAccounts();
	storage->m_dirty = false;

	observer->reset();
	CPPUNIT_ASSERT(observer->updated().count() == 0);

	hierarchyObserver->reset();
	CPPUNIT_ASSERT(hierarchyObserver->updated().count() == 0);

	QString id;
	MyMoneyAccount p = m->account("A000001");

	try {
		MyMoneyAccount q("Ainvalid", p);
		m->removeAccount(q);
		CPPUNIT_FAIL("Exception expected!");
	} catch(MyMoneyException *e) {
		delete e;
	}

	try {
		m->removeAccount(p);
		CPPUNIT_ASSERT(m->dirty() == true);
		CPPUNIT_ASSERT(m->accountCount() == 6);
		institution = m->institution("I000001");
		CPPUNIT_ASSERT(institution.accountCount() == 0);
		CPPUNIT_ASSERT(m->asset().accountList().count() == 1);

		institution = m->institution("I000002");
		CPPUNIT_ASSERT(institution.accountCount() == 1);

		CPPUNIT_ASSERT(observer->updated().count() == 1);
		CPPUNIT_ASSERT(observer->updated().contains(MyMoneyFile::NotifyClassAccount) == 1);

		CPPUNIT_ASSERT(hierarchyObserver->updated().count() == 1);
		CPPUNIT_ASSERT(hierarchyObserver->updated().contains(MyMoneyFile::NotifyClassAccountHierarchy) == 1);
		observer->reset();
		hierarchyObserver->reset();
	} catch(MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception!");
	}

	// Check that the standard account-groups cannot be removed
	testRemoveStdAccount(m->liability());
	testRemoveStdAccount(m->asset());
	testRemoveStdAccount(m->expense());
	testRemoveStdAccount(m->income());
}

void MyMoneyFileTest::testRemoveAccountTree() {
	testReparentAccount();
	MyMoneyAccount a = m->account("A000002");

	// remove the account
	try {
		m->removeAccount(a);
	} catch(MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception!");
	}
	CPPUNIT_ASSERT(m->accountCount() == 6);

	// make sure it's gone
	try {
		m->account("A000002");
		CPPUNIT_FAIL("Exception expected!");
	} catch(MyMoneyException *e) {
		delete e;
	}

	// make sure that children are re-parented to parent account
	try {
		a = m->account("A000001");
		CPPUNIT_ASSERT(a.parentAccountId() == m->asset().id());
	} catch(MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception!");
	}

}

void MyMoneyFileTest::testAccountListRetrieval () {
	QValueList<MyMoneyAccount> list;

	storage->m_dirty = false;
	list = m->accountList();
	CPPUNIT_ASSERT(m->dirty() == false);
	CPPUNIT_ASSERT(list.count() == 0);

	testAddAccounts();

	storage->m_dirty = false;
	list = m->accountList();
	CPPUNIT_ASSERT(m->dirty() == false);
	CPPUNIT_ASSERT(list.count() == 2);

	CPPUNIT_ASSERT(list[0].accountType() == MyMoneyAccount::Checkings);
	CPPUNIT_ASSERT(list[1].accountType() == MyMoneyAccount::Checkings);
}

void MyMoneyFileTest::testAddTransaction () {
	testAddAccounts();
	MyMoneyTransaction t, p;

	MyMoneyAccount exp1;
	exp1.setAccountType(MyMoneyAccount::Expense);
	exp1.setName("Expense1");
	MyMoneyAccount exp2;
	exp2.setAccountType(MyMoneyAccount::Expense);
	exp2.setName("Expense2");

	try {
		MyMoneyAccount parent = m->expense();
		m->addAccount(exp1, parent);
		m->addAccount(exp2, parent);
	} catch(MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception!");
	} 

	// fake the last modified flag to check that the
	// date is updated when we add the transaction
	MyMoneyAccount a = m->account("A000001");
	a.setLastModified(QDate(1,2,3));
	try {
		m->modifyAccount(a);
	} catch(MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception!");
	}

	CPPUNIT_ASSERT(m->accountCount() == 9);
	a = m->account("A000001");
	CPPUNIT_ASSERT(a.lastModified() == QDate(1,2,3));

	// construct a transaction and add it to the pool
	t.setPostDate(QDate(2002,2,1));
	t.setMemo("Memotext");

	MyMoneySplit split1;
	MyMoneySplit split2;

	split1.setAccountId("A000001");
	split1.setShares(-1000);
	split1.setValue(-1000);
	split2.setAccountId("A000003");
	split2.setValue(1000);
	split2.setShares(1000);
	try {
		t.addSplit(split1);
		t.addSplit(split2);
	} catch(MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception!");
	}

/*
	// FIXME: we don't have a payee and a number field right now
	// guess we should have a number field per split, don't know
	// about the payee
	t.setMethod(MyMoneyCheckingTransaction::Withdrawal);
	t.setPayee("Thomas Baumgart");
	t.setNumber("1234");
	t.setState(MyMoneyCheckingTransaction::Cleared);
*/
	storage->m_dirty = false;

	observer->reset();
	CPPUNIT_ASSERT(observer->updated().count() == 0);

	try {
		m->addTransaction(t);

		CPPUNIT_ASSERT(observer->updated().count() == 2);
		CPPUNIT_ASSERT(observer->updated().contains(MyMoneyFile::NotifyClassAccount) == 1);
		observer->reset();
	} catch(MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception!");
	}

	CPPUNIT_ASSERT(t.id() == "T000000000000000001");
	CPPUNIT_ASSERT(t.postDate() == QDate(2002,2,1));
	CPPUNIT_ASSERT(t.entryDate() == QDate::currentDate());
	CPPUNIT_ASSERT(m->dirty() == true);

	// check the balance of the accounts
	a = m->account("A000001");
	CPPUNIT_ASSERT(a.lastModified() == QDate::currentDate());
	CPPUNIT_ASSERT(a.balance() == MyMoneyMoney(-1000));

	MyMoneyAccount b = m->account("A000003");
	CPPUNIT_ASSERT(b.lastModified() == QDate::currentDate());
	CPPUNIT_ASSERT(b.balance() == MyMoneyMoney(1000));

	storage->m_dirty = false;

	// locate transaction in MyMoneyFile via id

	try {
		p = m->transaction("T000000000000000001");
		CPPUNIT_ASSERT(p.splitCount() == 2);
		CPPUNIT_ASSERT(p.memo() == "Memotext");
		CPPUNIT_ASSERT(p.splits()[0].accountId() == "A000001");
		CPPUNIT_ASSERT(p.splits()[1].accountId() == "A000003");
		CPPUNIT_ASSERT(observer->updated().count() == 0);
	} catch(MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception!");
	}

	// check if it's in the account(s) as well

	try {
		p = m->transaction("A000001", 0);
		CPPUNIT_ASSERT(p.id() == "T000000000000000001");
		CPPUNIT_ASSERT(p.splitCount() == 2);
		CPPUNIT_ASSERT(p.memo() == "Memotext");
		CPPUNIT_ASSERT(p.splits()[0].accountId() == "A000001");
		CPPUNIT_ASSERT(p.splits()[1].accountId() == "A000003");
		CPPUNIT_ASSERT(observer->updated().count() == 0);
	} catch(MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception!");
	}

	try {
		p = m->transaction("A000003", 0);
		CPPUNIT_ASSERT(p.id() == "T000000000000000001");
		CPPUNIT_ASSERT(p.splitCount() == 2);
		CPPUNIT_ASSERT(p.memo() == "Memotext");
		CPPUNIT_ASSERT(p.splits()[0].accountId() == "A000001");
		CPPUNIT_ASSERT(p.splits()[1].accountId() == "A000003");
		CPPUNIT_ASSERT(observer->updated().count() == 0);
	} catch(MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception!");
	}
}

void MyMoneyFileTest::testIsStandardAccount() {
	CPPUNIT_ASSERT(m->isStandardAccount(m->liability().id()) == true);
	CPPUNIT_ASSERT(m->isStandardAccount(m->asset().id()) == true);
	CPPUNIT_ASSERT(m->isStandardAccount(m->expense().id()) == true);
	CPPUNIT_ASSERT(m->isStandardAccount(m->income().id()) == true);
	CPPUNIT_ASSERT(m->isStandardAccount("A00001") == false);
}

void MyMoneyFileTest::testHasActiveSplits() {
	testAddTransaction();

	CPPUNIT_ASSERT(m->hasActiveSplits("A000001") == true);
	CPPUNIT_ASSERT(m->hasActiveSplits("A000002") == false);
}

void MyMoneyFileTest::testModifyTransactionSimple() {
	// this will test that we can modify the basic attributes
	// of a transaction
	testAddTransaction();

	MyMoneyTransaction t = m->transaction("T000000000000000001");
	t.setMemo("New Memotext");
	storage->m_dirty = false;

	observer->reset();
	CPPUNIT_ASSERT(observer->updated().count() == 0);

	try {
		m->modifyTransaction(t);
		t = m->transaction("T000000000000000001");
		CPPUNIT_ASSERT(t.memo() == "New Memotext");
		CPPUNIT_ASSERT(m->dirty() == true);

		CPPUNIT_ASSERT(observer->updated().count() == 2);
		CPPUNIT_ASSERT(observer->updated().contains(MyMoneyFile::NotifyClassAccount) == 1);
		observer->reset();
	} catch(MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception!");
	}
}

void MyMoneyFileTest::testModifyTransactionNewPostDate() {
	// this will test that we can modify the basic attributes
	// of a transaction
	testAddTransaction();

	MyMoneyTransaction t = m->transaction("T000000000000000001");
	t.setPostDate(QDate(2004,2,1));
	storage->m_dirty = false;
	observer->reset();
	CPPUNIT_ASSERT(observer->updated().count() == 0);

	try {
		m->modifyTransaction(t);
		t = m->transaction("T000000000000000001");
		CPPUNIT_ASSERT(t.postDate() == QDate(2004,2,1));
		t = m->transaction("A000001", 0);
		CPPUNIT_ASSERT(t.id() == "T000000000000000001");
		CPPUNIT_ASSERT(m->dirty() == true);

		CPPUNIT_ASSERT(observer->updated().count() == 2);
		CPPUNIT_ASSERT(observer->updated().contains(MyMoneyFile::NotifyClassAccount) == 1);
		observer->reset();
	} catch(MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception!");
	}
}

void MyMoneyFileTest::testModifyTransactionNewAccount() {
	// this will test that we can modify the basic attributes
	// of a transaction
	testAddTransaction();

	MyMoneyTransaction t = m->transaction("T000000000000000001");
	MyMoneySplit s;
	s = t.splits()[0];
	s.setAccountId("A000002");
	t.modifySplit(s);

	storage->m_dirty = false;
	observer->reset();
	CPPUNIT_ASSERT(observer->updated().count() == 0);
	try {
/* removed with MyMoneyAccount::Transaction
		CPPUNIT_ASSERT(m->account("A000001").transactionCount() == 1);
		CPPUNIT_ASSERT(m->account("A000002").transactionCount() == 0);
		CPPUNIT_ASSERT(m->account("A000003").transactionCount() == 1);
*/
		MyMoneyTransactionFilter f1("A000001");
		MyMoneyTransactionFilter f2("A000002");
		MyMoneyTransactionFilter f3("A000003");
		CPPUNIT_ASSERT(m->transactionList(f1).count() == 1);
		CPPUNIT_ASSERT(m->transactionList(f2).count() == 0);
		CPPUNIT_ASSERT(m->transactionList(f3).count() == 1);

		m->modifyTransaction(t);
		t = m->transaction("T000000000000000001");
		CPPUNIT_ASSERT(t.postDate() == QDate(2002,2,1));
		t = m->transaction("A000002", 0);
		CPPUNIT_ASSERT(m->dirty() == true);
/* removed with MyMoneyAccount::Transaction
		CPPUNIT_ASSERT(m->account("A000001").transactionCount() == 0);
		CPPUNIT_ASSERT(m->account("A000002").transactionCount() == 1);
		CPPUNIT_ASSERT(m->account("A000003").transactionCount() == 1);
*/
		CPPUNIT_ASSERT(m->transactionList(f1).count() == 0);
		CPPUNIT_ASSERT(m->transactionList(f2).count() == 1);
		CPPUNIT_ASSERT(m->transactionList(f3).count() == 1);

		CPPUNIT_ASSERT(observer->updated().count() == 3);
		CPPUNIT_ASSERT(observer->updated().contains(MyMoneyFile::NotifyClassAccount) == 1);
		observer->reset();
	} catch(MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception!");
	}
}

void MyMoneyFileTest::testRemoveTransaction () {
	testModifyTransactionNewPostDate();

	MyMoneyTransaction t;
	t = m->transaction("T000000000000000001");

	storage->m_dirty = false;
	observer->reset();
	CPPUNIT_ASSERT(observer->updated().count() == 0);
	try {
		m->removeTransaction(t);
		CPPUNIT_ASSERT(m->dirty() == true);
		CPPUNIT_ASSERT(m->transactionCount() == 0);
/* removed with MyMoneyAccount::Transaction
		CPPUNIT_ASSERT(m->account("A000001").transactionCount() == 0);
		CPPUNIT_ASSERT(m->account("A000002").transactionCount() == 0);
		CPPUNIT_ASSERT(m->account("A000003").transactionCount() == 0);
*/
		MyMoneyTransactionFilter f1("A000001");
		MyMoneyTransactionFilter f2("A000002");
		MyMoneyTransactionFilter f3("A000003");
		CPPUNIT_ASSERT(m->transactionList(f1).count() == 0);
		CPPUNIT_ASSERT(m->transactionList(f2).count() == 0);
		CPPUNIT_ASSERT(m->transactionList(f3).count() == 0);

		CPPUNIT_ASSERT(observer->updated().count() == 2);
		CPPUNIT_ASSERT(observer->updated().contains(MyMoneyFile::NotifyClassAccount) == 1);
		observer->reset();
	} catch(MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception!");
	}
}

/*
 * This function is currently not implemented. It's kind of tricky
 * because it modifies a lot of objects in a single call. This might
 * be a problem for the undo/redo stuff. That's why I left it out in
 * the first run. We migh add it, if we need it.
 * /
void testMoveSplits() {
	testModifyTransactionNewPostDate();

	CPPUNIT_ASSERT(m->account("A000001").transactionCount() == 1);
	CPPUNIT_ASSERT(m->account("A000002").transactionCount() == 0);
	CPPUNIT_ASSERT(m->account("A000003").transactionCount() == 1);

	try {
		m->moveSplits("A000001", "A000002");
		CPPUNIT_ASSERT(m->account("A000001").transactionCount() == 0);
		CPPUNIT_ASSERT(m->account("A000002").transactionCount() == 1);
		CPPUNIT_ASSERT(m->account("A000003").transactionCount() == 1);
	} catch(MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception!");
	}
}
*/

void MyMoneyFileTest::testBalanceTotal() {
	testAddTransaction();
	MyMoneyTransaction t;

	// construct a transaction and add it to the pool
	t.setPostDate(QDate(2002,2,1));
	t.setMemo("Memotext");

	MyMoneySplit split1;
	MyMoneySplit split2;

	try {
		split1.setAccountId("A000002");
		split1.setShares(-1000);
		split1.setValue(-1000);
		split2.setAccountId("A000004");
		split2.setValue(1000);
		split2.setShares(1000);
		t.addSplit(split1);
		t.addSplit(split2);
		m->addTransaction(t);
		CPPUNIT_ASSERT(t.id() == "T000000000000000002");
		CPPUNIT_ASSERT(m->totalBalance("A000001") == MyMoneyMoney(-1000));
		CPPUNIT_ASSERT(m->totalBalance("A000002") == MyMoneyMoney(-1000));

		MyMoneyAccount p = m->account("A000001");
		MyMoneyAccount q = m->account("A000002");
		m->reparentAccount(p, q);
		CPPUNIT_ASSERT(m->totalBalance("A000001") == MyMoneyMoney(-1000));
		CPPUNIT_ASSERT(m->totalBalance("A000002") == MyMoneyMoney(-2000));
	} catch(MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception!");
	}
}

void MyMoneyFileTest::testSetAccountName() {
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

void MyMoneyFileTest::testAddPayee() {
	MyMoneyPayee p;

	m->attach(MyMoneyFile::NotifyClassPayee, observer);
	observer->reset();
	CPPUNIT_ASSERT(observer->updated().count() == 0);

	p.setName("THB");
	CPPUNIT_ASSERT(m->dirty() == false);
	try {
		m->addPayee(p);
		CPPUNIT_ASSERT(m->dirty() == true);
		CPPUNIT_ASSERT(p.id() == "P000001");

	} catch (MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception");
	}
}

void MyMoneyFileTest::testModifyPayee() {
	MyMoneyPayee p;

	testAddPayee();
	CPPUNIT_ASSERT(observer->updated().count() == 0);

	p = m->payee("P000001");
	p.setName("New name");
	try {
		m->modifyPayee(p);
		p = m->payee("P000001");
		CPPUNIT_ASSERT(p.name() == "New name");

	} catch (MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception");
	}
}

void MyMoneyFileTest::testRemovePayee() {
	MyMoneyPayee p;

	testAddPayee();
	CPPUNIT_ASSERT(m->payeeList().count() == 1);
	CPPUNIT_ASSERT(observer->updated().count() == 0);

	p = m->payee("P000001");
	try {
		m->removePayee(p);
		CPPUNIT_ASSERT(m->payeeList().count() == 0);

	} catch (MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception");
	}
}

void MyMoneyFileTest::testAddTransactionStd() {
	testAddAccounts();
	MyMoneyTransaction t, p;
	MyMoneyAccount a;

	a = m->account("A000001");

	// construct a transaction and add it to the pool
	t.setPostDate(QDate(2002,2,1));
	t.setMemo("Memotext");

	MyMoneySplit split1;
	MyMoneySplit split2;

	split1.setAccountId("A000001");
	split1.setShares(-1000);
	split1.setValue(-1000);
	split2.setAccountId(STD_ACC_EXPENSE);
	split2.setValue(1000);
	split2.setShares(1000);
	try {
		t.addSplit(split1);
		t.addSplit(split2);
	} catch(MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception!");
	}

/*
	// FIXME: we don't have a payee and a number field right now
	// guess we should have a number field per split, don't know
	// about the payee
	t.setMethod(MyMoneyCheckingTransaction::Withdrawal);
	t.setPayee("Thomas Baumgart");
	t.setNumber("1234");
	t.setState(MyMoneyCheckingTransaction::Cleared);
*/
	storage->m_dirty = false;
	observer->reset();
	CPPUNIT_ASSERT(observer->updated().count() == 0);

	try {
		m->addTransaction(t);
		CPPUNIT_FAIL("Missing expected exception!");
	} catch(MyMoneyException *e) {
		delete e;
	}

	CPPUNIT_ASSERT(m->dirty() == false);
}

void MyMoneyFileTest::testAttachStorage() {
	IMyMoneyStorage *store = new MyMoneySeqAccessMgr;
	MyMoneyFile *file = new MyMoneyFile;

	CPPUNIT_ASSERT(file->storageAttached() == false);
	try {
		file->attachStorage(store);
		CPPUNIT_ASSERT(file->storageAttached() == true);
	} catch(MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception!");
	}

	try {
		file->attachStorage(store);
		CPPUNIT_FAIL("Exception expected!");
	} catch(MyMoneyException *e) {
		delete e;
	}

	try {
		file->attachStorage(0);
		CPPUNIT_FAIL("Exception expected!");
	} catch(MyMoneyException *e) {
		delete e;
	}

	try {
		file->detachStorage(store);
		CPPUNIT_ASSERT(file->storageAttached() == false);
	} catch(MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception!");
	}

	delete store;
	delete file;
}

void MyMoneyFileTest::testAccount2Category() {
	testReparentAccount();
	CPPUNIT_ASSERT(m->accountToCategory("A000001") == "Account2:Account1");
	CPPUNIT_ASSERT(m->accountToCategory("A000002") == "Account2");
}

void MyMoneyFileTest::testCategory2Account() {
	testAddTransaction();
	MyMoneyAccount a = m->account("A000003");
	MyMoneyAccount b = m->account("A000004");

	try {
		m->reparentAccount(b, a);
		CPPUNIT_ASSERT(m->categoryToAccount("Expense1") == "A000003");
		CPPUNIT_ASSERT(m->categoryToAccount("Expense1:Expense2") == "A000004");
		CPPUNIT_ASSERT(m->categoryToAccount("Acc2").isEmpty());
	} catch(MyMoneyException *e) {
		unexpectedException(e);
	}
}

void MyMoneyFileTest::testAttachedStorage() {
	CPPUNIT_ASSERT(m->storageAttached() == true);
	CPPUNIT_ASSERT(m->storage() != 0);
	IMyMoneyStorage *p = m->storage();
	m->detachStorage(p);
	CPPUNIT_ASSERT(m->storageAttached() == false);
	CPPUNIT_ASSERT(m->storage() == 0);
	m->attachStorage(p);
	CPPUNIT_ASSERT(m->storageAttached() == true);
	CPPUNIT_ASSERT(m->storage() != 0);
}

void MyMoneyFileTest::testHasAccount() {
	testAddAccounts();

	MyMoneyAccount a, b;
	a.setAccountType(MyMoneyAccount::Checkings);
	a.setName("Account3");
	b = m->account("A000001");
	try {
		m->addAccount(a, b);
		CPPUNIT_ASSERT(m->accountCount() == 8);
		CPPUNIT_ASSERT(a.parentAccountId() == "A000001");
		CPPUNIT_ASSERT(m->hasAccount("A000001", "Account3") == true);
		CPPUNIT_ASSERT(m->hasAccount("A000001", "Account2") == false);
		CPPUNIT_ASSERT(m->hasAccount("A000002", "Account3") == false);
	} catch(MyMoneyException *e) {
		unexpectedException(e);
	}
}

void MyMoneyFileTest::testAddEquityAccount() {
	MyMoneyAccount i;
	i.setName("Investment");
	i.setAccountType(MyMoneyAccount::Investment);

	try {
		MyMoneyAccount parent = m->asset();
		observer->reset();
		hierarchyObserver->reset();
		m->addAccount(i, parent);
	} catch(MyMoneyException *e) {
		unexpectedException(e);
	}

	// make sure, that only equity accounts can be children to it
	MyMoneyAccount a;
	a.setName("Testaccount");
	QValueList<MyMoneyAccount::accountTypeE> list;
	list << MyMoneyAccount::Checkings;
	list << MyMoneyAccount::Savings;
	list << MyMoneyAccount::Cash;
	list << MyMoneyAccount::CreditCard;
	list << MyMoneyAccount::Loan;
	list << MyMoneyAccount::CertificateDep;
	list << MyMoneyAccount::Investment;
	list << MyMoneyAccount::MoneyMarket;
	list << MyMoneyAccount::Asset;
	list << MyMoneyAccount::Liability;
	list << MyMoneyAccount::Currency;
	list << MyMoneyAccount::Income;
	list << MyMoneyAccount::Expense;
	list << MyMoneyAccount::AssetLoan;

	QValueList<MyMoneyAccount::accountTypeE>::Iterator it;
	for(it = list.begin(); it != list.end(); ++it) {
		a.setAccountType(*it);
		try {
			char	msg[100];
			observer->reset();
			hierarchyObserver->reset();
			m->addAccount(a, i);
			sprintf(msg, "Can add non-equity type %d to investment", *it);
			CPPUNIT_FAIL(msg);
		} catch(MyMoneyException *e) {
			delete e;
		}
	}
	try {
		observer->reset();
		hierarchyObserver->reset();
		a.setAccountType(MyMoneyAccount::Stock);
		m->addAccount(a,i);
	} catch(MyMoneyException *e) {
		unexpectedException(e);
	}
}

void MyMoneyFileTest::testReparentEquity() {
	testAddEquityAccount();
	testAddEquityAccount();
	MyMoneyAccount parent;

	// check the bad cases
	QValueList<MyMoneyAccount::accountTypeE> list;
	list << MyMoneyAccount::Checkings;
	list << MyMoneyAccount::Savings;
	list << MyMoneyAccount::Cash;
	list << MyMoneyAccount::CertificateDep;
	list << MyMoneyAccount::MoneyMarket;
	list << MyMoneyAccount::Asset;
	list << MyMoneyAccount::AssetLoan;
	list << MyMoneyAccount::Currency;
	parent = m->asset();
	testReparentEquity(list, parent);

	list.clear();
	list << MyMoneyAccount::CreditCard;
	list << MyMoneyAccount::Loan;
	list << MyMoneyAccount::Liability;
	parent = m->liability();
	testReparentEquity(list, parent);

	list.clear();
	list << MyMoneyAccount::Income;
	parent = m->income();
	testReparentEquity(list, parent);

	list.clear();
	list << MyMoneyAccount::Expense;
	parent = m->expense();
	testReparentEquity(list, parent);

	// now check the good case
	MyMoneyAccount stock = m->account("A000002");
	MyMoneyAccount inv = m->account("A000003");
	try {
		observer->reset();
		hierarchyObserver->reset();
		m->reparentAccount(stock, inv);
	} catch(MyMoneyException *e) {
		unexpectedException(e);
	}
}

void MyMoneyFileTest::testReparentEquity(QValueList<MyMoneyAccount::accountTypeE>& list, MyMoneyAccount& parent)
{
	MyMoneyAccount a;
	a.setName("Testaccount");
	MyMoneyAccount stock = m->account("A000002");

	QValueList<MyMoneyAccount::accountTypeE>::Iterator it;
	for(it = list.begin(); it != list.end(); ++it) {
		a.setAccountType(*it);
		try {
			m->addAccount(a, parent);
			char	msg[100];
			observer->reset();
			hierarchyObserver->reset();
			m->reparentAccount(stock, a);
			sprintf(msg, "Can reparent stock to non-investment type %d account", *it);
			CPPUNIT_FAIL(msg);
		} catch(MyMoneyException *e) {
			delete e;
		}
	}
}

void MyMoneyFileTest::testBaseCurrency(void)
{
	MyMoneySecurity base("EUR", "Euro", QChar(0x20ac));
	MyMoneySecurity ref;

	// make sure, no base currency is set
	try {
		ref = m->baseCurrency();
		CPPUNIT_ASSERT(ref.id().isEmpty());
	} catch(MyMoneyException *e) {
		unexpectedException(e);
	}

	// make sure, we cannot assign an unknown currency
	try {
		m->setBaseCurrency(base);
		CPPUNIT_FAIL("Missing expected exception");
	} catch(MyMoneyException *e) {
		delete e;
	}

	// add the currency and try again
	try {
		m->addCurrency(base);
		m->setBaseCurrency(base);
	} catch(MyMoneyException *e) {
		unexpectedException(e);
	}

	// make sure, the base currency is set
	try {
		ref = m->baseCurrency();
		CPPUNIT_ASSERT(ref.id() == "EUR");
		CPPUNIT_ASSERT(ref.name() == "Euro");
		CPPUNIT_ASSERT(ref.tradingSymbol() == QChar(0x20ac));
	} catch(MyMoneyException *e) {
		unexpectedException(e);
	}
}

void MyMoneyFileTest::testOpeningBalanceNoBase(void)
{
	MyMoneyAccount openingAcc;
	MyMoneySecurity base;

	try {
		base = m->baseCurrency();
		openingAcc = m->openingBalanceAccount(base);
		CPPUNIT_FAIL("Missing expected exception");
	} catch(MyMoneyException *e) {
		delete e;
	}
}

void MyMoneyFileTest::testOpeningBalance(void)
{
	MyMoneyAccount openingAcc;
	MyMoneySecurity second("USD", "US Dollar", "$");
	testBaseCurrency();

	try {
		openingAcc = m->openingBalanceAccount(m->baseCurrency());
		CPPUNIT_ASSERT(openingAcc.parentAccountId() == m->equity().id());
		CPPUNIT_ASSERT(openingAcc.name() == MyMoneyFile::OpeningBalancesPrefix);
		CPPUNIT_ASSERT(openingAcc.openingBalance() == MyMoneyMoney(0,1));
		CPPUNIT_ASSERT(openingAcc.openingDate() == QDate::currentDate());
	} catch(MyMoneyException *e) {
		unexpectedException(e);
	}

	// add a second currency
	try {
		m->addCurrency(second);
	} catch(MyMoneyException *e) {
		unexpectedException(e);
	}

	QString refName = QString("%1 (%2)").arg(MyMoneyFile::OpeningBalancesPrefix).arg("USD");
	try {
		openingAcc = m->openingBalanceAccount(second);
		CPPUNIT_ASSERT(openingAcc.parentAccountId() == m->equity().id());
		CPPUNIT_ASSERT(openingAcc.name() == refName);
		CPPUNIT_ASSERT(openingAcc.openingBalance() == MyMoneyMoney(0,1));
		CPPUNIT_ASSERT(openingAcc.openingDate() == QDate::currentDate());
	} catch(MyMoneyException *e) {
		unexpectedException(e);
	}
}

