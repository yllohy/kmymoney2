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
  QString helpstring = i18n(
  "<h1>GnuCash Import Options</h1>"
  "<h2>Investment Options</h2>"
    "<p>In KMyMoney, all accounts representing investments (stocks, shares, bonds, etc.) must "
    "have an associated investment (portfolio) account.</p>"
    "<p>GnuCash does not enforce this, so we cannot automate this association. If you have investments, "
    "please select one of the following options.</p>"
     "<p>o create a separate investment account for each stock with the same name as the stock</p>"
     "<p>o create a single investment account to hold all stocks - you will be asked for a name</p>"
     "<p>o create multiple investment accounts - you will be asked for a name for each stock</p>"
  "<h2>Scheduled Transactions</h2>"
    "<p>Due to differences in implementation, it is not always possible to import scheduled "
    "transactions correctly. Though best efforts are made, it may be that some "
    "imported transactions cause problems within KMyMoney.</p>"
    "<p>An attempt will be made to identify potential problem transactions, "
    "and setting this option will cause them to be dropped from the file. "
    "A report of which transactions were dropped, and why, will be produced on screen.</p>"
  "<h2>Debug Options</h2>"
  "<p>These should only be used under developer direction (or at your own risk!).</p>" );

  QDialog dlg;
  QVBoxLayout layout( &dlg, 11, 6, "Layout17");
  KTextBrowser te(&dlg,"Help");
  layout.addWidget(&te);
  te.setReadOnly(true);
  te.setTextFormat(Qt::RichText);
  te.setText(helpstring);
  dlg.setCaption(i18n("GnuCash Import Options Help"));
  unsigned width = QApplication::desktop()->width();
  unsigned height = QApplication::desktop()->height();
  te.setMinimumSize(width/2,height/2);
  layout.setResizeMode(QLayout::Minimum);
  dlg.exec();
}

#include "kgncimportoptionsdlg.moc"
