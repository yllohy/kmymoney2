/***************************************************************************
                          konlinebankingsetupwizard.cpp
                             -------------------
    begin                : Sat Jan 7 2006
    copyright            : (C) 2006 by Ace Jones
    email                : acejones@users.sourceforge.net
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

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include <../dialogs/konlinebankingsetupwizard.h>

KOnlineBankingSetupWizard::KOnlineBankingSetupWizard(QWidget *parent, const char *name):
  KOnlineBankingSetupDecl(parent,name)
{
}

KOnlineBankingSetupWizard::~KOnlineBankingSetupWizard()
{
}

void KOnlineBankingSetupWizard::next(void)
{
    KOnlineBankingSetupDecl::next();
}
// vim:cin:si:ai:et:ts=2:sw=2:
