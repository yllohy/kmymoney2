/***************************************************************************
                          kreportsview.cpp  -  description
                             -------------------
    begin                : Sat Mar 27 2004
    copyright            : (C) 2000-2004 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
                           Ace Jones <ace.j@hotpop.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kdecompat.h"

// ----------------------------------------------------------------------------
// QT Includes
#include <qlayout.h>
#include <qdatetime.h>
#include <qregexp.h>
#include <qdragobject.h>
#include <qclipboard.h>
#include <qapplication.h>
#include <qprinter.h>
#include <qpainter.h>
#include <qfile.h>
#include <qtimer.h>
#include <qiconset.h>
#include <qpopupmenu.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qtooltip.h>

// ----------------------------------------------------------------------------
// KDE Includes
#include <kglobal.h>
#include <kglobalsettings.h>
#include <klocale.h>
#if QT_VERSION > 300
#include <kstandarddirs.h>
#else
#include <kstddirs.h>
#endif

#include <khtmlview.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <klistview.h>

// ----------------------------------------------------------------------------
// Project Includes
#include "kreportsview.h"
#include "../mymoney/mymoneyfile.h"
#include "../mymoney/mymoneyreport.h"
#include "../dialogs/kreportconfigurationfilterdlg.h"
#include "pivottable.h"
#include "querytable.h"
using namespace reports;

#define VIEW_LEDGER         "ledger"
#define VIEW_SCHEDULE       "schedule"
#define VIEW_WELCOME        "welcome"
#define VIEW_HOME           "home"
#define VIEW_REPORTS        "reports"

/**
  * KReportsView::KReportTab Implementation
  */

KReportsView::KReportTab::KReportTab(KTabWidget* parent, const MyMoneyReport& report ):
  QWidget( parent, "reporttab" ),
  m_part( new KHTMLPart( this, "reporttabpart" ) ),
  m_chartView( new KReportChartView( this, "reportchart" ) ),
  m_control( new kMyMoneyReportControlDecl( this, "reportcontrol" ) ),
  m_layout( new QVBoxLayout( this, 11, 6, "reporttablayout" ) ),
  m_report( report ),
  m_deleteMe( false ),
  m_showingChart( false )
{
  if ( ! KReportChartView::implemented() )
  {
    m_control->buttonChart->hide();
  }

  m_chartView->hide();
  m_layout->addWidget( m_control ); //, 0, Qt::AlignTop );
  m_layout->addWidget( m_part->view() );
  m_layout->addWidget( m_chartView );
  QIconSet icons(QPixmap("/usr/share/icons/default.kde/16x16/mimetypes/spreadsheet.png"));

  // I like this icon...
  QString icon = KGlobal::dirs()->findResource("icon", "default.kde/16x16/mimetypes/spreadsheet.png");
  // but if it's not there, we'll use ye ol' standard icon
  if ( icon == QString::null )
    icon = KGlobal::dirs()->findResource("icon", "hicolor/16x16/apps/kmymoney2.png");

  parent->insertTab( this, QIconSet(QPixmap(icon)), report.name() );
  parent->setTabEnabled( this, true );
}

void KReportsView::KReportTab::print(void)
{
  m_part->view()->print();
}

void KReportsView::KReportTab::copyToClipboard(void)
{
  QTextDrag* pdrag =  new QTextDrag( createTable() );
  pdrag->setSubtype("html");
  QApplication::clipboard()->setData(pdrag);
}

void KReportsView::KReportTab::saveAs( const QString& filename )
{
  QFile file( filename );
  if ( file.open( IO_WriteOnly ) )
  {
    if ( QFileInfo(filename).extension(false).lower() == "csv")
    {
      if ( m_report.reportType() == MyMoneyReport::ePivotTable )
        QTextStream(&file) << PivotTable( m_report ).renderCSV();
      else if ( m_report.reportType() == MyMoneyReport::eQueryTable )
        QTextStream(&file) << QueryTable( m_report ).renderCSV();
    }
    else
      QTextStream(&file) <<  createTable();
    file.close();
  }
}

