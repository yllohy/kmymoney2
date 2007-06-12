/***************************************************************************
                          khomeview.cpp  -  description
                             -------------------
    begin                : Tue Jan 22 2002
    copyright            : (C) 2000-2002 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
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
#include <qdatetime.h>
#include <qapplication.h>
#include <dom/dom_element.h>
#include <dom/dom_doc.h>
#include <dom/dom_text.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qtimer.h>
#include <qbuffer.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <khtmlview.h>
#include <kconfig.h>
#include <kstdaction.h>
#include <kmainwindow.h>
#include <kactioncollection.h>
#include <kapplication.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <kmdcodec.h>
#include <kglobalsettings.h>

// ----------------------------------------------------------------------------
// Project Includes
#include "khomeview.h"
#include "../kmymoneyutils.h"
#include "../kmymoneysettings.h"
#include "../mymoney/mymoneyfile.h"
#include "../kmymoney2.h"
#include "../reports/kreportchartview.h"
#include "../reports/pivottable.h"
#include "../kmymoneyglobalsettings.h"


#define VIEW_LEDGER         "ledger"
#define VIEW_SCHEDULE       "schedule"
#define VIEW_WELCOME        "welcome"
#define VIEW_HOME           "home"
#define VIEW_REPORTS        "reports"

// in KOffice version < 1.5 KDCHART_PROPSET_NORMAL_DATA was a static const
// but in 1.5 this has been changed into a #define'd value. So we have to
// make sure, we use the right one.
#ifndef KDCHART_PROPSET_NORMAL_DATA
#define KMM_KDCHART_PROPSET_NORMAL_DATA KDChartParams::KDCHART_PROPSET_NORMAL_DATA
#else
#define KMM_KDCHART_PROPSET_NORMAL_DATA KDCHART_PROPSET_NORMAL_DATA
#endif


KHomeView::KHomeView(QWidget *parent, const char *name ) :
  KMyMoneyViewBase(parent, name, i18n("Home")),
  m_showAllSchedules(false),
  m_needReload(true)
{
  m_part = new KHTMLPart(this, "htmlpart_km2");
  m_viewLayout->addWidget(m_part->view());

  m_filename = KMyMoneyUtils::findResource("appdata", QString("html/home%1.html"));

//   m_part->openURL(m_filename);
  connect(m_part->browserExtension(), SIGNAL(openURLRequest(const KURL&, const KParts::URLArgs&)),
          this, SLOT(slotOpenURL(const KURL&, const KParts::URLArgs&)));

  connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotLoadView()));
}

KHomeView::~KHomeView()
{
  // if user wants to remember the font size, store it here
  if (KMyMoneySettings::self()->rememberFontSize())
  {
    KMyMoneySettings::self()->setFontSizePercentage(m_part->zoomFactor());
    //kdDebug() << "Storing font size: " << m_part->zoomFactor() << endl;
    KMyMoneySettings::self()->writeConfig();
  }
}

void KHomeView::slotLoadView(void)
{
  m_needReload = true;
  if(isVisible()) {
    loadView();
    m_needReload = false;
  }
}

void KHomeView::show(void)
{
  if(m_needReload) {
    loadView();
    m_needReload = false;
  }
  QWidget::show();
}

void KHomeView::slotPrintView(void)
{
  if(m_part && m_part->view())
    m_part->view()->print();
}

void KHomeView::loadView(void)
{
  m_part->setZoomFactor( KMyMoneySettings::self()->fontSizePercentage() );
  //kdDebug() << "Setting font size: " << m_part->zoomFactor() << endl;

  QValueList<MyMoneyAccount> list;
  MyMoneyFile::instance()->accountList(list);
  if(list.count() == 0)
  {
    m_part->openURL(m_filename);

#if 0
    // (ace) I am experimenting with replacing links in the
    // html depending on the state of the engine.  It's not
    // working.  That's why it's #if0'd out.

    DOM::Element e = m_part->document().getElementById("test");
    if ( e.isNull() )
    {
      qDebug("Element id=test not found");
    }
    else
    {
      qDebug("Element id=test found!");
      QString tagname = e.tagName().string();
      qDebug("%s",tagname.latin1());
      qDebug("%s id=%s",e.tagName().string().latin1(),e.getAttribute("id").string().latin1());

      // Find the character data node
      DOM::Node n = e.firstChild();
      while (!n.isNull())
      {
        qDebug("Child type %u",static_cast<unsigned>(n.nodeType()));
        if ( n.nodeType() == DOM::Node::TEXT_NODE )
        {
          DOM::Text t = n;
          t.setData("Success!!");
          e.replaceChild(n,t);
          m_part->document().setDesignMode(true);
          m_part->document().importNode(e,true);
          m_part->document().updateRendering();

          qDebug("Data is now %s",t.data().string().latin1());
        }
        n = n.nextSibling();
      }
    }
#endif
  } else {
    QString filename = KGlobal::dirs()->findResource("appdata", "html/kmymoney2.css");
    QString header = QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\">\n<html><head><link rel=\"stylesheet\" type=\"text/css\" href=\"%1\">\n").arg(filename);

    header += KMyMoneyUtils::variableCSS();

    header += "</head><body>\n";

    QString footer = "</body></html>\n";

    m_part->begin();
    m_part->write(header);

    m_part->write(QString("<h2>%1</h2>").arg(i18n("Your Financial Summary")));

    QStringList settings = KMyMoneyGlobalSettings::itemList();

    QStringList::ConstIterator it;

    for(it = settings.begin(); it != settings.end(); ++it) {
      int option = (*it).toInt();
      if(option > 0) {
        switch(option) {
          case 1:         // payments
            showPayments();
            break;

          case 2:         // preferred accounts
            showAccounts(Preferred, i18n("Preferred Accounts"));
            break;

          case 3:         // payment accounts
            // Check if preferred accounts are shown separately
            if(settings.find("2") == settings.end()) {
              showAccounts(static_cast<paymentTypeE> (Payment | Preferred),
                           i18n("Payment Accounts"));
            } else {
              showAccounts(Payment, i18n("Payment Accounts"));
            }
            break;
          case 4:         // favorite reports
            showFavoriteReports();
            break;
          case 5:          // forecast
            showForecast();
            break;
            case 6:        // net worth graph over all accounts
            showNetWorthGraph();
            break;
        }
        m_part->write("<div class=\"gap\">&nbsp;</div>\n");
      }
    }

    m_part->write(link(VIEW_WELCOME, QString()) + i18n("Show KMyMoney welcome page") + linkend());

    m_part->write(footer);
    m_part->end();

  }
}

void KHomeView::showNetWorthGraph(void)
{
#ifdef HAVE_KDCHART
  m_part->write(QString("<div class=\"itemheader\">%1</div>\n").arg(i18n("Networth Forecast")));

  MyMoneyReport reportCfg = MyMoneyReport(
      MyMoneyReport::eAssetLiability,
      MyMoneyReport::eMonths,
      MyMoneyTransactionFilter::userDefined, // overridden by the setDateFilter() call below
      false,
      i18n("Networth Forecast"),
      i18n("Generated Report"));

  reportCfg.setChartByDefault(true);
  reportCfg.setChartGridLines(false);
  reportCfg.setChartDataLabels(false);
  reportCfg.setDetailLevel(MyMoneyReport::eDetailTotal);
  reportCfg.setChartType(MyMoneyReport::eChartLine);
  reportCfg.setIncludingSchedules( true );
  reportCfg.addAccountGroup(MyMoneyAccount::Asset);
  reportCfg.addAccountGroup(MyMoneyAccount::Liability);
  reportCfg.setColumnsAreDays( true );
  reportCfg.setConvertCurrency( false );
  reportCfg.setDateFilter(QDate::currentDate().addDays(-10),QDate::currentDate().addDays(+90));

  reports::PivotTable table(reportCfg);

  reports::KReportChartView* chartWidget = new reports::KReportChartView(0, 0);

  table.drawChart(*chartWidget);

  chartWidget->params().setLineMarker(false);
  chartWidget->params().setLegendPosition(KDChartParams::NoLegend);
  chartWidget->params().setLineWidth(2);
  chartWidget->params().setDataColor(0, KGlobalSettings::textColor());

    // draw future values in a different line style
  KDChartPropertySet propSetLastValue("last value", KMM_KDCHART_PROPSET_NORMAL_DATA);
  KDChartPropertySet propSetFutureValue("future value", KMM_KDCHART_PROPSET_NORMAL_DATA);
  propSetLastValue.setExtraLinesAlign(KDChartPropertySet::OwnID, Qt::AlignLeft | Qt::AlignBottom);
  propSetLastValue.setExtraLinesWidth(KDChartPropertySet::OwnID, -4);
  propSetLastValue.setExtraLinesColor(KDChartPropertySet::OwnID, KMyMoneyGlobalSettings::listGridColor());
  // propSetLastValue.setShowMarker(KDChartPropertySet::OwnID, true);
  // propSetLastValue.setMarkerStyle(KDChartPropertySet::OwnID, KDChartParams::LineMarkerDiamond);

  propSetFutureValue.setLineStyle(KDChartPropertySet::OwnID, Qt::DotLine);
  const int idPropFutureValue = chartWidget->params().registerProperties(propSetFutureValue);
  const int idPropLastValue = chartWidget->params().registerProperties(propSetLastValue);
  for(int iCell = 10; iCell < 100; ++iCell) {
    chartWidget->setProperty(0, iCell, idPropFutureValue);
  }
  chartWidget->setProperty(0, 10, idPropLastValue);

  // Adjust the size
  if(width() < chartWidget->width()) {
    int nh;
    nh = (width()*chartWidget->height() ) / chartWidget->width();
    chartWidget->resize(width()-40, nh);
  }

  QPixmap pm(chartWidget->width(), chartWidget->height());
  pm.fill(KGlobalSettings::baseColor());
  QPainter p(&pm);
  chartWidget->paintTo(p);

  QByteArray ba;
  QBuffer buffer( ba );
  buffer.open( IO_WriteOnly );
  pm.save( &buffer, "PNG" ); // writes pixmap into ba in PNG format

  m_part->write(QString("<center><IMG SRC=\"data:image/png;base64,%1\" ALT=\"Networth\"></center>").arg(KCodecs::base64Encode(ba)));
  delete chartWidget;
#endif
}

void KHomeView::showPayments(void)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  QValueList<MyMoneySchedule> overdues;
  QValueList<MyMoneySchedule> schedule;
  int i = 0;

  schedule = file->scheduleList("", MyMoneySchedule::TYPE_ANY,
                                 MyMoneySchedule::OCCUR_ANY,
                                 MyMoneySchedule::STYPE_ANY,
                                 QDate::currentDate(),
                                 QDate::currentDate().addMonths(1));
  overdues = file->scheduleList("", MyMoneySchedule::TYPE_ANY,
                                 MyMoneySchedule::OCCUR_ANY,
                                 MyMoneySchedule::STYPE_ANY,
                                 QDate(), QDate(), true);

  if(schedule.empty() && overdues.empty())
    return;

  // HACK
  // Remove the finished schedules

  QValueList<MyMoneySchedule>::Iterator d_it;
  for (d_it=schedule.begin(); d_it!=schedule.end();)
  {
    if ((*d_it).isFinished() || (*d_it).nextPayment((*d_it).lastPayment()) == QDate())
    {
      d_it = schedule.remove(d_it);
      continue;
    }
    ++d_it;
  }

  for (d_it=overdues.begin(); d_it!=overdues.end();)
  {
    if ((*d_it).isFinished() || (*d_it).nextPayment((*d_it).lastPayment()) == QDate())
    {
      d_it = overdues.remove(d_it);
      continue;
    }
    ++d_it;
  }

  m_part->write(QString("<div class=\"itemheader\">%1</div>\n").arg(i18n("Payments")));

  if(overdues.count() > 0) {
    m_part->write("<div class=\"gap\">&nbsp;</div>\n");

    qBubbleSort(overdues);
    QValueList<MyMoneySchedule>::Iterator it;
    QValueList<MyMoneySchedule>::Iterator it_f;

    m_part->write("<table width=\"75%\" cellspacing=\"0\" cellpadding=\"2\">");
    m_part->write(QString("<tr><th class=\"warning\" colspan=\"3\">%1</th></tr>\n").arg(i18n("Overdue payments")));
    for(it = overdues.begin(); it != overdues.end(); ++it) {
      // determine number of overdue payments
      QDate nextDate = (*it).nextPayment((*it).lastPayment());
      int cnt = 0;
      while(nextDate.isValid() && nextDate < QDate::currentDate()) {
        ++cnt;
        nextDate = (*it).nextPayment(nextDate);
        // for single occurence nextDate will not change, so we
        // better get out of here.
        if((*it).occurence() == MyMoneySchedule::OCCUR_ONCE)
          break;
      }

      m_part->write(QString("<tr class=\"row-%1\">").arg(i++ & 0x01 ? "even" : "odd"));
      showPaymentEntry(*it, cnt);
      m_part->write("</tr>");
      // make sure to not repeat overdues later again
      for(it_f = schedule.begin(); it_f != schedule.end();) {
        if((*it).id() == (*it_f).id()) {
          it_f = schedule.remove(it_f);
          continue;
        }
        ++it_f;
      }
    }
    m_part->write("</table>");
  }

  if(schedule.count() > 0) {
    qBubbleSort(schedule);

    // Extract todays payments if any
    QValueList<MyMoneySchedule> todays;
    QValueList<MyMoneySchedule>::Iterator t_it;
    for (t_it=schedule.begin(); t_it!=schedule.end();)
    {
      if ((*t_it).nextPayment((*t_it).lastPayment()) == QDate::currentDate())
      {
        todays.append(*t_it);
        (*t_it).setLastPayment((*t_it).nextPayment((*t_it).lastPayment()));
      }
      ++t_it;
    }

    if (todays.count() > 0)
    {
      m_part->write("<div class=\"gap\">&nbsp;</div>\n");
      m_part->write("<table width=\"75%\" cellspacing=\"0\" cellpadding=\"2\">");
      m_part->write(QString("<tr class=\"item\"><th class=\"left\" colspan=\"3\">%1</th></tr>\n").arg(i18n("Todays payments")));

      for(t_it = todays.begin(); t_it != todays.end(); ++t_it) {
        m_part->write(QString("<tr class=\"row-%1\">").arg(i++ & 0x01 ? "even" : "odd"));
        showPaymentEntry(*t_it);
        m_part->write("</tr>");
      }
      m_part->write("</table>");
    }

    if (schedule.count() > 0)
    {
      m_part->write("<div class=\"gap\">&nbsp;</div>\n");

      QValueList<MyMoneySchedule>::Iterator it;

      m_part->write("<table width=\"75%\" cellspacing=\"0\" cellpadding=\"2\">");
      m_part->write(QString("<tr class=\"item\"><th class=\"left\" colspan=\"3\">%1</th></tr>\n").arg(i18n("Future payments")));

      // show all or the first 6 entries
      int cnt;
      cnt = (m_showAllSchedules) ? -1 : 6;
      bool needMoreLess = m_showAllSchedules;

      QDate lastDate = QDate::currentDate().addMonths(1);
      qBubbleSort(schedule);
      do {
        it = schedule.begin();
        if(it == schedule.end())
          break;

        QDate nextDate = (*it).nextPayment((*it).lastPayment());
        if(!nextDate.isValid()) {
          schedule.remove(it);
          continue;
        }

        if (nextDate > lastDate)
          break;

        if(cnt == 0) {
          needMoreLess = true;
          break;
        }
        if(cnt > 0)
          --cnt;

        m_part->write(QString("<tr class=\"row-%1\">").arg(i++ & 0x01 ? "even" : "odd"));
        showPaymentEntry(*it);
        m_part->write("</tr>");

        (*it).setLastPayment((*it).nextPayment((*it).lastPayment()));
        qBubbleSort(schedule);
      }
      while(1);

      m_part->write("</table>");
      if (needMoreLess) {
        if(m_showAllSchedules) {
          m_part->write(link(VIEW_SCHEDULE,  QString("?mode=%1").arg("reduced")) + i18n("Less ...") + linkend());
        } else {
          m_part->write(link(VIEW_SCHEDULE,  QString("?mode=%1").arg("full")) + i18n("More ...") + linkend());
        }
      }
    }
  }
}

void KHomeView::showPaymentEntry(const MyMoneySchedule& sched, int cnt)
{
  QString tmp;
  try {
    MyMoneyAccount acc = sched.account();
    if(acc.id()) {
      MyMoneyTransaction t = sched.transaction();
      // only show the entry, if it is still active
      if(!sched.isFinished() && sched.nextPayment(sched.lastPayment()) != QDate()) {
        MyMoneySplit sp = t.splitByAccount(acc.id(), true);

        tmp = QString("<td width=\"20%\">") +
          KGlobal::locale()->formatDate(sched.nextPayment(sched.lastPayment()), true) +
          "</td><td width=\"70%\">" +
          link(VIEW_SCHEDULE, QString("?id=%1").arg(sched.id())) + sched.name() + linkend();
        if(cnt > 1)
          tmp += i18n(" (%1 payments)").arg(cnt);
        tmp += "</td><td width=\"10%\" align=\"right\">";

        MyMoneySecurity currency = MyMoneyFile::instance()->currency(acc.currencyId());
        QString amount = (sp.value()*cnt).formatMoney(currency.tradingSymbol());
        amount.replace(" ","&nbsp;");
        tmp += amount;
        tmp += "</td>";
        // qDebug("paymentEntry = '%s'", tmp.latin1());
        m_part->write(tmp);
      }
    }
  } catch(MyMoneyException* e) {
    qDebug("Unable to display schedule entry: %s", e->what().data());
    delete e;
  }
}

void KHomeView::showAccounts(KHomeView::paymentTypeE type, const QString& header)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  QValueList<MyMoneyAccount> accounts;
  QValueList<MyMoneyAccount>::Iterator it;
  QValueList<MyMoneyAccount>::Iterator prevIt;
  QMap<QString, MyMoneyAccount> nameIdx;

  bool showClosedAccounts = kmymoney2->toggleAction("view_show_all_accounts")->isChecked();

  // get list of all accounts
  file->accountList(accounts);
  for(it = accounts.begin(); it != accounts.end();) {
    prevIt = it;
    if(!(*it).isClosed() || showClosedAccounts) {
      switch((*it).accountType()) {
        case MyMoneyAccount::Expense:
        case MyMoneyAccount::Income:
          // never show a category account
          // Note: This might be different in a future version when
          //       the homepage also shows category based information
          it = accounts.remove(it);
          break;

        // Asset and Liability accounts are only shown if they
        // have the preferred flag set
        case MyMoneyAccount::Asset:
        case MyMoneyAccount::Liability:
        case MyMoneyAccount::Investment:
          // if preferred accounts are requested, then keep in list
          if((*it).value("PreferredAccount") != "Yes"
          || (type & Preferred) == 0) {
            it = accounts.remove(it);
          }
          break;

        // Check payment accounts. If payment and preferred is selected,
        // then always show them. If only payment is selected, then
        // show only if preferred flag is not set.
        case MyMoneyAccount::Checkings:
        case MyMoneyAccount::Savings:
        case MyMoneyAccount::Cash:
        case MyMoneyAccount::CreditCard:
          switch(type & (Payment | Preferred)) {
            case Payment:
              if((*it).value("PreferredAccount") == "Yes")
                it = accounts.remove(it);
              break;

            case Preferred:
              if((*it).value("PreferredAccount") != "Yes")
                it = accounts.remove(it);
              break;

            case Payment | Preferred:
              break;

            default:
              it = accounts.remove(it);
              break;
          }
          break;

        // filter all accounts that are not used on homepage views
        default:
          it = accounts.remove(it);
          break;
      }

    } else if((*it).isClosed()) {
      // don't show if closed
      it = accounts.remove(it);
    }

    // if we still point to the same account we keep it in the list and move on ;-)
    if(prevIt == it) {
      QString key = (*it).name();
      if(nameIdx[key].id().isEmpty()) {
        nameIdx[key] = *it;

      } else if(nameIdx[key].id() != (*it).id()) {
        key = (*it).name() + "[%1]";
        int dup = 2;
        while(nameIdx[key.arg(dup)].id().isEmpty()
        || nameIdx[key.arg(dup)].id() != (*it).id())
          ++dup;
        nameIdx[key.arg(dup)] = *it;
      }
      ++it;
    }
  }

  if(accounts.count() > 0) {
    QString tmp;
    int i = 0;
    tmp = "<div class=\"itemheader\">" + header +
          "</div>\n<div class=\"gap\">&nbsp;</div>\n";

    m_part->write(tmp);
    m_part->write("<table width=\"75%\" cellspacing=\"0\" cellpadding=\"2\">");
    m_part->write("<tr class=\"item\"><th class=\"left\" width=\"70%\">");
    m_part->write(i18n("Account"));
    m_part->write("</th><th width=\"30%\" class=\"right\">");
    m_part->write(i18n("Balance"));
    m_part->write("</th></tr>");

    QMap<QString, MyMoneyAccount>::const_iterator it_m;
    for(it_m = nameIdx.begin(); it_m != nameIdx.end(); ++it_m) {
      m_part->write(QString("<tr class=\"row-%1\">").arg(i++ & 0x01 ? "even" : "odd"));
      showAccountEntry(*it_m);
      m_part->write("</tr>");
    }
    m_part->write("</table>");
  }
}

void KHomeView::showAccountEntry(const MyMoneyAccount& acc)
{
  QString tmp;
  MyMoneySecurity currency = MyMoneyFile::instance()->currency(acc.currencyId());

  QString amount;
  MyMoneyMoney value;
  if(acc.accountType() == MyMoneyAccount::Investment) {
    value = MyMoneyFile::instance()->totalValue(acc.id(), QDate::currentDate());
    amount = value.formatMoney(currency.tradingSymbol());
  } else {
    value = MyMoneyFile::instance()->balance(acc.id(), QDate::currentDate());
    amount = value.formatMoney(currency.tradingSymbol());
  }
  amount.replace(" ","&nbsp;");

  tmp = QString("<td width=\"70%\">") +
      link(VIEW_LEDGER, QString("?id=%1").arg(acc.id())) + acc.name() + linkend() + "</td>";

  QString fontStart, fontEnd;
  if(value.isNegative()) {
    QColor x = KMyMoneySettings::listNegativeValueColor();
    fontStart.sprintf("<font color=\"#%02x%02x%02x\">", x.red(), x.green(), x.blue());
    fontEnd = "</font>";
  }
  tmp += QString("<td width=\"30%\" align=\"right\">%1%2%3</td>").arg(fontStart).arg(amount).arg(fontEnd);
  // qDebug("accountEntry = '%s'", tmp.latin1());
  m_part->write(tmp);
}

void KHomeView::showFavoriteReports(void)
{
  QValueList<MyMoneyReport> reports = MyMoneyFile::instance()->reportList();

  if ( reports.count() > 0 )
  {
    m_part->write(QString("<div class=\"itemheader\">%1</div>\n<div class=\"gap\">&nbsp;</div>\n").arg(i18n("Favorite Reports")));
    m_part->write("<table width=\"75%\" cellspacing=\"0\" cellpadding=\"2\">");
    m_part->write("<tr class=\"item\"><th class=\"left\" width=\"50%\">");
    m_part->write(i18n("Report"));
    m_part->write("</th><th width=\"50%\" class=\"right\">");
    m_part->write(i18n("Comment"));
    m_part->write("</th></tr>");

    int row = 0;
    QValueList<MyMoneyReport>::const_iterator it_report = reports.begin();
    while( it_report != reports.end() )
    {
      if ( (*it_report).isFavorite() )
        m_part->write(QString("<tr class=\"row-%1\"><td>%2%3%4</td><td align=\"right\">%5</td></tr>")
          .arg(row++ & 0x01 ? "even" : "odd")
          .arg(link(VIEW_REPORTS, QString("?id=%1").arg((*it_report).id())))
          .arg((*it_report).name())
          .arg(linkend())
          .arg((*it_report).comment())
        );

      ++it_report;
    }
    m_part->write("</table>");
  }
}

void KHomeView::showForecast(void)
{
  typedef QMap<int, MyMoneyMoney> dailyBalances;
  QMap<QCString, dailyBalances> accountList;
  QMap<QString, QCString> nameIdx;
  MyMoneyFile* file = MyMoneyFile::instance();
  const int forecastDays = 90;
  int  txnCount = 0;

  QDate endDate = QDate::currentDate().addDays(forecastDays);
  MyMoneyTransactionFilter filter;

  // collect and process all transactions that have already been entered but
  // are located in the future.
  filter.setDateFilter(QDate::currentDate().addDays(1), endDate);
  filter.setReportAllSplits(false);

  QValueList<MyMoneyTransaction> transactions = file->transactionList(filter);
  QValueList<MyMoneyTransaction>::const_iterator it_t = transactions.begin();

  for(; it_t != transactions.end(); ++it_t ) {
    const QValueList<MyMoneySplit>& splits = (*it_t).splits();
    QValueList<MyMoneySplit>::const_iterator it_s = splits.begin();
    for(; it_s != splits.end(); ++it_s ) {
      if(!(*it_s).shares().isZero()) {
        MyMoneyAccount acc = file->account((*it_s).accountId());
        if((acc.accountGroup() == MyMoneyAccount::Asset
        || acc.accountGroup() == MyMoneyAccount::Liability)
        && acc.accountType() != MyMoneyAccount::Investment) {
          dailyBalances balance;
          balance = accountList[acc.id()];
          int offset = QDate::currentDate().daysTo((*it_t).postDate())+1;
          balance[offset] += (*it_s).shares();
          // check if this is a new account for us
          if(nameIdx[acc.name()] != acc.id()) {
            nameIdx[acc.name()] = acc.id();
            balance[0] = file->balance(acc.id(), QDate::currentDate());
          }
          accountList[acc.id()] = balance;
        }
      }
    }
    ++txnCount;
  }

#if 0
  QFile trcFile("forecast.csv");
  trcFile.open(IO_WriteOnly);
  QTextStream s(&trcFile);

  {
    s << "Already present transactions\n";
    QMap<QCString, dailyBalances>::Iterator it_a;
    QMap<QString, QCString>::ConstIterator it_n;
    for(it_n = nameIdx.begin(); it_n != nameIdx.end(); ++it_n) {
      MyMoneyAccount acc = file->account(*it_n);
      it_a = accountList.find(*it_n);
      s << "\"" << acc.name() << "\",";
      for(int i = 0; i < 90; ++i) {
        s << "\"" << (*it_a)[i].formatMoney("") << "\",";
      }
      s << "\n";
    }
  }
#endif

  // now process all the schedules that may have an impact
  QValueList<MyMoneySchedule> schedule;

  schedule = file->scheduleList("", MyMoneySchedule::TYPE_ANY,
                                 MyMoneySchedule::OCCUR_ANY,
                                 MyMoneySchedule::STYPE_ANY,
                                 QDate::currentDate(),
                                 endDate);
  if(schedule.count() > 0) {
    qBubbleSort(schedule);

    QValueList<MyMoneySchedule>::Iterator it;
    do {
      it = schedule.begin();
      if(it == schedule.end())
        break;

      QDate nextDate = (*it).nextPayment((*it).lastPayment());
      if(!nextDate.isValid()) {
        schedule.remove(it);
        continue;
      }

      if (nextDate > endDate)
        break;

      // found the next schedule. process it
      MyMoneyAccount acc = (*it).account();
      if(!acc.id().isEmpty()) {
        try {
          if(acc.accountType() != MyMoneyAccount::Investment) {
            MyMoneyTransaction t = (*it).transaction();

            // only process the entry, if it is still active
            if(!(*it).isFinished() && nextDate != QDate()) {

              // make sure we have all 'starting balances' so that the autocalc works
              QValueList<MyMoneySplit>::const_iterator it_s;
              QMap<QCString, MyMoneyMoney> balanceMap;

              for(it_s = t.splits().begin(); it_s != t.splits().end(); ++it_s ) {
                MyMoneyAccount acc = file->account((*it_s).accountId());
                if(acc.accountGroup() == MyMoneyAccount::Asset
                || acc.accountGroup() == MyMoneyAccount::Liability) {
                  // check if this is a new account for us
                  if(nameIdx[acc.name()] != acc.id()) {
                    nameIdx[acc.name()] = acc.id();
                    accountList[acc.id()][0] = file->balance(acc.id());
                  }
                  int offset = QDate::currentDate().daysTo(nextDate)+1;
                  if(offset <= 0) {  // collect all overdues on the first day
                    offset = 1;
                  }
                  for(int i = 0; i < offset; ++i) {
                    balanceMap[acc.id()] += accountList[acc.id()][i];
                  }
                }
              }

              // take care of the autoCalc stuff
              KMyMoneyUtils::calculateAutoLoan(*it, t, balanceMap);

              // now add the splits to the balances
              for(it_s = t.splits().begin(); it_s != t.splits().end(); ++it_s ) {
                MyMoneyAccount acc = file->account((*it_s).accountId());
                if(acc.accountGroup() == MyMoneyAccount::Asset
                || acc.accountGroup() == MyMoneyAccount::Liability) {
                  dailyBalances balance;
                  balance = accountList[acc.id()];
                  int offset = QDate::currentDate().daysTo(nextDate)+1;
                  if(offset <= 0) {  // collect all overdues on the first day
                    offset = 1;
                  }
                  balance[offset] += (*it_s).value();
                  accountList[acc.id()] = balance;
                }
              }
              ++txnCount;
            }
          }
          (*it).setLastPayment(nextDate);

        } catch(MyMoneyException* e) {
          kdDebug(2) << __func__ << " Schedule " << (*it).id() << " (" << (*it).name() << "): " << e->what() << endl;

          schedule.remove(it);
          delete e;
        }
      } else {
        // remove schedule from list
        schedule.remove(it);
      }

      qBubbleSort(schedule);
    }
    while(1);
  }

#if 0
  {
    s << "\n\nAdded scheduled transactions\n";
    QMap<QCString, dailyBalances>::Iterator it_a;
    QMap<QString, QCString>::ConstIterator it_n;
    for(it_n = nameIdx.begin(); it_n != nameIdx.end(); ++it_n) {
      MyMoneyAccount acc = file->account(*it_n);
      it_a = accountList.find(*it_n);
      s << "\"" << acc.name() << "\",";
      for(int i = 0; i < 90; ++i) {
        s << "\"" << (*it_a)[i].formatMoney("") << "\",";
      }
      s << "\n";
    }
  }
#endif

  if(accountList.count() > 0) {
    int i = 0;

    // Now output header
    m_part->write(QString("<div class=\"itemheader\">%1</div>\n<div class=\"gap\">&nbsp;</div>\n").arg(i18n("90 day forecast")));
    m_part->write("<table width=\"95%\" cellspacing=\"0\" cellpadding=\"2\">");
    m_part->write("<tr class=\"item\"><th class=\"left\" width=\"40%\">");
    m_part->write(i18n("Account"));
    m_part->write("</th>");
    for(i = 0; i < 3; ++i) {
      m_part->write("<th width=\"20%\" class=\"right\">");
      // m_part->write(QString("%1").arg(KGlobal::locale()->formatDate(QDate::currentDate().addDays((i+1)*30), true)));
      m_part->write(i18n("%1 days").arg((i+1)*30));
      m_part->write("</th>");
    }
    m_part->write("</tr>");

    // Now output entries
    i = 0;
    QMap<QCString, dailyBalances>::Iterator it_a;
    QMap<QString, QCString>::ConstIterator it_n;
    for(it_n = nameIdx.begin(); it_n != nameIdx.end(); ++it_n) {
      MyMoneyAccount acc = file->account(*it_n);
      it_a = accountList.find(*it_n);
      m_part->write(QString("<tr class=\"row-%1\">").arg(i++ & 0x01 ? "even" : "odd"));
      m_part->write(QString("<td width=\"40%\">") +
        link(VIEW_LEDGER, QString("?id=%1").arg(acc.id())) + acc.name() + linkend() + "</td>");

      dailyBalances::Iterator it_b = (*it_a).begin();
      MyMoneyMoney runningSum;
      int dropZero = -1, dropMinimum = -1;
      QString minimumBalance = acc.value("minimumBalance");
      MyMoneySecurity currency = file->security(acc.currencyId());

      for(int limit = 30; limit < 91; limit += 30) {
        while(it_b != (*it_a).end() && it_b.key() < limit) {
          runningSum += (*it_b);

          // check for the running sum going beyond the minimum balance for the first time
          if(!minimumBalance.isEmpty()
          && dropZero == -1
          && dropMinimum == -1
          && acc.accountGroup() == MyMoneyAccount::Asset
          && runningSum < MyMoneyMoney(minimumBalance)) {
            dropMinimum = it_b.key();
          }

          // check for the running sum going beyond 0 for the first time
          if(dropZero == -1
          && acc.accountGroup() == MyMoneyAccount::Asset
          && runningSum < MyMoneyMoney(0, 1)) {
            dropZero = it_b.key();
          }
          ++it_b;
        }
        QString amount;
        amount = runningSum.formatMoney(currency.tradingSymbol());
        amount.replace(" ","&nbsp;");
        m_part->write(QString("<td width=\"20%\" align=\"right\">%1</td>").arg(amount));
      }
      // tmp += QString("<td width=\"25%\" align=\"right\">%1</td>").arg(amount);
      m_part->write("</tr>");

      // spit out possible warnings
      QString msg;

      // if a minimum balance has been specified, an appropriate warning will
      // only be shown, if the drop below 0 is on a different day or not present
      MyMoneyMoney minBalance = MyMoneyMoney(minimumBalance);
      if(dropMinimum != -1
      && !minBalance.isZero()
      && (dropMinimum < dropZero
       || dropZero == -1)) {
        switch(dropMinimum) {
          case -1:
            break;
          case 0:
            msg = i18n("The balance of %1 is below the minimum balance %2 today.").arg(acc.name()).arg(minBalance.formatMoney(currency.tradingSymbol()));
            break;
          default:
            msg = i18n("The balance of %1 will drop below the minimum balance %2 in %3 days.").arg(acc.name()).arg(minBalance.formatMoney(currency.tradingSymbol())).arg(dropMinimum-1);
        }
        if(!msg.isEmpty()) {
          m_part->write(QString("<tr><td colspan=4 align=\"center\" ><font color=\"red\">%1</font></td></tr>").arg(msg));
        }
      }
      // a drop below zero is always shown
      msg = QString();
      switch(dropZero) {
        case -1:
          break;
        case 0:
          msg = i18n("The balance of %1 is below %2 today.").arg(acc.name()).arg(MyMoneyMoney().formatMoney(currency.tradingSymbol()));
          break;
        default:
          msg = i18n("The balance of %1 will drop below %2 in %3 days.").arg(acc.name()).arg(MyMoneyMoney().formatMoney(currency.tradingSymbol())).arg(dropZero-1);
      }
      if(!msg.isEmpty()) {
        m_part->write(QString("<tr><td colspan=4 align=\"center\" ><font color=\"red\"><b>%1</b></font></td></tr>").arg(msg));
      }
    }
    m_part->write("</table>");

  }
}

const QString KHomeView::link(const QString& view, const QString& query) const
{
  return QString("<a href=\"/") + view + query + "\">";
}

const QString KHomeView::linkend(void) const
{
  return "</a>";
}

void KHomeView::slotOpenURL(const KURL &url, const KParts::URLArgs& /* args */)
{
  QString protocol = url.protocol();
  QString view = url.fileName(false);
  QCString id = url.queryItem("id").data();
  QCString mode = url.queryItem("mode").data();

  if ( protocol == "http" )
  {
    KApplication::kApplication()->invokeBrowser(url.prettyURL());
  }
  else if ( protocol == "mailto" )
  {
    KApplication::kApplication()->invokeMailer(url);
  }
  else
  {
    if(view == VIEW_LEDGER) {
      emit ledgerSelected(id, QCString());

    } else if(view == VIEW_SCHEDULE) {
      if(!id.isEmpty())
        emit scheduleSelected(id);
      if(!mode.isEmpty()) {
        m_showAllSchedules = (mode == QCString("full"));
        loadView();
      }

    } else if(view == VIEW_REPORTS) {
      emit reportSelected(id);

    } else if(view == VIEW_WELCOME) {
      KMainWindow* mw = dynamic_cast<KMainWindow*>(qApp->mainWidget());
      Q_CHECK_PTR(mw);
      if ( mode == "whatsnew" )
      {
        QString fname = KMyMoneyUtils::findResource("appdata",QString("html/whats_new%1.html"));
        if(!fname.isEmpty())
          m_part->openURL(fname);
      }
      else
        m_part->openURL(m_filename);

    } else if(view == "action") {
        KMainWindow* mw = dynamic_cast<KMainWindow*>(qApp->mainWidget());
        Q_CHECK_PTR(mw);
        QTimer::singleShot(0, mw->actionCollection()->action( id ), SLOT(activate()));

    } else if(view == VIEW_HOME) {
      loadView();

    } else {
      qDebug("Unknown view '%s' in KHomeView::slotOpenURL()", view.latin1());
    }
  }
}

// Make sure, that these definitions are only used within this file
// this does not seem to be necessary, but when building RPMs the
// build option 'final' is used and all CPP files are concatenated.
// So it could well be, that in another CPP file these definitions
// are also used.
#undef VIEW_LEDGER
#undef VIEW_SCHEDULE
#undef VIEW_WELCOME
#undef VIEW_HOME
#undef VIEW_REPORTS

#include "khomeview.moc"
