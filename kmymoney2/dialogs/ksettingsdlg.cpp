/***************************************************************************
                          ksettingsdlg.cpp
                             -------------------
    copyright            : (C) 2000,2001 by Michael Edwardes
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

// ----------------------------------------------------------------------------
// QT Includes
#include <qapp.h>
#include <qlayout.h>
#include <qvbox.h>
#include <qlabel.h>
#include <qgroupbox.h>
#include <qtabwidget.h>
#include <qvalidator.h>
#include <qheader.h>
#include <qlistview.h>

// ----------------------------------------------------------------------------
// KDE Includes
#include <klocale.h>
#if QT_VERSION > 300
#include <kstandarddirs.h>
#else
#include <kstddirs.h>
#endif

#include <kiconloader.h>
#include <kconfig.h>
#if QT_VERSION > 300
#include <kcolorbutton.h>
#else
#include <kcolorbtn.h>
#endif

#include <kmessagebox.h>
#include <klistview.h>
#include <kpushbutton.h>
#include <kguiitem.h>
#include <kglobalsettings.h>

// ----------------------------------------------------------------------------
// Project Includes
#include "ksettingsdlg.h"
#include "../kmymoneyutils.h"

/** Standard constructor for the class.
  * The constructor passes some additional parameters to the base class KDialogBase
  * to set the buttons to be showed and the type of dialog to be shown.
**/
KSettingsDlg::KSettingsDlg(QWidget *parent, const char *name, bool modal)
 : KDialogBase(IconList, i18n("Configure"), Ok|Cancel|Apply|User1, Ok, parent,
    name, modal, true, i18n("&Reset"))
{
  // Setup the pages and then read the configuration object.
  setPageGeneral();
  setPageAccountsView();
  setPageList();
  setHomePage();
  configRead();
  
  m_bDoneApply=false;
  
  KPushButton *reset = static_cast<KPushButton *>(actionButton(User1));
  
  KIconLoader* il = KGlobal::iconLoader();
  KGuiItem resetButtonItem( i18n( "&Reset" ),
                    QIconSet(il->loadIcon("undo", KIcon::Small, KIcon::SizeSmall)),
                    i18n("Reset all settings"),
                    i18n("Use this to reset all settings to the state they were when the dialog was opened."));
  reset->setGuiItem(resetButtonItem);

}

/** Standard destructor for the class.
**/
KSettingsDlg::~KSettingsDlg()
{
}

/** Called to create the General page to be shown in the dialog.
**/
void KSettingsDlg::setPageGeneral()
{
  // Create the main frame to hold the widgets
  QVBox *qvboxMainFrame = addVBoxPage( i18n("General"), i18n("General settings"),
    DesktopIcon("misc"));

  // Startup page options
  // --------------------
  
  // Create a group box to hold the available options
  QButtonGroup *qfilebuttongroup = new QButtonGroup(qvboxMainFrame, "GroupBox1");
  qfilebuttongroup->setTitle( i18n( "Startup file options" ) );
  qfilebuttongroup->setColumnLayout(0, Qt::Vertical );
  qfilebuttongroup->layout()->setSpacing( 0 );
  qfilebuttongroup->layout()->setMargin( 0 );

  // Create a layout to organize the widgets.
  QVBoxLayout *qvboxfilelayout = new QVBoxLayout(qfilebuttongroup->layout());
  qvboxfilelayout->setAlignment( Qt::AlignTop );
  qvboxfilelayout->setSpacing( 6 );
  qvboxfilelayout->setMargin( 11 );

  // Create a check box to be in the group box
  m_qradiobuttonStartPrompt = new QRadioButton("start_prompt", qfilebuttongroup);
  m_qradiobuttonStartPrompt->setText( i18n( "Start with dialog prompt (default)" ) );
  qvboxfilelayout->addWidget(m_qradiobuttonStartPrompt);

  // Create another check box to the group box
  m_qradiobuttonStartFile = new QRadioButton("start_file", qfilebuttongroup);
  m_qradiobuttonStartFile->setText( i18n( "Start with last file used" ) );
  qvboxfilelayout->addWidget(m_qradiobuttonStartFile);

  // Startup file options
  // --------------------

  // Create a group box to hold the available options
  QButtonGroup *qpagebuttongroup = new QButtonGroup(qvboxMainFrame, "GroupBox2");
  qpagebuttongroup->setTitle( i18n( "Startup page options" ) );
  qpagebuttongroup->setColumnLayout(0, Qt::Vertical );
  qpagebuttongroup->layout()->setSpacing( 0 );
  qpagebuttongroup->layout()->setMargin( 0 );

  // Create a layout to organize the widgets.
  QVBoxLayout *qvboxpagelayout = new QVBoxLayout(qpagebuttongroup->layout());
  qvboxpagelayout->setAlignment( Qt::AlignTop );
  qvboxpagelayout->setSpacing( 6 );
  qvboxpagelayout->setMargin( 11 );

  // Create a check box to be in the group box
  m_qradiobuttonStartHome = new QRadioButton("start_home", qpagebuttongroup);
  m_qradiobuttonStartHome->setText( i18n( "Start with home page" ) );
  qvboxpagelayout->addWidget(m_qradiobuttonStartHome);

  // Create another check box to the group box
  m_qradiobuttonStartLast = new QRadioButton("start_last", qpagebuttongroup);
  m_qradiobuttonStartLast->setText( i18n( "Start with last selected page" ) );
  qvboxpagelayout->addWidget(m_qradiobuttonStartLast);
}

