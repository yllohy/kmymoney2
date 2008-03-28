/***************************************************************************
                          kmymoneyglobalsettings.h
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

#ifndef KMYMONEYGLOBALSETTINGS_H
#define KMYMONEYGLOBALSETTINGS_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/kmymoneysettings.h>

class KMyMoneyGlobalSettings : public KMyMoneySettings
{
public:
  static QFont listCellFont(void);
  static QFont listHeaderFont(void);
  static QColor listColor(void);
  static QColor listBGColor(void);
  static QStringList itemList(void);
  static int firstFiscalMonth(void);
  static int firstFiscalDay(void);
};
#endif
