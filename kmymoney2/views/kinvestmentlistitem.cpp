/***************************************************************************
                          kinvestmentlistitem.cpp  -  description
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
 
#include <klistview.h>
#include "kinvestmentlistitem.h"

#include "../mymoney/mymoneyequity.h"

KInvestmentListItem::KInvestmentListItem(KListView* parent, const MyMoneyEquity& equity, const QValueList<MyMoneyTransaction>& transactionList)
	: KListViewItem(parent)
{
  m_equity = equity;
	m_listView = parent;

  //KListViewItem* item1 = new KListViewItem(investmentTable, QString("Redhat"), QString("RHAT"), QString("24"), QString("$20.00"), QString("$13.43"), QString("$212"), QString("5.43%"), QString("9.43%"));
  //investmentTable->insertItem(item1);
  
	
	//column 0 (COLUMN_NAME_INDEX) is the name of the stock
	setText(COLUMN_NAME_INDEX, equity.name());
	
	//column 1 (COLUMN_SYMBOL_INDEX) is the ticker symbol
	setText(COLUMN_SYMBOL_INDEX, equity.tradingSymbol());
	
  //column 2 (COLUMN_QUANTITY_INDEX) is the quantity of shares owned
  if(transactionList.isEmpty())
  {
    setText(COLUMN_QUANTITY_INDEX, QString("0"));
  }
  else //find all the transactions that added/removed shares, and find the total number owned.
  {

  }
  
  //column 3 is the current price, using the QString translator to get a string value.
	QString strValue;
  equity_price_history history = equity.priceHistory();
  equity_price_history::ConstIterator it = history.begin();
  strValue = it.data().formatMoney();
  setText(COLUMN_CURRPRICE_INDEX, strValue);

  //column 4 (COLUMN_COSTBASIS_INDEX) is the cost basis
  if(transactionList.isEmpty())
  {
    MyMoneyMoney money;
    setText(COLUMN_COSTBASIS_INDEX, money.formatMoney());
  }
  else //find all the transactions that added/removed shares, and find the total number owned.
  {

  }

  //column 5 (COLUMN_RAWGAIN_INDEX) is the loss/gain that has occurred over the life of the ownership
  if(transactionList.isEmpty())
  {
    MyMoneyMoney money;
    setText(COLUMN_RAWGAIN_INDEX, money.formatMoney());
  }
  else //find all the transactions that added/removed shares, and find the total number owned.
  {

  }

  //column 6 (COLUMN_1WEEKGAIN_INDEX) is the percentage gain/loss over the last week.
  if(history.isEmpty())
  {
    setText(COLUMN_1WEEKGAIN_INDEX, QString("0.0%"));
  }
  else
  {
    setText(COLUMN_1WEEKGAIN_INDEX, calculate1WeekGain(history));
  }
  
  //column 7 (COLUMN_4WEEKGAIN_INDEX) is the percentage gain/loss over the last 4 weeks.
  if(history.isEmpty())
  {
    setText(COLUMN_4WEEKGAIN_INDEX, QString("0.0%"));
  }
  else
  {
    setText(COLUMN_4WEEKGAIN_INDEX, calculate4WeekGain(history));
  }
  
  //column 8 (COLUMN_3MONGAIN_INDEX) is the percentage gain/loss over the last 3 months.
  if(history.isEmpty())
  {
    setText(COLUMN_3MONGAIN_INDEX, QString("0.0%"));
  }
  else
  {
    setText(COLUMN_3MONGAIN_INDEX, calculate3MonthGain(history));
  }
  
  //column 9 (COLUMN_YTDGAIN_INDEX) is the percentage gain/loss YTD.
  if(history.isEmpty())
  {
    setText(COLUMN_YTDGAIN_INDEX, QString("0.0%"));
  }
  else
  {
    setText(COLUMN_YTDGAIN_INDEX, calculateYTDGain(history));
  }
}

KInvestmentListItem::~KInvestmentListItem()
{

}

const QString KInvestmentListItem::calculate1WeekGain(const equity_price_history& history)
{
  return QString("");
}

const QString KInvestmentListItem::calculate4WeekGain(const equity_price_history& history)
{
  return QString("");
}

const QString KInvestmentListItem::calculate3MonthGain(const equity_price_history& history)
{
  return QString("");
}

const QString KInvestmentListItem::calculateYTDGain(const equity_price_history& history)
{
  return QString("");
}

void KInvestmentListItem::paintCell(QPainter * p, const QColorGroup & cg, int column, int width, int align)
{
  QListViewItem::paintCell(p, cg, column, width, align);
}
