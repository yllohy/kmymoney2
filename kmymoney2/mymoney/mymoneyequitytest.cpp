/***************************************************************************
                          mymoneyinstitutiontest.cpp
                          -------------------
    copyright            : (C) 2002 by Kevin Tambascio
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

#include "mymoneyequitytest.h"

MyMoneyEquityTest::MyMoneyEquityTest()
{
}


void MyMoneyEquityTest::setUp () {
	m = new MyMoneyEquity();
}

void MyMoneyEquityTest::tearDown () {
	delete m;
}

void MyMoneyEquityTest::testEmptyConstructor() {
	CPPUNIT_ASSERT(m->id().isEmpty());
	CPPUNIT_ASSERT(m->name().isEmpty());
	CPPUNIT_ASSERT(m->tradingSymbol().isEmpty());
	CPPUNIT_ASSERT(m->priceHistory().count() == 0);
}

void MyMoneyEquityTest::testCopyConstructor() {
	MyMoneyEquity* n1 = new MyMoneyEquity("GUID1", *m);
	MyMoneyEquity n2(*n1);

	// CPPUNIT_ASSERT(*n1 == n2);

	delete n1;
}

void MyMoneyEquityTest::testNonemptyConstructor() {
	QDate date(2004,4,1);
	MyMoneyMoney val("1234/100");

	m->setName("name");
	m->setTradingSymbol("symbol");
	m->setEquityType(MyMoneyEquity::ETYPE_CURRENCY);
	m->addPriceHistory(date, val);

	MyMoneyEquity n("id", *m);

	CPPUNIT_ASSERT(n.id() == QCString("id"));
	CPPUNIT_ASSERT(n.tradingSymbol() == QString("symbol"));
	CPPUNIT_ASSERT(n.equityType() == MyMoneyEquity::ETYPE_CURRENCY);
	CPPUNIT_ASSERT(n.priceHistory().count() == 1);
}

/*

void MyMoneyEquityTest::testSetFunctions() {
}

void MyMoneyEquityTest::testMyMoneyFileConstructor() {
	MyMoneyEquity *t = new MyMoneyEquity("GUID", *n);

	CPPUNIT_ASSERT(t->id() == "GUID");

	delete t;
}

void MyMoneyEquityTest::testEquality () {
}

void MyMoneyEquityTest::testInequality () {
}

void MyMoneyEquityTest::testAccountIDList () {
	MyMoneyEquity equity;
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
*/

