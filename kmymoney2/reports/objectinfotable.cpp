/***************************************************************************
                          objectinfotable.cpp
                         -------------------
    begin                : Sat 28 jun 2008
    copyright            : (C) 2004-2005 by Ace Jones
                               2008 by Alvaro Soliverez
    email                :  acejones@users.sourceforge.net
                            asoliverez@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// ----------------------------------------------------------------------------
// QT Includes
#include <qvaluelist.h>
#include <qfile.h>
#include <qtextstream.h>

// ----------------------------------------------------------------------------
// KDE Includes
// This is just needed for i18n().  Once I figure out how to handle i18n
// without using this macro directly, I'll be freed of KDE dependency.

#include <klocale.h>
#include <kdebug.h>

// ----------------------------------------------------------------------------
// Project Includes
#include "../mymoney/mymoneyfile.h"
#include "../mymoney/mymoneyreport.h"
#include "../mymoney/mymoneyexception.h"
#include "../kmymoneyutils.h"
#include "reportdebug.h"
#include "objectinfotable.h"

namespace reports {

// ****************************************************************************
//
// ObjectInfoTable implementation
//
// ****************************************************************************

/**
  * TODO
  *
  * - Collapse 2- & 3- groups when they are identical
  * - Way more test cases (especially splits & transfers)
  * - Option to collapse splits
  * - Option to exclude transfers
  *
  */

ObjectInfoTable::ObjectInfoTable(const MyMoneyReport& _report): ListTable(_report)
{
  // seperated into its own method to allow debugging (setting breakpoints
  // directly in ctors somehow does not work for me (ipwizard))
  // TODO: remove the init() method and move the code back to the ctor
  init();
}

void ObjectInfoTable::init ( void )
{
  switch ( m_config.rowType() )
  {
    case MyMoneyReport::eSchedule:
      constructScheduleTable();
      m_columns = "name";
    default:
      break;
  }

  // Sort the data to match the report definition
  m_subtotal="value";

  switch ( m_config.rowType() )
  {
    case MyMoneyReport::eSchedule:
      m_group = "type";
      break;
    default:
      throw new MYMONEYEXCEPTION ( "ObjectInfoTable::ObjectInfoTable(): unhandled row type" );
  }

  QString sort = m_group + "," + m_columns + ",id,rank";

  switch ( m_config.rowType() ) {
    case MyMoneyReport::eSchedule:
      m_columns="name,nextduedate,occurence,paymenttype,account,payee";
      break;
  
    case MyMoneyReport::eAccountByTopAccount:
    case MyMoneyReport::eEquityType:
    case MyMoneyReport::eAccountType:
    case MyMoneyReport::eInstitution:
     default:
      m_columns = "";
  }

  TableRow::setSortCriteria ( sort );
  qHeapSort ( m_rows );
}

void ObjectInfoTable::constructScheduleTable(void)
{
  MyMoneyFile* file = MyMoneyFile::instance();

  QValueList<MyMoneySchedule> schedules;

  schedules = file->scheduleList("", MyMoneySchedule::TYPE_ANY, MyMoneySchedule::OCCUR_ANY, MyMoneySchedule::STYPE_ANY, m_config.fromDate(), m_config.toDate());

  QValueList<MyMoneySchedule>::const_iterator it_schedule = schedules.begin();
  while ( it_schedule != schedules.end() )
  {
    MyMoneySchedule schedule = *it_schedule;

    MyMoneyAccount account = schedule.account();

    //get fraction for account
    int fraction = account.fraction();

      //use base currency fraction if not initialized
    if(fraction == -1)
      fraction = MyMoneyFile::instance()->baseCurrency().smallestAccountFraction();

    TableRow qschedulerow;

    // help for sort and render functions
    qschedulerow["rank"] = "0";
    qschedulerow["id"] = schedule.id();
    qschedulerow["name"] = schedule.name();
    qschedulerow["nextduedate"] = schedule.nextDueDate().toString(Qt::ISODate);
    qschedulerow["type"] = KMyMoneyUtils::scheduleTypeToString(schedule.type());
    qschedulerow["occurence"] = KMyMoneyUtils::occurenceToString(schedule.occurence());
    qschedulerow["paymenttype"] = KMyMoneyUtils::paymentMethodToString(schedule.paymentType());

    qschedulerow["account"] = account.name();

    //to get the payee we must look into the splits of the transaction
    MyMoneyTransaction transaction = schedule.transaction();
    MyMoneySplit split = transaction.splitByAccount(account.id(), true);
    qschedulerow["value"] = split.value().toString();
    MyMoneyPayee payee = file->payee(split.payeeId());
    qschedulerow["payee"] = payee.name();
    
    m_rows += qschedulerow;
    ++it_schedule;
  }
}

}
// vim:cin:si:ai:et:ts=2:sw=2:
