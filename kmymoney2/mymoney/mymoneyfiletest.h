/***************************************************************************
                          mymoneyfiletest.h
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

#ifndef __MYMONEYFILETEST_H__
#define __MYMONEYFILETEST_H__

#include <cppunit/extensions/HelperMacros.h>


#define private public
#define protected public
#include "mymoneyfile.h"
#include "storage/mymoneyseqaccessmgr.h"
#undef private
#undef protected

class TestObserverSet : public MyMoneyObserver
{
public:
        TestObserverSet() { m_updated.clear(); }
        void update(const QCString& id) { m_updated.append(id); };
        const QCStringList& updated(void) { return m_updated; };
        void reset(void) { m_updated.clear(); };
private:
        QCStringList m_updated;
};


class MyMoneyFileTest : public CppUnit::TestFixture  {
        CPPUNIT_TEST_SUITE(MyMoneyFileTest);
	CPPUNIT_TEST(testEmptyConstructor);
	CPPUNIT_TEST(testAddOneInstitution);
	CPPUNIT_TEST(testAddTwoInstitutions);
	CPPUNIT_TEST(testInstitutionRetrieval);
	CPPUNIT_TEST(testRemoveInstitution);
	CPPUNIT_TEST(testInstitutionListRetrieval);
	CPPUNIT_TEST(testInstitutionModify);
	CPPUNIT_TEST(testSetFunctions);
	CPPUNIT_TEST(testAddAccounts);
	CPPUNIT_TEST(testModifyAccount);
	CPPUNIT_TEST(testReparentAccount);
	CPPUNIT_TEST(testRemoveAccount);
	CPPUNIT_TEST(testRemoveAccountTree);
	CPPUNIT_TEST(testAccountListRetrieval);
	CPPUNIT_TEST(testAddTransaction);
	CPPUNIT_TEST(testHasActiveSplits);
	CPPUNIT_TEST(testIsStandardAccount);
	CPPUNIT_TEST(testModifyTransactionSimple);
	CPPUNIT_TEST(testModifyTransactionNewPostDate);
	CPPUNIT_TEST(testModifyTransactionNewAccount);
	CPPUNIT_TEST(testRemoveTransaction);
	CPPUNIT_TEST(testBalanceTotal);
	CPPUNIT_TEST(testSetAccountName);
	CPPUNIT_TEST(testAddPayee);
	CPPUNIT_TEST(testModifyPayee);
	CPPUNIT_TEST(testRemovePayee);
	CPPUNIT_TEST(testAddTransactionStd);
	CPPUNIT_TEST(testAttachStorage);
#if 0
	CPPUNIT_TEST(testMoveSplits);
#endif
	CPPUNIT_TEST_SUITE_END();
protected:
	MyMoneyFile	*m;
	MyMoneySeqAccessMgr*	storage;
	TestObserverSet *observer;

public:
	MyMoneyFileTest ();

	void setUp ();
	void tearDown ();
	void testEmptyConstructor();
	void testAddOneInstitution();
	void testAddTwoInstitutions();
	void testRemoveInstitution();
	void testInstitutionRetrieval ();
	void testInstitutionListRetrieval ();
	void testInstitutionModify();
	void testSetFunctions();
	void testAddAccounts();
	void testModifyAccount();
	void testReparentAccount();
	void testRemoveAccount();
	void testRemoveAccountTree();
	void testAccountListRetrieval ();
	void testAddTransaction ();
	void testIsStandardAccount();
	void testHasActiveSplits();
	void testModifyTransactionSimple();
	void testModifyTransactionNewPostDate();
	void testModifyTransactionNewAccount();
	void testRemoveTransaction ();
	void testBalanceTotal();
	void testSetAccountName();
	void testAddPayee();
	void testModifyPayee();
	void testRemovePayee();
	void testAddTransactionStd();
	void testAttachStorage();

private:
	void testRemoveStdAccount(const MyMoneyAccount& acc);
};

#endif
