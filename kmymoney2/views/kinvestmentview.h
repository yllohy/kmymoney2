/***************************************************************************
                          kinvestmentview.h  -  description
                             -------------------
    begin                : Tue Jan 29 2002
    copyright            : (C) 2000-2002 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KINVESTMENTVIEW_H
#define KINVESTMENTVIEW_H

#include <qlist.h>
#include <kpopupmenu.h>
#include "../mymoney/mymoneyequity.h"
#include "kinvestmentviewdecl.h"

class MyMoneyAccount;
class MyMoneyTransaction;
class MyMoneyInvestTransaction;

/**
  *@author Kevin Tambascio
  */

class KInvestmentView : public kInvestmentViewDecl
{
	Q_OBJECT
public:
	typedef enum {
	  VIEW_SUMMARY = 0,
	  VIEW_INVESTMENT,
	  VIEW_CASH
	} eViewType;
	
	KInvestmentView(QWidget *parent=0, const char *name=0);
	~KInvestmentView();

  /** No descriptions */
  bool init(MyMoneyAccount *pAccount);

  /** No descriptions */
  void updateDisplay();

  /** No description */
  void displayNewEquity(const MyMoneyEquity *pEntry);

  /** No description */
  void addEquityEntry(MyMoneyEquity *pEntry);

protected slots:
	/**
	* This slot receives the signal from the listview control that an item was double-clicked,
	*/
  void slotListDoubleClicked(QListViewItem* pItem, const QPoint& pos, int c);

	/**
	* This slot receives the signal from the listview control that an item was right-clicked,
	*/
  void slotListRightMouse(QListViewItem* item, const QPoint& point, int);
  void slotNewInvestment();
	void slotEditInvestment();
  void slotUpdatePrice();
	void slotViewChanged(int);
	
private:
	KPopupMenu* m_popMenu;
	MyMoneyAccount *m_pAccount;
	QList<MyMoneyTransaction> m_transactionList;
};

#endif
