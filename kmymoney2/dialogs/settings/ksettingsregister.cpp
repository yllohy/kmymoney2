/***************************************************************************
                             ksettingsregister.cpp
                             --------------------
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

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ksettingsregister.h"

KSettingsRegister::KSettingsRegister(QWidget* parent, const char* name) :
  KSettingsRegisterDecl(parent, name)
{
}

KSettingsRegister::~KSettingsRegister()
{
}

#include "ksettingsregister.moc"
