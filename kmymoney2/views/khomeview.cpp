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

// ----------------------------------------------------------------------------
// Project Includes
#include "khomeview.h"
#include "../kmymoneyutils.h"
#include "../mymoney/mymoneyfile.h"

#define VIEW_LEDGER         "ledger"
#define VIEW_SCHEDULE       "schedule"
#define VIEW_WELCOME        "welcome"
#define VIEW_HOME           "home"
#define VIEW_REPORTS        "reports"

KHomeView::KHomeView(QWidget *parent, const char *name ) :
  KMyMoneyViewBase(parent, name, i18n("Home")),
  m_showAllSchedules(false)
{
  m_part = new KHTMLPart(this, "htmlpart_km2");
  m_viewLayout->addWidget(m_part->view());

  m_filename = KMyMoneyUtils::findResource("appdata", QString("html/home%1.html"));

//   m_part->openURL(m_filename);
  connect(m_part->browserExtension(), SIGNAL(openURLRequest(const KURL&, const KParts::URLArgs&)),
          this, SLOT(slotOpenURL(const KURL&, const KParts::URLArgs&)));
}

KHomeView::~KHomeView()
{
}

void KHomeView::show()
{
  slotRefreshView();
  QWidget::show();
  emit signalViewActivated();
}

void KHomeView::slotRefreshView(void)
{
  if(MyMoneyFile::instance()->accountList().count() == 0) {
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

    QString temp(i18n("Your Financial Summary"));
    QString financialPic = QString("<h2>%1</h2>").arg(temp);
    m_part->write(financialPic);

    KConfig *kconfig = KGlobal::config();
    kconfig->setGroup("Homepage Options");
    QStringList settings = kconfig->readListEntry("Itemlist");
    KMyMoneyUtils::addDefaultHomePageItems(settings);

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
        }
        m_part->write("<div class=\"gap\">&nbsp;</div>\n");
      }
    }

    m_part->write(link(VIEW_WELCOME, QString()) + i18n("Show KMyMoney welcome page") + linkend());

    m_part->write(footer);
    m_part->end();

  }
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

  QString tmp;
  tmp = "<div class=\"itemheader\">" + i18n("Payments") + "</div>\n";
  m_part->write(tmp);

  if(overdues.count() > 0) {
    tmp = "<div class=\"gap\">&nbsp;</div>\n";
    m_part->write(tmp);

    qBubbleSort(overdues);
    QValueList<MyMoneySchedule>::Iterator it;
    QValueList<MyMoneySchedule>::Iterator it_f;

    m_part->write("<table width=\"75%\" cellspacing=\"0\" cellpadding=\"2\">");
    tmp = "<tr><th class=\"warning\" colspan=\"3\">" + i18n("Overdue payments") + "</th></tr>\n";
    m_part->write(tmp);
    for(it = overdues.begin(); it != overdues.end(); ++it) {
      m_part->write(QString("<tr class=\"row-%1\">").arg(i++ & 0x01 ? "even" : "odd"));
      showPaymentEntry(*it);
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
      tmp = "<tr class=\"item\"><th class=\"left\" colspan=\"3\">" + i18n("Todays payments") + "</th></tr>\n";
      m_part->write(tmp);

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
      tmp = "<tr class=\"item\"><th class=\"left\" colspan=\"3\">" + i18n("Future payments") + "</th></tr>\n";
      m_part->write(tmp);

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

void KHomeView::showPaymentEntry(const MyMoneySchedule& sched)
{
  QString tmp;
  MyMoneyAccount acc = sched.account();
  if(acc.id()) {
    MyMoneyTransaction t = sched.transaction();
    // only show the entry, if it is still active
    if(!sched.isFinished() && sched.nextPayment(sched.lastPayment()) != QDate()) {
      MyMoneySplit sp = t.splitByAccount(acc.id(), true);

      tmp = QString("<td width=\"20%\">") +
        KGlobal::locale()->formatDate(sched.nextPayment(sched.lastPayment()), true) +
        "</td><td width=\"70%\">" +
        link(VIEW_SCHEDULE, QString("?id=%1").arg(sched.id())) + sched.name() + linkend() +
        "</td>" +
        "<td width=\"10%\" align=\"right\">";

      MyMoneySecurity currency = MyMoneyFile::instance()->currency(acc.currencyId());
      QString amount = sp.value().formatMoney(currency.tradingSymbol());
      amount.replace(" ","&nbsp;");
      tmp += amount;
      tmp += "</td>";
      // qDebug("paymentEntry = '%s'", tmp.latin1());
      m_part->write(tmp);
    }
  }
}

void KHomeView::showAccounts(KHomeView::paymentTypeE type, const QString& header)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  QValueList<MyMoneyAccount> accounts;
  QValueList<MyMoneyAccount>::Iterator it;
  QValueList<MyMoneyAccount>::Iterator prevIt;

  // get list of all accounts
  accounts = file->accountList();
  for(it = accounts.begin(); it != accounts.end();) {
    prevIt = it;
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
    // if we still point to the same account, we better move on ;-)
    if(prevIt == it)
      ++it;
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

    for(it = accounts.begin(); it != accounts.end(); ++it) {
      m_part->write(QString("<tr class=\"row-%1\">").arg(i++ & 0x01 ? "even" : "odd"));
      showAccountEntry(*it);
      m_part->write("</tr>");
    }
    m_part->write("</table>");
  }
}

void KHomeView::showAccountEntry(const MyMoneyAccount& acc)
{
  QString tmp;
  MyMoneySecurity currency = MyMoneyFile::instance()->currency(acc.currencyId());

  QString amount = MyMoneyFile::instance()->balance(acc.id()).formatMoney(currency.tradingSymbol());
  amount.replace(" ","&nbsp;");

  tmp = QString("<td width=\"70%\">") +
      link(VIEW_LEDGER, QString("?id=%1").arg(acc.id())) + acc.name() + linkend() + "</td>";
  tmp += QString("<td width=\"30%\" align=\"right\">%1</td>").arg(amount);
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
        slotRefreshView();
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
      else if ( mode == "manual" )
      {
        KMessageBox::sorry(qApp->mainWidget(),i18n("There is no user manual yet"),i18n("No manual"));
      }
      else
        m_part->openURL(m_filename);

    } else if(view == "action") {
        KMainWindow* mw = dynamic_cast<KMainWindow*>(qApp->mainWidget());
        Q_CHECK_PTR(mw);
        mw->actionCollection()->action( id )->activate();

#if 0
        // Enable this to get a dump of all action names
        unsigned idx = mw->actionCollection()->count();
        while( idx-- )
        {
          qDebug("%u: %s\n",idx,mw->actionCollection()->action(idx)->name());
        }
#endif

    } else if(view == VIEW_HOME) {
      slotRefreshView();

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
