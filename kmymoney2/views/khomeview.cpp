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
#include "../kmymoneyglobalsettings.h"
#include "../mymoney/mymoneyfile.h"
#include "../mymoney/mymoneyforecast.h"
#include "../kmymoney2.h"
#include "../reports/kreportchartview.h"
#include "../reports/pivottable.h"
#include "../reports/pivotgrid.h"
#include "../reports/reportaccount.h"
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

using namespace reports;

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
  if (KMyMoneyGlobalSettings::rememberFontSize())
  {
    KMyMoneyGlobalSettings::setFontSizePercentage(m_part->zoomFactor());
    //kdDebug() << "Storing font size: " << m_part->zoomFactor() << endl;
    KMyMoneyGlobalSettings::self()->writeConfig();
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
  m_part->setZoomFactor( KMyMoneyGlobalSettings::fontSizePercentage() );
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
          case 5:         // forecast
            showForecast();
            break;
          case 6:         // net worth graph over all accounts
            showNetWorthGraph();
            break;
          case 8:         // summary
              showSummary();
              break;
          case 9:         // budget
              showBudget();
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
  reportCfg.setIncludingSchedules( false );
  reportCfg.addAccountGroup(MyMoneyAccount::Asset);
  reportCfg.addAccountGroup(MyMoneyAccount::Liability);
  reportCfg.setColumnsAreDays( true );
  reportCfg.setConvertCurrency( true );
  reportCfg.setIncludingForecast( true );
  reportCfg.setDateFilter(QDate::currentDate(),QDate::currentDate().addDays(+90));

  reports::PivotTable table(reportCfg);

  reports::KReportChartView* chartWidget = new reports::KReportChartView(0, 0);

  table.drawChart(*chartWidget);

  chartWidget->params().setLineMarker(false);
  chartWidget->params().setLegendPosition(KDChartParams::NoLegend);
  chartWidget->params().setLineWidth(2);
  chartWidget->params().setDataColor(0, KGlobalSettings::textColor());

    // draw future values in a different line style
  KDChartPropertySet propSetFutureValue("future value", KMM_KDCHART_PROPSET_NORMAL_DATA);
  propSetFutureValue.setLineStyle(KDChartPropertySet::OwnID, Qt::DotLine);
  const int idPropFutureValue = chartWidget->params().registerProperties(propSetFutureValue);

  //KDChartPropertySet propSetLastValue("last value", idPropFutureValue);
  //propSetLastValue.setExtraLinesAlign(KDChartPropertySet::OwnID, Qt::AlignLeft | Qt::AlignBottom);
  //propSetLastValue.setExtraLinesWidth(KDChartPropertySet::OwnID, -4);
  //propSetLastValue.setExtraLinesColor(KDChartPropertySet::OwnID, KMyMoneyGlobalSettings::listGridColor());
  // propSetLastValue.setShowMarker(KDChartPropertySet::OwnID, true);
  // propSetLastValue.setMarkerStyle(KDChartPropertySet::OwnID, KDChartParams::LineMarkerDiamond);

  //const int idPropLastValue = chartWidget->params().registerProperties(propSetLastValue);
  for(int iCell = 0; iCell < 90; ++iCell) {
    chartWidget->setProperty(0, iCell, idPropFutureValue);
  }
  //chartWidget->setProperty(0, 10, idPropLastValue);

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
    // FIXME cleanup old code
    // if ((*d_it).isFinished() || (*d_it).nextPayment((*d_it).lastPayment()) == QDate())
    if ((*d_it).isFinished())
    {
      d_it = schedule.remove(d_it);
      continue;
    }
    ++d_it;
  }

  for (d_it=overdues.begin(); d_it!=overdues.end();)
  {
    // FIXME cleanup old code
    // if ((*d_it).isFinished() || (*d_it).nextPayment((*d_it).lastPayment()) == QDate())
    if ((*d_it).isFinished())
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
      QDate nextDate = (*it).nextDueDate();
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
    for (t_it=schedule.begin(); t_it!=schedule.end();) {
      if ((*t_it).nextDueDate() == QDate::currentDate()) {
        todays.append(*t_it);
        (*t_it).setNextDueDate((*t_it).nextPayment((*t_it).nextDueDate()));
      }
      ++t_it;
    }

    if (todays.count() > 0) {
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

        // if the next due date is invalid (schedule is finished)
        // we remove it from the list
        QDate nextDate = (*it).nextDueDate();
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

        // for single occurence we have reported everything so we
        // better get out of here.
        if((*it).occurence() == MyMoneySchedule::OCCUR_ONCE) {
          schedule.remove(it);
          continue;
        }

        (*it).setNextDueDate((*it).nextPayment((*it).nextDueDate()));
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
      // FIXME clean old code
      // if(!sched.isFinished() && sched.nextPayment(sched.lastPayment()) != QDate()) {
      if(!sched.isFinished()) {
        MyMoneySplit sp = t.splitByAccount(acc.id(), true);

        tmp = QString("<td width=\"20%\">") +
          KGlobal::locale()->formatDate(sched.nextDueDate(), true) +
          "</td><td width=\"70%\">" +
          link(VIEW_SCHEDULE, QString("?id=%1").arg(sched.id())) + sched.name() + linkend();
        if(cnt > 1)
          tmp += i18n(" (%1 payments)").arg(cnt);
        tmp += "</td><td width=\"10%\" align=\"right\">";

        const MyMoneySecurity& currency = MyMoneyFile::instance()->currency(acc.currencyId());
        QString amount = (sp.value()*cnt).formatMoney(acc, currency);
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

    } else if((*it).isClosed() || (*it).isInvest()) {
      // don't show if closed or a stock account
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
        while(!nameIdx[key.arg(dup)].id().isEmpty()
        && nameIdx[key.arg(dup)].id() != (*it).id())
          ++dup;
        nameIdx[key.arg(dup)] = *it;
      }
      ++it;
    }
  }

  if(accounts.count() > 0) {
    QString tmp;
    int i = 0;
    tmp = "<div class=\"itemheader\">" + header + "</div>\n<div class=\"gap\">&nbsp;</div>\n";
    m_part->write(tmp);
    m_part->write("<table width=\"75%\" cellspacing=\"0\" cellpadding=\"2\">");
    m_part->write("<tr class=\"item\"><th class=\"left\" width=\"40%\">");
    m_part->write(i18n("Account"));
    m_part->write("</th><th width=\"30%\" class=\"right\">");
    m_part->write(i18n("Balance"));
    m_part->write("</th>");
    m_part->write("</th><th width=\"30%\" class=\"right\">");
    m_part->write(i18n("To Minimum Balance"));
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
  MyMoneyFile* file = MyMoneyFile::instance();
  QString tmp;
  MyMoneySecurity currency = file->currency(acc.currencyId());
  QString minimumBalance = acc.value("minBalanceAbsolute");
  MyMoneyMoney minBalance = MyMoneyMoney(minimumBalance);
  MyMoneyMoney valueToMinBal;
  QString amount;
  QString amountToMinBal;
  MyMoneyMoney value;

  if(acc.accountType() == MyMoneyAccount::Investment) {
    //investment accounts show the balances of all its subaccounts
    value = investmentBalance(acc);
    amount = value.formatMoney(acc, currency);
    //investment accounts have no minimum balance
    showAccountEntry(acc, value, MyMoneyMoney(), true);
  } else {
    //get balance for normal accounts
    value = MyMoneyFile::instance()->balance(acc.id(), QDate::currentDate());
    valueToMinBal = value - minBalance;
    showAccountEntry(acc, value, valueToMinBal, true);
  }
}

void KHomeView::showAccountEntry(const MyMoneyAccount& acc, const MyMoneyMoney& value, const MyMoneyMoney& valueToMinBal, const bool showMinBal)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  QString tmp;
  MyMoneySecurity currency = file->currency(acc.currencyId());
  QString amount;
  QString amountToMinBal;

  //format amounts
  amount = value.formatMoney(acc, currency);
  amount.replace(" ","&nbsp;");
  if(showMinBal) {
    amountToMinBal = valueToMinBal.formatMoney(acc, currency);
    amountToMinBal.replace(" ","&nbsp;");
  }

  tmp = QString("<td>") +
      link(VIEW_LEDGER, QString("?id=%1").arg(acc.id())) + acc.name() + linkend() + "</td>";

  //show account balance
  tmp += QString("<td align=\"right\">%1</td>").arg(showColoredAmount(amount, value.isNegative()));

  //show minimum balance column if requested
  if(showMinBal) {
    //if it is an investment, show minimum balance empty
    if(acc.accountType() == MyMoneyAccount::Investment) {
      tmp += QString("<td align=\"right\">&nbsp;</td>");
    } else {
      //show minimum balance entry
      tmp += QString("<td align=\"right\">%1</td>").arg(showColoredAmount(amountToMinBal, valueToMinBal.isNegative()));
    }
  }
  // qDebug("accountEntry = '%s'", tmp.latin1());
  m_part->write(tmp);
}

