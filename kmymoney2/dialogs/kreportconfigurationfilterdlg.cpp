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
#include <qspinbox.h>
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
#include <kstdguiitem.h>
#include <klocale.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kreportconfigurationfilterdlg.h"
#include <kmymoney/kmymoneydateinput.h>
#include <kmymoney/kmymoneyedit.h>
#include <kmymoney/kmymoneylineedit.h>
#include <kmymoney/kmymoneyaccountselector.h>
#include <kmymoney/mymoneyfile.h>
#include <kmymoney/mymoneyreport.h>
#include <kmymoney/kmymoneycombo.h>
#include "../widgets/kmymoneyreportconfigtab1decl.h"
#include "../widgets/kmymoneyreportconfigtab2decl.h"
#include "../widgets/kmymoneyreportconfigtab3decl.h"
#include "../widgets/kmymoneyreportconfigtabchartdecl.h"

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

    setCaption( i18n( "Report Configuration" ) );
    delete TextLabel1;

    //
    // Rework the buttons
    //

    // the Ok button is always enabled
    disconnect(SIGNAL(selectionEmpty(bool)));
    m_searchButton->setGuiItem( KStdGuiItem::ok() );
    m_searchButton->setEnabled(true);

    // reconnect the close button
    m_closeButton->disconnect();
    connect(m_closeButton, SIGNAL(clicked()), this, SLOT(reject()));

    //
    // Add new tabs
    //

    m_tab1 = new kMyMoneyReportConfigTab1Decl( m_criteriaTab, "kMyMoneyReportConfigTab1" );
    m_criteriaTab->insertTab( m_tab1, i18n("Report"), 0 );

    if ( m_initialState.reportType() == MyMoneyReport::ePivotTable )
    {
      m_tab2 = new kMyMoneyReportConfigTab2Decl( m_criteriaTab, "kMyMoneyReportConfigTab2" );
      m_criteriaTab->insertTab( m_tab2, i18n( "Rows/Columns"), 1 );
      connect(m_tab2->m_comboRows, SIGNAL(highlighted(int)), this, SLOT(slotRowTypeChanged(int)));
      connect(m_tab2->m_comboColumns, SIGNAL(activated(int)), this, SLOT(slotColumnTypeChanged(int)));
      //control the state of the includeTransfer check
      connect(m_categoriesView, SIGNAL(stateChanged()), this, SLOT(slotUpdateCheckTransfers()));

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

    QValueList<MyMoneyBudget> list = MyMoneyFile::instance()->budgetList();
    QValueList<MyMoneyBudget>::const_iterator it_b;
    for(it_b = list.begin(); it_b != list.end(); ++it_b) {
      m_budgets.push_back(*it_b);
    }

    //
    // Now set up the widgets with proper values
    //
    slotReset();
}

KReportConfigurationFilterDlg::~KReportConfigurationFilterDlg()
{
}

void KReportConfigurationFilterDlg::slotSearch(void)
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

    // modify the rowtype only if the widget is enabled
    if(m_tab2->m_comboRows->isEnabled()) {
      MyMoneyReport::ERowType rt[2] = { MyMoneyReport::eExpenseIncome, MyMoneyReport::eAssetLiability };
      m_currentState.setRowType( rt[m_tab2->m_comboRows->currentItem()] );
    }

    m_currentState.setShowingRowTotals(false);
    if(m_tab2->m_comboRows->currentItem() == 0)
      m_currentState.setShowingRowTotals(m_tab2->m_checkTotalColumn->isChecked());

    MyMoneyReport::EColumnType ct[6] = { MyMoneyReport::eDays, MyMoneyReport::eWeeks, MyMoneyReport::eMonths, MyMoneyReport::eBiMonths, MyMoneyReport::eQuarters, MyMoneyReport::eYears };
    bool dy[6] = { true, true, false, false, false, false };
    m_currentState.setColumnType( ct[m_tab2->m_comboColumns->currentItem()] );

    //TODO (Ace) This should be implicit in the call above.  MMReport needs fixin'
    m_currentState.setColumnsAreDays( dy[m_tab2->m_comboColumns->currentItem()] );

    m_currentState.setIncludingSchedules( m_tab2->m_checkScheduled->isChecked() );

    m_currentState.setIncludingTransfers( m_tab2->m_checkTransfers->isChecked() );

    m_currentState.setIncludingUnusedAccounts( m_tab2->m_checkUnused->isChecked() );

    if(m_tab2->m_comboBudget->isEnabled()) {
      m_currentState.setBudget(m_budgets[m_tab2->m_comboBudget->currentItem()].id(), m_initialState.rowType() == MyMoneyReport::eBudgetActual);
    } else {
      m_currentState.setBudget(QString(), false);
    }
    
    //set moving average days
    if(m_tab2->m_movingAverageDays->isEnabled()) {
      m_currentState.setMovingAverageDays( m_tab2->m_movingAverageDays->value() );
    }
  }
  else if ( m_tab3 )
  {
    MyMoneyReport::ERowType rtq[7] = { MyMoneyReport::eCategory, MyMoneyReport::eTopCategory, MyMoneyReport::ePayee, MyMoneyReport::eAccount, MyMoneyReport::eTopAccount, MyMoneyReport::eMonth, MyMoneyReport::eWeek };
    m_currentState.setRowType( rtq[m_tab3->m_comboOrganizeBy->currentItem()] );

    unsigned qc = MyMoneyReport::eQCnone;

    if (m_currentState.queryColumns() & MyMoneyReport::eQCloan)
      // once a loan report, always a loan report
      qc = MyMoneyReport::eQCloan;

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
    if( m_tab3->m_checkBalance->isChecked() )
      qc |= MyMoneyReport::eQCbalance;

    m_currentState.setQueryColumns(static_cast<MyMoneyReport::EQueryColumns>(qc));

    m_currentState.setTax( m_tab3->m_checkTax->isChecked() );
    m_currentState.setInvestmentsOnly( m_tab3->m_checkInvestments->isChecked() );
    m_currentState.setLoansOnly( m_tab3->m_checkLoans->isChecked() );

    m_currentState.setDetailLevel(m_tab3->m_checkHideSplitDetails->isChecked() ?
        MyMoneyReport::eDetailNone : MyMoneyReport::eDetailAll);
  }

  if ( m_tabChart )
  {
    MyMoneyReport::EChartType ct[5] = { MyMoneyReport::eChartLine, MyMoneyReport::eChartBar, MyMoneyReport::eChartStackedBar, MyMoneyReport::eChartPie, MyMoneyReport::eChartRing };
    m_currentState.setChartType( ct[m_tabChart->m_comboType->currentItem()] );

    m_currentState.setChartGridLines( m_tabChart->m_checkGridLines->isChecked() );
    m_currentState.setChartDataLabels( m_tabChart->m_checkValues->isChecked() );
    m_currentState.setChartByDefault( m_tabChart->m_checkShowChart->isChecked() );
    m_currentState.setChartLineWidth( m_tabChart->m_lineWidth->value() );
  }

  // setup the date lock
  MyMoneyTransactionFilter::dateOptionE range = m_dateRange->currentItem();
  m_currentState.setDateFilter(range);

  done(true);
}

