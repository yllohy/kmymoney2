/***************************************************************************
                          mymoneyinstitutiontest.cpp
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

#include "mymoneyinstitutiontest.h"

MyMoneyInstitutionTest::MyMoneyInstitutionTest()
{
}


void MyMoneyInstitutionTest::setUp () {
	m = new MyMoneyInstitution();
	n = new MyMoneyInstitution("name", "town", "street", "postcode",
			    "telephone", "manager", "sortcode");
}

void MyMoneyInstitutionTest::tearDown () {
	delete m;
	delete n;
}

void MyMoneyInstitutionTest::testEmptyConstructor() {
	CPPUNIT_ASSERT(m->id() == "");
	CPPUNIT_ASSERT(m->street() == "");
	CPPUNIT_ASSERT(m->town() == "");
	CPPUNIT_ASSERT(m->postcode() == "");
	CPPUNIT_ASSERT(m->telephone() == "");
	CPPUNIT_ASSERT(m->manager() == "");

	CPPUNIT_ASSERT(m->accountCount() == 0);
}

void MyMoneyInstitutionTest::testSetFunctions() {
	m->setStreet("street");
	m->setTown("town");
	m->setPostcode("postcode");
	m->setTelephone("telephone");
	m->setManager("manager");
	m->setName("name");

	CPPUNIT_ASSERT(m->id() == "");
	CPPUNIT_ASSERT(m->file() == 0);
	CPPUNIT_ASSERT(m->street() == "street");
	CPPUNIT_ASSERT(m->town() == "town");
	CPPUNIT_ASSERT(m->postcode() == "postcode");
	CPPUNIT_ASSERT(m->telephone() == "telephone");
	CPPUNIT_ASSERT(m->manager() == "manager");
	CPPUNIT_ASSERT(m->name() == "name");
}

void MyMoneyInstitutionTest::testNonemptyConstructor() {
	CPPUNIT_ASSERT(n->id() == "");
	CPPUNIT_ASSERT(n->file() == 0);
	CPPUNIT_ASSERT(n->street() == "street");
	CPPUNIT_ASSERT(n->town() == "town");
	CPPUNIT_ASSERT(n->postcode() == "postcode");
	CPPUNIT_ASSERT(n->telephone() == "telephone");
	CPPUNIT_ASSERT(n->manager() == "manager");
	CPPUNIT_ASSERT(n->name() == "name");
	CPPUNIT_ASSERT(n->sortcode() == "sortcode");
}

void MyMoneyInstitutionTest::testCopyConstructor() {
	MyMoneyInstitution* n1 = new MyMoneyInstitution("GUID1", *n);
	MyMoneyInstitution n2(*n1);

	CPPUNIT_ASSERT(*n1 == n2);

	delete n1;
}

void MyMoneyInstitutionTest::testMyMoneyFileConstructor() {
	MyMoneyInstitution *t = new MyMoneyInstitution("GUID", *n);

	CPPUNIT_ASSERT(t->id() == "GUID");

	CPPUNIT_ASSERT(t->street() == "street");
	CPPUNIT_ASSERT(t->town() == "town");
	CPPUNIT_ASSERT(t->postcode() == "postcode");
	CPPUNIT_ASSERT(t->telephone() == "telephone");
	CPPUNIT_ASSERT(t->manager() == "manager");
	CPPUNIT_ASSERT(t->name() == "name");
	CPPUNIT_ASSERT(t->sortcode() == "sortcode");
	
	delete t;
}

void MyMoneyInstitutionTest::testEquality () {
	MyMoneyInstitution t("name", "town", "street", "postcode",
			"telephone", "manager", "sortcode");

	CPPUNIT_ASSERT(t == *n);
	t.setStreet("x");
	CPPUNIT_ASSERT(!(t == *n));
	t.setStreet("street");
	CPPUNIT_ASSERT(t == *n);
	t.setName("x");
	CPPUNIT_ASSERT(!(t == *n));
	t.setName("name");
	CPPUNIT_ASSERT(t == *n);
	t.setTown("x");
	CPPUNIT_ASSERT(!(t == *n));
	t.setTown("town");
	CPPUNIT_ASSERT(t == *n);
	t.setPostcode("x");
	CPPUNIT_ASSERT(!(t == *n));
	t.setPostcode("postcode");
	CPPUNIT_ASSERT(t == *n);
	t.setTelephone("x");
	CPPUNIT_ASSERT(!(t == *n));
	t.setTelephone("telephone");
	CPPUNIT_ASSERT(t == *n);
	t.setManager("x");
	CPPUNIT_ASSERT(!(t == *n));
	t.setManager("manager");
	CPPUNIT_ASSERT(t == *n);

	MyMoneyInstitution* n1 = new MyMoneyInstitution("GUID1", *n);
	MyMoneyInstitution* n2 = new MyMoneyInstitution("GUID1", *n);

	n1->addAccountId("A000001");
	n2->addAccountId("A000001");

	CPPUNIT_ASSERT(*n1 == *n2);

	delete n1;
	delete n2;
}

void MyMoneyInstitutionTest::testInequality () {
	MyMoneyInstitution* n1 = new MyMoneyInstitution("GUID0", *n);
	MyMoneyInstitution* n2 = new MyMoneyInstitution("GUID1", *n);
	MyMoneyInstitution* n3 = new MyMoneyInstitution("GUID2", *n);
	MyMoneyInstitution* n4 = new MyMoneyInstitution("GUID2", *n);

	CPPUNIT_ASSERT(!(*n1 == *n2));
	CPPUNIT_ASSERT(!(*n1 == *n3));
	CPPUNIT_ASSERT(*n3 == *n4);

	n3->addAccountId("A000001");
	n4->addAccountId("A000002");
	CPPUNIT_ASSERT(!(*n3 == *n4));

	delete n1;
	delete n2;
	delete n3;
	delete n4;
}

void MyMoneyInstitutionTest::testAccountIDList () {
	MyMoneyInstitution institution;
	QCStringList list;
	QString id;

	// list must be empty
	list = institution.accountList();
	CPPUNIT_ASSERT(list.count() == 0);

	// add one account
	institution.addAccountId("A000002");
	list = institution.accountList();
	CPPUNIT_ASSERT(list.count() == 1);
	CPPUNIT_ASSERT(list.contains("A000002") == 1);

	// adding same account shouldn't make a difference
	institution.addAccountId("A000002");
	list = institution.accountList();
	CPPUNIT_ASSERT(list.count() == 1);
	CPPUNIT_ASSERT(list.contains("A000002") == 1);

	// now add another account
	institution.addAccountId("A000001");
	list = institution.accountList();
	CPPUNIT_ASSERT(list.count() == 2);
	CPPUNIT_ASSERT(list.contains("A000002") == 1);
	CPPUNIT_ASSERT(list.contains("A000001") == 1);

	id = institution.removeAccountId("A000001");
	CPPUNIT_ASSERT(id == "A000001");
	list = institution.accountList();
	CPPUNIT_ASSERT(list.count() == 1);
	CPPUNIT_ASSERT(list.contains("A000002") == 1);
	
}

