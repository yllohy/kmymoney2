/***************************************************************************
                          kmymoneyforecastlistviewitem.h
                             -------------------
    begin                : Sun Nov 25 2007
    copyright            : (C) 2007 by Alvaro Soliverez
    email                : asoliverez@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KMYMONEYFORECASTLISTVIEWITEM_H
#define KMYMONEYFORECASTLISTVIEWITEM_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qobject.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klistview.h>

// ----------------------------------------------------------------------------
// Project Includes

/**
  * This class implements a derived version of a KListViewItem that
  * allows printing negative numbers in red
  *
  * @author Alvaro Soliverez
  */
class KMyMoneyForecastListViewItem : public KListViewItem
{
public:
  
  KMyMoneyForecastListViewItem(QListView* parent, QListViewItem* after, bool isNegative);
  
  ~KMyMoneyForecastListViewItem();
  
  void setNegative(bool isNegative);

  /**
    * use my own paint method
    */
  void paintCell(QPainter *p, const QColorGroup &cg, int column, int width, int alignment);


private:
  
  bool m_negative;

};

#endif
