/***************************************************************************
                          kreportconfigurationdlg.cpp  -  description
                             -------------------
    begin                : Mon Jun 21 2004
    copyright            : (C) 2000-2004 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
                           Ace Jones <ace.j@hotpop.com>
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
#include <qvariant.h>
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qframe.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

// ----------------------------------------------------------------------------
// KDE Includes
#include <kpushbutton.h>
#include <klineedit.h>
#include <klistview.h>
#include <kcombobox.h>
#include <kguiitem.h>
#include <kiconloader.h>
#include <kdebug.h>

// ----------------------------------------------------------------------------
// Project Includes
#include "kreportconfigurationfilterdlg.h"
#include "../widgets/kmymoneyregistersearch.h"
#include "../widgets/kmymoneydateinput.h"
#include "../widgets/kmymoneyedit.h"
#include "../widgets/kmymoneylineedit.h"
#include "../widgets/kmymoneyaccountselector.h"
#include "../mymoney/mymoneyfile.h"
#include "../mymoney/storage/imymoneystorage.h"
#include "../mymoney/mymoneyreport.h"

KReportConfigurationFilterDlg::KReportConfigurationFilterDlg(
  MyMoneyReport report, QWidget *parent, const char *name)
 : KFindTransactionDlg(parent, name),
 m_initialState(report),
 m_currentState(report)
{

    setCaption( tr2i18n( "Report Configuration" ) );
    TextLabel1->setText( tr2i18n( "Choose which transactions to display" ) );

    m_reportFrame = new QFrame( this, "m_reportFrame" );
    m_reportFrame->setFrameShape( QFrame::StyledPanel );
    m_reportFrame->setFrameShadow( QFrame::Raised );

    m_reportLayout = new QGridLayout( m_reportFrame, 1, 1, 10, 6, "m_reportLayout");

    m_searchButton->setText( tr2i18n( "OK" ) );

    delete m_closeButton;

    //
    // bgrpShow
    //

    bgrpShow = new QButtonGroup( m_reportFrame, "bgrpShow" );
    bgrpShow->setTitle( tr2i18n( "Show" ) );

    reportLayout74 = new QVBoxLayout( bgrpShow, 20, 6, "reportLayout74");

    radioCategoriesTop = new QRadioButton( bgrpShow, "radioCategoriesTop" );
    radioCategoriesTop->setText( tr2i18n( "&Top-Level Categories Only" ) );
    radioCategoriesTop->setChecked( TRUE );
    QWhatsThis::add( radioCategoriesTop, tr2i18n( "<b>Show: Top-Level Categories Only</b><br>Select this option to roll up sub-categories into their top-most category.  This will show one line for each top-level expense or income category, containing a subtotal of all its subcategories." ) );
    reportLayout74->addWidget( radioCategoriesTop );

    radioCategoriesAll = new QRadioButton( bgrpShow, "radioCategoriesAll" );
    radioCategoriesAll->setText( tr2i18n( "&All Categories" ) );
    QWhatsThis::add( radioCategoriesAll, tr2i18n( "<b>Show: All Categories</b>\n""<br>\n""Select this option to display one line in the report for each category.  For top-level categories with sub-categories, a subtotal line will also be displayed." ) );
    reportLayout74->addWidget( radioCategoriesAll );

    m_reportLayout->addWidget( bgrpShow, 2, 2 );

    //
    // labelREportName
    //

    reportLayout67 = new QHBoxLayout( 0, 0, 6, "reportLayout67");

    labelReportName = new QLabel( m_reportFrame, "labelReportName" );
    labelReportName->setText( tr2i18n( "Report Name" ) );
    QWhatsThis::add( labelReportName, tr2i18n( "<b>Report Name</b>\n""<br>\n""Change the name of this report to anything you like." ) );
    reportLayout67->addWidget( labelReportName );

    editReportname = new QLineEdit( m_reportFrame, "editReportname" );
    editReportname->setFocus();
    QWhatsThis::add( editReportname, tr2i18n( "<b>Report Name</b>\n""<br>\n""Change the name of this report to anything you like." ) );
    reportLayout67->addWidget( editReportname );

    m_reportLayout->addMultiCellLayout( reportLayout67, 0, 0, 0, 2 );

    //
    // labelREportName
    //

    reportLayout68 = new QHBoxLayout( 0, 0, 6, "reportLayout68");

    labelReportComment = new QLabel( m_reportFrame, "labelReportComment" );
    labelReportComment->setText( tr2i18n( "Comment" ) );
    QWhatsThis::add( labelReportComment, tr2i18n( "<b>Comment</b>\n""<br>\n""Information about the report to help you remember it." ) );
    reportLayout68->addWidget( labelReportComment );

    editReportComment = new QLineEdit( m_reportFrame, "editReportComment" );
    editReportComment->setFocus();
    QWhatsThis::add( editReportname, tr2i18n( "<b>Comment</b>\n""<br>\n""Information about the report to help you remember it." ) );
    reportLayout68->addWidget( editReportComment );

    m_reportLayout->addMultiCellLayout( reportLayout68, 1, 1, 0, 2 );

    //
    // bgrpRows
    //

    bgrpRows = new QButtonGroup( m_reportFrame, "bgrpRows" );
    bgrpRows->setTitle( tr2i18n( "Rows" ) );
    QWhatsThis::add( bgrpRows, tr2i18n( "<b>Rows</b>\n""<br>\n""Choose what is displayed in the rows of this report." ) );

    reportLayout75 = new QVBoxLayout( bgrpRows, 20, 6, "reportLayout75");

    radioRowsIE = new QRadioButton( bgrpRows, "radioRowsIE" );
    radioRowsIE->setText( tr2i18n( "&Income/Expenses" ) );
    radioRowsIE->setChecked( TRUE );
    QWhatsThis::add( radioRowsIE, tr2i18n( "<b>Income/Expenses</b>\n""<br>\n""Show only income and expense categories in this report." ) );
    reportLayout75->addWidget( radioRowsIE );

    radioRowsAL = new QRadioButton( bgrpRows, "radioRowsAL" );
    radioRowsAL->setText( tr2i18n( "Assets/&Liabilities" ) );
    QWhatsThis::add( radioRowsAL, tr2i18n( "<b>Assets/Liabilities</b>\n""<br>\n""Show only asset and liability accounts in this report." ) );
    reportLayout75->addWidget( radioRowsAL );

    m_reportLayout->addWidget( bgrpRows, 2, 1 );

    //
    // bgrpCurrency
    //

    bgrpCurrency = new QButtonGroup( m_reportFrame, "bgrpCurrency" );
    bgrpCurrency->setTitle( tr2i18n( "Currency" ) );

    reportLayout76 = new QVBoxLayout( bgrpCurrency, 20, 6, "reportLayout76");

    checkConvertCurrency = new QCheckBox( bgrpCurrency, "checkConvertCurrency" );
    // checkConvertCurrency->setEnabled( FALSE );
    // checkConvertCurrency->setGeometry( QRect( 10, 20, 190, 21 ) );
    checkConvertCurrency->setText( tr2i18n( "Convert to &Base Currency" ) );
    checkConvertCurrency->setChecked( TRUE );
    QWhatsThis::add( checkConvertCurrency, tr2i18n( "<b>Convert to Base Currency</b>\n""<br>\n""Choose whether all values should be converted to the file's base currency.  Column totals will only be shown if this option is checked.\n""<br><br>\n""This option is not yet implemented." ) );
    reportLayout76->addWidget( checkConvertCurrency );

    m_reportLayout->addMultiCellWidget( bgrpCurrency, 3, 3, 1, 2 );

    //
    // bgrpColumns
    //

    bgrpColumns = new QButtonGroup( m_reportFrame, "bgrpColumns" );
    bgrpColumns->setTitle( tr2i18n( "Columns" ) );
    QWhatsThis::add( bgrpColumns, tr2i18n( "<b>Columns</b>\n""<br>\n""Choose what range of data should be included in each column of this report." ) );
    bgrpColumns->setColumnLayout(0, Qt::Vertical );
    bgrpColumns->layout()->setSpacing( 6 );
    bgrpColumns->layout()->setMargin( 11 );
    bgrpColumnsLayout = new QHBoxLayout( bgrpColumns->layout() );
    bgrpColumnsLayout->setAlignment( Qt::AlignTop );

    reportLayout12 = new QVBoxLayout( 0, 0, 6, "reportLayout12");

    radioMonthCols = new QRadioButton( bgrpColumns, "radioMonthCols" );
    // radioMonthCols->setEnabled( FALSE );
    radioMonthCols->setText( tr2i18n( "&Monthly" ) );
    radioMonthCols->setChecked( TRUE );
    QToolTip::add( radioMonthCols, QString::null );
    QWhatsThis::add( radioMonthCols, tr2i18n( "<b>Monthly</b>\n""<br>\n""Sets the report to display one column for each month in the date range.\n""<br>\n""This option is not yet implemented." ) );
    reportLayout12->addWidget( radioMonthCols );

    radioBimonthCols = new QRadioButton( bgrpColumns, "radioBimonthCols" );
    // radioBimonthCols->setEnabled( FALSE );
    radioBimonthCols->setText( tr2i18n( "&Bi-Monthly" ) );
    QWhatsThis::add( radioBimonthCols, tr2i18n( "<b>Bi-Monthly</b>\n""<br>\n""Sets the report to display one column for every 2 months in the date range.\n""<br><br>\n""This option is not yet implemented." ) );
    reportLayout12->addWidget( radioBimonthCols );

    radioQuarterCols = new QRadioButton( bgrpColumns, "radioQuarterCols" );
    // radioQuarterCols->setEnabled( FALSE );
    radioQuarterCols->setText( tr2i18n( "&Quarterly" ) );
    QWhatsThis::add( radioQuarterCols, tr2i18n( "<b>Quarterly</b>\n""<br>\n""Sets the report to display one column for each quarter in the date range.\n""<br><br>\n""This option is not yet implemented." ) );
    reportLayout12->addWidget( radioQuarterCols );

    radioYearCols = new QRadioButton( bgrpColumns, "radioYearCols" );
    // radioYearCols->setEnabled( FALSE );
    radioYearCols->setText( tr2i18n( "&Yearly" ) );
    QWhatsThis::add( radioYearCols, tr2i18n( "<b>Yearly</b>\n""<br>\n""Sets the report to display one column for each year in the date range.\n""<br><br>\n""This option is not yet implemented." ) );
    reportLayout12->addWidget( radioYearCols );
    bgrpColumnsLayout->addLayout( reportLayout12 );

    m_reportLayout->addMultiCellWidget( bgrpColumns, 2, 3, 0, 0 );

    KFindTransactionDlgDeclLayout->insertWidget( 0, m_reportFrame );

    m_searchButton->disconnect();
    m_resetButton->disconnect();
    connect(m_searchButton, SIGNAL(clicked()), this, SLOT(slotSearch()));
    connect(m_resetButton, SIGNAL(clicked()), this, SLOT(slotReset()));
    slotReset();
}


KReportConfigurationFilterDlg::~KReportConfigurationFilterDlg()
{
}


void KReportConfigurationFilterDlg::slotSearch()
{
  // setup the filter from the dialog widgets
  setupFilter();

  // Copy the m_filter over to the filter part of m_currentConfig.
  m_currentState.assignFilter(m_filter);

  // Then extract the report properties
  m_currentState.setShowSubAccounts( radioCategoriesAll->isChecked() );
  m_currentState.setName( editReportname->text() );
  m_currentState.setComment( editReportComment->text() );
  m_currentState.setRowType( radioRowsAL->isChecked() ? MyMoneyReport::eAssetLiability : MyMoneyReport::eExpenseIncome );
  m_currentState.setConvertCurrency( checkConvertCurrency->isChecked() );

  if ( radioMonthCols->isChecked() )
    m_currentState.setColumnType( MyMoneyReport::eMonths );
  else if ( radioBimonthCols->isChecked() )
    m_currentState.setColumnType( MyMoneyReport::eBiMonths );
  else if ( radioQuarterCols->isChecked() )
    m_currentState.setColumnType( MyMoneyReport::eQuarters );
  else if ( radioYearCols->isChecked() )
    m_currentState.setColumnType( MyMoneyReport::eYears );

  done(true);
}

void KReportConfigurationFilterDlg::slotReset(void)
{
  //
  // Set up the widget from the initial filter
  //

  //
  // Report Properties
  //

  editReportname->setText( m_initialState.name() );
  editReportComment->setText( m_initialState.comment() );
  radioCategoriesAll->setChecked( m_initialState.isShowingSubAccounts() );
  checkConvertCurrency->setChecked( m_initialState.isConvertCurrency() );
  radioRowsAL->setChecked( m_initialState.rowType() == MyMoneyReport::eAssetLiability );
  radioRowsIE->setChecked( m_initialState.rowType() == MyMoneyReport::eExpenseIncome );
  radioMonthCols->setChecked( m_initialState.columnType() == MyMoneyReport::eMonths );
  radioBimonthCols->setChecked( m_initialState.columnType() == MyMoneyReport::eBiMonths );
  radioQuarterCols->setChecked( m_initialState.columnType() == MyMoneyReport::eQuarters );
  radioYearCols->setChecked( m_initialState.columnType() == MyMoneyReport::eYears );

  //
  // Text Filter
  //

  QRegExp textfilter;
  if ( m_initialState.textFilter(textfilter))
  {
    m_textEdit->setText(textfilter.pattern());
    m_caseSensitive->setChecked(textfilter.caseSensitive());
    m_regExp->setChecked(!textfilter.wildcard());
  }

  //
  // Type & State Filters
  //

  int type;
  if ( m_initialState.firstType(type) )
    m_typeBox->setCurrentItem(type);

  int state;
  if ( m_initialState.firstState(state) )
    m_stateBox->setCurrentItem(state);

  //
  // Number Filter
  //

  QString nrFrom, nrTo;
  if ( m_initialState.numberFilter(nrFrom, nrTo) )
  {
    if ( nrFrom == nrTo )
    {
      m_nrEdit->setEnabled(true);
      m_nrFromEdit->setEnabled(false);
      m_nrToEdit->setEnabled(false);
      m_nrEdit->setText(nrFrom);
      m_nrFromEdit->setText(QString());
      m_nrToEdit->setText(QString());
      m_nrButton->setChecked(true);
      m_nrRangeButton->setChecked(false);
    }
    else
    {
      m_nrEdit->setEnabled(false);
      m_nrFromEdit->setEnabled(true);
      m_nrToEdit->setEnabled(false);
      m_nrEdit->setText(QString());
      m_nrFromEdit->setText(nrFrom);
      m_nrToEdit->setText(nrTo);
      m_nrButton->setChecked(false);
      m_nrRangeButton->setChecked(true);
    }
  }
  else
  {
    m_nrEdit->setEnabled(true);
    m_nrFromEdit->setEnabled(false);
    m_nrToEdit->setEnabled(false);
    m_nrEdit->setText(QString());
    m_nrFromEdit->setText(QString());
    m_nrToEdit->setText(QString());
    m_nrButton->setChecked(true);
    m_nrRangeButton->setChecked(false);
  }

  //
  // Amount Filter
  //

  MyMoneyMoney from, to;
  if ( m_initialState.amountFilter(from,to) ) // bool getAmountFilter(MyMoneyMoney&,MyMoneyMoney&);
  {
    if ( from == to )
    {
      m_amountEdit->setEnabled(true);
      m_amountFromEdit->setEnabled(false);
      m_amountToEdit->setEnabled(false);
      m_amountEdit->loadText(QString::number(from.toDouble()));
      m_amountFromEdit->loadText(QString());
      m_amountToEdit->loadText(QString());
      m_amountButton->setChecked(true);
      m_amountRangeButton->setChecked(false);
    }
    else
    {
      m_amountEdit->setEnabled(false);
      m_amountFromEdit->setEnabled(true);
      m_amountToEdit->setEnabled(true);
      m_amountEdit->loadText(QString());
      m_amountFromEdit->loadText(QString::number(from.toDouble()));
      m_amountToEdit->loadText(QString::number(to.toDouble()));
      m_amountButton->setChecked(false);
      m_amountRangeButton->setChecked(true);
    }
  }
  else
  {
    m_amountEdit->setEnabled(true);
    m_amountFromEdit->setEnabled(false);
    m_amountToEdit->setEnabled(false);
    m_amountEdit->loadText(QString());
    m_amountFromEdit->loadText(QString());
    m_amountToEdit->loadText(QString());
    m_amountButton->setChecked(true);
    m_amountRangeButton->setChecked(false);
  }

  //
  // Payees Filter
  //

  QCStringList payees;
  if ( m_initialState.payees(payees) )
  {
    if ( payees.empty() )
    {
      m_emptyPayeesButton->setChecked(true);
    }
    else
    {
      selectAllItems(m_payeesView, false);
      selectItems(m_payeesView,payees,true);
    }
  }
  else
  {
    selectAllItems(m_payeesView, true);
  }

  //
  // Accounts Filter
  //

  QCStringList accounts;
  if ( m_initialState.accounts(accounts) )
  {
    m_accountsView->selectAllAccounts(false);
    m_accountsView->selectAccounts(accounts,true);
  }
  else
    m_accountsView->selectAllAccounts(true);

  //
  // Categories Filter
  //

  if ( m_initialState.categories(accounts) )
  {
    m_categoriesView->selectAllAccounts(false);
    m_categoriesView->selectAccounts(accounts,true);
  }
  else
    m_categoriesView->selectAllAccounts(true);

  //
  // Date Filter
  //

  // the following call implies a call to slotUpdateSelections,
  // that's why we call it last

  QDate dateFrom, dateTo;
  if ( m_initialState.dateFilter( dateFrom, dateTo ) )
  {
    m_dateRange->setCurrentItem(userDefined);
    m_fromDate->setDate(dateFrom);
    m_toDate->setDate(dateTo);
    slotDateChanged();
  }
  else
  {
    m_dateRange->setCurrentItem(allDates);
    slotDateRangeChanged(allDates);
  }

  QTimer::singleShot(0, this, SLOT(slotRightSize()));
}

#include "kreportconfigurationfilterdlg.moc"
