/***************************************************************************
                          mymoneyexceptiontest.h
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

#ifndef __MYMONEYEXCEPTIONTEST_H__
#define __MYMONEYEXCEPTIONTEST_H__

#include <cppunit/TestCaller.h>
#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>

class MyMoneyExceptionTest : public CppUnit::TestFixture  {
	CPPUNIT_TEST_SUITE(MyMoneyExceptionTest);
	CPPUNIT_TEST(testDefaultConstructor);
	CPPUNIT_TEST(testConstructor);
	CPPUNIT_TEST_SUITE_END();

protected:
public:
	MyMoneyExceptionTest() {};


void setUp()
{
}

void tearDown()
{
}

void testDefaultConstructor()
{
	MyMoneyException *e = new MYMONEYEXCEPTION("Message");
	CPPUNIT_ASSERT(e->what() == "Message");
	CPPUNIT_ASSERT(e->line() == __LINE__-2);
	CPPUNIT_ASSERT(e->file() == __FILE__);
	delete e;
}

void testConstructor()
{
	MyMoneyException *e = new MyMoneyException("New message",
						 "Joe's file", 1234);
	CPPUNIT_ASSERT(e->what() == "New message");
	CPPUNIT_ASSERT(e->line() == 1234);
	CPPUNIT_ASSERT(e->file() == "Joe's file");
	delete e;
}

};
#endif
