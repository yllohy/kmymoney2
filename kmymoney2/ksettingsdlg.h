/***************************************************************************
                          ksettingsdlg.h
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes
    email                : mte@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KSETTINGSDLG_H
#define KSETTINGSDLG_H
/*
//Generated area. DO NOT EDIT!!!(begin)
#include <qwidget.h>
#include <qpushbutton.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
//Generated area. DO NOT EDIT!!!(end)
#include <klocale.h>
#include <qdialog.h>
*/

#include "ksettingsdlgdecl.h"

// This dialog lets the user change the program settings.
// Doesn't do much at the moment !
class KSettingsDlg : public KSettingsDlgDecl  {
   Q_OBJECT
protected slots:
  void okClicked();
  void startDialogToggle(bool on);
  void lastFileToggle(bool on);

public:
  bool openLastFile;
  bool startDialog;

public:
  KSettingsDlg(bool checkDialog, bool checkLastFile, QWidget *parent=0, const char *name=0);
	~KSettingsDlg();

protected: 
/*
	void initDialog();
	//Generated area. DO NOT EDIT!!!(begin)
	QPushButton *cancelBtn;
	QPushButton *okBtn;
	QButtonGroup *startGroup;
	QRadioButton *startDialogRadio;
	QRadioButton *lastFileRadio;
	//Generated area. DO NOT EDIT!!!(end)
*/
private: 
};

#endif
