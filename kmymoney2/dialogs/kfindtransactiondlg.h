/***************************************************************************
                          kfindtransactiondlg.h
                             -------------------
    copyright            : (C) 2003 by Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KFINDTRANSACTIONDLG_H
#define KFINDTRANSACTIONDLG_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#if 0
#include <qwidget.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qpushbutton.h>

#include <klocale.h>
#include <qdialog.h>

#include "../widgets/kmymoneyedit.h"
#include "../widgets/kmymoneylineedit.h"
#include "../widgets/kmymoneydateinput.h"
#include "../mymoney/mymoneyfile.h"
#endif

#include "kfindtransactiondlgdecl.h"

/**
  * @author Thomas Baumgart
  */

class KFindTransactionDlg : public KFindTransactionDlgDecl  {
   Q_OBJECT
public:
  KFindTransactionDlg(QWidget *parent=0, const char *name=0);
  ~KFindTransactionDlg();

protected slots:
  
signals:

private:

private:
  void readConfig(void);
  void writeConfig(void);

};

#endif
