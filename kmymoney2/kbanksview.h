/***************************************************************************
                          kbanksview.h
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes
    email                : Michael.Edwardes@students.dmu.ac.uk
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

//#include <qdbt/qdbttabular.h>
#include <klistview.h>
//#include <qlistview.h>

#include <mymoney/mymoneyfile.h>
//#include "kbanklistview.h"
#include "kbankviewdecl.h"

// This class handles the bank 'view'.
// It handles the resize event, the totals widgets
// and the KBankListView itself
class KBanksView : public KBankViewDecl  {
   Q_OBJECT
private:
  bool m_bSelectedBank, m_bSelectedAccount;
  MyMoneyBank m_selectedBank;
  MyMoneyAccount m_selectedAccount;

public: 
	KBanksView(QWidget *parent=0, const char *name=0);
	~KBanksView();
	MyMoneyBank currentBank(bool&);
	MyMoneyAccount currentAccount(bool&);
	void refresh(MyMoneyFile file);
	void clear(void);

protected slots:
  void slotListRightMouse(QListViewItem* item, const QPoint& point, int);
  void slotListDoubleClick(QListViewItem* item, const QPoint& point, int col);

signals:
  void bankRightMouseClick(const MyMoneyBank, bool inList);
//  void bankDoubleClick(const MyMoneyBank, bool inList);
  void accountRightMouseClick(const MyMoneyAccount, bool inList);
  void accountDoubleClick(const MyMoneyAccount);
};

#endif
