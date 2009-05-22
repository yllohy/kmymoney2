/***************************************************************************
                          pluginloader.cpp
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

// ----------------------------------------------------------------------------
// QT Includes

#include <qstringlist.h>
#include <qcheckbox.h>
#include <qlayout.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <ktrader.h>
#include <kparts/componentfactory.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kconfig.h>
#include <kpluginselector.h>
#include <klocale.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyplugin.h"
#include "pluginloader.h"

namespace KMyMoneyPlugin {

//---------------------------------------------------------------------
//
// PluginLoader
//
//---------------------------------------------------------------------
static PluginLoader* s_instance = 0;

typedef QMap<QString, Plugin*> PluginsMap;

struct PluginLoader::Private
{
  QObject*          m_parent;
  KPluginInfo::List m_pluginList;
  KPluginSelector*  m_pluginSelector;
  PluginsMap        m_loadedPlugins;
};

PluginLoader::PluginLoader(QObject* parent)
{
  Q_ASSERT( s_instance == 0 );
  s_instance = this;

  d = new Private;

  d->m_parent = parent;

  KTrader::OfferList offers = KTrader::self()->query("KMyMoneyPlugin");
  d->m_pluginList = KPluginInfo::fromServices(offers);

  d->m_pluginSelector = new KPluginSelector(NULL);
  d->m_pluginSelector->setShowEmptyConfigPage(false);
  d->m_pluginSelector->addPlugins(d->m_pluginList);
  d->m_pluginSelector->load();

  connect(d->m_pluginSelector, SIGNAL(changed(bool)), this, SLOT(changed()));
  connect(d->m_pluginSelector, SIGNAL(configCommitted(const QCString &)), this, SLOT(changedConfigOfPlugin(const QCString &)));
}

PluginLoader::~PluginLoader()
{
  delete d;
}

void PluginLoader::loadPlugins()
{
  for( KPluginInfo::List::Iterator it = d->m_pluginList.begin(); it != d->m_pluginList.end(); ++it )
    loadPlugin( *it );
}

void PluginLoader::loadPlugin(KPluginInfo* info)
{
  if (info->isPluginEnabled()) {
    Plugin* plugin = getPluginFromInfo(info);

    if (!plugin) {
      // the plugin is enabled but it is not loaded
      KService::Ptr service = info->service();
      int error = 0;
      Plugin* plugin = KParts::ComponentFactory
                ::createInstanceFromService<Plugin>(service, d->m_parent, info->name().utf8(), QStringList(), &error);
      if (plugin) {
        kdDebug() << "KMyMoneyPlugin::PluginLoader: Loaded plugin " << plugin->name() << endl;
        d->m_loadedPlugins.insert(info->name(), plugin);
        emit PluginLoader::instance()->plug(info);
      }
      else {
        kdWarning() << "KMyMoneyPlugin::PluginLoader:: createInstanceFromService returned 0 for "
                    << info->name()
                    << " with error number "
                    << error << endl;
        if (error == KParts::ComponentFactory::ErrNoLibrary)
          kdWarning() << "KLibLoader says: "
                      << KLibLoader::self()->lastErrorMessage() << endl;
      }
    }
  }
  else {
    if (getPluginFromInfo(info) != NULL) {
      // everybody interested should say goodbye to the plugin
      emit PluginLoader::instance()->unplug(info);
      d->m_loadedPlugins.erase(info->name());
    }
  }
}

void PluginLoader::changed()
{
  loadPlugins();
}

void PluginLoader::changedConfigOfPlugin(const QCString & name)
{
  PluginsMap::iterator itPlugin = d->m_loadedPlugins.find(QString(name));
  if (itPlugin != d->m_loadedPlugins.end())
    configChanged(*itPlugin);
}

Plugin* PluginLoader::getPluginFromInfo(KPluginInfo* info)
{
  PluginsMap::iterator itPlugin = d->m_loadedPlugins.find(info->name());
  if (itPlugin != d->m_loadedPlugins.end())
    return *itPlugin;
  else
    return NULL;
}

PluginLoader* PluginLoader::instance()
{
  Q_ASSERT( s_instance != 0);
  return s_instance;
}

KPluginSelector* PluginLoader::pluginSelectorWidget()
{
  return d->m_pluginSelector;
}

} // namespace

#include "pluginloader.moc"
