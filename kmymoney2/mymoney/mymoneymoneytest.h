/***************************************************************************
                          mymoneymoneytest.h
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

#ifndef __MYMONEYMONEYTEST_H__
#define __MYMONEYMONEYTEST_H__

#include <cppunit/TestCaller.h>
#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>

template<typename MONEYCLASS>
class MyMoneyMoneyTest : public CppUnit::TestCase  {
protected:
MyMoneyMoney *m_0, *m_1, *m_2, *m_3;
public:
	MyMoneyMoneyTest (std::string name) : CppUnit::TestCase (name) {}


void setUp () {
	m_0 = new MyMoneyMoney(12.3);
	m_1 = new MyMoneyMoney(-10.3);
	m_2 = new MyMoneyMoney(2.0);
}

void tearDown () {
	delete m_0;
	delete m_1;
	delete m_2;
}

void testEmptyConstructor() {
	MyMoneyMoney *m = new MyMoneyMoney();
	CPPUNIT_ASSERT(m->amount() == 0.0);
}

void testDoubleConstructor() {
	MyMoneyMoney *m = new MyMoneyMoney(1.9);
	CPPUNIT_ASSERT(m->amount() == 1.9);
	delete m;

	m = new MyMoneyMoney(-1.9);
	CPPUNIT_ASSERT(m->amount() == -1.9);
	delete m;
}

void testAssignment() {
	MyMoneyMoney *m = new MyMoneyMoney();
	*m = *m_1;
	CPPUNIT_ASSERT(m->amount() == -10.3);
}

void testStringConstructor() {
	// The string constructor tests currently crash in QString::QString()
	// MyMoneyMoney *m = new MyMoneyMoney("1.9");
	// CPPUNIT_ASSERT(m->amount() == 1.9);
}

void testEquality () {
	CPPUNIT_ASSERT (*m_1 == *m_1);
	CPPUNIT_ASSERT (!(*m_1 == *m_0));
}

void testInequality () {
	CPPUNIT_ASSERT (*m_1 != *m_0);
	CPPUNIT_ASSERT (!(*m_1 != *m_1));
}


void testAddition () {
	CPPUNIT_ASSERT (*m_0 + *m_1 == *m_2);
	}

void testSubtraction () {
	CPPUNIT_ASSERT (*m_2 - *m_1 == *m_0);
	}

virtual void registerTests(CppUnit::TestSuite *suite)
{
      suite->addTest (new CppUnit::TestCaller<MyMoneyMoneyTest<MONEYCLASS> > ("testEmptyConstructor()",
         &MyMoneyMoneyTest<MONEYCLASS>::testEmptyConstructor, *this));

      suite->addTest (new CppUnit::TestCaller<MyMoneyMoneyTest<MONEYCLASS> > ("testConstructor(double)",
         &MyMoneyMoneyTest<MONEYCLASS>::testDoubleConstructor, *this));

      suite->addTest (new CppUnit::TestCaller<MyMoneyMoneyTest<MONEYCLASS> > ("testConstructor(string)",
         &MyMoneyMoneyTest<MONEYCLASS>::testStringConstructor, *this));

      suite->addTest (new CppUnit::TestCaller<MyMoneyMoneyTest<MONEYCLASS> > ("testAssignment()",
         &MyMoneyMoneyTest<MONEYCLASS>::testAssignment, *this));

      suite->addTest (new CppUnit::TestCaller<MyMoneyMoneyTest<MONEYCLASS> > ("testEquality()",
         &MyMoneyMoneyTest<MONEYCLASS>::testEquality, *this));

      suite->addTest (new CppUnit::TestCaller<MyMoneyMoneyTest<MONEYCLASS> > ("testInequality()",
         &MyMoneyMoneyTest<MONEYCLASS>::testInequality, *this));

      suite->addTest (new CppUnit::TestCaller<MyMoneyMoneyTest<MONEYCLASS> > ("testAddition()",
         &MyMoneyMoneyTest<MONEYCLASS>::testAddition, *this));

      suite->addTest (new CppUnit::TestCaller<MyMoneyMoneyTest<MONEYCLASS> > ("testSubtraction()",
         &MyMoneyMoneyTest<MONEYCLASS>::testSubtraction, *this));
}

};

#endif
