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
#include <kfiledialog.h>

KExportDlg::KExportDlg():KExportDlgDecl(0,0,TRUE){
  connect(buttonOk, SIGNAL(clicked()), this, SLOT(slotOkClicked()));
}
KExportDlg::~KExportDlg(){
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
