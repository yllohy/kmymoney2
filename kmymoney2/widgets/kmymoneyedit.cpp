/***************************************************************************
                          kmymoneyedit.cpp
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes,
                               2004 by Thomas Baumgart
    email                : mte@users.sourceforge.net
                           ipwizard@users.sourceforge.net
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

#include <qapplication.h>
#include <qdesktopwidget.h>
#include <qwidget.h>
#include <qvbox.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>
#include <klineedit.h>
#include <kiconloader.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyedit.h"
#include "kmymoneycalculator.h"
#include "../mymoney/mymoneymoney.h"

kMyMoneyMoneyValidator::kMyMoneyMoneyValidator(QObject * parent, const char * name) :
  QDoubleValidator(parent, name)
{
}

kMyMoneyMoneyValidator::kMyMoneyMoneyValidator( double bottom, double top, int decimals,
                                                QObject * parent, const char * name ) :
  QDoubleValidator(bottom, top, decimals, parent, name)
{
}

/*
 * The code of the following function is taken from kdeui/knumvalidator.cpp
 * and adjusted to always use the monetary symbols defined in the KDE control center
 */
QValidator::State kMyMoneyMoneyValidator::validate( QString & input, int & _p ) const
{
  QString s = input;
  KLocale * l = KGlobal::locale();
  // ok, we have to re-format the number to have:
  // 1. decimalSymbol == '.'
  // 2. negativeSign  == '-'
  // 3. positiveSign  == <empty>
  // 4. thousandsSeparator() == <empty> (we don't check that there
  //    are exactly three decimals between each separator):
  QString d = l->monetaryDecimalSymbol(),
          n = l->negativeSign(),
          p = l->positiveSign(),
          t = l->monetaryThousandsSeparator();
  // first, delete p's and t's:
  if ( !p.isEmpty() )
    for ( int idx = s.find( p ) ; idx >= 0 ; idx = s.find( p, idx ) )
      s.remove( idx, p.length() );


  if ( !t.isEmpty() )
    for ( int idx = s.find( t ) ; idx >= 0 ; idx = s.find( t, idx ) )
      s.remove( idx, t.length() );

  // then, replace the d's and n's
  if ( ( !n.isEmpty() && n.find('.') != -1 ) ||
       ( !d.isEmpty() && d.find('-') != -1 ) ) {
    // make sure we don't replace something twice:
    kdWarning() << "KDoubleValidator: decimal symbol contains '-' or "
                   "negative sign contains '.' -> improve algorithm" << endl;
    return Invalid;
  }

  if ( !d.isEmpty() && d != "." )
    for ( int idx = s.find( d ) ; idx >= 0 ; idx = s.find( d, idx + 1 ) )
      s.replace( idx, d.length(), ".");

  if ( !n.isEmpty() && n != "-" )
    for ( int idx = s.find( n ) ; idx >= 0 ; idx = s.find( n, idx + 1 ) )
      s.replace( idx, n.length(), "-" );

  return QDoubleValidator::validate( s, _p );
}

kMyMoneyEdit::kMyMoneyEdit(QWidget *parent, const char *name )
 : QHBox(parent, name)
{
  m_prec = KGlobal::locale()->fracDigits();
  m_edit = new KLineEdit(this);
  m_edit->installEventFilter(this);
  setFocusProxy(m_edit);

  // Yes, just a simple double validator !
  kMyMoneyMoneyValidator *validator = new kMyMoneyMoneyValidator(this);
  m_edit->setValidator(validator);
  m_edit->setAlignment(AlignRight | AlignVCenter);

  m_calculatorFrame = new QVBox(0,0,WType_Popup);

  m_calculatorFrame->setFrameStyle(QFrame::PopupPanel | QFrame::Raised);
  m_calculatorFrame->setLineWidth(3);

  m_calculator = new kMyMoneyCalculator(m_calculatorFrame);
  m_calculatorFrame->setFixedSize(m_calculator->width()+3, m_calculator->height()+3);
  m_calculatorFrame->hide();

  m_calcButton = new KPushButton(QIconSet(QPixmap(KGlobal::iconLoader()->iconPath("kcalc", -KIcon::SizeSmall))), QString(""), this);

  connect(m_edit, SIGNAL(textChanged(const QString&)), this, SLOT(theTextChanged(const QString&)));
  connect(m_calculator, SIGNAL(signalResultAvailable()), this, SLOT(slotCalculatorResult()));
  connect(m_calcButton, SIGNAL(clicked()), this, SLOT(slotCalculatorOpen()));
}

kMyMoneyEdit::~kMyMoneyEdit()
{
  delete m_calculatorFrame;
}

const bool kMyMoneyEdit::isValid(void) const
{
  return !(m_edit->text().isEmpty());
}

MyMoneyMoney kMyMoneyEdit::getMoneyValue(void)
{
  ensureFractionalPart();
  MyMoneyMoney money(m_edit->text());
  return money;
}

