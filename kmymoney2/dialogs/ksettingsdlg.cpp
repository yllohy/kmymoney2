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
#include <qlayout.h>
#include <qvbox.h>
#include <qlabel.h>
#include <qgroupbox.h>
#include <qtabwidget.h>
#include <qvalidator.h>

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

// ----------------------------------------------------------------------------
// Project Includes
#include "ksettingsdlg.h"

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
  configRead();
  m_bDoneApply=false;
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

  // Create a group box to hold the available options
  QButtonGroup *qbuttongroup = new QButtonGroup(qvboxMainFrame, "GroupBox1");
  qbuttongroup->setTitle( i18n( "Startup options" ) );
  qbuttongroup->setColumnLayout(0, Qt::Vertical );
  qbuttongroup->layout()->setSpacing( 0 );
  qbuttongroup->layout()->setMargin( 0 );

  // Create a layout to organize the widgets.
  QVBoxLayout *qvboxlayout = new QVBoxLayout(qbuttongroup->layout());
  qvboxlayout->setAlignment( Qt::AlignTop );
  qvboxlayout->setSpacing( 6 );
  qvboxlayout->setMargin( 11 );

  // Create a check box to be in the group box
  m_qradiobuttonStartPrompt = new QRadioButton("start_prompt", qbuttongroup);
  m_qradiobuttonStartPrompt->setText( i18n( "Start with dialog prompt (default)" ) );
  qvboxlayout->addWidget(m_qradiobuttonStartPrompt);

  // Create another check box to the group box
  m_qradiobuttonStartFile = new QRadioButton("start_file", qbuttongroup);
  m_qradiobuttonStartFile->setText( i18n( "Start with last file used" ) );
  qvboxlayout->addWidget(m_qradiobuttonStartFile);
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

  // Create a group to hold two radio buttons
  QButtonGroup *qbuttongroupDates = new QButtonGroup(qwidgetPage, "ButtonGroup1");
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
  qvboxlayoutPage->addWidget(qbuttongroupDates);


  // Add a vertical spacer to take up the remaining available space
  QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
  qvboxlayoutPage->addItem( spacer );

  // Add the page to the tab
  qtabwidget->insertTab(qwidgetPage, i18n("General"));

  // Create a new tab for the colour options
  QWidget *qwidgetColour = new QWidget(qtabwidget, "tab");

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
  QSize *qsizeDefaultSize = new QSize(470,470);
  this->resize(kconfig->readSizeEntry("Geometry", qsizeDefaultSize));

  kconfig->setGroup("General Options");
  m_bTempStartPrompt = kconfig->readBoolEntry("StartDialog", true);
  m_qradiobuttonStartPrompt->setChecked(m_bTempStartPrompt);
  m_qradiobuttonStartFile->setChecked(!m_bTempStartPrompt);

  m_bTempAccountWizard = kconfig->readBoolEntry("NewAccountWizard", true);
  m_qradiobuttonAccountWizard->setChecked(m_bTempAccountWizard);
  m_qradiobuttonAccountDialog->setChecked(!m_bTempAccountWizard);

  m_bTempLedgerLens = kconfig->readBoolEntry("LedgerLens", true);
  m_qcheckboxLedgerLens->setChecked(m_bTempLedgerLens);

  m_bTempTransactionForm = kconfig->readBoolEntry("TransactionForm", true);
  m_qcheckboxTransactionForm->setChecked(m_bTempTransactionForm);

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

/*
  m_bTempTextPrompt = kconfig->readBoolEntry("TextPrompt", true);
  m_qcheckboxTextPrompt->setChecked(m_bTempTextPrompt);
*/

  m_bTempColourPerTransaction = kconfig->readBoolEntry("ColourPerTransaction", true);
  m_qradiobuttonPerTransaction->setChecked(m_bTempColourPerTransaction);
  m_qradiobuttonOtherRow->setChecked(!m_bTempColourPerTransaction);

  QDateTime defaultDate;
  defaultDate.setTime_t(0);

  m_qdateTempStart = kconfig->readDateTimeEntry("StartDate", &defaultDate).date();
  m_dateinputStart->setDate(m_qdateTempStart);

  m_bTempNormalView = kconfig->readBoolEntry("NormalAccountsView", true);
  m_qradiobuttonNormalView->setChecked(m_bTempNormalView);
  m_qradiobuttonAccountView->setChecked(!m_bTempNormalView);
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
  // kconfig->writeEntry("ShowGrid", m_qcheckboxShowGrid->isChecked());
  kconfig->writeEntry("ColourPerTransaction", m_qradiobuttonPerTransaction->isChecked());
  // kconfig->writeEntry("TextPrompt", m_qcheckboxTextPrompt->isChecked());
#if QT_VERSION > 300
  kconfig->writeEntry("StartDate", QDateTime(m_dateinputStart->getQDate()));
#else
  kconfig->writeEntry("StartDate", m_dateinputStart->getQDate());
#endif

  kconfig->writeEntry("NormalAccountsView", m_qradiobuttonNormalView->isChecked());

  kconfig->setGroup("General Options");
  kconfig->writeEntry("StartDialog", m_qradiobuttonStartPrompt->isChecked());
  kconfig->writeEntry("NewAccountWizard", m_qradiobuttonAccountWizard->isChecked());
  kconfig->writeEntry("LedgerLens", m_qcheckboxLedgerLens->isChecked());
  kconfig->writeEntry("TransactionForm", m_qcheckboxTransactionForm->isChecked());
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
  kconfig->writeEntry("ColourPerTransaction", m_bTempColourPerTransaction);
  kconfig->writeEntry("TextPrompt", m_bTempTextPrompt);
#if QT_VERSION > 300
  kconfig->writeEntry("StartDate", QDateTime(m_qdateTempStart));
#else
  kconfig->writeEntry("StartDate", m_qdateTempStart);
#endif

  kconfig->writeEntry("NormalAccountsView", m_bTempNormalView);

  kconfig->setGroup("General Options");
  kconfig->writeEntry("StartDialog", m_bTempStartPrompt);
  kconfig->writeEntry("NewAccountWizard", m_bTempAccountWizard);
  kconfig->writeEntry("LedgerLens", m_bTempLedgerLens);
  kconfig->writeEntry("TransactionForm", m_bTempTransactionForm);

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
  m_kcolorbuttonList->setColor(m_qcolorTempList);
  m_kcolorbuttonBack->setColor(m_qcolorTempListBG);
  m_kfontchooserHeader->setFont(m_qfontTempHeader);
  m_kfontchooserCell->setFont(m_qfontTempCell);
  // m_klineeditRowCount->setText(m_qstringTempRowCount);
  m_qcheckboxShowGrid->setChecked(m_bTempShowGrid);
  m_qradiobuttonPerTransaction->setChecked(m_bTempColourPerTransaction);
  m_qradiobuttonOtherRow->setChecked(!m_bTempColourPerTransaction);
  // m_qcheckboxTextPrompt->setChecked(m_bTempTextPrompt);
  m_dateinputStart->setDate(m_qdateTempStart);
  m_qradiobuttonNormalView->setChecked(m_bTempNormalView);
  m_qradiobuttonAccountView->setChecked(!m_bTempNormalView);
  m_qradiobuttonAccountWizard->setChecked(m_bTempAccountWizard);
  m_qradiobuttonAccountDialog->setChecked(!m_bTempAccountWizard);
  m_qcheckboxLedgerLens->setChecked(m_bTempLedgerLens);
  m_qcheckboxTransactionForm->setChecked(m_bTempTransactionForm);
}