void KSettingsDlg::setPageAccountsView()
{
  // Create the main frame to hold the widgets
  QVBox *qvboxMainFrame = addVBoxPage( i18n("Accounts View"), i18n("Accounts view settings"),
    DesktopIcon("kmy"));

  // Create a group box to hold the available options
  QButtonGroup *qbuttongroup = new QButtonGroup(qvboxMainFrame, "GroupBox1");
  qbuttongroup->setTitle( i18n( "General Settings" ) );
  qbuttongroup->setColumnLayout(0, Qt::Vertical );
  qbuttongroup->layout()->setSpacing( 0 );
  qbuttongroup->layout()->setMargin( 0 );

  // Create a layout to organize the widgets.
  QVBoxLayout *qvboxlayout = new QVBoxLayout(qbuttongroup->layout());
  qvboxlayout->setAlignment( Qt::AlignTop );
  qvboxlayout->setSpacing( 6 );
  qvboxlayout->setMargin( 11 );

  // Create a check box to be in the group box
  m_qradiobuttonNormalView = new QRadioButton("normal_view", qbuttongroup);
  m_qradiobuttonNormalView->setText( i18n( "Use the normal institution view" ) );
  qvboxlayout->addWidget(m_qradiobuttonNormalView);

  // Create another check box to the group box
  m_qradiobuttonAccountView = new QRadioButton("account_view", qbuttongroup);
  m_qradiobuttonAccountView->setText( i18n( "Use the new accounts view" ) );
  qvboxlayout->addWidget(m_qradiobuttonAccountView);

  // Create a group box to hold the available options for the wizard
  QButtonGroup *qwizardbuttongroup = new QButtonGroup(qvboxMainFrame, "GroupBox1");
  qwizardbuttongroup->setTitle( i18n( "New Account Settings" ) );
  qwizardbuttongroup->setColumnLayout(0, Qt::Vertical );
  qwizardbuttongroup->layout()->setSpacing( 0 );
  qwizardbuttongroup->layout()->setMargin( 0 );

  // Create a layout to organize the widgets.
  QVBoxLayout *qwizardvboxlayout = new QVBoxLayout(qwizardbuttongroup->layout());
  qwizardvboxlayout->setAlignment( Qt::AlignTop );
  qwizardvboxlayout->setSpacing( 6 );
  qwizardvboxlayout->setMargin( 11 );

  // Create a check box to be in the group box
  m_qradiobuttonAccountDialog = new QRadioButton("normal_view", qwizardbuttongroup);
  m_qradiobuttonAccountDialog->setText( i18n( "Use the normal account dialog" ) );
  qwizardvboxlayout->addWidget(m_qradiobuttonAccountDialog);

  // Create another check box to the group box
  m_qradiobuttonAccountWizard = new QRadioButton("account_view", qwizardbuttongroup);
  m_qradiobuttonAccountWizard->setText( i18n( "Use the new account wizard" ) );
  qwizardvboxlayout->addWidget(m_qradiobuttonAccountWizard);

  // Create a check box for the hide category feature
  m_qcheckboxHideCategory = new QCheckBox("hide_categories", qvboxMainFrame);
  m_qcheckboxHideCategory->setText( i18n( "Don't show unused categories" ) );
}

void KSettingsDlg::setHomePage()
{
  // Create the page.
  QVBox *qvboxMainFrame = addVBoxPage( i18n("Home"), i18n("Home page settings"),
    DesktopIcon("home"));

  QFrame* frame = new QFrame(qvboxMainFrame, "frame");
  
  QHBoxLayout* Form1Layout = new QHBoxLayout( frame, 11, 6, "Form1Layout");

  m_homePageList = new KListView( frame, "KListView1" );
  m_homePageList->setMinimumSize( QSize( 250, 0 ) );
  m_homePageList->setSelectionMode(QListView::Single);
  m_homePageList->setSorting(-1);
  m_homePageList->header()->hide();
  m_homePageList->setAllColumnsShowFocus(true);
  m_homePageList->setColumnWidth(0, 250);
  
  Form1Layout->addWidget( m_homePageList );

  QVBoxLayout*  Layout5 = new QVBoxLayout( 0, 0, 6, "Layout5");
  QSpacerItem* spacer = new QSpacerItem( 0, 40, QSizePolicy::Minimum, QSizePolicy::Expanding );
  Layout5->addItem( spacer );

  QHBoxLayout* Layout2 = new QHBoxLayout( 0, 0, 6, "Layout2");

  QVBoxLayout* Layout1 = new QVBoxLayout( 0, 0, 6, "Layout1");

  
  KIconLoader* il = KGlobal::iconLoader();
  KGuiItem upButtonItem( i18n( "&Up" ),
                    QIconSet(il->loadIcon("up", KIcon::Small, KIcon::SizeSmall)),
                    i18n("Move selected item up"),
                    i18n("Use this to move the selected item up by one position in the list."));
  m_upButton = new KPushButton( upButtonItem, frame, "KPushButtonUp" );
  Layout1->addWidget( m_upButton );

  KGuiItem downButtonItem( i18n( "&Down" ),
                    QIconSet(il->loadIcon("down", KIcon::Small, KIcon::SizeSmall)),
                    i18n("Move selected item down"),
                    i18n("Use this to move the selected item down by one position in the list."));
  m_downButton = new KPushButton(downButtonItem, frame, "KPushButtonDown" );
  Layout1->addWidget( m_downButton );
  Layout2->addLayout( Layout1 );
  
  QSpacerItem* spacer_2 = new QSpacerItem( 177, 0, QSizePolicy::Expanding, QSizePolicy::Minimum );
  Layout2->addItem( spacer_2 );
  Layout5->addLayout( Layout2 );
  QSpacerItem* spacer_3 = new QSpacerItem( 0, 40, QSizePolicy::Minimum, QSizePolicy::Expanding );
  Layout5->addItem( spacer_3 );

  QLabel* TextLabel1 = new QLabel( frame, "TextLabel1" );
  TextLabel1->setText( i18n(
     "Selected entries are shown on the home page of the application.\n\n"
     "Use the buttons and checkboxes to customize the layout of the home page."
     ) );
  TextLabel1->setAlignment( int( QLabel::WordBreak | QLabel::AlignTop ) );

  Layout5->addWidget( TextLabel1 );
  QSpacerItem* spacer_4 = new QSpacerItem( 0, 40, QSizePolicy::Minimum, QSizePolicy::Expanding );
  Layout5->addItem( spacer_4 );
  Form1Layout->addLayout( Layout5 );

  m_homePageList->addColumn("");

  m_currentItem = -1;   // none selected

  connect(m_homePageList, SIGNAL(selectionChanged(QListViewItem*)),
          this, SLOT(slotSelectHomePageItem(QListViewItem *)));

  connect(m_upButton, SIGNAL(clicked()), this, SLOT(slotMoveUp()));          
  connect(m_downButton, SIGNAL(clicked()), this, SLOT(slotMoveDown()));
}

