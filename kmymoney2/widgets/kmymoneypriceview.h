/***************************************************************************
                          kmymoneypriceview.h  -  description
                             -------------------
    begin                : Wed Mar 24 2004
    copyright            : (C) 2004 by Thomas Baumgart
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

#ifndef KMYMONEYPRICEVIEW_H
#define KMYMONEYPRICEVIEW_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qwidget.h>
class QListViewItem;

// ----------------------------------------------------------------------------
// KDE Includes

#include <kpopupmenu.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneypriceviewdecl.h"
#include "../mymoney/mymoneymoney.h"
#include "kmymoneyaccountselector.h"

/**
  * @author Thomas Baumgart
  */

class kMyMoneyPriceItem : public kMyMoneyListViewItem
{
public:
  kMyMoneyPriceItem(KListView *, const QDate& data, const MyMoneyMoney& price);
  ~kMyMoneyPriceItem();

  int compare(QListViewItem *p, int col, bool ascending) const;
  const QDate date(void) const { return m_date; };
  const MyMoneyMoney price(void) const { return m_price; };
  void setPrice(const MyMoneyMoney& price);
  void setDate(const QDate& date);

private:
  QDate         m_date;
  MyMoneyMoney  m_price;
};


class kMyMoneyPriceView : public kMyMoneyPriceViewDecl
{
   Q_OBJECT
public:
  kMyMoneyPriceView(QWidget *parent=0, const char *name=0);
  ~kMyMoneyPriceView();

  void setHistory(const QMap<QDate,MyMoneyMoney>& history);
  const QMap<QDate, MyMoneyMoney> history(void) const;
  const bool dirty(void) const { return m_dirty; };

signals:
  /**
    * This signal is a forward of the listview's clicked() signal.
    * See QListView::clicked() for details.
    */
  void selectionChanged(QListViewItem* item);

protected:
  /// the resize event
  virtual void resizeEvent(QResizeEvent*);

protected slots:
  void slotListClicked(QListViewItem* item, const QPoint&, int);
  void slotAddPrice(void);
  void slotDeletePrice(void);
  void slotEditPrice(void);

private:
  bool            m_dirty;
  KPopupMenu*     m_contextMenu;
};

#endif
