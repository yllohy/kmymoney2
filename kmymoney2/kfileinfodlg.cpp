/***************************************************************************
                          kfileinfodlg.cpp
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
#include <kglobal.h>
#include <klocale.h>

#include "kfileinfodlg.h"

KFileInfoDlg::KFileInfoDlg(QDate created, QDate access, QDate modify, QWidget *parent, const char *name)
 : KFileInfoDlgDecl(parent,name,true)
{
//	initDialog();
	
	createdEdit->setReadOnly(true);
	createdEdit->setText(KGlobal::locale()->formatDate(created));
	lastAccessEdit->setReadOnly(true);
	lastAccessEdit->setText(KGlobal::locale()->formatDate(access));
	lastModifyEdit->setReadOnly(true);
	lastModifyEdit->setText(KGlobal::locale()->formatDate(modify));
	
	connect(okBtn, SIGNAL(clicked()), SLOT(accept()));
}

KFileInfoDlg::~KFileInfoDlg()
{
}
