/***************************************************************************
                          mymoneyscheduletest.cpp
                          -------------------
    copyright            : (C) 2002 by Michael Edwardes
    email                : mte@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "mymoneyscheduletest.h"

#include "mymoneysplit.h"
#include "mymoneymoney.h"

MyMoneyScheduleTest::MyMoneyScheduleTest()
{
}


void MyMoneyScheduleTest::setUp () {
}

void MyMoneyScheduleTest::tearDown () {
}

void MyMoneyScheduleTest::testEmptyConstructor() {
	MyMoneyFile *m_file = MyMoneyFile::instance();
	m_file = MyMoneyFile::instance();
  MyMoneySeqAccessMgr *storage = new MyMoneySeqAccessMgr;
	m_file->attachStorage(storage);

  MyMoneyInstitution institution;

	institution.setName("institution1");
	institution.setTown("town");
	institution.setStreet("street");
	institution.setPostcode("postcode");
	institution.setTelephone("telephone");
	institution.setManager("manager");
	institution.setSortcode("sortcode");

	QString id;

	try {
		m_file->addInstitution(institution);
	} catch(MyMoneyException *e) {
		CPPUNIT_FAIL("Unexpected exception");
		delete e;
	}

 	MyMoneyAccount  a;
	a.setAccountType(MyMoneyAccount::Checkings);
	a.setName("Account1");
	a.setInstitutionId(institution.id());

	try {
		MyMoneyAccount parent = m_file->asset();
		m_file->addAccount(a, parent);
    CPPUNIT_ASSERT(a.id() == "A000001");
	} catch(MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception!");
	}

  CPPUNIT_ASSERT(m_file->asset().accountList().count() == 1);


  MyMoneySchedule s;
	
	CPPUNIT_ASSERT(s.m_id == "");
	CPPUNIT_ASSERT(s.m_occurence == MyMoneySchedule::OCCUR_ANY);
}

void MyMoneyScheduleTest::testConstructor() {
	MyMoneySchedule s(	"A Name",
				MyMoneySchedule::TYPE_BILL,
				MyMoneySchedule::OCCUR_WEEKLY,
				MyMoneySchedule::STYPE_DIRECTDEBIT,
				QDate::currentDate(),
				false,
				true,
				true);

	CPPUNIT_ASSERT(s.type() == MyMoneySchedule::TYPE_BILL);
	CPPUNIT_ASSERT(s.occurence() == MyMoneySchedule::OCCUR_WEEKLY);
	CPPUNIT_ASSERT(s.paymentType() == MyMoneySchedule::STYPE_DIRECTDEBIT);
	CPPUNIT_ASSERT(s.startDate() == QDate::currentDate());
	CPPUNIT_ASSERT(s.willEnd() == false);
	CPPUNIT_ASSERT(s.isFixed() == true);
	CPPUNIT_ASSERT(s.autoEnter() == true);
	CPPUNIT_ASSERT(s.name() == "A Name");
}

void MyMoneyScheduleTest::testSetFunctions() {
	MyMoneySchedule s;
	
	s.setId("SCHED001");
	CPPUNIT_ASSERT(s.id() == "SCHED001");
	
	s.setType(MyMoneySchedule::TYPE_BILL);
	CPPUNIT_ASSERT(s.type() == MyMoneySchedule::TYPE_BILL);
}

void MyMoneyScheduleTest::testCopyConstructor() {
	MyMoneySchedule s;
	
	s.setId("SCHED001");
	s.setType(MyMoneySchedule::TYPE_BILL);
	
	MyMoneySchedule s2(s);

	CPPUNIT_ASSERT(s.id() == s2.id());
	CPPUNIT_ASSERT(s.type() == s2.type());
}

void MyMoneyScheduleTest::testAssignmentConstructor() {
	MyMoneySchedule s;
	
	s.setId("SCHED001");
	s.setType(MyMoneySchedule::TYPE_BILL);
	
	MyMoneySchedule s2 = s;

	CPPUNIT_ASSERT(s.id() == s2.id());
	CPPUNIT_ASSERT(s.type() == s2.type());
}

void MyMoneyScheduleTest::testSingleton() {
/*
	MyMoneyScheduled *m = MyMoneyScheduled::instance();
	CPPUNIT_ASSERT(m!=NULL);

	CPPUNIT_ASSERT(m->m_instance != NULL);
	CPPUNIT_ASSERT(m->m_nextId == 1);
*/
}

