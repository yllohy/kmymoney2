
/***************************************************************************
                          mymoneyinstitutiontest.h
                          -------------------
    copyright            : (C) 2004 by Kevin Tambascio
    email                : ktambascio@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __MYMONEYEQUITYTEST_H__
#define __MYMONEYEQUITYTEST_H__

#include <cppunit/extensions/HelperMacros.h>

#define private public
#include "mymoneyequity.h"
#undef private

class MyMoneyEquityTest : public CppUnit::TestFixture  {
	CPPUNIT_TEST_SUITE(MyMoneyEquityTest);
	CPPUNIT_TEST(testEmptyConstructor);
	CPPUNIT_TEST(testNonemptyConstructor);
	CPPUNIT_TEST(testCopyConstructor);
/*
	CPPUNIT_TEST(testSetFunctions);
	CPPUNIT_TEST(testMyMoneyFileConstructor);
	CPPUNIT_TEST(testEquality);
	CPPUNIT_TEST(testInequality);
	CPPUNIT_TEST(testAccountIDList);
*/
	CPPUNIT_TEST_SUITE_END();

protected:
	MyMoneyEquity	*m;

public:
	MyMoneyEquityTest();

	void setUp ();
	void tearDown ();
	void testEmptyConstructor();
	void testNonemptyConstructor();
	void testCopyConstructor();
	// void testSetFunctions();
	// void testMyMoneyFileConstructor();
	// void testEquality ();
	// void testInequality ();
	// void testAccountIDList ();
};

#endif
