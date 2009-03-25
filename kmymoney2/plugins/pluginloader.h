/***************************************************************************
                          pluginloader.h
                             -------------------
    begin                : Thu Feb 12 2009
    copyright            : (C) 2009 Cristian Onet
    email                : onet.cristian@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef PLUGINLOADER_H
#define PLUGINLOADER_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// ----------------------------------------------------------------------------
// QT Includes

#include <qwidget.h>
#include <qscrollview.h>

// ----------------------------------------------------------------------------
// KDE Includes
#include <kplugininfo.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/export.h>

class KPluginSelector;

namespace KMyMoneyPlugin
{
  class Plugin;

  class KMYMONEY_EXPORT PluginLoader : public QObject
  {
    Q_OBJECT
  public:
    PluginLoader(QObject* parent);
    virtual ~PluginLoader();
    static PluginLoader* instance();

    void loadPlugins();
    Plugin* getPluginFromInfo(KPluginInfo*);
    KPluginSelector* pluginSelectorWidget();

  private:
    void loadPlugin(KPluginInfo*);

  signals:
    void plug(KPluginInfo*);
    void unplug(KPluginInfo*);
    void configChanged(Plugin*);  // consfiguration of the plugin has changed not the enabled/disabled state

  private slots:
    void changed();
    void changedConfigOfPlugin( const QCString & );

  private:
    struct Private;
    Private* d;
  };
}

#endif /* PLUGINLOADER_H */
