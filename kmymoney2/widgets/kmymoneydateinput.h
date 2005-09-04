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

// ----------------------------------------------------------------------------
// QT Includes

#include <qwidget.h>
#include <qlineedit.h>
#include <qdatetime.h>
#include <qdatetimeedit.h>
#include <qvbox.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kdatepicker.h>
class KPushButton;

// ----------------------------------------------------------------------------
// Project Includes

// Ideas neatly taken from korganizer
// Respective authors are credited.
// Some ideas/code have been borrowed from Calendar-0.13 (phoenix.bmedesign.com/~qt)

/**
  * This class provides the general widget used for date selection
  * throughout the KMyMoney project. It provides an QDateEdit widget
  * which is based on an edit field with spin boxes and adds a QPushButton
  * to open a KDatePicker.
  */
class kMyMoneyDateInput : public QHBox
{
   Q_OBJECT

public:
  kMyMoneyDateInput(QWidget *parent=0, const char *name=0, Qt::AlignmentFlags flags=Qt::AlignLeft);
  ~kMyMoneyDateInput();

  // Use this to get the selected date
  QDate getQDate(void);
  void setDate(QDate date);
  void loadDate(const QDate& date);
  void resetDate(void);
  QWidget* focusWidget(void) const;

public slots:
  virtual void show(void);

signals:
  void dateChanged(const QDate& date);

protected:
  /**
    * - increments/decrements the date upon +/- key input
    * - decrements the date upon * key input
    * - sets the date to current date when the 'T' key is pressed.
    *   The actual key for this to happen might be overridden through
    *   an i18n package.
    */
  void keyPressEvent(QKeyEvent * k);
  void resizeEvent(QResizeEvent*);

protected slots:
  void slotDateChosen(QDate date);
  void toggleDatePicker();

private slots:
  void slotDateChosenRef(const QDate& date);
  void fixSize(void);

private:
  QDateEdit *dateEdit;
  KDatePicker *m_datePicker;
  QDate m_date;  // The date !
  QDate m_prevDate;
  Qt::AlignmentFlags m_qtalignment;
  QVBox *m_dateFrame;
  KPushButton *m_dateButton;
};

#endif

