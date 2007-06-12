/***************************************************************************
                             accounts.cpp
                             -------------------
    begin                : Fri Jun  1 2007
    copyright            : (C) 2007 Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
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

#include <qheader.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klistview.h>
#include <kmymoney/kmymoneyaccounttree.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "accounts.h"

Accounts::Accounts(QWidget* parent, const char* name) :
  AccountsDecl(parent, name)
{
  m_accountList->header()->hide();
}

#include "accounts.moc"
