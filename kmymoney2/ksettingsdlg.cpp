/***************************************************************************
                          ksettingsdlg.cpp
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes
    email                : Michael.Edwardes@students.dmu.ac.uk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "ksettingsdlg.h"
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qcheckbox.h>

KSettingsDlg::KSettingsDlg(bool checkDialog, bool checkLastFile, QWidget *parent, const char *name)
 : KSettingsDlgDecl(parent,name,true)
{
//	initDialog();
	
  startDialog = checkDialog;
  startDialogRadio->setChecked(checkDialog);
  openLastFile = checkLastFile;
  lastFileRadio->setChecked(checkLastFile);

  connect(cancelBtn, SIGNAL(clicked()), this, SLOT(reject()));
  connect(okBtn, SIGNAL(clicked()), this, SLOT(okClicked()));	
  connect(startDialogRadio, SIGNAL(toggled(bool)), this, SLOT(startDialogToggle(bool)));
  connect(lastFileRadio, SIGNAL(toggled(bool)), this, SLOT(lastFileToggle(bool)));
}

KSettingsDlg::~KSettingsDlg()
{
}

void KSettingsDlg::okClicked()
{
  accept();
}

void KSettingsDlg::startDialogToggle(bool on)
{
  if (on)
    startDialog = true;
  else
    startDialog = false;
}

void KSettingsDlg::lastFileToggle(bool on)
{
  if (on)
    openLastFile = true;
  else
    openLastFile = false;
}

//#include "ksettingsdlg.moc"
