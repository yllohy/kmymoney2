/***************************************************************************
                          kcsvprogressdlg.h  -  description
                             -------------------
    begin                : Sun Jul 29 2001
    copyright            : (C) 2000-2001 by Michael Edwardes
    email                : mte@users.sourceforge.net
                             Javier Campos Morales <javi_c@users.sourceforge.net>
                             Felix Rodriguez <frodriguez@users.sourceforge.net>
                             John C <thetacoturtle@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef KCSVPROGRESSDLG_H
#define KCSVPROGRESSDLG_H

// ----------------------------------------------------------------------------
// QT Includes
#include <qwidget.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes
#include "../mymoney/mymoneyaccount.h"
#include "kcsvprogressdlgdecl.h"

/**
  * This class is used to show the progress of an import or export of type
  * csv.  It could eventually become a base class for other import/export
  * types which would reimplement performImport/performExport.
  *
  * @author Michael Edwardes 2000-2001
  * $Id: kcsvprogressdlg.h,v 1.1 2001/07/29 20:59:18 mte Exp $
  *
  * @short A class to show the progress of a CSV import or export.
**/
class KCsvProgressDlg : public KCsvProgressDlgDecl  {
   Q_OBJECT
private:
  MyMoneyAccount *m_mymoneyaccount;
  int m_nType;
  bool m_bStopFlag;
  bool m_bProcessStarted;

protected:
  void performExport(void);
  void performImport(void);
  void readConfig(void);
  void writeConfig(void);

protected slots:
  void slotCancelClicked();
  void slotBrowseClicked();
  void slotRunClicked();
  void slotFileTextChanged(const QString& text);

public:
  KCsvProgressDlg(int type, MyMoneyAccount *account, QWidget *parent=0, const char *name=0);
  ~KCsvProgressDlg();
};

#endif
