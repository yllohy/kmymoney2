/***************************************************************************
                          kscheduledlistitem.h  -  description
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
#ifndef KSCHEDULEDLISTITEM_H
#define KSCHEDULEDLISTITEM_H

// ----------------------------------------------------------------------------
// QT Includes
#include <qwidget.h>
#include <qlistview.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes
#include "../mymoney/mymoneyscheduled.h"

/**
  * The list view item that describes a scheduled transaction.
  *
  * @author Michael Edwardes
  * $id$
  */
class KScheduledListItem : public QListViewItem  {
public:
  /**
    * This constructor is used to create a child of the main list view widget.
    *
    * The child should be a descriptor for the schedule type and one of
    * Bill,
    * Deposit or
    * Transfer.
    *
    * Other types may be added in the future.
    *
    * @param parent The list view to be a child of.
    * @param description The (translated) description.
    *
    * @see MyMoneySchedule
  **/
  KScheduledListItem(QListView *parent, const char *description);
  
  /**
    * This constructor is used to create a child of one of the children
    * created by the above method.
    *
    * This child describes a schedule and represents the data in schedule.
    *
    * @param parent The list view item to be a child of.
    * @param accountId The account id the schedule is for.
    * @param schedule The schedule to be represented.
    *
    * @see MyMoneySchedule
  **/
  KScheduledListItem(KScheduledListItem *parent, const MyMoneySchedule& schedule, bool even);

  /**
    * Standard destructor.
  **/
  ~KScheduledListItem();

  /**
    * Returns the schedule id for the instance being represented.  To be used
    * selection slots by the view.
    *
    * Returns an empty string for the top level items.
    *
    * @param none.
    * @return The schedule id.
  **/
  QCString scheduleId(void) const { return m_id; }

protected:
  void paintCell(QPainter* p, const QColorGroup& cg, int column, int width, int align);

private:
  /// The schedule's id.
  QCString m_id;
  
  bool m_even;
  bool m_base;
};

#endif
