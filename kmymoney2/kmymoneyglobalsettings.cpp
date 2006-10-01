/***************************************************************************
                          kmymoneyglobalsettings.cpp
                             -------------------
    copyright            : (C) 2006 by Thomas Baumgart
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

#include <kglobalsettings.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/kmymoneyglobalsettings.h>

QFont KMyMoneyGlobalSettings::listCellFont(void)
{
  if(useSystemFont()) {
    return KGlobalSettings::generalFont();
  } else {
    return KMyMoneySettings::listCellFont();
  }
}

QFont KMyMoneyGlobalSettings::listHeaderFont(void)
{
  if(useSystemFont()) {
    QFont font = KGlobalSettings::generalFont();
    font.setBold(true);
    return font;
  } else {
    return KMyMoneySettings::listHeaderFont();
  }
}
