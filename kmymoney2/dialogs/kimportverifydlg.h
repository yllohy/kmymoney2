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

  /**
    * This method is used to register a callback function that
    * can be used to by this dialog to display progress information.
    *
    * @param callback pointer to a callback function that takes three
    *                 parameters. See KMyMoney2App::progressCallback
    *                 for a description of the parameters.
    */
  void setProgressCallback(void(*callback)(int, int, const QString&));

private:
  /**
    * This method is used to update the progress information. It
    * checks if an appropriate function is known and calls it.
    *
    * For a parameter description see KMyMoneyView::progressCallback().
    */
  void signalProgress(int current, int total, const QString& = QString());

public slots:
  int exec(void);
    
protected slots:
  void slotOkClicked(void);
  void slotCancelClicked(void);
  void slotShowIntroduction(void) const;
    
private:
  MyMoneyAccount m_account;
  void (*m_progressCallback)(int, int, const QString&);

};

#endif
