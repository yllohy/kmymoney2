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
#include <klocale.h>
#if QT_VERSION > 300
#include <kstandarddirs.h>
#else
#include <kstddirs.h>
#endif

#include <qpixmap.h>
#include <qlabel.h>

#include <kglobal.h>
#include <klocale.h>
#if QT_VERSION > 300
#include <kstandarddirs.h>
#else
#include <kstddirs.h>
#endif

#include <qpixmap.h>

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
  QString filename = KGlobal::dirs()->findResource("appdata", "pics/dlg_backup.png");
  QPixmap *pm = new QPixmap(filename);
  m_qpixmaplabel->setPixmap(*pm);

  readConfig();
  connect(chooseButton, SIGNAL(clicked()), this, SLOT(chooseButtonClicked()));
  connect(btnOK,SIGNAL(clicked()),this,SLOT(accept()));
  connect(btnCancel,SIGNAL(clicked()),this,SLOT(reject()));
}

KBackupDlg::~KBackupDlg()
{
  writeConfig();
}

void KBackupDlg::chooseButtonClicked()
{
  QString newDir = KFileDialog::getExistingDirectory();
  if (!newDir.isEmpty())
    txtMountPoint->setText(newDir);
}

void KBackupDlg::readConfig(void)
{
  KConfig *config = KGlobal::config();
  config->setGroup("Last Use Settings");
  mountCheckBox->setChecked(config->readBoolEntry("KBackupDlg_mountDevice", false));
  txtMountPoint->setText(config->readEntry("KBackupDlg_BackupMountPoint", "/mnt/floppy"));
}

void KBackupDlg::writeConfig(void)
{
  KConfig *config = KGlobal::config();
  config->setGroup("Last Use Settings");
  config->writeEntry("KBackupDlg_mountDevice", mountCheckBox->isChecked());
  config->writeEntry("KBackupDlg_BackupMountPoint", txtMountPoint->text());
  config->sync();
}