MyMoneyMoney KHomeView::investmentBalance(const MyMoneyAccount& acc)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  MyMoneyMoney value;
  MyMoneySecurity currency = file->currency(acc.currencyId());

  value = file->balance(acc.id());
  try {
    if(currency.id() != file->baseCurrency().id()) {
        //convert current balance
      value = value * file->price(currency.id(), file->baseCurrency().id()).rate(file->baseCurrency().id());
      value.convert(file->baseCurrency().smallestAccountFraction());
    }
  } catch(MyMoneyException* e) {
    qWarning("%s", (QString("cannot convert balance to base currency: %1").arg(e->what())).data());
    delete e;
  }
  QValueList<QCString>::const_iterator it_a;
  for(it_a = acc.accountList().begin(); it_a != acc.accountList().end(); ++it_a) {
    MyMoneyAccount stock = file->account(*it_a);
    try {
      MyMoneyMoney val;
      MyMoneyMoney balance = file->balance(stock.id());
      MyMoneySecurity security = file->security(stock.currencyId());
      MyMoneyPrice price = file->price(stock.currencyId(), security.tradingCurrency());
      val = balance * price.rate(security.tradingCurrency());

      /*if(security.tradingCurrency() != file->baseCurrency().id()) {
        MyMoneySecurity sec = file->currency(security.tradingCurrency());
        val = val * file->price(security.tradingCurrency(), file->baseCurrency().id()).rate(file->baseCurrency().id());
      }*/
      val = val.convert(acc.fraction());
      value += val;
    } catch(MyMoneyException* e) {
      qWarning("%s", (QString("cannot convert stock balance of %1 to base currency: %2").arg(stock.name(), e->what())).data());
      delete e;
    }
  }
  return value;
}

void KHomeView::showFavoriteReports(void)
{
  QValueList<MyMoneyReport> reports = MyMoneyFile::instance()->reportList();

  if ( reports.count() > 0 )
  {
    bool firstTime = 1;
    int row = 0;
    QValueList<MyMoneyReport>::const_iterator it_report = reports.begin();
    while( it_report != reports.end() )
    {
      if ( (*it_report).isFavorite() ) {
        if(firstTime) {
          m_part->write(QString("<div class=\"itemheader\">%1</div>\n<div class=\"gap\">&nbsp;</div>\n").arg(i18n("Favorite Reports")));
          m_part->write("<table width=\"75%\" cellspacing=\"0\" cellpadding=\"2\">");
          m_part->write("<tr class=\"item\"><th class=\"left\" width=\"50%\">");
          m_part->write(i18n("Report"));
          m_part->write("</th><th width=\"50%\" class=\"right\">");
          m_part->write(i18n("Comment"));
          m_part->write("</th></tr>");
          firstTime = false;
        }

        m_part->write(QString("<tr class=\"row-%1\"><td>%2%3%4</td><td align=\"right\">%5</td></tr>")
          .arg(row++ & 0x01 ? "even" : "odd")
          .arg(link(VIEW_REPORTS, QString("?id=%1").arg((*it_report).id())))
          .arg((*it_report).name())
          .arg(linkend())
          .arg((*it_report).comment())
        );
      }

      ++it_report;
    }
    if(!firstTime)
      m_part->write("</table>");
  }
}

