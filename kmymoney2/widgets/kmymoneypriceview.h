/***************************************************************************
                          kmymoneypriceview.h  -  description
                             -------------------
    begin                : Wed Mar 24 2004
    copyright            : (C) 2004 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
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

#include "../widgets/kmymoneypriceviewdecl.h"
#include <kmymoney/kmymoneylistviewitem.h>
#include <kmymoney/mymoneyfile.h>
#include <kmymoney/mymoneymoney.h>
#include <kmymoney/mymoneyprice.h>

/**
  * @author Thomas Baumgart
  */

class kMyMoneyPriceItem : public KMyMoneyListViewItem
{
public:
  kMyMoneyPriceItem(KListView *, const MyMoneyPrice& pr);
  ~kMyMoneyPriceItem() {};

  int compare(QListViewItem *p, int col, bool ascending) const;

  const MyMoneyPrice price(void) const { return m_pr; };

private:
  MyMoneyPrice  m_pr;
};


class kMyMoneyPriceView : public kMyMoneyPriceViewDecl
{
   Q_OBJECT
public:
  kMyMoneyPriceView(QWidget *parent=0, const char *name=0);
  ~kMyMoneyPriceView();

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
  void slotNewPrice(void);
  void slotDeletePrice(void);
  int slotEditPrice(void);
  void slotShowAllPrices(bool enabled);
  void slotOnlinePriceUpdate(void);
  void slotReloadWidget(void);

private slots:
  void slotTimerDone(void);

private:
  KPopupMenu*     m_contextMenu;
  bool            m_showAll;
};

#endif
