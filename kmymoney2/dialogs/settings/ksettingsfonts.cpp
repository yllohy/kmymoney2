/***************************************************************************
                             ksettingsfonts.cpp
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

#include "ksettingsfonts.h"

KSettingsFonts::KSettingsFonts(QWidget* parent, const char* name) :
  KSettingsFontsDecl(parent, name)
{
}

KSettingsFonts::~KSettingsFonts()
{
}

#include "ksettingsfonts.moc"