/** Called to create the Main List page shown in the dialog.
**/
void KSettingsDlg::setPageList()
{
  // Create the page.
  QVBox *qvboxMainFrame = addVBoxPage( i18n("Register"), i18n("Register settings"),
    DesktopIcon("ledger"));
//    locate("appdata", "pics/setting_list.png"));

  // Create the tab widget
  QTabWidget *qtabwidget = new QTabWidget(qvboxMainFrame, "TabWidget2");

  // Create the first page
  QWidget *qwidgetPage = new QWidget(this);

  // Create the layout for the page
  QVBoxLayout *qvboxlayoutPage = new QVBoxLayout(qwidgetPage);
  qvboxlayoutPage->setSpacing( 6 );
  qvboxlayoutPage->setMargin( 11 );

/*
  // Create a horizontal layout to hold two widgets
  QHBoxLayout *qhboxlayout = new QHBoxLayout;
  qhboxlayout->setSpacing( 6 );
  qhboxlayout->setMargin( 0 );

  // Create the first widget
  QLabel *qlabel = new QLabel(i18n("Number of lines in the register view:"), qwidgetPage);
  qhboxlayout->addWidget(qlabel);

  // Create the second widget
  m_klineeditRowCount = new KLineEdit(qwidgetPage);
  QIntValidator *qintvalidator = new QIntValidator(1, 3, m_klineeditRowCount);
  m_klineeditRowCount->setValidator(qintvalidator);
  qhboxlayout->addWidget(m_klineeditRowCount);

  // Add the horizontal layout
  qvboxlayoutPage->addLayout(qhboxlayout);
*/

  // Ceate another widget
  m_qcheckboxShowGrid = new QCheckBox(i18n("Show a grid in the register."), qwidgetPage);
  qvboxlayoutPage->addWidget(m_qcheckboxShowGrid);

/*
  // and another widget
  m_qcheckboxTextPrompt = new QCheckBox(i18n("Show a textual prompt in the register."), qwidgetPage);
  qvboxlayoutPage->addWidget(m_qcheckboxTextPrompt);
*/

  // and one more
  m_qcheckboxLedgerLens = new QCheckBox(i18n("Use the ledger lens"), qwidgetPage);
  qvboxlayoutPage->addWidget(m_qcheckboxLedgerLens);

  // Setting for transaction entry form
  m_qcheckboxTransactionForm = new QCheckBox(i18n("Show transaction form"), qwidgetPage);
  qvboxlayoutPage->addWidget(m_qcheckboxTransactionForm);

  // In some transaction forms, no Nr. field is shown. This
  // switch allows to override this feature.
  m_qcheckboxShowNrField = new QCheckBox(i18n("Always show a Nr. field in transaction form"), qwidgetPage);
  qvboxlayoutPage->addWidget(m_qcheckboxShowNrField);
  connect(m_qcheckboxShowNrField, SIGNAL(toggled(bool)),
    this, SLOT(slotNrFieldToggled(bool)));

  // Setting for switch to copy transaction type into Nr. field upon
  // new transactions
  m_qcheckboxTypeToNr = new QCheckBox(i18n("Insert transaction type into Nr. field for new transactions"), qwidgetPage);
  qvboxlayoutPage->addWidget(m_qcheckboxTypeToNr);

#if 0
  // Create a group to hold two radio buttons
  QButtonGroup *qbuttongroup = new QButtonGroup(qwidgetPage, "ButtonGroup1");
  qbuttongroup->setTitle(i18n("Row Colour options"));
  qbuttongroup->setColumnLayout(0, Qt::Vertical );
  qbuttongroup->layout()->setSpacing( 0 );
  qbuttongroup->layout()->setMargin( 0 );

  // Create a layout
  QVBoxLayout *qvboxlayout = new QVBoxLayout(qbuttongroup->layout());
  qvboxlayout->setAlignment( Qt::AlignTop );
  qvboxlayout->setSpacing( 6 );
  qvboxlayout->setMargin( 11 );

  // Add the first radio button
  m_qradiobuttonPerTransaction = new QRadioButton(qbuttongroup, "m_per_trans");
  m_qradiobuttonPerTransaction->setText( i18n("Use one colour per transaction") );
  qvboxlayout->addWidget(m_qradiobuttonPerTransaction);

  // Add the second radio button
  m_qradiobuttonOtherRow = new QRadioButton(qbuttongroup, "m_every_other");
  m_qradiobuttonOtherRow->setText( i18n( "Change colour every other row" ) );
  qvboxlayout->addWidget(m_qradiobuttonOtherRow);
  
  qvboxlayoutPage->addWidget(qbuttongroup);
#endif

  // Add a vertical spacer to take up the remaining available space
  QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
  qvboxlayoutPage->addItem( spacer );

  // Add the page to the tab
  qtabwidget->insertTab(qwidgetPage, i18n("General"));






  // Create a new tab for the filter options
  QWidget *qwidgetFilter = new QWidget(qtabwidget, "filterTab");

  // Create a vertical layout
  QVBoxLayout *qvboxlayoutFilter = new QVBoxLayout(qwidgetFilter);
  qvboxlayoutFilter->setSpacing( 6 );
  qvboxlayoutFilter->setMargin( 11 );

  // Create a group to hold two radio buttons
  QButtonGroup *qbuttongroupDates = new QButtonGroup(qwidgetFilter, "ButtonGroup1");
  qbuttongroupDates->setTitle(i18n("Restrict by date"));
  qbuttongroupDates->setColumnLayout(0, Qt::Vertical );
  qbuttongroupDates->layout()->setSpacing( 0 );
  qbuttongroupDates->layout()->setMargin( 0 );

  QHBoxLayout *qhboxlayout2 = new QHBoxLayout(qbuttongroupDates->layout());
  qhboxlayout2->setAlignment( Qt::AlignTop );
  qhboxlayout2->setSpacing( 6 );
  qhboxlayout2->setMargin( 11 );

  m_dateinputStart = new kMyMoneyDateInput(qbuttongroupDates, "dateinput");
  QLabel *m_qlabelStartDate = new QLabel(i18n("Start Date: "), qbuttongroupDates);
  qhboxlayout2->addWidget(m_qlabelStartDate);
  qhboxlayout2->addWidget(m_dateinputStart);
  qvboxlayoutFilter->addWidget(qbuttongroupDates);

  // Add a vertical spacer to take up the remaining available space
  spacer = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
  qvboxlayoutFilter->addItem( spacer );

  // Add the page to the tab
  qtabwidget->insertTab(qwidgetFilter, i18n("Filter"));









  // Create a new tab for the colour options
  QWidget *qwidgetColour = new QWidget(qtabwidget, "colorTab");

  // Create a vertical layout
  QVBoxLayout *qvboxlayoutColour = new QVBoxLayout(qwidgetColour);
  qvboxlayoutColour->setSpacing( 6 );
  qvboxlayoutColour->setMargin( 11 );

  // Create a horizontal layout to include the label and button
  QHBoxLayout *qhboxlayoutColour = new QHBoxLayout;
  qhboxlayoutColour->setSpacing( 6 );
  qhboxlayoutColour->setMargin( 0 );

  // Add the label and button
  QLabel *qlabelListColour = new QLabel(i18n( "List view colour :" ), qwidgetColour);
  qhboxlayoutColour->addWidget(qlabelListColour);
  m_kcolorbuttonList = new KColorButton(qwidgetColour, "colour_list");
  qhboxlayoutColour->addWidget(m_kcolorbuttonList);

  // Add the horizontal layout
  qvboxlayoutColour->addLayout(qhboxlayoutColour);

  // Create another horizontal layout to include the label and button
  QHBoxLayout *qhboxlayoutBGColour = new QHBoxLayout;
  qhboxlayoutBGColour->setSpacing( 6 );
  qhboxlayoutBGColour->setMargin( 0 );

  // Add the label and button
  QLabel *qlabelListBGColour = new QLabel(i18n( "List background colour :" ), qwidgetColour);
  qhboxlayoutBGColour->addWidget(qlabelListBGColour);
  m_kcolorbuttonBack = new KColorButton(qwidgetColour, "colour_back");
  qhboxlayoutBGColour->addWidget(m_kcolorbuttonBack);

  // Add the horizontal layout
  qvboxlayoutColour->addLayout(qhboxlayoutBGColour);

  // Create another horizontal layout to include the label and button
  QHBoxLayout *qhboxlayoutGridColour = new QHBoxLayout;
  qhboxlayoutBGColour->setSpacing( 6 );
  qhboxlayoutBGColour->setMargin( 0 );

  // Add the label and button
  QLabel *qlabelListGridColour = new QLabel(i18n( "List grid colour :" ), qwidgetColour);
  qhboxlayoutGridColour->addWidget(qlabelListGridColour);
  m_kcolorbuttonGrid = new KColorButton(qwidgetColour, "colour_grid");
  qhboxlayoutGridColour->addWidget(m_kcolorbuttonGrid);

  // Add the horizontal layout
  qvboxlayoutColour->addLayout(qhboxlayoutGridColour);


  // Add a vertical spacer to take up the remaining available space
  QSpacerItem* qspaceritemColour = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
  qvboxlayoutColour->addItem( qspaceritemColour );

  // Add the page to the tab widget
  qtabwidget->insertTab(qwidgetColour, i18n( "Color"));

  // Create another tab adding a font chooser widget
  QVBox *qvboxInsideTab1 = new QVBox( this, "tab1" );
  qvboxInsideTab1->setSpacing( 6 );
  qvboxInsideTab1->setMargin( 11 );
  m_kfontchooserHeader = new KFontChooser(qvboxInsideTab1);
  qtabwidget->insertTab(qvboxInsideTab1, i18n("Header Font"));

  // Create another tab adding a font chooser widget
  QVBox *qvboxInsideTab2 = new QVBox( this, "tab2" );
  qvboxInsideTab2->setSpacing( 6 );
  qvboxInsideTab2->setMargin( 11 );
  m_kfontchooserCell = new KFontChooser(qvboxInsideTab2);
  qtabwidget->addTab(qvboxInsideTab2, i18n("Cell Font"));
}

