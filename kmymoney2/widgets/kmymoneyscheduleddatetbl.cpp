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
#include <kdeversion.h>

// ----------------------------------------------------------------------------
// Project Includes
#include "kmymoneyscheduleddatetbl.h"
#include "../mymoney/mymoneyfile.h"

kMyMoneyScheduledDateTbl::kMyMoneyScheduledDateTbl(QWidget *parent, QDate date_, const char* name, WFlags f )
  : kMyMoneyDateTbl(parent, date_, name, f),
  m_filterBills(false), m_filterDeposits(false), m_filterTransfers(false)
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
  MyMoneyFile *file = MyMoneyFile::instance();

  // -----
  font.setPointSize(fontsize);
  QFont fontLarge(font);
  QFont fontSmall(font);
  fontLarge.setPointSize(fontsize*2);
  fontSmall.setPointSize(fontsize-1);
  
  painter->setFont(font);


  if (m_type == MONTHLY)
  {
    if (theDate.month() != date.month())
    {
      painter->setFont(fontSmall);
      pen = lightGray;
    }
    else
    {
      pen = gray;
    }

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

    MyMoneyFile *file = MyMoneyFile::instance();
    QValueList<MyMoneySchedule> schedules;
    try
    {

      // Honour the filter.
      if (!m_filterBills)
      {
        schedules += file->scheduleList("",
                                     MyMoneySchedule::TYPE_BILL,
                                     MyMoneySchedule::OCCUR_ANY,
                                     MyMoneySchedule::STYPE_ANY,
                                     theDate,
                                     theDate);
      }
      if (!m_filterDeposits)
      {
        schedules += file->scheduleList("",
                                     MyMoneySchedule::TYPE_DEPOSIT,
                                     MyMoneySchedule::OCCUR_ANY,
                                     MyMoneySchedule::STYPE_ANY,
                                     theDate,
                                     theDate);
      }
      if (!m_filterTransfers)
      {
        schedules += file->scheduleList("",
                                     MyMoneySchedule::TYPE_TRANSFER,
                                     MyMoneySchedule::OCCUR_ANY,
                                     MyMoneySchedule::STYPE_ANY,
                                     theDate,
                                     theDate);
      }

      if (m_filterAccounts.count() > 0)
      {
        // Filter out the accounts
        QValueList<MyMoneySchedule> toDelete;
        QValueList<MyMoneySchedule>::Iterator schedit;
        for (schedit=schedules.begin(); schedit!=schedules.end(); ++schedit)
        {
          QCStringList::Iterator it;
          for (it=m_filterAccounts.begin(); it != m_filterAccounts.end(); ++it)
          {
            if ((*it) == (*schedit).accountId())
            {
              toDelete.append(*schedit);
              break;
            }
          }
        }
        QValueList<MyMoneySchedule>::Iterator delit;
        for (delit=toDelete.begin(); delit!=toDelete.end(); ++delit)
        {
          schedules.remove(delit);
        }
      }
    }
    catch ( MyMoneyException* e)
    {
      // SAfe to ignore here, cause no schedules might exist
      // for the selected account
      delete e;
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

    painter->drawText(0, 0, w-2, h, AlignRight, QDate::shortDayName(theDate.dayOfWeek()) + " " + text, -1, &rect);

    QValueList<MyMoneySchedule> billSchedules;
    QValueList<MyMoneySchedule> depositSchedules;
    QValueList<MyMoneySchedule> transferSchedules;
    try
    {
      text = "";

      if (!m_filterBills)
      {
        billSchedules = file->scheduleList("",
                                     MyMoneySchedule::TYPE_BILL,
                                     MyMoneySchedule::OCCUR_ANY,
                                     MyMoneySchedule::STYPE_ANY,
                                     theDate,
                                     theDate);
        if (m_filterAccounts.count() > 0)
        {
          QValueList<MyMoneySchedule> toDelete;
          QValueList<MyMoneySchedule>::Iterator schedit;
          for (schedit=billSchedules.begin(); schedit!=billSchedules.end(); ++schedit)
          {
            QCStringList::Iterator it;
            for (it=m_filterAccounts.begin(); it != m_filterAccounts.end(); ++it)
            {
              if ((*it) == (*schedit).accountId())
              {
                toDelete.append(*schedit);
                break;
              }
            }
          }
          QValueList<MyMoneySchedule>::Iterator delit;
          for (delit=toDelete.begin(); delit!=toDelete.end(); ++delit)
          {
            billSchedules.remove(delit);
          }
        }
        if (billSchedules.count() >= 1)
        {
          text += QString::number(billSchedules.count());
          text += i18n(" Bills.");
        }
      }

      if (!m_filterDeposits)
      {
        depositSchedules = file->scheduleList("",
                                     MyMoneySchedule::TYPE_DEPOSIT,
                                     MyMoneySchedule::OCCUR_ANY,
                                     MyMoneySchedule::STYPE_ANY,
                                     theDate,
                                     theDate);
        if (m_filterAccounts.count() > 0)
        {
          // Filter out the accounts
          QValueList<MyMoneySchedule> toDelete;
          QValueList<MyMoneySchedule>::Iterator schedit;
          for (schedit=depositSchedules.begin(); schedit!=depositSchedules.end(); ++schedit)
          {
            QCStringList::Iterator it;
            for (it=m_filterAccounts.begin(); it != m_filterAccounts.end(); ++it)
            {
              if ((*it) == (*schedit).accountId())
              {
                toDelete.append(*schedit);
                break;
              }
            }
          }
          QValueList<MyMoneySchedule>::Iterator delit;
          for (delit=toDelete.begin(); delit!=toDelete.end(); ++delit)
          {
            depositSchedules.remove(delit);
          }
        }
        if (depositSchedules.count() >= 1)
        {
          text += "  ";
          text += QString::number(depositSchedules.count());
          text += i18n("Deposits.");
        }
      }

      if (!m_filterTransfers)
      {
        transferSchedules = file->scheduleList("",
                                     MyMoneySchedule::TYPE_TRANSFER,
                                     MyMoneySchedule::OCCUR_ANY,
                                     MyMoneySchedule::STYPE_ANY,
                                     theDate,
                                     theDate);
        if (m_filterAccounts.count() > 0)
        {
          // Filter out the accounts
          QValueList<MyMoneySchedule> toDelete;
          QValueList<MyMoneySchedule>::Iterator schedit;
          for (schedit=transferSchedules.begin(); schedit!=transferSchedules.end(); ++schedit)
          {
            QCStringList::Iterator it;
            for (it=m_filterAccounts.begin(); it != m_filterAccounts.end(); ++it)
            {
              if ((*it) == (*schedit).accountId())
              {
                toDelete.append(*schedit);
                break;
              }
            }
          }
          QValueList<MyMoneySchedule>::Iterator delit;
          for (delit=toDelete.begin(); delit!=toDelete.end(); ++delit)
          {
            transferSchedules.remove(delit);
          }
        }
        
        if (transferSchedules.count() >= 1)
        {
          text += "  ";
          text += QString::number(transferSchedules.count());
          text += i18n("Transfers.");
        }
      }
    }
    catch (MyMoneyException* e)
    {
      // SAfe to ignore here, cause no schedules might exist
      // for the selected account
      delete e;
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
    case 21:
    case 31:
      text += i18n("st");
      break;
    case 2:
    case 22:
      text += i18n("nd");
      break;
    case 3:
    case 23:
      text += i18n("rd");
      break;
    default:
      text += i18n("th");
  }
}

void kMyMoneyScheduledDateTbl::refresh()
{
  repaintContents(false);
}

void kMyMoneyScheduledDateTbl::contentsMouseMoveEvent(QMouseEvent* e)
{
  int row, col, pos;
  QPoint mouseCoord;

  mouseCoord = e->pos();
  row = rowAt(mouseCoord.y());
  col = columnAt(mouseCoord.x());
  if (row<1 || col<0)
  {
    return;
  }

#if KDE_VERSION < 310
  int firstWeekDay = KGlobal::locale()->weekStartsMonday() ? 1 : 0;
#else
  int firstWeekDay = KGlobal::locale()->weekStartDay();
#endif

  QDate drawDate(date);
  QString text;

  if (m_type == MONTHLY)
  {
    pos=7*(row-1)+col;
    if ( firstWeekDay < 4 )
      pos += firstWeekDay;
    else
      pos += firstWeekDay - 7;

    if (pos<firstday || (firstday+numdays<=pos))
    { // we are either
      // ° painting a day of the previous month or
      // ° painting a day of the following month

      if (pos<firstday)
      { // previous month
        drawDate = drawDate.addMonths(-1);
        text.setNum(numDaysPrevMonth+pos-firstday+1);
        drawDate.setYMD(drawDate.year(), drawDate.month(), text.toInt());
      } else { // following month
        drawDate = drawDate.addMonths(1);
        text.setNum(pos-firstday-numdays+1);
        drawDate.setYMD(drawDate.year(), drawDate.month(), text.toInt());
      }
    } else { // paint a day of the current month
      text.setNum(pos-firstday+1);
      drawDate.setYMD(drawDate.year(), drawDate.month(), text.toInt());
    }
  }
  else if (m_type == WEEKLY)
  {
    // TODO: Handle other start weekdays than Monday
    text = QDate::shortDayName(row);
    text += " ";

    int dayOfWeek = date.dayOfWeek();
    int diff;

    if (row < dayOfWeek)
    {
      diff = -(dayOfWeek - row);
    }
    else
    {
      diff = row - dayOfWeek;
    }

    drawDate = date.addDays(diff);
  }
  else if (m_type == QUARTERLY)
  {
  }

  m_drawDateOrig = drawDate;
  MyMoneyFile *file = MyMoneyFile::instance();
  QValueList<MyMoneySchedule> schedules;

  try
  {
    if (!m_filterBills)
    {
        schedules += file->scheduleList("",
                                     MyMoneySchedule::TYPE_BILL,
                                     MyMoneySchedule::OCCUR_ANY,
                                     MyMoneySchedule::STYPE_ANY,
                                     drawDate,
                                     drawDate);
    }

    if (!m_filterDeposits)
    {
        schedules += file->scheduleList("",
                                     MyMoneySchedule::TYPE_DEPOSIT,
                                     MyMoneySchedule::OCCUR_ANY,
                                     MyMoneySchedule::STYPE_ANY,
                                     drawDate,
                                     drawDate);
    }

    if (!m_filterTransfers)
    {
        schedules += file->scheduleList("",
                                     MyMoneySchedule::TYPE_TRANSFER,
                                     MyMoneySchedule::OCCUR_ANY,
                                     MyMoneySchedule::STYPE_ANY,
                                     drawDate,
                                     drawDate);
    }
    if (m_filterAccounts.count() > 0)
    {
      // Filter out the accounts
      QValueList<MyMoneySchedule> toDelete;
      QValueList<MyMoneySchedule>::Iterator schedit;
      for (schedit=schedules.begin(); schedit!=schedules.end(); ++schedit)
      {
        QCStringList::Iterator it;
        for (it=m_filterAccounts.begin(); it != m_filterAccounts.end(); ++it)
        {
          if ((*it) == (*schedit).accountId())
          {
            toDelete.append(*schedit);
            break;
          }
        }
      }
      QValueList<MyMoneySchedule>::Iterator delit;
      for (delit=toDelete.begin(); delit!=toDelete.end(); ++delit)
      {
        schedules.remove(delit);
      }
    }
  }
  catch ( MyMoneyException* e)
  {
    // SAfe to ignore here, cause no schedules might exist
    // for the selected account
    delete e;
  }

  emit hoverSchedules(schedules, drawDate);
}

void kMyMoneyScheduledDateTbl::filterBills(bool enable)
{
  m_filterBills = enable;
  repaintContents(false);
}

void kMyMoneyScheduledDateTbl::filterDeposits(bool enable)
{
  m_filterDeposits = enable;
  repaintContents(false);
}

void kMyMoneyScheduledDateTbl::filterTransfers(bool enable)
{
  m_filterTransfers = enable;
  repaintContents(false);
}
