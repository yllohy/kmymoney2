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
#include <knumvalidator.h>

// ----------------------------------------------------------------------------
// Project Includes

class MyMoneyMoney;
class kMyMoneyCalculator;

// This class is derived from KDoubleValidator and uses
// the monetary symbols instead of the numeric symbols.
// Also, it always accepts localized input.
class kMyMoneyMoneyValidator : public KDoubleValidator {
  Q_OBJECT

public:
  /**
    * Constuct a locale-aware KDoubleValidator with default range
    * (whatever @ref QDoubleValidator uses for that) and parent @p
    * parent
    */
  kMyMoneyMoneyValidator( QObject * parent, const char * name=0 );
  /**
    * Constuct a locale-aware KDoubleValidator for range [@p bottom,@p
    * top] and a precision of @p decimals decimals after the decimal
    * point.
    */
  kMyMoneyMoneyValidator( double bottom, double top, int decimals,
                    QObject * parent, const char * name=0 );
  /**
    * Destructs the validator.
    */
  virtual ~kMyMoneyMoneyValidator() {};

  /** Overloaded for internal reasons. The API is not affected. */
  virtual QValidator::State validate( QString & input, int & pos ) const;

};

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
  /**
    * This method ensures that the text version contains a
    * fractional part.
    */
  void ensureFractionalPart(void);
  
protected slots:
  void theTextChanged(const QString & text);
  void slotCalculatorResult();

public:
  kMyMoneyEdit(QWidget *parent=0, const char *name=0);
  ~kMyMoneyEdit();
  MyMoneyMoney getMoneyValue(void);

  void resetText(void);
  
  const bool isValid(void) const;
  
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