void KReportConfigurationFilterDlg::slotRowTypeChanged(int row)
{
  m_tab2->m_checkTotalColumn->setEnabled(row == 0);
}

void KReportConfigurationFilterDlg::slotColumnTypeChanged(int row)
{
  if(m_tab2->m_comboBudget->isEnabled() && row < 2) {
    m_tab2->m_comboColumns->setCurrentItem(2);
  }
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

    switch(m_initialState.rowType()) {
      case MyMoneyReport::eExpenseIncome:
      case MyMoneyReport::eBudget:
      case MyMoneyReport::eBudgetActual:
        m_tab2->m_comboRows->setCurrentItem(0); // income / expense
        break;
      default:
        m_tab2->m_comboRows->setCurrentItem(1); // asset / liability
        break;
    }
    m_tab2->m_checkTotalColumn->setChecked(m_initialState.isShowingRowTotals());

    slotRowTypeChanged(m_tab2->m_comboRows->currentItem());

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
      default:
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
      default:
        break;
      }
    }

    //load budgets combo
    if(m_initialState.rowType() == MyMoneyReport::eBudget
    || m_initialState.rowType() == MyMoneyReport::eBudgetActual) {
      m_tab2->m_comboRows->setEnabled(false);
      m_tab2->m_budgetFrame->setEnabled(!m_budgets.empty());
      QValueVector<MyMoneyBudget>::const_iterator it_b;
      int i = 0;
      for(it_b = m_budgets.begin(); it_b != m_budgets.end(); ++it_b) {
        m_tab2->m_comboBudget->insertItem((*it_b).name(), i);
        //set the current selected item
        if( (m_initialState.budget() == "Any" && (*it_b).budgetStart().year() == QDate::currentDate().year())
             || m_initialState.budget() == (*it_b).id())
          m_tab2->m_comboBudget->setCurrentItem(i);
        i++;
      }
    }

    //set moving average days spinbox
    m_tab2->m_movingAverageDays->setEnabled( m_initialState.isIncludingMovingAverage() );
    if(m_initialState.isIncludingMovingAverage() ) {
      m_tab2->m_movingAverageDays->setValue( m_initialState.movingAverageDays() );
    }

    m_tab2->m_checkScheduled->setChecked( m_initialState.isIncludingSchedules() );
    m_tab2->m_checkTransfers->setChecked( m_initialState.isIncludingTransfers() );
    m_tab2->m_checkUnused->setChecked( m_initialState.isIncludingUnusedAccounts() );
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
    default:
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
    m_tab3->m_checkBalance->setChecked(qc & MyMoneyReport::eQCbalance);

    m_tab3->m_checkTax->setChecked( m_initialState.isTax() );
    m_tab3->m_checkInvestments->setChecked( m_initialState.isInvestmentsOnly() );
    m_tab3->m_checkLoans->setChecked( m_initialState.isLoansOnly() );

    m_tab3->m_checkHideSplitDetails->setChecked
      (m_initialState.detailLevel() == MyMoneyReport::eDetailNone);
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
    m_tabChart->m_lineWidth->setValue(m_initialState.chartLineWidth());
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
    m_textNegate->setCurrentItem(m_initialState.isInvertingText());
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

  QStringList payees;
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

  QStringList accounts;
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
    m_dateRange->setCurrentItem(MyMoneyTransactionFilter::userDefined);
    m_fromDate->setDate(dateFrom);
    m_toDate->setDate(dateTo);
    slotDateChanged();
  }
  else
  {
    m_dateRange->setCurrentItem(MyMoneyTransactionFilter::allDates);
    slotDateRangeChanged(MyMoneyTransactionFilter::allDates);
  }

  slotRightSize();
}

void KReportConfigurationFilterDlg::slotShowHelp(void)
{
  kapp->invokeHelp("details.reports.config");
}

//TODO Fix the reports and engine to include transfers even if categories are filtered - bug #1523508
void KReportConfigurationFilterDlg::slotUpdateCheckTransfers(void)
{
  if(!m_categoriesView->allItemsSelected()) {
    m_tab2->m_checkTransfers->setChecked(false);
    m_tab2->m_checkTransfers->setDisabled(true);
  } else {
    m_tab2->m_checkTransfers->setEnabled(true);
  }
}

#include "kreportconfigurationfilterdlg.moc"
