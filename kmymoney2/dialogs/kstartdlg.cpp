/***************************************************************************
                          KStartDlg.cpp  -  description
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "config.h"

#include "kstartdlg.h"

#include <qvbox.h>
#include <qlayout.h>
#include <qbuttongroup.h>
#include <qabstractlayout.h>
#include <qpixmap.h>
#include <qtextview.h>
#include <qlabel.h>

#if QT_VERSION > 300
#include <kstandarddirs.h>
#else
#include <kstddirs.h>
#endif

#include <kiconloader.h>
#include <kconfig.h>
#include <kglobalsettings.h>
#include <kfiledialog.h>
#include <kurl.h>
#include <kurlrequester.h>
#include <kfile.h>
#include <kio/netaccess.h>
#include <kmessagebox.h>

KStartDlg::KStartDlg(QWidget *parent, const char *name, bool modal) : KDialogBase(IconList,i18n("Start Dialog"),Help|Ok|Cancel,Ok, parent, name, modal, true)
{
	setPage_Template();
  setPage_Documents();

	isnewfile = false;
	isopenfile = false;

	readConfig();
}

KStartDlg::~KStartDlg()
{
}

/** Set the font  Page of the preferences dialog */
void KStartDlg::setPage_Template()
{
  KIconLoader *ic = KGlobal::iconLoader();
	templateMainFrame = addVBoxPage( i18n("Templates"), i18n("Select templates"), DesktopIcon("wizard"));
  view_wizard = new KIconView( templateMainFrame, "view_options" );
  (void)new QIconViewItem( view_wizard, i18n("Blank Document"), ic->loadIcon("mime_empty.png", KIcon::Desktop, KIcon::SizeLarge)/*QPixmap( locate("icon","hicolor/48x48/mimetypes/mime_empty.png") )*/ );
  connect( view_wizard, SIGNAL( executed(QIconViewItem *) ), this, SLOT( slotTemplateClicked(QIconViewItem *) ) );
  connect(view_wizard, SIGNAL(selectionChanged(QIconViewItem*)),
    this, SLOT(slotTemplateSelectionChanged(QIconViewItem*)));
}

/** Set the Misc options Page of the preferences dialog */
void KStartDlg::setPage_Documents()
{
	recentMainFrame = addPage( i18n("Open"), i18n("Open a KMyMoney document"), DesktopIcon("fileopen"));
  QVBoxLayout *mainLayout = new QVBoxLayout( recentMainFrame );

  kurlrequest = new KURLRequester( recentMainFrame, "kurlrequest" );

	//allow user to select either a .kmy file, or any generic file.
  kurlrequest->fileDialog()->setFilter( i18n("%1|KMyMoney files (*.kmy)\n" "%2|All files (*.*)").arg("*.kmy").arg("*.*") );
  kurlrequest->fileDialog()->setMode(KFile::File || KFile::ExistingOnly);
  mainLayout->addWidget( kurlrequest );

	QLabel *label1 = new QLabel( recentMainFrame, "label1" );
	label1->setText( i18n("Recent Files") );
	mainLayout->addWidget( label1 );
  view_recent = new KIconView( recentMainFrame, "view_recent" );
  connect( view_recent, SIGNAL( executed(QIconViewItem *) ), this, SLOT( slotRecentClicked(QIconViewItem *) ) );
  mainLayout->addWidget( view_recent );
  view_recent->setArrangement(KIconView::TopToBottom);
  view_recent->setItemTextPos(KIconView::Right);

  connect(view_recent, SIGNAL(selectionChanged(QIconViewItem*)),
    this, SLOT(slotRecentSelectionChanged(QIconViewItem*)));
}

void KStartDlg::slotTemplateClicked(QIconViewItem *item)
{
  if(!item) return;

  // If the item is the blank document turn isnewfile variable true, else is template or wizard
  if( item->text() == i18n("Blank Document") )
     isnewfile = true;
     else
     templatename = item->text();

  isopenfile = false;
  // Close the window if the user press an icon
	slotOk();
}

/** Read config window */
void KStartDlg::readConfig()
{
	QString key;
	QString value = "";

	KConfig *config = KGlobal::config();

	config->setGroup("Recent Files");

	// read file list
	uint i=1;
	while( !value.isNull() )
	{
		key = QString( "File%1" ).arg( i );
		value = config->readEntry( key, QString::null );
		if( !value.isNull() && fileExists(value) )
			(void)new QIconViewItem( view_recent, value, QPixmap( locate("icon","hicolor/48x48/mimetypes/kmy.png") ) );
		i++;
	}

	config->setGroup("Start Dialog");
	QSize *defaultSize = new QSize(400,300);
	this->resize( config->readSizeEntry("Geometry", defaultSize ) );

  // Restore the last page viewed
  // default to the recent files page if no entry exists
  this->showPage(config->readNumEntry("LastPage", this->pageIndex(recentMainFrame)));

}

/** Write config window */
void KStartDlg::writeConfig()
{
  KConfig *config = KGlobal::config();

  config->setGroup("Start Dialog");
  config->writeEntry("Geometry", this->size() );
  config->writeEntry("LastPage", this->activePageIndex());
  config->sync();
}

/** slot to recent view */
void KStartDlg::slotRecentClicked(QIconViewItem *item)
{
  if(!item) return;

  isopenfile = true;
	kurlrequest->setURL( item->text() );
  // Close the window if the user press an icon
	slotOk();
}

/** No descriptions */
void KStartDlg::slotOk()
{
	writeConfig();
	this->accept();	
}

bool KStartDlg::fileExists(KURL url)
{
  return KIO::NetAccess::exists(url);
}

void KStartDlg::slotTemplateSelectionChanged(QIconViewItem* item)
{
  if(!item) return;

  // Clear the other selection
  view_recent->clearSelection();

  // If the item is the blank document turn isnewfile variable true, else is template or wizard
  if( item->text() == i18n("Blank Document") )
     isnewfile = true;
  else
    templatename = item->text();

  isopenfile = false;
}
  
void KStartDlg::slotRecentSelectionChanged(QIconViewItem* item)
{
  if(!item) return;

  // Clear the other selection
  view_wizard->clearSelection();

  isnewfile = false;
  isopenfile = true;
  kurlrequest->setURL( item->text() );
}
