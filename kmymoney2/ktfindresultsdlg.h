/***************************************************************************
                          ktfindresultsdlg.h
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

#ifndef KTFINDRESULTSDLG_H
#define KTFINDRESULTSDLG_H

#include <klocale.h>
#include <qdialog.h>
#include <qlist.h>

#include "mymoney/mymoneytransaction.h"
//#include "kaccountlistview.h"
#include "ktfindresultsdlgdecl.h"

/**
  *@author Michael Edwardes
  */

class KTFindResultsDlg : public KTFindResultsDlgDecl  {
   Q_OBJECT
public: 
	KTFindResultsDlg(QWidget *parent=0, const char *name=0);
	~KTFindResultsDlg();
	void setList(QList<MyMoneyTransaction> list);
};

#endif
