/***************************************************************************
                          kbanklistitem.h
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

#ifndef KACCOUNTLISTITEM_H
#define KACCOUNTLISTITEM_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qwidget.h>
#include <klistview.h>

// ----------------------------------------------------------------------------
// KDE Includes


// ----------------------------------------------------------------------------
// Project Includes

#include "../mymoney/mymoneyfile.h"

/**
  *@author Michael Edwardes, Thomas Baumgart
  */

class KAccountListItem : public QListViewItem, MyMoneyObserver  {
private:
  QCString m_accountID;
  bool m_bViewNormal;
  int m_nAccountColumn;
  int m_nInstitutionColumn;

public:
  KAccountListItem(KListView *parent, const QString& accountName,
      const QCString& accountID, const QString&, const QString&);
  KAccountListItem(KAccountListItem *parent, const QString& accountName,
      const QCString& accountID, const QString&, const QString&);
  KAccountListItem(KListView *parent, const QString& institutionName,
      const QCString& institutionID);

  KAccountListItem(KListView *parent, const MyMoneyAccount& account);
  KAccountListItem(KAccountListItem *parent, const MyMoneyAccount& account);

	~KAccountListItem();
	QCString accountID(void);
  void paintCell(QPainter *p, const QColorGroup & cg, int column, int width, int align);

  void update(const QCString& id);

private:
  void  newAccount(const MyMoneyAccount& account);

};

#endif
