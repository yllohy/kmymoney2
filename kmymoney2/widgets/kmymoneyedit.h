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

#include <qwidget.h>
#include <klineedit.h>
#include "../mymoney/mymoneymoney.h"

// This class replaces OE Hansens kdbMoneyEdit I used
// to use.  It has simpler interface and fixes some
// issues I had with the original widget.
class kMyMoneyEdit : public KLineEdit  {
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

  virtual bool eventFilter(QObject * , QEvent * );

signals: // Signals
  /** No descriptions */
  void signalEnter();
  /** No descriptions */
  void signalNextTransaction();
  /** signal is sent, when the tab key is pressed */
  void signalTab();
  /** signal is sent, when the Back-tab (Shift-Tab) key is pressed */
  void signalBackTab();

};

#endif
