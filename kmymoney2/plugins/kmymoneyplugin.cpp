/***************************************************************************
                          kmymoneyplugin.cpp
                             -------------------
    begin                : Wed Jan 5 2005
    copyright            : (C) 2005 Thomas Baumgart
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

#include <kinstance.h>
#include <kaboutdata.h>

// ----------------------------------------------------------------------------
// Project Includes


#include "kmymoneyplugin.h"

KMyMoneyPlugin::Plugin::Plugin(QObject* o, const char* name) :
  QObject(o, name)
{
}

KMyMoneyPlugin::Plugin::~Plugin()
{
}

KMyMoneyPlugin::ViewInterface* KMyMoneyPlugin::Plugin::viewInterface()
{
  return static_cast<ViewInterface*>( parent()->child( 0, "KMyMoneyPlugin::ViewInterface" ) );
}

KMyMoneyPlugin::StatementInterface* KMyMoneyPlugin::Plugin::statementInterface()
{
  return static_cast<StatementInterface*>( parent()->child( 0, "KMyMoneyPlugin::StatementInterface" ) );
}

