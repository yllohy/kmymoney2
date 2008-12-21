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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

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
#include <qpushbutton.h>
#include <qtooltip.h>
#include <qcheckbox.h>
#include <qvbox.h>
// ----------------------------------------------------------------------------
// KDE Includes

#include <kglobal.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <khtmlview.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <klistview.h>
#include <kmessagebox.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/mymoneyfile.h>
#include <kmymoney/mymoneyreport.h>
#include <kmymoney/kmymoneyglobalsettings.h>

#include "kreportsview.h"
#include "../reports/querytable.h"
#include "../reports/objectinfotable.h"
#include "../dialogs/kreportconfigurationfilterdlg.h"
#include "../kmymoneyutils.h"

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
  m_showingChart( false ),
  m_needReload( true ),
  m_table(0)
{
  m_part->setZoomFactor( KMyMoneyGlobalSettings::fontSizePercentage() );

  if ( ! KReportChartView::implemented() || m_report.reportType() != MyMoneyReport::ePivotTable )
  {
    m_control->buttonChart->hide();
  }

  m_chartView->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
  m_chartView->hide();
  m_layout->addWidget( m_control ); //, 0, Qt::AlignTop );
  m_layout->addWidget( m_part->view() );
  m_layout->addWidget( m_chartView );

  // I like this icon...
  QString icon = KGlobal::dirs()->findResource("icon", "default.kde/16x16/mimetypes/spreadsheet.png");
  // but if it's not there, we'll use ye ol' standard icon
  if ( icon == QString::null )
    icon = KGlobal::dirs()->findResource("icon", "hicolor/16x16/apps/kmymoney2.png");

  parent->insertTab( this, QIconSet(QPixmap(icon)), report.name() );
  parent->setTabEnabled( this, true );

#ifdef HAVE_KDCHART
  if ( m_report.isChartByDefault() )
    toggleChart();
#endif
}

KReportsView::KReportTab::~KReportTab()
{
  delete m_table;
}

void KReportsView::KReportTab::print(void)
{
  if(m_part && m_part->view())
    m_part->view()->print();
}

void KReportsView::KReportTab::copyToClipboard(void)
{
  QTextDrag* pdrag =  new QTextDrag( createTable() );
  pdrag->setSubtype("html");
  QApplication::clipboard()->setData(pdrag);
}

void KReportsView::KReportTab::saveAs( const QString& filename, bool includeCSS )
{
  QFile file( filename );
  if ( file.open( IO_WriteOnly ) )
  {
    if ( QFileInfo(filename).extension(false).lower() == "csv")
    {
      QTextStream(&file) << m_table->renderCSV();
    }
    else {
      QTextStream stream(&file);
      QRegExp exp("(.*)(<link.*css\" href=)\"(.*)\">(<meta.*utf-8\" />)(.*)");
      QString table = createTable();
      if(exp.search(table) != -1 && includeCSS) {
        QFile cssFile(exp.cap(3));
        if(cssFile.open(IO_ReadOnly)) {
          QTextStream cssStream(&cssFile);
          stream << exp.cap(1);
          stream << exp.cap(4);
          stream << endl << "<style type=\"text/css\">" << endl << "<!--" << endl;
          stream << cssStream.read();
          stream << "-->" << endl << "</style>" << endl;
          stream << exp.cap(5);
          cssFile.close();
        } else {
          stream << table;
        }
      } else {
        stream << table;
      }
    }
    file.close();
  }
}

void KReportsView::KReportTab::loadTab(void)
{
  m_needReload = true;
  if(isVisible()) {
    m_needReload = false;
    updateReport();
  }
}

void KReportsView::KReportTab::show(void)
{
  if(m_needReload) {
    m_needReload = false;
    updateReport();
  }
  QWidget::show();
}

void KReportsView::KReportTab::updateReport(void)
{
  // reload the report from the engine. It might have
  // been changed by the user

  try {
    // Don't try to reload default reports from the engine
    if(!m_report.id().isEmpty())
      m_report = MyMoneyFile::instance()->report(m_report.id());
  } catch(MyMoneyException* e) {
    delete e;
  }

  delete m_table;
  m_table = 0;

  if ( m_report.reportType() == MyMoneyReport::ePivotTable ) {
    m_table = new PivotTable(m_report);
  } else if ( m_report.reportType() == MyMoneyReport::eQueryTable ) {
    m_table = new QueryTable(m_report);
  } else if ( m_report.reportType() == MyMoneyReport::eInfoTable ) {
    m_table = new ObjectInfoTable(m_report);
  }

  m_part->begin();
  m_part->write(createTable());
  m_part->end();

  m_table->drawChart( *m_chartView );
  m_chartView->update();
}

