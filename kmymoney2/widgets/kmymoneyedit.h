/***************************************************************************
                          kmymoneyedit.h
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes
    email                : Michael.Edwardes@students.dmu.ac.uk
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

#include <qwidget.h>
#include <qlineedit.h>
#include "../mymoney/mymoneymoney.h"

// This class replaces OE Hansens kdbMoneyEdit I used
// to use.  It has simpler interface and fixes some
// issues I had with the original widget.
class kMyMoneyEdit : public QLineEdit  {
   Q_OBJECT

private:
  QString previousText; // keep track of what has been typed

protected:
  void focusOutEvent(QFocusEvent *e);

protected slots:
  void theTextChanged(const QString & text);

public:
	kMyMoneyEdit(QWidget *parent=0, const char *name=0);
	~kMyMoneyEdit();
  MyMoneyMoney getMoneyValue(void);
};

#endif
