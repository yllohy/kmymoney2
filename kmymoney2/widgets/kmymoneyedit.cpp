/***************************************************************************
                          kmymoneyedit.cpp
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


// ----------------------------------------------------------------------------
// KDE Includes

#include <knumvalidator.h>
#include <kglobal.h>
#include <klocale.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyedit.h"
#include "kmymoneycalculator.h"
#include "../mymoney/mymoneymoney.h"

kMyMoneyEdit::kMyMoneyEdit(QWidget *parent, const char *name )
 : KLineEdit(parent,name)
{
	// Yes, just a simple double validator !
	KFloatValidator *validator = new KFloatValidator(this);
	validator->setAcceptLocalizedNumbers(true);
	setValidator(validator);
	setAlignment(AlignRight | AlignVCenter);

  m_calculatorFrame = new QVBox(0,0,WType_Popup);

	m_calculatorFrame->setFrameStyle(QFrame::PopupPanel | QFrame::Raised);
  m_calculatorFrame->setLineWidth(3);
	
	m_calculator = new kMyMoneyCalculator(m_calculatorFrame);
  m_calculatorFrame->setFixedSize(m_calculator->width()+3, m_calculator->height()+3);
  m_calculatorFrame->hide();

  connect(this, SIGNAL(textChanged(const QString&)), this, SLOT(theTextChanged(const QString&)));
  connect(m_calculator, SIGNAL(signalResultAvailable()), this, SLOT(slotCalculatorResult()));
}

kMyMoneyEdit::~kMyMoneyEdit()
{
  delete m_calculatorFrame;
}

MyMoneyMoney kMyMoneyEdit::getMoneyValue(void)
{
  MyMoneyMoney money(text().toDouble());
  return money;
}

void kMyMoneyEdit::loadText(const QString& text)
{
  m_text = text;
  setText(text);
}

void kMyMoneyEdit::resetText(void)
{
  setText(m_text);
}

void kMyMoneyEdit::theTextChanged(const QString & theText)
{
  QString l_text = theText;
  int i = 0;
  QValidator::State state =  this->validator()->validate( l_text, i);
  if (state==QValidator::Invalid || state==QValidator::Intermediate && (!l_text.isEmpty() && (l_text!="-"))) {
    setText(previousText);
  } else
    previousText = l_text;
}

void kMyMoneyEdit::focusOutEvent(QFocusEvent *e)
{
  KLocale* locale = KGlobal::locale();
  // If text contains no 'monetaryDecimalSymbol' then add it
  // followed by the required number of 0s
  QString s(text());
  if (!s.isEmpty()) {
    if (!s.contains(locale->monetaryDecimalSymbol())) {
      s += locale->monetaryDecimalSymbol();
      for (int i=0; i < locale->fracDigits(); i++)
        s += "0";
      setText(s);
    }
  }
  if(text().toDouble() != m_text.toDouble())
    emit valueChanged(text());

  QLineEdit::focusOutEvent(e);
}

bool kMyMoneyEdit::eventFilter(QObject *o , QEvent *e )
{
  if(e->type() == QEvent::KeyPress) {
    QKeyEvent *k = static_cast<QKeyEvent *> (e);
    switch(k->key()) {
      case Qt::Key_Return:
      case Qt::Key_Enter:
        emit signalEnter();
        break;

      case Qt::Key_Escape:
        emit signalEsc();
        break;

      case Qt::Key_Tab:
        if(k->state() & Qt::ShiftButton)
          emit signalBackTab();
        else
          emit signalTab();
        break;

      case Qt::Key_Backtab:
        emit signalBackTab();
        break;

      case Qt::Key_Plus:
      case Qt::Key_Minus:
      case Qt::Key_Slash:
      case Qt::Key_Asterisk:
      case Qt::Key_Percent:
        m_calculator->setInitialValues(text(), k);

        QPoint p = mapToGlobal(QPoint(0,0));
        QRect r = m_calculator->geometry();
        r.moveTopLeft(p);

        m_calculatorFrame->setGeometry(r);
        m_calculatorFrame->show();
        break;
    }
  }
  return KLineEdit::eventFilter(o,e);
}

void kMyMoneyEdit::slotCalculatorResult()
{
  QString result;
  if(m_calculator != 0) {
    setText(m_calculator->result());
    m_calculatorFrame->hide();
  }
}