void KReportsView::KReportTab::updateReport(void)
{
  m_part->begin();
  m_part->write(createTable());
  m_part->end();
}

QString KReportsView::KReportTab::createTable(const QString& links)
{
  QString filename = KGlobal::dirs()->findResource("appdata", "html/kmymoney2.css");
  QString header = QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\">\n") +
    QString("<html><head><link rel=\"stylesheet\" type=\"text/css\" href=\"%1\">").arg(filename);
  header += "</head><body>\n";

  QString footer = "</body></html>\n";

  QString html;
  try {
    html += header;
    html += links;

    if ( m_report.reportType() == MyMoneyReport::ePivotTable )
      html += PivotTable( m_report ).renderHTML();
    else if ( m_report.reportType() == MyMoneyReport::eQueryTable )
      html += QueryTable( m_report ).renderHTML();

    html += footer;
  }
  catch(MyMoneyException *e)
  {
    kdDebug(2) << "KReportsView::KReportTab::createTable(): ERROR " << e->what() << endl;

    QString error = QString(i18n("There was an error creating your report: \"%1\".\nPlease report this error to the developer's list: kmymoney2-developer@lists.sourceforge.net")).arg(e->what());

    QMessageBox::critical(this,i18n("Critical Error"), error, QMessageBox::Ok, QMessageBox::NoButton );

    html += header;
    html += links;
    html += "<h1>"+i18n("Unable to generate report")+"</h1><p>"+error+"</p>";
    html += footer;

    delete e;
  }
  return html;

}

void KReportsView::KReportTab::toggleChart(void)
{
  // for now it will just SHOW the chart.  In the future it actually has to toggle it.

  if ( m_showingChart )
  {
    m_part->show();
    m_chartView->hide();

    m_control->buttonChart->setText( i18n( "Chart" ) );
    QToolTip::add( m_control->buttonChart, i18n( "Show the chart version of this report" ) );
  }
  else
  {
    //FIXME: Check the type of report and call the correct report type, don't just assume
    //it's a pivot table...duh!
    PivotTable t( m_report );
    t.drawChart( *m_chartView );
    m_part->hide();
    m_chartView->show();

    m_control->buttonChart->setText( i18n( "Report" ) );
    QToolTip::add( m_control->buttonChart, i18n( "Show the report version of this chart" ) );
  }
  m_showingChart = ! m_showingChart;
}

/**
  * KReportsView Implementation
  */

KReportsView::KReportsView(QWidget *parent, const char *name )
 : QWidget(parent,name)
{
  m_qvboxlayoutPage = new QVBoxLayout(this);
  m_qvboxlayoutPage->setSpacing( 6 );
  m_qvboxlayoutPage->setMargin( 11 );

  m_reportTabWidget = new KTabWidget( this, "m_reportTabWidget" );
  m_qvboxlayoutPage->addWidget( m_reportTabWidget );
#if KDE_IS_VERSION(3,2,0)
  m_reportTabWidget->setHoverCloseButton( true );
#endif

  m_listTab = (new QWidget( m_reportTabWidget, "indextab" ));
  m_listTabLayout = ( new QVBoxLayout( m_listTab, 11, 6, "indextabLayout") );
  m_reportListView = new KListView( m_listTab, "m_reportListView" );
  m_listTabLayout->addWidget( m_reportListView );
  m_reportTabWidget->insertTab( m_listTab, "Reports" );

  m_reportListView->addColumn(i18n("Report"));
  m_reportListView->addColumn(i18n("Comment"));
  m_reportListView->setResizeMode(QListView::AllColumns);
  m_reportListView->setAllColumnsShowFocus(true);
  m_reportListView->setRootIsDecorated(true);

#if KDE_IS_VERSION(3,2,0)
  connect( m_reportTabWidget, SIGNAL(closeRequest(QWidget*)),
    this, SLOT(slotClose(QWidget*)) );
#endif
  connect(m_reportListView, SIGNAL(doubleClicked(QListViewItem*)),
    this, SLOT(slotOpenReport(QListViewItem*)));
  connect(m_reportListView, SIGNAL(returnPressed(QListViewItem*)),
    this, SLOT(slotOpenReport(QListViewItem*)));
  connect( m_reportListView, SIGNAL(contextMenu(KListView*,QListViewItem*,const QPoint &)),
    this, SLOT(slotListContextMenu(KListView*,QListViewItem*,const QPoint &)));
}

