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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

// ----------------------------------------------------------------------------
// QT Includes

#include <qvbox.h>
#include <qlayout.h>
#include <qbuttongroup.h>
#include <qabstractlayout.h>
#include <qpixmap.h>
#include <qtextview.h>
#include <qlabel.h>

// ----------------------------------------------------------------------------
// KDE Includes

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

// ----------------------------------------------------------------------------
// Project Includes

#include "kstartdlg.h"
#include "krecentfileitem.h"
#include "../kmymoney2.h"

#include <qtooltip.h>

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
  (void)new QIconViewItem( view_wizard, i18n("New KMyMoney document"), ic->loadIcon("mime_empty.png", KIcon::Desktop, KIcon::SizeLarge)/*QPixmap( locate("icon","hicolor/48x48/mimetypes/mime_empty.png") )*/ );
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
  kurlrequest->fileDialog()->setURL(KURL(kmymoney2->readLastUsedDir()));//kurlrequest->fileDialog()->setURL(KURL(KGlobalSettings::documentPath()));
  mainLayout->addWidget( kurlrequest );

  QLabel *label1 = new QLabel( recentMainFrame, "label1" );
  label1->setText( i18n("Recent Files") );
  mainLayout->addWidget( label1 );
  view_recent = new KIconView( recentMainFrame, "view_recent" );
  connect( view_recent, SIGNAL( executed(QIconViewItem *) ), this, SLOT( slotRecentClicked(QIconViewItem *) ) );
  mainLayout->addWidget( view_recent );
  view_recent->setArrangement(KIconView::LeftToRight/*TopToBottom*/);
  view_recent->setItemTextPos(KIconView::Bottom);

  connect(view_recent, SIGNAL(selectionChanged(QIconViewItem*)),
    this, SLOT(slotRecentSelectionChanged(QIconViewItem*)));
}

void KStartDlg::slotTemplateClicked(QIconViewItem *item)
{
  if(!item) return;

  // If the item is the blank document turn isnewfile variable true, else is template or wizard
  if( item->text() == i18n("New KMyMoney document") )
     isnewfile = true;
   else
     templatename = item->text();

  isopenfile = false;
  // Close the window if the user pressed an icon
  slotOk();
}

/** Read config window */
void KStartDlg::readConfig()
{
  QString value;
  unsigned int i = 1;

  KConfig *config = KGlobal::config();
  KIconLoader *il = KGlobal::iconLoader();

  // read file list
  do {
    // for some reason, I had to setup the group to get reasonable results
    // after program startup. If the wizard was opened the second time,
    // it does not make a difference, if you call setGroup() outside of
    // this loop. The first time it does make a difference!
    config->setGroup("Recent Files");
    value = config->readEntry( QString( "File%1" ).arg( i ), QString::null );
    if( !value.isNull() && fileExists(value) )
    {
      QString file_name = value.mid(value.findRev('/')+1);
      (void)new KRecentFileItem( value, view_recent, file_name, il->loadIcon("kmy", KIcon::Desktop, KIcon::SizeLarge));
    }
    i++;
  } while( !value.isNull() );

  config->setGroup("Start Dialog");
  QSize *defaultSize = new QSize(400,300);
  this->resize( config->readSizeEntry("Geometry", defaultSize ) );

  // Restore the last page viewed
  // default to the recent files page if no entry exists but files have been found
  // otherwise, default to template page
  if(view_recent->count() > 0)
    this->showPage(config->readNumEntry("LastPage", this->pageIndex(recentMainFrame)));
  else
    this->showPage(config->readNumEntry("LastPage", this->pageIndex(templateMainFrame)));
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
  KRecentFileItem *kitem = (KRecentFileItem*)item;
  if(!kitem) return;

  isopenfile = true;
  kurlrequest->setURL( kitem->fileURL() );
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

  // If the item is the blank document turn isnewfile
  // variable true, else is template or wizard
  if( item->text() == i18n("Blank Document") )
    isnewfile = true;
  else
    templatename = item->text();

  isopenfile = false;
}
  
void KStartDlg::slotRecentSelectionChanged(QIconViewItem* item)
{
  KRecentFileItem *kitem = (KRecentFileItem*)item;
  if(!kitem) return;

  // Clear the other selection
  view_wizard->clearSelection();

  isnewfile = false;
  isopenfile = true;
  kurlrequest->setURL( kitem->fileURL() );
}
