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

/**
  * This class provides a dialog to edit the details pertaining to
  * a scheduled bill.
  *
  * @author Michael Edwardes
  * $Id: keditscheduledbilldlg.h,v 1.2 2002/02/17 22:26:01 mte Exp $
  *
  * @short Edit details for a scheduled bill.
**/
class KEditScheduledBillDlg : public kEditScheduledBillDlgDecl  {
   Q_OBJECT
private:
  MyMoneyFile *m_mymoneyfile;
  QString m_lastPayee;
  MyMoneyTransaction *m_transaction;

  void reloadFromFile(void);
  void readConfig(void);
  void writeConfig(void);

protected slots:
  void slotSplitClicked();

public: 
	KEditScheduledBillDlg(MyMoneyFile *file, QWidget *parent=0, const char *name=0);
	~KEditScheduledBillDlg();
};

#endif
