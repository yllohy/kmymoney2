/***************************************************************************
                          mymoneyforecasttest.h
                          -------------------
    copyright            : (C) 2007 by Alvaro Soliverez
    email                : asoliverez@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __MYMONEYFORECASTTEST_H__
#define __MYMONEYFORECASTTEST_H__

#include <cppunit/extensions/HelperMacros.h>

#include "../mymoney/mymoneyfile.h"
#include "../mymoney/storage/mymoneyseqaccessmgr.h"

#define private public
#include "mymoneyforecast.h"
#undef private


class MyMoneyForecastTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE ( MyMoneyForecastTest );
    CPPUNIT_TEST ( testEmptyConstructor );
    CPPUNIT_TEST ( testDoForecast );
    CPPUNIT_TEST ( testDoForecastInit );
    CPPUNIT_TEST ( testGetForecastAccountList );
    CPPUNIT_TEST ( testCalculateAccountTrend );
    CPPUNIT_TEST ( testGetForecastBalance );
    CPPUNIT_TEST ( testIsForecastAccount );
    CPPUNIT_TEST ( testDoFutureScheduledForecast );
    CPPUNIT_TEST ( testDaysToMinimumBalance );
    CPPUNIT_TEST ( testDaysToZeroBalance );
    CPPUNIT_TEST_SUITE_END();



  public:
    MyMoneyForecastTest();
    void setUp ();
    void tearDown ();
    void testEmptyConstructor();
    void testDoForecast();
    void testDoForecastInit();
    void testGetForecastAccountList();
    void testCalculateAccountTrend();
    void testGetForecastBalance();
    void testIsForecastAccount();
    void testDoFutureScheduledForecast();
    void testDaysToMinimumBalance();
    void testDaysToZeroBalance();

  protected:
    MyMoneyForecast *m;

  private:
    MyMoneyAccount  *account;

    MyMoneySeqAccessMgr* storage;
    MyMoneyFile* file;
    
    MyMoneyMoney moT1;
    MyMoneyMoney moT2;
    MyMoneyMoney moT3;
    MyMoneyMoney moT4;
    MyMoneyMoney moT5;
    QCString acCash;

};

#endif
