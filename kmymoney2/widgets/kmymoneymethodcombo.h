/***************************************************************************
                          kmymoneymethodcombo.h  -  description
                             -------------------
    begin                : Sat May 5 2001
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

#ifndef KMYMONEYMETHODCOMBO_H
#define KMYMONEYMETHODCOMBO_H

#include <kcombobox.h>

/**
  *@author Michael Edwardes
  */

class kMyMoneyMethodCombo : public KComboBox  {
	Q_OBJECT
public: 
	kMyMoneyMethodCombo();
	~kMyMoneyMethodCombo();
  /** No descriptions */
  virtual bool eventFilter(QObject *, QEvent *);
signals: // Signals
  /** No descriptions */
  void signalFocusOut();
};

#endif
