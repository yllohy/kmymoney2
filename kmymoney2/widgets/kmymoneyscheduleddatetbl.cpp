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

kMyMoneyScheduledDateTbl::kMyMoneyScheduledDateTbl(QWidget *parent, QDate date_, const char* name, WFlags f )
  : kMyMoneyDateTbl(parent, date_, name, f)
{
}

kMyMoneyScheduledDateTbl::~kMyMoneyScheduledDateTbl()
{
}

void kMyMoneyScheduledDateTbl::drawCellContents(QPainter *painter, int row, int col)
{
  QRect rect;
  QString text;
  QPen pen;
  int w=cellWidth();
  int h=cellHeight();
  int pos;
  QBrush brushBlue(KGlobalSettings::activeTitleColor());
  QBrush brushLightblue(KGlobalSettings::baseColor());
  QFont font=KGlobalSettings::generalFont();
  // -----
  font.setPointSize(fontsize);
  QFont fontLarge(font);
  fontLarge.setPointSize(fontsize*2);
  int firstWeekDay = KGlobal::locale()->weekStartDay();

  painter->setFont(font);
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
      text.setNum(numDaysPrevMonth+pos-firstday+1);
    } else { // following month
      text.setNum(pos-firstday-numdays+1);
    }
    painter->setPen(lightGray);
  } else { // paint a day of the current month
    text.setNum(pos-firstday+1);
    painter->setPen(lightGray);
  }

  pen=painter->pen();
  if (firstday+date.day()-1==pos)
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

  QDate cur_date = QDate::currentDate();
  if ( (date.year()  == cur_date.year()) &&
    (date.month() == cur_date.month()) &&
    (firstday+cur_date.day()-1 == pos) )
  {
    painter->setPen(lightGray);
  }

  painter->drawRect(0, 0, w, h);
  painter->setPen(pen);
  addDayPostfix(text);
  painter->drawText(0, 0, w-2, h, AlignRight, text, -1, &rect);

  if (row == 2)
  {
    painter->setPen(darkGray);
    painter->setFont(fontLarge);
    painter->drawText(0, 0, w, h, AlignCenter, "4", -1, &rect);
  }

  painter->setPen(lightGray);
  painter->setBrush(Qt::NoBrush);
  painter->drawRect(0, 0, w, h);
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
