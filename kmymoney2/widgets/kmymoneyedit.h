/***************************************************************************
                          kmymoneyedit.h
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

#ifndef KMYMONEYEDIT_H
#define KMYMONEYEDIT_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qwidget.h>
#include <qvbox.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klineedit.h>

// ----------------------------------------------------------------------------
// Project Includes

class MyMoneyMoney;
class kMyMoneyCalculator;


// This class replaces OE Hansens kdbMoneyEdit I used
// to use.  It has simpler interface and fixes some
// issues I had with the original widget.
class kMyMoneyEdit : public KLineEdit  {
   Q_OBJECT

private:
  QString previousText; // keep track of what has been typed
  QString m_text;       // keep track of what was the original value
  kMyMoneyCalculator* m_calculator;
  QVBox*              m_calculatorFrame;

protected:
  void focusOutEvent(QFocusEvent *e);

protected slots:
  void theTextChanged(const QString & text);
  void slotCalculatorResult();

public:
  kMyMoneyEdit(QWidget *parent=0, const char *name=0);
  ~kMyMoneyEdit();
  MyMoneyMoney getMoneyValue(void);

  void resetText(void);

  virtual bool eventFilter(QObject * , QEvent * );

public slots:
  void loadText(const QString& text);

signals: // Signals
  /**
    * This signal is emitted when the user presses RETURN while editing
    */
  void signalEnter();

  /**
    * This signal is emitted when the user presses ESC while editing
    */
  void signalEsc();

  /** No descriptions */
  // void signalNextTransaction();
  /** signal is sent, when the tab key is pressed */
  void signalTab();
  /** signal is sent, when the Back-tab (Shift-Tab) key is pressed */
  void signalBackTab();
  /** signal is sent, when amount has been changed by user */
  void valueChanged(const QString& text);
};

#endif
