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
#include <kmessagebox.h>

#include <qlayout.h>
#include <qvbox.h>
#include <qlabel.h>
#include <qgroupbox.h>
#include <qtabwidget.h>
#include <qvalidator.h>

KSettingsDlg::KSettingsDlg(QWidget *parent, const char *name, bool modal)
 : KDialogBase(IconList, i18n("Configure"), Ok|Cancel|Apply|User1, Ok, parent, name, modal, true,
    "&Reset")
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

  QButtonGroup *qbuttongroup = new QButtonGroup( theTab, "ButtonGroup1" );
  qbuttongroup->setColumnLayout(0, Qt::Vertical );
  qbuttongroup->layout()->setSpacing( 0 );
  qbuttongroup->layout()->setMargin( 0 );
  QVBoxLayout *qvboxlayout = new QVBoxLayout(qbuttongroup->layout());
  qvboxlayout->setAlignment( Qt::AlignTop );
  qvboxlayout->setSpacing( 6 );
  qvboxlayout->setMargin( 11 );
  qbuttongroup->setTitle( i18n( "Row Colour options" ) );

  m_qradiobuttonPerTransaction = new QRadioButton(qbuttongroup, "m_per_trans");
  m_qradiobuttonPerTransaction->setGeometry( QRect( 10, 20, 88, 21 ) );
  m_qradiobuttonPerTransaction->setText( i18n("Use one colour per transaction") );
  qvboxlayout->addWidget(m_qradiobuttonPerTransaction);

  m_qradiobuttonOtherRow = new QRadioButton(qbuttongroup, "m_every_other");
  m_qradiobuttonOtherRow->setGeometry( QRect( 10, 50, 90, 21 ) );
  m_qradiobuttonOtherRow->setText( i18n( "Change colour every other row" ) );
  qvboxlayout->addWidget(m_qradiobuttonOtherRow);
  tab0->addWidget(qbuttongroup);

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
	m_bStartPrompt = config->readBoolEntry("StartDialog", true);
	start_prompt->setChecked(m_bStartPrompt);
	
  config->setGroup("List Options");

  QFont defaultFont = QFont("helvetica", 12);
  QColor defaultColor = Qt::white;
  QColor defaultBGColor = Qt::gray;
	
	m_tempListColour = config->readColorEntry("listColor", &defaultColor);
	color_list->setColor(m_tempListColour);
	
	m_tempListBG = config->readColorEntry("listBGColor", &defaultBGColor);
	color_back->setColor(m_tempListBG);
	
	m_tempFontHeader = config->readFontEntry("listHeaderFont", &defaultFont);
	font_header->setFont(m_tempFontHeader);
	
	m_tempFontCell = config->readFontEntry("listCellFont", &defaultFont);
	font_cell->setFont(m_tempFontCell);
	
	m_tempRowCount = config->readEntry("RowCount", "2");
	m_klineeditRowCount->setText(m_tempRowCount);
	
	m_tempShowGrid = config->readBoolEntry("ShowGrid", true);
	m_qcheckboxShowGrid->setChecked(m_tempShowGrid);
	
  m_tempColourPerTransaction = config->readBoolEntry("ColourPerTransaction", true);
  if (m_tempColourPerTransaction) {
    m_qradiobuttonPerTransaction->setChecked(true);
    m_qradiobuttonOtherRow->setChecked(false);
  } else {
    m_qradiobuttonPerTransaction->setChecked(false);
    m_qradiobuttonOtherRow->setChecked(true);
  }
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
	config->writeEntry("ColourPerTransaction", m_qradiobuttonPerTransaction->isChecked());
	
	config->setGroup("General Options");
	config->writeEntry("StartDialog", start_prompt->isChecked());

  config->sync();
}

/** Slot ok */
void KSettingsDlg::slotOk()
{
	int nCount = m_klineeditRowCount->text().toInt();
	if (nCount <= 0 || nCount >= 4) {
	  KMessageBox::information(this, "The row count has to be between 1 and 3");
	  m_klineeditRowCount->setFocus();
	  return;
	}
	configWrite();
	this->accept();
}

void KSettingsDlg::slotApply()
{
	int nCount = m_klineeditRowCount->text().toInt();
	if (nCount <= 0 || nCount >= 4) {
	  KMessageBox::information(this, "The row count has to be between 1 and 3");
	  m_klineeditRowCount->setFocus();
	  return;
	}
  m_bDoneApply = true;
  configWrite();
  emit signalApply();
}

void KSettingsDlg::slotCancel()
{
  // make sure the config object is the same as we left it
  KConfig *config = KGlobal::config();
  config->setGroup("List Options");
	config->writeEntry("listColor", m_tempListColour);
	config->writeEntry("listBGColor", m_tempListBG);
	config->writeEntry("listHeaderFont", m_tempFontHeader);
	config->writeEntry("listCellFont", m_tempFontCell);
	config->writeEntry("RowCount", m_tempRowCount);
	config->writeEntry("ShowGrid", m_tempShowGrid);
	config->writeEntry("ColourPerTransaction", m_tempColourPerTransaction);
	
	config->setGroup("General Options");
	config->writeEntry("StartDialog", m_bStartPrompt);

  config->sync();

  if (m_bDoneApply)
    accept();
  else
    reject();
}

void KSettingsDlg::slotUser1()
{
	start_prompt->setChecked(m_bStartPrompt);
	color_list->setColor(m_tempListColour);
	color_back->setColor(m_tempListBG);
	font_header->setFont(m_tempFontHeader);
	font_cell->setFont(m_tempFontCell);
	m_klineeditRowCount->setText(m_tempRowCount);
	m_qcheckboxShowGrid->setChecked(m_tempShowGrid);
  if (m_tempColourPerTransaction) {
    m_qradiobuttonPerTransaction->setChecked(true);
    m_qradiobuttonOtherRow->setChecked(false);
  } else {
    m_qradiobuttonPerTransaction->setChecked(false);
    m_qradiobuttonOtherRow->setChecked(true);
  }
}
