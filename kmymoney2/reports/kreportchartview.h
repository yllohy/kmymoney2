/***************************************************************************
                          kreportchartview.h
                             -------------------
    begin                : Sat May 22 2004
    copyright            : (C) 2004-2005 by Ace Jones
    email                : <ace.j@hotpop.com>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KREPORTCHARTVIEW_H
#define KREPORTCHARTVIEW_H

// ----------------------------------------------------------------------------
// QT Includes
#include <qcanvas.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

namespace reports {

class KReportChartView: public QCanvasView 
{
public:
  QCanvas* m_canvas;
  
  KReportChartView( QWidget* parent, const char* name ): QCanvasView(parent,name) { m_canvas = new QCanvas(parent); setCanvas( m_canvas ); m_canvas->resize(600,600); }
  ~KReportChartView() { if ( m_canvas ) delete m_canvas; }
  static bool implemented(void) { return false; }
};

} // end namespace reports

#endif // KREPORTCHARTVIEW_H
