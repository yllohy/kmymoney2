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

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <klistview.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "../mymoney/mymoneyequity.h"
#include "../mymoney/mymoneyaccount.h"
#include "../mymoney/mymoneytransaction.h"
#include "../mymoney/mymoneyobserver.h"

//indexes for the various columns on the summary view
#define COLUMN_NAME_INDEX       0
#define COLUMN_SYMBOL_INDEX     1
#define COLUMN_QUANTITY_INDEX   2
#define COLUMN_CURRVALUE_INDEX  3
#define COLUMN_COSTBASIS_INDEX  4
#define COLUMN_RAWGAIN_INDEX    5
#define COLUMN_1WEEKGAIN_INDEX  6
#define COLUMN_4WEEKGAIN_INDEX  7
#define COLUMN_3MONGAIN_INDEX   8
#define COLUMN_YTDGAIN_INDEX    9

/**
  *@author Kevin Tambascio
  */

class KInvestmentListItem : public KListViewItem, public MyMoneyObserver
{
public:
  KInvestmentListItem(KListView* parent, const MyMoneyAccount& equity);
  ~KInvestmentListItem();

  QCString equityId() const { return m_account.currencyId(); }
  void update(const QCString& id);

protected:
  void paintCell(QPainter * p, const QColorGroup & cg, int column, int width, int align);

private:
  const QString calculate1WeekGain(const equity_price_history& history);
  const QString calculate4WeekGain(const equity_price_history& history);
  const QString calculate3MonthGain(const equity_price_history& history);
  const QString calculateYTDGain(const equity_price_history& history);
  const QString calculateGain(const equity_price_history& history, int dayDifference, int monthDifference, bool YTD, bool& bNegative);

private:
  KListView*        m_listView;
  MyMoneyAccount    m_account;
  bool bColumn5Negative, bColumn6Negative, bColumn7Negative, bColumn8Negative, bColumn9Negative;
};

#endif
