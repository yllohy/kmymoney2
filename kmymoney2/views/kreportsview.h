/***************************************************************************
                          kreportsview.h  -  description
                             -------------------
    begin                : Sat Mar 27 2004
    copyright            : (C) 2000-2004 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
                           Ace Jones <ace.jones@hotpop.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef KREPORTSVIEW_H
#define KREPORTSVIEW_H

// ----------------------------------------------------------------------------
// QT Includes
#include <qvaluevector.h>
#include <qwidget.h>
class QVBoxLayout;
class QTabWidget;

// ----------------------------------------------------------------------------
// KDE Includes
#include <khtml_part.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "../mymoney/mymoneyscheduled.h"
#include "../mymoney/mymoneyaccount.h"
#include "pivottable.h"

/**
  * Displays a page where reports can be placed.  For now only contains
  * one simple report, but can easily be extended.
  *
  * @author Ace Jones
  *
  * @short A view for reports.
**/
class KReportsView : public QWidget  {
   Q_OBJECT

private:
  QValueVector<QWidget*> m_tab;
  QValueVector<KHTMLPart*> m_part;
  QValueVector<QVBoxLayout*> m_tabLayout;
  QVBoxLayout *m_qvboxlayoutPage;
  QTabWidget* m_reportTabWidget;
  bool m_boolShowSubAccounts;

  QValueVector<reports::ReportConfiguration> m_reports;
    
public:
  /**
    * Standard constructor.
    *
    * @param parent The QWidget this is used in.
    * @param name The QT name.
    *
    * @return An object of type KReportsView
    *
    * @see ~KReportsView
    */
  KReportsView(QWidget *parent=0, const char *name=0);

  /**
    * Standard destructor.
    *
    * @return Nothing.
    *
    * @see KReportsView
    */
  ~KReportsView();

  /**
    * Overridden so we can emit the activated signal.
    *
    * @return Nothing.
    */
  void show();

protected:
  static const QString linkfull(const QString& view, const QString& query, const QString& label);
  const QString createTable(const reports::ReportConfiguration& report, const QString& links = QString()) const;
  
public slots:
  void slotOpenURL(const KURL &url, const KParts::URLArgs& args);

  void slotRefreshView(void);
  void slotPrintView(void);
  void slotCopyView(void);
  void slotSaveView(void);
  void slotReloadView(void) { slotRefreshView(); };
  void slotConfigure(void);

signals:
  /**
    * This signal is emitted whenever this view is activated.
    */
  void signalViewActivated(void);
};

#endif
