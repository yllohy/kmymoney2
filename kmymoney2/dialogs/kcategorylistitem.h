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
#include <qlistview.h>
#include <qstringlist.h>

// This class represents a List item that is used in KCategoriesDlg.
// It holds information on whether it is a major category and the
// values.
class KCategoryListItem : public QListViewItem  {
public:
  KCategoryListItem( QListView *parent, QString text, QStringList minors, bool income, bool major, QString majorName=QString::null );
  KCategoryListItem( KCategoryListItem *parent, QString text, bool income, bool major, QString majorName=QString::null );
	~KCategoryListItem();
	bool isMajor(void);
  QString majorName(void);
  bool income(void);
  QStringList minors(void);

protected:
  bool m_major;
  QString m_majorName;
  bool m_income;
  QStringList m_minors;
};

#endif
