/***************************************************************************
                          kbanksview.h
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

#ifndef KBANKSVIEW_H
#define KBANKSVIEW_H

#include <qwidget.h>
#include <qlabel.h>

#include <klocale.h>
#include <qwidget.h>
#include <qevent.h>
#include <qsize.h>

#include <klistview.h>

#include "../mymoney/mymoneyfile.h"
#include "kbankviewdecl.h"
#include "kbanklistitem.h"

// This class handles the bank 'view'.
// It handles the resize event, the totals widgets
// and the KBankListView itself
class KAccountsView : public KBankViewDecl  {
   Q_OBJECT
private:
  bool m_bSelectedAccount;
  QCString m_selectedAccount;
  bool m_bSignals;
  bool m_bViewNormalAccountsView;

  void showSubAccounts(QCStringList accounts, KAccountListItem *parentItem, MyMoneyFile* file);

public: 
	KAccountsView(QWidget *parent=0, const char *name=0);
	~KAccountsView();
	QCString currentAccount(bool&);
	void refresh(MyMoneyFile* file, const QCString& selectAccount);
	void clear(void);
  void show();

  /* NEVER USE unless you know what you're doing. */
  /* Contact mte@users.sourceforge.net for more info */
  void setSignals(bool enable);

protected:
  void resizeEvent(QResizeEvent*);

protected slots:
	
	/**
	* This slot receives the signal from the listview control that an item was double-clicked,
	* if this item was a bank account, try to show the list of transactions for that bank.
	*/
  void slotListDoubleClicked(QListViewItem* pItem, const QPoint& pos, int c);

	/**
	* This slot receives the signal from the listview control that an item was right-clicked,
	* Pass this signal along to the main view to display the RMB menu.
	*/
  void slotListRightMouse(QListViewItem* item, const QPoint& point, int);

  void slotSelectionChanged(QListViewItem *item);

signals:
  void accountRightMouseClick(const QCString&, bool inList);
  void accountDoubleClick();
  //void accountSelected();
  void signalViewActivated();
};

#endif