/** Read all the settings in from the global KConfig object and set all the
  * widgets appropriately.
**/
void KSettingsDlg::configRead()
{
  KConfig *kconfig = KGlobal::config();
  kconfig->setGroup("Settings Dialog");
  QSize qsizeDefaultSize(470,470);
  this->resize(kconfig->readSizeEntry("Geometry", &qsizeDefaultSize));

  kconfig->setGroup("General Options");
  m_bTempStartPrompt = kconfig->readBoolEntry("StartDialog", true);
  m_qradiobuttonStartPrompt->setChecked(m_bTempStartPrompt);
  m_qradiobuttonStartFile->setChecked(!m_bTempStartPrompt);

  m_bTempStartPage = kconfig->readBoolEntry("StartLastViewSelected", false);
  m_qradiobuttonStartHome->setChecked(!m_bTempStartPage);
  m_qradiobuttonStartLast->setChecked(m_bTempStartPage);
  
  m_bTempAccountWizard = kconfig->readBoolEntry("NewAccountWizard", true);
  m_qradiobuttonAccountWizard->setChecked(m_bTempAccountWizard);
  m_qradiobuttonAccountDialog->setChecked(!m_bTempAccountWizard);

  m_bTempLedgerLens = kconfig->readBoolEntry("LedgerLens", true);
  m_qcheckboxLedgerLens->setChecked(m_bTempLedgerLens);

  m_bTempTransactionForm = kconfig->readBoolEntry("TransactionForm", true);
  m_qcheckboxTransactionForm->setChecked(m_bTempTransactionForm);

  m_bTempShowNrField = kconfig->readBoolEntry("AlwaysShowNrField", false);
  m_qcheckboxShowNrField->setChecked(m_bTempShowNrField);

  if(m_bTempShowNrField == true) {
    m_bTempTypeToNr = kconfig->readBoolEntry("CopyTypeToNr", false);
    m_qcheckboxTypeToNr->setChecked(m_bTempTypeToNr);
    m_qcheckboxTypeToNr->setEnabled(true);
  } else {
    m_bTempTypeToNr = false;
    m_qcheckboxTypeToNr->setChecked(m_bTempTypeToNr);
    m_qcheckboxTypeToNr->setEnabled(false);
  }

  kconfig->setGroup("List Options");

  QFont qfontDefault = QFont("helvetica", 12);
  QColor qcolorDefault = Qt::white;
  QColor qcolorDefaultBG = Qt::gray;
  QColor qcolorDefaultGrid = Qt::black;

  m_qcolorTempList = kconfig->readColorEntry("listColor", &qcolorDefault);
  m_kcolorbuttonList->setColor(m_qcolorTempList);

  m_qcolorTempListBG = kconfig->readColorEntry("listBGColor", &qcolorDefaultBG);
  m_kcolorbuttonBack->setColor(m_qcolorTempListBG);

  m_qcolorTempListGrid = kconfig->readColorEntry("listGridColor", &qcolorDefaultGrid);
  m_kcolorbuttonGrid->setColor(m_qcolorTempListGrid);

  m_qfontTempHeader = kconfig->readFontEntry("listHeaderFont", &qfontDefault);
  m_kfontchooserHeader->setFont(m_qfontTempHeader);

  m_qfontTempCell = kconfig->readFontEntry("listCellFont", &qfontDefault);
  m_kfontchooserCell->setFont(m_qfontTempCell);

/*
  m_qstringTempRowCount = kconfig->readEntry("RowCount", "2");
  m_klineeditRowCount->setText(m_qstringTempRowCount);
*/

  m_bTempShowGrid = kconfig->readBoolEntry("ShowGrid", true);
  m_qcheckboxShowGrid->setChecked(m_bTempShowGrid);

  m_bTempHideCategory = kconfig->readBoolEntry("HideUnusedCategory", false);
  m_qcheckboxHideCategory->setChecked(m_bTempHideCategory);

#if 0  
  m_bTempColourPerTransaction = kconfig->readBoolEntry("ColourPerTransaction", true);
  m_qradiobuttonPerTransaction->setChecked(m_bTempColourPerTransaction);
  m_qradiobuttonOtherRow->setChecked(!m_bTempColourPerTransaction);
#endif

  QDateTime defaultDate;
  defaultDate.setTime_t(0);

  m_qdateTempStart = kconfig->readDateTimeEntry("StartDate", &defaultDate).date();
  m_dateinputStart->setDate(m_qdateTempStart);

  m_bTempNormalView = kconfig->readBoolEntry("NormalAccountsView", false);
  m_qradiobuttonNormalView->setChecked(m_bTempNormalView);
  m_qradiobuttonAccountView->setChecked(!m_bTempNormalView);

  kconfig->setGroup("Homepage Options");
  m_tempHomePageItems = kconfig->readListEntry("Itemlist");
  QStringList list = m_tempHomePageItems;
  KMyMoneyUtils::addDefaultHomePageItems(list);
  fillHomePageItems(list);
}

