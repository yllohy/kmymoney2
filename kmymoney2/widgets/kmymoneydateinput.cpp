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
#include <qtimer.h>

#include "kmymoneydateinput.h"

#if QT_VERSION > 300
#include <qstyle.h>
#endif

kMyMoneyDateInput::kMyMoneyDateInput(QWidget *parent, const char *name, Qt::AlignmentFlags flags)
 : QHBox(parent,name)
{
  m_qtalignment = flags;
  m_date = QDate::currentDate();

  dateEdit = new QDateEdit(m_date, this, "dateEdit");
	setFocusProxy(dateEdit);
	
  m_dateFrame = new QVBox(0,0,WType_Popup);
	m_dateFrame->setFrameStyle(QFrame::PopupPanel | QFrame::Raised);
  m_dateFrame->setFixedSize(200,200);
  m_dateFrame->setLineWidth(3);
  m_dateFrame->hide();
	
	m_datePicker = new KDatePicker(m_dateFrame, m_date);
	
	m_dateButton = new QPushButton(this);
	
	connect(m_dateButton,SIGNAL(clicked()),SLOT(toggleDatePicker()));
  connect(dateEdit, SIGNAL(returnPressed()), this, SLOT(slotEnterPressed()));
	connect(m_datePicker, SIGNAL(dateSelected(QDate)), this, SLOT(slotDateChosen(QDate)));
  connect(m_datePicker, SIGNAL(dateEntered(QDate)), this, SLOT(slotDateChosen(QDate)));
	connect(m_datePicker, SIGNAL(dateSelected(QDate)), m_dateFrame, SLOT(hide()));
}

kMyMoneyDateInput::~kMyMoneyDateInput()
{
  delete m_dateFrame;
}

void kMyMoneyDateInput::toggleDatePicker()
{
  if(m_dateFrame->isVisible())
	{
		m_dateFrame->hide();
	}
	else
	{
    QPoint tmpPoint = mapToGlobal(m_dateButton->geometry().bottomRight());
    if (m_qtalignment == Qt::AlignRight)
		{
	    m_dateFrame->setGeometry(tmpPoint.x(), tmpPoint.y(), 200, 200);
		}
		else
		{
			tmpPoint.setX(tmpPoint.x() - m_datePicker->width());
	    m_dateFrame->setGeometry(tmpPoint.x(), tmpPoint.y(), 200, 200);
		}

    QDate date = dateEdit->date();
    if(date.isValid())
		{
      m_datePicker->setDate(date);
    }
		else
		{
      m_datePicker->setDate(QDate::currentDate());
    }
    
		m_dateFrame->show();
  }
}

/** Overriding QWidget::keyPressEvent
  *
  * increments/decrements the date upon +/- key input
  * /
void kMyMoneyDateInput::keyPressEvent(QKeyEvent * k)
{
  if (k->key()==Key_Plus) {
     slotDateChosen(m_date.addDays(1));
  } else if (k->key()==Key_Minus) {
     slotDateChosen(m_date.addDays(-1));
  } else if (k->key()==Key_PageDown) {
     QPoint point = mapToGlobal(rect().bottomRight());
     if (m_qtalignment == Qt::AlignRight)
      point.setX(point.x());
     else
      point.setX(point.x() - m_datePicker->width());

    m_datePicker->move(point);
    m_datePicker->show();
    return;
  }
}
*/

void kMyMoneyDateInput::slotDateChosen(QDate date)
{
  dateEdit->setDate(date);
  m_date=date;
  m_datePicker->setDate(m_date);
}

void kMyMoneyDateInput::slotEnterPressed()
{
  dateEdit->setDate(m_date);
  m_datePicker->setDate(m_date);
}

QDate kMyMoneyDateInput::getQDate(void)
{
  return m_date;
}

void kMyMoneyDateInput::setDate(QDate date)
{
  slotDateChosen(date);
}

