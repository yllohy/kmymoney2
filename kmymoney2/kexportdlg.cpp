/***************************************************************************
                          kexportdlg.cpp  -  description
                             -------------------
    begin                : Tue May 22 2001
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
#include <kmessagebox.h>
#include "kexportdlg.h"
#include <qlineedit.h>
#include <qcheckbox.h>
#include "widgets/kmymoneydateinput.h"
#include <kfiledialog.h>

KExportDlg::KExportDlg():KExportDlgDecl(0,0,TRUE)
{
  readConfig();

  connect( btnBrowse, SIGNAL( clicked() ), this, SLOT( slotBrowse() ) );
  connect(buttonOk, SIGNAL(clicked()), this, SLOT(slotOkClicked()));
}
KExportDlg::~KExportDlg()
{
  writeConfig();
}
/** No descriptions */
void KExportDlg::slotBrowse(){

	QString s(KFileDialog::getSaveFileName(QString::null,"*.QIF"));
  txtFileExport->setText(s);
}

void KExportDlg::slotOkClicked()
{
  if (txtFileExport->text().isEmpty()) {
    KMessageBox::information(this, "Please enter the path to the QIF file", "Export");
    txtFileExport->setFocus();
    return;
  }
  accept();
}

void KExportDlg::readConfig(void)
{
  KConfig *config = KGlobal::config();
  config->setGroup("Last Use Settings");
  txtFileExport->setText(config->readEntry("KExportDlg_LastFile"));
  cbxAccount->setChecked(config->readBoolEntry("KExportDlg_AccountOpt", true));
  cbxCategories->setChecked(config->readBoolEntry("KExportDlg_CatOpt", true));
  dateStartDate->setDate(config->readDateTimeEntry("KExportDlg_StartDate").date());
  dateEndDate->setDate(config->readDateTimeEntry("KExportDlg_EndDate").date());
}

void KExportDlg::writeConfig(void)
{
  KConfig *config = KGlobal::config();
  config->setGroup("Last Use Settings");
  config->writeEntry("KExportDlg_LastFile", txtFileExport->text());
  config->writeEntry("KExportDlg_AccountOpt", cbxAccount->isChecked());
  config->writeEntry("KExportDlg_CatOpt", cbxCategories->isChecked());
  config->writeEntry("KExportDlg_StartDate", QDateTime(dateStartDate->getQDate()));
  config->writeEntry("KExportDlg_EndDate", QDateTime(dateEndDate->getQDate()));

  config->sync();
}
