/***************************************************************************
                          kmymoneydateinput.h
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

#ifndef KMYMONEYDATEINPUT_H
#define KMYMONEYDATEINPUT_H

#include <kdatepicker.h>

#include <qwidget.h>
#include <qlineedit.h>
#include <qdatetime.h>
#include <qdatetimeedit.h>
#include <qvbox.h>
#include <qpushbutton.h>

// Ideas neatly taken from korganizer
// Respective authors are credited.
// Some ideas/code have been borrowed from Calendar-0.13 (phoenix.bmedesign.com/~qt)

class kMyMoneyDateInput : public QHBox  {
   Q_OBJECT

public: 
	kMyMoneyDateInput(QWidget *parent=0, const char *name=0, Qt::AlignmentFlags flags=Qt::AlignLeft);
	~kMyMoneyDateInput();
	
	// Use this to get the selected date
	QDate getQDate(void);
	void setDate(QDate date);
  void loadDate(const QDate& date);
  void resetDate(void);

signals:
  void dateChanged(const QDate& date);

protected:
  /** Overriding QWidget::keyPressEvent */
  //void keyPressEvent(QKeyEvent * k);

protected slots:
  void slotDateChosen(QDate date);
  void slotEnterPressed();
	void toggleDatePicker();

private:
  QDateEdit *dateEdit;
  KDatePicker *m_datePicker;
  QDate m_date;  // The date !
  QDate m_prevDate;
  Qt::AlignmentFlags m_qtalignment;
	QVBox *m_dateFrame;
	QPushButton *m_dateButton;
};

#endif

