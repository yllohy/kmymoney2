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

KReportsView::KReportsView(QWidget *parent, const char *name )
 : QWidget(parent,name)
{
  m_qvboxlayoutPage = new QVBoxLayout(this);
  m_qvboxlayoutPage->setSpacing( 6 );
  m_qvboxlayoutPage->setMargin( 11 );

  m_reportTabWidget = new QTabWidget( this, "reportTabWidget" );
  m_qvboxlayoutPage->addWidget( m_reportTabWidget );
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

const QString KReportsView::createTable(int page, const QString& links) const
{
  DEBUG_ENTER("KReportsView::createTable()");
  
  QString filename = KGlobal::dirs()->findResource("appdata", "html/kmymoney2.css");
  QString header = QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\">\n") +
    QString("<html><head><link rel=\"stylesheet\" type=\"text/css\" href=\"%1\">").arg(filename);
  header += "</head><body>\n";

  QString footer = "</body></html>\n";

  QString html;
  html += header;
  html += links;
  html += PivotTable( MyMoneyFile::instance()->report(m_reportid[page]) ).renderHTML();
  html += footer;

  return html;
}

void KReportsView::slotRefreshView(void)
{
  QValueList<MyMoneyReport> reports = MyMoneyFile::instance()->reportList();
  
  // Delete unnecessary tabs
  while ( reports.count() < m_tab.count() )
  {
    delete m_tab.back();
    m_tab.pop_back();
    m_tabLayout.pop_back();
    m_part.pop_back();
  }
  
  // Add additional tabs as needed
  while ( m_tab.count() < reports.count() )
  {
    m_tab.push_back(new QWidget( m_reportTabWidget, "tab" ));
    m_tabLayout.push_back( new QVBoxLayout( m_tab.back(), 11, 6, "tabLayout") );
    m_part.push_back(new KHTMLPart( m_tab.back(), "htmlpart_km2") );
    m_tabLayout.back()->addWidget( m_part.back()->view() );
    m_reportTabWidget->insertTab( m_tab.back(), "" );
    m_reportTabWidget->setTabEnabled( m_tab.back(),true );

    connect( m_part.back()->browserExtension(), SIGNAL(openURLRequest(const KURL&, const KParts::URLArgs&)),
            this, SLOT(slotOpenURL(const KURL&, const KParts::URLArgs&)) );
  }

  // Set the name and ID's of each tab
  m_reportid.clear();
  QValueList<MyMoneyReport>::const_iterator it_report = reports.begin();
  QValueVector<QWidget*>::iterator it_tab = m_tab.begin();
  while( it_report != reports.end() )
  {
    m_reportid.push_back((*it_report).id());
    m_reportTabWidget->changeTab( *it_tab, (*it_report).name() );
    
    ++it_report;
    ++it_tab;
  }
  QString links;

  links += linkfull(VIEW_REPORTS, QString("?command=configure&target=1"),i18n("Configure This Report"));
  links += "&nbsp;|&nbsp;";
  links += linkfull(VIEW_REPORTS, QString("?command=duplicate&target=1"),i18n("Create New Report"));
  links += "&nbsp;|&nbsp;";
  links += linkfull(VIEW_REPORTS, QString("?command=copy&target=1"),i18n("Copy to Clipboard"));
  links += "&nbsp;|&nbsp;";
  links += linkfull(VIEW_REPORTS, QString("?command=save&target=1"),i18n("Save to File"));
  
  // run each report and assign the contents into the html part
  unsigned index = 0;
  while ( index < reports.count() )
  {
    m_part[index]->begin();
    m_part[index]->write(createTable(index,links));
    m_part[index]->end();
    ++index;
  }
}

void KReportsView::slotPrintView(void)
{
  int page = m_reportTabWidget->currentPageIndex();

  KHTMLPart part(this, "htmlpart_km2");

  part.begin();
  part.write(createTable(page));
  part.end();

  part.view()->print();
}

void KReportsView::slotCopyView(void)
{
  int page = m_reportTabWidget->currentPageIndex();

  QTextDrag* pdrag =  new QTextDrag( createTable(page) );
  pdrag->setSubtype("html");
  QApplication::clipboard()->setData(pdrag);
}

void KReportsView::slotSaveView(void)
{
  int page = m_reportTabWidget->currentPageIndex();

  QString newName=KFileDialog::getSaveFileName(KGlobalSettings::documentPath(),
                                               i18n("*.html|HTML files\n""*.*|All files"), this, i18n("Save as..."));

  if(!newName.isEmpty())
  {
    if(newName.findRev('.') == -1)
      newName.append(".html");

    QFile file( newName );
    if ( file.open( IO_WriteOnly ) ) {
      QTextStream stream( &file );
      stream <<  createTable(page);
      file.close();
    }
  }
}

void KReportsView::slotConfigure(void)
{
  int page = m_reportTabWidget->currentPageIndex();

  KReportConfigurationFilterDlg dlg(MyMoneyFile::instance()->report(m_reportid[page]));

  if (dlg.exec())
  {
    MyMoneyFile::instance()->modifyReport(dlg.getConfig());
    slotRefreshView();
    
    // NOTE: This is only valid as long as we aren't deleting or adding pages here!!
    m_reportTabWidget->setCurrentPage(page);
    
  }
}

void KReportsView::slotDuplicate(void)
{
  int page = m_reportTabWidget->currentPageIndex();
  
  MyMoneyReport dupe = MyMoneyFile::instance()->report(m_reportid[page]);
  dupe.setName( QString("Copy of %1").arg(dupe.name()) );
  dupe.setId(QCString());
  MyMoneyFile::instance()->addReport(dupe);
  
  kdDebug(2) << "KReportsView::slotDuplicate(): Added report " << dupe.id() << endl;
  
  KReportConfigurationFilterDlg dlg(dupe);
  if (dlg.exec())
    MyMoneyFile::instance()->modifyReport(dlg.getConfig());
  
  slotRefreshView();
  m_reportTabWidget->setCurrentPage(MyMoneyFile::instance()->countReports()-1);
}

const QString KReportsView::linkfull(const QString& view, const QString& query, const QString& label)
{
  return QString("<a href=\"/") + view + query + "\">" + label + "</a>";
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
    else
      qDebug("Unknown command '%s' in KReportsView::slotOpenURL()", static_cast<const char*>(command));

  } else {
    qDebug("Unknown view '%s' in KReportsView::slotOpenURL()", view.latin1());
  }
}

