/***************************************************************************
                          mymoneystoragebintest.h
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

#ifndef __MYMONEYSTORAGEBINTEST_H__
#define __MYMONEYSTORAGEBINTEST_H__

#include <cppunit/TestCaller.h>
#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>
#include <cppunit/extensions/HelperMacros.h>

#define private public
#include "mymoneystoragebin.h"
#undef private

class MyMoneyStorageBinTest : public CppUnit::TestFixture  {
  CPPUNIT_TEST_SUITE(MyMoneyStorageBinTest);
  CPPUNIT_TEST(testEmptyConstructor);
  CPPUNIT_TEST(testReadOldV3MyMoneyFile);
  CPPUNIT_TEST(testReadOldV4MyMoneyFile);
  CPPUNIT_TEST(testReadOldMyMoneyFileEx);
  CPPUNIT_TEST_SUITE_END();

protected:
  MyMoneyStorageBin *m;
public:
  MyMoneyStorageBinTest();


	void setUp();
	void tearDown();
	void testEmptyConstructor();
	void testReadOldV3MyMoneyFile();
	void testReadOldV4MyMoneyFile ();
	void testReadNewMyMoneyFile ();
	void testReadOldMyMoneyFileEx();

private:
void appendV3FileInfo(QDataStream& s, QString version, unsigned long magic);
void appendV3Institution(QDataStream& s,
       const QString name,
       const QString city,
       const QString street,
       const QString postcode,
       const QString telephone,
       const QString manager,
       const QString sortcode);
void appendV3Account(QDataStream& s,
      unsigned long magic,
      const QString& name,
      const QString& desc,
      const QString& number,
      const Q_INT32& type,
      const QDate& open,
      const QDate& reconcile,
      const MyMoneyMoney& balance);
void appendV3Transaction(QDataStream& s,
        unsigned int id,
        const QString& number,
        const QString& payee,
        double amount,
        const QDate& date,
        Q_UINT32 method,
        const QString& major,
        const QString& minor,
        const QString& atmBank,
        const QString& fromAccount,
        const QString& toAccount,
        const QString& memo,
        Q_INT32 state);
void createOldFile(const QString version, unsigned long magic);
void testReadOldMyMoneyFile(const QString version, unsigned long magic);
};

#endif
