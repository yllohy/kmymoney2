/***************************************************************************
                          kimportdlg.cpp  -  description
                             -------------------
    begin                : Wed May 16 2001
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
#include <qpushbutton.h>
#include <kmessagebox.h>
#include "kimportdlg.h"
#include <qlineedit.h>
#include <kfiledialog.h>

KImportDlg::KImportDlg():KImportDlgDecl(0,0,TRUE)
{
  readConfig();

  connect( btnBrowse, SIGNAL( clicked() ), this, SLOT( slotBrowse() ) );
  connect(buttonOk, SIGNAL(clicked()), this, SLOT(slotOkClicked()));
}

KImportDlg::~KImportDlg()
{
  writeConfig();
}
/** No descriptions */
void KImportDlg::slotBrowse()
{
	//KFileDialog *browseFile = new KFileDialog();
	QString s(KFileDialog::getOpenFileName(QString::null,"*.QIF"));
  //delete browseFile;
	txtFileImport->setText(s);

}

void KImportDlg::slotOkClicked()
{
  if (txtFileImport->text().isEmpty()) {
    KMessageBox::information(this, "Please enter the path to the QIF file", "Import");
    txtFileImport->setFocus();
    return;
  }
  accept();
}

void KImportDlg::readConfig(void)
{
  KConfig *config = KGlobal::config();
  config->setGroup("Last Use Settings");
  txtFileImport->setText(config->readEntry("KImportDlg_LastFile"));
}

void KImportDlg::writeConfig(void)
{
  KConfig *config = KGlobal::config();
  config->setGroup("Last Use Settings");
  config->writeEntry("KImportDlg_LastFile", txtFileImport->text());
  config->sync();
}
