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

#include <knumvalidator.h>
#include <kglobal.h>
#include <klocale.h>
#include "kmymoneyedit.h"

kMyMoneyEdit::kMyMoneyEdit(QWidget *parent, const char *name )
 : KLineEdit(parent,name)
{
	// Yes, just a simple double validator !
	KFloatValidator *validator = new KFloatValidator(this);
	validator->setAcceptLocalizedNumbers(true);
	setValidator(validator);
	setAlignment(AlignRight | AlignVCenter);
  connect(this, SIGNAL(textChanged(const QString&)), this, SLOT(theTextChanged(const QString&)));
}

kMyMoneyEdit::~kMyMoneyEdit()
{
}

MyMoneyMoney kMyMoneyEdit::getMoneyValue(void)
{
#warning "FIXME:"
  MyMoneyMoney money((long)(text().toDouble()*100));
  return money;
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
  QLineEdit::focusOutEvent(e);
}

bool kMyMoneyEdit::eventFilter(QObject *o , QEvent *e )
{
  if(e->type() == QEvent::KeyRelease) {
    QKeyEvent *k = static_cast<QKeyEvent *> (e);
    if((k->key() == Qt::Key_Return) ||
       (k->key() == Qt::Key_Enter)) {
      emit signalEnter();
      emit signalNextTransaction();
    }

  } else if(e->type() == QEvent::KeyPress) {
    QKeyEvent *k = static_cast<QKeyEvent *> (e);
    if(k->key() == Qt::Key_Backtab ||
       (k->key() == Qt::Key_Tab &&
       (k->state() & Qt::ShiftButton)) ) {
      emit signalBackTab();

    } else if(k->key() == Qt::Key_Tab) {
      emit signalTab();
    }
  }
  return KLineEdit::eventFilter(o,e);
}