/** Write out all the settings to the global KConfig object.
**/
void KSettingsDlg::configWrite()
{
  KConfig *kconfig = KGlobal::config();
  kconfig->setGroup("Settings Dialog");
  kconfig->writeEntry("Geometry", this->size() );

  kconfig->setGroup("List Options");
  kconfig->writeEntry("listColor", m_kcolorbuttonList->color());
  kconfig->writeEntry("listBGColor", m_kcolorbuttonBack->color());
  kconfig->writeEntry("listGridColor", m_kcolorbuttonGrid->color());
  kconfig->writeEntry("listHeaderFont", m_kfontchooserHeader->font());
  kconfig->writeEntry("listCellFont", m_kfontchooserCell->font());
  // kconfig->writeEntry("RowCount", m_klineeditRowCount->text());
  kconfig->writeEntry("ShowGrid", m_qcheckboxShowGrid->isChecked());
  // kconfig->writeEntry("ColourPerTransaction", m_qradiobuttonPerTransaction->isChecked());
  kconfig->writeEntry("HideUnusedCategory", m_qcheckboxHideCategory->isChecked());
#if QT_VERSION > 300
  kconfig->writeEntry("StartDate", QDateTime(m_dateinputStart->getQDate()));
#else
  kconfig->writeEntry("StartDate", m_dateinputStart->getQDate());
#endif

  kconfig->writeEntry("NormalAccountsView", m_qradiobuttonNormalView->isChecked());

  kconfig->setGroup("General Options");
  kconfig->writeEntry("StartDialog", m_qradiobuttonStartPrompt->isChecked());
  kconfig->writeEntry("StartLastViewSelected", m_qradiobuttonStartLast->isChecked());
  kconfig->writeEntry("NewAccountWizard", m_qradiobuttonAccountWizard->isChecked());
  kconfig->writeEntry("LedgerLens", m_qcheckboxLedgerLens->isChecked());
  kconfig->writeEntry("TransactionForm", m_qcheckboxTransactionForm->isChecked());
  kconfig->writeEntry("CopyTypeToNr", m_qcheckboxTypeToNr->isChecked());
  kconfig->writeEntry("AlwaysShowNrField", m_qcheckboxShowNrField->isChecked());

  kconfig->setGroup("Homepage Options");
  kconfig->writeEntry("Itemlist", homePageItems());

  kconfig->sync();
}

