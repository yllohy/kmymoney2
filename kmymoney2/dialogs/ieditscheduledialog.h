/***************************************************************************
                          keditscheduledialog.h  -  description
                             -------------------
    begin                : Tue Jul 22 2003
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
#ifndef KEDITSCHEDULEDIALOG_H
#define KEDITSCHEDULEDIALOG_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "keditschedtransdlgdecl.h"
#include "../mymoney/mymoneyfile.h"
#include "../mymoney/mymoneyscheduled.h"


/**
  * @author Michael Edwardes, Thomas Baumgart
  */

class KEditScheduleDialog : public kEditScheduledTransferDlgDecl
{
  Q_OBJECT
public:
  /**
    * Standard QWidget constructor.
    **/
  KEditScheduleDialog(const QCString& action, const MyMoneySchedule& schedule, QWidget *parent=0, const char *name=0);

  /**
    * Standard destructor.
    **/
  ~KEditScheduleDialog();

  /**
    * Returns the edited schedule.
    *
    * @return MyMoneySchedule The schedule details.
    **/
  MyMoneySchedule schedule(void);

protected slots:
  /**
    * Called when the split button is clicked
    **/
  void slotSplitClicked();

  /**
    * Called when the 'will end at some time' check box is clicked.
    **/
  void slotWillEndToggled(bool on);

  /**
    * Called when the OK button is clicked.
    **/
  void okClicked();

  void slotRemainingChanged(const QString& text);
  void slotEndDateChanged(const QDate& date);

  void slotAmountChanged(const QString&);
  void slotAccountChanged(const QString&);
  void slotScheduleNameChanged(const QString&);
  void slotToChanged(const QString&);
  void slotMethodChanged(int);
  void slotPayeeChanged(const QString&);
  void slotDateChanged(const QDate&);
  void slotFrequencyChanged(int);
  void slotEstimateChanged();
  void slotCategoryChanged(const QString&);
  void slotAutoEnterChanged();
  void slotMemoChanged(const QString& text);


private:
  /// Save last payee used for convenience
  QString m_lastPayee;

  /// The transaction details
  MyMoneyTransaction m_transaction;

  /// The schedule details.
  MyMoneySchedule m_schedule;

  /// the action
  QCString m_actionType;

  /**
    * Sets up the widgets based on whats in MyMoneyFile.
    */
  void reloadFromFile(void);

  /**
    * Read stored settings.
    **/
  void readConfig(void);

  /**
    * Write setting to config file.
    **/
  void writeConfig(void);

  /**
    * Reloads the qidgets from the global schedule.
    **/
  void loadWidgetsFromSchedule(void);

  MyMoneySchedule::occurenceE comboToOccurence(void);
  void createSplits();
  bool checkCategory();
  void checkPayee();
  QCString theAccountId();
};

#endif
