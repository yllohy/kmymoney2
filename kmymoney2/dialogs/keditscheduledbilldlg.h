/***************************************************************************
                          keditscheduledbilldlg.h  -  description
                             -------------------
    begin                : Sun Feb 17 2002
    copyright            : (C) 2000-2002 by Michael Edwardes
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
#ifndef KEDITSCHEDULEDBILLDLG_H
#define KEDITSCHEDULEDBILLDLG_H

// ----------------------------------------------------------------------------
// QT Includes
#include <qwidget.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes
#include "keditschedbilldlgdecl.h"
#include "../mymoney/mymoneyfile.h"
#include "../mymoney/mymoneyscheduled.h"

/**
  * This class provides a dialog to edit the details pertaining to
  * a scheduled bill.
  *
  * @author Michael Edwardes
  * $Id: keditscheduledbilldlg.h,v 1.5 2003/07/17 11:32:13 mte Exp $
  *
  * @short Edit details for a scheduled bill.
**/
class KEditScheduledBillDlg : public kEditScheduledBillDlgDecl  {
   Q_OBJECT
private:
  /// Save last payee used for convenience
  QString m_lastPayee;

  /// The transaction details
  MyMoneyTransaction m_transaction;

  /// The schedule details.
  MyMoneySchedule m_schedule;
  
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
    * Reloads the widgets from the global transaction.
  **/
  void reloadWidgets(void);

  /**
    * Fills in all the widgets from a schedule.
  **/
  void loadWidgetsFromSchedule(void);

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
  
public:
  /**
    * Standard QWidget constructor.
  **/
	KEditScheduledBillDlg(const MyMoneySchedule& schedule, QWidget *parent=0, const char *name=0);

  /**
    * Standard destructor.
  **/
	~KEditScheduledBillDlg();

  /**
    * Returns the edited schedule.
    *
    * @return MyMoneySchedule The schedule details.
  **/
  MyMoneySchedule schedule(void);
};

#endif
