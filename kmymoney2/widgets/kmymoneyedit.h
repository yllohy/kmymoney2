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

#include "kdecompat.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <qhbox.h>
#include <qvalidator.h>

class QVBox;
class QWidget;

// ----------------------------------------------------------------------------
// KDE Includes

#include <klineedit.h>
class KPushButton;

// ----------------------------------------------------------------------------
// Project Includes

class MyMoneyMoney;
class kMyMoneyCalculator;
#include "../mymoney/mymoneyequity.h"

#if KDE_VERSION <= KDE_MAKE_VERSION(3,1,0)
  #define KDoubleValidator QDoubleValidator
#endif

/**
  * This class is derived from KDoubleValidator and uses
  * the monetary symbols instead of the numeric symbols.
  * Also, it always accepts localized input.
  *
  * @author Thomas Baumgart
  */
class kMyMoneyMoneyValidator : public QDoubleValidator
{
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

/**
  * This class represents a widget to enter monetary values.
  * It has an edit field and a button to select a popup
  * calculator. The result of the calculator (if used) is
  * stored in the edit field.
  *
  * @author Michael Edwardes, Thomas Baumgart
  */
class kMyMoneyEdit : public QHBox
{
  Q_OBJECT

private:
  QString previousText; // keep track of what has been typed
  QString m_text;       // keep track of what was the original value
  kMyMoneyCalculator* m_calculator;
  QVBox*              m_calculatorFrame;
  KLineEdit*          m_edit;
  KPushButton*        m_calcButton;
  int                 m_prec;

private:
  /**
    * Internal helper function for value() and ensureFractionalPart(void).
    */
  void ensureFractionalPart(QString& txt) const;

protected:
  /**
    * This method ensures that the text version contains a
    * fractional part.
    */
  void ensureFractionalPart(void);

  /**
    * This method opens the calculator and replays the key
    * event pointed to by @p ev. If @p ev is 0, then no key
    * event is replayed.
    *
    * @param ev pointer to QKeyEvent that started the calculator.
    */
  void calculatorOpen(QKeyEvent* ev);

  /**
    * Helper method for constructors.
    */
  void init(void);

protected slots:
  void theTextChanged(const QString & text);
  void slotCalculatorResult(void);
  void slotCalculatorOpen(void);

public:
  kMyMoneyEdit(QWidget *parent=0, const char *name=0, const int prec = -1);
  kMyMoneyEdit(const MyMoneyEquity& eq, QWidget *parent=0, const char *name=0);
  ~kMyMoneyEdit();

  /**
    * @deprecated Use value() instead
    */
  MyMoneyMoney getMoneyValue(void) KDE_DEPRECATED;

  MyMoneyMoney value(void) const;

  void setValue(const MyMoneyMoney& value);

  void resetText(void);

  const bool isValid(void) const;

  virtual bool eventFilter(QObject * , QEvent * );

  QString text(void) const { return m_edit->text(); };

  void setMinimumWidth(int w) { m_edit->setMinimumWidth(w); };

  /**
    * Set the number of fractional digits that should be shown
    *
    * @param prec number of fractional digits.
    *
    * @note should be used prior to calling loadText()
    */
  void setPrecision(const int prec);

  QWidget* focusWidget(void) const;

  /**
    * This method allows to show/hide the calculator button of the widget.
    * The parameter @p show controls the behavior. Default is to show the
    * button.
    *
    * @param show if true, button is shown, if false it is hidden
    */
  void showCalculatorButton(const bool show = true);

  /**
    * This method allows to hide the calculator button. It is
    * provided as convenience function and is equivilant to @p
    * showCalculatorButton(false).
    */
  void hideCalculatorButton(void) { showCalculatorButton(false); }

public slots:
  void loadText(const QString& text);
  void setReadOnly(bool ro) { m_edit->setReadOnly(ro); };
  void setText(const QString& txt) { m_edit->setText(txt); };

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

  void textChanged(const QString& text);
};

#endif
