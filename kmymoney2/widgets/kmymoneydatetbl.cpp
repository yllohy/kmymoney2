/***************************************************************************
                          kmymoneydatetbl.cpp  -  description
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
#include <qdatetime.h>
#include <qstring.h>
#include <qpen.h>
#include <qpainter.h>
#include <qdialog.h>

// ----------------------------------------------------------------------------
// KDE Includes
#include <kdatetbl.h> // Use the classes available for maximum re-use
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kapplication.h>
#include <klocale.h>
#include <kdebug.h>
#include <knotifyclient.h>

// ----------------------------------------------------------------------------
// Project Includes
#include "kmymoneydatetbl.h"

//#include <assert.h>

kMyMoneyDateTbl::kMyMoneyDateTbl(QWidget *parent, QDate date_, const char* name, WFlags f)
  : QGridView(parent, name, f)
{
  setFontSize(10);
  if(!date_.isValid())
    {
      kdDebug() << "kMyMoneyDateTbl ctor: WARNING: Given date is invalid, using current date." << endl;
      date_=QDate::currentDate();
    }
  setFocusPolicy( QWidget::StrongFocus );
  setNumRows(7); // 6 weeks max + headline
  setNumCols(7); // 7 days a week
  setHScrollBarMode(AlwaysOff);
  setVScrollBarMode(AlwaysOff);
  viewport()->setEraseColor(KGlobalSettings::baseColor());
  setDate(date_); // this initializes firstday, numdays, numDaysPrevMonth
}

void
kMyMoneyDateTbl::paintCell(QPainter *painter, int row, int col)
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
  int firstWeekDay = KGlobal::locale()->weekStartDay();
  if(row==0)
    { // we are drawing the headline
      font.setBold(true);
      painter->setFont(font);
      bool normalday = true;
      QString daystr;
      if ( col+firstWeekDay < 8 )
          daystr = KGlobal::locale()->weekDayName(col+firstWeekDay, true);
      else
          daystr = KGlobal::locale()->weekDayName(col+firstWeekDay-7, true);
      if ( daystr==i18n("Sunday", "Sun") || daystr==i18n("Saturday", "Sat") )
          normalday=false;

      if (!normalday)
        {
          painter->setPen(KGlobalSettings::baseColor());
          painter->setBrush(brushLightblue);
          painter->drawRect(0, 0, w, h);
          painter->setPen(KGlobalSettings::activeTitleColor());
        } else {
          painter->setPen(KGlobalSettings::activeTitleColor());
          painter->setBrush(brushBlue);
          painter->drawRect(0, 0, w, h);
          painter->setPen(KGlobalSettings::activeTextColor());
        }
      painter->drawText(0, 0, w, h-1, AlignCenter,
                        daystr, -1, &rect);
      painter->setPen(KGlobalSettings::textColor());
      painter->moveTo(0, h-1);
      painter->lineTo(w-1, h-1);
      // ----- draw the weekday:
    } else {
      painter->setFont(font);
      pos=7*(row-1)+col;
      if ( firstWeekDay < 4 )
          pos += firstWeekDay;
      else
          pos += firstWeekDay - 7;
      if(pos<firstday || (firstday+numdays<=pos))
        { // we are either
          // ° painting a day of the previous month or
          // ° painting a day of the following month
          if(pos<firstday)
            { // previous month
              text.setNum(numDaysPrevMonth+pos-firstday+1);
            } else { // following month
              text.setNum(pos-firstday-numdays+1);
            }
          painter->setPen(gray);
        } else { // paint a day of the current month
          text.setNum(pos-firstday+1);
          painter->setPen(KGlobalSettings::textColor());
        }

      pen=painter->pen();
      if(firstday+date.day()-1==pos)
        {
          if(hasFocus())
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
         painter->setPen(KGlobalSettings::textColor());
      }

      painter->drawRect(0, 0, w, h);
      painter->setPen(pen);
      painter->drawText(0, 0, w, h, AlignCenter, text, -1, &rect);
    }
  if(rect.width()>maxCell.width()) maxCell.setWidth(rect.width());
  if(rect.height()>maxCell.height()) maxCell.setHeight(rect.height());
}

void
kMyMoneyDateTbl::keyPressEvent( QKeyEvent *e )
{
    if ( e->key() == Qt::Key_Prior ) {
        setDate(date.addMonths(-1));
        return;
    }
    if ( e->key() == Qt::Key_Next ) {
        setDate(date.addMonths(1));
        return;
    }

    if ( e->key() == Qt::Key_Up ) {
        if ( date.day() > 7 ) {
            setDate(date.addDays(-7));
            return;
        }
    }
    if ( e->key() == Qt::Key_Down ) {
        if ( date.day() <= date.daysInMonth()-7 ) {
            setDate(date.addDays(7));
            return;
        }
    }
    if ( e->key() == Qt::Key_Left ) {
        if ( date.day() > 1 ) {
            setDate(date.addDays(-1));
            return;
        }
    }
    if ( e->key() == Qt::Key_Right ) {
        if ( date.day() < date.daysInMonth() ) {
            setDate(date.addDays(1));
            return;
        }
    }

    if ( e->key() == Qt::Key_Minus ) {
        setDate(date.addDays(-1));
        return;
    }
    if ( e->key() == Qt::Key_Plus ) {
        setDate(date.addDays(1));
        return;
    }
    if ( e->key() == Qt::Key_N ) {
        setDate(QDate::currentDate());
        return;
    }

    KNotifyClient::beep();
}

void
kMyMoneyDateTbl::viewportResizeEvent(QResizeEvent * e)
{
  QGridView::viewportResizeEvent(e);

  setCellWidth(viewport()->width()/7);
  setCellHeight(viewport()->height()/7);
}

void
kMyMoneyDateTbl::setFontSize(int size)
{
  int count;
  QFontMetrics metrics(fontMetrics());
  QRect rect;
  // ----- store rectangles:
  fontsize=size;
  // ----- find largest day name:
  maxCell.setWidth(0);
  maxCell.setHeight(0);
  for(count=0; count<7; ++count)
    {
      rect=metrics.boundingRect(KGlobal::locale()->weekDayName(count+1, true));
      maxCell.setWidth(QMAX(maxCell.width(), rect.width()));
      maxCell.setHeight(QMAX(maxCell.height(), rect.height()));
    }
  // ----- compare with a real wide number and add some space:
  rect=metrics.boundingRect(QString::fromLatin1("88"));
  maxCell.setWidth(QMAX(maxCell.width()+2, rect.width()));
  maxCell.setHeight(QMAX(maxCell.height()+4, rect.height()));
}

void
kMyMoneyDateTbl::wheelEvent ( QWheelEvent * e )
{
    setDate(date.addMonths( -(int)(e->delta()/120)) );
    e->accept();
}

void
kMyMoneyDateTbl::contentsMousePressEvent(QMouseEvent *e)
{
  if(e->type()!=QEvent::MouseButtonPress)
    { // the KDatePicker only reacts on mouse press events:
      return;
    }
  if(!isEnabled())
    {
      KNotifyClient::beep();
      return;
    }

  //int dayoff = KGlobal::locale()->weekStartsMonday() ? 1 : 0;
  int dayoff = KGlobal::locale()->weekStartDay();
  // -----
  int row, col, pos, temp;
  QPoint mouseCoord;
  // -----
  mouseCoord = e->pos();
  row=rowAt(mouseCoord.y());
  col=columnAt(mouseCoord.x());
  if(row<1 || col<0)
    { // the user clicked on the frame of the table
      return;
    }

  // Rows and columns are zero indexed.  The (row - 1) below is to avoid counting
  // the row with the days of the week in the calculation.  We however want pos
  // to be "1 indexed", hence the "+ 1" at the end of the sum.

  pos = (7 * (row - 1)) + col + 1;

  // This gets pretty nasty below.  firstday is a locale independant index for
  // the first day of the week.  dayoff is the day of the week that the week
  // starts with for the selected locale.  Monday = 1 .. Sunday = 7
  // Strangely, in some cases dayoff is in fact set to 8, hence all of the
  // "dayoff % 7" sections below.

  if(pos + dayoff % 7 <= firstday)
    { // this day is in the previous month
      setDate(date.addDays(-1 * (date.day() + firstday - pos - dayoff % 7)));
      return;
    }
  if(firstday + numdays < pos + dayoff % 7)
    { // this date is in the next month
      setDate(date.addDays(pos - firstday - date.day() + dayoff % 7));
      return;
    }
  temp = firstday + date.day() - dayoff % 7 - 1;

  setDate(QDate(date.year(), date.month(), pos - firstday + dayoff % 7));

  updateCell(temp/7+1, temp%7); // Update the previously selected cell
  updateCell(row, col); // Update the selected cell
  // assert(QDate(date.year(), date.month(), pos-firstday+dayoff).isValid());
  emit(tableClicked());
}

bool
kMyMoneyDateTbl::setDate(const QDate& date_)
{
  bool changed=false;
  QDate temp;
  // -----
  if(!date_.isValid())
    {
      kdDebug() << "kMyMoneyDateTbl::setDate: refusing to set invalid date." << endl;
      return false;
    }
  if(date!=date_)
    {
      date=date_;
      changed=true;
    }
  temp.setYMD(date.year(), date.month(), 1);
  firstday=temp.dayOfWeek();
  if(firstday==1) firstday=8;
  numdays=date.daysInMonth();
  if(date.month()==1)
    { // set to december of previous year
      temp.setYMD(date.year()-1, 12, 1);
    } else { // set to previous month
      temp.setYMD(date.year(), date.month()-1, 1);
    }
  numDaysPrevMonth=temp.daysInMonth();
  if(changed)
    {
      repaintContents(false);
    }
  emit(dateChanged(date));
  return true;
}

const QDate&
kMyMoneyDateTbl::getDate() const
{
  return date;
}

// what are those repaintContents() good for? (pfeiffer)
void kMyMoneyDateTbl::focusInEvent( QFocusEvent *e )
{
//    repaintContents(false);
    QGridView::focusInEvent( e );
}

void kMyMoneyDateTbl::focusOutEvent( QFocusEvent *e )
{
//    repaintContents(false);
    QGridView::focusOutEvent( e );
}

QSize
kMyMoneyDateTbl::sizeHint() const
{
  if(maxCell.height()>0 && maxCell.width()>0)
    {
      return QSize(maxCell.width()*numCols()+2*frameWidth(),
             (maxCell.height()+2)*numRows()+2*frameWidth());
    } else {
      kdDebug() << "kMyMoneyDateTbl::sizeHint: obscure failure - " << endl;
      return QSize(-1, -1);
    }
}

void kMyMoneyDateTbl::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }
