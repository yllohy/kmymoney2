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

#ifdef HAVE_LIBCPPUNIT
#include "cppunit/TextTestRunner.h"
#include "cppunit/TextTestResult.h"
#include "cppunit/TestSuite.h"
#include "cppunit/extensions/HelperMacros.h"

#include "mymoneyutils.h"

#define private public
// #include "qdatastream.h"
#include "mymoneyexception.h"
#include "mymoneymoney.h"
#include "mymoneysplit.h"
#include "mymoneyinstitution.h"
#include "mymoneyaccount.h"
#include "mymoneytransaction.h"
#include "mymoneyfile.h"
#include "storage/mymoneyseqaccessmgr.h"
#include "storage/mymoneystoragebin.h"
#include "mymoneysubject.h"
#include "mymoneyobserver.h"
#undef private

#include "mymoneyexceptiontest.h"
#include "mymoneymoneytest.h"
#include "mymoneyobservertest.h"
#include "mymoneyinstitutiontest.h"
#include "mymoneysplittest.h"
#include "mymoneyaccounttest.h"
#include "mymoneytransactiontest.h"
#include "storage/mymoneyseqaccessmgrtest.h"
#include "storage/mymoneystoragebintest.h"
#include "mymoneyfiletest.h"

#include "cppunit/TextTestProgressListener.h"

class MyProgressListener : public CppUnit::TextTestProgressListener
{
	void startTest(CppUnit::Test *test) {
		QString name = test->getName().c_str();
		name = name.mid(2);		// cut off first 2 chars
		name = name.left(name.find('.'));
		if(m_name != name) {
			if(m_name != "")
				cout << endl;
			cout << "Running: " << name << endl;
			m_name = name;
		}
	}
private:
	QString m_name;
};

#endif


int
main(int argc, char** argv)
{
#ifdef HAVE_LIBCPPUNIT
#define THB CPPUNIT_TEST_SUITE_REGISTRATION( fixture )
  _CheckMemory_Init(0);

  CPPUNIT_TEST_SUITE_REGISTRATION(MyMoneyExceptionTest); 
  CPPUNIT_TEST_SUITE_REGISTRATION(MyMoneyMoneyTest); 
  CPPUNIT_TEST_SUITE_REGISTRATION(MyMoneyObserverTest);
  CPPUNIT_TEST_SUITE_REGISTRATION(MyMoneyInstitutionTest);
  CPPUNIT_TEST_SUITE_REGISTRATION(MyMoneySplitTest);
  CPPUNIT_TEST_SUITE_REGISTRATION(MyMoneyTransactionTest);
  CPPUNIT_TEST_SUITE_REGISTRATION(MyMoneyAccountTest);
  CPPUNIT_TEST_SUITE_REGISTRATION(MyMoneySeqAccessMgrTest);
  CPPUNIT_TEST_SUITE_REGISTRATION(MyMoneyStorageBinTest);
  CPPUNIT_TEST_SUITE_REGISTRATION(MyMoneyFileTest);

  CppUnit::TestFactoryRegistry &registry =
    CppUnit::TestFactoryRegistry::getRegistry();
  CppUnit::Test *suite = registry.makeTest();

  CppUnit::TextTestRunner* runner = new CppUnit::TextTestRunner();
  runner->addTest(suite);

  MyProgressListener progress;
  runner->eventManager().addListener(&progress);
  runner->run();

  delete runner;

  chkmem.CheckMemoryLeak( true );
  _CheckMemory_End();
#else
  std::cout << "libcppunit not installed. no automatic tests available."
		 << std::endl;
#endif
  return 0;
}
