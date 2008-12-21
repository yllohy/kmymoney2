/***************************************************************************
                          kbalancechartdlg  -  description
                             -------------------
    begin                : Mon Nov 26 2007
    copyright            : (C) 2007 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
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

#include <qlayout.h>
#include <qframe.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kpushbutton.h>
#include <klocale.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kbalancechartdlg.h"

#include <kmymoney/mymoneyreport.h>
#include "../reports/kreportchartview.h"
#include "../reports/pivottable.h"

#include <kmymoney/kmymoneyglobalsettings.h>


// in KOffice version < 1.5 KDCHART_PROPSET_NORMAL_DATA was a static const
// but in 1.5 this has been changed into a #define'd value. So we have to
// make sure, we use the right one.
#ifndef KDCHART_PROPSET_NORMAL_DATA
#define KMM_KDCHART_PROPSET_NORMAL_DATA KDChartParams::KDCHART_PROPSET_NORMAL_DATA
#else
#define KMM_KDCHART_PROPSET_NORMAL_DATA KDCHART_PROPSET_NORMAL_DATA
#endif

KBalanceChartDlg::KBalanceChartDlg(const MyMoneyAccount& account, QWidget* parent, const char* name) :
  KDialog(parent, name)
{
#ifdef HAVE_KDCHART
  setCaption(i18n("Balance of %1").arg(account.name()));
  setSizeGripEnabled( TRUE );
  setModal( TRUE );

  QVBoxLayout* KBalanceChartDlgLayout = new QVBoxLayout( this, 11, 6, "KBalanceChartDlgLayout");

  MyMoneyReport reportCfg = MyMoneyReport(
                                          MyMoneyReport::eAssetLiability,
                                          MyMoneyReport::eMonths,
                                          MyMoneyTransactionFilter::userDefined, // overridden by the setDateFilter() call below
                                          false,
                                          i18n("%1 Balance History").arg(account.name()),
                                               i18n("Generated Report")
                                         );
  reportCfg.setChartByDefault(true);
  reportCfg.setChartGridLines(false);
  reportCfg.setChartDataLabels(false);
  reportCfg.setDetailLevel(MyMoneyReport::eDetailTotal);
  reportCfg.setChartType(MyMoneyReport::eChartLine);
  reportCfg.setIncludingSchedules( true );
  if(account.accountType() == MyMoneyAccount::Investment) {
    QStringList::const_iterator it_a;
    for(it_a = account.accountList().begin(); it_a != account.accountList().end(); ++it_a)
      reportCfg.addAccount(*it_a);
  } else
    reportCfg.addAccount(account.id());
  reportCfg.setColumnsAreDays( true );
  reportCfg.setConvertCurrency( false );
  reportCfg.setDateFilter(QDate::currentDate().addDays(-90),QDate::currentDate().addDays(+90));

  reports::PivotTable table(reportCfg);

  reports::KReportChartView* chartWidget = new reports::KReportChartView(this, 0);

  table.drawChart(*chartWidget);

  chartWidget->params().setLineMarker(false);
  chartWidget->params().setLegendPosition(KDChartParams::NoLegend);
  chartWidget->params().setLineWidth(2);
  chartWidget->params().setDataColor(0, KGlobalSettings::textColor());

  // draw future values in a different line style
  KDChartPropertySet propSetFutureValue("future value", KMM_KDCHART_PROPSET_NORMAL_DATA);
  propSetFutureValue.setLineStyle(KDChartPropertySet::OwnID, Qt::DotLine);
  int m_idPropFutureValue = chartWidget->params().registerProperties(propSetFutureValue);

  KDChartPropertySet propSetLastValue("last value", m_idPropFutureValue);
  propSetLastValue.setExtraLinesAlign(KDChartPropertySet::OwnID, Qt::AlignLeft | Qt::AlignBottom);
  propSetLastValue.setExtraLinesWidth(KDChartPropertySet::OwnID, -4);
  propSetLastValue.setExtraLinesColor(KDChartPropertySet::OwnID, KMyMoneyGlobalSettings::listGridColor());

  int m_idPropLastValue = chartWidget->params().registerProperties(propSetLastValue);

  KDChartPropertySet propSetMinBalance("min balance", m_idPropFutureValue);
  propSetMinBalance.setLineStyle(KDChartPropertySet::OwnID, Qt::NoPen);
  propSetMinBalance.setExtraLinesAlign(KDChartPropertySet::OwnID, Qt::AlignLeft | Qt::AlignRight);
  int m_idPropMinBalance = chartWidget->params().registerProperties(propSetMinBalance);

  KDChartPropertySet propSetMaxCredit("max credit", m_idPropMinBalance);
  propSetMaxCredit.setExtraLinesColor(KDChartPropertySet::OwnID, KMyMoneyGlobalSettings::listNegativeValueColor());
  propSetMaxCredit.setExtraLinesStyle(KDChartPropertySet::OwnID, Qt::DotLine);
  int m_idPropMaxCredit = chartWidget->params().registerProperties(propSetMaxCredit);

  KBalanceChartDlgLayout->addWidget(chartWidget, 10);


  // add another row for markers if required or remove it if not necessary
  // see http://www.klaralvdalens-datakonsult.se/kdchart/ProgrammersManual/KDChart.pdf
  // Chapter 6, "Adding separate Lines/Markers".

  bool needRow = false;
  bool haveMinBalance = false;
  bool haveMaxCredit = false;
  MyMoneyMoney minBalance, maxCredit;
  MyMoneyMoney factor(1,1);
  if(account.accountGroup() == MyMoneyAccount::Asset)
    factor = -factor;

  if(account.value("maxCreditEarly").length() > 0) {
    needRow = true;
    haveMaxCredit = true;
    maxCredit = MyMoneyMoney(account.value("maxCreditEarly")) * factor;
  }
  if(account.value("maxCreditAbsolute").length() > 0) {
    needRow = true;
    haveMaxCredit = true;
    maxCredit = MyMoneyMoney(account.value("maxCreditAbsolute")) * factor;
  }

  if(account.value("minBalanceEarly").length() > 0) {
    needRow = true;
    haveMinBalance = true;
    minBalance = MyMoneyMoney(account.value("minBalanceEarly"));
  }
  if(account.value("minBalanceAbsolute").length() > 0) {
    needRow = true;
    haveMinBalance = true;
    minBalance = MyMoneyMoney(account.value("minBalanceAbsolute"));
  }

  KDChartTableDataBase* data = chartWidget->data();
  if(!needRow && data->usedRows() == 2) {
    data->expand( data->usedRows()-1, data->usedCols() );
  } else if(needRow && data->usedRows() == 1) {
    data->expand( data->usedRows()+1, data->usedCols() );
  }

  if(needRow) {
    if(haveMinBalance) {
      data->setCell(1, 0, minBalance.toDouble());
      chartWidget->setProperty(1, 0, m_idPropMinBalance);
    }
    if(haveMaxCredit) {
      data->setCell(1, 1, maxCredit.toDouble());
      chartWidget->setProperty(1, 1, m_idPropMaxCredit);
    }
  }

  for(int iCell = 90; iCell < 180; ++iCell) {
    chartWidget->setProperty(0, iCell, m_idPropFutureValue);
  }
  chartWidget->setProperty(0, 90, m_idPropLastValue);



  QFrame* line1 = new QFrame( this, "line1" );
  line1->setFrameShape( QFrame::HLine );
  line1->setFrameShadow( QFrame::Sunken );
  line1->setFrameShape( QFrame::HLine );

  KBalanceChartDlgLayout->addWidget(line1);
  QHBoxLayout* Layout1 = new QHBoxLayout( KBalanceChartDlgLayout, 6, "Layout1");
#if 0
  KPushButton* buttonHelp = new KPushButton( this, "buttonHelp" );
  buttonHelp->setAutoDefault( TRUE );
  buttonHelp->setText(i18n("&Help"));
  Layout1->addWidget( buttonHelp );
#endif

  QSpacerItem* Horizontal_Spacing2 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  Layout1->addItem( Horizontal_Spacing2 );

#if 0
  KPushButton* buttonOk = new KPushButton( this, "buttonOk" );
  buttonOk->setAutoDefault( TRUE );
  buttonOk->setDefault( TRUE );
  buttonOk->setText(i18n("&OK"));
  Layout1->addWidget( buttonOk );
#endif
  KPushButton* buttonClose = new KPushButton( this, "buttonClose" );
  buttonClose->setEnabled( TRUE );
  buttonClose->setAutoDefault( TRUE );
  buttonClose->setText(i18n("&Close"));
  Layout1->addWidget( buttonClose );

  // connect( buttonOk, SIGNAL( clicked() ), this, SLOT( accept() ) );
  connect( buttonClose, SIGNAL( clicked() ), this, SLOT( accept() ) );

  resize( QSize(700, 500).expandedTo(minimumSizeHint()) );
  clearWState( WState_Polished );
#endif
}


KBalanceChartDlg::~KBalanceChartDlg()
{
}

#include "kbalancechartdlg.moc"

