/***************************************************************************
                          kenterscheduledialog.h  -  description
                             -------------------
    begin                : Mon Sep 1 2003
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

#ifndef KENTERSCHEDULEDIALOG_H
#define KENTERSCHEDULEDIALOG_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qwidget.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "../dialogs/kenterscheduledialogdecl.h"
#include "../mymoney/mymoneyscheduled.h"


/**
  * @author Michael Edwardes, Thomas Baumgart
  */

class KEnterScheduleDialog : public kEnterScheduleDialogDecl
{
  Q_OBJECT
public: 
  KEnterScheduleDialog(QWidget *parent, const MyMoneySchedule& schedule, const QDate& date=QDate());
  ~KEnterScheduleDialog();

protected slots:
  void slotOK();
  void slotSplitClicked();
  void slotFromActivated(const QCString&);
  void slotToActivated(const QCString&);

  /**
    * Make sure the date is between a certain range valid for the schedule.
  **/
  bool checkDateInPeriod(const QDate& date);

private:
  MyMoneySchedule m_schedule;
  MyMoneyTransaction m_transaction;
  QDate m_schedDate;

  /**
    * This method is used to determine the amount of amortization
    * and interest in a loan payment transaction.
    */
  void calculateInterest(void);
  void initWidgets();
  bool checkData(void);
  void checkCategory();
  void setPayee();
  void setTo();
  void setFrom();
  void setCategory();
  void setMemo();
  void setAmount();
  void createSplits();
  void commitTransaction();
  QCString theAccountId();
  QCString m_toAccountId, m_fromAccountId;
};

#endif
