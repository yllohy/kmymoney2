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

// ----------------------------------------------------------------------------
// Project Includes
#include "khomeview.h"
#include "../mymoney/mymoneyfile.h"

#define VIEW_LEDGER     "ledger"
#define VIEW_SCHEDULE   "schedule"

KHomeView::KHomeView(QWidget *parent, const char *name )
 : QWidget(parent,name)
{
  m_qvboxlayoutPage = new QVBoxLayout(this);
  m_qvboxlayoutPage->setSpacing( 6 );
  m_qvboxlayoutPage->setMargin( 11 );

  m_part = new KHTMLPart(this, "htmlpart_km2");
  m_qvboxlayoutPage->addWidget(m_part->view());
  QString filename = KGlobal::dirs()->findResource("appdata", "html/home.html");
  m_part->openURL(filename);
  connect(m_part->browserExtension(), SIGNAL(openURLRequest(const KURL&, const KParts::URLArgs&)),
          this, SLOT(slotOpenURL(const KURL&, const KParts::URLArgs&)));
}

KHomeView::~KHomeView()
{
}

void KHomeView::show()
{
  if(MyMoneyFile::instance()->accountList().count() == 0) {
    QString filename = KGlobal::dirs()->findResource("appdata", "html/home.html");
    m_part->openURL(filename);
  } else {
    QString filename = KGlobal::dirs()->findResource("appdata", "html/kmymoney2.css");
    QString header = QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\">\n") +
      QString("<html><head><link rel=\"stylesheet\" type=\"text/css\" href=\"%1\"></head>\n").arg(filename);
    QString footer = "</body></html>\n";

    m_part->begin();
    m_part->write(header);

    showPayments();
    
    m_part->write("<div class=\"gap\">&nbsp;</div>\n");

    showAccounts();

    m_part->write(footer);
    m_part->end();
  
  }
  emit signalViewActivated();
  QWidget::show();
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
                                     
  QString tmp;
  tmp = "<div class=\"itemheader\">" + i18n("Payments") +
        "</div>\n<div class=\"gap\">&nbsp;</div>\n";

  m_part->write(tmp);
  if(overdues.count() > 0) {
    qBubbleSort(overdues);
    QValueList<MyMoneySchedule>::Iterator it;
    QValueList<MyMoneySchedule>::Iterator it_f;
    tmp = "<div class=\"warning\">" + i18n("Overdue payments") + "</div>\n";
    m_part->write(tmp);
    
    m_part->write("<table cellspacing=\"0\" cellpadding=\"1\">");
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
    QValueList<MyMoneySchedule>::Iterator it;
    tmp = "<div class=\"item\">" + i18n("Future payments") + "</div>\n";
    m_part->write(tmp);
    
    m_part->write("<table cellspacing=\"0\" cellpadding=\"1\">");
    for(it = schedule.begin(); it != schedule.end(); ++it) {
      m_part->write(QString("<tr class=\"row-%1\">").arg(i++ & 0x01 ? "even" : "odd"));
      showPaymentEntry(*it);
      m_part->write("</tr>");
    }
    m_part->write("</table>");
  }
}

void KHomeView::showPaymentEntry(const MyMoneySchedule& sched)
{
  QString tmp;
  MyMoneyAccount acc = sched.account();
  if(acc.id()) {
    MyMoneyTransaction t = sched.transaction();
    MyMoneySplit sp = t.split(acc.id(), true);

    tmp = QString("<td width=\"10%\">") +
      KGlobal::locale()->formatDate(sched.nextPayment(sched.lastPayment()), true) +
      "</td><td width=\"80%\">" +
      link(VIEW_SCHEDULE, QString("?id=%1").arg(sched.id())) + sched.name() + linkend() +
      "</td>" +
      "<td width=\"10%\" align=\"right\">";
    tmp += sp.value().formatMoney();
    tmp += "</td>";
    // qDebug("paymentEntry = '%s'", tmp.latin1());
    m_part->write(tmp);
  }  
}

void KHomeView::showAccounts(void)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  QValueList<MyMoneyAccount> accounts;
  QValueList<MyMoneyAccount>::Iterator it;

  // get list of the preferred accounts
  accounts = file->accountList();
  for(it = accounts.begin(); it != accounts.end();) {
    if((*it).value("PreferredAccount") != "Yes") {
      it = accounts.remove(it);
      continue;
    }
    ++it;
  }

  if(accounts.count() > 0) {
    QString tmp;
    int i = 0;
    tmp = "<div class=\"itemheader\">" + i18n("Preferred Accounts") +
          "</div>\n<div class=\"gap\">&nbsp;</div>\n";

    m_part->write(tmp);
    m_part->write("<table cellspacing=\"0\" cellpadding=\"2\" width=\"60%\">");
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
  
  tmp = QString("<td width=\"70%\">") +
      link(VIEW_LEDGER, QString("?id=%1").arg(acc.id())) + acc.name() + linkend() + "</td>";
  tmp += QString("<td width=\"30%\" align=\"right\">") +
      MyMoneyFile::instance()->balance(acc.id()).formatMoney() +
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
    emit ledgerSelected(id, "");
    
  } else if(view == VIEW_SCHEDULE) {
    
  } else {
    qDebug("Unknown view '%s' in KHomeView::slotOpenURL()", view.latin1());
  }
}

