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
//#include "../dialogs/keditscheduledbilldlg.h"
//#include "../dialogs/keditscheduleddepositdlg.h"
//#include "../dialogs/keditscheduledtransferdlg.h"
#include "../mymoney/mymoneyfile.h"
#include "../mymoney/mymoneyaccount.h"
#include "../widgets/kmymoneyscheduledcalendar.h"

class KPopupMenu;

/**
  * Contains all the scheduled transactions be they bills, deposits or transfers.
  * Encapsulates all the operations including adding, editing and deleting.
  * Used by the KMyMoneyView class to show the view.
  *
  * @author Michael Edwardes 2000-2002
  * $Id: kscheduledview.h,v 1.14 2003/07/23 14:09:32 mte Exp $
  *
  * @short A class to encapsulate recurring transaction operations.
  */
class KScheduledView : public kScheduledViewDecl {
   Q_OBJECT
   
public:
  /**
    * Standard constructor for QWidgets.
    */
  KScheduledView(QWidget *parent=0, const char *name=0);

  /**
    * Standard destructor.
    */
  ~KScheduledView();

  /**
    * Called by KMyMoneyView.
    */
  void show();

  void refreshView(void);
  
signals:
  /**
    * Emitted when this view is shown.
    */
  void signalViewActivated();

protected:
  /**
    * Re-implement the standard qt resize event.
    */
  void resizeEvent(QResizeEvent *);

  /**
    * Re-implement the update method from MyMoneyObserver.
    *
    * It just updates the account list in the combo box.  (We dont need to
    * update anything else.
    */
  void update(const QCString& account);

protected slots:
  /**
    * Edit button clicked.
    */
  void slotEditClicked();

  /**
    * Delete button clicked.
    */
  void slotDeleteClicked();

  /**
    * New Bill chosen.
    */
  void slotNewBill();

  /**
    * New Deposit chosen.
    */
  void slotNewDeposit();

  /**
    * New transfer chosen.
    */
  void slotNewTransfer();

  /**
    * Slot to handle the account selection in the combo box.
    *
    * @param accountName Const reference to the item string selected.
  **/
//  void slotAccountSelected(const QString& accountName);

  /**
    * Creates and shows the context menu when the user right clicks or presses
    * a 'windows' key when an item is selected.
    *
    * @param item The item
    * @param pos The position to popup
    * @param col The column where the click occurred
    * @return none
  **/
  void slotListViewContextMenu(QListViewItem *item, const QPoint& pos, int col);

  void slotListItemExecuted(QListViewItem*);

  void slotAccountActivated(int);

private:
  /// The account currently selected via the accounts view
//  QCString m_accountId;

  /// The selected schedule id in the list view.
  QCString m_selectedSchedule;

  /// Read config file
  void readConfig(void);

  /// Write config file
  void writeConfig(void);

  /**
    * Refresh the view.
    */
  void refresh(bool full=true, const QCString schedId="");

  /**
    * Loads the accounts into the combo box.
    */
//  void loadAccounts(void);

  KPopupMenu *m_kaccPopup;
  QCStringList m_filterAccounts;
};

#endif
