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

#include <qstring.h>
#include <klineedit.h>

/**
  *@author Thomas Baumgart
  */

class kMyMoneyPayee : public KLineEdit {
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
    */
  void resetText(void);

public slots:
  void setText(const QString& text);

signals:
  void newPayee(const QString& payee);

protected:
  void focusOutEvent(QFocusEvent *ev);

private:
  /**
    * This member keeps the initial value. It is used during
    * resetText() to set the widgets text back to this initial value
    */
  QString m_text;

  QMap<QString, QCString>  m_payeeConversionList;
};

#endif
