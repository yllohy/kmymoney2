/***************************************************************************
                          kstocktransactionview.h  -  description
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

#ifndef KSTOCKTRANSACTIONVIEW_H
#define KSTOCKTRANSACTIONVIEW_H

#include <qwidget.h>
#include <qlabel.h>
#include <qlayout.h>

#include <kpushbutton.h>
#include <klineedit.h>
#include <kcombobox.h>
#include <klocale.h>
#include <klistview.h>
#include <kpopupmenu.h>

#include "../mymoney/mymoneyfile.h"
#include "../mymoney/mymoneyaccount.h"
#include "../mymoney/mymoneytransaction.h"

#include "../widgets/kmymoneydateinput.h"
#include "../widgets/kmymoneylineedit.h"
#include "../widgets/kmymoneycombo.h"
#include "../widgets/kmymoneyedit.h"

#include "kstocktransactionviewdecl.h"
#include "../widgets/kmymoneyhlayout.h"

/**
  *@author Kevin Tambascio
  */

class KStockTransactionView : public kStockTransactionViewDecl {
	Q_OBJECT
public: 
	KStockTransactionView(QWidget *parent = NULL, const char *name = NULL);
	~KStockTransactionView();
};

#endif
