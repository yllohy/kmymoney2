/***************************************************************************
                          kinvestmentlistitem.h  -  description
                             -------------------
    begin                : Wed Feb 6 2002
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

#ifndef KINVESTMENTLISTITEM_H
#define KINVESTMENTLISTITEM_H

#include <klistview.h>

#include "../mymoney/mymoneyequity.h"
#include "../mymoney/mymoneytransaction.h"

//indexes for the various columns on the summary view
#define COLUMN_NAME_INDEX       0
#define COLUMN_SYMBOL_INDEX     1
#define COLUMN_QUANTITY_INDEX   2
#define COLUMN_CURRPRICE_INDEX  3
#define COLUMN_COSTBASIS_INDEX  4
#define COLUMN_RAWGAIN_INDEX    5
#define COLUMN_1WEEKGAIN_INDEX  6
#define COLUMN_4WEEKGAIN_INDEX  7
#define COLUMN_3MONGAIN_INDEX   8
#define COLUMN_YTDGAIN_INDEX    9

/**
  *@author Kevin Tambascio
  */

class KInvestmentListItem : public KListViewItem  {
public: 
	KInvestmentListItem(KListView* parent, const MyMoneyEquity& equity, const QValueList<MyMoneyTransaction>& transactionList);
	~KInvestmentListItem();

  QCString equityId() const { return m_equity.id(); }	
protected:
  void paintCell(QPainter * p, const QColorGroup & cg, int column, int width, int align);

private:
  const QString calculate1WeekGain(const equity_price_history& history);
  const QString calculate4WeekGain(const equity_price_history& history);
  const QString calculate3MonthGain(const equity_price_history& history);
  const QString calculateYTDGain(const equity_price_history& history);
	KListView *m_listView;
  MyMoneyEquity m_equity;	
};

#endif
