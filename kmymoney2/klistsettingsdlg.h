/***************************************************************************
                          klistsettingsdlg.h
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

#ifndef KLISTSETTINGSDLG_H
#define KLISTSETTINGSDLG_H
/*
//Generated area. DO NOT EDIT!!!(begin)
#include <qwidget.h>
#include <qgroupbox.h>
#include <kcolorbtn.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qbuttongroup.h>
#include <qcheckbox.h>
//Generated area. DO NOT EDIT!!!(end)
*/
//#include <klocale.h>
//#include <qdialog.h>

#include "klistsettingsdlgdecl.h"

/**
  *@author Michael Edwardes
  */

class KListSettingsDlg : public KListSettingsDlgDecl  {
   Q_OBJECT
public: 
	KListSettingsDlg(QWidget *parent=0, const char *name=0);
	~KListSettingsDlg();
	
	QColor m_listColor;
	QColor m_listBGColor;
	QFont m_listHeaderFont;
	QFont m_listCellFont;

protected slots:
  void okClicked();
  void headerBtnClicked();
  void cellBtnClicked();
};

#endif