/** Called on OK being pressed */
void KSettingsDlg::slotOk()
{
/*
  int nCount = m_klineeditRowCount->text().toInt();
  if (nCount <= 0 || nCount >= 4) {
    KMessageBox::information(this, i18n("The row count has to be between 1 and 3"));
    m_klineeditRowCount->setFocus();
    return;
  }
*/
  configWrite();
  this->accept();
}

/** Called on Apply being pressed */
void KSettingsDlg::slotApply()
{
/*
  int nCount = m_klineeditRowCount->text().toInt();
  if (nCount <= 0 || nCount >= 4) {
    KMessageBox::information(this, i18n("The row count has to be between 1 and 3"));
    m_klineeditRowCount->setFocus();
    return;
  }
*/
  m_bDoneApply = true;
  configWrite();
  emit signalApply();
}

/** Called on Cancel being pressed.
  * It writes out all the original settings read when it was created.
**/
void KSettingsDlg::slotCancel()
{
  // make sure the config object is the same as we left it
  KConfig *kconfig = KGlobal::config();
  kconfig->setGroup("List Options");
  kconfig->writeEntry("listColor", m_qcolorTempList);
  kconfig->writeEntry("listBGColor", m_qcolorTempListBG);
  kconfig->writeEntry("listHeaderFont", m_qfontTempHeader);
  kconfig->writeEntry("listCellFont", m_qfontTempCell);
  // kconfig->writeEntry("RowCount", m_qstringTempRowCount);
  kconfig->writeEntry("ShowGrid", m_bTempShowGrid);
  // kconfig->writeEntry("ColourPerTransaction", m_bTempColourPerTransaction);
  kconfig->writeEntry("HideUnusedCategory", m_bTempHideCategory);
#if QT_VERSION > 300
  kconfig->writeEntry("StartDate", QDateTime(m_qdateTempStart));
#else
  kconfig->writeEntry("StartDate", m_qdateTempStart);
#endif

  kconfig->writeEntry("NormalAccountsView", m_bTempNormalView);

  kconfig->setGroup("General Options");
  kconfig->writeEntry("StartDialog", m_bTempStartPrompt);
  kconfig->writeEntry("StartLastViewSelected", m_bTempStartPage);
  kconfig->writeEntry("NewAccountWizard", m_bTempAccountWizard);
  kconfig->writeEntry("LedgerLens", m_bTempLedgerLens);
  kconfig->writeEntry("TransactionForm", m_bTempTransactionForm);
  kconfig->writeEntry("CopyTypeToNr", m_bTempTypeToNr);
  kconfig->writeEntry("AlwaysShowNrField", m_bTempShowNrField);

  kconfig->setGroup("Homepage Options");
  kconfig->writeEntry("Itemlist", m_tempHomePageItems);
  kconfig->sync();

  if (m_bDoneApply)
    accept();
  else
    reject();
}

