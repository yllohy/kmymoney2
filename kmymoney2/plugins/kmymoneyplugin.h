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

/**
  * This class describes the interface between the KMyMoney
  * application and it's IMPORTER plugins. All importer plugins 
  * must be derived from this class.
  *
  * A good tutorial on how to design and develop a plugin
  * structure for a KDE application (e.g. KMyMoney) can be found at
  * http://developer.kde.org/documentation/tutorials/developing-a-plugin-structure/index.html
  *
  */
  class ImporterPlugin : public QObject
  {
    Q_OBJECT
  public:
    ImporterPlugin(QObject* parent, const char* name);
    virtual ~ImporterPlugin();
    
    /**
      * This method returns the english-language name of the format
      * this plugin imports, e.g. "OFX"
      *
      * @return QString Name of the format
      */
    virtual QString formatName(void) const /*= 0*/;
  
    /**
      * This method returns whether this plugin is able to import
      * a particular file.
      *
      * @param filename Fully-qualified pathname to a file
      *
      * @return bool Whether the indicated file is importable by this plugin
      */
    virtual bool isMyFormat( const QString& filename ) const /*= 0*/;
    
    /**
      * Import a file
      *
      * @param filename File to import
      * @param result List of statements onto which to add the resulting 
      *  statements 
      *
      * @return bool Whether the import was successful.  If the return value is
      *  false, the @p result list should be unmodified.
      */
    virtual bool import( const QString& filename, QValueList<MyMoneyStatement>& result ) /*= 0*/;
  
    /**
      * Returns the error result of the last import
      *
      * @return QString English-language name of the error encountered in the
      *  last import, or QString() if it was successful.
      * 
      */
    virtual QString lastError(void) const /*= 0*/;
    
  };
  
}; // end of namespace
#endif
