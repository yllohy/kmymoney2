/***************************************************************************
                          kfindtransactiondlg.cpp
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
 
// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <kglobal.h>
#include <klocale.h>
#if QT_VERSION > 300
#include <kstandarddirs.h>
#else
#include <kstddirs.h>
#endif

#include <kpushbutton.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kfindtransactiondlg.h"
#include "../widgets/kmymoneyregistersearch.h"

KFindTransactionDlg::KFindTransactionDlg(QWidget *parent, const char *name)
 : KFindTransactionDlgDecl(parent, name, false)
{
  connect(m_searchButton, SIGNAL(clicked()), m_transactionView, SLOT(hide()));
  connect(m_resetButton, SIGNAL(clicked()), m_transactionView, SLOT(show()));
  connect(m_closeButton, SIGNAL(clicked()), this, SLOT(deleteLater()));
  
  // hide the transaction register and make sure the dialog is
  // displayed as small as can be. We make sure that the larger
  // version (with the transaction register) will also fit on the screen
  // by moving the dialog by (-45,-45).
  m_transactionView->hide();
  update();
  resize(minimumSizeHint());
  
  move(x()-45, y()-45);

  // readConfig();
}

KFindTransactionDlg::~KFindTransactionDlg()
{
  // writeConfig();
}


