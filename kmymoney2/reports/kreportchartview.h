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

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif
#ifdef HAVE_KDCHART

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes
// Some STL headers in GCC4.3 contain operator new. Memory checker mangles these
#ifdef _CHECK_MEMORY
  #undef new
#endif

#include <KDChartWidget.h>
#include <KDChartTable.h>
#include <KDChartParams.h>
#include <KDChartAxisParams.h>

// ----------------------------------------------------------------------------
// Project Includes
#ifdef _CHECK_MEMORY
  #include <kmymoney/mymoneyutils.h>
#endif

namespace reports {

class KReportChartView: public KDChartWidget
{
public:
  KReportChartView( QWidget* parent, const char* name );
  ~KReportChartView() {}
  static bool implemented(void) { return true; }
  void setNewData( const KDChartTableData& newdata ) { m_data = newdata; }
  KDChartParams& params(void) { return m_params; }
  QStringList& abscissaNames(void) { return m_abscissaNames; }
  void refreshLabels(void) { m_params.setAxisLabelStringParams( KDChartAxisParams::AxisPosBottom,&m_abscissaNames,0); }
  void setProperty(int row, int col, int id);
//   void setCircularLabels(void) { m_params.setAxisLabelStringParams( KDChartAxisParams::AxisPosCircular,&m_abscissaNames,0); }

private:
  KDChartParams m_params;
  KDChartTableData m_data;
  QStringList m_abscissaNames;
};

} // end namespace reports

#else

namespace reports {

class KReportChartView : public QWidget
{
public:
  KReportChartView( QWidget* parent, const char* name ): QWidget(parent,name) {}
  ~KReportChartView() {}
  static bool implemented(void) { return false; }
};

} // end namespace reports

#endif

#endif // KREPORTCHARTVIEW_H