QString KReportsView::KReportTab::createTable(const QString& links)
{
  QString filename;
  if(!MyMoneyFile::instance()->value("reportstylesheet").isEmpty())
    filename = KGlobal::dirs()->findResource("appdata", QString("html/%1").arg(MyMoneyFile::instance()->value("reportstylesheet")));
  if(filename.isEmpty())
    filename = KGlobal::dirs()->findResource("appdata", "html/kmymoney2.css");
  QString header = QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\">\n") +
    QString("<html><head><link rel=\"stylesheet\" type=\"text/css\" href=\"%1\">").arg(filename);

  header += "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />";
  header += KMyMoneyUtils::variableCSS();

  header += "</head><body>\n";

  QString footer = "</body></html>\n";

  QString html;
  try {
    html += header;
    html += links;

    html += m_table->renderHTML();

    html += footer;
  }
  catch(MyMoneyException *e)
  {
    kdDebug(2) << "KReportsView::KReportTab::createTable(): ERROR " << e->what() << endl;

    QString error = QString(i18n("There was an error creating your report: \"%1\".\nPlease report this error to the developer's list: kmymoney2-developer@lists.sourceforge.net")).arg(e->what());

    KMessageBox::error(this, error, i18n("Critical Error"));

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

class KReportsViewPrivate
{
public:
  KReportsViewPrivate() :
    includeCSS(0) {}

  QCheckBox* includeCSS;
};

KReportsView::KReportsView(QWidget *parent, const char *name ) :
  KMyMoneyViewBase(parent, name, i18n("Reports")),
  d(new KReportsViewPrivate),
  m_needReload(false)
{
  m_reportTabWidget = new KTabWidget( this, "m_reportTabWidget" );
  addWidget( m_reportTabWidget );
  m_reportTabWidget->setHoverCloseButton( true );

  m_listTab = (new QWidget( m_reportTabWidget, "indextab" ));
  m_listTabLayout = ( new QVBoxLayout( m_listTab, 11, 6, "indextabLayout") );
  m_reportListView = new KListView( m_listTab, "m_reportListView" );
  m_listTabLayout->addWidget( m_reportListView );
  m_reportTabWidget->insertTab( m_listTab, i18n("Reports") );

  m_reportListView->addColumn(i18n("Report"));
  m_reportListView->addColumn(i18n("Comment"));
  m_reportListView->setResizeMode(QListView::AllColumns);
  m_reportListView->setAllColumnsShowFocus(true);
  m_reportListView->setRootIsDecorated(true);
  m_reportListView->setShadeSortColumn(false);

  connect( m_reportTabWidget, SIGNAL(closeRequest(QWidget*)),
    this, SLOT(slotClose(QWidget*)) );
  connect(m_reportListView, SIGNAL(doubleClicked(QListViewItem*)),
    this, SLOT(slotOpenReport(QListViewItem*)));
  connect(m_reportListView, SIGNAL(returnPressed(QListViewItem*)),
    this, SLOT(slotOpenReport(QListViewItem*)));
  connect( m_reportListView, SIGNAL(contextMenu(KListView*,QListViewItem*,const QPoint &)),
    this, SLOT(slotListContextMenu(KListView*,QListViewItem*,const QPoint &)));

  connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotLoadView()));
}

KReportsView::~KReportsView()
{
  delete d;
}

void KReportsView::show()
{
  if(m_needReload) {
    loadView();
    m_needReload = false;
  }

  KReportTab* tab = dynamic_cast<KReportTab*>(m_reportTabWidget->currentPage());
  if(tab)
    emit reportSelected(tab->report());
  else
    emit reportSelected(MyMoneyReport());

  // don't forget base class implementation
  KMyMoneyViewBase::show();
}

void KReportsView::slotLoadView(void)
{
  m_needReload = true;
  if(isVisible()) {
    loadView();
    m_needReload = false;
  }
}

QString KReportsView::KReportGroupListItem::key ( int column, bool ascending ) const
{
  if (column == 0)
    return QString::number(m_nr).rightJustify(3,'0');
  else
    return KListViewItem::key(column,ascending);
}

KReportsView::KReportGroupListItem::KReportGroupListItem(KListView* parent, const int nr, QString name) :
  KListViewItem(parent),
  m_name(name)
{
  setNr(nr);
}

void KReportsView::KReportGroupListItem::setNr(const int nr)
{
  m_nr = nr;
  setText(0, QString("%1. %2").arg(nr).arg(m_name));
}

