/***************************************************************************
                          kmymoneyplugin.h
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

#ifndef KMYMONEYPLUGIN_H
#define KMYMONEYPLUGIN_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// ----------------------------------------------------------------------------
// QT Includes

#include <qobject.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kxmlguiclient.h>
class KAboutData;
class KInstance;

// ----------------------------------------------------------------------------
// Project Includes

#include "viewinterface.h"
#include "statementinterface.h"

namespace KMyMoneyPlugin {

/**
  * This class describes the interface between the KMyMoney
  * application and it's plugins. All plugins must be derived
  * from this class.
  *
  * A good tutorial on how to design and develop a plugin
  * structure for a KDE application (e.g. KMyMoney) can be found at
  * http://developer.kde.org/documentation/tutorials/developing-a-plugin-structure/index.html
  *
  */
  class Plugin : public QObject, public KXMLGUIClient
  {
    Q_OBJECT
  public:
    Plugin(QObject* parent, const char* name);
    virtual ~Plugin();

  protected:

    // define interface classes here
    // they are defined in the following form for an interface
    // named Xxx:
    //
    // XxxInterface* xxxInterface();
    ViewInterface*          viewInterface();
    StatementInterface*     statementInterface();

  };

}; // end of namespace
#endif
