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

// ----------------------------------------------------------------------------
// Project Includes
#include "kmymoneybriefschedule.h"
#include "../mymoney/mymoneyscheduled.h"

KMyMoneyBriefSchedule::KMyMoneyBriefSchedule(QWidget *parent, const char *name )
  : kScheduleBriefWidget(parent,name)
{
  m_nextButton->setPixmap(BarIcon(QString::fromLatin1("1rightarrow")));
  m_prevButton->setPixmap(BarIcon(QString::fromLatin1("1leftarrow")));

  connect(m_prevButton, SIGNAL(clicked()), this, SLOT(slotPrevClicked()));
  connect(m_nextButton, SIGNAL(clicked()), this, SLOT(slotNextClicked()));
}

KMyMoneyBriefSchedule::~KMyMoneyBriefSchedule()
{
}

void KMyMoneyBriefSchedule::setSchedules(QValueList<MyMoneySchedule> list)
{
  m_scheduleList = list;
  
  if (list.count() >= 1)
  {
    loadSchedule(0);
  }
}

void KMyMoneyBriefSchedule::loadSchedule(unsigned int index)
{
  try
  {
    if (index < m_scheduleList.count())
    {
      m_index = index;
      MyMoneySchedule sched = m_scheduleList[index];
      m_indexLabel->setText(QString::number(m_index+1) + i18n(" of ") + QString::number(m_scheduleList.count()));
      m_name->setText(sched.name());
      m_type->setText(sched.typeToString());
      m_account->setText(MyMoneyFile::instance()->account(sched.accountId()).name());
      QString text(i18n("Next payment on "));
      text += sched.nextPayment().toString();
      text += i18n(" for ");
      MyMoneyMoney amount = sched.transaction().split(sched.accountId()).value();
      if (amount < 0)
        amount = -amount;
      text += amount.formatMoney();
      if (sched.willEnd())
      {
        text += i18n(" with ");
        text += QString::number(sched.transactionsRemaining());
        text += i18n(" transactions remaining ");
      }
      text += i18n(" occuring ");
      text += sched.occurenceToString();
      text += ".";
      m_details->setText(text);

      m_prevButton->setEnabled(true);
      m_nextButton->setEnabled(true);

      if (index == 0)
        m_prevButton->setEnabled(false);
      if (index == (m_scheduleList.count()-1))
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
    loadSchedule(--m_index);
}

void KMyMoneyBriefSchedule::slotNextClicked()
{
  if (m_index < (m_scheduleList.count()-1))
    loadSchedule(++m_index);
}
