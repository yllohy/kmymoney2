/***************************************************************************
                          kmymoneyscheduleddatetbl.cpp  -  description
                             -------------------
    begin                : Thu Jul 3 2003
    copyright            : (C) 2000-2003 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
 ***************************************************************************/
 /****************************************************************************
 Contains code from the KDateTable class ala kdelibs-3.1.2.  Original license:

    This file is part of the KDE libraries
    Copyright (C) 1997 Tim D. Gilman (tdgilman@best.org)
              (C) 1998-2001 Mirko Boehm (mirko@kde.org)
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
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
#include <qstring.h>
#include <qpen.h>
#include <qpainter.h>
#include <qdialog.h>
#include <qdrawutil.h>

// ----------------------------------------------------------------------------
// KDE Includes
#include <kglobal.h>
#include <kglobalsettings.h>
//#include <kapplication.h>
#include <klocale.h>
//#include <kdebug.h>
//#include <knotifyclient.h>

// ----------------------------------------------------------------------------
// Project Includes
#include "kmymoneyscheduleddatetbl.h"
#include "../mymoney/mymoneyscheduled.h"

kMyMoneyScheduledDateTbl::kMyMoneyScheduledDateTbl(QWidget *parent, QDate date_, const char* name, WFlags f )
  : kMyMoneyDateTbl(parent, date_, name, f)
{
}

kMyMoneyScheduledDateTbl::~kMyMoneyScheduledDateTbl()
{
}

void kMyMoneyScheduledDateTbl::drawCellContents(QPainter *painter, int row, int col, const QDate& theDate)
{
  QRect rect;
  QString text;
  int w=cellWidth();
  int h=cellHeight();
  QPen pen;
  QBrush brushBlue(KGlobalSettings::activeTitleColor());
  QBrush brushLightblue(KGlobalSettings::baseColor());
  QFont font=KGlobalSettings::generalFont();

  // -----
  font.setPointSize(fontsize);
  QFont fontLarge(font);
  fontLarge.setPointSize(fontsize*2);

  painter->setFont(font);

  if (m_type == MONTHLY)
  {
    pen=lightGray;

    if (theDate == date)
    {
      if (hasFocus())
      { // draw the currently selected date
        painter->setPen(KGlobalSettings::highlightColor());
        painter->setBrush(KGlobalSettings::highlightColor());
        pen=white;
      } else {
        painter->setPen(KGlobalSettings::calculateAlternateBackgroundColor(KGlobalSettings::highlightColor()));
        painter->setBrush(KGlobalSettings::calculateAlternateBackgroundColor(KGlobalSettings::highlightColor()));
        pen=white;
      }
    } else {
      painter->setBrush(KGlobalSettings::baseColor());
      painter->setPen(KGlobalSettings::baseColor());
    }
    painter->drawRect(0, 0, w, h);
    painter->setPen(pen);
    text = QString::number(theDate.day());
    addDayPostfix(text);
    painter->drawText(0, 0, w-2, h, AlignRight, text, -1, &rect);

    MyMoneyScheduled *scheduled;
    QStringList schedules;
    try
    {
      scheduled = MyMoneyScheduled::instance();

      schedules = scheduled->getScheduled(
          m_accountId,
          theDate,
          theDate);
    }
    catch ( MyMoneyException*)
    {
      // SAfe to ignore here, cause no schedules might exist
      // for the selected account
    }

    if (schedules.count() >= 1)
    {
      painter->setPen(darkGray);
      painter->setFont(fontLarge);
      painter->drawText(0, 0, w, h, AlignCenter, QString::number(schedules.count()),
          -1, &rect);
    }

    painter->setPen(lightGray);
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(0, 0, w, h);
  }
  else if (m_type == WEEKLY)
  {
    // TODO: Handle other start weekdays than Monday
    if (theDate == date)
    {
      painter->setBrush(KGlobalSettings::highlightColor());
    }
    else
    {
      painter->setBrush(KGlobalSettings::baseColor());
      painter->setPen(KGlobalSettings::baseColor());
    }
    
    painter->setPen(lightGray);
    painter->drawRect(0, 0, w, h);

    text = QString::number(theDate.day());
    addDayPostfix(text);

    painter->drawText(0, 0, w-2, h, AlignRight, QDate::shortDayName(theDate.month()) + " " + text, -1, &rect);

    MyMoneyScheduled *scheduled;
    QStringList billSchedules;
    QStringList depositSchedules;
    QStringList transferSchedules;
    try
    {
      scheduled = MyMoneyScheduled::instance();

/*
  QStringList getScheduled(const QCString& accountId, const QDate& startDate, const QDate& endDate,
    const MyMoneySchedule::typeE type=MyMoneySchedule::TYPE_ANY,
    const MyMoneySchedule::paymentTypeE paymentType=MyMoneySchedule::STYPE_ANY,
    const MyMoneySchedule::occurenceE occurence=MyMoneySchedule::OCCUR_ANY);
*/
      text = "";
      billSchedules = scheduled->getScheduled(
          m_accountId,
          theDate,
          theDate,
          MyMoneySchedule::TYPE_BILL);
      if (billSchedules.count() >= 1)
      {
        text += QString::number(billSchedules.count());
        text += " Bills.  ";
      }
      
      depositSchedules = scheduled->getScheduled(
          m_accountId,
          theDate,
          theDate,
          MyMoneySchedule::TYPE_DEPOSIT);
      if (depositSchedules.count() >= 1)
      {
        text += QString::number(depositSchedules.count());
        text += " Deposits.  ";
      }

      transferSchedules = scheduled->getScheduled(
          m_accountId,
          theDate,
          theDate,
          MyMoneySchedule::TYPE_TRANSFER);
      if (transferSchedules.count() >= 1)
      {
        text += QString::number(transferSchedules.count());
        text += " Transfers.";
      }

    }
    catch (MyMoneyException*)
    {
      // SAfe to ignore here, cause no schedules might exist
      // for the selected account
    }

    painter->setPen(darkGray);
    painter->setFont(fontLarge);
    painter->drawText(0, 0, w, h, AlignCenter, text,
          -1, &rect);
  }
  else if (m_type == QUARTERLY)
  {
    painter->setBrush(KGlobalSettings::baseColor());

    painter->setPen(lightGray);
    painter->drawRect(0, 0, w, h);
  }
}

void kMyMoneyScheduledDateTbl::addDayPostfix(QString& text)
{
  int d = text.toInt();
  switch (d)
  {
    case 1:
      text += i18n("st");
      break;
    case 2:
      text += i18n("nd");
      break;
    case 3:
      text += i18n("rd");
      break;
    default:
      text += i18n("th");
  }
}

void kMyMoneyScheduledDateTbl::refresh(const QCString& accountId)
{
  m_accountId = accountId;
  repaintContents(false);
}