KReportsView::~KReportsView()
{
}

void KReportsView::show()
{
  slotRefreshView();
  QWidget::show();
  emit signalViewActivated();
}

void KReportsView::slotRefreshView(void)
{
  //
  // Rebuild the list page
  //
  m_reportListView->clear();
  unsigned pagenumber = 1;

  // Default Reports
  
  QMap<QString,KListViewItem*> groupitems;
  QValueList<ReportGroup> defaultreports;
  defaultReports(defaultreports);
  QValueList<ReportGroup>::const_iterator it_group = defaultreports.begin();
  while ( it_group != defaultreports.end() )
  {
    QString groupname = (*it_group).name();
    QString pagename = QString::number(pagenumber++) + ". " + groupname;
    KListViewItem* curnode = new KListViewItem(m_reportListView,pagename);
    groupitems[groupname] = curnode;
  
    QValueList<MyMoneyReport>::const_iterator it_report = (*it_group).begin();
    while( it_report != (*it_group).end() )
    {
      MyMoneyReport report = *it_report;
      report.setGroup(groupname);
      new KReportListItem( curnode, report );
      ++it_report;
    }
    
    ++it_group;
  }  
  // Custom reports
  
  QString pagename = QString::number(pagenumber++) + ". " + i18n("Favorite Reports");
  KListViewItem* favoritenode = new KListViewItem(m_reportListView,pagename);
  KListViewItem* orphannode = NULL;
  
  QValueList<MyMoneyReport> customreports = MyMoneyFile::instance()->reportList();
  QValueList<MyMoneyReport>::const_iterator it_report = customreports.begin();
  while( it_report != customreports.end() )
  {
    // If this report is in a known group, place it there
    KListViewItem* groupnode = groupitems[(*it_report).group()];
    if ( groupnode )
      new KReportListItem( groupnode, *it_report );
    else
    // otherwise, place it in the orphanage
    { 
      if ( ! orphannode )
      {
        QString pagename = QString::number(pagenumber++) + ". " + i18n("Old Customized Reports");
        orphannode = new KListViewItem(m_reportListView,pagename);
      }
      new KReportListItem( orphannode, *it_report );
    }
    
    // ALSO place it into the Favorites list if it's a favorite
    if ( (*it_report).isFavorite() )
      new KReportListItem( favoritenode, *it_report );
    
    ++it_report;
  }

  //
  // Go through the tabs to update all the reports, or delete them if needed
  //

  int index = 1;
  while ( index < m_reportTabWidget->count() )
  {
    // TODO: Find some way of detecting the file is closed and kill these tabs!!
    KReportTab* tab = dynamic_cast<KReportTab*>(m_reportTabWidget->page(index));
    if ( tab->isReadyToDelete() /* || ! reports.count() */ )
    {
      delete tab;
      --index;
    }
    else
      tab->updateReport();
    ++index;
  }
}

void KReportsView::slotOpenURL(const KURL &url, const KParts::URLArgs& /* args */)
{
  QString view = url.fileName(false);
  QCString id = url.queryItem("id").data();
  QCString command = url.queryItem("command").data();

  if(view == VIEW_REPORTS) {

    if ( command.isEmpty() )
      slotRefreshView();
    else if ( command == "print" )
      slotPrintView();
    else if ( command == "copy" )
      slotCopyView();
    else if ( command == "save" )
      slotSaveView();
    else if ( command == "configure" )
      slotConfigure();
    else if ( command == "duplicate" )
      slotDuplicate();
    else if ( command == "close" )
      slotCloseCurrent();
    else if ( command == "delete" )
      slotDelete();
    else
      qDebug("Unknown command '%s' in KReportsView::slotOpenURL()", static_cast<const char*>(command));

  } else {
    qDebug("Unknown view '%s' in KReportsView::slotOpenURL()", view.latin1());
  }
}

