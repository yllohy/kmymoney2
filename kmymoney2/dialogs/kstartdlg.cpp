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
	QVBox *mainFrame = addVBoxPage( i18n("Templates"), i18n("Select templates"), DesktopIcon("wizard"));
  view_wizard = new KIconView( mainFrame, "view_options" );
  (void)new QIconViewItem( view_wizard, i18n("Blank Document"), QPixmap( locate("icon","hicolor/48x48/mimetypes/mime_empty.png") ) );
  connect( view_wizard, SIGNAL( executed(QIconViewItem *) ), this, SLOT( slotTemplateClicked(QIconViewItem *) ) );
}

/** Set the Misc options Page of the preferences dialog */
void KStartDlg::setPage_Documents()
{
	QFrame *mainFrame = addPage( i18n("Open"), i18n("Open a KMyMoney document"), DesktopIcon("fileopen"));
  QVBoxLayout *mainLayout = new QVBoxLayout( mainFrame );

  kurlrequest = new KURLRequester( mainFrame, "kurlrequest" );

	//allow user to select either a .kmy file, or any generic file.
  (kurlrequest->fileDialog())->setFilter( i18n("%1|KMyMoney files (*.kmy)\n" "%2|All files (*.*)").arg("*.kmy").arg("*.*") );
  mainLayout->addWidget( kurlrequest );

	QLabel *label1 = new QLabel( mainFrame, "label1" );
	label1->setText( i18n("Recent Files") );
	mainLayout->addWidget( label1 );
  view_recent = new KIconView( mainFrame, "view_recent" );
  connect( view_recent, SIGNAL( executed(QIconViewItem *) ), this, SLOT( slotRecentClicked(QIconViewItem *) ) );
  mainLayout->addWidget( view_recent );
  view_recent->setArrangement(KIconView::TopToBottom);
  view_recent->setItemTextPos(KIconView::Right);
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

}

/** Write config window */
void KStartDlg::writeConfig()
{
  KConfig *config = KGlobal::config();

  config->setGroup("Start Dialog");
  config->writeEntry("Geometry", this->size() );
  config->sync();
}

/** slot to recent view */
void KStartDlg::slotRecentClicked(QIconViewItem *item)
{
  if(!item) return;

	KURL fileNAME;
  // If the item is the blank document turn isnewfile variable true, else is template or wizard
  isopenfile = true;
	fileNAME = item->text();
	kurlrequest->setURL( fileNAME.directory(false,true)+fileNAME.fileName() );
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
  if (url.isLocalFile()) {
    // Lets make sure it exists first
    if (url.fileName().length()>=1) {
      QFile f(url.directory(false,true)+url.fileName());
      return f.exists();
    }
  }
  // We don't bother checking URL's or showing them
  // because at the moment MyMoneyFile can't read them
  // anyway
  return false;
}
