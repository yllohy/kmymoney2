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

#include <iostream>
#include <string>

#ifdef HAVE_LIBCPPUNIT

#include "cppunit/TextTestRunner.h"
#include "cppunit/TextTestResult.h"
#include "cppunit/TestSuite.h"
#include "cppunit/extensions/HelperMacros.h"

#include "mymoney/mymoneyutils.h"

#define private public
#include "mymoney/mymoneysubject.h"
#include "mymoney/mymoneyobserver.h"
#undef private

#include "mymoney/mymoneyexceptiontest.h"
#include "mymoney/mymoneymoneytest.h"
#include "mymoney/mymoneyobservertest.h"
#include "mymoney/mymoneyinstitutiontest.h"
#include "mymoney/mymoneysplittest.h"
#include "mymoney/mymoneyaccounttest.h"
#include "mymoney/mymoneytransactiontest.h"
#include "mymoney/storage/mymoneyseqaccessmgrtest.h"
#include "mymoney/storage/mymoneystoragebintest.h"
#include "mymoney/mymoneyfiletest.h"
#include "mymoney/mymoneykeyvaluecontainertest.h"
#include "mymoney/mymoneyscheduletest.h"
#include "mymoney/mymoneyfinancialcalculatortest.h"
#include "mymoney/mymoneyequitytest.h"

#include "views/kreportsviewtest.h"

#include "cppunit/TextTestProgressListener.h"

class MyProgressListener : public CppUnit::TextTestProgressListener
{
	void startTest(CppUnit::Test *test) {
		QString name = test->getName().c_str();
		name = name.mid(2);		// cut off first 2 chars
		name = name.left(name.find('.'));
		if(m_name != name) {
			if(m_name != "")
				std::cout << std::endl;
			std::cout << "Running: " << name << std::endl;
			m_name = name;
		}
	}
private:
	QString m_name;
};

void unexpectedException(MyMoneyException *e)
{
	std::string msg = "Unexpected exception: ";
	msg += e->what().latin1();
	msg += " thrown in ";
	msg += e->file().latin1();
	msg += ":";
	char line[8];
	sprintf(line, "%ld", e->line());
	msg += line;
	delete e;
	CPPUNIT_FAIL(msg);
}

#endif // HAVE_LIBCPPUNIT

int
main(int /* argc */, char** /* argv */ )
{
#ifdef HAVE_LIBCPPUNIT

#ifdef _CHECK_MEMORY
  _CheckMemory_Init(0);
#endif

  // mymoney tests
  CPPUNIT_TEST_SUITE_REGISTRATION(MyMoneyExceptionTest); 
  CPPUNIT_TEST_SUITE_REGISTRATION(MyMoneyKeyValueContainerTest);
  CPPUNIT_TEST_SUITE_REGISTRATION(MyMoneyObserverTest);
  CPPUNIT_TEST_SUITE_REGISTRATION(MyMoneyMoneyTest); 
  CPPUNIT_TEST_SUITE_REGISTRATION(MyMoneySplitTest);
  CPPUNIT_TEST_SUITE_REGISTRATION(MyMoneyTransactionTest);
  CPPUNIT_TEST_SUITE_REGISTRATION(MyMoneyFinancialCalculatorTest);
  CPPUNIT_TEST_SUITE_REGISTRATION(MyMoneyInstitutionTest);
  CPPUNIT_TEST_SUITE_REGISTRATION(MyMoneyAccountTest);
  CPPUNIT_TEST_SUITE_REGISTRATION(MyMoneySeqAccessMgrTest);
  CPPUNIT_TEST_SUITE_REGISTRATION(MyMoneyStorageBinTest);
  CPPUNIT_TEST_SUITE_REGISTRATION(MyMoneyFileTest);
  CPPUNIT_TEST_SUITE_REGISTRATION(MyMoneyScheduleTest);
  CPPUNIT_TEST_SUITE_REGISTRATION(MyMoneyEquityTest);

  // views tests
  CPPUNIT_TEST_SUITE_REGISTRATION(KReportsViewTest);

  // off we go
  CppUnit::TestFactoryRegistry &registry =
    CppUnit::TestFactoryRegistry::getRegistry();
  CppUnit::Test *suite = registry.makeTest();

  CppUnit::TextTestRunner* runner = new CppUnit::TextTestRunner();
  runner->addTest(suite);

  MyProgressListener progress;
  runner->eventManager().addListener(&progress);
  runner->run();

  delete runner;

  // make sure to delete the singletons before we start memory checking
  // to avoid false error reports
  // delete MyMoneyFile::instance();

#ifdef _CHECK_MEMORY
  chkmem.CheckMemoryLeak( true );
  _CheckMemory_End();
#endif // _CHECK_MEMORY

#else
  std::cout << "libcppunit not installed. no automatic tests available."
		 << std::endl;
#endif // HAVE_LIBCPPUNIT
  return 0;
}
