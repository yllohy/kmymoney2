/***************************************************************************
                          kcategorylistitem.cpp
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
#include <klocale.h>
#include <klistview.h>
#include "kcategorylistitem.h"

KCategoryListItem::KCategoryListItem(KListView *parent, const QString& accountName, const QCString& accountID, const QString& typeName)
  : QListViewItem(parent), m_accountID(accountID)
{
  setText(0, accountName);
  setText(1, typeName);
}

KCategoryListItem::KCategoryListItem(KCategoryListItem *parent, const QString& accountName, const QCString& accountID, const QString& typeName)
  : QListViewItem(parent), m_accountID(accountID)
{
  setText(0, accountName);
  setText(1, typeName);
}

KCategoryListItem::~KCategoryListItem()
{
}

QCString KCategoryListItem::accountID(void)
{
  return m_accountID;
}