void KReportsView::slotPrintView(void)
{
  KReportTab* tab = dynamic_cast<KReportTab*>(m_reportTabWidget->currentPage());
  tab->print();
}

void KReportsView::slotCopyView(void)
{
  KReportTab* tab = dynamic_cast<KReportTab*>(m_reportTabWidget->currentPage());
  tab->copyToClipboard();
}

void KReportsView::slotSaveView(void)
{
  KReportTab* tab = dynamic_cast<KReportTab*>(m_reportTabWidget->currentPage());
  QString newName=KFileDialog::getSaveFileName(KGlobalSettings::documentPath(),i18n("*.csv|CSV files\n""*.html|HTML files\n""*.*|All files"), this, i18n("Save as..."));

  if(!newName.isEmpty())
  {
    if(newName.findRev('.') == -1)
      newName.append(".html");

    tab->saveAs( newName );
  }
}

void KReportsView::slotConfigure(void)
{
  KReportTab* tab = dynamic_cast<KReportTab*>(m_reportTabWidget->currentPage());

  MyMoneyReport report = tab->report();
  if ( report.comment() == i18n("Default Report") )
  {
    report.setComment( i18n("Custom Report") );
    report.setName( report.name() + i18n(" (Customized)") );
  }

  KReportConfigurationFilterDlg dlg(report);

  if (dlg.exec())
  {
    MyMoneyReport newreport = dlg.getConfig();
    
    // If this report has an ID, then MODIFY it, otherwise ADD it
    if ( ! newreport.id().isEmpty() )
    {
      MyMoneyFile::instance()->modifyReport(newreport);
      tab->modifyReport(newreport);
      slotRefreshView();
  
      m_reportTabWidget->changeTab( tab, newreport.name() );
      m_reportTabWidget->showPage(tab);
    }
    else
    {
      MyMoneyFile::instance()->addReport(newreport);
      new KReportListItem( m_reportListView, newreport );
      addReportTab(newreport);
    }
  }
}

void KReportsView::slotDuplicate(void)
{
  KReportTab* tab = dynamic_cast<KReportTab*>(m_reportTabWidget->currentPage());

  MyMoneyReport dupe = tab->report();
  dupe.setName( QString(i18n("Copy of %1")).arg(dupe.name()) );
  if ( dupe.comment() == i18n("Default Report") )
    dupe.setComment( i18n("Custom Report") );
  dupe.setId(QCString());

  KReportConfigurationFilterDlg dlg(dupe);
  if (dlg.exec())
  {
    dupe = dlg.getConfig();
    MyMoneyFile::instance()->addReport(dupe);
    new KReportListItem( m_reportListView, dupe );
    addReportTab(dupe);
  }
}

void KReportsView::slotDelete(void)
{
  KReportTab* tab = dynamic_cast<KReportTab*>(m_reportTabWidget->currentPage());

  MyMoneyReport report = tab->report();
  if ( ! report.id().isEmpty() )
  {
    if ( QMessageBox::Ok == QMessageBox::warning(this,i18n("Delete Report?"),QString(i18n("Are you sure you want to delete %1?  There is no way to recover it!")).arg(report.name()), QMessageBox::Ok, QMessageBox::Cancel) )
    {
      MyMoneyFile::instance()->removeReport(report);
      slotClose(tab);
      slotRefreshView();
    }
  }
  else
    QMessageBox::warning(this,i18n("Delete Report?"),QString(i18n("Sorry, %1 is a default report.  You may not delete it.")).arg(report.name()), QMessageBox::Ok, 0);
}

