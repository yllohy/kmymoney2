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

/**
  * The list view item that describes a scheduled transaction.
  *
  * @author Michael Edwardes
  * $id$
  */
class KScheduledListItem : public QListViewItem  {
public:
  KScheduledListItem(QListView *parent=0, const char *description="");
  ~KScheduledListItem();
};

#endif
