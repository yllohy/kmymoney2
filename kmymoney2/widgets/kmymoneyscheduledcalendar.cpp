/***************************************************************************
                          kmymoneyscheduledcalendar.cpp  -  description
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

// ----------------------------------------------------------------------------
// QT Includes
#include <qpushbutton.h>
#include <qkeysequence.h>
#include <qcursor.h>
#include <qapplication.h>

// ----------------------------------------------------------------------------
// KDE Includes
#include <klocale.h>
#include <kpopupmenu.h>
#include <kmessagebox.h>

// ----------------------------------------------------------------------------
// Project Includes
#include "kmymoneyscheduledcalendar.h"
#include "kmymoneyscheduleddatetbl.h"
#include "../mymoney/mymoneyfile.h"

kMyMoneyScheduledCalendar::kMyMoneyScheduledCalendar(QWidget *parent, const char *name )
  : kMyMoneyCalendar(parent,name)
{
  QPushButton *pb1 = new QPushButton(i18n("Select Schedules"), this);

  kpopupmenu = new KPopupMenu(this);
  kpopupmenu->setCheckable(true);
  kpopupmenu->insertItem(i18n("Bills"), 0);
  kpopupmenu->insertItem(i18n("Deposits"), 1);
  kpopupmenu->insertItem(i18n("Transfers"), 2);
  kpopupmenu->connectItem(0, this, SLOT(slotSetViewBills()));
  kpopupmenu->connectItem(1, this, SLOT(slotSetViewDeposits()));
  kpopupmenu->connectItem(2, this, SLOT(slotSetViewTransfers())); 
  kpopupmenu->setItemChecked(0, true);
  kpopupmenu->setItemChecked(1, true);
  kpopupmenu->setItemChecked(2, true);
  pb1->setPopup(kpopupmenu);

  m_scheduledDateTable = new kMyMoneyScheduledDateTbl(this);
  setDateTable((kMyMoneyDateTbl*)m_scheduledDateTable);
  
  setUserButton1(true, pb1);
  
  init( QDate::currentDate() );

  connect(m_scheduledDateTable, SIGNAL(hoverSchedules(QValueList<MyMoneySchedule>, QDate)),
    this, SLOT(slotHoverSchedules(QValueList<MyMoneySchedule>, QDate)));

  connect(&briefWidget, SIGNAL(enterClicked(const MyMoneySchedule&)), this, SLOT(slotEnterClicked(const MyMoneySchedule&)));
}

kMyMoneyScheduledCalendar::~kMyMoneyScheduledCalendar()
{
}

void kMyMoneyScheduledCalendar::slotSetViewBills()
{
  kpopupmenu->setItemChecked(0, ((kpopupmenu->isItemChecked(0)) ? false : true));
  m_scheduledDateTable->filterBills(!kpopupmenu->isItemChecked(0));
}

void kMyMoneyScheduledCalendar::slotSetViewDeposits()
{
  kpopupmenu->setItemChecked(1, ((kpopupmenu->isItemChecked(1)) ? false : true));
  m_scheduledDateTable->filterDeposits(!kpopupmenu->isItemChecked(1));
}

void kMyMoneyScheduledCalendar::slotSetViewTransfers()
{
  kpopupmenu->setItemChecked(2, ((kpopupmenu->isItemChecked(2)) ? false : true));
  m_scheduledDateTable->filterTransfers(!kpopupmenu->isItemChecked(2));
}

void kMyMoneyScheduledCalendar::slotHoverSchedules(QValueList<MyMoneySchedule> list, QDate date)
{
  if (list.count() >= 1)
  {
    briefWidget.setSchedules(list, date);

    int h = briefWidget.height();
    int w = briefWidget.width();

    // Take off five pixels so the mouse cursor
    // will be over the widget
    QPoint p = QCursor::pos();
    if (p.y() + h > QApplication::desktop()->height())
    {
      p.setY(p.y() - (h-5));
    }
    else
      p.setY(p.y() - 5);

    if (p.x() + w > QApplication::desktop()->width())
    {
      p.setX(p.x() - (w-5));
    }
    else
      p.setX(p.x() - 5);

    briefWidget.move(p);
    briefWidget.show();
  }
  else
  {
    briefWidget.hide();
  }
}

void kMyMoneyScheduledCalendar::slotEnterClicked(const MyMoneySchedule& schedule)
{
  emit enterClicked(schedule);
}