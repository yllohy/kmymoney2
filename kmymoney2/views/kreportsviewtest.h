/***************************************************************************
                          mymoneyaccounttest.h
                          -------------------
    copyright            : (C) 2002 by Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
                           Ace Jones <ace.jones@hotpop.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __KREPORTSVIEWTEST_H__
#define __KREPORTSVIEWTEST_H__

#include <cppunit/extensions/HelperMacros.h>
#include "../mymoney/mymoneyfile.h"
#include "../mymoney/storage/mymoneyseqaccessmgr.h"

class KReportsViewTest : public CppUnit::TestFixture  {
        CPPUNIT_TEST_SUITE(KReportsViewTest);
	CPPUNIT_TEST(testTest);
	CPPUNIT_TEST_SUITE_END();

private:
	MyMoneyAccount	*m;

  MyMoneySeqAccessMgr* storage;
  MyMoneyFile* file;

public:
	KReportsViewTest();
	void setUp ();
	void tearDown ();
	void testTest();
};

#endif