void KReportsView::slotOpenReport(QListViewItem* item)
{
  KReportListItem *reportItem = dynamic_cast<KReportListItem*> (item);

  if ( reportItem )
  {
    KReportTab* page = NULL;

    // Find the tab which contains the report indicated by this list item
    int index = 1;
    while ( index < m_reportTabWidget->count() )
    {
      KReportTab* current = dynamic_cast<KReportTab*>(m_reportTabWidget->page(index));
      
      // If this report has an ID, we'll use the ID to match
      if ( ! reportItem->report().id().isEmpty() )
      {
        if ( current->report().id() == reportItem->report().id() )
        {
          page = current;
          break;
        }
      }
      // Otherwise, use the name to match.  THIS ASSUMES that no 2 default reports
      // have the same name...but that would be pretty a boneheaded thing to do.
      else
      {
        if ( current->report().name() == reportItem->report().name() )
        {
          page = current;
          break;
        }
      }
      
      ++index;
    }

    // Show the tab, or create a new one, as needed
    if ( page )
      m_reportTabWidget->showPage( page );
    else
      addReportTab(reportItem->report());
  }
  else
  {
    // this is not a KReportListItem, so it's a regular QListViewItem, which
    // means its a header.
    //
    // double-click on a header means toggle the expand/collapse state
    
    item->setOpen( ! item->isOpen() );
  }
}

void KReportsView::slotToggleChart(void)
{
  KReportTab* tab = dynamic_cast<KReportTab*>(m_reportTabWidget->currentPage());
  tab->toggleChart();
}

void KReportsView::slotCloseCurrent(void)
{
  slotClose(m_reportTabWidget->currentPage());
}

void KReportsView::slotClose(QWidget* w)
{
  KReportTab* tab = dynamic_cast<KReportTab*>(w);
  m_reportTabWidget->removePage(tab);
  tab->setReadyToDelete(true);
}

void KReportsView::addReportTab(const MyMoneyReport& report)
{
  KReportTab* tab = new KReportTab(m_reportTabWidget,report);

  connect( tab->control()->buttonChart, SIGNAL(clicked()),
    this, SLOT(slotToggleChart(void )));
  connect( tab->control()->buttonConfigure, SIGNAL(clicked()),
    this, SLOT(slotConfigure(void )));
  connect( tab->control()->buttonNew, SIGNAL(clicked()),
    this, SLOT(slotDuplicate(void )));
  connect( tab->control()->buttonCopy, SIGNAL(clicked()),
    this, SLOT(slotCopyView(void )));
  connect( tab->control()->buttonExport, SIGNAL(clicked()),
    this, SLOT(slotSaveView(void )));
  connect( tab->control()->buttonDelete, SIGNAL(clicked()),
    this, SLOT(slotDelete(void )));
  connect( tab->control()->buttonClose, SIGNAL(clicked()),
    this, SLOT(slotCloseCurrent(void )));

  // if this is a default report, then you can't delete it!
  if ( report.id().isEmpty() )
    tab->control()->buttonDelete->setEnabled(false);
    
  slotRefreshView();

  m_reportTabWidget->showPage(tab);

};

void KReportsView::slotListContextMenu(KListView* lv,QListViewItem* item,const QPoint & p)
{
  if ( lv == m_reportListView && item )
  {
    QPopupMenu* contextmenu = new QPopupMenu(this);
    contextmenu->insertItem( i18n("&Open"), this, SLOT(slotOpenFromList()) );
    contextmenu->insertItem( i18n("&Configure"), this, SLOT(slotConfigureFromList()) );
    contextmenu->insertItem( i18n("&New report"), this, SLOT(slotNewFromList()) );
    contextmenu->insertItem( i18n("&Delete"), this, SLOT(slotDeleteFromList()) );

    contextmenu->popup(p);
  }
}

void KReportsView::slotOpenFromList(void)
{
  KReportListItem *reportItem = dynamic_cast<KReportListItem*> (m_reportListView->selectedItem());

  if ( reportItem )
    slotOpenReport(reportItem);
}

void KReportsView::slotConfigureFromList(void)
{
  KReportListItem *reportItem = dynamic_cast<KReportListItem*> (m_reportListView->selectedItem());

  if ( reportItem )
  {
    slotOpenReport(reportItem);
    slotConfigure();
  }
}
void KReportsView::slotNewFromList(void)
{
  KReportListItem *reportItem = dynamic_cast<KReportListItem*> (m_reportListView->selectedItem());

  if ( reportItem )
  {
    slotOpenReport(reportItem);
    slotDuplicate();
  }
}

