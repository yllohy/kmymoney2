/***************************************************************************
                          kmymoneylineedit.h  -  description
                             -------------------
    begin                : Wed May 9 2001
    copyright            : (C) 2001 by Michael Edwardes
    email                : mte@users.sourceforge.net
                             Javier Campos Morales <javi_c@ctv.es>
                             Felix Rodriguez <frodriguez@mail.wesleyan.edu>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KMYMONEYLINEEDIT_H
#define KMYMONEYLINEEDIT_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <klineedit.h>

// ----------------------------------------------------------------------------
// Project Includes

/**
  *@author Michael Edwardes
  */

class kMyMoneyLineEdit : public KLineEdit  {
	Q_OBJECT
public: 
	kMyMoneyLineEdit(QWidget *w);
	~kMyMoneyLineEdit();
  /** No descriptions */
  virtual bool eventFilter(QObject * , QEvent * );

  /**
    * This method is used to set the value of the widget back to
    * the one passed using loadText().
    */
  void resetText(void);

public slots:
  void loadText(const QString& text);

signals: // Signals
  void lineChanged(const QString& str);

  /** No descriptions */
  void signalEnter();
  /** No descriptions */
  void signalNextTransaction();
  /** signal is sent, when the tab key is pressed */
  void signalTab();
  /** signal is sent, when the Back-tab (Shift-Tab) key is pressed */
  void signalBackTab();

protected:
  void focusOutEvent(QFocusEvent *ev);

private:
  /**
    * This member keeps the initial value. It is used during
    * resetText() to set the widgets text back to this initial value
    */
  QString m_text;
};

#endif
