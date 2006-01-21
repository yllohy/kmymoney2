/***************************************************************************
                          knewbudgetdlg.cpp
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

// ----------------------------------------------------------------------------
// QT Includes

#include <qcheckbox.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kpushbutton.h>
#include <klistview.h>
#include <kiconloader.h>
#include <kguiitem.h>

// ----------------------------------------------------------------------------
// Project Includes
#include "knewbudgetdlg.h"

KNewBudgetDlg::KNewBudgetDlg(QWidget* parent, const char *name) :
  KNewBudgetDlgDecl(parent, name)
{
}

KNewBudgetDlg::~KNewBudgetDlg()
{
}

#include "knewbudgetdlg.moc"
