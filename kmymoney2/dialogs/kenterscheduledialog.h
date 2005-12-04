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

  // FIXME possibly move to KMyMoney2App
  void commitTransaction();

protected slots:
  void slotOK();
  void slotSplitClicked();
  void slotFromActivated(const QCString&);
  void slotToActivated(const QCString&);

  /**
    * Make sure the date is between a certain range valid for the schedule.
  **/
  bool checkDateInPeriod(const QDate& date);

signals:
  /**
    * This signal is emitted, when a new category name has been
    * entered by the user and this name is not known as account
    * by the MyMoneyFile object.
    * Before the signal is emitted, a MyMoneyAccount is constructed
    * by this object and filled with the desired name. All other members
    * of MyMoneyAccount will remain in their default state. Upon return,
    * the connected slot should have created the object in the MyMoneyFile
    * engine and filled the member @p id.
    *
    * @param acc reference to MyMoneyAccount object that caries the name
    *            and will return information about the created category.
    */
  void newCategory(MyMoneyAccount& acc);

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
  QCString theAccountId();
  QCString m_toAccountId, m_fromAccountId;
};

#endif