void KReportsView::loadView(void)
{
  ::timetrace("Start KReportsView::loadView");

  // remember the id of the current selected item and the
  // items that are shown 'expanded'
  QMap<QString, bool> isOpen;
  QListViewItem *item = m_reportListView->selectedItem();
  QString selectedPage = (item) ? item->text(0) : QString();

  // keep a map of all 'expanded' accounts
  QListViewItemIterator it_lvi(m_reportListView);
  while(it_lvi.current()) {
    item = it_lvi.current();
    if(item && item->isOpen()) {
      isOpen[item->text(0)] = true;
    }
    ++it_lvi;
  }

  // remember the upper left corner of the viewport
  QPoint startPoint = m_reportListView->viewportToContents(QPoint(0, 0));

  // turn off updates to avoid flickering during reload
  m_reportListView->setUpdatesEnabled(false);

  //
  // Rebuild the list page
  //
  m_reportListView->clear();
  unsigned pagenumber = 1;

  // Default Reports
  KReportGroupListItem* chartnode = new KReportGroupListItem(m_reportListView, 10, i18n("Charts"));

  QMap<QString,KReportGroupListItem*> groupitems;
  QValueList<ReportGroup> defaultreports;
  defaultReports(defaultreports);
  QValueList<ReportGroup>::const_iterator it_group = defaultreports.begin();
  while ( it_group != defaultreports.end() )
  {
    QString groupname = (*it_group).name();
    KReportGroupListItem* curnode = new KReportGroupListItem(m_reportListView, pagenumber++, (*it_group).title());
    curnode->setOpen(isOpen.find(curnode->text(0)) != isOpen.end());
    groupitems[groupname] = curnode;

    QValueList<MyMoneyReport>::const_iterator it_report = (*it_group).begin();
    while( it_report != (*it_group).end() )
    {
      MyMoneyReport report = *it_report;
      report.setGroup(groupname);
      KReportListItem* r = new KReportListItem( curnode, report );
      if(report.name() == selectedPage)
        m_reportListView->setSelected(r, true);

      // ALSO place it into the Charts list if it's displayed as a chart by default
      if ( (*it_report).isChartByDefault() )
        new KReportListItem( chartnode, *it_report );

      ++it_report;
    }

    ++it_group;
  }

  // Rename the charts item to place it at this point in the list.
  chartnode->setNr(pagenumber++);
  chartnode->setOpen(isOpen.find(chartnode->text(0)) != isOpen.end());

  // Custom reports

  KReportGroupListItem* favoritenode = new KReportGroupListItem(m_reportListView,pagenumber++, i18n("Favorite Reports"));
  favoritenode->setOpen(isOpen.find(favoritenode->text(0)) != isOpen.end());
  KReportGroupListItem* orphannode = NULL;

  QValueList<MyMoneyReport> customreports = MyMoneyFile::instance()->reportList();
  QValueList<MyMoneyReport>::const_iterator it_report = customreports.begin();
  while( it_report != customreports.end() )
  {
    // If this report is in a known group, place it there
    KReportGroupListItem* groupnode = groupitems[(*it_report).group()];
    if ( groupnode )
      new KReportListItem( groupnode, *it_report );
    else
    // otherwise, place it in the orphanage
    {
      if ( ! orphannode )
        orphannode = new KReportGroupListItem(m_reportListView, pagenumber++, i18n("Old Customized Reports"));
      new KReportListItem( orphannode, *it_report );
    }

    // ALSO place it into the Favorites list if it's a favorite
    if ( (*it_report).isFavorite() )
      new KReportListItem( favoritenode, *it_report );

    // ALSO place it into the Charts list if it's displayed as a chart by default
    if ( (*it_report).isChartByDefault() )
      new KReportListItem( chartnode, *it_report );

    ++it_report;
  }

  // reposition viewport
  m_reportListView->setContentsPos(startPoint.x(), startPoint.y());

  // turn updates back on
  m_reportListView->setUpdatesEnabled(true);
  m_reportListView->repaintContents();

  //
  // Go through the tabs to set their update flag or delete them if needed
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
      tab->loadTab();
    ++index;
  }
  ::timetrace("Done KReportsView::loadView");
}

