/***************************************************************************
                          kimportverifydlg.h  -  description
                             -------------------
    begin                : Mon Jun 9 2003
    copyright            : (C) 2000-2003 by Michael Edwardes
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

#ifndef KIMPORTVERIFYDLG_H
#define KIMPORTVERIFYDLG_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qwidget.h>
#include <qcstring.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kimportverifydlgdecl.h"
#include "../mymoney/mymoneyaccount.h"

/**
  * @author Thomas Baumgart
  */

class KImportVerifyDlg : public KImportVerifyDlgDecl
{
  Q_OBJECT
  
public: 
  KImportVerifyDlg(const MyMoneyAccount& accountId, QWidget *parent=0, const char *name=0);
  ~KImportVerifyDlg();

protected slots:
  void slotOkClicked(void);
  
private:
  MyMoneyAccount m_account;
};

#endif
