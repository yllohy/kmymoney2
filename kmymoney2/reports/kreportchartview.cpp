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

KReportChartView::KReportChartView( QWidget* parent, const char* name ): KDChartWidget(parent,name)
{
    // ********************************************************************
    // Set KMyMoney's Chart Parameter Defaults
    // ********************************************************************
    this->setPaletteBackgroundColor( Qt::white );

    KDChartParams* _params = new KDChartParams();
    _params->setChartType( KDChartParams::Line );
    _params->setAxisLabelStringParams( KDChartAxisParams::AxisPosBottom,&m_abscissaNames,0);
    _params->setDataSubduedColors();

    /**
    // use line marker, but only circles.
    _params->setLineMarker( true );
    _params->setLineMarkerSize( QSize(8,8) );
    _params->setLineMarkerStyle( 0, KDChartParams::LineMarkerCircle );
    _params->setLineMarkerStyle( 1, KDChartParams::LineMarkerCircle );
    _params->setLineMarkerStyle( 2, KDChartParams::LineMarkerCircle );
    **/

    // initialize parameters
    this->setParams(_params);

    // initialize data
    KDChartTableData* _data = new KDChartTableData();
    this->setData(_data);

    // ********************************************************************
    // Some Examplatory Chart Table Data
    // ********************************************************************

    /**
    // 1st series
    this->data()->setCell( 0, 0,    17.5   );
    this->data()->setCell( 0, 1,   125     );  // highest value
    this->data()->setCell( 0, 2,     6.67  );  // lowest value
    this->data()->setCell( 0, 3,    33.333 );
    this->data()->setCell( 0, 4,    30     );
    // 2nd series
    this->data()->setCell( 1, 0,    40     );
    this->data()->setCell( 1, 1,    40     );
    this->data()->setCell( 1, 2,    45.5   );
    this->data()->setCell( 1, 3,    45     );
    this->data()->setCell( 1, 4,    35     );
    // 3rd series
    this->data()->setCell( 2, 0,    25     );
    // missing value: setCell( 2, 1,   25 );
    this->data()->setCell( 2, 2,    30     );
    this->data()->setCell( 2, 3,    45     );
    this->data()->setCell( 2, 4,    40     );
    **/

    // ********************************************************************
    // Tooltip Setup
    // ********************************************************************
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
    uint dataset;          // the current dataset (eg. category)
    uint datasets;         // the total number of datasets
    double value;          // the value of the region
    double pivot_sum;      // the sum over all categories in the current pivot point

    // the data region in which the cursor was last time
    static uint previous;

    // if mouse tracking is disabled, don't show any tooltip
    if ( !this->hasMouseTracking() )
        return ;

    // find the data region below the current mouse location
    // ..by going through every data region and checking whether it
    //   contains the mouse pointer
    KDChartDataRegion* current = 0;
    QPtrListIterator < KDChartDataRegion > it( *(this->dataRegions()) );
    while ( ( current = it.current() ) ) {
        ++it;
        if ( current->contains( event->pos() ) )
        {
            // we found the data region that contains the mouse
            value = this->data()->cellVal(current->row, current->col).toDouble();

            // get the dataset that the region corresponds to
            if ( this->getAccountSeries() )
            {
              dataset = current->row;
              datasets= this->data()->rows();
              pivot_sum = value * 100.0 / this->data()->colSum(current->col);
            }
            else
            {
              dataset = current->col;
              datasets= this->data()->cols();
              pivot_sum = value * 100.0 / this->data()->rowSum(current->row);
            }

            // if we entered a new data region or the label was invisible
            if ( !label->isVisible() || previous != dataset )
            {
              // if there is more than one dataset, show percentage
              if(datasets > 1)
              {
                // set the tooltip text
                label->setText(QString("<h2>%1</h2><strong>%2</strong><br>(%3\%)")
                    .arg(this->params()->legendText( dataset ))
                    .arg(value, 0, 'f', 2)
                    .arg(pivot_sum, 0, 'f', 2)
                    );
              }
              else // if there is only one dataset, don't show percentage
              {
                // set the tooltip text
                label->setText(QString("<h2>%1</h2><strong>%2</strong>")
                    .arg(this->params()->legendText( dataset ))
                    .arg(value, 0, 'f', 2)
                    );
              }

              previous = dataset;
            }

            translate.setX( -10 - label->width());
            translate.setY( 20);

            // display the label near the cursor
            pos = event->pos() + translate;

            // but don't let the label leave the visible area
            if( pos.x() < 0 )
                pos.setX(0);
            if( pos.y() < 0 )
                pos.setY(0);
            if( pos.x() + label->width() > this->width() )
                pos.setX( this->width() - label->width() );
            if( pos.y() + label->height() > this->height() )
                pos.setY( this->height() - label->height() );

            // now set the label position and show the label
            label->move( pos );
            label->show();

            // In a more abstract class, we would emit a dateMouseMove event:
            //emit this->dataMouseMove( event->pos(), current->row, current->col );

            return ;
        }
    }
    // if the cursor was not found in any data region, hide the label
    label->hide();
}

void KReportChartView::setProperty(int row, int col, int id)
{
#ifdef HAVE_KDCHART_SETPROP
  this->data()->setProp(row, col, id);
#else
  this->data()->cell(row, col).setPropertySet(id);
#endif
}

#endif