void KReportsView::slotOpenURL(const KURL &url, const KParts::URLArgs& /* args */)
{
  QString view = url.fileName(false);
  QString command = url.queryItem("command").data();

  if(view == VIEW_REPORTS) {

    if ( command.isEmpty() ) {
      // slotRefreshView();
    } else if ( command == "print" )
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
  if(tab)
    tab->print();
}

void KReportsView::slotCopyView(void)
{
  KReportTab* tab = dynamic_cast<KReportTab*>(m_reportTabWidget->currentPage());
  if(tab)
    tab->copyToClipboard();
}

void KReportsView::slotSaveView(void)
{
  KReportTab* tab = dynamic_cast<KReportTab*>(m_reportTabWidget->currentPage());
  if(tab) {
    QVBox* vbox = new QVBox();
    d->includeCSS = new QCheckBox(i18n("Include Stylesheet"), vbox);

    // the following code is copied from KFileDialog::getSaveFileName,
    // adjust to our local needs (filetypes etc.) and
    // enhanced to show the m_saveEncrypted combo box
    KFileDialog dlg( ":kmymoney-export",
                   QString("%1|%2\n").arg("*.csv").arg(i18n("CSV (Filefilter)", "CSV files")) +
                   QString("%1|%2\n").arg("*.html").arg(i18n("HTML (Filefilter)", "HTML files")),
                   this, "filedialog", true, vbox);
    connect(&dlg, SIGNAL(filterChanged(const QString&)), this, SLOT(slotSaveFilterChanged(const QString&)));

    dlg.setOperationMode( KFileDialog::Saving );
    dlg.setCaption(i18n("Export as"));
    slotSaveFilterChanged("*.csv");    // init gui

    if(dlg.exec() == QDialog::Accepted) {
      KURL newURL = dlg.selectedURL();
      if (!newURL.isEmpty()) {
        QString newName = newURL.prettyURL(0, KURL::StripFileProtocol);

        if(newName.findRev('.') == -1)
          newName.append(".html");

        tab->saveAs( newName, d->includeCSS->isEnabled() && d->includeCSS->isChecked() );
      }
    }
  }
}

void KReportsView::slotSaveFilterChanged(const QString& filter)
{
  d->includeCSS->setEnabled(filter ==  "*.html");
}

void KReportsView::slotConfigure(void)
{
  KReportTab* tab = dynamic_cast<KReportTab*>(m_reportTabWidget->currentPage());

  if(tab) {
    MyMoneyReport report = tab->report();
    if ( report.comment() == i18n("Default Report") || report.comment() == i18n("Generated Report") )
    {
      report.setComment( i18n("Custom Report") );
      report.setName( report.name() + i18n(" (Customized)") );
    }

    KReportConfigurationFilterDlg dlg(report);

    if (dlg.exec())
    {
      MyMoneyReport newreport = dlg.getConfig();

      // If this report has an ID, then MODIFY it, otherwise ADD it
      MyMoneyFileTransaction ft;
      if ( ! newreport.id().isEmpty() )
      {
        MyMoneyFile::instance()->modifyReport(newreport);
        ft.commit();
        tab->modifyReport(newreport);

        m_reportTabWidget->changeTab( tab, newreport.name() );
        m_reportTabWidget->showPage(tab);
      }
      else
      {
        MyMoneyFile::instance()->addReport(newreport);
        ft.commit();
        new KReportListItem( m_reportListView, newreport );
        addReportTab(newreport);
      }
    }
  }
}

void KReportsView::slotDuplicate(void)
{
  KReportTab* tab = dynamic_cast<KReportTab*>(m_reportTabWidget->currentPage());

  if(tab) {
    MyMoneyReport dupe = tab->report();
    dupe.setName( QString(i18n("Copy of %1")).arg(dupe.name()) );
    if ( dupe.comment() == i18n("Default Report") )
      dupe.setComment( i18n("Custom Report") );
    dupe.clearId();

    KReportConfigurationFilterDlg dlg(dupe);
    if (dlg.exec())
    {
      dupe = dlg.getConfig();
      MyMoneyFileTransaction ft;
      try {
        MyMoneyFile::instance()->addReport(dupe);
        ft.commit();
        new KReportListItem( m_reportListView, dupe );
        addReportTab(dupe);
      } catch(MyMoneyException* e) {
        qDebug("Cannot add report");
        delete e;
      }
    }
  }
}

void KReportsView::slotDelete(void)
{
  KReportTab* tab = dynamic_cast<KReportTab*>(m_reportTabWidget->currentPage());

  if(tab) {
    MyMoneyReport report = tab->report();
    if ( ! report.id().isEmpty() )
    {
      if ( KMessageBox::Continue == KMessageBox::warningContinueCancel(this, QString("<qt>")+i18n("Are you sure you want to delete report <b>%1</b>?  There is no way to recover it!").arg(report.name())+QString("</qt>"), i18n("Delete Report?")))
      {
        // close the tab and then remove the report so that it is not
        // generated again during the following loadView() call
        slotClose(tab);

        MyMoneyFileTransaction ft;
        MyMoneyFile::instance()->removeReport(report);
        ft.commit();
      }
    }
    else
      KMessageBox::information(this, QString("<qt>")+i18n("Sorry, <b>%1</b> is a default report.  You may not delete it.").arg(report.name())+QString("</qt>"), i18n("Delete Report?"));
  }
}

void KReportsView::slotOpenReport(const QString& id)
{
  if ( ! id.isEmpty() )
  {
    KReportTab* page = NULL;
    int index = 1;
    while ( index < m_reportTabWidget->count() )
    {
      KReportTab* current = dynamic_cast<KReportTab*>(m_reportTabWidget->page(index));

      if ( current->report().id() == id )
      {
        page = current;
        break;
      }

      ++index;
    }

    // Show the tab, or create a new one, as needed
    if ( page )
      m_reportTabWidget->showPage( page );
    else
      addReportTab(MyMoneyFile::instance()->report(id));
  }
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
  else if (item)
  {
    // this is not a KReportListItem, so it's a regular QListViewItem, which
    // means its a header.
    //
    // double-click on a header means toggle the expand/collapse state

    item->setOpen( ! item->isOpen() );
  }
}

void KReportsView::slotOpenReport(const MyMoneyReport& report)
{
  kdDebug(2) << __func__ << " " << report.name() << endl;
    KReportTab* page = NULL;

    // Find the tab which contains the report indicated by this list item
    int index = 1;
    while ( index < m_reportTabWidget->count() )
    {
      KReportTab* current = dynamic_cast<KReportTab*>(m_reportTabWidget->page(index));

        if ( current->report().name() == report.name() )
        {
          page = current;
          break;
        }

      ++index;
    }

    // Show the tab, or create a new one, as needed
    if ( page )
      m_reportTabWidget->showPage( page );
    else
      addReportTab(report);
}

void KReportsView::slotToggleChart(void)
{
  KReportTab* tab = dynamic_cast<KReportTab*>(m_reportTabWidget->currentPage());
  if(tab)
    tab->toggleChart();
}

void KReportsView::slotCloseCurrent(void)
{
  if(m_reportTabWidget->currentPage())
    slotClose(m_reportTabWidget->currentPage());
}

void KReportsView::slotClose(QWidget* w)
{
  KReportTab* tab = dynamic_cast<KReportTab*>(w);
  if(tab) {
    m_reportTabWidget->removePage(tab);
    tab->setReadyToDelete(true);
  }
}

void KReportsView::slotCloseAll(void)
{
  KReportTab* tab = dynamic_cast<KReportTab*>(m_reportTabWidget->page(1));
  while (tab)
  {
    m_reportTabWidget->removePage(tab);
    tab->setReadyToDelete(true);

    tab = dynamic_cast<KReportTab*>(m_reportTabWidget->page(1));
  }
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

  // slotRefreshView();

  m_reportTabWidget->showPage(tab);

}

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
    ReportGroup list("Income and Expenses", i18n("Income and Expenses"));

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
    list.push_back(MyMoneyReport(
      MyMoneyReport::eExpenseIncome,
      MyMoneyReport::eYears,
      MyMoneyTransactionFilter::allDates,
      true,
      i18n("Income and Expenses By Year"),
      i18n("Default Report")
    ));

#ifdef HAVE_KDCHART
    list.push_back(MyMoneyReport(
      MyMoneyReport::eExpenseIncome,
      MyMoneyReport::eMonths,
      MyMoneyTransactionFilter::yearToDate,
      true,
      i18n("Income and Expenses Graph"),
      i18n("Default Report")
    ));
    list.back().setChartByDefault(true);
    list.back().setDetailLevel(MyMoneyReport::eDetailTop);
    list.back().setChartType(MyMoneyReport::eChartLine);
    list.back().setChartDataLabels(false);

    list.push_back(MyMoneyReport(
      MyMoneyReport::eExpenseIncome,
      MyMoneyReport::eMonths,
      MyMoneyTransactionFilter::yearToDate,
      false,
      i18n("Income and Expenses Pie Chart"),
      i18n("Default Report")
    ));
    list.back().setChartByDefault(true);
    list.back().setDetailLevel(MyMoneyReport::eDetailGroup);
    list.back().setChartType(MyMoneyReport::eChartPie);
    list.back().setShowingRowTotals(false);
#endif

    groups.push_back(list);
  }
  {
    ReportGroup list("Net Worth", i18n("Net Worth"));

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
    list.push_back(MyMoneyReport(
      MyMoneyReport::eAssetLiability,
      MyMoneyReport::eMonths,
      MyMoneyTransactionFilter::next7Days,
      false,
      i18n("7-day Cash Flow Forecast"),
      i18n("Default Report")
    ));
    list.back().setIncludingSchedules( true );
    list.back().setColumnsAreDays( true );

#ifdef HAVE_KDCHART
    list.push_back(MyMoneyReport(
      MyMoneyReport::eAssetLiability,
      MyMoneyReport::eMonths,
      MyMoneyTransactionFilter::last12Months,
      false,
      i18n("Net Worth Graph"),
      i18n("Default Report")
    ));
    list.back().setChartByDefault(true);
    list.back().setChartGridLines(false);
    list.back().setDetailLevel(MyMoneyReport::eDetailTotal);
    list.back().setChartType(MyMoneyReport::eChartLine);

    list.push_back(MyMoneyReport(
      MyMoneyReport::eInstitution,
      MyMoneyReport::eQCnone,
      MyMoneyTransactionFilter::yearToDate,
      false,
      i18n("Account Balances by Institution"),
      i18n("Default Report")
    ));
#endif

    list.push_back(MyMoneyReport(
      MyMoneyReport::eAccountType,
      MyMoneyReport::eQCnone,
      MyMoneyTransactionFilter::yearToDate,
      false,
      i18n("Account Balances by Type"),
      i18n("Default Report")
    ));

    groups.push_back(list);
  }
  {
    ReportGroup list("Transactions", i18n("Transactions"));

    list.push_back(MyMoneyReport(
      MyMoneyReport::eAccount,
      MyMoneyReport::eQCnumber|MyMoneyReport::eQCpayee|MyMoneyReport::eQCcategory|MyMoneyReport::eQCbalance,
      MyMoneyTransactionFilter::yearToDate,
      false,
      i18n("Transactions by Account"),
      i18n("Default Report")
    ));
    //list.back().setConvertCurrency(false);
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
    list.push_back(MyMoneyReport(
      MyMoneyReport::eAccount,
      MyMoneyReport::eQCloan,
      MyMoneyTransactionFilter::allDates,
      false,
      i18n("Loan Transactions"),
      i18n("Default Report")
    ));
    list.back().setLoansOnly(true);
    list.push_back(MyMoneyReport(
      MyMoneyReport::eAccountReconcile,
      MyMoneyReport::eQCnumber|MyMoneyReport::eQCpayee|MyMoneyReport::eQCcategory|MyMoneyReport::eQCbalance,
      MyMoneyTransactionFilter::last3Months,
      false,
      i18n("Transactions by Reconciliation Status"),
      i18n("Default Report")
    ));
    groups.push_back(list);
  }
  {
    ReportGroup list("Investments", i18n("Investments"));

    list.push_back(MyMoneyReport(
      MyMoneyReport::eTopAccount,
      MyMoneyReport::eQCaction|MyMoneyReport::eQCshares|MyMoneyReport::eQCprice,
      MyMoneyTransactionFilter::yearToDate,
      false,
      i18n("Investment Transactions"),
      i18n("Default Report")
    ));
    list.back().setInvestmentsOnly(true);

    list.push_back(MyMoneyReport(
      MyMoneyReport::eAccountByTopAccount,
      MyMoneyReport::eQCshares|MyMoneyReport::eQCprice,
      MyMoneyTransactionFilter::yearToDate,
      false,
      i18n("Investment Holdings by Account"),
      i18n("Default Report")
    ));
    list.back().setInvestmentsOnly(true);

    list.push_back(MyMoneyReport(
      MyMoneyReport::eEquityType,
      MyMoneyReport::eQCshares|MyMoneyReport::eQCprice,
      MyMoneyTransactionFilter::yearToDate,
      false,
      i18n("Investment Holdings by Type"),
      i18n("Default Report")
    ));
    list.back().setInvestmentsOnly(true);

    list.push_back(MyMoneyReport(
      MyMoneyReport::eAccountByTopAccount,
      MyMoneyReport::eQCperformance,
      MyMoneyTransactionFilter::yearToDate,
      false,
      i18n("Investment Performance by Account"),
      i18n("Default Report")
    ));
    list.back().setInvestmentsOnly(true);

    list.push_back(MyMoneyReport(
      MyMoneyReport::eEquityType,
      MyMoneyReport::eQCperformance,
      MyMoneyTransactionFilter::yearToDate,
      false,
      i18n("Investment Performance by Type"),
      i18n("Default Report")
    ));
    list.back().setInvestmentsOnly(true);
#ifdef HAVE_KDCHART
    list.push_back(MyMoneyReport(
      MyMoneyReport::eAssetLiability,
      MyMoneyReport::eMonths,
      MyMoneyTransactionFilter::today,
      false,
      i18n("Investment Holdings Pie"),
      i18n("Default Report")
    ));
    list.back().setChartByDefault(true);
    list.back().setChartGridLines(false);
    list.back().setDetailLevel(MyMoneyReport::eDetailAll);
    list.back().setChartType(MyMoneyReport::eChartPie);
    list.back().setInvestmentsOnly(true);

    list.push_back(MyMoneyReport(
      MyMoneyReport::eAssetLiability,
      MyMoneyReport::eMonths,
      MyMoneyTransactionFilter::last12Months,
      false,
      i18n("Investment Worth Graph"),
      i18n("Default Report")
    ));
    list.back().setChartByDefault(true);
    list.back().setChartGridLines(false);
    list.back().setDetailLevel(MyMoneyReport::eDetailAll);
    list.back().setChartType(MyMoneyReport::eChartLine);
    list.back().setColumnsAreDays( true );
    list.back().setInvestmentsOnly(true);

    list.push_back(MyMoneyReport(
      MyMoneyReport::eAssetLiability,
      MyMoneyReport::eMonths,
      MyMoneyTransactionFilter::last30Days,
      false,
      i18n("Investment Moving Average"),
      i18n("Default Report")
    ));
    list.back().setChartGridLines(false);
    list.back().setDetailLevel(MyMoneyReport::eDetailAll);
    list.back().setChartType(MyMoneyReport::eChartLine);
    list.back().setColumnsAreDays( true );
    list.back().setInvestmentsOnly(true);
    list.back().setIncludingMovingAverage(true);
    list.back().setMovingAverageDays(10);

#endif
    groups.push_back(list);
  }
  {
    ReportGroup list("Taxes", i18n("Taxes"));

    list.push_back(MyMoneyReport(
      MyMoneyReport::eCategory,
      MyMoneyReport::eQCnumber|MyMoneyReport::eQCpayee|MyMoneyReport::eQCaccount,
      MyMoneyTransactionFilter::yearToDate,
      false,
      i18n("Tax Transactions by Category"),
      i18n("Default Report")
    ));
    list.back().setTax(true);
    list.push_back(MyMoneyReport(
      MyMoneyReport::ePayee,
      MyMoneyReport::eQCnumber|MyMoneyReport::eQCcategory|MyMoneyReport::eQCaccount,
      MyMoneyTransactionFilter::yearToDate,
      false,
      i18n("Tax Transactions by Payee"),
      i18n("Default Report")
    ));
    list.back().setTax(true);
    list.push_back(MyMoneyReport(
      MyMoneyReport::eCategory,
      MyMoneyReport::eQCnumber|MyMoneyReport::eQCpayee|MyMoneyReport::eQCaccount,
      MyMoneyTransactionFilter::lastFiscalYear,
      false,
      i18n("Tax Transactions by Category Last Fiscal Year"),
      i18n("Default Report")
    ));
    list.back().setTax(true);
    list.push_back(MyMoneyReport(
      MyMoneyReport::ePayee,
      MyMoneyReport::eQCnumber|MyMoneyReport::eQCcategory|MyMoneyReport::eQCaccount,
      MyMoneyTransactionFilter::lastFiscalYear,
      false,
      i18n("Tax Transactions by Payee Last Fiscal Year"),
      i18n("Default Report")
    ));
    list.back().setTax(true);
    groups.push_back(list);
  }
  {
    ReportGroup list("Budgeting", i18n("Budgeting"));

    list.push_back(MyMoneyReport(
      MyMoneyReport::eBudgetActual,
      MyMoneyReport::eMonths,
      MyMoneyTransactionFilter::yearToDate,
      true,
      i18n("Budgeted vs. Actual This Year"),
      i18n("Default Report")
    ));
    list.back().setShowingRowTotals(true);
    list.back().setBudget("Any",true);

    list.push_back(MyMoneyReport(
      MyMoneyReport::eBudgetActual,
      MyMoneyReport::eMonths,
      MyMoneyTransactionFilter::currentMonth,
      true,
      i18n("Monthly Budgeted vs. Actual"),
      i18n("Default Report")
    ));
    list.back().setBudget("Any",true);

    list.push_back(MyMoneyReport(
      MyMoneyReport::eBudgetActual,
      MyMoneyReport::eMonths,
      MyMoneyTransactionFilter::currentYear,
      true,
      i18n("Yearly Budgeted vs. Actual"),
      i18n("Default Report")
    ));
    list.back().setBudget("Any",true);
    list.back().setShowingRowTotals(true);

    list.push_back(MyMoneyReport(
      MyMoneyReport::eBudget,
      MyMoneyReport::eMonths,
      MyMoneyTransactionFilter::currentMonth,
      true,
      i18n("Monthly Budget"),
      i18n("Default Report")
    ));
    list.back().setBudget("Any",false);

    list.push_back(MyMoneyReport(
      MyMoneyReport::eBudget,
      MyMoneyReport::eMonths,
      MyMoneyTransactionFilter::currentYear,
      true,
      i18n("Yearly Budget"),
      i18n("Default Report")
    ));
    list.back().setBudget("Any",false);
    list.back().setShowingRowTotals(true);
#ifdef HAVE_KDCHART
    list.push_back(MyMoneyReport(
      MyMoneyReport::eBudgetActual,
      MyMoneyReport::eMonths,
      MyMoneyTransactionFilter::currentYear,
      true,
      i18n("Yearly Budgeted vs Actual Graph"),
      i18n("Default Report")
    ));
    list.back().setChartByDefault(true);
    list.back().setChartGridLines(false);
    list.back().setBudget("Any",true);
    list.back().setDetailLevel(MyMoneyReport::eDetailGroup);
    list.back().setChartType(MyMoneyReport::eChartLine);
#endif

    groups.push_back(list);
  }
  {
    ReportGroup list("Forecast", i18n("Forecast"));

    list.push_back(MyMoneyReport(
      MyMoneyReport::eAssetLiability,
      MyMoneyReport::eMonths,
      MyMoneyTransactionFilter::next12Months,
      false,
      i18n("Forecast By Month"),
      i18n("Default Report")
    ));
    list.back().setIncludingForecast( true );
    list.push_back(MyMoneyReport(
      MyMoneyReport::eAssetLiability,
      MyMoneyReport::eMonths,
      MyMoneyTransactionFilter::nextQuarter,
      false,
      i18n("Forecast Next Quarter"),
      i18n("Default Report")
    ));
    list.back().setColumnsAreDays( true );
    list.back().setIncludingForecast( true );

#ifdef HAVE_KDCHART
    list.push_back(MyMoneyReport(
      MyMoneyReport::eAssetLiability,
      MyMoneyReport::eMonths,
      MyMoneyTransactionFilter::next3Months,
      false,
      i18n("Net Worth Forecast Graph"),
      i18n("Default Report")
    ));
    list.back().setColumnsAreDays( true );
    list.back().setIncludingForecast( true );
    list.back().setChartByDefault(true);
    list.back().setChartGridLines(false);
    list.back().setDetailLevel(MyMoneyReport::eDetailTotal);
    list.back().setChartType(MyMoneyReport::eChartLine);
#endif
    groups.push_back(list);
  }
  {
    ReportGroup list("Information", i18n("General Information"));

    list.push_back(MyMoneyReport(
      MyMoneyReport::eSchedule,
      MyMoneyReport::eMonths,
      MyMoneyTransactionFilter::next12Months,
      false,
      i18n("Schedule Information"),
      i18n("Default Report")
    ));
    list.back().setDetailLevel(MyMoneyReport::eDetailAll);
    list.push_back(MyMoneyReport(
      MyMoneyReport::eSchedule,
      MyMoneyReport::eMonths,
      MyMoneyTransactionFilter::next12Months,
      false,
      i18n("Schedule Summary Information"),
      i18n("Default Report")
    ));
    list.back().setDetailLevel(MyMoneyReport::eDetailTop);
    list.push_back(MyMoneyReport(
      MyMoneyReport::eAccountInfo,
      MyMoneyReport::eMonths,
      MyMoneyTransactionFilter::today,
      false,
      i18n("Account Information"),
      i18n("Default Report")
    ));
    list.back().setConvertCurrency(false);
    list.push_back(MyMoneyReport(
      MyMoneyReport::eAccountLoanInfo,
      MyMoneyReport::eMonths,
      MyMoneyTransactionFilter::today,
      false,
      i18n("Loan Information"),
      i18n("Default Report")
    ));
    list.back().setConvertCurrency(false);
    groups.push_back(list);
  }
}

// Make sure, that these definitions are only used within this file
// this does not seem to be necessary, but when building RPMs the
// build option 'final' is used and all CPP files are concatenated.
// So it could well be, that in another CPP file these definitions
// are also used.
#undef VIEW_LEDGER
#undef VIEW_SCHEDULE
#undef VIEW_WELCOME
#undef VIEW_HOME
#undef VIEW_REPORTS

#include "kreportsview.moc"
// vim:cin:si:ai:et:ts=2:sw=2:
