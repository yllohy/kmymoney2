/***************************************************************************
                          kfileinfodlg.h
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

#ifndef KFILEINFODLG_H
#define KFILEINFODLG_H

//Generated area. DO NOT EDIT!!!(begin)
#include <qwidget.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
//Generated area. DO NOT EDIT!!!(end)

#include <qdialog.h>
#include <klocale.h>
#include <qdatetime.h>

#include "kfileinfodlgdecl.h"

// This dialog just displays some data
class KFileInfoDlg : public KFileInfoDlgDecl  {
   Q_OBJECT

public: 
	KFileInfoDlg(QDate created, QDate access, QDate modify, QWidget *parent=0, const char *name=0);
	~KFileInfoDlg();

protected: 
/*
	void initDialog();
	//Generated area. DO NOT EDIT!!!(begin)
	QLabel *QLabel_1;
	QLabel *QLabel_2;
	QLabel *QLabel_3;
	QLineEdit *createdEdit;
	QLineEdit *lastAccessEdit;
	QLineEdit *lastModifyEdit;
	QPushButton *okBtn;
	//Generated area. DO NOT EDIT!!!(end)
*/
};

#endif
