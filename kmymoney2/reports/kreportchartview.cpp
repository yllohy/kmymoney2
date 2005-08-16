/***************************************************************************
                          kreportchartview.cpp
                             -------------------
    begin                : Sun Aug 14 2005
    copyright            : (C) 2004-2005 by Ace Jones
    email                : <ace.j@hotpop.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif
#ifdef HAVE_KDCHART

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kreportchartview.h"

using namespace reports;

KReportChartView::KReportChartView( QWidget* parent, const char* name ): KDChartWidget(parent,name), m_data(3,5)
{
    // ********************************************************************
    // Chart Params
    // ********************************************************************
    m_params.setChartType( KDChartParams::Line );
    m_params.setLineMarker( true );
    
    // ********************************************************************
    // set Chart Table Data
    // ********************************************************************
    // 1st series
    m_data.setCell( 0, 0,    17.5   );
    m_data.setCell( 0, 1,   125     );  // highest value
    m_data.setCell( 0, 2,     6.67  );  // lowest value
    m_data.setCell( 0, 3,    33.333 );
    m_data.setCell( 0, 4,    30     );
    // 2nd series
    m_data.setCell( 1, 0,    40     );
    m_data.setCell( 1, 1,    40     );
    m_data.setCell( 1, 2,    45.5   );
    m_data.setCell( 1, 3,    45     );
    m_data.setCell( 1, 4,    35     );
    // 3rd series
    m_data.setCell( 2, 0,    25     );
    // missing value: d.setCell( 2, 1,   25 );
    m_data.setCell( 2, 2,    30     );
    m_data.setCell( 2, 3,    45     );
    m_data.setCell( 2, 4,    40     );

    setPaletteBackgroundColor( Qt::white );
    setData(&m_data);
    setParams(&m_params);
}

#endif
