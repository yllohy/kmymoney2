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

// ----------------------------------------------------------------------------
// Project Includes
#include "kreportsview.h"
#include "../mymoney/mymoneyfile.h"
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

  ReportConfiguration spending;
  spending.setName(i18n("Monthly Income and Expenses"));
  spending.setDateRange(QDate(QDate::currentDate().year(),1,1),QDate::currentDate());
  spending.setShowSubAccounts(true);
  spending.setRowFilter( ReportConfiguration::eExpense | ReportConfiguration::eIncome );
  m_reports.push_back(spending);

  ReportConfiguration networth;
  networth.setName(i18n("Net Worth Over Time"));
  networth.setDateRange(QDate(QDate::currentDate().year(),1,1),QDate::currentDate());
  networth.setShowSubAccounts(false);
  networth.setRowFilter( ReportConfiguration::eAsset | ReportConfiguration::eLiability );
  m_reports.push_back(networth);

  m_qvboxlayoutPage = new QVBoxLayout(this);
  m_qvboxlayoutPage->setSpacing( 6 );
  m_qvboxlayoutPage->setMargin( 11 );

  m_reportTabWidget = new QTabWidget( this, "reportTabWidget" );

  m_tab.push_back(new QWidget( m_reportTabWidget, "tab[0]" ));
  m_tabLayout.push_back(new QVBoxLayout( m_tab[0], 11, 6, "tabLayout[0]"));
  m_part.push_back(new KHTMLPart(m_tab[0], "htmlpart_km2[0]"));
  m_tabLayout[0]->addWidget( m_part[0]->view() );
  m_reportTabWidget->insertTab( m_tab[0], m_reports[0].getName() );
  connect(m_part[0]->browserExtension(), SIGNAL(openURLRequest(const KURL&, const KParts::URLArgs&)),
          this, SLOT(slotOpenURL(const KURL&, const KParts::URLArgs&)));

  m_tab.push_back(new QWidget( m_reportTabWidget, "tab[1]" ));
  m_tabLayout.push_back(new QVBoxLayout( m_tab[1], 11, 6, "tabLayout[1]"));
  m_part.push_back( new KHTMLPart(m_tab[1], "htmlpart_km2[1]"));
  m_tabLayout[1]->addWidget( m_part[1]->view() );
  m_reportTabWidget->insertTab( m_tab[1], m_reports[1].getName() );
  connect(m_part[1]->browserExtension(), SIGNAL(openURLRequest(const KURL&, const KParts::URLArgs&)),
          this, SLOT(slotOpenURL(const KURL&, const KParts::URLArgs&)));
  
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

const QString KReportsView::createTable(const ReportConfiguration& report, const QString& links) const
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
  html += PivotTable( report ).renderHTML();
  html += footer;

  return html;
}

void KReportsView::slotRefreshView(void)
{
  QString links;

  links += linkfull(VIEW_REPORTS, QString("?command=configure&target=1"),i18n("Configure This Report"));
  links += "&nbsp;|&nbsp;";
  links += linkfull(VIEW_REPORTS, QString("?command=copy&target=1"),i18n("Copy to Clipboard"));
  links += "&nbsp;|&nbsp;";
  links += linkfull(VIEW_REPORTS, QString("?command=save&target=1"),i18n("Save to File"));
  
  m_part[0]->begin();
  m_part[0]->write(createTable(m_reports[0],links));
  m_part[0]->end();

  m_part[1]->begin();
  m_part[1]->write(createTable(m_reports[1],links));
  m_part[1]->end();
}

void KReportsView::slotPrintView(void)
{
  int page = m_reportTabWidget->currentPageIndex();

  KHTMLPart part(this, "htmlpart_km2");

  part.begin();
  part.write(createTable(m_reports[page]));
  part.end();

  part.view()->print();
}

void KReportsView::slotCopyView(void)
{
  int page = m_reportTabWidget->currentPageIndex();

  QTextDrag* pdrag =  new QTextDrag( createTable(m_reports[page]) );
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
      stream <<  createTable(m_reports[page]);
      file.close();
    }
  }
}

#include <kmessagebox.h>
#include "../dialogs/kreportconfigurationdlg.h"

void KReportsView::slotConfigure(void)
{
  int page = m_reportTabWidget->currentPageIndex();

  KReportConfigurationDlg dlg(m_reports[page]);

  if (dlg.exec())
  {
    m_reports[page] = dlg.getResult();
    m_reportTabWidget->changeTab( m_tab[page], m_reports[page].getName() );

    slotRefreshView();
  }
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
    else
      qDebug("Unknown command '%s' in KReportsView::slotOpenURL()", static_cast<const char*>(command));

  } else {
    qDebug("Unknown view '%s' in KReportsView::slotOpenURL()", view.latin1());
  }
}