void KReportsView::slotDeleteFromList(void)
{
  KReportListItem *reportItem = dynamic_cast<KReportListItem*> (m_reportListView->selectedItem());

  if ( reportItem )
  {
    slotOpenReport(reportItem);
    slotDelete();
  }
}

void KReportsView::defaultReports(QValueList<ReportGroup>& groups)
{
  {
    ReportGroup list = i18n("Income and Expenses");
  
    list.push_back(MyMoneyReport(
      MyMoneyReport::eExpenseIncome,
      MyMoneyReport::eMonths,
      MyMoneyTransactionFilter::currentMonth,
      true,
      i18n("Income and Expenses This Month"),
      i18n("Default Report")
    ));
    list.push_back(MyMoneyReport(
      MyMoneyReport::eExpenseIncome,
      MyMoneyReport::eMonths,
      MyMoneyTransactionFilter::yearToDate,
      true,
      i18n("Income and Expenses This Year"),
      i18n("Default Report")
    ));
    groups.push_back(list);
  }
  {
    ReportGroup list = i18n("Net Worth");
  
    list.push_back(MyMoneyReport(
      MyMoneyReport::eAssetLiability,
      MyMoneyReport::eMonths,
      MyMoneyTransactionFilter::yearToDate,
      false,
      i18n("Net Worth By Month"),
      i18n("Default Report")
    ));
    list.push_back(MyMoneyReport(
      MyMoneyReport::eAssetLiability,
      MyMoneyReport::eMonths,
      MyMoneyTransactionFilter::currentMonth,
      false,
      i18n("Net Worth Today"),
      i18n("Default Report")
    ));
    list.push_back(MyMoneyReport(
      MyMoneyReport::eAssetLiability,
      MyMoneyReport::eYears,
      MyMoneyTransactionFilter::allDates,
      false,
      i18n("Net Worth By Year"),
      i18n("Default Report")
    ));

    groups.push_back(list);
  }
  {
    ReportGroup list = i18n("Transactions");
        
    list.push_back(MyMoneyReport(
      MyMoneyReport::eAccount,
      MyMoneyReport::eQCnumber|MyMoneyReport::eQCpayee|MyMoneyReport::eQCcategory,
      MyMoneyTransactionFilter::yearToDate,
      false,
      i18n("Transactions by Account"),
      i18n("Default Report")
    ));
    list.push_back(MyMoneyReport(
      MyMoneyReport::eCategory,
      MyMoneyReport::eQCnumber|MyMoneyReport::eQCpayee|MyMoneyReport::eQCaccount,
      MyMoneyTransactionFilter::yearToDate,
      false,
      i18n("Transactions by Category"),
      i18n("Default Report")
    ));
    list.push_back(MyMoneyReport(
      MyMoneyReport::ePayee,
      MyMoneyReport::eQCnumber|MyMoneyReport::eQCcategory,
      MyMoneyTransactionFilter::yearToDate,
      false,
      i18n("Transactions by Payee"),
      i18n("Default Report")
    ));
    list.push_back(MyMoneyReport(
      MyMoneyReport::eMonth,
      MyMoneyReport::eQCnumber|MyMoneyReport::eQCpayee|MyMoneyReport::eQCcategory,
      MyMoneyTransactionFilter::yearToDate,
      false,
      i18n("Transactions by Month"),
      i18n("Default Report")
    ));
    list.push_back(MyMoneyReport(
      MyMoneyReport::eWeek,
      MyMoneyReport::eQCnumber|MyMoneyReport::eQCpayee|MyMoneyReport::eQCcategory,
      MyMoneyTransactionFilter::yearToDate,
      false,
      i18n("Transactions by Week"),
      i18n("Default Report")
    ));
    
    groups.push_back(list);
  }    
  {
    ReportGroup list = i18n("Taxes");
  
    list.push_back(MyMoneyReport(
      MyMoneyReport::eCategory,
      MyMoneyReport::eQCnumber|MyMoneyReport::eQCpayee|MyMoneyReport::eQCaccount,
      MyMoneyTransactionFilter::yearToDate,
      false,
      i18n("Tax Transactions"),
      i18n("Default Report")
    ));
    list.back().setTax(true);
    groups.push_back(list);
  }
}
