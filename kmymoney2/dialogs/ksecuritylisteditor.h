/***************************************************************************
                          ksecuritylisteditor.h  -  description
                             -------------------
    begin                : Wed Dec 16 2004
    copyright            : (C) 2004 by Thomas Baumgart
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

#ifndef KSECURITYLISTEDITOR_H
#define KSECURITYLISTEDITOR_H

// ----------------------------------------------------------------------------
// QT Includes

class QListViewItem;

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ksecuritylisteditordecl.h"

#include "../mymoney/mymoneysecurity.h"

/**
  * @author Thomas Baumgart
  */

class KSecurityListEditor : public KSecurityListEditorDecl
{
  Q_OBJECT
public:
  KSecurityListEditor(QWidget *parent, const char* name = 0);
  ~KSecurityListEditor();

protected slots:
  void slotLoadList(void);
  void slotUpdateButtons(void);
  void slotEditSecurity(void);
  void slotDeleteSecurity(void);

protected:
  void fillItem(QListViewItem* item, const MyMoneySecurity& security);

};

#endif
