/***************************************************************************
                          kgncimportoptions.cpp
                             -------------------
    copyright            : (C) 2005 by Ace Jones
    author               : Tony Bloomfield
    email                : tonybloom@users.sourceforge.net
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
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <qapplication.h>

// ----------------------------------------------------------------------------
// KDE Includes
#include <kapplication.h>
#include <kurlrequester.h>
#include <ktextbrowser.h>
#include <klocale.h>

// ----------------------------------------------------------------------------
// Project Includes
#include "kgncimportoptionsdlg.h"

KGncImportOptionsDlg::KGncImportOptionsDlg(QWidget *parent, const char *name)
 : KGncImportOptionsDlgDecl(parent, name)
{
  buttonInvestGroup->setRadioButtonExclusive (true);
  buttonInvestGroup->setButton (0);
  checkSchedules->setChecked (false);
  checkDebugGeneral->setChecked (false);
  checkDebugXML->setChecked (false);
  checkAnonymize->setChecked (false);
}

KGncImportOptionsDlg::~KGncImportOptionsDlg()
{
}


void KGncImportOptionsDlg::slotHelp(void)
{
  kapp->invokeHelp ("details.impexp.gncoptions");
}

#include "kgncimportoptionsdlg.moc"
