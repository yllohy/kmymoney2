/***************************************************************************
                          kschedulelistitem.h
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes
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

#ifndef KSCHEDULELISTITEM_H
#define KSCHEDULELISTITEM_H

#include <kglobal.h>
#include <klocale.h>
#include <klistview.h>

#include <qwidget.h>
#include <qlistview.h>
#include "./mymoney/mymoneyscheduled.h"
#include "mymoney/mymoneybank.h"
#include "mymoney/mymoneyaccount.h"

/**
  *@author Michael Edwardes
  */

class KScheduleListItem : public QListViewItem  {
//   Q_OBJECT
  MyMoneyScheduled::s_scheduleData m_data;
public:
  KScheduleListItem(QListView *parent, MyMoneyScheduled::s_scheduleData scheduleData, QString bankName, QString accountName );
	~KScheduleListItem();
	MyMoneyScheduled::s_scheduleData data(void);
};

#endif
