/***************************************************************************
                          kmymoneydateinput.cpp
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes
    email                : mte@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <kglobal.h>
#include <klocale.h>

#include <qpainter.h>
#include <qdrawutil.h>
#include <qpoint.h>
#include <qvalidator.h>
#include <kdatetbl.h>

#include "kmymoneydateinput.h"

kMyMoneyDateInput::kMyMoneyDateInput(QWidget *parent, const char *name )
 : QWidget(parent,name)
{
  lineEdit=new QLineEdit(this);
  lineEdit->setReadOnly(true);

//  connect(lineEdit,SIGNAL(returnPressed()),SLOT(slotEnterPressed()));

  // I use KTempDatePicker so I can use the WType_Popup flag in the constructor
  datePicker = new KTempDatePicker(parent, QDate::currentDate(), "datePicker", WType_Popup);
  datePicker->resize(190, 130);
  datePicker->hide();

  connect(datePicker, SIGNAL(dateSelected(QDate)), this, SLOT(slotDateChosen(QDate)));
  connect(datePicker, SIGNAL(dateEntered(QDate)), this, SLOT(slotDateChosen(QDate)));

  slotDateChosen(QDate::currentDate());
}

kMyMoneyDateInput::kMyMoneyDateInput(QWidget *parent, const QDate& date )
 : QWidget(parent)
{
  lineEdit=new QLineEdit(this);
  lineEdit->setReadOnly(true);

  // I use KTempDatePicker so I can use the WType_Popup flag in the constructor
  datePicker = new KTempDatePicker(parent, date, "datePicker", WType_Popup);
  datePicker->resize(190, 130);
  datePicker->hide();

  connect(datePicker, SIGNAL(dateSelected(QDate)), this, SLOT(slotDateChosen(QDate)));
  connect(datePicker, SIGNAL(dateEntered(QDate)), this, SLOT(slotDateChosen(QDate)));

  slotDateChosen(QDate::currentDate());
}

kMyMoneyDateInput::~kMyMoneyDateInput()
{
}

void kMyMoneyDateInput::paintEvent(QPaintEvent*)
{
  // Code mostly borrowed from Calendar-0.13. See header file
  QPainter paint;
  paint.begin(this);
  qDrawShadePanel(&paint,0,0,width(),height(),colorGroup(),FALSE,1,NULL);
  style().drawArrow(&paint,DownArrow,FALSE,width()-19,5,height()-12,
    height()-12,colorGroup(), this->isEnabled());
  qDrawShadeLine(&paint,width()-18,height()-7,width()-7,height()-7,
    colorGroup(),FALSE,1,1);
  paint.end();
}

void kMyMoneyDateInput::resizeEvent(QResizeEvent*)
{
  lineEdit->setGeometry(0,0,width()-23,height());
}

void kMyMoneyDateInput::mousePressEvent(QMouseEvent* qme)
{
  // Code mostly borrowed from Calendar-0.13. See header file
  int x = qme->x();
  int y = qme->y();
  if (x > width()-20 && x < width()-4 && y > 5 && y < height()-5) {
    if (datePicker->isVisible()) {
      datePicker->hide();
    } else {
      QPoint point = mapToGlobal(rect().bottomRight());
      point.setX(point.x());
//      point.setX(point.x() - datePicker->width());
      datePicker->move(point);
      datePicker->show();
    }
  }
}

void kMyMoneyDateInput::slotDateChosen(QDate date)
{
  lineEdit->setText(KGlobal::locale()->formatDate(date, true));
  m_date=date;
  datePicker->hide();
}

/*
void kMyMoneyDateInput::hide()
{
 	lineEdit->hide();
  //datePicker->hide();
}

void kMyMoneyDateInput::show()
{
 	lineEdit->show();
  //datePicker->show();
}
*/
QWidget* kMyMoneyDateInput::getLineEdit()
{
 	return lineEdit;
}

QDate kMyMoneyDateInput::getQDate(void)
{
  return m_date;
}

void kMyMoneyDateInput::setDate(QDate date)
{
  slotDateChosen(date);
}

QSize kMyMoneyDateInput::sizeHint() const
{
  if (lineEdit)
    return lineEdit->sizeHint();

  return QSize(-1, -1);
}

QSizePolicy kMyMoneyDateInput::sizePolicy() const
{
  if (lineEdit)
    return lineEdit->sizePolicy();

  return QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
}
