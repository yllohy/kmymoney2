/***************************************************************************
                          mymoneyobjecttest.cpp
                          -------------------
    copyright            : (C) 2005 by Thomas Baumgart
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

#include "mymoneyobjecttest.h"

MyMoneyObjectTest::MyMoneyObjectTest()
{
}


void MyMoneyObjectTest::setUp () {
}

void MyMoneyObjectTest::tearDown () {
}

void MyMoneyObjectTest::testEmptyConstructor() {
	MyMoneyObject a;
	CPPUNIT_ASSERT(a.id().isEmpty());
}

void MyMoneyObjectTest::testConstructor() {
	MyMoneyObject a(QCString("thb"));

	CPPUNIT_ASSERT(!a.id().isEmpty());
	CPPUNIT_ASSERT(a.id() == QCString("thb"));
}

void MyMoneyObjectTest::testClearId() {
	MyMoneyObject a(QCString("thb"));

	CPPUNIT_ASSERT(!a.id().isEmpty());
	a.clearId();
	CPPUNIT_ASSERT(a.id().isEmpty());
}

void MyMoneyObjectTest::testCopyConstructor() {
	MyMoneyObject a(QCString("thb"));
	MyMoneyObject b(a);

	CPPUNIT_ASSERT(a.id() == b.id());
}

void MyMoneyObjectTest::testAssignmentConstructor() {
	MyMoneyObject a(QCString("thb"));
	MyMoneyObject b = a;

	CPPUNIT_ASSERT(a.id() == b.id());
}

void MyMoneyObjectTest::testEquality() {
	MyMoneyObject a(QCString("thb"));
	MyMoneyObject b(QCString("thb"));
	MyMoneyObject c(QCString("ace"));

	CPPUNIT_ASSERT(a == b);
	CPPUNIT_ASSERT(!(a == c));
}

void MyMoneyObjectTest::testRTTI() {
	MyMoneyObject a;
	CPPUNIT_ASSERT(a.rtti() == MyMoneyObject::UnknownObject);
	a.setRtti(MyMoneyObject::Account);
	CPPUNIT_ASSERT(a.rtti() == MyMoneyObject::MyMoneyObject::Account);
}
