/***************************************************************************
                          ktfindresultsdlg.cpp
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
//#include <klistview.h>
#include "ktfindresultsdlg.h"
//#include "ktransactionlistitem.h"
#include <qtable.h>
#include "mymoney/mymoneyaccount.h"

KTFindResultsDlg::KTFindResultsDlg(QWidget *parent, const char *name)
 : KTFindResultsDlgDecl(parent,name,false)
{
//	initDialog();
	
//	transactionList = new KTransactionListView(this);
//	transactionList->setGeometry(10, 10, 660, 420);
	
//	connect(buttonOk, SIGNAL(clicked()), this, SLOT(accept()));
}

KTFindResultsDlg::~KTFindResultsDlg()
{
}

void KTFindResultsDlg::setList(QList<MyMoneyTransaction> list)
{
/*
	MyMoneyTransaction *transaction;
	MyMoneyMoney balance;
	
	for (transaction = list.first(); transaction!=0; transaction = list.next()) {
  	(void) new QTableItem();
  }
 */
}
