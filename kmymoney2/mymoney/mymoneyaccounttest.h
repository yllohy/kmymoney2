/***************************************************************************
                          mymoneyaccounttest.h
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

#ifndef __MYMONEYACCOUNTTEST_H__
#define __MYMONEYACCOUNTTEST_H__

#include <cppunit/extensions/HelperMacros.h>

#define private public
#include "mymoneyaccount.h"
#undef private

class MyMoneyAccountTest : public CppUnit::TestFixture  {
        CPPUNIT_TEST_SUITE(MyMoneyAccountTest);
	CPPUNIT_TEST(testEmptyConstructor);
	CPPUNIT_TEST(testConstructor);
	CPPUNIT_TEST(testSetFunctions);
	CPPUNIT_TEST(testCopyConstructor);
	CPPUNIT_TEST(testAssignmentConstructor);
/* removed with MyMoneyAccount::Transaction
	CPPUNIT_TEST(testTransactionList);
	CPPUNIT_TEST(testTransactionRetrieval);
	CPPUNIT_TEST(testBalance);
*/
	CPPUNIT_TEST(testSubAccounts);
	CPPUNIT_TEST(testEquality);
	CPPUNIT_TEST_SUITE_END();

protected:
	MyMoneyAccount	*m;

public:
	MyMoneyAccountTest();
	void setUp ();
	void tearDown ();
	void testEmptyConstructor();
	void testConstructor();
	void testSetFunctions();
	void testCopyConstructor();
	void testAssignmentConstructor();
	void testTransactionList();
	void testTransactionRetrieval();
	void testBalance();
	void testSubAccounts();
	void testEquality();
};

#endif
