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

// ----------------------------------------------------------------------------
// QT Headers
#include <qstring.h>

// ----------------------------------------------------------------------------
// KDE Headers
#include <kurl.h>

// ----------------------------------------------------------------------------
// Project Headers
#include "kimportdlgdecl.h"
#include "../mymoney/mymoneyaccount.h"

/**
  * This class is used to import a qif file to an account.
  * It relies upon the QIF file handling routines in MyMoneyAccount to do
  * the actual writing of QIF files.
  *
  * It uses the global KConfig object to read and write the application
  * settings.
  *
  * @see MyMoneyAccount
  *
  * @author Felix Rodriguez, Michael Edwardes 2000-2001
  * $Id: kimportdlg.h,v 1.4 2001/08/23 17:07:16 mte Exp $
  *
  * @short A class to import a qif file to an account.
**/
class KImportDlg : public KImportDlgDecl  {
  Q_OBJECT

private:
  void readConfig(void);
  void writeConfig(void);

  QString m_qstringLastFormat;
  MyMoneyAccount *m_mymoneyaccount;

  bool fileExists(KURL url);

protected slots:
  /** Performs the import process */
  void slotOkClicked();

  /** Called to let the user browse for a QIF file to import from. */
  void slotBrowse();

  /**
    * Called when the progress bar needs updating.
    *
    * @param progress An integer representing the new progress.
  */
  void slotSetProgress(int progress);

  /** Test whether to enable the buttons */
  void slotFileTextChanged(const QString& text);

public:
  /**
    * Standard constructor
    *
    * @param account The account to import to.
  */
  KImportDlg(MyMoneyAccount *account, QWidget *parent);

  /** Standard destructor */
  ~KImportDlg();
};

#endif
