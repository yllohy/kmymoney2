/***************************************************************************
                          keditequityentrydlg.h  -  description
                             -------------------
    begin                : Sat Mar 6 2004
    copyright            : (C) 2000-2004 by Michael Edwardes
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

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#ifndef KEDITEQUITYENTRYDLG_H
#define KEDITEQUITYENTRYDLG_H

#include <qdialog.h>
#include <klocale.h>

#include "keditequityentrydecl.h"

/**
  *@author Kevin Tambascio
  */

class KEditEquityEntryDlg : public kEditEquityEntryDecl
{
  Q_OBJECT
public: 
	KEditEquityEntryDlg(QWidget *parent = NULL, const char *name = NULL);
	~KEditEquityEntryDlg();
                          
protected slots:
  void onOKClicked();
	void onCancelClicked();	
};

#endif
