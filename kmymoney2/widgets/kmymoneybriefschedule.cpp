/***************************************************************************
                          kmymoneybriefschedule.cpp  -  description
                             -------------------
    begin                : Sun Jul 6 2003
    copyright            : (C) 2000-2003 by Michael Edwardes
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
#include <qlabel.h>
#include <qlineedit.h>
#include <qtextedit.h>
#include <qtoolbutton.h>

// ----------------------------------------------------------------------------
// KDE Includes
#include <klocale.h>
#include <kiconloader.h>
#include <kpushbutton.h>

// ----------------------------------------------------------------------------
// Project Includes
#include "kmymoneybriefschedule.h"
#include "../mymoney/mymoneyscheduled.h"
#include "../kmymoneyutils.h"

KMyMoneyBriefSchedule::KMyMoneyBriefSchedule(QWidget *parent, const char *name )
  : kScheduleBriefWidget(parent,name, WStyle_Customize | WStyle_NoBorder)
{
  m_nextButton->setPixmap(BarIcon(QString::fromLatin1("1rightarrow")));
  m_prevButton->setPixmap(BarIcon(QString::fromLatin1("1leftarrow")));

  connect(m_prevButton, SIGNAL(clicked()), this, SLOT(slotPrevClicked()));
  connect(m_nextButton, SIGNAL(clicked()), this, SLOT(slotNextClicked()));
  connect(m_closeButton, SIGNAL(clicked()), this, SLOT(hide()));
  connect(m_buttonEnter, SIGNAL(clicked()), this, SLOT(slotEnterClicked()));

  KIconLoader *ic = KGlobal::iconLoader();
  KGuiItem closeGuiItem(  i18n("&Close"),
                          QIconSet(ic->loadIcon("remove", KIcon::Small, KIcon::SizeSmall)),
                          i18n("Close this window"),
                          i18n("Use this button to close the window"));
  m_closeButton->setGuiItem(closeGuiItem);

  KGuiItem enterGuiItem(  i18n("&Enter"),
                          QIconSet(ic->loadIcon("enter", KIcon::Small, KIcon::SizeSmall)),
                          i18n("Record this transaction into the register"),
                          i18n("Use this button to record this transaction"));
  m_buttonEnter->setGuiItem(enterGuiItem);
}

KMyMoneyBriefSchedule::~KMyMoneyBriefSchedule()
{
}

void KMyMoneyBriefSchedule::setSchedules(QValueList<MyMoneySchedule> list, const QDate& date)
{
  m_scheduleList = list;
  m_date = date;

  m_index = 0;
  if (list.count() >= 1)
  {
    loadSchedule();
  }
}

void KMyMoneyBriefSchedule::loadSchedule()
{
  try
  {
    if (m_index < m_scheduleList.count())
    {
      MyMoneySchedule sched = m_scheduleList[m_index];

      // TODO
      // For now hide the enter button.  At some point in the
      // future we will be able to add any occurence and will
      // have to remove this code.
      QDate nextPayment = sched.nextPayment(sched.lastPayment());
      if (m_date < QDate::currentDate())
        if (m_date != nextPayment)
          m_buttonEnter->hide();
        else
          m_buttonEnter->show();
      else
        m_buttonEnter->hide();
        
      m_indexLabel->setText(QString::number(m_index+1) + i18n(" of ") + QString::number(m_scheduleList.count()));
      m_name->setText(sched.name());
      m_type->setText(KMyMoneyUtils::scheduleTypeToString(sched.type()));
      m_account->setText(sched.account().name());
      QString text(i18n("Payment on "));
      text += m_date.toString();
      text += i18n(" for ");
      MyMoneyMoney amount = sched.transaction().splitByAccount(sched.account().id()).value();
      if (amount < 0)
        amount = -amount;
      text += amount.formatMoney();
      if (sched.willEnd())
      {
        text += i18n(" with ");
        int transactions = sched.paymentDates(m_date, sched.endDate()).count()-1;
        text += QString::number(transactions);
        text += i18n(" transactions remaining ");
      }
      text += i18n(" occuring ");
      text += KMyMoneyUtils::occurenceToString(sched.occurence());
      text += ".";

      if (m_date < QDate::currentDate())
      {
        if (sched.isOverdue())
        {
          QDate startD = (sched.lastPayment().isValid()) ?
            sched.lastPayment() :
            sched.startDate();
            
          if (m_date.isValid())
            startD = m_date;
          
          int days = startD.daysTo(QDate::currentDate());
          int transactions = sched.paymentDates(startD, QDate::currentDate()).count();

          text += "<br><font color=red>";
          text += QString::number(days);
          text += i18n(" days overdue (");
          text += QString::number(transactions);
          text += i18n(" occurences).</color>");
        }
      }
      
      m_details->setText(text);

      m_prevButton->setEnabled(true);
      m_nextButton->setEnabled(true);

      if (m_index == 0)
        m_prevButton->setEnabled(false);
      if (m_index == (m_scheduleList.count()-1))
        m_nextButton->setEnabled(false);
    }
  }
  catch (MyMoneyException *e)
  {
    delete e;
  }
}

void KMyMoneyBriefSchedule::slotPrevClicked()
{
  if (m_index >= 1)
  {
    --m_index;
    loadSchedule();
  }
}

void KMyMoneyBriefSchedule::slotNextClicked()
{
  if (m_index < (m_scheduleList.count()-1))
  {
    m_index++;
    loadSchedule();
  }
}

void KMyMoneyBriefSchedule::slotEnterClicked()
{
  hide();
  emit enterClicked(m_scheduleList[m_index], m_date);
}
