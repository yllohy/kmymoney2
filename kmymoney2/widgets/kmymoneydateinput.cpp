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

// ----------------------------------------------------------------------------
// QT Includes

#include <qpainter.h>
#include <qdrawutil.h>
#include <qpoint.h>
#include <qvalidator.h>
#include <qtimer.h>
#if QT_VERSION > 300
#include <qstyle.h>
#endif
#include <qlayout.h>
#include <qapplication.h>
#include <qdesktopwidget.h>
#include <qpixmap.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneydateinput.h"

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

  QString dateFormat = KGlobal::locale()->dateFormatShort().lower();
  QString order, separator;
  for(unsigned i = 0; i < dateFormat.length(); ++i) {
    if(dateFormat[i] == 'y' || dateFormat[i] == 'm' || dateFormat[i] == 'd')
      order += dateFormat[i];
    else if(dateFormat[i] != '%' && separator.isEmpty())
      separator = dateFormat[i];
    if(order.length() == 3)
      break;
  }

  // see if we find a known format. If it's unknown, then we use YMD (international)
  if(order == "mdy") {
    dateEdit->setOrder(QDateEdit::MDY);
  } else if(order == "dmy") {
    dateEdit->setOrder(QDateEdit::DMY);
  } else if(order == "ydm") {
    dateEdit->setOrder(QDateEdit::YDM);
  } else {
    dateEdit->setOrder(QDateEdit::YMD);
  }
  dateEdit->setSeparator(separator);

	m_datePicker = new KDatePicker(m_dateFrame, m_date);

  // the next line is a try to add an icon to the button
	m_dateButton = new QPushButton(QIconSet(QPixmap( locate("icon","hicolor/16x16/apps/korganizer.png"))), QString(""), this);
	// m_dateButton = new QPushButton(this);

	connect(m_dateButton,SIGNAL(clicked()),SLOT(toggleDatePicker()));
  connect(dateEdit, SIGNAL(valueChanged(const QDate&)), this, SLOT(slotDateChosenRef(const QDate&)));
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

    // usually, the datepicker widget is shown underneath the dateEdit widget
    // if it does not fit on the screen, we show it above this widget

    if(tmpPoint.y() + 200 > QApplication::desktop()->height()) {
      tmpPoint.setY(tmpPoint.y() - 200 - m_dateButton->height());
    }

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


void kMyMoneyDateInput::resizeEvent(QResizeEvent* ev)
{
  m_dateButton->setMaximumHeight(ev->size().height());
  m_dateButton->setMaximumWidth(ev->size().height());
  dateEdit->setMaximumHeight(ev->size().height());

  // qDebug("Received resize-event %d,%d", ev->size().width(), ev->size().height());
}


/** Overriding QWidget::keyPressEvent
  *
  * increments/decrements the date upon +/- key input
  */
void kMyMoneyDateInput::keyPressEvent(QKeyEvent * k)
{
  switch(k->key()) {
    case Key_Plus:
      slotDateChosen(m_date.addDays(1));
      break;

    case Key_Minus:
      slotDateChosen(m_date.addDays(-1));
      break;

    case Key_Enter:
    case Key_Return:
      emit signalEnter();
      break;

    case Key_Escape:
      emit signalEsc();
      break;

/*
    case Key_PageDown:
      QPoint point = mapToGlobal(rect().bottomRight());
      if (m_qtalignment == Qt::AlignRight)
        point.setX(point.x());
      else
        point.setX(point.x() - m_datePicker->width());

      m_datePicker->move(point);
      m_datePicker->show();
*/
  }
}

void kMyMoneyDateInput::slotDateChosenRef(const QDate& date)
{
  slotDateChosen(QDate(date));
}

void kMyMoneyDateInput::slotDateChosen(QDate date)
{
  dateEdit->blockSignals(true);
  dateEdit->setDate(date);
  m_datePicker->setDate(date);
  m_date=date;

  emit dateChanged(date);

  dateEdit->blockSignals(false);
}

QDate kMyMoneyDateInput::getQDate(void)
{
  return dateEdit->date();
}

void kMyMoneyDateInput::setDate(QDate date)
{
  slotDateChosen(date);
}

void kMyMoneyDateInput::loadDate(const QDate& date)
{
  m_date = m_prevDate = date;

  blockSignals(true);
  slotDateChosen(date);
  blockSignals(false);
}

void kMyMoneyDateInput::resetDate(void)
{
  setDate(m_prevDate);
}

QWidget* kMyMoneyDateInput::focusWidget(void) const
{
  QWidget* w = dateEdit;
  while(w->focusProxy())
    w = w->focusProxy();
  return w;
}
