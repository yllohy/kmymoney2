/***************************************************************************
                          autotest.cpp
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

#include "config.h"

#ifdef HAVE_LIBCPPUNIT
#include "cppunit/TextTestRunner.h"
#include "cppunit/TextTestResult.h"
#include "cppunit/TestSuite.h"

#include "qdatastream.h"
#include "mymoneymoney.h"

#include "mymoneymoneytest.h"

#endif

#include <vector>
#include <iostream>

int
main(int argc, char** argv)
{
#ifdef HAVE_LIBCPPUNIT
  CppUnit::TestSuite suite;
  MyMoneyMoneyTest<MyMoneyMoney> myt("MyMoneyMoneyTest<MyMoneyMoney>");

  myt.registerTests(&suite);

  CppUnit::TextTestResult res;

  suite.run( &res );
  std::cout << res << std::endl;
#else
  std::cout << "libcppunit not installed. no automatic tests available."
		 << std::endl;
#endif
  return 0;
}

