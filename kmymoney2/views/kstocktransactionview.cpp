/***************************************************************************
                          kstocktransactionview.cpp  -  description
                             -------------------
    begin                : Mon Jan 28 2002
    copyright            : (C) 2000-2002 by Michael Edwardes
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
#include <kglobal.h>
#include <kconfig.h>
#include <klocale.h>
#include <kiconloader.h>

#include <qpushbutton.h>
#include <kcombobox.h>
#include <qtabwidget.h>
#include <qtable.h>
#include <qinputdialog.h>

//#include "../dialogs/knewcategorydlg.h"
#include "../dialogs/ksplittransactiondlg.h"
#include <kmessagebox.h>

#if QT_VERSION > 300
#include <qcursor.h>
#endif

#include "kstocktransactionview.h"

KStockTransactionView::KStockTransactionView(QWidget *parent, const char *name)
 : kStockTransactionViewDecl(parent, name)
{
}

KStockTransactionView::~KStockTransactionView()
{
}
