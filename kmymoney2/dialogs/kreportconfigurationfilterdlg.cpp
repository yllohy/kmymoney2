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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

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
#include <qtabwidget.h>
#include <qtextedit.h>
#include <qlayout.h>
#include <qapplication.h>

// ----------------------------------------------------------------------------
// KDE Includes
#include <kapplication.h>
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
#include "../widgets/kmymoneyreportconfigtab1decl.h"
#include "../widgets/kmymoneyreportconfigtab2decl.h"
#include "../widgets/kmymoneyreportconfigtab3decl.h"
#include "../widgets/kmymoneyreportconfigtabchartdecl.h"
#include "../mymoney/mymoneyfile.h"
#include "../mymoney/storage/imymoneystorage.h"
#include "../mymoney/mymoneyreport.h"

KReportConfigurationFilterDlg::KReportConfigurationFilterDlg(
  MyMoneyReport report, QWidget *parent, const char *name)
 : KFindTransactionDlg(parent, name),
 m_tab2(0),
 m_tab3(0),
 m_tabChart(0),
 m_initialState(report),
 m_currentState(report)
{
    //
    // Rework labelling
    //

    setCaption( tr2i18n( "Report Configuration" ) );
    delete TextLabel1;

    //
    // Rework the buttons
    //


    m_searchButton->setText( tr2i18n( "OK" ) );
    m_searchButton->disconnect();
    m_resetButton->disconnect();
    m_closeButton->disconnect();
    m_helpButton->disconnect();
    m_helpButton->show();
    connect(m_searchButton, SIGNAL(clicked()), this, SLOT(slotSearch()));
    connect(m_resetButton, SIGNAL(clicked()), this, SLOT(slotReset()));
    connect(m_closeButton, SIGNAL(clicked()), this, SLOT(reject()));
    connect(m_helpButton, SIGNAL(clicked()), this, SLOT(slotHelp()));

    //
    // Add new tabs
    //

    m_tab1 = new kMyMoneyReportConfigTab1Decl( m_criteriaTab, "kMyMoneyReportConfigTab1" );
    m_criteriaTab->insertTab( m_tab1, i18n("Report"), 0 );

    if ( m_initialState.reportType() == MyMoneyReport::ePivotTable )
    {
      m_tab2 = new kMyMoneyReportConfigTab2Decl( m_criteriaTab, "kMyMoneyReportConfigTab2" );
      m_criteriaTab->insertTab( m_tab2, i18n( "Rows/Columns"), 1 );

#ifdef HAVE_KDCHART
      m_tabChart = new kMyMoneyReportConfigTabChartDecl( m_criteriaTab, "kMyMoneyReportConfigTabChart" );
      m_criteriaTab->insertTab( m_tabChart, i18n( "Chart"), 2 );
#endif
    }
    else if ( m_initialState.reportType() == MyMoneyReport::eQueryTable )
    {
      // eInvestmentHoldings is a special-case report, and you cannot configure the
      // rows & columns of that report.
      if ( m_initialState.rowType() < MyMoneyReport::eAccountByTopAccount )
      {
        m_tab3 = new kMyMoneyReportConfigTab3Decl( m_criteriaTab, "kMyMoneyReportConfigTab3" );
        m_criteriaTab->insertTab( m_tab3, i18n("Rows/Columns"), 1 );
      }
    }

    m_criteriaTab->showPage( m_tab1 );
    m_criteriaTab->setMinimumSize( 500,200 );

    //
    // Now set up the widgets with proper values
    //

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
  m_currentState.setName( m_tab1->m_editName->text() );
  m_currentState.setComment( m_tab1->m_editComment->text() );
  m_currentState.setConvertCurrency( m_tab1->m_checkCurrency->isChecked() );
  m_currentState.setFavorite( m_tab1->m_checkFavorite->isChecked() );

  if ( m_tab2 )
  {
    MyMoneyReport::EDetailLevel dl[4] = { MyMoneyReport::eDetailAll, MyMoneyReport::eDetailTop, MyMoneyReport::eDetailGroup, MyMoneyReport::eDetailTotal };

    m_currentState.setDetailLevel( dl[m_tab2->m_comboDetail->currentItem()] );

    MyMoneyReport::ERowType rt[2] = { MyMoneyReport::eExpenseIncome, MyMoneyReport::eAssetLiability };
    m_currentState.setRowType( rt[m_tab2->m_comboRows->currentItem()] );

    MyMoneyReport::EColumnType ct[6] = { MyMoneyReport::eDays, MyMoneyReport::eWeeks, MyMoneyReport::eMonths, MyMoneyReport::eBiMonths, MyMoneyReport::eQuarters, MyMoneyReport::eYears };
    bool dy[6] = { true, true, false, false, false, false };
    m_currentState.setColumnType( ct[m_tab2->m_comboColumns->currentItem()] );
    
    //TODO (Ace) This should be implicit in the call above.  MMReport needs fixin'
    m_currentState.setColumnsAreDays( dy[m_tab2->m_comboColumns->currentItem()] );

    m_currentState.setIncludingSchedules( m_tab2->m_checkScheduled->isChecked() );
    
    m_currentState.setIncludingTransfers( m_tab2->m_checkTransfers->isChecked() );
    
  }
  else if ( m_tab3 )
  {
    MyMoneyReport::ERowType rtq[7] = { MyMoneyReport::eCategory, MyMoneyReport::eTopCategory, MyMoneyReport::ePayee, MyMoneyReport::eAccount, MyMoneyReport::eTopAccount, MyMoneyReport::eMonth, MyMoneyReport::eWeek };
    m_currentState.setRowType( rtq[m_tab3->m_comboOrganizeBy->currentItem()] );

    unsigned qc = MyMoneyReport::eQCnone;

    if ( m_tab3->m_checkNumber->isChecked() )
      qc |= MyMoneyReport::eQCnumber;
    if ( m_tab3->m_checkPayee->isChecked() )
      qc |= MyMoneyReport::eQCpayee;
    if ( m_tab3->m_checkCategory->isChecked() )
      qc |= MyMoneyReport::eQCcategory;
    if ( m_tab3->m_checkMemo->isChecked() )
      qc |= MyMoneyReport::eQCmemo;
    if ( m_tab3->m_checkAccount->isChecked() )
      qc |= MyMoneyReport::eQCaccount;
    if ( m_tab3->m_checkReconciled->isChecked() )
      qc |= MyMoneyReport::eQCreconciled;
    if ( m_tab3->m_checkAction->isChecked() )
      qc |= MyMoneyReport::eQCaction;
    if ( m_tab3->m_checkShares->isChecked() )
      qc |= MyMoneyReport::eQCshares;
    if ( m_tab3->m_checkPrice->isChecked() )
      qc |= MyMoneyReport::eQCprice;

    m_currentState.setQueryColumns(static_cast<MyMoneyReport::EQueryColumns>(qc));

    m_currentState.setTax( m_tab3->m_checkTax->isChecked() );
    m_currentState.setInvestmentsOnly( m_tab3->m_checkInvestments->isChecked() );
    m_currentState.setLoansOnly( m_tab3->m_checkLoans->isChecked() );
  }

  if ( m_tabChart )
  {
    MyMoneyReport::EChartType ct[5] = { MyMoneyReport::eChartLine, MyMoneyReport::eChartBar, MyMoneyReport::eChartStackedBar, MyMoneyReport::eChartPie, MyMoneyReport::eChartRing };
    m_currentState.setChartType( ct[m_tabChart->m_comboType->currentItem()] );

    m_currentState.setChartGridLines( m_tabChart->m_checkGridLines->isChecked() );
    m_currentState.setChartDataLabels( m_tabChart->m_checkValues->isChecked() );
    m_currentState.setChartByDefault( m_tabChart->m_checkShowChart->isChecked() );
  }

  // setup the date lock
  unsigned range = m_dateRange->currentItem();
  m_currentState.setDateFilter(range);

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

  m_tab1->m_editName->setText( m_initialState.name() );
  m_tab1->m_editComment->setText( m_initialState.comment() );
  m_tab1->m_checkCurrency->setChecked( m_initialState.isConvertCurrency() );
  m_tab1->m_checkFavorite->setChecked( m_initialState.isFavorite() );

  if ( m_tab2 )
  {
    switch ( m_initialState.detailLevel() )
    {
    case MyMoneyReport::eDetailNone:
    case MyMoneyReport::eDetailEnd:
    case MyMoneyReport::eDetailAll:
      m_tab2->m_comboDetail->setCurrentItem(0);
      break;
    case MyMoneyReport::eDetailTop:
      m_tab2->m_comboDetail->setCurrentItem(1);
      break;
    case MyMoneyReport::eDetailGroup:
      m_tab2->m_comboDetail->setCurrentItem(2);
      break;
    case MyMoneyReport::eDetailTotal:
      m_tab2->m_comboDetail->setCurrentItem(3);
      break;
    }

    if ( m_initialState.rowType() == MyMoneyReport::eExpenseIncome )
      m_tab2->m_comboRows->setCurrentItem(0);
    else
      m_tab2->m_comboRows->setCurrentItem(1);

    if ( m_initialState.isColumnsAreDays() )
    {
      switch ( m_initialState.columnType() )
      {
      case MyMoneyReport::eNoColumns:
      case MyMoneyReport::eDays:
        m_tab2->m_comboColumns->setCurrentItem(0);
        break;
      case MyMoneyReport::eWeeks:
        m_tab2->m_comboColumns->setCurrentItem(1);
        break;
      }
    }
    else
    {
      switch ( m_initialState.columnType() )
      {
      case MyMoneyReport::eNoColumns:
      case MyMoneyReport::eMonths:
        m_tab2->m_comboColumns->setCurrentItem(2);
        break;
      case MyMoneyReport::eBiMonths:
        m_tab2->m_comboColumns->setCurrentItem(3);
        break;
      case MyMoneyReport::eQuarters:
        m_tab2->m_comboColumns->setCurrentItem(4);
        break;
      case MyMoneyReport::eYears:
        m_tab2->m_comboColumns->setCurrentItem(5);
        break;
      }
    }
    m_tab2->m_checkScheduled->setChecked( m_currentState.isIncludingSchedules() );
    m_tab2->m_checkTransfers->setChecked( m_currentState.isIncludingTransfers() );
  }
  else if ( m_tab3 )
  {
    switch ( m_initialState.rowType() )
    {
    case MyMoneyReport::eNoColumns:
    case MyMoneyReport::eCategory:
      m_tab3->m_comboOrganizeBy->setCurrentItem(0);
      break;
    case MyMoneyReport::eTopCategory:
      m_tab3->m_comboOrganizeBy->setCurrentItem(1);
      break;
    case MyMoneyReport::ePayee:
      m_tab3->m_comboOrganizeBy->setCurrentItem(2);
      break;
    case MyMoneyReport::eAccount:
      m_tab3->m_comboOrganizeBy->setCurrentItem(3);
      break;
    case MyMoneyReport::eTopAccount:
      m_tab3->m_comboOrganizeBy->setCurrentItem(4);
      break;
    case MyMoneyReport::eMonth:
      m_tab3->m_comboOrganizeBy->setCurrentItem(5);
      break;
    case MyMoneyReport::eWeek:
      m_tab3->m_comboOrganizeBy->setCurrentItem(6);
      break;
    case MyMoneyReport::eAccountByTopAccount:
    case MyMoneyReport::eEquityType:
    case MyMoneyReport::eAccountType:
    case MyMoneyReport::eInstitution:
    case MyMoneyReport::eAssetLiability:
    case MyMoneyReport::eExpenseIncome:
      throw new MYMONEYEXCEPTION("KReportConfigurationFilterDlg::slotReset(): QueryTable report has invalid rowtype");
    }

    unsigned qc = m_initialState.queryColumns();
    m_tab3->m_checkNumber->setChecked(qc & MyMoneyReport::eQCnumber);
    m_tab3->m_checkPayee->setChecked(qc & MyMoneyReport::eQCpayee);
    m_tab3->m_checkCategory->setChecked(qc & MyMoneyReport::eQCcategory);
    m_tab3->m_checkMemo->setChecked(qc & MyMoneyReport::eQCmemo);
    m_tab3->m_checkAccount->setChecked(qc & MyMoneyReport::eQCaccount);
    m_tab3->m_checkReconciled->setChecked(qc & MyMoneyReport::eQCreconciled);
    m_tab3->m_checkAction->setChecked(qc & MyMoneyReport::eQCaction);
    m_tab3->m_checkShares->setChecked(qc & MyMoneyReport::eQCshares);
    m_tab3->m_checkPrice->setChecked(qc & MyMoneyReport::eQCprice);

    m_tab3->m_checkTax->setChecked( m_initialState.isTax() );
    m_tab3->m_checkInvestments->setChecked( m_initialState.isInvestmentsOnly() );
    m_tab3->m_checkLoans->setChecked( m_initialState.isLoansOnly() );
  }

  if ( m_tabChart )
  {
    switch( m_initialState.chartType() )
    {
      case MyMoneyReport::eChartNone:
      case MyMoneyReport::eChartLine:
        m_tabChart->m_comboType->setCurrentItem(0);
        break;
      case MyMoneyReport::eChartBar:
        m_tabChart->m_comboType->setCurrentItem(1);
        break;
      case MyMoneyReport::eChartStackedBar:
        m_tabChart->m_comboType->setCurrentItem(2);
        break;
      case MyMoneyReport::eChartPie:
        m_tabChart->m_comboType->setCurrentItem(3);
        break;
      case MyMoneyReport::eChartRing:
        m_tabChart->m_comboType->setCurrentItem(4);
        break;
      case MyMoneyReport::eChartEnd:
        throw new MYMONEYEXCEPTION("KReportConfigurationFilterDlg::slotReset(): Report has invalid charttype");
    }
    m_tabChart->m_checkGridLines->setChecked(m_initialState.isChartGridLines());
    m_tabChart->m_checkValues->setChecked(m_initialState.isChartDataLabels());
    m_tabChart->m_checkShowChart->setChecked(m_initialState.isChartByDefault());
  }

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
    m_accountsView->selectAllItems(false);
    m_accountsView->selectItems(accounts,true);
  }
  else
    m_accountsView->selectAllItems(true);

  //
  // Categories Filter
  //

  if ( m_initialState.categories(accounts) )
  {
    m_categoriesView->selectAllItems(false);
    m_categoriesView->selectItems(accounts,true);
  }
  else
    m_categoriesView->selectAllItems(true);

  //
  // Date Filter
  //

  // the following call implies a call to slotUpdateSelections,
  // that's why we call it last

  m_initialState.updateDateFilter();
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

  //QTimer::singleShot(0, this, SLOT(slotRightSize()));

  slotRightSize();
}

void KReportConfigurationFilterDlg::slotHelp(void)
{
  kapp->invokeHelp("details.reports.config");
}

#include "kreportconfigurationfilterdlg.moc"
