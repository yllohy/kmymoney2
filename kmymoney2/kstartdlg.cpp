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
#include "kstartdlg.moc"

#include <stream.h>

#include <qvbox.h>
#include <qlayout.h>
#include <qbuttongroup.h>
#include <qabstractlayout.h>
#include <qpixmap.h>
#include <qtextview.h>

#include <kstddirs.h>
#include <kiconloader.h>
#include <kconfig.h>
#include <kglobalsettings.h>
#include <kfiledialog.h>
#include <kurl.h>
#include <kurlrequester.h>
#include <kfile.h>

KStartDlg::KStartDlg(QString &obsolete, QWidget *parent, const char *name, bool modal) : KDialogBase(IconList,i18n("Start KMyMoney 2"),Help|Ok|Cancel,Ok, parent, name, modal, true)
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
  (kurlrequest->fileDialog())->setFilter( i18n("%1|KMyMoney files (*.kmy)").arg("*.kmy") );
  mainLayout->addWidget( kurlrequest );

  QGroupBox *GroupBox1 = new QGroupBox( mainFrame, "GroupBox1" );
  GroupBox1->setTitle( i18n( "Recent Documents" ) );
  GroupBox1->setColumnLayout(0, Qt::Vertical );
  GroupBox1->layout()->setSpacing( 0 );
  GroupBox1->layout()->setMargin( 0 );
  QVBoxLayout *GroupBox1Layout = new QVBoxLayout( GroupBox1->layout() );
  GroupBox1Layout->setAlignment( Qt::AlignTop );
  GroupBox1Layout->setSpacing( 6 );
  GroupBox1Layout->setMargin( 11 );
  view_recent = new KIconView( GroupBox1, "view_recent" );
  connect( view_recent, SIGNAL( executed(QIconViewItem *) ), this, SLOT( slotRecentClicked(QIconViewItem *) ) );
  GroupBox1Layout->addWidget( view_recent );
  mainLayout->addWidget( GroupBox1 );
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
		if( !value.isNull() )
			(void)new QIconViewItem( view_recent, value, QPixmap( locate("icon","hicolor/48x48/mimetypes/kmymoney2.png") ) );
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
