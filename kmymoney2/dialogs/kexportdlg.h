/***************************************************************************
                          kexportdlg.h  -  description
                             -------------------
    begin                : Tue May 22 2001
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
#ifndef KEXPORTDLG_H
#define KEXPORTDLG_H

// ----------------------------------------------------------------------------
// QT Headers
#include <qstring.h>

// ----------------------------------------------------------------------------
// KDE Headers

// ----------------------------------------------------------------------------
// Project Headers
#include "kexportdlgdecl.h"
#include "../mymoney/mymoneyaccount.h"

/**
  * This class is used to export a specified account to the popular QIF format.
  * It relies upon the QIF file handling routines in MyMoneyAccount to do
  * the actual writing of QIF files.
  *
  * It uses the global KConfig object to read and write the application
  * settings.
  *
  * @see MyMoneyAccount
  *
  * @author Felix Rodriguez, Michael Edwardes 2000-2001
  * $Id: kexportdlg.h,v 1.5 2001/08/23 17:07:16 mte Exp $
  *
  * @short A class to export a specified account to the popular QIF format.
**/
class KExportDlg : public KExportDlgDecl  {
  Q_OBJECT

private:
  void readConfig(void);
  void writeConfig(void);

  QString m_qstringLastFormat;
  MyMoneyAccount *m_mymoneyaccount;

protected slots:
  /**
    * Called when the user clicked on the OK button
    *
  */
  void slotOkClicked();

  /**
    * Called when the progress bar needs updating.
    *
    * @param progress An integer representing the new progress.
  */
  void slotSetProgress(int progress);

  /** Called when the user needs to browse the filesystem for a QIF file */
  void slotBrowse();

  /** Test whether to enable the buttons */
  void slotFileTextChanged(const QString& text);

public:
  /**
    * Standard constructor
    *
    * @param account The account to export from.
   */
  KExportDlg(MyMoneyAccount *account, QWidget *parent);

  /** Standard destructor */
  ~KExportDlg();
};

#endif
