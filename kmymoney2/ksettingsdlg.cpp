/***************************************************************************
                          ksettingsdlg.cpp
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes
    email                : mte@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "ksettingsdlg.h"
#include "kmymoneysettings.h"

#include <klocale.h>
#include <kstddirs.h>
#include <kiconloader.h>
#include <kconfig.h>
#include <kcolorbutton.h>

#include <qlayout.h>
#include <qvbox.h>
#include <qlabel.h>
#include <qgroupbox.h>
#include <qtabwidget.h>

KSettingsDlg::KSettingsDlg(QWidget *parent, const char *name, bool modal)
 : KDialogBase(IconList, i18n("Configure"), Ok|Cancel, Ok, parent, name, modal, true)
{
	setPageGeneral();
	setPageList();
	configRead();
}

KSettingsDlg::~KSettingsDlg()
{
}

/** Set page general */
void KSettingsDlg::setPageGeneral()
{
	QVBox *mainFrame = addVBoxPage( i18n("General"), i18n("General settings"), DesktopIcon("misc"));

	QGroupBox *GroupBox1 = new QGroupBox( mainFrame, "GroupBox1" );
	GroupBox1->setTitle( tr( "Startup options" ) );
	GroupBox1->setColumnLayout(0, Qt::Vertical );
	GroupBox1->layout()->setSpacing( 0 );
	GroupBox1->layout()->setMargin( 0 );
	QVBoxLayout *GroupBox1Layout = new QVBoxLayout( GroupBox1->layout() );
	GroupBox1Layout->setAlignment( Qt::AlignTop );
	GroupBox1Layout->setSpacing( 6 );
	GroupBox1Layout->setMargin( 11 );

	start_prompt = new QRadioButton( GroupBox1, "start_prompt" );
	start_prompt->setText( tr( "Start with dialog prompt (default)" ) );
	start_prompt->setChecked( TRUE );
	GroupBox1Layout->addWidget( start_prompt );

	start_last = new QRadioButton( GroupBox1, "start_last" );
	start_last->setText( tr( "Open last proyect" ) );
	GroupBox1Layout->addWidget( start_last );
}

/** Set page list settings */
void KSettingsDlg::setPageList()
{
	QVBox *mainFrame = addVBoxPage( i18n("Main List"), i18n("List settings"), locate("appdata", "pics/setting_list.png"));

	QTabWidget *TabWidget2 = new QTabWidget( mainFrame, "TabWidget2" );

	QWidget *tab = new QWidget( TabWidget2, "tab" );
	QGridLayout *tabLayout = new QGridLayout( tab );
	tabLayout->setSpacing( 6 );
	tabLayout->setMargin( 11 );
	QLabel *TextLabel1_2 = new QLabel( tab, "TextLabel1_2" );
	TextLabel1_2->setText( tr( "List view colour :" ) );
	TextLabel1_2->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );
	tabLayout->addWidget( TextLabel1_2, 0, 0 );
	QLabel *TextLabel2_2 = new QLabel( tab, "TextLabel2_2" );
	TextLabel2_2->setText( tr( "List background color :" ) );
	TextLabel2_2->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );
	tabLayout->addWidget( TextLabel2_2, 1, 0 );
	color_list = new KColorButton( tab, "color_list" );
	tabLayout->addWidget( color_list, 1, 1 );
	color_back = new KColorButton( tab, "color_back" );
	tabLayout->addWidget( color_back, 0, 1 );
	TabWidget2->insertTab( tab, i18n( "Color" ) );

	QVBox *insideTab1 = new QVBox( this, "tab1" );
	insideTab1->setSpacing( 6 );
	insideTab1->setMargin( 11 );
	font_header = new KFontChooser(insideTab1);
	TabWidget2->insertTab( insideTab1, i18n("Header Font") );

	QVBox *insideTab2 = new QVBox( this, "tab2" );
	insideTab2->setSpacing( 6 );
	insideTab2->setMargin( 11 );
	font_cell = new KFontChooser(insideTab2);
	TabWidget2->addTab( insideTab2, "Cell Font" );
}

/** Read settings */
void KSettingsDlg::configRead()
{
	KConfig *config = KGlobal::config();
	config->setGroup("Settings Dialog");
	QSize *defaultSize = new QSize(470,470);
	this->resize( config->readSizeEntry("Geometry", defaultSize ) );

  KMyMoneySettings *p_settings = KMyMoneySettings::singleton();
  color_list->setColor(p_settings->lists_color());
	color_back->setColor(p_settings->lists_BGColor());
	font_header->setFont(p_settings->lists_headerFont());
	font_cell->setFont(p_settings->lists_cellFont());
}

/** Write settings */
void KSettingsDlg::configWrite()
{
  KConfig *config = KGlobal::config();
	config->setGroup("Settings Dialog");
	config->writeEntry("Geometry", this->size() );
  config->sync();

  KMyMoneySettings *p_settings = KMyMoneySettings::singleton();
	p_settings->setListSettings(
				color_list->color(),
      	color_back->color(),
      	font_header->font(),
      	font_cell->font() );
}

/** Slot ok */
void KSettingsDlg::slotOk()
{
	configWrite();
	this->accept();
}
