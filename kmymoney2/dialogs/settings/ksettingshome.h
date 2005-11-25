/***************************************************************************
                          ksettingshome.h
                             -------------------
    copyright            : (C) 2005 by Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KSETTINGSHOME_H
#define KSETTINGSHOME_H

// ----------------------------------------------------------------------------
// QT Includes

class QListViewItem;

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoney2/dialogs/settings/ksettingshomedecl.h"

class KSettingsHome : public KSettingsHomeDecl
{
  Q_OBJECT

public:
  KSettingsHome(QWidget* parent = 0, const char* name = 0);
  ~KSettingsHome();

protected slots:
  void slotLoadItems(void);
  void slotUpdateItemList(void);
  void slotSelectHomePageItem(QListViewItem *);
  void slotMoveUp(void);
  void slotMoveDown(void);

private:
  bool m_noNeedToUpdateList;
};
#endif

