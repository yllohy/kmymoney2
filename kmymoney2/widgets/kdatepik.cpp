// This file is modified from the original KDE2 1.92 libraries until
// they are updated.
// I will then remove these two files (kdatepik.h kdatepik.cpp) from the
// source tree
// Michael Edwardes 15/10/2000

/*  -*- C++ -*-
    This file is part of the KDE libraries
    Copyright (C) 1997 Tim D. Gilman (tdgilman@best.org)
              (C) 1998, 1999 Mirko Sucker (mirko@kde.org)
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

#include "kdatepik.h"
#include <kglobal.h>
#include <kapp.h>
#include <klocale.h>
#include <kiconloader.h>
#include <qframe.h>
#include <qpainter.h>
#include <qtableview.h>
#include <qdialog.h>
#include <qtoolbutton.h>
#include <qfont.h>
#include <qlineedit.h>
#include <qvalidator.h>
#include <kdebug.h>
#include <kdatetbl.h>
//#include "kdatepik.moc"

KTempDatePicker::KTempDatePicker(QWidget *parent, QDate dt, const char *name, WFlags f)
  : QFrame(parent,name, f),
    yearForward(new QToolButton(this)),
    yearBackward(new QToolButton(this)),
    monthForward(new QToolButton(this)),
    monthBackward(new QToolButton(this)),
    selectMonth(new QToolButton(this)),
    selectYear(new QToolButton(this)),
    line(new QLineEdit(this)),
    val(new KDateValidator(this)),
    table(new KDateTable(this)),
    fontsize(10)
{
  // -----
  setFontSize(10);
  line->setValidator(val);
  yearForward->setPixmap(BarIcon(QString::fromLatin1("2rightarrow")));
  yearBackward->setPixmap(BarIcon(QString::fromLatin1("2leftarrow")));
  monthForward->setPixmap(BarIcon(QString::fromLatin1("1rightarrow")));
  monthBackward->setPixmap(BarIcon(QString::fromLatin1("1leftarrow")));
  setDate(dt); // set button texts
  connect(table, SIGNAL(dateChanged(QDate)), SLOT(dateChangedSlot(QDate)));
  connect(table, SIGNAL(tableClicked()), SLOT(tableClickedSlot()));
  connect(monthForward, SIGNAL(clicked()), SLOT(monthForwardClicked()));
  connect(monthBackward, SIGNAL(clicked()), SLOT(monthBackwardClicked()));
  connect(yearForward, SIGNAL(clicked()), SLOT(yearForwardClicked()));
  connect(yearBackward, SIGNAL(clicked()), SLOT(yearBackwardClicked()));
  connect(selectMonth, SIGNAL(clicked()), SLOT(selectMonthClicked()));
  connect(selectYear, SIGNAL(clicked()), SLOT(selectYearClicked()));
  connect(line, SIGNAL(returnPressed()), SLOT(lineEnterPressed()));
}

KTempDatePicker::~KTempDatePicker()
{
}

void
KTempDatePicker::resizeEvent(QResizeEvent*)
{
    int cx, cy; // counters for storing size hint values
    QWidget *buttons[] = {
	yearBackward,
	    monthBackward,
	    selectMonth,
	    selectYear,
	    monthForward,
	    yearForward };
    const int NoOfButtons=sizeof(buttons)/sizeof(buttons[0]);
    QSize sizes[NoOfButtons];
    int buttonHeight=0;
    int count;
    int w;
    int x=0;
    // ----- calculate button row height:
    for(count=0; count<NoOfButtons; ++count) {
	sizes[count]=buttons[count]->sizeHint();
	buttonHeight=QMAX(buttonHeight, sizes[count].height());
    }
    cy=buttonHeight;
    // ----- calculate size of the month button:
    w=0;
    for(count=0; count<NoOfButtons; ++count) {
	if(buttons[count]!=selectMonth)
	{
	    w+=sizes[count].width();
	} else {
	    x=count;
	}
    }
    cx=w+sizes[count].width(); // sum of all recommended button widths
    sizes[x].setWidth(width()-w); // stretch the month button
    // ----- place the buttons:
    x=0;
    for(count=0; count<NoOfButtons; ++count)
    {
	w=sizes[count].width();
	buttons[count]->setGeometry(x, 0, w, buttonHeight);
	x+=w;
    }
    // ----- place the line edit for direct input:
    sizes[0]=line->sizeHint();
    line->setGeometry(0, height()-sizes[0].height(), width(), sizes[0].height());
    // ----- adjust the table:
    table->setGeometry(0, buttonHeight, width(),
		       height()-buttonHeight-sizes[0].height());
}

void
KTempDatePicker::dateChangedSlot(QDate date)
{
    kdDebug() << "KDatePicker::dateChangedSlot: date changed (" << date.year() << "/" << date.month() << "/" << date.day() << ")." << endl;
    line->setText(KGlobal::locale()->formatDate(date, true));
    emit(dateChanged(date));
}

void
KTempDatePicker::tableClickedSlot()
{
  kdDebug() << "KDatePicker::tableClickedSlot: table clicked." << endl;
  emit(dateSelected(table->getDate()));
  emit(tableClicked());
}

const QDate&
KTempDatePicker::getDate()
{
  return table->getDate();
}

bool
KTempDatePicker::setDate(const QDate& date)
{
    if(date.isValid()) {
	QString temp;
	// -----
	table->setDate(date);
	selectMonth->setText(KGlobal::locale()->monthName(date.month(), false));
	temp.setNum(date.year());
	selectYear->setText(temp);
	line->setText(KGlobal::locale()->formatDate(date, true));
	return true;
    } else {
	kdDebug() << "KDatePicker::setDate: refusing to set invalid date." << endl;
	return false;
    }
}

void
KTempDatePicker::monthForwardClicked()
{
    QDate temp=table->getDate();
    int day=temp.day();
    // -----
    if(temp.month()==12) {
	temp.setYMD(temp.year()+1, 1, 1);
    } else {
	temp.setYMD(temp.year(), temp.month()+1, 1);
    }
    if(temp.daysInMonth()<day) {
	temp.setYMD(temp.year(), temp.month(), temp.daysInMonth());
    } else {
	temp.setYMD(temp.year(), temp.month(), day);
    }
    // assert(temp.isValid());
    setDate(temp);
}

void
KTempDatePicker::monthBackwardClicked()
{
  QDate temp=table->getDate();
  int day=temp.day();
  // -----
  if(temp.month()==1)
    {
      temp.setYMD(temp.year()-1, 12, 1);
    } else {
      temp.setYMD(temp.year(), temp.month()-1, 1);
    }
  if(temp.daysInMonth()<day)
    {
      temp.setYMD(temp.year(), temp.month(), temp.daysInMonth());
    } else {
      temp.setYMD(temp.year(), temp.month(), day);
    }
  // assert(temp.isValid());
  setDate(temp);
}

void
KTempDatePicker::yearForwardClicked()
{
  QDate temp=table->getDate();
  int day=temp.day();
  // -----
  temp.setYMD(temp.year()+1, temp.month(), 1);
  if(temp.daysInMonth()<day)
    {
      temp.setYMD(temp.year(), temp.month(), temp.daysInMonth());
    } else {
      temp.setYMD(temp.year(), temp.month(), day);
    }
  // assert(temp.isValid());
  setDate(temp);
}

void
KTempDatePicker::yearBackwardClicked()
{
  QDate temp=table->getDate();
  int day=temp.day();
  // -----
  temp.setYMD(temp.year()-1, temp.month(), 1);
  if(temp.daysInMonth()<day)
    {
      temp.setYMD(temp.year(), temp.month(), temp.daysInMonth());
    } else {
      temp.setYMD(temp.year(), temp.month(), day);
    }
  // assert(temp.isValid());
  setDate(temp);
}

void
KTempDatePicker::selectMonthClicked()
{
  int month;
  KPopupFrame popup;
  KDateInternalMonthPicker picker(fontsize, &popup);
  // -----
  picker.resize(picker.sizeHint());
  popup.setMainWidget(&picker);
  picker.setFocus();
  connect(&picker, SIGNAL(closeMe(int)), &popup, SLOT(close(int)));
  if(popup.exec(selectMonth->mapToGlobal(QPoint(0, selectMonth->height()))))
    {
      QDate date;
      int day;
      // -----
      month=picker.getResult();
      date=table->getDate();
      day=date.day();
      // ----- construct a valid date in this month:
      date.setYMD(date.year(), month, 1);
      date.setYMD(date.year(), month, QMIN(day, date.daysInMonth()));
      // ----- set this month
      setDate(date);
    } else {
      kapp->beep();
    }
}

void
KTempDatePicker::selectYearClicked()
{
  int year;
  KPopupFrame popup;
  KDateInternalYearSelector picker(fontsize, &popup);
  // -----
  picker.resize(picker.sizeHint());
  popup.setMainWidget(&picker);
  connect(&picker, SIGNAL(closeMe(int)), &popup, SLOT(close(int)));
  if(popup.exec(selectYear->mapToGlobal(QPoint(0, selectMonth->height()))))
    {
      QDate date;
      int day;
      // -----
      year=picker.getYear();
      date=table->getDate();
      day=date.day();
      // ----- construct a valid date in this month:
      date.setYMD(year, date.month(), 1);
      date.setYMD(year, date.month(), QMIN(day, date.daysInMonth()));
      // ----- set this month
      setDate(date);
    } else {
      kapp->beep();
    }
}

void
KTempDatePicker::setEnabled(bool enable)
{
  QWidget *widgets[]= {
    yearForward, yearBackward, monthForward, monthBackward,
    selectMonth, selectYear,
    line, table };
  const int Size=sizeof(widgets)/sizeof(widgets[0]);
  int count;
  // -----
  for(count=0; count<Size; ++count)
    {
      widgets[count]->setEnabled(enable);
    }
}

void
KTempDatePicker::lineEnterPressed()
{
  QDate temp;
  // -----
  if(val->date(line->text(), temp)==QValidator::Acceptable)
    {
	kdDebug() << "KDatePicker::lineEnterPressed: valid date entered." << endl;
	emit(dateEntered(temp));
	setDate(temp);
    } else {
      kapp->beep();
      kdDebug() << "KDatePicker::lineEnterPressed: invalid date entered." << endl;
    }
}

QSize
KTempDatePicker::sizeHint() const
{
  QSize tableSize=table->sizeHint();
  QWidget *buttons[]={
    yearBackward,
    monthBackward,
    selectMonth,
    selectYear,
    monthForward,
    yearForward };
  const int NoOfButtons=sizeof(buttons)/sizeof(buttons[0]);
  QSize sizes[NoOfButtons];
  int cx=0, cy=0, count;
  // ----- store the size hints:
  for(count=0; count<NoOfButtons; ++count)
    {
      sizes[count]=buttons[count]->sizeHint();
      if(buttons[count]==selectMonth)
	{
	  cx+=maxMonthRect.width();
	} else {
	  cx+=sizes[count].width();
	}
      cy=QMAX(sizes[count].height(), cy);
    }
  // ----- calculate width hint:
  cx=QMAX(cx, tableSize.width()); // line edit ignored
  // ----- calculate height hint:
  cy+=tableSize.height()+line->sizeHint().height();
  return QSize(cx, cy);
}

void
KTempDatePicker::setFontSize(int s)
{
  QWidget *buttons[]= {
    // yearBackward,
    // monthBackward,
    selectMonth,
    selectYear,
    // monthForward,
    // yearForward
  };
  const int NoOfButtons=sizeof(buttons)/sizeof(buttons[0]);
  int count;
  QFont font;
  QRect r;
  // -----
  fontsize=s;
  for(count=0; count<NoOfButtons; ++count)
    {
      font=buttons[count]->font();
      font.setPointSize(s);
      buttons[count]->setFont(font);
    }
  QFontMetrics metrics(selectMonth->fontMetrics());
  for(int i=1; i <= 12; ++i)
    { // maxMonthRect is used by sizeHint()
      r=metrics.boundingRect(KGlobal::locale()->monthName(i, false));
      maxMonthRect.setWidth(QMAX(r.width(), maxMonthRect.width()));
      maxMonthRect.setHeight(QMAX(r.height(),  maxMonthRect.height()));
    }
  table->setFontSize(s);
}
