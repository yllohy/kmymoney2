/***************************************************************************
                          kmymoneypayee.h  -  description
                             -------------------
    begin                : Sat Aug 10 2002
    copyright            : (C) 2000-2002 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KMYMONEYPAYEE_H
#define KMYMONEYPAYEE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qstring.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klineedit.h>

// ----------------------------------------------------------------------------
// Project Includes


/**
  * @author Thomas Baumgart
  */
class kMyMoneyPayee : public KLineEdit
{
  Q_OBJECT
public:
  kMyMoneyPayee(QWidget *parent=0, const char *name=0);
  ~kMyMoneyPayee();

  /**
    * This method is used to load the combo box from the
    * MyMoneyFile object
    */
  void loadList(void);

  /**
    * This method is used to set the value of the widget back to
    * the one passed using loadText().
    */
  void resetText(void);

  /**
    * This method is used to turn on/off the hint display
    */
  void setHint(const QString& hint) { m_hint = hint; };

public slots:
  /**
    * This slot is used to load the local text copy and the widget with
    * the value passed as argument. One can use resetText() to load the
    * widget again with the value passed here at a later time.
    *
    * @param text reference to the text to be loaded and remembered
    */
  void loadText(const QString& text);

signals:
  /**
    * This signal is emitted, when a new payee/receiver name has been
    * entered by the user and this name is not known by the MyMoneyFile
    * object.
    */
  void newPayee(const QString& payee);

  /**
    * This signal is emitted when the user entered a different name. This
    * signal is emitted after the newPayee(const QString& payee) signal and
    * the new payee is possibly available in the MyMoneyFile object.
    */
  void payeeChanged(const QString& payee);

protected:
  void focusOutEvent(QFocusEvent *ev);
  virtual void keyPressEvent( QKeyEvent * );

  /** reimplemented */
  void drawContents( QPainter *);

private:
  /**
    * This member keeps the initial value. It is used during
    * resetText() to set the widgets text back to this initial value
    */
  QString m_text;

  /**
    * This member tells what to display as hint as long as the field is empty
    */
  QString m_hint;
};

#endif