void MyMoneyScheduleTest::testAddSchedule()
{
/*
	MyMoneyScheduled *m = MyMoneyScheduled::instance();
	CPPUNIT_ASSERT(m!=NULL);
	
	try {

  	MyMoneySplit sp1;
  	sp1.setShares(MyMoneyMoney(1));
  	sp1.setValue(MyMoneyMoney(1));
  	sp1.setAccountId("MTE1");
  	sp1.setMemo("MTE1");
  	sp1.setPayeeId("MTE1");

  	MyMoneySplit sp2;
  	sp2.setShares(MyMoneyMoney(1));
  	sp2.setValue(MyMoneyMoney(1));
  	sp2.setAccountId("MTE2");
  	sp2.setMemo("MTE2");
  	sp2.setPayeeId("MTE2");

  	MyMoneyTransaction t;
  	t.addSplit(sp1);
  	t.addSplit(sp2);

  	MyMoneySchedule s1(	"s1",
				MyMoneySchedule::TYPE_BILL,
  				MyMoneySchedule::OCCUR_WEEKLY,
  				MyMoneySchedule::STYPE_DIRECTDEBIT,
  				QDate(2001, 1, 1),
  				false,
  				true,
  				true);
  	s1.setTransaction(t);
  	MyMoneySchedule s2(	"s2",
				MyMoneySchedule::TYPE_DEPOSIT,
  				MyMoneySchedule::OCCUR_MONTHLY,
  				MyMoneySchedule::STYPE_MANUALDEPOSIT,
  				QDate(2001, 2, 1),
  				false,
  				true,
  				true);
  	s2.setTransaction(t);
  	MyMoneySchedule s3(	"s3",
				MyMoneySchedule::TYPE_TRANSFER,
  				MyMoneySchedule::OCCUR_YEARLY,
  				MyMoneySchedule::STYPE_WRITECHEQUE,
  				QDate(2001, 3, 1),
  				false,
  				true,
  				true);
  	s3.setTransaction(t);

		
		m->addSchedule("A000001", s1);
		m->addSchedule("A000001", s2);
		m->addSchedule("A000001", s3);
	} catch(MyMoneyException *e) {
		char buf[256];
		sprintf(buf, "Unexpected exception: %s", e->what().latin1());
		CPPUNIT_FAIL(buf);
		delete e;
	}

	CPPUNIT_ASSERT(m->m_nextId == 4);
	CPPUNIT_ASSERT(m->m_accountsScheduled["A000001"].size() == 3);
*/
}

void MyMoneyScheduleTest::testAnyScheduled()
{
/*
	MyMoneyScheduled *m = MyMoneyScheduled::instance();
	CPPUNIT_ASSERT(m!=NULL);

	// Successes
	CPPUNIT_ASSERT(m->anyScheduled("A000001"));
	CPPUNIT_ASSERT(m->anyScheduled("A000001", MyMoneySchedule::TYPE_BILL));
	CPPUNIT_ASSERT(m->anyScheduled("A000001", MyMoneySchedule::TYPE_DEPOSIT));
	CPPUNIT_ASSERT(m->anyScheduled("A000001", MyMoneySchedule::TYPE_TRANSFER));
	CPPUNIT_ASSERT(m->anyScheduled("A000001", MyMoneySchedule::TYPE_ANY,
				MyMoneySchedule::OCCUR_MONTHLY));
	CPPUNIT_ASSERT(m->anyScheduled("A000001", MyMoneySchedule::TYPE_ANY,
				MyMoneySchedule::OCCUR_WEEKLY));
	CPPUNIT_ASSERT(m->anyScheduled("A000001", MyMoneySchedule::TYPE_ANY,
				MyMoneySchedule::OCCUR_YEARLY));
	CPPUNIT_ASSERT(m->anyScheduled("A000001", MyMoneySchedule::TYPE_ANY,
				MyMoneySchedule::OCCUR_ANY,
				MyMoneySchedule::STYPE_DIRECTDEBIT));
	CPPUNIT_ASSERT(m->anyScheduled("A000001", MyMoneySchedule::TYPE_ANY,
				MyMoneySchedule::OCCUR_ANY,
				MyMoneySchedule::STYPE_MANUALDEPOSIT));
	CPPUNIT_ASSERT(m->anyScheduled("A000001", MyMoneySchedule::TYPE_ANY,
				MyMoneySchedule::OCCUR_ANY,
				MyMoneySchedule::STYPE_WRITECHEQUE));

	// Failures
	CPPUNIT_ASSERT(m->anyScheduled("A000001", MyMoneySchedule::TYPE_BILL,
				MyMoneySchedule::OCCUR_MONTHLY) == false);
*/
}

void MyMoneyScheduleTest::testAnyOverdue()
{
/*
	MyMoneyScheduled *m = MyMoneyScheduled::instance();
	CPPUNIT_ASSERT(m!=NULL);

	try
	{
		CPPUNIT_ASSERT(m->anyOverdue("A000001"));
		CPPUNIT_ASSERT(m->anyOverdue("A000001", MyMoneySchedule::TYPE_BILL));
		CPPUNIT_ASSERT(m->anyOverdue("A000001", MyMoneySchedule::TYPE_TRANSFER));
		CPPUNIT_ASSERT(m->anyOverdue("A000001", MyMoneySchedule::TYPE_DEPOSIT));
	} catch(MyMoneyException *e) {
		char buf[256];
		sprintf(buf, "Unexpected exception: %s", e->what().latin1());
		CPPUNIT_FAIL(buf);
		delete e;
	}
*/
}

void MyMoneyScheduleTest::testGetSchedule()
{
/*
	MyMoneyScheduled *m = MyMoneyScheduled::instance();
	CPPUNIT_ASSERT(m!=NULL);

	try
	{
		MyMoneySchedule s = m->getSchedule("A000001", "SCHED00002");

		CPPUNIT_ASSERT(s.type() == MyMoneySchedule::TYPE_DEPOSIT);
		CPPUNIT_ASSERT(s.occurence() == MyMoneySchedule::OCCUR_MONTHLY);
		CPPUNIT_ASSERT(s.paymentType() == MyMoneySchedule::STYPE_MANUALDEPOSIT);
		CPPUNIT_ASSERT(s.startDate() == QDate(2001, 2, 1));
		CPPUNIT_ASSERT(s.willEnd() == false);
		CPPUNIT_ASSERT(s.isFixed() == true);
		CPPUNIT_ASSERT(s.autoEnter() == true);

		MyMoneyTransaction t = s.transaction();
		CPPUNIT_ASSERT(t.splitCount() == 2);

		s = m->getSchedule("A000001", "SCHED00005");

		CPPUNIT_FAIL("Exception expected while getting schedule SCHED00005");
	} catch (MyMoneyException *e)
	{
		delete e;
	}
*/
}

void MyMoneyScheduleTest::testGetScheduled()
{
/*
	MyMoneyScheduled *m = MyMoneyScheduled::instance();
	CPPUNIT_ASSERT(m!=NULL);

	try
	{
		QValueList<QString> testList;

		testList = m->getScheduled("A000001");
		CPPUNIT_ASSERT(testList.size() == 3);
		CPPUNIT_ASSERT(testList[0] == "SCHED00001");
		CPPUNIT_ASSERT(testList[1] == "SCHED00002");
		CPPUNIT_ASSERT(testList[2] == "SCHED00003");

		testList = m->getScheduled("A000001", MyMoneySchedule::TYPE_DEPOSIT);
		CPPUNIT_ASSERT(testList.size() == 1);
		CPPUNIT_ASSERT(testList[0] == "SCHED00002");

		testList = m->getScheduled("A000001", MyMoneySchedule::TYPE_BILL);
		CPPUNIT_ASSERT(testList.size() == 1);
		CPPUNIT_ASSERT(testList[0] == "SCHED00001");

		testList = m->getScheduled("A000001", MyMoneySchedule::TYPE_TRANSFER);
		CPPUNIT_ASSERT(testList.size() == 1);
		CPPUNIT_ASSERT(testList[0] == "SCHED00003");

		testList = m->getScheduled("A000001", MyMoneySchedule::TYPE_DEPOSIT,
				MyMoneySchedule::STYPE_MANUALDEPOSIT,
				MyMoneySchedule::OCCUR_MONTHLY);
		CPPUNIT_ASSERT(testList.size() == 1);
		CPPUNIT_ASSERT(testList[0] == "SCHED00002");

		testList = m->getScheduled("A000001", QDate(2001, 1, 1), QDate(2001, 2, 1));
		CPPUNIT_ASSERT(testList.size() == 2);
		CPPUNIT_ASSERT(testList[0] == "SCHED00001");
		CPPUNIT_ASSERT(testList[1] == "SCHED00002");

	} catch(MyMoneyException *e) {
		char buf[256];
		sprintf(buf, "Unexpected exception: %s", e->what().latin1());
		CPPUNIT_FAIL(buf);
		delete e;
	}
*/
}

void MyMoneyScheduleTest::testGetOverdue()
{
/*
	MyMoneyScheduled *m = MyMoneyScheduled::instance();
	CPPUNIT_ASSERT(m!=NULL);

	try
	{
		QValueList<QString> testList;

		testList = m->getOverdue("A000001");
		CPPUNIT_ASSERT(testList.size() == 3);
		CPPUNIT_ASSERT(testList[0] == "SCHED00001");
		CPPUNIT_ASSERT(testList[1] == "SCHED00002");
		CPPUNIT_ASSERT(testList[2] == "SCHED00003");

		testList = m->getOverdue("A000001", MyMoneySchedule::TYPE_DEPOSIT);
		CPPUNIT_ASSERT(testList.size() == 1);
		CPPUNIT_ASSERT(testList[0] == "SCHED00002");

		testList = m->getOverdue("A000001", MyMoneySchedule::TYPE_BILL);
		CPPUNIT_ASSERT(testList.size() == 1);
		CPPUNIT_ASSERT(testList[0] == "SCHED00001");

		testList = m->getOverdue("A000001", MyMoneySchedule::TYPE_TRANSFER);
		CPPUNIT_ASSERT(testList.size() == 1);
		CPPUNIT_ASSERT(testList[0] == "SCHED00003");

		testList = m->getOverdue("A000001", MyMoneySchedule::TYPE_DEPOSIT,
				MyMoneySchedule::STYPE_MANUALDEPOSIT,
				MyMoneySchedule::OCCUR_MONTHLY);
		CPPUNIT_ASSERT(testList.size() == 1);
		CPPUNIT_ASSERT(testList[0] == "SCHED00002");
	} catch(MyMoneyException *e) {
		char buf[256];
		sprintf(buf, "Unexpected exception: %s", e->what().latin1());
		CPPUNIT_FAIL(buf);
		delete e;
	}
*/
}