void KHomeView::showForecast(void)
{
  QMap<QCString, QCString> nameIdx;
  MyMoneyFile* file = MyMoneyFile::instance();
  QValueList<MyMoneyAccount> accList;
  MyMoneyForecast forecast;

  //If forecastDays lower than accountsCycle, adjust to the first cycle
  if(forecast.accountsCycle() > forecast.forecastDays())
    forecast.setForecastDays(forecast.accountsCycle());

  //Get all accounts of the right type to calculate forecast
  forecast.doForecast();
  accList = forecast.accountList();
  QValueList<MyMoneyAccount>::const_iterator accList_t = accList.begin();
  for ( ; accList_t != accList.end(); ++accList_t )
  {
    MyMoneyAccount acc = *accList_t;
    if ( nameIdx[acc.id() ] != acc.id() ) { //Check if the account is there
      nameIdx[acc.id() ] = acc.id();

    }
  }

  if(nameIdx.count() > 0) {
    int i = 0;

    int colspan = 1;
    //get begin day
    int beginDay = QDate::currentDate().daysTo(forecast.beginForecastDate());
    //if begin day is today skip to next cycle
    if(beginDay == 0)
      beginDay = forecast.accountsCycle();

    // Now output header
    m_part->write(QString("<div class=\"itemheader\">%1</div>\n<div class=\"gap\">&nbsp;</div>\n").arg(i18n("%1 day forecast").arg(forecast.forecastDays())));
    m_part->write("<table width=\"95%\" cellspacing=\"0\" cellpadding=\"2\">");
    m_part->write("<tr class=\"item\"><th class=\"left\" width=\"40%\">");
    m_part->write(i18n("Account"));
    m_part->write("</th>");
    int colWidth = 55/ (forecast.forecastDays() / forecast.accountsCycle());
    for(i = 0; (i*forecast.accountsCycle() + beginDay) <= forecast.forecastDays(); ++i) {
      m_part->write(QString("<th width=\"%1%\" class=\"right\">").arg(colWidth));

      m_part->write(i18n("%1 days").arg(i*forecast.accountsCycle() + beginDay));
      m_part->write("</th>");
      colspan++;
    }
    m_part->write("</tr>");

    // Now output entries
    i = 0;

    QMap<QCString, QCString>::ConstIterator it_n;
    for(it_n = nameIdx.begin(); it_n != nameIdx.end(); ++it_n) {
      MyMoneyAccount acc = file->account(*it_n);

      m_part->write(QString("<tr class=\"row-%1\">").arg(i++ & 0x01 ? "even" : "odd"));
      m_part->write(QString("<td width=\"40%\">") +
          link(VIEW_LEDGER, QString("?id=%1").arg(acc.id())) + acc.name() + linkend() + "</td>");

      int dropZero = -1; //account dropped below zero
      int dropMinimum = -1; //account dropped below minimum balance
      QString minimumBalance = acc.value("minimumBalance");
      MyMoneyMoney minBalance = MyMoneyMoney(minimumBalance);
      MyMoneySecurity currency;
      MyMoneyMoney forecastBalance;

      //change account to deep currency if account is an investment
      if(acc.isInvest()) {
        MyMoneySecurity underSecurity = file->security(acc.currencyId());
        currency = file->security(underSecurity.tradingCurrency());
      } else {
        currency = file->security(acc.currencyId());
      }

      for (int f = beginDay; f <= forecast.forecastDays(); f += forecast.accountsCycle()) {
        forecastBalance = forecast.forecastBalance(acc, QDate::currentDate().addDays(f));
        QString amount;
        amount = forecastBalance.formatMoney(acc, currency);
        amount.replace(" ","&nbsp;");
        m_part->write(QString("<td width=\"%1%\" align=\"right\">").arg(colWidth));
        m_part->write(QString("%1</td>").arg(showColoredAmount(amount, forecastBalance.isNegative())));
      }

      m_part->write("</tr>");

      //Check if the account is going to be below zero or below the minimal balance in the forecast period

      //Check if the account is going to be below minimal balance
      dropMinimum = forecast.daysToMinimumBalance(acc);

      //Check if the account is going to be below zero in the future
      dropZero = forecast.daysToZeroBalance(acc);


      // spit out possible warnings
      QString msg;

      // if a minimum balance has been specified, an appropriate warning will
      // only be shown, if the drop below 0 is on a different day or not present

      if(dropMinimum != -1
         && !minBalance.isZero()
         && (dropMinimum < dropZero
         || dropZero == -1)) {
        switch(dropMinimum) {
          case -1:
            break;
          case 0:
            msg = i18n("The balance of %1 is below the minimum balance %2 today.").arg(acc.name()).arg(minBalance.formatMoney(acc, currency));
            break;
          default:
            msg = i18n("The balance of %1 will drop below the minimum balance %2 in %3 days.").arg(acc.name()).arg(minBalance.formatMoney(acc, currency)).arg(dropMinimum-1);
        }

        if(!msg.isEmpty()) {
          m_part->write(QString("<tr class=\"row-even\"><td colspan=%2 align=\"center\" ><font color=\"red\">%1</font></td></tr>").arg(msg).arg(colspan));
        }
         }
      // a drop below zero is always shown
         msg = QString();
         switch(dropZero) {
           case -1:
             break;
           case 0:
             if(acc.accountGroup() == MyMoneyAccount::Asset) {
               msg = i18n("The balance of %1 is below %2 today.").arg(acc.name()).arg(MyMoneyMoney().formatMoney(acc, currency));
               break;
             }
             if(acc.accountGroup() == MyMoneyAccount::Liability) {
               msg = i18n("The balance of %1 is above %2 today.").arg(acc.name()).arg(MyMoneyMoney().formatMoney(acc, currency));
               break;
             }
             break;
           default:
             if(acc.accountGroup() == MyMoneyAccount::Asset) {
               msg = i18n("The balance of %1 will drop below %2 in %3 days.").arg(acc.name()).arg(MyMoneyMoney().formatMoney(acc, currency)).arg(dropZero);
               break;
             }
             if(acc.accountGroup() == MyMoneyAccount::Liability) {
               msg = i18n("The balance of %1 will raise above %2 in %3 days.").arg(acc.name()).arg(MyMoneyMoney().formatMoney(acc, currency)).arg(dropZero);
               break;
             }
         }
         if(!msg.isEmpty()) {
           m_part->write(QString("<tr class=\"row-even\"><td colspan=%2 align=\"center\" ><font color=\"red\"><b>%1</b></font></td></tr>").arg(msg).arg(colspan));
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

void KHomeView::showSummary(void)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  QValueList<MyMoneyAccount> accounts;
  QValueList<MyMoneyAccount>::Iterator it;
  QMap<QString, MyMoneyAccount> nameAssetsIdx;
  QMap<QString, MyMoneyAccount> nameLiabilitiesIdx;
  MyMoneyMoney netAssets;
  MyMoneyMoney netLiabilities;
  QString fontStart, fontEnd;
  int prec = MyMoneyMoney::denomToPrec(file->baseCurrency().smallestAccountFraction());
  int i = 0;


  // get list of all accounts
  file->accountList(accounts);
  for(it = accounts.begin(); it != accounts.end();) {
    if(!(*it).isClosed()) {
      switch((*it).accountType()) {
        //group all assets into one list
        case MyMoneyAccount::Checkings:
        case MyMoneyAccount::Savings:
        case MyMoneyAccount::Cash:
        case MyMoneyAccount::Investment:
        case MyMoneyAccount::Asset:
        case MyMoneyAccount::AssetLoan:
        {
          //add it to a map to have it ordered by name
          QString key = (*it).name();
          if(nameAssetsIdx[key].id().isEmpty()) {
            nameAssetsIdx[key] = *it;
            //take care of accounts with duplicate names
          } else if(nameAssetsIdx[key].id() != (*it).id()) {
            key = (*it).name() + "[%1]";
            int dup = 2;
            while(!nameAssetsIdx[key.arg(dup)].id().isEmpty()
                   && nameAssetsIdx[key.arg(dup)].id() != (*it).id())
              ++dup;
            nameAssetsIdx[key.arg(dup)] = *it;
          }
          break;
        }
        //group the liabilities into the other
        case MyMoneyAccount::CreditCard:
        case MyMoneyAccount::Liability:
        case MyMoneyAccount::Loan:
        {
          //add it to a map to have it ordered by name
          QString key = (*it).name();
          if(nameLiabilitiesIdx[key].id().isEmpty()) {
            nameLiabilitiesIdx[key] = *it;
            //take care of duplicate account names
          } else if(nameLiabilitiesIdx[key].id() != (*it).id()) {
            key = (*it).name() + "[%1]";
            int dup = 2;
            while(!nameLiabilitiesIdx[key.arg(dup)].id().isEmpty()
                   && nameLiabilitiesIdx[key.arg(dup)].id() != (*it).id())
              ++dup;
            nameLiabilitiesIdx[key.arg(dup)] = *it;
          }
          break;
        }
        default:
          break;
      }
    }
    ++it;
  }

  //only do it if we have assets or liabilities account
  if(nameAssetsIdx.count() > 0 || nameLiabilitiesIdx.count() > 0) {
    //print header
    m_part->write("<div class=\"itemheader\">" + i18n("Summary") + "</div>\n<div class=\"gap\">&nbsp;</div>\n");
    m_part->write("<table width=\"100%\" cellspacing=\"0\" cellpadding=\"2\">");
    //asset and liability titles
    m_part->write("<tr class=\"item\"><th class=\"center\" colspan=\"2\">");
    m_part->write(i18n("Assets"));
    m_part->write("<th></th>");
    m_part->write("</th><th class=\"center\" colspan=\"2\">");
    m_part->write(i18n("Liabilities"));
    m_part->write("</th></tr>");
    //column titles
    m_part->write("<tr class=\"item\"><th class=\"left\" width=\"30%\">");
    m_part->write(i18n("Accounts"));
    m_part->write("</th><th width=\"15%\" class=\"right\">");
    m_part->write(i18n("Balance"));
    //intermediate row to separate both columns
    m_part->write("<th width=\"10%\"></th>");
    m_part->write("</th>");
    m_part->write("<th class=\"left\" width=\"30%\">");
    m_part->write(i18n("Accounts"));
    m_part->write("</th><th width=\"15%\" class=\"right\">");
    m_part->write(i18n("Balance"));
    m_part->write("</th></tr>");

    //get asset and liability accounts
    QMap<QString, MyMoneyAccount>::const_iterator asset_it = nameAssetsIdx.begin();
    QMap<QString,MyMoneyAccount>::const_iterator liabilities_it = nameLiabilitiesIdx.begin();
    for(; asset_it != nameAssetsIdx.end() || liabilities_it != nameLiabilitiesIdx.end();) {
      m_part->write(QString("<tr class=\"row-%1\">").arg(i++ & 0x01 ? "even" : "odd"));
      //write an asset account if we still have any
      if(asset_it != nameAssetsIdx.end()) {
        MyMoneyMoney value;
        //investment accounts consolidate the balance of its subaccounts
        if( (*asset_it).accountType() == MyMoneyAccount::Investment) {
          value = investmentBalance(*asset_it);
        } else {
        value = MyMoneyFile::instance()->balance((*asset_it).id(), QDate::currentDate());
        }
        //calculate balance for foreign currency accounts
        if((*asset_it).currencyId() != file->baseCurrency().id()) {
          ReportAccount repAcc = ReportAccount((*asset_it).id());
          MyMoneyMoney curPrice = repAcc.baseCurrencyPrice(QDate::currentDate());
          MyMoneyMoney baseValue = value * curPrice;
          netAssets += baseValue;
        } else {
          netAssets += value;
        }
        //show the account without minimum balance
        showAccountEntry(*asset_it, value, MyMoneyMoney(), false);
        ++asset_it;
      } else {
        //write a white space if we don't
        m_part->write("<td></td><td></td>");
      }

      //leave the intermediate column empty
      m_part->write("<td></td>");

      //write a liability account
      if(liabilities_it != nameLiabilitiesIdx.end()) {
        MyMoneyMoney value;
        value = MyMoneyFile::instance()->balance((*liabilities_it).id(), QDate::currentDate());
        //calculate balance if foreign currency
        if((*liabilities_it).currencyId() != file->baseCurrency().id()) {
          ReportAccount repAcc = ReportAccount((*liabilities_it).id());
          MyMoneyMoney curPrice = repAcc.baseCurrencyPrice(QDate::currentDate());
          MyMoneyMoney baseValue = value * curPrice;
          netLiabilities += baseValue;
        } else {
          netLiabilities += value;
        }
        //show the account without minimum balance
        showAccountEntry(*liabilities_it, value, MyMoneyMoney(), false);
        ++liabilities_it;
      } else {
        //leave the space empty if we run out of liabilities
        m_part->write("<td></td><td></td>");
      }
      m_part->write("</tr>");
    }
    //calculate net worth
    MyMoneyMoney netWorth = netAssets+netLiabilities;

    //format assets, liabilities and net worth
    QString amountAssets = netAssets.formatMoney(file->baseCurrency().tradingSymbol(), prec);
    QString amountLiabilities = netLiabilities.formatMoney(file->baseCurrency().tradingSymbol(), prec);
    QString amountNetWorth = netWorth.formatMoney(file->baseCurrency().tradingSymbol(), prec);
    amountAssets.replace(" ","&nbsp;");
    amountLiabilities.replace(" ","&nbsp;");
    amountNetWorth.replace(" ","&nbsp;");

    m_part->write(QString("<tr class=\"row-%1\" style=\"font-weight:bold;\">").arg(i++ & 0x01 ? "even" : "odd"));

    //print total for assets
    m_part->write(QString("<td class=\"left\">%1</td><td align=\"right\">%2</td>").arg(i18n("Total Assets")).arg(showColoredAmount(amountAssets, netAssets.isNegative())));

    //leave the intermediate column empty
    m_part->write("<td></td>");

    //print total liabilities
    m_part->write(QString("<td class=\"left\">%1</td><td align=\"right\">%2</td>").arg(i18n("Total Liabilities")).arg(showColoredAmount(amountLiabilities, netLiabilities.isNegative())));
    m_part->write("</tr>");

    //print net worth
    m_part->write(QString("<tr class=\"row-%1\" style=\"font-weight:bold;\">").arg(i++ & 0x01 ? "even" : "odd"));

      m_part->write(QString("<td class=\"left\">%1</td><td align=\"right\">%2</td><td></td><td></td><td></td>").arg(i18n("Net Worth")).arg(showColoredAmount(amountNetWorth, netWorth.isNegative() )));
    m_part->write("</tr>");

  }

  //Add total income and expenses for this month
  MyMoneyTransactionFilter filter;
  MyMoneyMoney incomeValue;
  MyMoneyMoney expenseValue;
  QDate startOfMonth = QDate(QDate::currentDate().year(), QDate::currentDate().month(), 1);
  QDate endOfMonth = QDate(QDate::currentDate().year(), QDate::currentDate().month(), QDate::currentDate().daysInMonth());

  //get transaction for current month
  filter.setDateFilter(startOfMonth, endOfMonth);
  filter.setReportAllSplits(false);

  QValueList<MyMoneyTransaction> transactions = file->transactionList(filter);
  QValueList<MyMoneyTransaction>::const_iterator it_t = transactions.begin();
  //if no transaction then skip and print total in zero
  if(transactions.size() > 0) {
    //get all transactions for this month
    for(; it_t != transactions.end(); ++it_t ) {
      const QValueList<MyMoneySplit>& splits = (*it_t).splits();
      QValueList<MyMoneySplit>::const_iterator it_s = splits.begin();
      for(; it_s != splits.end(); ++it_s ) {
        if(!(*it_s).shares().isZero()) {
          MyMoneyAccount acc = file->account((*it_s).accountId());
          if(acc.isIncomeExpense()) {
            //the balance is stored as negative number
            if(acc.accountType() == MyMoneyAccount::Income) {
              incomeValue += ((*it_s).shares() * MyMoneyMoney(-1, 1));
            } else {
              expenseValue += (*it_s).shares() * MyMoneyMoney(-1, 1);
            }
          }
        }
      }
    }
  }
  //format income and expenses
  QString amountIncome = incomeValue.formatMoney(file->baseCurrency().tradingSymbol(), prec);
  QString amountExpense = expenseValue.formatMoney(file->baseCurrency().tradingSymbol(), prec);
  amountIncome.replace(" ","&nbsp;");
  amountExpense.replace(" ","&nbsp;");

  m_part->write(QString("<tr class=\"row-%1\" style=\"font-weight:bold;\">").arg(i++ & 0x01 ? "even" : "odd"));

    //print total for income
  m_part->write(QString("<td class=\"left\">%1</td><td align=\"right\">%2</td>").arg(i18n("Total Incomes This Month")).arg(showColoredAmount(amountIncome, incomeValue.isNegative())));

    //leave the intermediate column empty
  m_part->write("<td></td>");

    //print total income
  m_part->write(QString("<td class=\"left\">%1</td><td align=\"right\">%2</td>").arg(i18n("Total Expenses This Month")).arg(showColoredAmount(amountExpense, expenseValue.isNegative())));
  m_part->write("</tr>");

  //Add all schedules for this month
  MyMoneyMoney scheduledIncome;
  MyMoneyMoney scheduledExpense;
  QValueList<MyMoneySchedule> schedule;

  //get overdues and schedules until the end of this month
  schedule = file->scheduleList("", MyMoneySchedule::TYPE_ANY,
                                MyMoneySchedule::OCCUR_ANY,
                                MyMoneySchedule::STYPE_ANY,
                                QDate(),
                                endOfMonth);

  //Remove the finished schedules
  QValueList<MyMoneySchedule>::Iterator d_it;
  for (d_it=schedule.begin(); d_it!=schedule.end();) {
    if ((*d_it).isFinished()) {
      d_it = schedule.remove(d_it);
      continue;
    }
    ++d_it;
  }

  //add incomes and expenses
  QValueList<MyMoneySchedule>::Iterator t_it;
  for (t_it=schedule.begin(); t_it!=schedule.end();) {
    QDate nextDate = (*t_it).nextDueDate();
    int cnt = 0;

    while(nextDate.isValid() && nextDate <= endOfMonth) {
      ++cnt;
      nextDate = (*t_it).nextPayment(nextDate);
        // for single occurence nextDate will not change, so we
        // better get out of here.
      if((*t_it).occurence() == MyMoneySchedule::OCCUR_ONCE)
        break;
    }

    MyMoneyAccount acc = (*t_it).account();
    if(acc.id()) {
      MyMoneyTransaction t = (*t_it).transaction();
      // only show the entry, if it is still active

      MyMoneySplit sp = t.splitByAccount(acc.id(), true);

      MyMoneyMoney value = (sp.value()*cnt);
      if(acc.currencyId() != file->baseCurrency().id()) {
        ReportAccount repAcc = ReportAccount(acc.id());
        MyMoneyMoney curPrice = repAcc.baseCurrencyPrice(QDate::currentDate());
        value = value * curPrice;
      }
      if(sp.value().isNegative()) {
        scheduledExpense += value;
      } else {
        scheduledIncome += value;
      }
    }
    ++t_it;
  }

  //format the currency strings
  QString amountScheduledIncome = scheduledIncome.formatMoney(file->baseCurrency().tradingSymbol(), prec);
  QString amountScheduledExpense = scheduledExpense.formatMoney(file->baseCurrency().tradingSymbol(), prec);

  amountScheduledIncome.replace(" ","&nbsp;");
  amountScheduledExpense.replace(" ","&nbsp;");

  m_part->write(QString("<tr class=\"row-%1\" style=\"font-weight:bold;\">").arg(i++ & 0x01 ? "even" : "odd"));

  //print the scheduled income
  m_part->write(QString("<td class=\"left\">%1</td><td align=\"right\">%2</td>").arg(i18n("Scheduled Incomes This Month")).arg(amountScheduledIncome));

  //leave the intermediate column empty
  m_part->write("<td></td>");

  //print the scheduled expenses
  m_part->write(QString("<td class=\"left\">%1</td><td align=\"right\">%2</td>").arg(i18n("Scheduled Expenses This Month")).arg(showColoredAmount(amountScheduledExpense,  true)));
  m_part->write("</tr>");
  m_part->write("</table>");
}

void KHomeView::showBudget(void)
{
  MyMoneyFile* file = MyMoneyFile::instance();

  if ( file->countBudgets() ) {
    int prec = MyMoneyMoney::denomToPrec(file->baseCurrency().smallestAccountFraction());
    int i = 0;

    //config report just like "Monthly Budgeted vs Actual
    MyMoneyReport reportCfg = MyMoneyReport(
      MyMoneyReport::eBudgetActual,
      MyMoneyReport::eMonths,
      MyMoneyTransactionFilter::currentMonth,
      true,
      i18n("Monthly Budgeted vs. Actual"),
      i18n("Generated Report"));

    reportCfg.setBudget("Any",true);

    reports::PivotTable table(reportCfg);

    PivotGrid grid = table.grid();

    //table header
    m_part->write("<div class=\"itemheader\">" + i18n("Budget Overruns") + "</div>\n<div class=\"gap\">&nbsp;</div>\n");
    m_part->write("<table width=\"75%\" cellspacing=\"0\" cellpadding=\"2\">");
      //asset and liability titles
    m_part->write("<tr class=\"item\">");
    m_part->write("<th class=\"left\" width=\"30%\">");
    m_part->write(i18n("Account"));
    m_part->write("</th>");
    m_part->write("<th class=\"right\" width=\"20%\">");
    m_part->write(i18n("Budgeted"));
    m_part->write("</th>");
    m_part->write("<th class=\"right\" width=\"20%\">");
    m_part->write(i18n("Actual"));
    m_part->write("</th>");
    m_part->write("<th class=\"right\" width=\"20%\">");
    m_part->write(i18n("Difference"));
    m_part->write("</th></tr>");


    PivotGrid::iterator it_outergroup = grid.begin();
    while ( it_outergroup != grid.end() )
    {
      i = 0;
      PivotOuterGroup::iterator it_innergroup = (*it_outergroup).begin();
      while ( it_innergroup != (*it_outergroup).end() )
      {
        PivotInnerGroup::iterator it_row = (*it_innergroup).begin();
        while ( it_row != (*it_innergroup).end() )
        {
          //column number is 1 because the report includes only current month
          if(it_row.data().m_budgetDiff[1].isNegative()) {
            //get report account to get the name later
            ReportAccount rowname = it_row.key();

            //write the outergroup if it is the first row of outergroup being shown
            if(i == 0) {
              m_part->write("<tr style=\"font-weight:bold;\">");
              m_part->write(QString("<td class=\"left\" colspan=\"4\">%1</td>").arg(KMyMoneyUtils::accountTypeToString( rowname.accountType())));
              m_part->write("</tr>");
            }
            m_part->write(QString("<tr class=\"row-%1\">").arg(i++ & 0x01 ? "even" : "odd"));

            //get values from grid
            MyMoneyMoney actualValue = it_row.data()[1];
            MyMoneyMoney budgetValue = it_row.data().m_budget[1];
            MyMoneyMoney budgetDiffValue = it_row.data().m_budgetDiff[1];

            //format amounts
            QString actualAmount = actualValue.formatMoney(file->baseCurrency().tradingSymbol(), prec);
            QString budgetAmount = budgetValue.formatMoney(file->baseCurrency().tradingSymbol(), prec);
            QString budgetDiffAmount = budgetDiffValue.formatMoney(file->baseCurrency().tradingSymbol(), prec);

            //account name
            m_part->write(QString("<td class=\"left\">%1</td>").arg(rowname.name().replace(QRegExp(" "), "&nbsp;")));
  
            //show amounts
            m_part->write(QString("<td align=\"right\">%1</td>").arg(showColoredAmount(budgetAmount, budgetValue.isNegative())));
            m_part->write(QString("<td align=\"right\">%1</td>").arg(showColoredAmount(actualAmount, actualValue.isNegative())));
            m_part->write(QString("<td align=\"right\">%1</td>").arg(showColoredAmount(budgetDiffAmount, budgetDiffValue.isNegative())));
            m_part->write("</tr>");
          }
          ++it_row;
        }
        ++it_innergroup;
      }
      ++it_outergroup;
    }

    //if no negative differences are found, then inform that
    if(i == 0) {
      m_part->write(QString("<tr class=\"row-%1\" style=\"font-weight:bold;\">").arg(i++ & 0x01 ? "even" : "odd"));
      m_part->write(QString("<td class=\"center\" colspan=\"4\">%1</td>").arg(i18n("No Budget Categories have been overrun")));
      m_part->write("</tr>");
    }
    m_part->write("</table>");
  }
}

QString KHomeView::showColoredAmount(const QString& amount, bool isNegative)
{
  if(isNegative) {
    //if negative, get the settings for negative numbers
    return QString("<font color=\"%1\">%2</font>").arg(KMyMoneyGlobalSettings::listNegativeValueColor().name(), amount);
  }

  //if positive, return the same string
  return amount;
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
