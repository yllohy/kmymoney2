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

#include <qwidget.h>
#include "keditschedbilldlgdecl.h"

/**
  *@author Michael Edwardes
  */

class KEditScheduledBillDlg : public kEditScheduledBillDlgDecl  {
   Q_OBJECT
public: 
	KEditScheduledBillDlg(QWidget *parent=0, const char *name=0);
	~KEditScheduledBillDlg();
};

#endif
