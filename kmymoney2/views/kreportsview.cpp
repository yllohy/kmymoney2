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
#include <qtabwidget.h>
#include <qtimer.h>
#include <qiconset.h>

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
#include <ktabwidget.h>

// ----------------------------------------------------------------------------
// Project Includes
#include "kreportsview.h"
#include "../mymoney/mymoneyfile.h"
#include "../mymoney/mymoneyreport.h"
#include "../dialogs/kreportconfigurationfilterdlg.h"
#include "pivottable.h"
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
  m_layout( new QVBoxLayout( this, 11, 6, "reporttablayout" ) ),
  m_reportId( report.id() ),
  m_deleteMe( false )
{
  m_layout->addWidget( m_part->view() );
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
  KHTMLPart part(this, "htmlpart_km2");

  part.begin();
  part.write(createTable());
  part.end();

  part.view()->print();
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
  if ( file.open( IO_WriteOnly ) ) {
    QTextStream(&file) <<  createTable();
    file.close();
  }
}

void KReportsView::KReportTab::updateReport(void)
{
  QString links;

  links += QString("<a href=\"/%1?command=configure\">%2</a>").arg(VIEW_REPORTS).arg(i18n("Configure This Report"));
  links += "&nbsp;|&nbsp;";
  links += QString("<a href=\"/%1?command=duplicate\">%2</a>").arg(VIEW_REPORTS).arg(i18n("Create New Report"));
  links += "&nbsp;|&nbsp;";
  links += QString("<a href=\"/%1?command=copy\">%2</a>").arg(VIEW_REPORTS).arg(i18n("Copy to Clipboard"));
  links += "&nbsp;|&nbsp;";
  links += QString("<a href=\"/%1?command=save\">%2</a>").arg(VIEW_REPORTS).arg(i18n("Save to File"));
  links += "&nbsp;|&nbsp;";
  links += QString("<a href=\"/%1?command=close\">%2</a>").arg(VIEW_REPORTS).arg(i18n("Close"));
    
  m_part->begin();
  m_part->write(createTable(links));
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
    html += PivotTable( MyMoneyFile::instance()->report(m_reportId) ).renderHTML();
    html += footer;
  }
  catch(MyMoneyException *e) 
  {
    kdDebug(2) << "KReportsView::KReportTab::createTable(): ERROR " << e->what() << endl;
    delete e;
  }
  return html;

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
  m_reportTabWidget->setHoverCloseButton( true );

  m_listTab = (new QWidget( m_reportTabWidget, "indextab" ));
  m_listTabLayout = ( new QVBoxLayout( m_listTab, 11, 6, "indextabLayout") );
  m_reportListView = new KListView( m_listTab, "m_reportListView" );
  m_listTabLayout->addWidget( m_reportListView );
  m_reportTabWidget->insertTab( m_listTab, "Reports" );
  
  m_reportListView->addColumn(i18n("Report"));
  m_reportListView->setResizeMode(QListView::AllColumns);

  connect( m_reportTabWidget, SIGNAL(closeRequest(QWidget*)), 
    this, SLOT(slotClose(QWidget*)) );
  connect(m_reportListView, SIGNAL(doubleClicked(QListViewItem*)),
    this, SLOT(slotReportDoubleClicked(QListViewItem*)));
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
  QValueList<MyMoneyReport> reports = MyMoneyFile::instance()->reportList();

  //
  // Rebuild the list page
  //
  
  m_reportListView->clear();
  QValueList<MyMoneyReport>::const_iterator it_report = reports.begin();
  while( it_report != reports.end() )
  {
    new KReportListItem( m_reportListView, *it_report );
    ++it_report;
  }

  //
  // Update all the reports, or delete them if needed
  //
  
  int index = 1;
  while ( index < m_reportTabWidget->count() )
  {
    KReportTab* tab = dynamic_cast<KReportTab*>(m_reportTabWidget->page(index));
    if ( tab->isReadyToDelete() || ! reports.count() )
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
  QString newName=KFileDialog::getSaveFileName(KGlobalSettings::documentPath(),i18n("*.html|HTML files\n""*.*|All files"), this, i18n("Save as..."));

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
  KReportConfigurationFilterDlg dlg(MyMoneyFile::instance()->report(tab->id()));

  if (dlg.exec())
  {
    MyMoneyReport report = dlg.getConfig();
    MyMoneyFile::instance()->modifyReport(report);
    slotRefreshView();
    
    m_reportTabWidget->changeTab( tab, report.name() );
    m_reportTabWidget->showPage(tab);
  }
}

void KReportsView::slotDuplicate(void)
{
  KReportTab* tab = dynamic_cast<KReportTab*>(m_reportTabWidget->currentPage());

  MyMoneyReport dupe = MyMoneyFile::instance()->report(tab->id());
  dupe.setName( QString("Copy of %1").arg(dupe.name()) );
  dupe.setId(QCString());
  MyMoneyFile::instance()->addReport(dupe);
  
  KReportConfigurationFilterDlg dlg(dupe);
  if (dlg.exec())
    MyMoneyFile::instance()->modifyReport(dlg.getConfig());
  
  new KReportListItem( m_reportListView, dupe );
  addReportTab(dupe);
}

void KReportsView::slotReportDoubleClicked(QListViewItem* item)
{  
  KReportListItem *reportItem = dynamic_cast<KReportListItem*> (item);
  KReportTab* page = NULL;

  // Find the tab which contains the report indicated by this list item  
  int index = 1;
  while ( index < m_reportTabWidget->count() )
  {
    KReportTab* current = dynamic_cast<KReportTab*>(m_reportTabWidget->page(index));
    if ( current->id() == reportItem->id() )
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
    addReportTab(MyMoneyFile::instance()->report(reportItem->id()));
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
  connect( tab->browserExtension(), SIGNAL(openURLRequest(const KURL&, const KParts::URLArgs&)),
          this, SLOT(slotOpenURL(const KURL&, const KParts::URLArgs&)) );
        
  slotRefreshView();
  
  m_reportTabWidget->showPage(tab);
  
};

