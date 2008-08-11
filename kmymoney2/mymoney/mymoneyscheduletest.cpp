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

#include <iostream>

MyMoneyScheduleTest::MyMoneyScheduleTest()
{
}


void MyMoneyScheduleTest::setUp () {
}

void MyMoneyScheduleTest::tearDown () {
}

void MyMoneyScheduleTest::testEmptyConstructor() {
	MyMoneySchedule s;
	
	CPPUNIT_ASSERT(s.id().isEmpty());
	CPPUNIT_ASSERT(s.m_occurence == MyMoneySchedule::OCCUR_ANY);
	CPPUNIT_ASSERT(s.m_type == MyMoneySchedule::TYPE_ANY);
	CPPUNIT_ASSERT(s.m_paymentType == MyMoneySchedule::STYPE_ANY);
	CPPUNIT_ASSERT(s.m_fixed == false);
	CPPUNIT_ASSERT(!s.m_startDate.isValid());
	CPPUNIT_ASSERT(!s.m_endDate.isValid());
	CPPUNIT_ASSERT(!s.m_lastPayment.isValid());
	CPPUNIT_ASSERT(s.m_autoEnter == false);
	CPPUNIT_ASSERT(s.m_name.isEmpty());
	CPPUNIT_ASSERT(s.willEnd() == false);
}

void MyMoneyScheduleTest::testConstructor() {
	MyMoneySchedule s(	"A Name",
				MyMoneySchedule::TYPE_BILL,
				MyMoneySchedule::OCCUR_WEEKLY,
				MyMoneySchedule::STYPE_DIRECTDEBIT,
				QDate::currentDate(),
				QDate(),
				true,
				true);

	CPPUNIT_ASSERT(s.type() == MyMoneySchedule::TYPE_BILL);
	CPPUNIT_ASSERT(s.occurence() == MyMoneySchedule::OCCUR_WEEKLY);
	CPPUNIT_ASSERT(s.paymentType() == MyMoneySchedule::STYPE_DIRECTDEBIT);
	CPPUNIT_ASSERT(s.startDate() == QDate());
	CPPUNIT_ASSERT(s.willEnd() == false);
	CPPUNIT_ASSERT(s.isFixed() == true);
	CPPUNIT_ASSERT(s.autoEnter() == true);
	CPPUNIT_ASSERT(s.name() == "A Name");
	CPPUNIT_ASSERT(!s.m_endDate.isValid());
	CPPUNIT_ASSERT(!s.m_lastPayment.isValid());
}

void MyMoneyScheduleTest::testSetFunctions() {
	MyMoneySchedule s;
	
	s.setId("SCHED001");
	CPPUNIT_ASSERT(s.id() == "SCHED001");
	
	s.setType(MyMoneySchedule::TYPE_BILL);
	CPPUNIT_ASSERT(s.type() == MyMoneySchedule::TYPE_BILL);

	s.setEndDate(QDate::currentDate());
	CPPUNIT_ASSERT(s.endDate() == QDate::currentDate());
	CPPUNIT_ASSERT(s.willEnd() == true);
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

void MyMoneyScheduleTest::testOverdue()
{
	MyMoneySchedule sch_overdue;
	MyMoneySchedule sch_intime;

	// the following checks only work correctly, if currentDate() is
	// between the 1st and 27th. If it is between 28th and 31st
	// we don't perform them. Note: this should be fixed.
	if(QDate::currentDate().day() > 27 || QDate::currentDate().day() == 1) {
		std::cout << std::endl << "testOverdue() skipped because current day is between 28th and 2nd" << std::endl;
		return;
	}

	QDate startDate = QDate::currentDate().addDays(-1).addMonths(-23);
	QDate lastPaymentDate = QDate::currentDate().addDays(-1).addMonths(-1);

	QString ref = QString(
		"<!DOCTYPE TEST>\n"
		"<SCHEDULE-CONTAINER>\n"
		" <SCHEDULED_TX startDate=\"%1\" autoEnter=\"0\" weekendOption=\"2\" lastPayment=\"%2\" paymentType=\"8\" endDate=\"\" type=\"5\" id=\"SCH0002\" name=\"A Name\" fixed=\"0\" occurence=\"32\" >\n"
		"  <PAYMENTS>\n"
		"   <PAYMENT date=\"%3\" />\n"
		"  </PAYMENTS>\n"
		"  <TRANSACTION postdate=\"\" memo=\"Wohnung:Miete\" id=\"\" commodity=\"EUR\" entrydate=\"\" >\n"
		"   <SPLITS>\n"
		"    <SPLIT payee=\"P000001\" reconciledate=\"\" shares=\"96379/100\" action=\"\" number=\"\" reconcileflag=\"2\" memo=\"\" value=\"96379/100\" account=\"A000076\" />\n"
		"    <SPLIT payee=\"P000001\" reconciledate=\"\" shares=\"-96379/100\" action=\"\" number=\"\" reconcileflag=\"1\" memo=\"\" value=\"-96379/100\" account=\"A000276\" />\n"
		"   </SPLITS>\n"
		"   <KEYVALUEPAIRS>\n"
		"    <PAIR key=\"key\" value=\"value\" />\n"
		"   </KEYVALUEPAIRS>\n"
		"  </TRANSACTION>\n"
		" </SCHEDULED_TX>\n"
		"</SCHEDULE-CONTAINER>\n");
	QString ref_overdue = ref.arg(startDate.toString(Qt::ISODate))
	 .arg(lastPaymentDate.toString(Qt::ISODate))
	 .arg(lastPaymentDate.toString(Qt::ISODate));

	QString ref_intime = ref.arg(startDate.addDays(1).toString(Qt::ISODate))
	 .arg(lastPaymentDate.addDays(1).toString(Qt::ISODate))
	 .arg(lastPaymentDate.addDays(1).toString(Qt::ISODate));

	QDomDocument doc;
	QDomElement node;

	// std::cout << ref_intime << std::endl;
	try {
		doc.setContent(ref_overdue);
		node = doc.documentElement().firstChild().toElement();
		sch_overdue = MyMoneySchedule(node);
		doc.setContent(ref_intime);
		node = doc.documentElement().firstChild().toElement();
		sch_intime = MyMoneySchedule(node);

		CPPUNIT_ASSERT(sch_overdue.isOverdue() == true);
		CPPUNIT_ASSERT(sch_intime.isOverdue() == false);

	} catch(MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception");
	}
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
/* 
 * Test for a schedule where a payment hasn't yet been made. 
 * First payment is in the future.
*/
{
	MyMoneySchedule sch;
	QString future_sched = QString(
		"<!DOCTYPE TEST>\n"
		"<SCHEDULE-CONTAINER>\n"
		"<SCHEDULED_TX startDate=\"2007-02-17\" autoEnter=\"1\" weekendOption=\"2\" lastPayment=\"\" paymentType=\"1\" endDate=\"\" type=\"1\" id=\"SCH000058\" name=\"Car Tax\" fixed=\"1\" occurence=\"16384\" >\n"
		"  <PAYMENTS/>\n"
		"  <TRANSACTION postdate=\"\" memo=\"\" id=\"\" commodity=\"GBP\" entrydate=\"\" >\n"
		"  <SPLITS>\n"
		"    <SPLIT payee=\"P000044\" reconciledate=\"\" shares=\"-15000/100\" action=\"Withdrawal\" bankid=\"\" number=\"\" reconcileflag=\"0\" memo=\"\" value=\"-15000/100\" account=\"A000155\" />\n"
		"    <SPLIT payee=\"\" reconciledate=\"\" shares=\"15000/100\" action=\"Withdrawal\" bankid=\"\" number=\"\" reconcileflag=\"0\" memo=\"\" value=\"15000/100\" account=\"A000182\" />\n"
		"  </SPLITS>\n"
		"  <KEYVALUEPAIRS/>\n"
		"  </TRANSACTION>\n"
		"</SCHEDULED_TX>\n"
		"</SCHEDULE-CONTAINER>\n"
	);

	QDomDocument doc;
	QDomElement node;
	doc.setContent(future_sched);
	node = doc.documentElement().firstChild().toElement();

	try {
		sch = MyMoneySchedule(node);
		CPPUNIT_ASSERT(sch.nextPayment(QDate(2007,2,14)) == QDate(2007,2,17));
		CPPUNIT_ASSERT(sch.nextPayment(QDate(2007,2,17)) == QDate(2008,2,17));
		CPPUNIT_ASSERT(sch.nextPayment(QDate(2007,2,18)) == QDate(2008,2,17));

	} catch(MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception");
	}
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
	MyMoneySchedule sch;
	QString ref_ok = QString(
		"<!DOCTYPE TEST>\n"
		"<SCHEDULE-CONTAINER>\n"

		"<SCHEDULED_TX startDate=\"2003-12-31\" autoEnter=\"1\" weekendOption=\"0\" lastPayment=\"2006-01-31\" paymentType=\"2\" endDate=\"\" type=\"2\" id=\"SCH000032\" name=\"DSL\" fixed=\"0\" occurence=\"32\" >\n"
		" <PAYMENTS/>\n"
		" <TRANSACTION postdate=\"2006-02-28\" memo=\"\" id=\"\" commodity=\"EUR\" entrydate=\"\" >\n"
		"  <SPLITS>\n"
		"   <SPLIT payee=\"P000076\" reconciledate=\"\" shares=\"1200/100\" action=\"Deposit\" bankid=\"\" number=\"\" reconcileflag=\"0\" memo=\"\" value=\"1200/100\" account=\"A000076\" />\n"
		"   <SPLIT payee=\"\" reconciledate=\"\" shares=\"-1200/100\" action=\"Deposit\" bankid=\"\" number=\"\" reconcileflag=\"0\" memo=\"\" value=\"-1200/100\" account=\"A000009\" />\n"
		"  </SPLITS>\n"
		"  <KEYVALUEPAIRS/>\n"
		" </TRANSACTION>\n"
		"</SCHEDULED_TX>\n"

		"</SCHEDULE-CONTAINER>\n"
	);

	QDomDocument doc;
	QDomElement node;
	doc.setContent(ref_ok);
	node = doc.documentElement().firstChild().toElement();

	QDate startDate(2006,1,28);
	QDate endDate(2006,5,30);

	try {
		sch = MyMoneySchedule(node);
		QDate nextPayment = sch.nextPayment(startDate);
		QValueList<QDate> list = sch.paymentDates(nextPayment, endDate);
		CPPUNIT_ASSERT(list.count() == 3);
		CPPUNIT_ASSERT(list[0] == QDate(2006,2,28));
		CPPUNIT_ASSERT(list[1] == QDate(2006,3,31));
		CPPUNIT_ASSERT(list[2] == QDate(2006,4,30));

	} catch(MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception");
	}
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

void MyMoneyScheduleTest::testWriteXML() {
	MyMoneySchedule sch(	"A Name",
				MyMoneySchedule::TYPE_BILL,
				MyMoneySchedule::OCCUR_WEEKLY,
				MyMoneySchedule::STYPE_DIRECTDEBIT,
				QDate::currentDate(),
				QDate(),
				true,
				true);

	sch.setLastPayment(QDate::currentDate());
	sch.recordPayment(QDate::currentDate());
	sch.setId("SCH0001");

	MyMoneyTransaction t;
	t.setPostDate(QDate(2001,12,28));
	t.setEntryDate(QDate(2003,9,29));
	t.setId("T000000000000000001");
	t.setMemo("Wohnung:Miete");
	t.setCommodity("EUR");
	t.setValue("key", "value");

	MyMoneySplit s;
	s.setPayeeId("P000001");
	s.setShares(MyMoneyMoney(96379, 100));
	s.setValue(MyMoneyMoney(96379, 100));
	s.setAccountId("A000076");
	s.setBankID("SPID1");
	s.setReconcileFlag(MyMoneySplit::Reconciled);
	t.addSplit(s);

	s.setPayeeId("P000001");
	s.setShares(MyMoneyMoney(-96379, 100));
	s.setValue(MyMoneyMoney(-96379, 100));
	s.setAccountId("A000276");
	s.setBankID("SPID2");
	s.setReconcileFlag(MyMoneySplit::Cleared);
	s.clearId();
	t.addSplit(s);

	sch.setTransaction(t);

	QDomDocument doc("TEST");
	QDomElement el = doc.createElement("SCHEDULE-CONTAINER");
	doc.appendChild(el);
	sch.writeXML(doc, el);

	QString ref = QString(
		"<!DOCTYPE TEST>\n"
		"<SCHEDULE-CONTAINER>\n"
		" <SCHEDULED_TX startDate=\"%1\" autoEnter=\"1\" weekendOption=\"2\" lastPayment=\"%2\" paymentType=\"1\" endDate=\"\" type=\"1\" id=\"SCH0001\" name=\"A Name\" fixed=\"1\" occurence=\"4\" >\n"
		"  <PAYMENTS>\n"
		"   <PAYMENT date=\"%3\" />\n"
		"  </PAYMENTS>\n"
		"  <TRANSACTION postdate=\"2001-12-28\" memo=\"Wohnung:Miete\" id=\"\" commodity=\"EUR\" entrydate=\"2003-09-29\" >\n"
		"   <SPLITS>\n"
		"    <SPLIT payee=\"P000001\" reconciledate=\"\" shares=\"96379/100\" action=\"\" bankid=\"SPID1\" number=\"\" reconcileflag=\"2\" memo=\"\" value=\"96379/100\" id=\"S0001\" account=\"A000076\" />\n"
		"    <SPLIT payee=\"\" reconciledate=\"\" shares=\"-96379/100\" action=\"\" bankid=\"SPID2\" number=\"\" reconcileflag=\"1\" memo=\"\" value=\"-96379/100\" id=\"S0002\" account=\"A000276\" />\n"
		"   </SPLITS>\n"
		"   <KEYVALUEPAIRS>\n"
		"    <PAIR key=\"key\" value=\"value\" />\n"
		"   </KEYVALUEPAIRS>\n"
		"  </TRANSACTION>\n"
		" </SCHEDULED_TX>\n"
		"</SCHEDULE-CONTAINER>\n"
	).arg(QDate::currentDate().toString(Qt::ISODate))
	 .arg(QDate::currentDate().toString(Qt::ISODate))
	 .arg(QDate::currentDate().toString(Qt::ISODate));

	CPPUNIT_ASSERT(doc.toString() == ref);
}

void MyMoneyScheduleTest::testReadXML() {
	MyMoneySchedule sch;

	QString ref_ok1 = QString(
		"<!DOCTYPE TEST>\n"
		"<SCHEDULE-CONTAINER>\n"
		" <SCHEDULED_TX startDate=\"%1\" autoEnter=\"1\" weekendOption=\"2\" lastPayment=\"%2\" paymentType=\"1\" endDate=\"\" type=\"1\" id=\"SCH0002\" name=\"A Name\" fixed=\"1\" occurence=\"4\" >\n"
		"  <PAYMENTS>\n"
		"   <PAYMENT date=\"%3\" />\n"
		"  </PAYMENTS>\n"
		"  <TRANSACTION postdate=\"2001-12-28\" memo=\"Wohnung:Miete\" id=\"\" commodity=\"EUR\" entrydate=\"2003-09-29\" >\n"
		"   <SPLITS>\n"
		"    <SPLIT payee=\"P000001\" reconciledate=\"\" shares=\"96379/100\" action=\"\" bankid=\"SPID1\" number=\"\" reconcileflag=\"2\" memo=\"\" value=\"96379/100\" account=\"A000076\" />\n"
		"    <SPLIT payee=\"P000001\" reconciledate=\"\" shares=\"-96379/100\" action=\"\" bankid=\"SPID2\" number=\"\" reconcileflag=\"1\" memo=\"\" value=\"-96379/100\" account=\"A000276\" />\n"
		"   </SPLITS>\n"
		"   <KEYVALUEPAIRS>\n"
		"    <PAIR key=\"key\" value=\"value\" />\n"
		"   </KEYVALUEPAIRS>\n"
		"  </TRANSACTION>\n"
		" </SCHEDULED_TX>\n"
		"</SCHEDULE-CONTAINER>\n"
	).arg(QDate::currentDate().toString(Qt::ISODate))
	 .arg(QDate::currentDate().toString(Qt::ISODate))
	 .arg(QDate::currentDate().toString(Qt::ISODate));

	// diff to ref_ok1 is that we now have an empty entrydate
	// in the transaction parameters
	QString ref_ok2 = QString(
		"<!DOCTYPE TEST>\n"
		"<SCHEDULE-CONTAINER>\n"
		" <SCHEDULED_TX startDate=\"%1\" autoEnter=\"1\" weekendOption=\"2\" lastPayment=\"%2\" paymentType=\"1\" endDate=\"\" type=\"1\" id=\"SCH0002\" name=\"A Name\" fixed=\"1\" occurence=\"4\" >\n"
		"  <PAYMENTS>\n"
		"   <PAYMENT date=\"%3\" />\n"
		"  </PAYMENTS>\n"
		"  <TRANSACTION postdate=\"2001-12-28\" memo=\"Wohnung:Miete\" id=\"\" commodity=\"EUR\" entrydate=\"\" >\n"
		"   <SPLITS>\n"
		"    <SPLIT payee=\"P000001\" reconciledate=\"\" shares=\"96379/100\" action=\"\" bankid=\"SPID1\" number=\"\" reconcileflag=\"2\" memo=\"\" value=\"96379/100\" account=\"A000076\" />\n"
		"    <SPLIT payee=\"P000001\" reconciledate=\"\" shares=\"-96379/100\" action=\"\" bankid=\"SPID2\" number=\"\" reconcileflag=\"1\" memo=\"\" value=\"-96379/100\" account=\"A000276\" />\n"
		"   </SPLITS>\n"
		"   <KEYVALUEPAIRS>\n"
		"    <PAIR key=\"key\" value=\"value\" />\n"
		"   </KEYVALUEPAIRS>\n"
		"  </TRANSACTION>\n"
		" </SCHEDULED_TX>\n"
		"</SCHEDULE-CONTAINER>\n"
	).arg(QDate::currentDate().toString(Qt::ISODate))
	 .arg(QDate::currentDate().toString(Qt::ISODate))
	 .arg(QDate::currentDate().toString(Qt::ISODate));

	QString ref_false = QString(
		"<!DOCTYPE TEST>\n"
		"<SCHEDULE-CONTAINER>\n"
		" <SCHEDULE startDate=\"%1\" autoEnter=\"1\" weekendOption=\"2\" lastPayment=\"%2\" paymentType=\"1\" endDate=\"\" type=\"1\" id=\"SCH0002\" name=\"A Name\" fixed=\"1\" occurence=\"4\" >\n"
		"  <PAYMENTS count=\"1\" >\n"
		"   <PAYMENT date=\"%3\" />\n"
		"  </PAYMENTS>\n"
		"  <TRANSACTION postdate=\"2001-12-28\" memo=\"Wohnung:Miete\" id=\"\" commodity=\"EUR\" entrydate=\"2003-09-29\" >\n"
		"   <SPLITS>\n"
		"    <SPLIT payee=\"P000001\" reconciledate=\"\" shares=\"96379/100\" action=\"\" bankid=\"SPID1\" number=\"\" reconcileflag=\"2\" memo=\"\" value=\"96379/100\" account=\"A000076\" />\n"
		"    <SPLIT payee=\"P000001\" reconciledate=\"\" shares=\"-96379/100\" action=\"\" bankid=\"SPID2\" number=\"\" reconcileflag=\"1\" memo=\"\" value=\"-96379/100\" account=\"A000276\" />\n"
		"   </SPLITS>\n"
		"   <KEYVALUEPAIRS>\n"
		"    <PAIR key=\"key\" value=\"value\" />\n"
		"   </KEYVALUEPAIRS>\n"
		"  </TRANSACTION>\n"
		" </SCHEDULED_TX>\n"
		"</SCHEDULE-CONTAINER>\n"
	).arg(QDate::currentDate().toString(Qt::ISODate))
	 .arg(QDate::currentDate().toString(Qt::ISODate))
	 .arg(QDate::currentDate().toString(Qt::ISODate));

	QDomDocument doc;
	QDomElement node;
	doc.setContent(ref_false);
	node = doc.documentElement().firstChild().toElement();

	try {
		sch = MyMoneySchedule(node);
		CPPUNIT_FAIL("Missing expected exception");
	} catch(MyMoneyException *e) {
		delete e;
	}

	doc.setContent(ref_ok1);
	node = doc.documentElement().firstChild().toElement();


	try {
		sch = MyMoneySchedule(node);
		CPPUNIT_ASSERT(sch.id() == "SCH0002");
		CPPUNIT_ASSERT(sch.nextDueDate() == QDate::currentDate().addDays(7));
		CPPUNIT_ASSERT(sch.startDate() == QDate::currentDate());
		CPPUNIT_ASSERT(sch.endDate() == QDate());
		CPPUNIT_ASSERT(sch.autoEnter() == true);
		CPPUNIT_ASSERT(sch.isFixed() == true);
		CPPUNIT_ASSERT(sch.weekendOption() == MyMoneySchedule::MoveNothing);
		CPPUNIT_ASSERT(sch.lastPayment() == QDate::currentDate());
		CPPUNIT_ASSERT(sch.paymentType() == MyMoneySchedule::STYPE_DIRECTDEBIT);
		CPPUNIT_ASSERT(sch.type() == MyMoneySchedule::TYPE_BILL);
		CPPUNIT_ASSERT(sch.name() == "A Name");
		CPPUNIT_ASSERT(sch.occurence() == MyMoneySchedule::OCCUR_WEEKLY);
		CPPUNIT_ASSERT(sch.nextDueDate() == sch.lastPayment().addDays(7));
		CPPUNIT_ASSERT(sch.recordedPayments().count() == 1);
		CPPUNIT_ASSERT(sch.recordedPayments()[0] == QDate::currentDate());
	} catch(MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception");
	}

	doc.setContent(ref_ok2);
	node = doc.documentElement().firstChild().toElement();


	try {
		sch = MyMoneySchedule(node);
		CPPUNIT_ASSERT(sch.id() == "SCH0002");
		CPPUNIT_ASSERT(sch.nextDueDate() == QDate::currentDate().addDays(7));
		CPPUNIT_ASSERT(sch.startDate() == QDate::currentDate());
		CPPUNIT_ASSERT(sch.endDate() == QDate());
		CPPUNIT_ASSERT(sch.autoEnter() == true);
		CPPUNIT_ASSERT(sch.isFixed() == true);
		CPPUNIT_ASSERT(sch.weekendOption() == MyMoneySchedule::MoveNothing);
		CPPUNIT_ASSERT(sch.lastPayment() == QDate::currentDate());
		CPPUNIT_ASSERT(sch.paymentType() == MyMoneySchedule::STYPE_DIRECTDEBIT);
		CPPUNIT_ASSERT(sch.type() == MyMoneySchedule::TYPE_BILL);
		CPPUNIT_ASSERT(sch.name() == "A Name");
		CPPUNIT_ASSERT(sch.occurence() == MyMoneySchedule::OCCUR_WEEKLY);
		CPPUNIT_ASSERT(sch.nextDueDate() == sch.lastPayment().addDays(7));
		CPPUNIT_ASSERT(sch.recordedPayments().count() == 1);
		CPPUNIT_ASSERT(sch.recordedPayments()[0] == QDate::currentDate());
	} catch(MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception");
	}
}

void MyMoneyScheduleTest::testHasReferenceTo()
{
	MyMoneySchedule sch;
	QString ref_ok = QString(
		"<!DOCTYPE TEST>\n"
		"<SCHEDULE-CONTAINER>\n"
		" <SCHEDULED_TX startDate=\"%1\" autoEnter=\"1\" weekendOption=\"2\" lastPayment=\"%2\" paymentType=\"1\" endDate=\"\" type=\"1\" id=\"SCH0002\" name=\"A Name\" fixed=\"1\" occurence=\"4\" >\n"
		"  <PAYMENTS>\n"
		"   <PAYMENT date=\"%3\" />\n"
		"  </PAYMENTS>\n"
		"  <TRANSACTION postdate=\"2001-12-28\" memo=\"Wohnung:Miete\" id=\"\" commodity=\"EUR\" entrydate=\"2003-09-29\" >\n"
		"   <SPLITS>\n"
		"    <SPLIT payee=\"P000001\" reconciledate=\"\" shares=\"96379/100\" action=\"\" number=\"\" reconcileflag=\"2\" memo=\"\" value=\"96379/100\" account=\"A000076\" />\n"
		"    <SPLIT payee=\"P000001\" reconciledate=\"\" shares=\"-96379/100\" action=\"\" number=\"\" reconcileflag=\"1\" memo=\"\" value=\"-96379/100\" account=\"A000276\" />\n"
		"   </SPLITS>\n"
		"   <KEYVALUEPAIRS>\n"
		"    <PAIR key=\"key\" value=\"value\" />\n"
		"   </KEYVALUEPAIRS>\n"
		"  </TRANSACTION>\n"
		" </SCHEDULED_TX>\n"
		"</SCHEDULE-CONTAINER>\n"
	).arg(QDate::currentDate().toString(Qt::ISODate))
	 .arg(QDate::currentDate().toString(Qt::ISODate))
	 .arg(QDate::currentDate().toString(Qt::ISODate));

	QDomDocument doc;
	QDomElement node;
	doc.setContent(ref_ok);
	node = doc.documentElement().firstChild().toElement();

	try {
		sch = MyMoneySchedule(node);

	} catch(MyMoneyException *e) {
		delete e;
		CPPUNIT_FAIL("Unexpected exception");
	}

	CPPUNIT_ASSERT(sch.hasReferenceTo("P000001") == true);
	CPPUNIT_ASSERT(sch.hasReferenceTo("A000276") == true);
	CPPUNIT_ASSERT(sch.hasReferenceTo("A000076") == true);
	CPPUNIT_ASSERT(sch.hasReferenceTo("EUR") == true);
}

void MyMoneyScheduleTest::testAdjustedNextDueDate()
{
	MyMoneySchedule s;

	QDate dueDate(2007,9,3); // start on a monday
	for(int i = 0; i < 7; ++i) {
		s.setNextDueDate(dueDate);
		s.setWeekendOption(MyMoneySchedule::MoveNothing);
		CPPUNIT_ASSERT(s.adjustedNextDueDate() == dueDate);

		s.setWeekendOption(MyMoneySchedule::MoveFriday);
		switch(i) {
		    case 5: // saturday
		    case 6: // sunday
			break;
			CPPUNIT_ASSERT(s.adjustedNextDueDate() == QDate(2007,9,7));
		    default:
			CPPUNIT_ASSERT(s.adjustedNextDueDate() == dueDate);
			break;
		}

		s.setWeekendOption(MyMoneySchedule::MoveMonday);
		switch(i) {
		    case 5: // saturday
		    case 6: // sunday
			CPPUNIT_ASSERT(s.adjustedNextDueDate() == QDate(2007,9,10));
			break;
		    default:
			CPPUNIT_ASSERT(s.adjustedNextDueDate() == dueDate);
			break;
		}
		dueDate = dueDate.addDays(1);
	}
}

void MyMoneyScheduleTest::testModifyNextDueDate(void)
{
	MyMoneySchedule s;
	s.setStartDate(QDate(2007, 1, 1));
	s.setOccurence(MyMoneySchedule::OCCUR_MONTHLY);
	s.setNextDueDate(s.startDate().addMonths(1));
	s.setLastPayment(s.startDate());

	QValueList<QDate> dates;
	dates = s.paymentDates(QDate(2007,2,1), QDate(2007,2,1));
	CPPUNIT_ASSERT(s.nextDueDate() == QDate(2007,2,1));
	CPPUNIT_ASSERT(dates.count() == 1);
	CPPUNIT_ASSERT(dates[0] == QDate(2007,2,1));

	s.setNextDueDate(QDate(2007,1,24));

	dates = s.paymentDates(QDate(2007,2,1), QDate(2007,2,1));
	CPPUNIT_ASSERT(s.nextDueDate() == QDate(2007,1,24));
	CPPUNIT_ASSERT(dates.count() == 0);

	dates = s.paymentDates(QDate(2007,1,24), QDate(2007,1,24));
	CPPUNIT_ASSERT(dates.count() == 1);

	dates = s.paymentDates(QDate(2007,1,24), QDate(2007,2,24));
	CPPUNIT_ASSERT(dates.count() == 2);
	CPPUNIT_ASSERT(dates[0] == QDate(2007,1,24));
	CPPUNIT_ASSERT(dates[1] == QDate(2007,2,24));
	
}
