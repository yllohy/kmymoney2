/***************************************************************************
                          keditscheduledtransferdlg.h  -  description
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
#ifndef KEDITSCHEDULEDTRANSFERDLG_H
#define KEDITSCHEDULEDTRANSFERDLG_H

// ----------------------------------------------------------------------------
// QT Includes
#include <qwidget.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes
#include "../mymoney/mymoneyfile.h"
#include "keditschedtransdlgdecl.h"
#include "../mymoney/mymoneyscheduled.h"

/**
  * This class provides a dialog to edit the details pertaining to
  * a scheduled transfer.
  *
  * @author Michael Edwardes
  * $Id: keditscheduledtransferdlg.h,v 1.3 2003/01/24 14:23:22 mte Exp $
  *
  * @short Edit details for a scheduled transfer.
**/
class KEditScheduledTransferDlg : public kEditScheduledTransferDlgDecl  {
   Q_OBJECT
private:
  /// Save last payee used for convenience
  QString m_lastPayee;

  /// The account we're scheduling for
  QCString m_accountId;

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
	KEditScheduledTransferDlg(const QCString& accountId, QWidget *parent=0, const char *name=0);

  /**
    * Standard destructor.
  **/
	~KEditScheduledTransferDlg();

  /**
    * Returns the edited schedule.
    *
    * @return MyMoneySchedule The schedule details.
  **/
  MyMoneySchedule schedule(void);
};

#endif
