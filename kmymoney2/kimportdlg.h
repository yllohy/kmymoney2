/***************************************************************************
                          kimportdlg.h  -  description
                             -------------------
    begin                : Wed May 16 2001
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

#ifndef KIMPORTDLG_H
#define KIMPORTDLG_H

#include "kimportdlgdecl.h"

/**
  *@author Michael Edwardes
  */

class KImportDlg : public KImportDlgDecl  {
	Q_OBJECT
public: 
	KImportDlg();
	~KImportDlg();
public slots: // Public slots
  /** No descriptions */
  void slotBrowse();
};

#endif
