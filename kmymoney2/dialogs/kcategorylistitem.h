/***************************************************************************
                          kcategorylistitem.h
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

#ifndef KCATEGORYLISTITEM_H
#define KCATEGORYLISTITEM_H

#include <qwidget.h>
#include <klistview.h>
#include <qstring.h>
#include <qcstring.h>

// This class represents a List item that is used in KCategoriesDlg.
// It holds information on whether it is a major category and the
// values.
class KCategoryListItem : public QListViewItem  {
  QCString m_accountID;

public:
  KCategoryListItem(KListView *parent, const QString& accountName, const QCString& accountID, const QString&);
  KCategoryListItem(KCategoryListItem *parent, const QString& accountName, const QCString& accountID, const QString&);
	~KCategoryListItem();
	QCString accountID(void);
};

#endif
