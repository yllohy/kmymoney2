/***************************************************************************
                          mymoneytransactiontest.h
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

#ifndef __MYMONEYTRANSACTIONTEST_H__
#define __MYMONEYTRANSACTIONTEST_H__

/*
#include <cppunit/TestCaller.h>
#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>
*/
#include <cppunit/extensions/HelperMacros.h>
#include "autotest.h"

#define private public
#include "mymoneytransaction.h"
#undef private

class MyMoneyTransactionTest : public CppUnit::TestFixture  {
        CPPUNIT_TEST_SUITE(MyMoneyTransactionTest);
	CPPUNIT_TEST(testEmptyConstructor);
	CPPUNIT_TEST(testSetFunctions);
	CPPUNIT_TEST(testConstructor);
	CPPUNIT_TEST(testCopyConstructor);
	CPPUNIT_TEST(testAssignmentConstructor);
	CPPUNIT_TEST(testAddSplits);
	CPPUNIT_TEST(testModifySplits);
	CPPUNIT_TEST(testDeleteSplits);
	CPPUNIT_TEST(testDeleteAllSplits);
	CPPUNIT_TEST(testEquality);
	CPPUNIT_TEST(testInequality);
	CPPUNIT_TEST(testExtractSplit);
	CPPUNIT_TEST(testSplitSum);
        CPPUNIT_TEST_SUITE_END();

protected:
	MyMoneyTransaction *m;

public:
	MyMoneyTransactionTest ();

	void setUp ();
	void tearDown ();
	void testEmptyConstructor();
	void testSetFunctions();
	void testConstructor();
	void testCopyConstructor();
	void testAssignmentConstructor();
	void testEquality();
	void testInequality();
	void testAddSplits();
	void testModifySplits();
	void testDeleteSplits();
	void testExtractSplit();
	void testDeleteAllSplits();
	void testSplitSum();
};
#endif
