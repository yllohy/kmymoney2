/***************************************************************************
                          kscheduledview.h  -  description
                             -------------------
    begin                : Sun Jan 27 2002
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
#ifndef KSCHEDULEDVIEW_H
#define KSCHEDULEDVIEW_H

// ----------------------------------------------------------------------------
// QT Includes
#include <qwidget.h>
#include <qlistview.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes
#include "kscheduledviewdecl.h"
#include "../mymoney/mymoneyfile.h"
#include "../dialogs/keditscheduledbilldlg.h"
#include "../dialogs/keditscheduleddepositdlg.h"
#include "../dialogs/keditscheduledtransferdlg.h"

/**
  * Contains all the scheduled transactions be they bills or deposits.
  * Encapsulates all the operations including adding, editing and deleting.
  * Used by the KMyMoneyView class to show the view.
  *
  * @author Michael Edwardes 2000-2002
  * $Id: kscheduledview.h,v 1.4 2002/06/04 19:05:17 mte Exp $
  *
  * @short A class to encapsulate recurring transaction operations.
  */
class KScheduledView : public kScheduledViewDecl  {
   Q_OBJECT
private:
  MyMoneyFile *m_file;
  QString m_lastCat;

  // Remember state
  void readConfig(void);
  void writeConfig(void);
  void refresh(void);

protected:
  void resizeEvent(QResizeEvent *);

protected slots:
  // Called on appropriate signals
  void slotEditClicked();
  void slotDeleteClicked();
  void slotSelectionChanged(QListViewItem*);
  void slotNewBill();
  void slotNewDeposit();
  void slotNewTransfer();

signals:
  void signalViewActivated();

public:
  KScheduledView(QWidget *parent=0, const char *name=0);
  ~KScheduledView();
  void show();
};

#endif
