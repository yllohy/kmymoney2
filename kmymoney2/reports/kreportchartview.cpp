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
#include <KDChartDataRegion.h>

using namespace reports;

KReportChartView::KReportChartView( QWidget* parent, const char* name ): KDChartWidget(parent,name), m_data(3,5)
{
    // ********************************************************************
    // Chart Params
    // ********************************************************************
    m_params.setChartType( KDChartParams::Line );
    m_params.setLineWidth( 2 );
    m_params.setAxisLabelStringParams( KDChartAxisParams::AxisPosBottom,&m_abscissaNames,0);
    m_params.setDataSubduedColors();

    // use line marker, but only circles.
    m_params.setLineMarker( true );
    m_params.setLineMarkerSize( QSize(8,8) );
    m_params.setLineMarkerStyle( 0, KDChartParams::LineMarkerCircle );
    m_params.setLineMarkerStyle( 1, KDChartParams::LineMarkerCircle );
    m_params.setLineMarkerStyle( 2, KDChartParams::LineMarkerCircle );

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

    label = new QLabel( this );
    label->hide();
    // mouse tracking on will force the mouseMoveEvent() method to be called from Qt
    label->setMouseTracking( true );
    label->setFrameStyle( QFrame::PopupPanel | QFrame::Raised );
    label->setAlignment( AlignRight );
    label->setAutoResize( true );
}

/**
 * This function implements mouseMoveEvents
  */
void KReportChartView::mouseMoveEvent( QMouseEvent* event )
{
    QPoint translate, pos; // some movement helpers
    uint current_category; // the current row (or column) (e.g. category)
    double value;          // the value of the region
    double pivot_sum;      // the sum over all categories in the current pivot point

    if ( !this->hasMouseTracking() )
       return ;

    // find the data region under the current mouse location
    KDChartDataRegion* current = 0;
    QPtrListIterator < KDChartDataRegion > it( *(this->dataRegions()) );
    while ( ( current = it.current() ) ) {
        ++it;
        if ( current->contains( event->pos() ) )
        {
            // we found the data region
            value = this->data()->cellVal(current->row, current->col).toDouble();
            if ( this->getAccountSeries() )
            {
              current_category = current->row;
              pivot_sum = value * 100.0 / this->data()->colSum(current->col);
            }
            else
            {
              current_category = current->col;
              pivot_sum = value * 100.0 / this->data()->rowSum(current->row);
            }

            // now draw the tooltip
            label->setText(QString("<h2>%1</h2><strong>%2</strong><br>(%3\%)")
                .arg(this->params().legendText( current_category ))
                .arg(value, 0, 'f', 2)
                .arg(pivot_sum, 0, 'f', 2)
                );

            translate.setX( -10 - label->width());
            translate.setY( 20);

            // display the label near the cursor
            pos = event->pos() + translate;

            // but don't let the label move outside the visible area
            if( pos.x() < 0 )
                pos.setX(0);
            if( pos.y() < 0 )
                pos.setY(0);
            if( pos.x() + label->width() > this->width() )
                pos.setX( this->width() - label->width() );
            if( pos.y() + label->height() > this->height() )
                pos.setY( this->height() - label->height() );

            // now move the label
            label->move( pos );
            if ( !label->isVisible() )
                label->show();

            //
            // In a more abstract class, we would emit a dateMouseMove event:
            //emit this->dataMouseMove( event->pos(), current->row, current->col );

            return ;
        }
    }
    // mouse cursor not found in any data region
    label->hide();
}

void KReportChartView::setProperty(int row, int col, int id)
{
#ifdef HAVE_KDCHART_SETPROP
  m_data.setProp(row, col, id);
#else
  m_data.cell(row, col).setPropertySet(id);
#endif
}

#endif
