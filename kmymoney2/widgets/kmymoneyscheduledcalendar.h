/***************************************************************************
                          kmymoneyscheduledcalendar.h  -  description
                             -------------------
    begin                : Wed Jul 2 2003
    copyright            : (C) 2000-2003 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KMYMONEYSCHEDULEDCALENDAR_H
#define KMYMONEYSCHEDULEDCALENDAR_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes
#include "kmymoneycalendar.h"
#include "kmymoneyscheduleddatetbl.h"
#include "kmymoneybriefschedule.h"

class KPopupMenu;
class kMyMoneyDateTbl;

/**
  * A representation of a calendar.
  *
  * Uses the base class kMyMoneyCalendar to actually render
  * the calendar.
  *
  * @author Michael Edwardes 2003
  *
**/
class kMyMoneyScheduledCalendar : public kMyMoneyCalendar  {
   Q_OBJECT
   
public:
  /**
    * Standard constructor.
  **/
  kMyMoneyScheduledCalendar(QWidget *parent=0, const char *name=0);

  /**
    * Standard destructor.
  **/
  ~kMyMoneyScheduledCalendar();

  /**
    * Dynamically set the Date Table
  **/
  void setDateTable(kMyMoneyDateTbl* tbl) { table = tbl; }

  void refresh(const QCString& accountId) { m_scheduledDateTable->refresh(accountId); }


protected slots:
  void slotSetViewBills();
  void slotSetViewDeposits();
  void slotSetViewTransfers();
  void slotHoverSchedules(QValueList<MyMoneySchedule>, QDate date);

private:
  KPopupMenu* kpopupmenu;
  kMyMoneyScheduledDateTbl *m_scheduledDateTable;
  KMyMoneyBriefSchedule briefWidget;
};

#endif
