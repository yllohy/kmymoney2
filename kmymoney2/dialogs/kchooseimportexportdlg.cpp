/***************************************************************************
                          kchooseimportexportdlg.cpp  -  description
                             -------------------
    begin                : Thu Jul 12 2001
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
#include <qlabel.h>
#include <qcombobox.h>
#include <qpushbutton.h>

#include <kglobal.h>
#include <kconfig.h>
#include "kchooseimportexportdlg.h"

KChooseImportExportDlg::KChooseImportExportDlg(int type, QWidget *parent, const char *name )
 : KChooseImportExportDlgDecl(parent,name, true)
{
  if (type==0) { // QIF
    topLabel->setText("Please choose the type of import you wish to perform.   A simple explanation\n"
        "of the import type is available at the bottom of the screen and is updated when\n"
        "you select an item from the choice box."
        "\n\nOnce you have chosen an import type please press the OK button." );
    promptLabel->setText("Choose import type:");
    setCaption("Choose Import Type Dialog");
  } else {
    topLabel->setText("Please choose the type of export you wish to perform.   A simple explanation\n"
        "of the export type is available at the bottom of the screen and is updated when\n"
        "you select an item from the choice box."
        "\n\nOnce you have chosen an export type please press the OK button." );
    promptLabel->setText("Choose export type:");
    setCaption("Choose Export Type Dialog");
  }

  readConfig();
  slotTypeActivated(m_lastType);

  connect(typeCombo, SIGNAL(activated(const QString&)), this, SLOT(slotTypeActivated(const QString&)));
  connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
  connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

KChooseImportExportDlg::~KChooseImportExportDlg()
{
  writeConfig();
}

void KChooseImportExportDlg::slotTypeActivated(const QString& text)
{
  if (text=="QIF") {
    descriptionLabel->setText("QIF files are created by the popular accounting program Quicken.\n"
      "Another dialog will appear, if you choose this type, asking for further\n"
      "information relevant to the Quicken format.");
  } else {
    descriptionLabel->setText("The CSV type uses a comma delimeted text file that can be used by\n"
      "most popular spreadsheet programs available for Linux and other operating\n"
      "systems.");
  }
}

QString KChooseImportExportDlg::importExportType(void)
{
  return typeCombo->currentText();
}

void KChooseImportExportDlg::readConfig(void)
{
  KConfig *config = KGlobal::config();
  config->setGroup("Last Use Settings");
  m_lastType = config->readEntry("KChooseImportExportDlg_LastType");
}

void KChooseImportExportDlg::writeConfig(void)
{
  KConfig *config = KGlobal::config();
  config->setGroup("Last Use Settings");
  config->writeEntry("KChooseImportExportDlg_LastType", typeCombo->currentText());
  config->sync();
}
