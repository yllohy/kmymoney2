/***************************************************************************
                          kbackupdialog.cpp  -  description
                             -------------------
    begin                : Mon Jun 4 2001
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
#include <kglobal.h>
#include <kconfig.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <kfiledialog.h>
#include "kbackupdlg.h"

KBackupDlg::KBackupDlg( QWidget* parent,  const char* name/*, bool modal*/)
  : kbackupdlgdecl( parent,  name , true)
{
  KConfig *config = KGlobal::config();
  config->setGroup("Last Use Settings");
  mountCheckBox->setChecked(config->readBoolEntry("KBackupDlg_mountDevice", false));

  connect(chooseButton, SIGNAL(clicked()), this, SLOT(chooseButtonClicked()));
}

KBackupDlg::~KBackupDlg()
{
  KConfig *config = KGlobal::config();
  config->setGroup("Last Use Settings");
  config->writeEntry("KBackupDlg_mountDevice", mountCheckBox->isChecked());
  config->sync();
  qDebug("saved backup settings");
}

void KBackupDlg::chooseButtonClicked()
{
  txtMountPoint->setText(KFileDialog::getExistingDirectory());
}
