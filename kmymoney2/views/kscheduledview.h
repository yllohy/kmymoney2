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
#include <kmymoney/mymoneyfile.h>
#include <kmymoney/mymoneyaccount.h>
#include "../widgets/kmymoneyscheduledcalendar.h"

class KPopupMenu;

/**
  * Contains all the scheduled transactions be they bills, deposits or transfers.
  * Encapsulates all the operations including adding, editing and deleting.
  * Used by the KMyMoneyView class to show the view.
  *
  * @author Michael Edwardes 2000-2002
  * $Id: kscheduledview.h,v 1.28 2007/08/24 09:01:12 ipwizard Exp $
  *
  * @short A class to encapsulate recurring transaction operations.
  */
class KScheduledView : public KScheduledViewDecl
{
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

public slots:
  void slotSelectSchedule(const QCString& schedule);
  void slotReloadView(void);

signals:
  void scheduleSelected(const MyMoneySchedule& schedule);
  void openContextMenu(void);
  void enterSchedule(void);

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
    * Shows the context menu when the user right clicks or presses
    * a 'windows' key when an item is selected.
    *
    * @param view a pointer to the view
    * @param item a pointer to the current selected listview item
    * @param pos The position to popup
    * @return none
  **/
  void slotListViewContextMenu(KListView* view, QListViewItem* item, const QPoint& pos);

  void slotListItemExecuted(QListViewItem*, const QPoint&, int);

  void slotAccountActivated(int);

  void slotListViewCollapsed(QListViewItem* item);
  void slotListViewExpanded(QListViewItem* item);

  void slotBriefEnterClicked(const MyMoneySchedule& schedule, const QDate&);

  void slotTimerDone(void);

  void slotSetSelectedItem(QListViewItem* item);

  void slotRearrange(void);

private:
  /// The selected schedule id in the list view.
  QCString m_selectedSchedule;

  /// Read config file
  void readConfig(void);

  /// Write config file
  void writeConfig(void);

  /**
    * Refresh the view.
    */
  void refresh(bool full=true, const QCString schedId = QCString());

  /**
    * Loads the accounts into the combo box.
    */
//  void loadAccounts(void);

  KPopupMenu *m_kaccPopup;
  QCStringList m_filterAccounts;
  bool m_openBills;
  bool m_openDeposits;
  bool m_openTransfers;
  bool m_openLoans;
  bool m_needReload;
};

#endif
