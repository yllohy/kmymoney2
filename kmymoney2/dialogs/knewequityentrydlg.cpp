/***************************************************************************
                          knewequityentrydlg.cpp  -  description
                             -------------------
    begin                : Tue Jan 29 2002
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
#include <klocale.h>
#include <kstddirs.h>
#include <qpixmap.h>

// ----------------------------------------------------------------------------
// QT Includes
#include <qfile.h>
#include <qtextstream.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qprogressbar.h>
#include <qlineedit.h>
#include <qgroupbox.h>

// ----------------------------------------------------------------------------
// KDE Includes
#include <kfiledialog.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kconfig.h>
#include <knuminput.h>

#include "knewequityentrydlg.h"

KNewEquityEntryDlg::KNewEquityEntryDlg(QWidget *parent, const char *name)
	: kNewEquityEntryDecl(parent, name, TRUE)
{
	connect(btnOK, SIGNAL(clicked()), this, SLOT(onOKClicked()));
	connect(btnCancel, SIGNAL(clicked()), this, SLOT(onCancelClicked()));
}

KNewEquityEntryDlg::~KNewEquityEntryDlg()
{
}
/** No descriptions */
void KNewEquityEntryDlg::onOKClicked()
{
	accept();
}

void KNewEquityEntryDlg::onCancelClicked()
{
	reject();
}
/** No descriptions */
double KNewEquityEntryDlg::getStockPrice()
{
  return dblCurrentPrice->text().toDouble();
}