void MyMoneyScheduleTest::testNextPayment()
{
/*
	MyMoneyScheduled *m = MyMoneyScheduled::instance();
	CPPUNIT_ASSERT(m!=NULL);

	try
	{
		MyMoneySchedule s1 = m->getSchedule("A000001", "SCHED00001");
		MyMoneySchedule s2 = m->getSchedule("A000001", "SCHED00002");
		MyMoneySchedule s3 = m->getSchedule("A000001", "SCHED00003");

		QDate nextPayment1 = s1.nextPayment();
		QDate nextPayment2 = s2.nextPayment();
		QDate nextPayment3 = s3.nextPayment();
		
		CPPUNIT_ASSERT(nextPayment1.year() != 1900);
		CPPUNIT_ASSERT(nextPayment2.year() != 1900);
		CPPUNIT_ASSERT(nextPayment3.year() != 1900);
	} catch (MyMoneyException *e)
	{
		CPPUNIT_FAIL("Unexpected exception");
		delete e;
	}
*/
}

void MyMoneyScheduleTest::testPaymentDates()
{
/*
	MyMoneyScheduled *m = MyMoneyScheduled::instance();
	CPPUNIT_ASSERT(m!=NULL);

	try
	{
		MyMoneySchedule s1 = m->getSchedule("A000001", "SCHED00001");
		MyMoneySchedule s2 = m->getSchedule("A000001", "SCHED00002");
		MyMoneySchedule s3 = m->getSchedule("A000001", "SCHED00003");

		QValueList<QDate> payments1 = s1.paymentDates(QDate(2001, 1, 1), QDate(2001, 2, 1));
		QValueList<QDate> payments2 = s2.paymentDates(QDate(2001, 2, 1), QDate(2001, 6, 1));
		QValueList<QDate> payments3 = s3.paymentDates(QDate(2001, 3, 1), QDate(2005, 3, 1));

		CPPUNIT_ASSERT(payments1.size() == 5);
		CPPUNIT_ASSERT(payments2.size() == 5);
		CPPUNIT_ASSERT(payments3.size() == 5);
	} catch (MyMoneyException *e)
	{
		CPPUNIT_FAIL("Unexpected exception");
		delete e;
	}
*/
}

void MyMoneyScheduleTest::testReplaceSchedule()
{
/*
	MyMoneyScheduled *m = MyMoneyScheduled::instance();
	CPPUNIT_ASSERT(m!=NULL);

	try
	{
		MyMoneySchedule s = m->getSchedule("A000001", "SCHED00002");
		CPPUNIT_ASSERT(s.type() == MyMoneySchedule::TYPE_DEPOSIT);
		s.setType(MyMoneySchedule::TYPE_TRANSFER);
		m->replaceSchedule("A000001", "SCHED00002", s);
		s = m->getSchedule("A000001", "SCHED00002");
		CPPUNIT_ASSERT(s.type() == MyMoneySchedule::TYPE_TRANSFER);

	} catch(MyMoneyException *e) {
		char buf[256];
		sprintf(buf, "Unexpected exception: %s", e->what().latin1());
		CPPUNIT_FAIL(buf);
		delete e;
	}
*/
}

void MyMoneyScheduleTest::testRemoveSchedule()
{
/*
	MyMoneyScheduled *m = MyMoneyScheduled::instance();
	CPPUNIT_ASSERT(m!=NULL);

	try
	{
		QValueList<QString> testList;

		testList = m->getScheduled("A000001");
		CPPUNIT_ASSERT(testList.size() == 3);

		m->removeSchedule("A000001", "SCHED00002");

		testList = m->getScheduled("A000001");
		CPPUNIT_ASSERT(testList.size() == 2);

		m->getSchedule("A000001", "SCHED00002");

		CPPUNIT_FAIL("Exception expected while getting schedule SCHED00002");
	} catch (MyMoneyException *e)
	{
		delete e;
	}
*/
}


