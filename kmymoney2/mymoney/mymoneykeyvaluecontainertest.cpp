/***************************************************************************
                          mymoneykeyvaluecontainertest.cpp
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

#include "mymoneykeyvaluecontainertest.h"

MyMoneyKeyValueContainerTest::MyMoneyKeyValueContainerTest()
{
}


void MyMoneyKeyValueContainerTest::setUp () {
	m = new MyMoneyKeyValueContainer;
}

void MyMoneyKeyValueContainerTest::tearDown () {
	delete m;
}

void MyMoneyKeyValueContainerTest::testEmptyConstructor() {
	CPPUNIT_ASSERT(m->m_kvp.count() == 0);
}

void MyMoneyKeyValueContainerTest::testRetrieveValue() {
	// load a value into the container
	m->m_kvp["Key"] = "Value";
	// make sure it's there
	CPPUNIT_ASSERT(m->m_kvp.count() == 1);
	CPPUNIT_ASSERT(m->m_kvp["Key"] == "Value");
	// now check that the access function works
	CPPUNIT_ASSERT(m->value("Key") == "Value");
	CPPUNIT_ASSERT(m->value("key").isEmpty());
}

void MyMoneyKeyValueContainerTest::testSetValue() {
	m->setValue("Key", "Value");
	CPPUNIT_ASSERT(m->m_kvp.count() == 1);
	CPPUNIT_ASSERT(m->m_kvp["Key"] == "Value");
}

void MyMoneyKeyValueContainerTest::testDeletePair() {
	m->setValue("Key", "Value");
	m->setValue("key", "value");
	CPPUNIT_ASSERT(m->m_kvp.count() == 2);
	m->deletePair("Key");
	CPPUNIT_ASSERT(m->m_kvp.count() == 1);
	CPPUNIT_ASSERT(m->value("Key").isEmpty());
	CPPUNIT_ASSERT(m->value("key") == "value");
}

void MyMoneyKeyValueContainerTest::testRetrieveList() {
	QMap<QCString, QString> copy;

	copy = m->pairs();
	CPPUNIT_ASSERT(copy.count() == 0);
	m->setValue("Key", "Value");
	m->setValue("key", "value");
	copy = m->pairs();
	CPPUNIT_ASSERT(copy.count() == 2);
	CPPUNIT_ASSERT(copy["Key"] == "Value");
	CPPUNIT_ASSERT(copy["key"] == "value");
}

void MyMoneyKeyValueContainerTest::testLoadList() {
	QMap<QCString, QString> copy;

	copy["Key"] = "Value";
	copy["key"] = "value";
	m->setPairs(copy);

	CPPUNIT_ASSERT(m->m_kvp.count() == 2);
	CPPUNIT_ASSERT(m->m_kvp["Key"] == "Value");
	CPPUNIT_ASSERT(m->m_kvp["key"] == "value");
}

