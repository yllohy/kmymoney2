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

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes
#include <klistview.h>
#include "kinvestmentlistitem.h"

#include "../mymoney/mymoneyequity.h"
#include "../mymoney/mymoneyfile.h"

KInvestmentListItem::KInvestmentListItem(KListView* parent, const MyMoneyAccount& account)
  : KListViewItem(parent)
{
  m_account = account;
  m_listView = parent;
  update(account.id());

  MyMoneyFile::instance()->attach(m_account.id(), this);
  MyMoneyFile::instance()->attach(m_account.currencyId(), this);
}

KInvestmentListItem::~KInvestmentListItem()
{
  MyMoneyFile::instance()->detach(m_account.currencyId(), this);
  MyMoneyFile::instance()->detach(m_account.id(), this);
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

void KInvestmentListItem::update(const QCString& id)
{
  MyMoneyFile* file = MyMoneyFile::instance();

  try {
    MyMoneyEquity equity;
    m_account = file->account(m_account.id());
    equity = file->equity(m_account.currencyId());
    QValueList<MyMoneyTransaction> transactionList;
    equity_price_history history = equity.priceHistory();

    //column 0 (COLUMN_NAME_INDEX) is the name of the stock
    setText(COLUMN_NAME_INDEX, equity.name());

    //column 1 (COLUMN_SYMBOL_INDEX) is the ticker symbol
    setText(COLUMN_SYMBOL_INDEX, equity.tradingSymbol());

    //column 2 (COLUMN_QUANTITY_INDEX) is the quantity of shares owned
    setText(COLUMN_QUANTITY_INDEX, file->balance(m_account.id()).formatMoney("", 2));

    //column 3 is the current price
    setText(COLUMN_CURRPRICE_INDEX, equity.price(QDate::currentDate()).formatMoney());

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

    if(history.isEmpty())
    {
      setText(COLUMN_1WEEKGAIN_INDEX, QString("0.0%"));
      setText(COLUMN_4WEEKGAIN_INDEX, QString("0.0%"));
      setText(COLUMN_3MONGAIN_INDEX, QString("0.0%"));
      setText(COLUMN_YTDGAIN_INDEX, QString("0.0%"));
    }
    else
    {
      setText(COLUMN_1WEEKGAIN_INDEX, calculate1WeekGain(history));
      setText(COLUMN_4WEEKGAIN_INDEX, calculate4WeekGain(history));
      setText(COLUMN_3MONGAIN_INDEX, calculate3MonthGain(history));
      setText(COLUMN_YTDGAIN_INDEX, calculateYTDGain(history));
    }

  } catch(MyMoneyException *e) {
    delete e;
  }
}