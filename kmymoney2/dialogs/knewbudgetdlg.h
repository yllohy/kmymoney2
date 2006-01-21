/***************************************************************************
                          knewbudgetdlg.h
                             -------------------
    begin                : Wed Jan 18 2006
    copyright            : (C) 2000-2004 by Darren Gould
    email                : darren_gould@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KNEWBUDGETDLG_H
#define KNEWBUDGETDLG_H

// ----------------------------------------------------------------------------
// QT Includes

class QListViewItem;

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "knewbudgetdlgdecl.h"

class KNewBudgetDlg : public KNewBudgetDlgDecl
{
  Q_OBJECT
public:
  KNewBudgetDlg(QWidget* parent, const char *name);
  ~KNewBudgetDlg();
};

#endif // KNEWBUDGETDLG_H
