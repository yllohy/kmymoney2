/***************************************************************************
                          mymoneymoneytest.cpp
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

#include "mymoneymoneytest.h"

MyMoneyMoneyTest::MyMoneyMoneyTest()
{
}


void MyMoneyMoneyTest::setUp()
{
	m_0 = new MyMoneyMoney(12);
	m_1 = new MyMoneyMoney(-10);
	m_2 = new MyMoneyMoney(2);
}

void MyMoneyMoneyTest::tearDown()
{
	delete m_0;
	delete m_1;
	delete m_2;
}

void MyMoneyMoneyTest::testEmptyConstructor()
{
	MyMoneyMoney *m = new MyMoneyMoney();
	CPPUNIT_ASSERT(m->value() == 0);
	delete m;
}

void MyMoneyMoneyTest::testAssignment()
{
	MyMoneyMoney *m = new MyMoneyMoney();
	*m = *m_1;
	CPPUNIT_ASSERT(m->value() == -10);

	*m = 0;
	CPPUNIT_ASSERT(m->value() == 0);

	*m = 777888999;
	CPPUNIT_ASSERT(m->value() == 777888999);

	*m = (int)-5678;
	CPPUNIT_ASSERT(m->value() == -5678);

	*m = QString("-987");
	CPPUNIT_ASSERT(m->value() == -987);

	*m = QString("999888777666555444");
	CPPUNIT_ASSERT(m->value() == 999888777666555444LL);

	*m = -666555444333222111LL;
	CPPUNIT_ASSERT(m->value() == -666555444333222111LL);

	delete m;
}

void MyMoneyMoneyTest::testStringConstructor()
{
	MyMoneyMoney *m1 = new MyMoneyMoney("-999666555444");
	CPPUNIT_ASSERT(m1->value() == -999666555444LL);

	MyMoneyMoney *m2 = new MyMoneyMoney("444555666999");
	CPPUNIT_ASSERT(m2->value() == 444555666999LL);

	*m1 = 444555666999LL;
	CPPUNIT_ASSERT(*m1 == *m2);

	delete m1;
	delete m2;
}

void MyMoneyMoneyTest::testEquality()
{
	CPPUNIT_ASSERT (*m_1 == *m_1);
	CPPUNIT_ASSERT (!(*m_1 == *m_0));

	MyMoneyMoney m1(999666555444LL);
	MyMoneyMoney m2(999666555444LL);
	CPPUNIT_ASSERT(m1 == m2);

	MyMoneyMoney m3(-999666555444LL);
	MyMoneyMoney m4(-999666555444LL);
	CPPUNIT_ASSERT(m3 == m4);
}

void MyMoneyMoneyTest::testInequality()
{
	CPPUNIT_ASSERT (*m_1 != *m_0);
	CPPUNIT_ASSERT (!(*m_1 != *m_1));

	MyMoneyMoney m1(999666555444LL);
	MyMoneyMoney m2(-999666555444LL);
	CPPUNIT_ASSERT(m1 != m2);
	CPPUNIT_ASSERT(m1 != (int)97);

	MyMoneyMoney m3(-999666555444LL);
	MyMoneyMoney m4(999666555444LL);
	CPPUNIT_ASSERT(m3 != m4);
}


void MyMoneyMoneyTest::testAddition()
{
	CPPUNIT_ASSERT (*m_0 + *m_1 == *m_2);

	MyMoneyMoney m1(100);

	CPPUNIT_ASSERT((m1+50) == 150);
	CPPUNIT_ASSERT((m1+1000000000) == 1000000100);
	CPPUNIT_ASSERT((m1+-50) == 50);

	CPPUNIT_ASSERT((m1 += *m_0) == 112);
	CPPUNIT_ASSERT((m1 += -12) == 100);

	m1++;
	CPPUNIT_ASSERT(m1 == 101);
	CPPUNIT_ASSERT((++m1) == 102);
}

void MyMoneyMoneyTest::testSubtraction()
{
	CPPUNIT_ASSERT (*m_2 - *m_1 == *m_0);

	MyMoneyMoney m1(100);

	CPPUNIT_ASSERT((m1-50) == 50);
	CPPUNIT_ASSERT((m1-1000000000) == -999999900);
	CPPUNIT_ASSERT((m1 - -50) == 150);

	CPPUNIT_ASSERT((m1 -= *m_0) == 88);
	CPPUNIT_ASSERT((m1 -= -12) == 100);

	m1--;
	CPPUNIT_ASSERT(m1 == 99);
	CPPUNIT_ASSERT((--m1) == 98);
}

void MyMoneyMoneyTest::testMultiplication()
{
	MyMoneyMoney m1(100);

	CPPUNIT_ASSERT((m1 * 50) == 5000);
	CPPUNIT_ASSERT((m1 * 10000000) == 1000000000);
	CPPUNIT_ASSERT((m1 * (*m_0)) == 1200);
}

void MyMoneyMoneyTest::testFormatMoney()
{
	CPPUNIT_ASSERT(m_0->formatMoney() == QString("0.12"));
	CPPUNIT_ASSERT(m_1->formatMoney() == QString("-0.10"));

	MyMoneyMoney m1(10099);
	CPPUNIT_ASSERT(m1.formatMoney() == QString("100.99"));

	m1 = 10000;
	CPPUNIT_ASSERT(m1.formatMoney() == QString("100.00"));
}

void MyMoneyMoneyTest::testRelation()
{
	MyMoneyMoney m1(100);
	MyMoneyMoney m2(50);
	MyMoneyMoney m3(100);

	CPPUNIT_ASSERT(m1 > m2);
	CPPUNIT_ASSERT(m2 < m1);

	CPPUNIT_ASSERT(m1 <= m3);
	CPPUNIT_ASSERT(m3 >= m1);
}

void MyMoneyMoneyTest::testUnaryMinus()
{
	MyMoneyMoney m1(100);
	MyMoneyMoney m2;

	m2 = -m1;

	CPPUNIT_ASSERT(m1 == 100);
	CPPUNIT_ASSERT(m2 == -100);
}

void MyMoneyMoneyTest::testDoubleConstructor()
{
	for(int i = -123456; i < 123456; ++i) {
		double d = i;
		MyMoneyMoney r(i);
		d /= 100;
		MyMoneyMoney t(d);
		CPPUNIT_ASSERT(t == r);
	}
}
