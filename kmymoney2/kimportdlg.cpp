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

#include "kimportdlg.h"
#include "qfiledialog.h"

KImportDlg::KImportDlg():KImportDlgDecl(0,0,TRUE){

 // connect( &btnBrowse, SIGNAL( clicked() ), this, SLOT( slotBrowse() ) );

}
KImportDlg::~KImportDlg(){
}
/** No descriptions */
void KImportDlg::slotBrowse(){

	QString s(QFileDialog::getOpenFileName());

	txtFileImport->setText(s);
		
}
