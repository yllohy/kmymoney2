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
#include <qvalidator.h>

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
	GroupBox1->setTitle( i18n( "Startup options" ) );
	GroupBox1->setColumnLayout(0, Qt::Vertical );
	GroupBox1->layout()->setSpacing( 0 );
	GroupBox1->layout()->setMargin( 0 );
	QVBoxLayout *GroupBox1Layout = new QVBoxLayout( GroupBox1->layout() );
	GroupBox1Layout->setAlignment( Qt::AlignTop );
	GroupBox1Layout->setSpacing( 6 );
	GroupBox1Layout->setMargin( 11 );

	start_prompt = new QCheckBox( "start_prompt", GroupBox1 );
	start_prompt->setText( i18n( "Start with dialog prompt (default)" ) );
	start_prompt->setChecked( TRUE );
	GroupBox1Layout->addWidget( start_prompt );
}

/** Set page list settings */
void KSettingsDlg::setPageList()
{
	QVBox *mainFrame = addVBoxPage( i18n("Main List"), i18n("List settings"), locate("appdata", "pics/setting_list.png"));

	QTabWidget *TabWidget2 = new QTabWidget( mainFrame, "TabWidget2" );

	QWidget *theTab = new QWidget(this);
	QVBoxLayout *tab0 = new QVBoxLayout(theTab);
	tab0->setSpacing( 6 );
	tab0->setMargin( 11 );
	QLabel *label0 = new QLabel("some description text", theTab);
	tab0->addWidget(label0);
  QHBoxLayout *Layout1 = new QHBoxLayout;
  Layout1->setSpacing( 6 );
  Layout1->setMargin( 0 );
	QLabel *label = new QLabel(i18n("Number of lines in the register view:"), theTab);
	Layout1->addWidget(label);
//	GroupBox1Layout->addWidget(label);
	m_klineeditRowCount = new KLineEdit(theTab);
	QIntValidator *qintvalidator = new QIntValidator(1, 3, m_klineeditRowCount);
	m_klineeditRowCount->setValidator(qintvalidator);
	Layout1->addWidget(m_klineeditRowCount);
	tab0->addLayout(Layout1);
	m_qcheckboxShowGrid = new QCheckBox(i18n("Show a grid in the register view"), theTab);
	tab0->addWidget(m_qcheckboxShowGrid);
  QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
  tab0->addItem( spacer );
  TabWidget2->insertTab(theTab, i18n("General"));
	
	QWidget *tab = new QWidget( TabWidget2, "tab" );
	QGridLayout *tabLayout = new QGridLayout( tab );
	tabLayout->setSpacing( 6 );
	tabLayout->setMargin( 11 );
	QLabel *TextLabel1_2 = new QLabel( tab, "TextLabel1_2" );
	TextLabel1_2->setText( i18n( "List view colour :" ) );
	TextLabel1_2->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );
	tabLayout->addWidget( TextLabel1_2, 0, 0 );
	QLabel *TextLabel2_2 = new QLabel( tab, "TextLabel2_2" );
	TextLabel2_2->setText( i18n( "List background color :" ) );
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

	config->setGroup("General Options");
	start_prompt->setChecked(config->readBoolEntry("StartDialog", true));
	
  config->setGroup("List Options");

  QFont defaultFont = QFont("helvetica", 12);
  QColor defaultColor = Qt::white;
  QColor defaultBGColor = Qt::gray;
	
	color_list->setColor(config->readColorEntry("listColor", &defaultColor));
	color_back->setColor(config->readColorEntry("listBGColor", &defaultBGColor));
	font_header->setFont(config->readFontEntry("listHeaderFont", &defaultFont));
	font_cell->setFont(config->readFontEntry("listCellFont", &defaultFont));
	m_klineeditRowCount->setText(config->readEntry("RowCount", "2"));
	m_qcheckboxShowGrid->setChecked(config->readBoolEntry("ShowGrid", true));
}

/** Write settings */
void KSettingsDlg::configWrite()
{
  KConfig *config = KGlobal::config();
	config->setGroup("Settings Dialog");
	config->writeEntry("Geometry", this->size() );

  config->setGroup("List Options");
	config->writeEntry("listColor", color_list->color());
	config->writeEntry("listBGColor", color_back->color());
	config->writeEntry("listHeaderFont", font_header->font());
	config->writeEntry("listCellFont", font_cell->font());
	config->writeEntry("RowCount", m_klineeditRowCount->text());
	config->writeEntry("ShowGrid", m_qcheckboxShowGrid->isChecked());
	
	config->setGroup("General Options");
	config->writeEntry("StartDialog", start_prompt->isChecked());

  config->sync();
}

/** Slot ok */
void KSettingsDlg::slotOk()
{
	configWrite();
	this->accept();
}
