/***************************************************************************
                          kmymoneycalendar.h  -  description
                             -------------------
    begin                : Wed Jul 2 2003
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

#ifndef KMYMONEYCALENDAR_H
#define KMYMONEYCALENDAR_H

// ----------------------------------------------------------------------------
// QT Includes
#include <qgridview.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes


/**
  * A representation of a calendar.
  *  
  * @author Michael Edwardes 2003
  *
**/
class kMyMoneyCalendar : public QGridView  {
   Q_OBJECT
   
public:
  /**
    * Standard constructor.
  **/
  kMyMoneyCalendar(QWidget *parent=0, const char *name=0);

  /**
    * Standard destructor.
  **/
  ~kMyMoneyCalendar();

protected:
  /**
    * Reimplement paintCell so we can custom draw the cells.
    *
    * @see QGridView
  **/
  void paintCell(QPainter *p, int row, int col);

  /**
    * Handle the resize events.
  **/
  void resizeEvent(QResizeEvent* e);

private:
  /// The number of columns and rows.  Dynamically set.
  int m_cols;
  int m_rows;
};

#endif
