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
#include <locale.h>
#include "kmymoneyedit.h"

kMyMoneyEdit::kMyMoneyEdit(QWidget *parent, const char *name )
 : KLineEdit(parent,name)
{
	// Yes, just a simple double validator !
	KFloatValidator *validator = new KFloatValidator(this);
	validator->setAcceptLocalizedNumbers(true);
	setValidator(validator);
	setAlignment(AlignRight);
  connect(this, SIGNAL(textChanged(const QString&)), this, SLOT(theTextChanged(const QString&)));
}

kMyMoneyEdit::~kMyMoneyEdit()
{
}

MyMoneyMoney kMyMoneyEdit::getMoneyValue(void)
{
  // Truncate to frac_digits
  // This will all be user specified in the future.
  QString convString = QString::number(text().toFloat(), 'f', int(localeconv()->frac_digits));
  MyMoneyMoney money(convString.toFloat());
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
  // If text contains no .?? then make the text read ?.00
  QString s(text());
  if (!s.isEmpty()) {
    if (!s.contains('.')) {
      s += ".";
      for (int i=0; i<(int(localeconv()->frac_digits)); i++)
        s += "0";
      setText(s);
    }
  }
  QLineEdit::focusOutEvent(e);
}

bool kMyMoneyEdit::eventFilter(QObject *o , QEvent *e )
{
  if(e->type() == QEvent::KeyRelease)
  {
    QKeyEvent *k = (QKeyEvent *) e;
    if((k->key() == Qt::Key_Return) ||
       (k->key() == Qt::Key_Enter))
    {
      emit signalEnter();
      emit signalNextTransaction();
    }
  }
  return KLineEdit::eventFilter(o,e);
}