void kMyMoneyEdit::loadText(const QString& txt)
{
  m_edit->setText(txt);
  if(isEnabled())
    ensureFractionalPart();
  m_text = m_edit->text();
}

void kMyMoneyEdit::resetText(void)
{
  m_edit->setText(m_text);
}

void kMyMoneyEdit::theTextChanged(const QString & theText)
{
  KLocale * l = KGlobal::locale();
  QString d = l->monetaryDecimalSymbol();
  QString l_text = theText;
  int i = 0;
  if(isEnabled()) {
    QValidator::State state =  m_edit->validator()->validate( l_text, i);
    if(state == QValidator::Intermediate) {
      if(l_text.length() == 1) {
        if(l_text != d && l_text != "-")
          state = QValidator::Invalid;
      }
    }
    if (state==QValidator::Invalid)
      m_edit->setText(previousText);
    else {
      previousText = l_text;
      emit textChanged(m_edit->text());
    }
  }
}

void kMyMoneyEdit::ensureFractionalPart(void)
{
  KLocale* locale = KGlobal::locale();
  // If text contains no 'monetaryDecimalSymbol' then add it
  // followed by the required number of 0s
  QString s(m_edit->text());
  if (!s.isEmpty()) {
    if (!s.contains(locale->monetaryDecimalSymbol())) {
      s += locale->monetaryDecimalSymbol();
      for (int i=0; i < m_prec; i++)
        s += "0";
      m_edit->setText(s);
    }
  }
}

bool kMyMoneyEdit::eventFilter(QObject * /* o */ , QEvent *e )
{
  bool rc = false;

  // we want to catch some keys that are usually handled by
  // the base class (e.g. '+', '-', etc.)
  if(e->type() == QEvent::KeyPress) {
    QKeyEvent *k = static_cast<QKeyEvent *> (e);

    rc = true;
    switch(k->key()) {
      default:
        rc = false;
        break;

      case Qt::Key_Return:
      case Qt::Key_Enter:
        emit signalEnter();
        break;

      case Qt::Key_Escape:
        emit signalEsc();
        break;

      case Qt::Key_Tab:
        rc = false;         // we signal, but we also use the standard behaviour
        if(k->state() & Qt::ShiftButton)
          emit signalBackTab();
        else
          emit signalTab();
        break;

      case Qt::Key_Backtab:
        rc = false;         // we signal, but we also use the standard behaviour
        emit signalBackTab();
        break;

      case Qt::Key_Plus:
      case Qt::Key_Minus:
        if(m_edit->hasSelectedText()) {
          m_edit->cut();
        }
        if(m_edit->text().length() == 0) {
          rc = false;
          break;
        }
        // in case of '-' we do not enter the calculator when
        // the current position is the beginning and there is
        // no '-' sign at the first position.
        if(k->key() == Qt::Key_Minus) {
          if(m_edit->cursorPosition() == 0 && m_edit->text()[0] != '-') {
            rc = false;
            break;
          }
        }
        // otherwise, tricky fall through here!

      case Qt::Key_Slash:
      case Qt::Key_Asterisk:
      case Qt::Key_Percent:
        if(m_edit->hasSelectedText()) {
          // remove the selected text
          m_edit->cut();
        }
        calculatorOpen(k);
        break;
    }

  } else if(e->type() == QEvent::FocusOut) {
    ensureFractionalPart();

    if(MyMoneyMoney(m_edit->text()) != MyMoneyMoney(m_text)) {
      emit valueChanged(m_edit->text());
      m_text = m_edit->text();
    }
  }
  return rc;
}

void kMyMoneyEdit::slotCalculatorOpen(void)
{
  calculatorOpen(0);
}

void kMyMoneyEdit::calculatorOpen(QKeyEvent* k)
{
  m_calculator->setInitialValues(m_edit->text(), k);

  int h = m_calculatorFrame->height();

  // usually, the calculator widget is shown underneath the MoneyEdit widget
  // if it does not fit on the screen, we show it above this widget

  QPoint p = mapToGlobal(QPoint(0,0));
  if(p.y() + h > QApplication::desktop()->height()) {
    p.setY(p.y() - h);
  } else {
    p.setY(p.y() + height());
  }
  QRect r = m_calculator->geometry();
  r.moveTopLeft(p);
  m_calculatorFrame->setGeometry(r);
  m_calculatorFrame->show();
}

void kMyMoneyEdit::slotCalculatorResult(void)
{
  QString result;
  if(m_calculator != 0) {
    m_calculatorFrame->hide();
    m_edit->setText(m_calculator->result());
    ensureFractionalPart();
    emit valueChanged(m_edit->text());
    m_text = m_edit->text();
  }
}

QWidget* kMyMoneyEdit::focusWidget(void) const
{
  QWidget* w = m_edit;
  while(w->focusProxy())
    w = w->focusProxy();
  return w;
}
