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

// ----------------------------------------------------------------------------
// KDE Includes
#include <kglobal.h>
#include <klocale.h>
#if QT_VERSION > 300
#include <kstandarddirs.h>
#else
#include <kstddirs.h>
#endif

#include <khtmlview.h>
#include <kconfig.h>

// ----------------------------------------------------------------------------
// Project Includes
#include "khomeview.h"
#include "../mymoney/mymoneyfile.h"
#include "../kmymoneyutils.h"

#define VIEW_LEDGER         "ledger"
#define VIEW_SCHEDULE       "schedule"
#define VIEW_WELCOME        "welcome"
#define VIEW_HOME           "home"

KHomeView::KHomeView(QWidget *parent, const char *name )
 : QWidget(parent,name)
{
  m_qvboxlayoutPage = new QVBoxLayout(this);
  m_qvboxlayoutPage->setSpacing( 6 );
  m_qvboxlayoutPage->setMargin( 11 );

  m_part = new KHTMLPart(this, "htmlpart_km2");
  m_qvboxlayoutPage->addWidget(m_part->view());
  QString language = KGlobal::locale()->language();
  QString country = KGlobal::locale()->country();

  m_filename = KGlobal::dirs()->findResource("appdata", QString("html/home_%1.%2.html").arg(country).arg(language));
  if(m_filename.isEmpty()) {
    // qDebug(QString("html/home_%1.%2.html not found").arg(country).arg(language).latin1());
    m_filename = KGlobal::dirs()->findResource("appdata", QString("html/home_%1.html").arg(country));
  }
  if(m_filename.isEmpty()) {
    // qDebug(QString("html/home_%1.html not found").arg(country).latin1());
    m_filename = KGlobal::dirs()->findResource("appdata", "html/home.html");
  }

  m_part->openURL(m_filename);
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
  } else {
    QString filename = KGlobal::dirs()->findResource("appdata", "html/kmymoney2.css");
    QString header = QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\">\n") +
      QString("<html><head><link rel=\"stylesheet\" type=\"text/css\" href=\"%1\"></head>\n").arg(filename);
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
    tmp = "<div class=\"warning\">" + i18n("Overdue payments") + "</div>\n";
    m_part->write(tmp);

    m_part->write("<table width=\"75%\" cellspacing=\"0\" cellpadding=\"1\">");
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
        t_it = schedule.remove(t_it);
        continue;
      }
      ++t_it;
    }

    if (todays.count() > 0)
    {
      m_part->write("<div class=\"gap\">&nbsp;</div>\n");
      tmp = "<div class=\"item\">" + i18n("Todays payments") + "</div>\n";
      m_part->write(tmp);
      m_part->write("<table width=\"75%\" cellspacing=\"0\" cellpadding=\"1\">");

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
      tmp = "<div class=\"item\">" + i18n("Future payments") + "</div>\n";
      m_part->write(tmp);

      m_part->write("<table width=\"75%\" cellspacing=\"0\" cellpadding=\"1\">");
      for(it = schedule.begin(); it != schedule.end(); ++it) {
        m_part->write(QString("<tr class=\"row-%1\">").arg(i++ & 0x01 ? "even" : "odd"));
        showPaymentEntry(*it);
        m_part->write("</tr>");
      }
      m_part->write("</table>");
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
      tmp += sp.value().formatMoney();
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
    m_part->write("<table width=\"75%\" cellspacing=\"0\" cellpadding=\"2\" width=\"60%\">");
    m_part->write("<tr class=\"item\"><td width=\"70%\">");
    m_part->write(i18n("Account"));
    m_part->write("</td><td width=\"30%\" align=\"right\">");
    m_part->write(i18n("Balance"));
    m_part->write("</td></tr>");

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
  MyMoneyCurrency currency = MyMoneyFile::instance()->currency(acc.currencyId());

  tmp = QString("<td width=\"70%\">") +
      link(VIEW_LEDGER, QString("?id=%1").arg(acc.id())) + acc.name() + linkend() + "</td>";
  tmp += QString("<td width=\"30%\" align=\"right\">") +
      MyMoneyFile::instance()->balance(acc.id()).formatMoney(currency.tradingSymbol()) +
      "</td>";
  // qDebug("accountEntry = '%s'", tmp.latin1());
  m_part->write(tmp);
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
  QString view = url.fileName(false);
  QCString id = url.queryItem("id").data();

  // qDebug("view = '%s'", url.fileName(false).latin1());
  // qDebug("id = '%s'", url.queryItem("id").latin1());

  if(view == VIEW_LEDGER) {
    emit ledgerSelected(id, QCString());

  } else if(view == VIEW_SCHEDULE) {
    emit scheduleSelected(id);

  } else if(view == VIEW_WELCOME) {
    m_part->openURL(m_filename);

  } else if(view == VIEW_HOME) {
    slotRefreshView();

  } else {
    qDebug("Unknown view '%s' in KHomeView::slotOpenURL()", view.latin1());
  }
}

