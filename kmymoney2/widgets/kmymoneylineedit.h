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

#include <klineedit.h>

/**
  *@author Michael Edwardes
  */

class kMyMoneyLineEdit : public KLineEdit  {
	Q_OBJECT
public: 
	kMyMoneyLineEdit();
	~kMyMoneyLineEdit();
  /** No descriptions */
  virtual bool eventFilter(QObject * , QEvent * );
signals: // Signals
  /** No descriptions */
  void signalEnter();
};

#endif
