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

#include "kdecompat.h"

// ----------------------------------------------------------------------------
// QT Includes
#include <qvaluevector.h>
#include <qwidget.h>
class QVBoxLayout;
class QListViewItem;
class kMyMoneyTitleLabel;

// ----------------------------------------------------------------------------
// KDE Includes
#include <khtml_part.h>
#include <klistview.h>

#if KDE_IS_VERSION(3,2,0)
  #include <ktabwidget.h>
#else
  #include <qtabwidget.h>
  // In the case of KDE < 3.2, we only have the basic QTabWidget.
  #define KTabWidget QTabWidget
#endif

// ----------------------------------------------------------------------------
// Project Includes

#include "../mymoney/mymoneyscheduled.h"
#include "../mymoney/mymoneyaccount.h"
#include "../widgets/kmymoneyreportcontroldecl.h"
class MyMoneyReport;

#include "pivottable.h"

/**
  * Displays a page where reports can be placed.
  *
  * @author Ace Jones
  *
  * @short A view for reports.
**/
class KReportsView : public QWidget
{
  Q_OBJECT

public:
  /**
    * Helper class for KReportView.
    *
    * Associates a report id with a list view item.
    *
    * @author Ace Jones
    */

  class KReportListItem: public KListViewItem
  {
  private:
    QCString m_id;
    MyMoneyReport m_report;

  public:
    KReportListItem( KListView* parent, const MyMoneyReport& report ):
      KListViewItem( parent, report.name(), report.comment() ),
      m_id( report.id() ),
      m_report( report )
    {}
    KReportListItem( KListViewItem* parent, const MyMoneyReport& report ):
      KListViewItem( parent, report.name(), report.comment() ),
      m_id( report.id() ),
      m_report( report )
    {}
    //const QCString& id(void) const { return m_id; }
    const MyMoneyReport& report(void) const { return m_report; }
  };

  /**
    * Helper class for KReportView.
    *
    * This is the widget which displays a single report in the TabWidget that comprises this view.
    *
    * @author Ace Jones
    */

  class KReportTab: public QWidget
  {
  private:
    KHTMLPart* m_part;
    reports::KReportChartView* m_chartView;
    kMyMoneyReportControlDecl* m_control;
    QVBoxLayout* m_layout;
    MyMoneyReport m_report;
    bool m_deleteMe;
    bool m_showingChart;

  public:
    KReportTab(KTabWidget* parent, const MyMoneyReport& report );
    const MyMoneyReport& report(void) const { return m_report; }
    void print(void);
    void toggleChart(void);
    void copyToClipboard(void);
    void saveAs( const QString& filename );
    void updateReport(void);
    QString createTable(const QString& links=QString());
    const kMyMoneyReportControlDecl* control(void) const { return m_control; }
    bool isReadyToDelete(void) const { return m_deleteMe; }
    void setReadyToDelete(bool f) { m_deleteMe = f; }
    void modifyReport( const MyMoneyReport& report ) { m_report = report; }
  };

  /**
    * Helper class for KReportView.
    *
    * This is a named list of reports, which will be one section 
    * in the list of default reports
    *
    * @author Ace Jones
    */
  class ReportGroup: public QValueList<MyMoneyReport>
  {
  private:
    QString m_name;
  public:
    ReportGroup( void ) {}
    ReportGroup( const QString& name ): m_name( name ) {};
    const QString& name( void ) const { return m_name; };
  };
  
private:
  QVBoxLayout *m_qvboxlayoutPage;
  KTabWidget* m_reportTabWidget;
  KListView* m_reportListView;
  QWidget* m_listTab;
  QVBoxLayout* m_listTabLayout;
  kMyMoneyTitleLabel* titleLabel;
  QFrame* titleLine;

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
  void addReportTab(const MyMoneyReport&);
  static void defaultReports(QValueList<ReportGroup>&);

public slots:
  void slotOpenURL(const KURL &url, const KParts::URLArgs& args);

  void slotRefreshView(void);
  void slotPrintView(void);
  void slotCopyView(void);
  void slotSaveView(void);
  void slotReloadView(void) { slotRefreshView(); };
  void slotConfigure(void);
  void slotDuplicate(void);
  void slotToggleChart(void);
  void slotOpenReport(QListViewItem*);
  void slotOpenReport(const QCString&);
  void slotCloseCurrent(void);
  void slotClose(QWidget*);
  void slotCloseAll(void);
  void slotDelete(void);
  void slotListContextMenu(KListView*,QListViewItem*,const QPoint &);
  void slotOpenFromList(void);
  void slotConfigureFromList(void);
  void slotNewFromList(void);
  void slotDeleteFromList(void);

signals:
  /**
    * This signal is emitted whenever this view is activated.
    */
  void signalViewActivated(void);
};

#endif
