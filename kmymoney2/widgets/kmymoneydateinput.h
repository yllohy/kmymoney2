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

#include "./kdatepik.h"

#include <qwidget.h>
#include <qlineedit.h>
#include <qdatetime.h>

// This class replaces the DateInput class I was using from
// Calendar-0.13.
// It has been designed to have the same interface as DateInput.
// Some ideas/code have been borrowed from Calendar-0.13 (phoenix.bmedesign.com/~qt)
class kMyMoneyDateInput : public QWidget  {
   Q_OBJECT

public: 
	kMyMoneyDateInput(QWidget *parent=0, const char *name=0, Qt::AlignmentFlags flags=Qt::AlignLeft);
  kMyMoneyDateInput(QWidget *parent=0, const QDate& date=QDate::currentDate(), Qt::AlignmentFlags flags=Qt::AlignLeft);
	~kMyMoneyDateInput();
	
	// Use this to get the selected date
	QDate getQDate(void);
	void setDate(QDate date);
  QWidget* getLineEdit();
/*
  virtual void hide();
  virtual void show();
*/
  virtual QSize sizeHint() const;
  virtual QSizePolicy sizePolicy() const;


protected:
  void paintEvent(QPaintEvent*);
  void resizeEvent(QResizeEvent*);
  void mousePressEvent(QMouseEvent* qme);
  /** Overriding QWidget::keyPressEvent */
  void keyPressEvent(QKeyEvent * k);

protected slots:
  void slotDateChosen(QDate date);
  void slotEnterPressed();

private:
  KDateEdit *dateEdit;
  KTempDatePicker *datePicker;
  QDate m_date;  // The date !
  Qt::AlignmentFlags m_qtalignment;
};

#endif
