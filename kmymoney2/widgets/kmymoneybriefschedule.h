/***************************************************************************
                          kmymoneybriefschedule.h  -  description
                             -------------------
    begin                : Sun Jul 6 2003
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
#ifndef KMYMONEYBRIEFSCHEDULE_H
#define KMYMONEYBRIEFSCHEDULE_H


// ----------------------------------------------------------------------------
// QT Includes
#include <qwidget.h>
#include <qstringlist.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes
#include "kschedulebriefwidget.h"
#include "../mymoney/mymoneyfile.h"

/**
  *@author Michael Edwardes
  */

class KMyMoneyBriefSchedule : public kScheduleBriefWidget  {
   Q_OBJECT
public: 
  KMyMoneyBriefSchedule(QWidget *parent=0, const char *name=0);
  ~KMyMoneyBriefSchedule();
  void setSchedules(QValueList<MyMoneySchedule> list);

protected slots:
  void slotPrevClicked();
  void slotNextClicked();

private:
  QValueList<MyMoneySchedule> m_scheduleList;
  unsigned int m_index;

  void loadSchedule(unsigned int index);
};

#endif