/** Called when the user presses the User1 button.  In our case that is the
  * reset button.
  *
  * It just resets all the attributes to the values read on creation.
**/
void KSettingsDlg::slotUser1()
{
  m_qradiobuttonStartPrompt->setChecked(m_bTempStartPrompt);
  m_qradiobuttonStartFile->setChecked(!m_bTempStartPrompt);
  m_kcolorbuttonList->setColor(m_qcolorTempList);
  m_kcolorbuttonBack->setColor(m_qcolorTempListBG);
  m_kfontchooserHeader->setFont(m_qfontTempHeader);
  m_kfontchooserCell->setFont(m_qfontTempCell);
  // m_klineeditRowCount->setText(m_qstringTempRowCount);
  m_qcheckboxShowGrid->setChecked(m_bTempShowGrid);
#if 0  
  m_qradiobuttonPerTransaction->setChecked(m_bTempColourPerTransaction);
  m_qradiobuttonOtherRow->setChecked(!m_bTempColourPerTransaction);
#endif  
  m_qcheckboxHideCategory->setChecked(m_bTempHideCategory);
  m_dateinputStart->setDate(m_qdateTempStart);
  m_qradiobuttonNormalView->setChecked(m_bTempNormalView);
  m_qradiobuttonAccountView->setChecked(!m_bTempNormalView);
  m_qradiobuttonAccountWizard->setChecked(m_bTempAccountWizard);
  m_qradiobuttonAccountDialog->setChecked(!m_bTempAccountWizard);
  m_qcheckboxLedgerLens->setChecked(m_bTempLedgerLens);
  m_qcheckboxTransactionForm->setChecked(m_bTempTransactionForm);
  m_qcheckboxTypeToNr->setChecked(m_bTempTypeToNr);
  m_qcheckboxShowNrField->setChecked(m_bTempShowNrField);
  m_qradiobuttonStartLast->setChecked(m_bTempStartPage);
  m_qradiobuttonStartHome->setChecked(!m_bTempStartPage);
  
  QStringList list = m_tempHomePageItems;
  KMyMoneyUtils::addDefaultHomePageItems(list);
  fillHomePageItems(list);
}

void KSettingsDlg::slotNrFieldToggled(bool state)
{
  if(state == true) {
    m_qcheckboxTypeToNr->setEnabled(true);
  } else {
    m_qcheckboxTypeToNr->setChecked(false);
    m_qcheckboxTypeToNr->setEnabled(false);
  }
}

const QStringList KSettingsDlg::homePageItems(void) const
{
  QStringList list;
  QListViewItem *it;

  for(it = m_homePageList->firstChild(); it; it = it->nextSibling()) {
    int item = KMyMoneyUtils::stringToHomePageItem(it->text(0));
    if(!(static_cast<QCheckListItem*>(it)->isOn()))
      item = -item;
    list << QString::number(item);
  }
  return list;
}

void KSettingsDlg::fillHomePageItems(QStringList& list)
{
  QStringList::ConstIterator it;
  int w = 0;
  m_homePageList->clear();
  QCheckListItem *sel = 0;
    
  m_upButton->setEnabled(false);
  m_downButton->setEnabled(false);

  QFontMetrics fm( KGlobalSettings::generalFont());
  QCheckListItem* last = 0;

  for(it = list.begin(); it != list.end(); ++it) {
    int idx = (*it).toInt();
    bool enabled = idx > 0;
    if(!enabled) idx = -idx;
    QCheckListItem* item = new QCheckListItem(m_homePageList, KMyMoneyUtils::homePageItemToString(idx), QCheckListItem::CheckBox);
    if(last)
      item->moveItem(last);

    // qDebug("Adding %s", item->text(0).latin1());
    item->setOn(enabled);
    if(item->width(fm, m_homePageList, 0) > w)
      w = item->width(fm, m_homePageList, 0);
      
    if(sel == 0)
      sel = item;
    last = item;
  }

  if(sel) {
    m_homePageList->setSelected(sel, true);
    slotSelectHomePageItem(sel);
  }

  m_homePageList->setMinimumWidth(w+30);
  m_homePageList->setMaximumWidth(w+30);
  m_homePageList->setColumnWidth(0, w+28);
}

void KSettingsDlg::slotSelectHomePageItem(QListViewItem *item)
{
  m_upButton->setEnabled(m_homePageList->firstChild() != item);
  m_downButton->setEnabled(item->nextSibling());  
}

void KSettingsDlg::slotMoveUp(void)
{
  QListViewItem *item = m_homePageList->currentItem();
  QListViewItem *prev = item->itemAbove();
  if(prev) {
    prev->moveItem(item);
    slotSelectHomePageItem(item);
  }
}

void KSettingsDlg::slotMoveDown(void)
{
  QListViewItem *item = m_homePageList->currentItem();
  QListViewItem *next = item->nextSibling();
  if(next) {
    item->moveItem(next);
    slotSelectHomePageItem(item);
  }
}
